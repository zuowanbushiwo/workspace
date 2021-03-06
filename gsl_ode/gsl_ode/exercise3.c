/*
 * main.c
 *
 *  Created on: Oct 21, 2013
 *      Author: rolund03
 */

#include <stdlib.h>
#include <stdio.h>

#include <gsl_errno.h>
#include <gsl_matrix.h>
#include <gsl_odeiv.h>
//#include <gsl_odeiv2.h>


typedef struct vdp_params{
	double alpha;
	double beta;
	double gamma;
	double gmax;
	int count;
	int jac_count;
} vdp_params;


int vdp(double t, const double y[], double dydt[], void *params);
int jac_vdp(double t, const double y[], double dfdy[], double dfdt[], void *params);

int main(int argc, char** argv){
	if(argc < 3){
		fprintf(stderr, "%s gamma gmax\n", argv[0]);
		return EXIT_FAILURE;
	}

	const gsl_odeiv_step_type *T = gsl_odeiv_step_rkf45;
	gsl_odeiv_step 		*s = gsl_odeiv_step_alloc(T, 3);
	gsl_odeiv_control 	*c = gsl_odeiv_control_y_new(1e-6, 0.0); //(abs, rel)
	gsl_odeiv_evolve 	*e = gsl_odeiv_evolve_alloc(3); // 3-dimensional
	vdp_params par = {2, 4, atof(argv[1]), atof(argv[2]), 0, 0};

	gsl_odeiv_system sys = {vdp, jac_vdp, 3, &par};

	double t 	= 0.0;	//Start at this value
	double t1 	= 20;
	double interval = 0.1;
	double h	= 1e-6;	//Start at this timestep
	double y[3]	= {1.0, 0.1, 3.0}; //Initial condition

	while(t < t1){
		int status = gsl_odeiv_evolve_apply(e, c, s, &sys, &t, t1, &h, y);
		if(status != GSL_SUCCESS){
			fprintf(stderr, "Could not evaluate step.");
			break;
		}
//		if(t > t2 + interval){
			printf("%.5e %.5e %.5e %.5e\n", t, y[0], y[1], y[2]);
//			t2 = t;
//		}
	}

	//Print out data to stderr
	fprintf(stderr, "count: %d\n", par.count);
	fprintf(stderr, "jaccount: %d\n", par.jac_count);

	//Free up memory
	gsl_odeiv_evolve_free(e);
	gsl_odeiv_control_free(c);
	gsl_odeiv_step_free(s);
	return EXIT_SUCCESS;
}

int vdp(double t, const double y[], double dydt[], void *params){
	vdp_params *p = (vdp_params *)params;
	++p->count;
	double alpha = p->alpha;
	double beta = p->beta;
	double gamma = p->gamma;
	double gmax = p->gmax;

	dydt[0] = y[0]*(1 - y[1] - y[0]/y[2]);
	dydt[1] = alpha*(y[0]-1)*y[1];
	dydt[2] = gamma - beta*y[0]; //gamma*(1-y[2]/gmax) - beta*y[0];

	return GSL_SUCCESS;

}

//TODO: what is dfdt?
int jac_vdp(double t, const double y[], double dfdy[], double dfdt[], void *params){
	vdp_params *p = (vdp_params *)params;
	++p->jac_count;
	double alpha = p->alpha;
	double beta = p->beta;
	double gamma = p->gamma;
	double gmax = p->gmax;

	gsl_matrix_view dfdy_mat = gsl_matrix_view_array(dfdy, 2, 2);
	gsl_matrix *m = &dfdy_mat.matrix;

	//Set value at (i,j) to something
	gsl_matrix_set(m, 0, 0, 1 - y[1]-2*y[0]/y[2]);
	gsl_matrix_set(m, 0, 1, -y[0]);
	gsl_matrix_set(m, 0, 2, y[0]*y[0]/(y[2]*y[2]));

	gsl_matrix_set(m, 1, 0, alpha*y[1]);
	gsl_matrix_set(m, 1, 1, alpha*(y[0]-1));
	gsl_matrix_set(m, 1, 2, 0);

	gsl_matrix_set(m, 2, 0, -beta);
	gsl_matrix_set(m, 2, 1, 0);
	gsl_matrix_set(m, 2, 2, -gamma/gmax);

	dfdt[0] = 0.0;
	dfdt[1] = 0.0;
	dfdt[2] = 0.0;

	return GSL_SUCCESS;
}


int save_vector(gsl_vector *v, const char *file){
	FILE *f = fopen(file, "w+");
	if(f){
		gsl_vector_fprintf(f, v, "%f");
	}
	else{
		perror("fopen");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
