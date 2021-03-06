/*
 * main.c
 * Reads a file with machine code in each row for the mips processor
 * and prints out the following information of the instruction:
 * 	-Instruction.
 * 	-Type of instruction.
 * 	-bits divided into specific fields for the format in both dec and hex.
 * 	-The assembly syntax for the instruction.
 *
 *  Created on: Nov 17, 2013
 *      Author: Robin Lundberg, ens10rlg
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h"


//Function Prototypes
unsigned int getop(unsigned int inst);
unsigned int getrs(unsigned int instruction, char *regName);
unsigned int getrt(unsigned int instruction, char *regName);
unsigned int getrd(unsigned int instruction, char *regName);
unsigned int getimm(unsigned int instruction);
unsigned int getshamt(unsigned int instruction, char *regName);
unsigned int getaddress(unsigned int instruction);
void strrep(char *out, const char *old, const char *new);
void printOperation(unsigned int instruction);
struct funccode_t * getfunc(struct opcode_t *op, unsigned int instruction);


int main(int argc, char *argv[]){
	FILE *file;
	char line[256];
	unsigned int instruction;

	//Check input
	if(argc != 2){
		fprintf(stderr, "Use the syntax: %s file", argv[0]);
		exit(EXIT_FAILURE);
	}
	file = fopen("machinecode.txt", "r");
	if(file == NULL){
		fprintf(stderr, "Can't open file: %s.", argv[1]);
		exit(EXIT_FAILURE);
	}

	//Print out info on instruction
	printf("Instruction \tType \tField values in dec \tField values in hex \tmnemonic representation\n");
	while (fgets(line, sizeof(line), file ) != NULL ){
		instruction = strtol(line, NULL, 0);
		printOperation(instruction);
	}

	fclose(file);
	return EXIT_SUCCESS;
}

/*	\brief retrieves the op code for the current instruction.
 * 	\param	instruction - the current instruction.
 * 	\return numeric value of the op code.
 */
unsigned int getop(unsigned int inst){
	return inst>>26;
}

/*	\brief Prints out information for an instruction.
 * 	\param instruction - the current instruction.
 */
void printOperation(unsigned int instruction){
	unsigned int opcode;
	struct opcode_t *op;
	struct funccode_t *func;
	char regName[6];
	unsigned int rs;
	unsigned int rd;
	unsigned int rt;
	unsigned int imm;
	unsigned int shamt;
	unsigned int address;

	//If instruction is zero, this is the nop instruction
	if(instruction == 0){
		printf("%10d; \t\t\t\t\t\t\t\tnop\n", 0);
		return;
	}

	//Check and see if instruction is valid
	opcode = getop(instruction);
	if(opcode >= sizeof(operations)/sizeof(struct opcode_t)){
		printf("%10u; \t%s; \t%s\n", instruction, NO_INSTRUCTION, "invalid opcode.");
		return;
	}
	op = &operations[opcode];
	if(op->type == INVALIDOP){
		if(op->opnum == 0x10 || op->opnum == 0x12){
			printf("%10u; \t%s\n", instruction, "Floating point instructions not supported.");
			return;
		}
		printf("%10u; \t%s; \t%s\n", instruction, NO_INSTRUCTION, "invalid opcode.");
		return;
	}
	func = getfunc(op, instruction);
	//Test to see if unsupported floating point instruction.
	if(op->opnum == 0x11 && func==NULL){
		printf("%10u; \t%s\n", instruction, "Floating point instructions not supported.");
		return;
	}
	if(func == NULL){
		printf("%10u; \t%s; \t%s\n", instruction, NO_INSTRUCTION, "invalid function code.");
		return;
	}
	char out[128];
	strcpy(out, func->name);

	//Special case syscall
	if(op->opnum == 0 && func->funcnum == 0xc){
		unsigned int code = instruction&0x3ffffc0;
		if(code != 0){
			printf("%10u; \t%s; \t%s\n", instruction, NO_INSTRUCTION, "bit 6 to 25 need to be all zeros.");
			return;
		}
		printf("%10d; \tO; \t",instruction);
		printf("%2u %7u %2u; \t\t",op->opnum, code, func->funcnum);
		printf("%2x %7x %2x; \t\t",op->opnum, code, func->funcnum);
		printf("%s\n", func->name);
		return;
	}

	//Special case break
	if(op->opnum == 0 && func->funcnum == 0xd){
		unsigned int code = instruction&0x3ffffc0;
		printf("%10d; \tO; \t",instruction);
		printf("%2u %7u %2u; \t",op->opnum, code, func->funcnum);
		printf("%2x %7x %2x; \t\t",op->opnum, code, func->funcnum);
		printf("%s %u\n", func->name, code);
		return;
	}

	//Print out the operation
	switch(op->type){
		case(R):
			rs = getrs(instruction, regName);

			strrep(out, "%rs%", regName);

			rd = getrd(instruction, regName);
			strrep(out, "%rd%", regName);

			rt = getrt(instruction, regName);

			//Special case for the movf operation
			if(op->opnum==0 && func->funcnum==1){
				unsigned int cc = rt>>2;
				sprintf(regName, "%u", cc);
			}

			strrep(out, "%rt%", regName);

			shamt = getshamt(instruction, regName);
			strrep(out, "%shamt%", regName);

			printf("%10u; \tR; \t", instruction);
			printf("%2u %2u %2u %2u %2u %2u; \t", op->opnum, rs, rd, rt, shamt, func->funcnum);
			printf("%2x %2x %2x %2x %2x %2x; \t", op->opnum, rs, rd, rt, shamt, func->funcnum);
			break;
		case(I):
			rs = getrs(instruction, regName);
			strrep(out, "%rs%", regName);

			rt = getrt(instruction, regName);
			//Special case for op=0x11
			if(op->opnum == 0x11){
				unsigned int cc;
				char buff[128];
				if((rs == 8 && (rt&0x03) == 0) || (rs == 8 && (rt&0x03) == 1)){
					cc = rt>>2;
					sprintf(regName, "%u", cc);
				}
				else if(rs == 0 || rs == 4){
					getrd(instruction, buff);
					strrep(out, "%rd%", buff);
				}
			}

			strrep(out, "%rt%", regName);

			imm = getimm(instruction);
			sprintf(regName, "%u", imm);
			strrep(out, "%imm%", regName);

			printf("%10u; \tI; \t", instruction);
			printf("%2u %2u %2u %5u; \t", op->opnum, rs, rt, imm);
			printf("%2x %2x %2x %5x; \t", op->opnum, rs, rt, imm);
			break;
		case(J):
			address = getaddress(instruction);
			sprintf(regName, "%u", address);
			strrep(out, "%address%", regName);

			printf("%10u; \tJ; \t", instruction);
			printf("%2u %8u; \t\t", op->opnum, address);
			printf("%2x %8x; \t\t", op->opnum, address);
			break;
		default:
			printf("Should never reach this, the programmer sucks!");
			exit(EXIT_FAILURE);
	}
	printf("%s\n", out);
}

/*	\brief replaces substring in a string for another substring.
 * 	\param	str 	- pointer to string.
 * 	\param	orig 	- original substring to be removed.
 * 	\param	rep 	- the substring to replace the original.
 */
void strrep(char *str, const char *orig, const char *rep){
	char buffer[256];
	char *p;

	if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
		return;

	strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
	buffer[p-str] = '\0';

	sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

	strcpy(str, buffer);
}

/*	\brief retrieves a pointer to a function code corresponding to the current instruction.
 * 	\param	op 			- struct to given op code.
 * 	\param	instruction	- current instruction.
 * 	\return pointer to function struct corresponding to instruction
 */
struct funccode_t * getfunc(struct opcode_t *op, unsigned int instruction){
	struct funccode_t *func;
	unsigned int funccode;

	//Special case for op=0x11
	if(op->opnum == 0x11){
		unsigned int rt, rs;
		char buffer[128];
		func = NULL;
		rt = getrt(instruction, buffer);
		rs = getrs(instruction, buffer);
		if(rs == 8 && (rt&0x03) == 0)
			func  = &op->func[0];
		else if(rs == 8 && (rt&0x03) == 1)
			func  = &op->func[1];
		else if(rs == 0)
			func  = &op->func[2];
		else if(rs == 4)
			func  = &op->func[3];
		return func;
	}

	if(op->funcstart == 0){
		func = op->func;
	}
	else{
		funccode = instruction << (32-op->funcstart);
		if(op->type == R)
			funccode >>= 26;
		else if(op->opnum == 1)
			funccode >>= 27;
		if(funccode >= op->maxfunc){
			func = NULL;
			return func;
		}
		else
			func = &op->func[funccode];
	}
	if(func->funcnum == INVALIDFUNC){
		func = NULL;
	}
	return func;
}

/*	\brief Gets the value of the register field and stores it's corresponding name
 * 			in given string.
 * 	\param	instruction	- the current instruction.
 * 	\param	regName		- string to store the name for given register.
 * 	\return numeric value for the register.
 */
unsigned int getrs(unsigned int instruction, char *regName){
	unsigned int rs;
	rs = instruction << 6;
	rs >>= 27;
	strcpy(regName, registers[rs]);
	return rs;
}

/*	\brief Gets the value of the register field and stores it's corresponding name
 * 			in given string.
 * 	\param	instruction	- the current instruction.
 * 	\param	regName		- string to store the name for given register.
 * 	\return numeric value for the register.
 */
unsigned int getrt(unsigned int instruction, char *regName){
	unsigned int rt;
	rt = instruction << 11;
	rt >>= 27;
	strcpy(regName, registers[rt]);
	return rt;
}

/*	\brief Gets the value of the register field and stores it's corresponding name
 * 			in given string.
 * 	\param	instruction	- the current instruction.
 * 	\param	regName		- string to store the name for given register.
 * 	\return numeric value for the register.
 */
unsigned int getrd(unsigned int instruction, char *regName){
	unsigned int rd;
	rd = instruction << 16;
	rd >>= 27;
	strcpy(regName, registers[rd]);
	return rd;
}

/*	\brief Gets the value of the register field and stores it's corresponding name
 * 			in given string.
 * 	\param	instruction	- the current instruction.
 * 	\param	regName		- string to store the name for given register.
 * 	\return numeric value for the register.
 */
unsigned int getshamt(unsigned int instruction, char *regName){
	unsigned int shamt;
	shamt = instruction << 21;
	shamt >>= 27;
	strcpy(regName, registers[shamt]);
	return shamt;
}

/*	\brief Gets the value of the immediate field.
 * 	\param	instruction	- the current instruction.
 * 	\return numeric value for the immediate field.
 */
unsigned int getimm(unsigned int instruction){
	unsigned int imm;
	imm = instruction << 16;
	imm >>= 16;
//	strcpy(regName, registers[imm]);
	return imm;
}

/*	\brief Gets the value of the address field.
 * 	\param	instruction	- the current instruction.
 * 	\return numeric value for the address field.
 */
unsigned int getaddress(unsigned int instruction){
	unsigned int address;
	address = instruction << 6;
	address >>= 6;
//	strcpy(regName, registers[address]);
	return address;
}
