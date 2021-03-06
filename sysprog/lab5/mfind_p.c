/*
 * mfind_p.c
 *
 *  Created on: Oct 23, 2013
 *      Author: ens10rlg
 *      Name: Robin Lundberg
 *  	Laboration 5, systemnara programmering
 *  	Searches recursively for a given file using
 *  	specified number of threads.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <pcre.h>
#include <pthread.h>

#define IS_FILE 		S_IFREG
#define IS_DIRECTORY 	S_IFDIR
#define IS_LINK 		S_IFLNK
#define IS_ANY 			(IS_FILE|IS_DIRECTORY|IS_LINK)

#define S_ISTYPE(mode, type)		(__S_ISTYPE(mode, type)||(type)==IS_ANY)

#define DIR_MAX_NAME_LENGTH 0x800
#define USAGE "[-t type] [-p nrthr] start1 [start2 ...] name"
#define EXIT_ALLOC_FAIL(ptr) if(ptr==NULL){fprintf(stderr, "Failed to allocate memory, exiting program!"); exit(EXIT_FAILURE);}

#define SEM_ITEMS "sem_items_mfind_p"

struct node {
//   unsigned int uid;
   char* directory;
   struct node* next;
};

typedef struct par_t{
    char **dirs;
    int numDirs;
    int nrthr;
    int type;
    char* name;
} par_t;

typedef struct arg_t{
	pcre *find;
	int type;
} arg_t;

void * taskManager(void *pArg);
void mfind_p(char *path, pcre *find, int type);
void read_arguments(int argc, char *argv[], par_t *par);
void replace_str(char *str, char *orig, char *rep);
char * popTask(void);
void pushTask(char* v);

//Global Variables
pthread_mutex_t accessStackBlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t getTaskBlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t taskInitBlock = PTHREAD_MUTEX_INITIALIZER;
struct node *taskStack; //FIFO

int nrthr = 1;
int waiting = 0;
int quit = 0;
int tInit = 0;
short *blocked;
pthread_t *tid;
arg_t pArg;
par_t par;
pcre *find;


//mfind_p [-t type]  [-p nrthr] start1 [start2 ...]
//type = d,f,l; d=directory, f=file, l=link.
int main(int argc, char *argv[]){
    unsigned int n;
    const char *error;
    int erroffset;


    //Read in and save number program arguments.
    if(argc <= 2){
    	fprintf(stderr, "Bad input, usage: %s %s\n", argv[0], USAGE);
    	exit(EXIT_FAILURE);
    }
    read_arguments(argc, argv, &par);

    //Convert name to regular expresion syntax
    char regexp[256];
    sprintf(regexp, "^%s$", par.name);

    //substitutue \* by .*
    replace_str(regexp, "*", ".*");

    //Pre-compile regular expression
    find = pcre_compile(regexp, 0, &error, &erroffset, NULL);
    if(find == NULL){
    	fprintf(stderr, "%s\n", error);
    	exit(EXIT_FAILURE);
    }

    //Set up thread blocks
    if(pthread_mutex_init(&accessStackBlock, NULL) != 0){
    	fprintf(stderr, "Could not initialize mutex!\n");
    }
    if(pthread_mutex_init(&getTaskBlock, NULL) != 0){
    	fprintf(stderr, "Could not initialize mutex!\n");
    }
    if(pthread_mutex_init(&taskInitBlock, NULL) != 0){
    	fprintf(stderr, "Could not initialize mutex!\n");
    }

    //Set up taskpool
    taskStack = NULL;
    for(n=0; n<par.numDirs; n++){
    	pushTask(par.dirs[n]);
    }


    //Allocate memory for thread pointers
    nrthr = par.nrthr;
    tid = malloc((nrthr-1)*sizeof(pthread_t));
    EXIT_ALLOC_FAIL(tid);
    blocked = calloc(nrthr, sizeof(short));
    EXIT_ALLOC_FAIL(blocked);

    //SEARCH!
    pArg.find = find;
    pArg.type = par.type;
    for(n=0; n<nrthr-1; n++){
    	if(pthread_create(&tid[n], NULL, &taskManager, (void*)&pArg) < 0){
    		perror("pthreade_create error");
    		exit(EXIT_FAILURE);
    	}
    }
    taskManager(&pArg);

    //Close threads
    for(n=0; n<nrthr-1; n++){
    	if(pthread_join(tid[n], NULL) != 0){
    		perror("pthread_join");
    		exit(EXIT_FAILURE);
    	}
    }

    //Free memory
    pcre_free(find);
    free(tid);
    for(n=0; n<par.numDirs; n++){
    	free(par.dirs[n]);
    }
    free(par.dirs);
    free(par.name);

	return 0;
}


/*	\brief Handles task for the threads and quits when finished.
 * 	\param	pArg 	- contains regular expression and type of file to find.
 * 	\return NULL
 */
void* taskManager(void *pArg){
	unsigned int count = 0;
	arg_t *p = (arg_t*) pArg;
	pcre *find = p->find;
	int type = p->type;
	int tNum;

	pthread_mutex_lock(&taskInitBlock);
	tNum = tInit;
	tInit++;
	pthread_mutex_unlock(&taskInitBlock);

	while(1){
		pthread_mutex_lock(&getTaskBlock);
		pthread_mutex_lock(&accessStackBlock);
		char *path = popTask();
		pthread_mutex_unlock(&accessStackBlock);
		if(path == NULL){
			if(!blocked[tNum]){
				waiting++;
				blocked[tNum] = 1;
			}
		}
		else{
			waiting = 0;
			memset(blocked, 0, nrthr*sizeof(short));
		}
		if(waiting >= nrthr){
			quit = 1;
		}
		pthread_mutex_unlock(&getTaskBlock);
		if(path == NULL){
			if(quit == 1)
				break;
			else
				continue;
		}
		else{
			mfind_p(path, find, type);
			free(path);
			count++;
		}
	}
	printf("%lu: %u\n", (unsigned long)pthread_self(), count);
	return NULL;
}


/*	\brief Goes through all files in a folder. Adds the folders fined into the task stack and tries to find
 * 			the file searched for.
 * 	\param	path 	- path to the folder to search in.
 * 	\param	find 	- Regular expression with what to find.
 * 	\param	type 	- Type of file to find.
 * 	\return None
 */
void mfind_p(char *path, pcre *find, int type){
	struct dirent *dent;
	struct stat fileStat;
	int success;
	int rc;
	DIR *dir;

	dir = opendir(path);
	if(dir == NULL){
		if(errno == EACCES) fprintf(stderr, "No permission to open %s\n", path);
		else if(errno == ENOENT) fprintf(stderr, "The path does not exit: %s\n", path);
		else fprintf(stderr, "ERROR: Directory %s could not be opened.\n", path);
		return;
	}

	//Try to find match in dir and save all directories to search deeper
	while((dent=readdir(dir)) != NULL){
		//Check if file found
		rc = pcre_exec(find, NULL, dent->d_name, strlen(dent->d_name), 0, 0, NULL, 0);

		//Store path to file
		char* filePath = malloc(strlen(path)+strlen(dent->d_name)+2);
		EXIT_ALLOC_FAIL(filePath);
		sprintf(filePath, "%s/%s", path, dent->d_name);

		//Get info on file
		success = lstat(filePath, &fileStat);
		if(success == -1){
			if(errno == EACCES) fprintf(stderr, "No permission to open %s\n", filePath);
			else if(errno == ENOENT) fprintf(stderr, "The path does not exit: %s\n", filePath);
			else fprintf(stderr, "ERROR: Directory %s could not be opened.\n", filePath);
			free(filePath);
			continue;
		}
		//Print out file found, if found
		if(!(rc & PCRE_ERROR_NOMATCH) && S_ISTYPE(fileStat.st_mode, type) ){
			printf("%s\n", filePath);	//Found files
		}

		//If another folder, put it in tasklist
		if( S_ISDIR(fileStat.st_mode) && strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")){ //If directory, check it
			pthread_mutex_lock(&accessStackBlock);
			pushTask(filePath);
			pthread_mutex_unlock(&accessStackBlock);
		}
		free(filePath);
	}
	closedir(dir);
}


/*	\brief Pushes task into stack.
 * 	\param	v 	- pushes this into the stack
 * 	\return None
 */
void pushTask(char* v){
	struct node *node = malloc(sizeof(struct node));
	EXIT_ALLOC_FAIL(node);
	node->directory = malloc(strlen(v)+1);
	EXIT_ALLOC_FAIL(node->directory);

	strcpy(node->directory, v);

	node->next = taskStack;
	taskStack = node;
}


/*	\brief Retrieves the task at the top of the stack and remove it from stack.
 * 	\return pointer to the task in the top of the stack
 */
char * popTask(void){
	struct node *node;

	if(taskStack == NULL){
		return NULL;
	}
	char *top = malloc(strlen(taskStack->directory)+1);
	EXIT_ALLOC_FAIL(top);
	strcpy(top, taskStack->directory);

	node = taskStack->next;
	free(taskStack->directory);
	free(taskStack);
	taskStack = node;

	return top;
}


/*	\brief Read input arguments and stores directories to search from, search pattern, depth and type.
 * 	\param	argc 	- Number of input arguments
 * 	\param	argv 	- Input arguments
 * 	\param	par 	- Struct to store all input arguments in a suitable format
 * 	\return None
 */
void read_arguments(int argc, char *argv[], par_t *par){
	unsigned int n, m;
	int opt;
	char *optstring = "t:p:";
	extern char* optarg;
	extern int optind, opterr, optopt;

	par->type = 'a';
	while ((opt = getopt(argc, argv, optstring)) != -1){
	    if(opt == 't'){
	    	par->type = optarg[0];
	    }
	    else if(opt == 'p'){
	    	par->nrthr = atoi(optarg);
	    	if(par->nrthr < 1) par->nrthr = 1;
	    }
	    else{
	        fprintf(stderr, "Usage: %s %s\n", argv[0], USAGE);
	        exit(EXIT_FAILURE);
	    }
	}

    switch(par->type){
    case 'a':
    	par->type = IS_ANY;
    	break;
    case 'd':
    	par->type = IS_DIRECTORY;
    	break;
    case 'f':
    	par->type = IS_FILE;
    	break;
    case 'l':
    	par->type = IS_LINK;
    	break;
    default:
	    fprintf(stderr, "type can be:\n%s\n%s\n%s\n%s\n", "a\t- Any type (Default)", \
                "d\t- Directory", "f\t- Regular file", "l\t- Link");
	    exit(EXIT_FAILURE);
    	break;
    }

	par->dirs = (char**)malloc((argc-2)*sizeof(char*)); //More than enough, but that's ok
	EXIT_ALLOC_FAIL(par->dirs);
	m=0;
	if(optind == argc-1){
		fprintf(stderr, "Usage: %s %s\n", argv[0], USAGE);
	}
	for(n=optind; n<argc-1; n++){
		par->dirs[m] = malloc(strlen(argv[n]) + 1);
		EXIT_ALLOC_FAIL(par->dirs[m]);
		strcpy(par->dirs[m], argv[n]);
		m++;
	}

	par->numDirs = m;

	par->name = malloc(strlen(argv[argc-1]) + 1);
	EXIT_ALLOC_FAIL(par->name);
	strcpy(par->name, argv[argc-1]);

	return;
}


/*	\brief Replaces substring with another substring in input string.
 * 	\param	str 	- String to have substring replaced
 * 	\param	orig 	- substring to be replaced
 * 	\param	rep 	- new substring inserted
 * 	\return pointer to new string
 */
void replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  strcpy(str, buffer);
  return;
}
