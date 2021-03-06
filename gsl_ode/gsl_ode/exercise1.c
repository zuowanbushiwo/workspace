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


typedef struct vdp_params{
	double mu;
	int count;
	int jac_count;
} vdp_params;


int vdp(double t, const double y[], double dydt[], void *params);
int jac_vdp(double t, const double y[], double dfdy[], double dfdt[], void *params);

int main(int argc, char** argv){
	const gsl_odeiv_step_type *T;

	if(argc < 3){
		fprintf(stderr, "%s solver mu", argv[0]);
		return EXIT_FAILURE;
	}

	int solv = atoi(argv[1]);

	switch(solv){
	case 45:
		T = gsl_odeiv_step_rkf45; break;
	case 2:
		T = gsl_odeiv_step_rk2simp; break;
	case 4:
		T = gsl_odeiv_step_rk4imp; break;
	}

	gsl_odeiv_step 		*s = gsl_odeiv_step_alloc(T, 2);
	gsl_odeiv_control 	*c = gsl_odeiv_control_y_new(1e-6, 0.0); //(abs, rel)
	gsl_odeiv_evolve 	*e = gsl_odeiv_evolve_alloc(2); // 2-dimensional
	vdp_params par = {atof(argv[2]), 0, 0}; //mu should be around 10

	gsl_odeiv_system sys = {vdp, jac_vdp, 2, &par};

	double t 	= 0.0;	//Start at this value
	double t1 	= 100.0; //Run to this value
	double h	= 1e-6;	//Start at this timestep
	double y[2]	= {1.0, 0.0}; //Initial condition

	//main loop
	while(t < t1){
		int status = gsl_odeiv_evolve_apply(e, c, s, &sys, &t, t1, &h, y);
		if(status != GSL_SUCCESS){
			fprintf(stderr, "Could not evaluate step.");
			break;
		}
		printf("%.5e %.5e %.5e\n", t, y[0], y[1]);
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
	double mu = p->mu;

	dydt[0] = y[1];
	dydt[1] = -y[0] - mu * y[1] * (y[0]*y[0] - 1.0);

	return GSL_SUCCESS;

}

//TODO: what is dfdt?
int jac_vdp(double t, const double y[], double dfdy[], double dfdt[], void *params){
	vdp_params *p = (vdp_params *)params;
	++p->jac_count;

	gsl_matrix_view dfdy_mat = gsl_matrix_view_array(dfdy, 2, 2);
	gsl_matrix *m = &dfdy_mat.matrix;

	//Set value at (i,j) to something
	gsl_matrix_set(m, 0, 0, 0);
	gsl_matrix_set(m, 0, 1, 0);
	gsl_matrix_set(m, 1, 0, 0);
	gsl_matrix_set(m, 1, 1, 0);

	dfdt[0] = 0.0;
	dfdt[1] = 0.0;

	return GSL_SUCCESS;
}
