/*****************************************************************************
 *	\file		lec.c
 *
 *	\date		2013-June-11
 *
 *	\brief		
 *
 *	Copyright	Limes Audio AB
 *
 *	This code should not be modified, copied, duplicated, reproduced, licensed
 *	or sublicensed without the prior written consent of Limes Audio AB.
 *	This code (or any right in the code) should not be transferred or conveyed
 *	without the prior written consent of Limes Audio AB.
 *
 *****************************************************************************/

// ---< Include >---
#include <string.h>
#include "true_voice_internal.h"
#include "lec.h"
#include "subband.h"
#include "fract_math.h"

#if USE_LEC

// ---< Defines >---

// ---< Global variabels >---

#pragma  DATA_ALIGN(pcq15NLMS_coefs_bg, 8)
#pragma  DATA_ALIGN(pcq15NLMS_coefs_fg, 8)
#pragma  DATA_ALIGN(pcq31NLMS_delay, 8)

// subband variables
complex_q31 pcq31RX_fft[NUM_ACTIVE_SUB];
complex_q31 pcq31LS_fft[NUM_ACTIVE_SUB];
complex_q31 pcq31MIC_fft[NUM_ACTIVE_SUB];
complex_q31 pcq31TX_fft[NUM_ACTIVE_SUB];

complex_q31 pcq31MICE_bg_fft[NUM_ACTIVE_SUB];
complex_q31 pcq31MICE_fg_fft[NUM_ACTIVE_SUB];
complex_q31 pcq31AErr_bg_fft[NUM_ACTIVE_SUB];
complex_q31 pcq31AErr_fg_fft[NUM_ACTIVE_SUB];

// AEC coefficients
complex_q15 pcq15NLMS_coefs_bg[NUM_ACTIVE_SUB][NLMS_FILTER_LENGTH];
complex_q15 pcq15NLMS_coefs_fg[NUM_ACTIVE_SUB][NLMS_FILTER_LENGTH];
// Delay buffer for AEC
complex_q31 pcq31NLMS_delay[NUM_ACTIVE_SUB][NLMS_FILTER_LENGTH];
int uiNLMS_delay_index;
// Loudspeaker energy
q31 pq31NLMS_energy[NUM_ACTIVE_SUB];

// Estimated Return Loss Enhancement
q15 pq15AErle_bg[NUM_ACTIVE_SUB];
q15 pq15AErle_fg[NUM_ACTIVE_SUB];
q15 pq15AErle_fg_compensation[NUM_ACTIVE_SUB];

// For testign/plotting
bool bAECUpdate[NUM_ACTIVE_SUB];
bool bCopyFilter[NUM_ACTIVE_SUB];
bool bResetFilter[NUM_ACTIVE_SUB];

// Filter divergence
q31 pq31Filter_div_bg[NUM_ACTIVE_SUB];
q31 pq31Filter_div_fg[NUM_ACTIVE_SUB];
complex_q31 pcq31EYhat_bg_ave[NUM_ACTIVE_SUB];
complex_q31 pcq31EYhat_fg_ave[NUM_ACTIVE_SUB];
complex_q31 pcq31YYhat_bg_ave[NUM_ACTIVE_SUB];
complex_q31 pcq31YYhat_fg_ave[NUM_ACTIVE_SUB];
q31 pcq31YhatYhat_fg_ave[NUM_ACTIVE_SUB];

// TX damping
q15 pq15AErr_damp[NUM_ACTIVE_SUB];
q15 pq15AErr_damp_set[NUM_ACTIVE_SUB];

q15 pq15AFeedback[NUM_ACTIVE_SUB];

// Echo tail measures (background filter)
q31 pq31GammaValue_bg[NUM_ACTIVE_SUB];
q15 pq15LastValue_bg[NUM_ACTIVE_SUB];
unsigned int puiMaxIndex_bg[NUM_ACTIVE_SUB];
q15 pq15MaxValue_bg[NUM_ACTIVE_SUB];
// Echo tail measures (foreground filter)
q31 pq31GammaValue_fg[NUM_ACTIVE_SUB];
q15 pq15LastValue_fg[NUM_ACTIVE_SUB];
unsigned int puiMaxIndex_fg[NUM_ACTIVE_SUB];

q31 pq31RX_ave[NUM_ACTIVE_SUB];
q31 pq31LS_ave[NUM_ACTIVE_SUB];
q31 pq31MIC_ave[NUM_ACTIVE_SUB];
q31 pq31TX_ave[NUM_ACTIVE_SUB];

q31 pq31MIChat_bg_ave[NUM_ACTIVE_SUB];
q31 pq31MIChat_fg_ave[NUM_ACTIVE_SUB];

q31 pq31AErr_bg_ave[NUM_ACTIVE_SUB];
q31 pq31AErr_fg_ave[NUM_ACTIVE_SUB];

// For ERLE estimation
q31 pq31MIC_ave_slow[NUM_ACTIVE_SUB];
q31 pq31AErr_bg_ave_slow[NUM_ACTIVE_SUB];
q31 pq31AErr_fg_ave_slow[NUM_ACTIVE_SUB];

q31 pq31LS_hold_ave[NUM_ACTIVE_SUB];
q31 pq31LS_tail_ave[NUM_ACTIVE_SUB];

// Magnitude of LS sample being shifted out from the AEC
// filter delay vector
q31 pq31LS_tail_abs[NUM_ACTIVE_SUB];

// Estimated noise level in each subband
q31 pq31LS_noi_est[NUM_ACTIVE_SUB];
q31 pq31MIC_noi_est[NUM_ACTIVE_SUB];

// Buffers for LS_hold
static q31 pq31LS_hold1[NUM_ACTIVE_SUB];
static q31 pq31LS_hold2[NUM_ACTIVE_SUB];

// Analysis / synthesis buffers
static q15 pq15Prot_delayLS[NUM_PROT_COEFS];
static q15 pq15Prot_delayMIC[NUM_PROT_COEFS];
static q31 pq31Prot_delayTX[NUM_PROT_COEFS];

// Circular buffer indices
static int iProt_delayIndexLS;
static int iProt_delayIndexMIC;
static int iProt_delayIndexTX;

// Noise level estimation settings
//noiEstSet_t rxNoiEstSet;
noiEstSet_t lsNoiEstSet;
noiEstSet_t micNoiEstSet;

// Comfort noise
complex_q31 pcq31CNoi_fft[NUM_ACTIVE_SUB];
// Delaybuffer
static q15 pq15Prot_delayCNOI[NUM_PROT_COEFS];
// Circular buffer index
static int iProt_delayIndexCNOI;

q31 pq31CNoi_ave[NUM_ACTIVE_SUB];

q31 q31DistEcho;
q31 pq31echoEst[NUM_ACTIVE_SUB];
q31 pq31SpeechEst[NUM_ACTIVE_SUB];

// Delay buffer so that residual dampings will have time to settle
static complex_q31 ppcq31TX_delay[NUM_ACTIVE_SUB][TX_DELAY];
bool bAtalk, bBtalk, bBtalk_instant, bBtalkStrong;


// ---< Local function prototypes >---

static void calc_erle_fg_compensation();
// Calculates aec filter deviation measures
static void calculate_filter_deviation();
// Copies bg filter to fg in one subband. Both filter coefficients and performance variables are copied.
static void filter_copy_bg2fg(unsigned int s);
// Resets bg-filter using fg-filter coefficients and performance
static void reset_bg_filter(unsigned int s);

static void averages(enum AVE_TYPES);
static void averages_init(void);

static void noise_est(const q31*, q31*, const complex_q31*, noiEstSet_t*,bool);
static void noise_gen_cnoi(complex_q31*, q31*, q15*);


static void resdamp_init();
static void resdamp_detect_atalk();
//void resdamp_detect_btalk();
static void resdamp_tx_gains_org(q15*);
static void resdamp_delay_tx(complex_q31*, complex_q31*);

static void lec_subband_nlms(complex_q31*, const complex_q31*, complex_q31*, complex_q31*);
static void lec_subband_fgfilt(const complex_q31*, complex_q31*, complex_q31*);
static void lec_check_performance();
static void lec_compare_filters();
//static void lec_ls_delay_put(complex_q31 *);
//static void lec_ls_delay_get(complex_q31 *);

// ---< Functions >---

void internal_lec_init(bool reset_all) {

	unsigned int i;

	subband_init();
	resdamp_init();
	averages_init();

	if(reset_all) {
		memset(pcq15NLMS_coefs_bg, 0, sizeof(complex_q15)*(NUM_ACTIVE_SUB)*NLMS_FILTER_LENGTH);
		memset(pcq15NLMS_coefs_fg, 0, sizeof(complex_q15)*(NUM_ACTIVE_SUB)*NLMS_FILTER_LENGTH);
	}
	memset(pq31NLMS_energy, 0, sizeof(q31)*(NUM_ACTIVE_SUB));
	memset(pcq31NLMS_delay, 0, sizeof(complex_q31)*(NUM_ACTIVE_SUB)*NLMS_FILTER_LENGTH);
	uiNLMS_delay_index = 0;
	for(i=0;i<NUM_ACTIVE_SUB;i++) {


		pq15AFeedback[i] =MAX_16; //TODO what should feedback be set to?

		// Reset ERLE
		pq15AErle_bg[i] = MAX_16;
		pq15AErle_fg[i] = MAX_16;

	 }

	// Clear analysis and synthesis delay vectors
	memset(pq15Prot_delayLS, 0, sizeof(int)*NUM_PROT_COEFS);
	memset(pq15Prot_delayMIC, 0, sizeof(short)*NUM_PROT_COEFS);
	memset(pq31Prot_delayTX, 0, sizeof(int)*NUM_PROT_COEFS);

	iProt_delayIndexLS = 0;
	iProt_delayIndexMIC = 0;
	iProt_delayIndexTX = 0;



	noi_est_init(&lsNoiEstSet,pq31LS_noi_est);
	noi_est_init(&micNoiEstSet,pq31MIC_noi_est);

	// Init delay buffer for generating comfort noise
	memset(pq15Prot_delayCNOI, 0, NUM_PROT_COEFS*sizeof(q15));
	iProt_delayIndexCNOI = 0;

	bAtalk = false;
}

void internal_lec(q15* line_in, q15* line_out, q15* line_in_processed) {
	q31 q31Temp[BUFLEN_8KHZ];
	complex_q31 ctemp[NUM_ACTIVE_SUB];
	int i;

	// Split into subbands
	subband_analysis(line_in, pcq31MIC_fft, pq15Prot_delayMIC, &iProt_delayIndexMIC);
	subband_analysis(line_out, pcq31LS_fft, pq15Prot_delayLS, &iProt_delayIndexLS);

	// Calculate averages of input signal
	averages(AVE_MIC_IN);
	averages(AVE_LS_OUT);

	noise_est(pq31LS_ave, pq31LS_noi_est, pcq31LS_fft, &lsNoiEstSet, true);


	// Acoustic echo cancellation (background filter)
	lec_subband_nlms(pcq31LS_fft, pcq31MIC_fft, pcq31MICE_bg_fft, pcq31AErr_bg_fft);

	lec_subband_fgfilt(pcq31MIC_fft, pcq31MICE_fg_fft, pcq31AErr_fg_fft);

	noise_est(pq31AErr_fg_ave, pq31MIC_noi_est, pcq31AErr_fg_fft, &micNoiEstSet, true);

	// Calculate averages of internal signals
	averages(AVE_INTER);

	// Estimate acoustic feedback and echo return loss enhancement based on the adaptime filters
	lec_check_performance();
	// Update foreground filter?
	lec_compare_filters();

	// Calculate noise reduction dampings
	noise_reduce(pq15AErr_damp_set, pq31AErr_fg_ave, pq31MIC_noi_est, false, false);

	// Detect A-talk (near-end talk)
	resdamp_detect_atalk();

	// Calculate gains for TX
	resdamp_tx_gains_org(pq15AErr_damp_set);
	// Adjust gains for TX
	resdamp_tx_gains_adjust(pq15AErr_damp_set, pq31AErr_fg_ave);
	// Smooth gains for TX
	resdamp_tx_gains_smooth(bBtalk, pq15AErr_damp_set, pq15AErr_damp);

	// Delay signal (to let gains settle)
	resdamp_delay_tx(pcq31AErr_fg_fft, ctemp);
	// Apply subband gains
	resdamp_apply_gains(pq15AErr_damp, ctemp, pcq31TX_fft);

	// Generate and add comfort noise
	noise_gen_cnoi(pcq31TX_fft, pq31MIC_noi_est, pq15AErr_damp);

	// Calculate averages of TX (and CNOI) signal
	averages(AVE_TX_OUT);


	//subband_synthesis( pcq31AErr_fg_fft, q31Temp, pq31Prot_delayTX, &iProt_delayIndexTX, 0);
	//subband_synthesis( pcq31AErr_bg_fft, q31Temp, pq31Prot_delayTX, &iProt_delayIndexTX, 0);
	subband_synthesis( pcq31TX_fft, q31Temp, pq31Prot_delayTX, &iProt_delayIndexTX, 0);

	for(i=0;i<BUFLEN_8KHZ;i++){
		line_in_processed[i]=q31Temp[i];
	}

	dumpdata_post_dump();
}


static void lec_subband_nlms(complex_q31 *x_in, const complex_q31 *y_in, complex_q31 *yhat, complex_q31 *e_out) {
	const int NLMS_DELTA = (40000 >> (13-SUB_SIGNAL_SHIFT)); // 40000 works when SUB_SIGNAL_SHIFT=13
	const int UPDATE_THRESH = (5000 >> (13-SUB_SIGNAL_SHIFT)); // 5000 works when SUB_SIGNAL_SHIFT=13
	unsigned int i;
	int temp1, temp2, temp3, x_re_sq, x_im_sq, xn_re, xn_im;
	complex_q31 c_step;
	bool bA, bB, bC;

	//NLMS_DELTA = shl_q31(40,SUB_SIGNAL_SHIFT);
	//UPDATE_THRESH = shl_q31(4,SUB_SIGNAL_SHIFT);

	// Is module disabled?
	if(!tvModules.bLec) {
		for(i=0; i<NUM_ACTIVE_SUB; i++) {
			e_out[i].re = y_in[i].re;
			e_out[i].im = y_in[i].im;
			yhat[i].re = 0;
			yhat[i].im = 0;
		}
		return;
	}

	// Update delay vectors
	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		// Old sample which is about to be overwritten
		xn_re = pcq31NLMS_delay[i][uiNLMS_delay_index].re;
		xn_im = pcq31NLMS_delay[i][uiNLMS_delay_index].im;


		// Put new sample in delay-vector
		pcq31NLMS_delay[i][uiNLMS_delay_index] = x_in[i];

		temp1 = pq31NLMS_energy[i]; // Read current energy

		// Current sample
		x_re_sq = square_shift14(x_in[i].re);
		x_im_sq = square_shift14(x_in[i].im);
		temp2 = x_re_sq;
		temp2 += x_im_sq;

		// Oldest sample
		x_re_sq = square_shift14(xn_re);
		x_im_sq = square_shift14(xn_im);
		temp3 = x_re_sq;
		temp3 += x_im_sq;

		// Add new and remove old
		temp1 += temp2 - temp3;
		// Store energy
		pq31NLMS_energy[i] = temp1;

	}

	// AEC filtering (generate output errors)
	//memcpy(e_out, y_in, NUM_ACTIVE_SUB*sizeof(complex_q31));
	//aec_filter_asm((complex_q15*)pcq15NLMS_coefs_bg, (complex_q31*)pcq31NLMS_delay, yhat, e_out, uiNLMS_delay_index, NLMS_FILTER_LENGTH, NUM_ACTIVE_SUB, puiAEC_coefs_shift);

	// Generate output errors
	for(i=0;i<DEC_INDEX+1;i++) {
		// X^T(n)H(n-1)
		yhat[i] = cvecdot_conj(pcq15NLMS_coefs_bg[i], pcq31NLMS_delay[i],uiNLMS_delay_index, NLMS_FILTER_LENGTH, NLMS_FILTER_LENGTH);
		// Scale coefficients
		yhat[i].re = shr_q31(yhat[i].re, puiAEC_coefs_shift[i]);
		yhat[i].im = shr_q31(yhat[i].im, puiAEC_coefs_shift[i]);
		// e(n) = y(n) - X^T(n)H(n-1)
		e_out[i].re = sub_q31(y_in[i].re, yhat[i].re);
		e_out[i].im = sub_q31(y_in[i].im, yhat[i].im);
	}

	// Update filters
	for(i=0;i<NUM_ACTIVE_SUB;i++) {
		//lsNoi_sq[i]=square_shift14(shr_q31(pq31LS_noi_est[i], SUB_SIGNAL_SHIFT))*NLMS_FILTER_LENGTH;
		//lsNoi_sq[i]=shl_q31(lsNoi_sq,2);
		//c_step.r = div_q20x31(e_out[i].r, add_q31(add_q31(pq31NLMS_energy[i], pq31MIC_noi_est[i]), NLMS_DELTA));
		//c_step.i = div_q20x31(-e_out[i].i, add_q31(add_q31(pq31NLMS_energy[i], pq31MIC_noi_est[i]), NLMS_DELTA)); // Complex conjugate
		temp1 = MAX(NLMS_DELTA, shl_q31(square_shift14(pq31LS_noi_est[i]), NLMS_FILTER_LENGTH_SHIFT+1));
#ifdef USE_DUMPDATA
		//nlms_delta[i]=temp1; //TODO: removed since I dont know what it is, seems like some sort of debug thing
#endif
		//temp1=0; //TODO: Remove (debug)
		c_step.re = div_q20x31(e_out[i].re, add_q31(pq31NLMS_energy[i], temp1));
		c_step.im = div_q20x31(-e_out[i].im, add_q31(pq31NLMS_energy[i], temp1)); // Complex conjugate

		// Calculate step-size parameter
		// (from J. Benesty, H. Rey, L.R. Vega and S. Tressens, "A Nonparametric VSS NLMS Algorithm",
		// IEEE Signal Processing Letters, vol. 13, no. 10, October 2006.)

		// 1-(Mic_noi*5.5 /(aerr_bg+mic_noi*4) )

		temp1 = add_q31(shl_q31(pq31MIC_noi_est[i], 1), pq31MIC_noi_est[i]);
		temp1 = add_q31(temp1, shr_q31(pq31MIC_noi_est[i], 1));
		temp1 = div_q31x31(temp1, MAX(add_q31(pq31AErr_bg_ave[i], shl_q31(pq31MIC_noi_est[i], 1)), 1));
		temp1 = MAX_32 - temp1;
		temp1 = MAX(temp1, 0);
		temp1 = shl_q31(temp1, 2); // Reduce stepsize
		//temp1 = MAX_32; //TODO remove

#ifdef USE_DUMPDATA
		//pq31Stepsize[i]=temp1; //TODO: removed since I dont know what it is, seems like some sort of debug thing
#endif

		// Adjust the update to the re-scaling of the filter coefficients

#if AEC_COEFS_SHIFT_NEG
		c_step.re = shr_q31(mult_q31x31(c_step.re, temp1), puiAEC_coefs_shift[i]);
		c_step.im = shr_q31(mult_q31x31(c_step.im, temp1), puiAEC_coefs_shift[i]);
#else
		c_step.re = shl_q31(mult_q31x31(c_step.re, temp1), puiAEC_coefs_shift[i]);
		c_step.im = shl_q31(mult_q31x31(c_step.im, temp1), puiAEC_coefs_shift[i]);
#endif
#ifdef USE_DUMPDATA
		temp_step[i] = c_step.re;
#endif

		// Update criterions
		bA = pq31NLMS_energy[i] > UPDATE_THRESH; // Enough energy on LS
		bB = mult_q31x15(pq31LS_hold_ave[i], pq15AFeedback[i])> pq31MIC_ave[i];
		bC = 1;//(pq31MIC_ave[i] > shl_q31_pos(pq31MIC_noi_est[i],1)); // MIC level over noise

		//if(bA){ //TODO remove
		if (bA && bB && bC) {
			bAECUpdate[i]=true;
#ifdef USE_DUMPDATA
			//temp_step[i] = cabs_approx(c_step); //TODO: removed since I dont know what it is, seems like some sort of debug thing
#endif

			// Leaky NLMS
			//aec_leaky(pcq15NLMS_coefs_bg[i], &psNLMS_leaky_index[i]);
			// Update
			cvecvaddsmult(pcq15NLMS_coefs_bg[i], pcq31NLMS_delay[i], c_step, uiNLMS_delay_index);
		}else{
			bAECUpdate[i]=false;
		}

	}

	// Next sample
	uiNLMS_delay_index = circindex(uiNLMS_delay_index, -1, NLMS_FILTER_LENGTH);

}

static void lec_subband_fgfilt(const complex_q31 *y_in, complex_q31 *yhat, complex_q31 *e_out) {
	unsigned int i, k;

	// Is module disabled?
	if (!tvModules.bLec) {
		for (i = 0; i < NUM_ACTIVE_SUB; i++) {
			e_out[i].re = y_in[i].re;
			e_out[i].im = y_in[i].im;
			yhat[i].re = 0;
			yhat[i].im = 0;
		}
		return;
	}

	k = uiNLMS_delay_index;
	k = circindex(k, 1, NLMS_FILTER_LENGTH);

	// AEC filtering (generate output error(s) from foreground filters)
	// Step through all subbands
	for (i = 0; i < DEC_INDEX + 1; i++) {
		// X^T(n)H(n-1)
		yhat[i] = cvecdot_conj(pcq15NLMS_coefs_fg[i], pcq31NLMS_delay[i], k,
				NLMS_FILTER_LENGTH,NLMS_FILTER_LENGTH);
		// Scale coefficients
		yhat[i].re = shr_q31(yhat[i].re, puiAEC_coefs_shift[i]);
		yhat[i].im = shr_q31(yhat[i].im, puiAEC_coefs_shift[i]);
		// e(n) = y(n) - X^T(n)H(n-1)
		e_out[i].re = sub_q31(y_in[i].re, yhat[i].re);
		e_out[i].im = sub_q31(y_in[i].im, yhat[i].im);
	}
}

// Estimates acoustic feedback and echo return loss enhancement
#define AERLE_BG_UPDATE_TRESH 20000 // -4.3dB
static void lec_check_performance() {
	static unsigned int sub_counter = 0; // Subband counter
	unsigned int i;

	// If module not active just skip calculations.
	if(!tvModules.bLec) {
		for(i=0; i<NUM_ACTIVE_SUB; i++) {
			//if we dont use AEC then no reduction of echo.
			pq15AErle_bg[i] = MAX_16;
			pq15AErle_fg[i] = MAX_16;

			//set pq15MaxValue_bg?
			//set pq15LastValue_bg?
			//...
		}
		return;
	}

	// Find largest coefficient and calculate gamma value for all background filters
	find_minmaxgamma(pcq15NLMS_coefs_bg, pq15MaxValue_bg, puiMaxIndex_bg, pq15LastValue_bg, pq31GammaValue_bg, sub_counter);
	// Another subband / coefficient index the next time
	if(++sub_counter >= NUM_ACTIVE_SUB) sub_counter = 0;

	// Calculate Echo Return Loss Enhancement (ERLE) in each subband
	// Step through the subbands
	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		// Enough energy in subband?
		if(mult_q31x15(pq31MIC_ave[i], AERLE_BG_UPDATE_TRESH) > pq31MIC_noi_est[i]) {

			// ERLE is estimated by taking minimum of two different average ratios.
			// This has the effect that AErle_bg is not set too high after a filter copy.
			// This could possibly lead to problems during an echo path change since
			// AErle_bg could be set lower than the true ERLE

			// Background filter
			erle_estimation(&pq15AErle_bg[i], pq31AErr_bg_ave[i], pq31AErr_bg_ave_slow[i], pq31MIC_ave[i], pq31MIC_ave_slow[i], bAtalk);

			// Foreground filter
			erle_estimation(&pq15AErle_fg[i], pq31AErr_fg_ave[i], pq31AErr_fg_ave_slow[i], pq31MIC_ave[i], pq31MIC_ave_slow[i], false);
		}
	}

	calc_erle_fg_compensation();
}

#define DEV_COUNT 128
// Conditions must be true for 20 consecutive samples for FG-filter to update
#define  FILTER_COPY_HOLD_TIME 25
// Conditions must be true for 10 consecutive samples for BG-filter reset
#define FILTER_RESET_HOLD_TIME 20
int piBGtoFG_copy_counter[NUM_ACTIVE_SUB];
int piBG_reset_counter[NUM_ACTIVE_SUB];

// Compares the foreground and background adaptive filters and updates the foreground filter if needed
static void lec_compare_filters() {
	static unsigned int sub_counter = 0; // Subband counter
	unsigned int i;
	bool bA,bB,bC,bD,bE,bF;

	// If module not active just skip calculations.
	if(!tvModules.bLec) return;

	// Accumulate output error and filter divergence
	calculate_filter_deviation();

	// Initializaton
	bCopyFilter[sub_counter] = false;
	bResetFilter[sub_counter] = false;
	// Conditions good for foreground filter updating?
	for(i=0; i<NUM_ACTIVE_SUB; i++) {
		bA = !bAtalk;
		bB = pq31Filter_div_fg[i] >= pq31Filter_div_bg[i];
		bC = add_q31(pq31AErr_bg_ave[i],shr_q31(pq31AErr_bg_ave[i],2)) < pq31AErr_fg_ave[i]; //atleast 1dB better
		bD = add_q31(pq31AErr_bg_ave[i],shr_q31(pq31AErr_bg_ave[i],2)) < pq31MIC_ave[i];	//atleast 1dB better
		bE = pq31MIC_ave[i] > shl_q31(pq31MIC_noi_est[i],2);
		bF = pq31LS_hold_ave[i] > shl_q31(pq31LS_noi_est[i], 2);

		// All conditions satisfied?
		if(bA && bB && bC && bD && bE && bF) {

			// Count up to FILTER_COPY_HOLD_TIME
			piBGtoFG_copy_counter[i] = MIN(piBGtoFG_copy_counter[i]+1, FILTER_COPY_HOLD_TIME);

		} else if(bA && bE && bF) {

			// The "important" conditions not satisfied -> reset counter

			// Set counter to 0 (or lower)
			piBGtoFG_copy_counter[i] = MIN(piBGtoFG_copy_counter[i], 0);
		}
	}

	if(piBGtoFG_copy_counter[sub_counter] >= FILTER_COPY_HOLD_TIME){
		bCopyFilter[sub_counter] = true;
	}

	// Copy background filter to foreground?
	if(bCopyFilter[sub_counter]) {//&&add_q31(pq31AErr_bg_ave[i],shr_q31(pq31AErr_bg_ave[i],1)) < pq31AErr_fg_ave[i]) {

		// Reset divergence measure at startup
		if(pq15AErle_fg[sub_counter] == MAX_16) {
			pcq31EYhat_fg_ave[sub_counter].re = pcq31EYhat_bg_ave[sub_counter].re;
			pcq31EYhat_fg_ave[sub_counter].im = pcq31EYhat_bg_ave[sub_counter].im;
			pcq31YYhat_fg_ave[sub_counter].re = pcq31YYhat_bg_ave[sub_counter].re;
			pcq31YYhat_fg_ave[sub_counter].im = pcq31YYhat_bg_ave[sub_counter].im;
		}

		// Copy filter and associated variables
		filter_copy_bg2fg(sub_counter);
		piBGtoFG_copy_counter[sub_counter]=0;

	}

	// Conditions for reseting BG filter
	for(i=0; i<NUM_ACTIVE_SUB; i++) {

		// Conditions that indicate a misadjusted BG filter
		bA = shr_q31(pq31AErr_bg_ave[i], 2) > add_q31(pq31AErr_fg_ave[i], 8);
		bB = shr_q31(pq31AErr_bg_ave[i], 2) > add_q31(pq31MIC_ave[i], 8);

		// true? We might need a reset..
		if(bA || bB) {
			// Count up to HOLD_TIME
			piBG_reset_counter[i] = MIN(piBG_reset_counter[i]+1, FILTER_RESET_HOLD_TIME);

		} else {
			piBG_reset_counter[i] = 0;
		}
	}

	if(piBG_reset_counter[sub_counter] >= FILTER_RESET_HOLD_TIME) {
		bResetFilter[sub_counter] = true;
	}
	if(bResetFilter[sub_counter]) {
		reset_bg_filter(sub_counter);
		piBG_reset_counter[sub_counter] = 0;
	}

	// Another subband the next time
	if(++sub_counter >= NUM_ACTIVE_SUB) sub_counter = 0;
}

static void calc_erle_fg_compensation() {
	const q31 EPSILON = 1;
	const q15 gamma = (q15)(0.98*MAX_16);
	q31 temp0, temp1, temp2;
	unsigned int i;

	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		// Remove influence of noise level so that the ratio goes to zero instead of one during single near end talk
		temp2 = shl_q31(pq31MIC_noi_est[i], 4); // Multiply by 16
		temp1 = MAX(pq31MIC_noi_est[i], mult_q31x15(pq31LS_noi_est[i], pq15AFeedback[i]));
		temp0 = mult_q31x31(temp2, shl_q31(temp1, 8)); // Scale up to correspond to pcq31EYhat_fg_ave[] scaling

		temp1 = cabs_approx(&pcq31EYhat_fg_ave[i]);
		temp1 = MAX(sub_q31(temp1, temp0), 0);
		temp2 = pcq31YhatYhat_fg_ave[i];

		temp2 = temp1 / add_q31(shr_q31(add_q31(temp2, temp0), 15), EPSILON);
		temp2 = MIN(temp2, MAX_16); // Saturate
		pq15AErle_fg_compensation[i] = lec_smoothing_q15(temp2, pq15AErle_fg_compensation[i], gamma, 0);
	}
}

// Calculates aec filter deviation measures
static void calculate_filter_deviation() {
	unsigned int i;
	q31 temp1, temp2;

	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		// Filter deviation measures. This should be shifted in some fashion since
		// denominator is sometimes smaller than numerator

		temp1 = cabs_approx(&pcq31EYhat_bg_ave[i]);
		temp2 = cabs_approx(&pcq31YYhat_bg_ave[i]);
		pq31Filter_div_bg[i] = div_q31x31(add_q31(temp1, 1), add_q31(temp2, 1));
		temp1 = cabs_approx(&pcq31EYhat_fg_ave[i]);
		temp2 = cabs_approx(&pcq31YYhat_fg_ave[i]);
		pq31Filter_div_fg[i] = div_q31x31(add_q31(temp1, 1), add_q31(temp2, 1));
	}
}

// Copies bg filter to fg in one subband. Both filter coefficients and performance variables are copied.
static void filter_copy_bg2fg(unsigned int s) {
	complex_q15 *bg, *fg;

	// NLMS-filter
	bg = pcq15NLMS_coefs_bg[s];
	fg = pcq15NLMS_coefs_fg[s];

	// Copy the updating NLMS filter
	memcpy(fg, bg, sizeof(complex_q15)*(NLMS_FILTER_LENGTH));

	// Copy performance variables
	puiMaxIndex_fg[s] = puiMaxIndex_bg[s];
	pq15LastValue_fg[s] = pq15LastValue_bg[s];
	pq15LastValue_bg[s] = 0;
	// Calculate gamma
	pq31GammaValue_fg[s] = pq31GammaValue_bg[s];
	// Update ERLE
	// Let fg rise if bg is worse, (a separate estimation on fg lets fg fall)
	pq15AErle_fg[s] = MAX(pq15AErle_bg[s], pq15AErle_fg[s]);
	//pq15AErle_fg[s] = pq15AErle_bg[s];
	pq15AErle_bg[s] = MAX_16; // Reset background filter ERLE
}

// Resets bg-filter using fg-filter coefficients and performance
static void reset_bg_filter(unsigned int s) {
	// Reset coefficients
	memcpy(pcq15NLMS_coefs_bg[s], pcq15NLMS_coefs_fg[s], sizeof(complex_q15)*(NLMS_FILTER_LENGTH));

	// Variables associated with the filter
	pq15LastValue_bg[s] = pq15LastValue_fg[s];
	puiMaxIndex_bg[s] = puiMaxIndex_fg[s];
	pq31GammaValue_bg[s] = pq31GammaValue_fg[s];
	pq15AErle_bg[s] = pq15AErle_fg[s];

	// Averages
	pcq31EYhat_bg_ave[s].re = pcq31EYhat_fg_ave[s].re;
	pcq31EYhat_bg_ave[s].im = pcq31EYhat_fg_ave[s].im;
	pcq31YYhat_bg_ave[s].re = pcq31YYhat_fg_ave[s].re;
	pcq31YYhat_bg_ave[s].im = pcq31YYhat_fg_ave[s].im;
	pq31AErr_bg_ave[s] = pq31AErr_fg_ave[s];
}

/*****************************************************************************/
/*
 *	\brief	Signal averages
 *	\input	mode	- Decides what signals that should be averaged
 *	\output	None
 *	\date	2010-09-23
 *	\author	Markus Borgh, Magnus Berggren, Christian Sch\FCldt
 */
static void averages(enum AVE_TYPES mode) {
	const q31 gamma = 0x74000000; // 0.9063
	const q31 gamma2 = 0x7f5c28f4; // 0.9850 (For filter deviation (slow))
//	const q31 gamma3 = 0x7eb851ea; // 0.9800 (For doubletalk detection (fast))
	const q31 gamma5 = 0x7fc00000; // 0.9980 (For estimating ERLE)
	static unsigned int hold_counter = 0;
	unsigned int i;
	complex_q31 ctemp, aerr_bg, aerr_fg, mice_bg, mice_fg, mic;
	q31 temp;

	switch(mode) {

	case AVE_RX_IN:

		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			temp = cabs_approx(&pcq31RX_fft[i]);
			pq31RX_ave[i] = lec_smoothing(temp, pq31RX_ave[i], gamma);
		}
		break;

	case AVE_MIC_IN:

		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			temp = cabs_approx(&pcq31MIC_fft[i]);
			pq31MIC_ave[i] = lec_smoothing(temp, pq31MIC_ave[i], gamma);
			pq31MIC_ave_slow[i] = lec_smoothing2(temp, pq31MIC_ave_slow[i], gamma, gamma5); // For ERLE estimation
		}
		break;

	case AVE_LS_OUT:

		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			temp = cabs_approx(&pcq31LS_fft[i]);
			pq31LS_ave[i] = lec_smoothing(temp, pq31LS_ave[i], gamma);

			// Estimated echo tail (Uncancelled echo due to insufficient AEC-filter length)
			pq31LS_tail_ave[i] = lec_smoothing2(pq31LS_tail_abs[i], pq31LS_tail_ave[i], 0, pq31GammaValue_fg[i]);
		}

		hold_counter++;
		for(i=0;i<NUM_ACTIVE_SUB;i++) {

			pq31LS_hold1[i] = MAX(pq31LS_hold1[i], pq31LS_ave[i]);
			pq31LS_hold2[i] = MAX(pq31LS_hold2[i], pq31LS_ave[i]);

			if(hold_counter >= LS_AVE_HOLD_TIME) {
				pq31LS_hold1[i] = pq31LS_hold2[i];
				pq31LS_hold2[i] = 0;
			}

			// Smooth with instant rise
			pq31LS_hold_ave[i] = lec_smoothing2(pq31LS_hold1[i], pq31LS_hold_ave[i], 0, gamma);
		}
		if(hold_counter >= LS_AVE_HOLD_TIME) hold_counter = 0;
		break;

	case AVE_INTER:

		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			temp = cabs_approx(&pcq31MICE_bg_fft[i]);
			pq31MIChat_bg_ave[i] = lec_smoothing(temp, pq31MIChat_bg_ave[i], gamma);

			temp = cabs_approx(&pcq31MICE_fg_fft[i]);
			pq31MIChat_fg_ave[i] = lec_smoothing(temp, pq31MIChat_fg_ave[i], gamma);

			temp = cabs_approx(&pcq31AErr_bg_fft[i]);
			pq31AErr_bg_ave[i] = lec_smoothing(temp, pq31AErr_bg_ave[i], gamma);
			pq31AErr_bg_ave_slow[i] = lec_smoothing2(temp, pq31AErr_bg_ave_slow[i], gamma, gamma5); // For ERLE estimation

			temp = cabs_approx(&pcq31AErr_fg_fft[i]);
			pq31AErr_fg_ave[i] = lec_smoothing(temp, pq31AErr_fg_ave[i], gamma);
			pq31AErr_fg_ave_slow[i] = lec_smoothing2(temp, pq31AErr_fg_ave_slow[i], gamma, gamma5); // For ERLE estimation

			temp = cabs_approx(&pcq31NLMS_delay[i][uiNLMS_delay_index]);
			temp = mult_q31x15(temp, pq15LastValue_fg[i]);
			pq31LS_tail_abs[i] = temp;
		}

		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			/*
			// Increase precision
			mic.r = shl_q31(pcq31MIC_fft[i].r, 10-SUB_SIGNAL_SHIFT);
			mic.i = shl_q31(pcq31MIC_fft[i].i, 10-SUB_SIGNAL_SHIFT);
			mice_bg.r = shl_q31(pcq31MICE_bg_fft[i].r, 10-SUB_SIGNAL_SHIFT);
			mice_bg.i = shl_q31(pcq31MICE_bg_fft[i].i, 10-SUB_SIGNAL_SHIFT);
			mice_fg.r = shl_q31(pcq31MICE_fg_fft[i].r, 10-SUB_SIGNAL_SHIFT);
			mice_fg.i = shl_q31(pcq31MICE_fg_fft[i].i, 10-SUB_SIGNAL_SHIFT);
			aerr_bg.r = shl_q31(pcq31AErr_bg_fft[i].r, 10-SUB_SIGNAL_SHIFT);
			aerr_bg.i = shl_q31(pcq31AErr_bg_fft[i].i, 10-SUB_SIGNAL_SHIFT);
			aerr_fg.r = shl_q31(pcq31AErr_fg_fft[i].r, 10-SUB_SIGNAL_SHIFT);
			aerr_fg.i = shl_q31(pcq31AErr_fg_fft[i].i, 10-SUB_SIGNAL_SHIFT);

			// *** For filter deviation comparison

			// Aerr * MIChat' - Complex multiplication. Note the complex conjugate!
			ctemp.r = add_q31(mult_q31x31(aerr_bg.r, mice_bg.r), mult_q31x31(aerr_bg.i, mice_bg.i));
			ctemp.i = sub_q31(mult_q31x31(aerr_bg.i, mice_bg.r), mult_q31x31(aerr_bg.r, mice_bg.i));
			pcq31EYhat_bg_ave[i].r = smoothing(ctemp.r, pcq31EYhat_bg_ave[i].r, gamma2, 0);
			pcq31EYhat_bg_ave[i].i = smoothing(ctemp.i, pcq31EYhat_bg_ave[i].i, gamma2, 0);
			ctemp.r = add_q31(mult_q31x31(aerr_fg.r, mice_fg.r), mult_q31x31(aerr_fg.i, mice_fg.i));
			ctemp.i = sub_q31(mult_q31x31(aerr_fg.i, mice_fg.r), mult_q31x31(aerr_fg.r, mice_fg.i));
			pcq31EYhat_fg_ave[i].r = smoothing(ctemp.r, pcq31EYhat_fg_ave[i].r, gamma2, 0);
			pcq31EYhat_fg_ave[i].i = smoothing(ctemp.i, pcq31EYhat_fg_ave[i].i, gamma2, 0);
			// MIC * MIChat' - Complex multiplication. Note the complex conjugate!
			ctemp.r = add_q31(mult_q31x31(mic.r, mice_bg.r), mult_q31x31(mic.i, mice_bg.i));
			ctemp.i = sub_q31(mult_q31x31(mic.i, mice_bg.r), mult_q31x31(mic.r, mice_bg.i));
			pcq31YYhat_bg_ave[i].r = smoothing(ctemp.r, pcq31YYhat_bg_ave[i].r, gamma2, 0);
			pcq31YYhat_bg_ave[i].i = smoothing(ctemp.i, pcq31YYhat_bg_ave[i].i, gamma2, 0);
			ctemp.r = add_q31(mult_q31x31(mic.r, mice_fg.r), mult_q31x31(mic.i, mice_fg.i));
			ctemp.i = sub_q31(mult_q31x31(mic.i, mice_fg.r), mult_q31x31(mic.r, mice_fg.i));
			pcq31YYhat_fg_ave[i].r = smoothing(ctemp.r, pcq31YYhat_fg_ave[i].r, gamma2, 0);
			pcq31YYhat_fg_ave[i].i = smoothing(ctemp.i, pcq31YYhat_fg_ave[i].i, gamma2, 0);
			*/

			mic.re = pcq31MIC_fft[i].re;
			mic.im = pcq31MIC_fft[i].im;
			mice_bg.re = pcq31MICE_bg_fft[i].re;
			mice_bg.im = pcq31MICE_bg_fft[i].im;
			mice_fg.re = pcq31MICE_fg_fft[i].re;
			mice_fg.im = pcq31MICE_fg_fft[i].im;
			aerr_bg.re = pcq31AErr_bg_fft[i].re;
			aerr_bg.im = pcq31AErr_bg_fft[i].im;
			aerr_fg.re = pcq31AErr_fg_fft[i].re;
			aerr_fg.im = pcq31AErr_fg_fft[i].im;
			// Aerr * MIChat' - Complex multiplication. Note the complex conjugate!
			ctemp.re = add_q31(mult_q31x31(aerr_bg.re, mice_bg.re), mult_q31x31(aerr_bg.im, mice_bg.im));
			ctemp.im = sub_q31(mult_q31x31(aerr_bg.im, mice_bg.re), mult_q31x31(aerr_bg.re, mice_bg.im));
			pcq31EYhat_bg_ave[i].re = lec_smoothing_shift(ctemp.re, pcq31EYhat_bg_ave[i].re, gamma2, 10);
			pcq31EYhat_bg_ave[i].im = lec_smoothing_shift(ctemp.im, pcq31EYhat_bg_ave[i].im, gamma2, 10);
			ctemp.re = add_q31(mult_q31x31(aerr_fg.re, mice_fg.re), mult_q31x31(aerr_fg.im, mice_fg.im));
			ctemp.im = sub_q31(mult_q31x31(aerr_fg.im, mice_fg.re), mult_q31x31(aerr_fg.re, mice_fg.im));
			pcq31EYhat_fg_ave[i].re = lec_smoothing_shift(ctemp.re, pcq31EYhat_fg_ave[i].re, gamma2, 10);
			pcq31EYhat_fg_ave[i].im = lec_smoothing_shift(ctemp.im, pcq31EYhat_fg_ave[i].im, gamma2, 10);
			// MIC * MIChat' - Complex multiplication. Note the complex conjugate!
			ctemp.re = add_q31(mult_q31x31(mic.re, mice_bg.re), mult_q31x31(mic.im, mice_bg.im));
			ctemp.im = sub_q31(mult_q31x31(mic.im, mice_bg.re), mult_q31x31(mic.re, mice_bg.im));
			pcq31YYhat_bg_ave[i].re = lec_smoothing_shift(ctemp.re, pcq31YYhat_bg_ave[i].re, gamma2, 10);
			pcq31YYhat_bg_ave[i].im = lec_smoothing_shift(ctemp.im, pcq31YYhat_bg_ave[i].im, gamma2, 10);
			ctemp.re = add_q31(mult_q31x31(mic.re, mice_fg.re), mult_q31x31(mic.im, mice_fg.im));
			ctemp.im = sub_q31(mult_q31x31(mic.im, mice_fg.re), mult_q31x31(mic.re, mice_fg.im));
			pcq31YYhat_fg_ave[i].re = lec_smoothing_shift(ctemp.re, pcq31YYhat_fg_ave[i].re, gamma2, 10);
			pcq31YYhat_fg_ave[i].im = lec_smoothing_shift(ctemp.im, pcq31YYhat_fg_ave[i].im, gamma2, 10);

			ctemp.re = add_q31(mult_q31x31(mice_fg.re, mice_fg.re), mult_q31x31(mice_fg.im, mice_fg.im));
			pcq31YhatYhat_fg_ave[i] = lec_smoothing_shift(ctemp.re, pcq31YhatYhat_fg_ave[i], gamma2, 10);
		}
		break;

	case AVE_TX_OUT:
		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			temp = cabs_approx(&pcq31CNoi_fft[i]);
			pq31CNoi_ave[i] = lec_smoothing(temp, pq31CNoi_ave[i], gamma5);
			temp = cabs_approx(&pcq31TX_fft[i]);
			pq31TX_ave[i] = lec_smoothing(temp,pq31TX_ave[i], gamma);
		}
		break;
	}
}

/*****************************************************************************/
/*
 *	\brief	Initialization of signal averages
 *	\input	None
 *	\output	None
 *	\date	2012-Jul-13
 */
static void averages_init() {
	unsigned int i;

	for(i=0;i<NUM_ACTIVE_SUB;i++) {
		pq31RX_ave[i] = shl_q31(10000, SUB_SIGNAL_SHIFT); // Higher value than 0 to keep noi_est OK
		pq31LS_ave[i] = 0;
		pq31MIC_ave[i] = shl_q31(10000, SUB_SIGNAL_SHIFT); // Higher value than 0 to keep noi_est OK
		pq31TX_ave[i] = 0;

		pq31AErr_bg_ave[i] = pq31MIC_ave[i]; // This removes risk of wrong AErle at startup
		pq31AErr_fg_ave[i] = pq31MIC_ave[i]; // This removes risk of wrong AErle at startup

		pq31MIC_ave_slow[i] = pq31MIC_ave[i];
		pq31AErr_bg_ave_slow[i] = pq31MIC_ave[i];
		pq31AErr_fg_ave_slow[i] = pq31MIC_ave[i];

		pq31LS_hold_ave[i] = 0;
		pq31LS_tail_ave[i] = pq31LS_tail_abs[i];

		pcq31EYhat_bg_ave[i].re = 0;
		pcq31EYhat_bg_ave[i].im = 0;
		pcq31EYhat_fg_ave[i].re = 0;
		pcq31EYhat_fg_ave[i].im = 0;
		pcq31YYhat_bg_ave[i].re = 0;
		pcq31YYhat_bg_ave[i].im = 0;
		pcq31YYhat_fg_ave[i].re = 0;
		pcq31YYhat_fg_ave[i].im = 0;
	}
}

/*****************************************************************************/
/*
 *	\brief	Noise level estimation for unspecified audio channel
 *	\input	*pq31ave		- Array with average signal level for each subband
 *			*pq31noi_est	- Array with noise level estimation for each subband
 *			*pcq31fft		- Array with subband signal
 *			*set			- Settings for noise level estimation for an audio
 *							  channel (mic, rx)
 *	\output	New noise level estimation is returned in pq31noi_est
 */
static void noise_est(const q31 *__restrict pq31ave, q31 *__restrict pq31noi_est, const complex_q31 *__restrict pcq31fft, noiEstSet_t *__restrict set, bool estimate_noise) {
	const unsigned int RUN_LENGTH = 32;
	const int ACC_L_SHIFT = 16; // Shift to be able to accumulate weak audio signal
	unsigned int i;
	q31 temp31, temp31_2;
	q31 *p = set->pq31block;
	q31 *pmax1 = set->pq31max1;
	q31 *pmax2 = set->pq31max2;
	q31 *pmin1 = set->pq31min1;
	q31 *pmin2 = set->pq31min2;

	// Accumulate short block(s) (loudspeaker)
	for(i=0;i<NUM_ACTIVE_SUB;i++) {
		temp31 = shl_q31(cabs_approx(&pcq31fft[i]), (ACC_L_SHIFT-SUB_SIGNAL_SHIFT));
		p[i] = add_q31(temp31, p[i]);
	}

	if(NUM_ACK == ++set->j) {
		// Full block is accumulated
		set->bNoiEstAct = true;
		// Restart block-accumulation
		set->j = 0;
		set->bInstantFall = true;
	}

	if(set->bNoiEstAct) {

		if(++set->k == RUN_LENGTH/2) {

			// Reset one set of the running max / min variables
			memset(pmax2, 0, sizeof(set->pq31max2));
			memset(pmin2, MAX_8, sizeof(set->pq31min2));

		} else if(set->k >= RUN_LENGTH) {

			// Reset the other set of the running max / min variables
			memset(pmax1, 0, sizeof(set->pq31max1));
			memset(pmin1, MAX_8, sizeof(set->pq31min1));

			// Reset counter
			set->k = 0;
		}

		// Update the min/max variables
		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			temp31 = p[i];
			pmax1[i] = MAX(pmax1[i], temp31);
			pmax2[i] = MAX(pmax2[i], temp31);
			pmin1[i] = MIN(pmin1[i], temp31);
			pmin2[i] = MIN(pmin2[i], temp31);
		}

		set->bEstimateNoise = estimate_noise;
		// Signal sufficiently stationary in all subbands? (except the lowest)
		check_stationary(pmax1, pmax2, pmin1, pmin2, &set->bEstimateNoise);
		// Reset blocks
		memset(p, 0, sizeof(set->pq31block));

		set->bNoiEstAct = false;
	}

	// Estimate noise?
	if(set->bEstimateNoise) {
		// If only noise present -> "slow" rise
		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			// Slow increase of noise estimation
			temp31 = MAX(mult_q31x31(set->beta, pq31noi_est[i]), 1); //always allowed to add one.
			temp31 = add_q31(temp31, pq31noi_est[i]);
			// Avoid getting stuck at 0
			temp31 = MAX(temp31, 50000); //minimum noise level

			// Adaption of noise level towards signal average
			temp31_2 = lec_smoothing(pq31ave[i], pq31noi_est[i], set->gamma);
			// Correction with last accumulated signal block to reach combined amplitude of 1 from GAMMA+GAMMA_INV
			//temp31_2 = add_q31(temp31_2, mult_q31x31(set->GAMMA_INV, set->ppq15blocks[i][set->k]));

			// Noise estimation is minimum of the two methods
			pq31noi_est[i] = MIN(temp31, temp31_2);
		}
	}
	if(set->bInstantFall) {
		for(i=0;i<NUM_ACTIVE_SUB;i++) {
			// If the the lowest sound is lower than the previous value of noi_est_*** -> update with new value.
			// This is used to reduce effects of erroneus estimated noise level during tones, even when bEstimate_noise==false!!
			pq31noi_est[i] = MIN(pq31noi_est[i], lec_smoothing(pq31ave[i], pq31noi_est[i], set->gamma)); // Loudspeaker, fall
		}
	}
}

// Generate comfort noise using LRS (Linear Recursive Sequences)
static void noise_gen_cnoi(complex_q31 *in, q31 *noi_est, q15 *gains) {
	const int cpoly = 0x100B; // Primitive polynomial: x^16 + x^12 + x^3 + x + 1
	static unsigned int cstart = 0x0001; // Could be anything other than 0,
										 // since the sequence will be maximum length anyway.
	int cnew, cnoi, bits;
	q15 temp[DEC_INDEX]; // Temporary input buffer
	q31 temp32;
	unsigned int i;
	static unsigned int startupCounter = 0;

	// Generate new fullband samples of "white" noise
	for(i=0;i<DEC_INDEX;i++) {
		// Generate a new noise sample
		cnew = cstart&cpoly;
		// Count bits
		bits = countbits(cnew); // Total number of 1-bits
		// Random value (-16383 or 16384)
		cnoi = 8192 - ((cstart&1)*16384);
		cstart = cstart>>1; // Shift
		cstart |= (bits&1)<<15; // Add new bit
		// Store sample in buffer
		temp[i] = (q15)cnoi;
	}

	// Subband splitting
	subband_analysis(temp, pcq31CNoi_fft, pq15Prot_delayCNOI, &iProt_delayIndexCNOI);

	// Do not apply comfort noise at startup, let noise estimation adapt first
	// ... also, pq31CNoi_ave has got to adapt before adding comfort noise so pcq31CNoi_fft must be created
	if(++startupCounter < 250) return;
	else --startupCounter; // Prevent overflow

	// Add comfort noise in each subband
	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		// Only comfort noise if noise estimation has begun.
		if(noi_est[i] != NOI_EST_INIT_VALUE) {

			// Comfort noise in level with real noise
			temp32 = noi_est[i];
			// Adjust level since we have estimated noise level wrong
			temp32 = add_q31(temp32, shr_q31(temp32, 1)); // +3dB

			// Adjust according to actual level of generated noise
			temp32 = div_q31x31(temp32, pq31CNoi_ave[i]);
			//temp32 = min_q31(temp32, comf_noi_max[i]);
			// Inverse damping
			if(false) {
				temp32 = mult_q31x15(temp32, MAX((32767>>NOI_DAMP_SHIFT_TX) - gains[i], 0));
			} else {
				temp32 = mult_q31x15(temp32, MAX(32767 - gains[i], 0));
			}

			// Apply damping and add comfort noise to output signal
			in[i].re += mult_q31x31(pcq31CNoi_fft[i].re, temp32);
			in[i].im += mult_q31x31(pcq31CNoi_fft[i].im, temp32);
		}
	}
}

static void resdamp_init() {
	bBtalk = false;
	bBtalk_instant = false;
	bBtalkStrong = false;

	// Clear delay buffer
	memset(ppcq31TX_delay, 0, sizeof(ppcq31TX_delay));
}

// Detects near end talk (A-talk)
static void resdamp_detect_atalk() {
	const q31 ECHO_EST_GAMMA = (q31)(0.9*MAX_32);
	const q15 DIST_MAX_LVL = (q15)(0.0625*MAX_16); //-24dB
	const int ATALK_HOLD = 32;//128;
	const int ATALK_NRSUBBANDS_THRESH = 2; //4;
	q31 temp32, echo_est1, echo_est2;
	q15 temp16;
	unsigned int i;
	static int atalk_hold_count = 0;
	int nr_atalk_subbands = 0;


	if(atalk_hold_count == 0)
		bAtalk = false;

	q31DistEcho=0;
	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		temp32 = mult_q31x31(pq31MIChat_fg_ave[i], mult_q31x31(pq31LS_hold_ave[i], pq31LS_hold_ave[i]));
		//temp2=pLS_hold_ave[i]; //assumes that ls_hold_ave is 1 when the maximum distortion arises.
		q31DistEcho = MAX(mult_q31x15(shl_q31(temp32, 1), DIST_MAX_LVL), q31DistEcho);
	}

	// Calculate q31echoEst[]
	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		temp32 = sub_q31(pq31LS_hold_ave[i], shr_q31(pq31LS_noi_est[i], 1));
		temp16 = shl_q15_pos(pq15AErle_fg[i], 2); // Assume 12 dB worse
		// AErle compensation during EPC
		temp16 = MAX(temp16, pq15AErle_fg_compensation[i]);

		// More distortion in lower bands (and very high bands)

		// Acoustic coupling
		echo_est1 = mult_q31(pq15AFeedback[i], temp16);
		// Estimate based on the LS signal
		echo_est1 = mult_q31x31(echo_est1, temp32);
		// EStimate based on the MIChat signal
		echo_est2 = mult_q31x15(pq31MIChat_fg_ave[i], temp16);

		// Weight the two echo estimates using AErle_fg (the better AErle the more we use echoEst2)
		temp32 = add_q31(mult_q31x15(echo_est1, pq15AErle_fg[i]), mult_q31x15(echo_est2, MAX_16-pq15AErle_fg[i]));

		// Uncancelled echo (due to insufficient AEC-filter length)
		temp32 = MAX(temp32, pq31LS_tail_ave[i]);
		// Background noise level (otherwise the echo estimation can be below the noise level)
		//temp32 = MAX(temp32, pq31MIC_noi_est[i]);
		temp32 = add_q31(temp32, shr_q31(pq31MIC_noi_est[i], 1));
#ifdef USE_DUMPDATA
		//TODO: removed since I dont know what it is, seems like some sort of debug thing
		//pq31echoEst_dump[i] = temp32;
		//pq31echoEst2_dump[i] = echo_est2;
#endif
		// Add dist echo
		temp32 = add_q31(temp32, q31DistEcho);

		// Slow fall of echo estimation
		pq31echoEst[i] = MAX(lec_smoothing(temp32, pq31echoEst[i], ECHO_EST_GAMMA), temp32);
	}
	/********** atalk detection*********************/
	for(i=2;i<NUM_ACTIVE_SUB;i++) {

		// Uncancelled echo (due to distortion)
		temp32 = add_q31(shr_q31(q31DistEcho, MAX((i+1)>>3, 0)), pq31echoEst[i]); //lower the expeted distortion by 6dB every 8th subband.
		temp32 = add_q31(shl_q31(pq31MIC_noi_est[i],1), temp32);

		if(MIN(pq31AErr_bg_ave[i],pq31AErr_fg_ave[i]) > temp32)
			nr_atalk_subbands++;
	}
	atalk_hold_count = MAX(atalk_hold_count-1, 0); // Count down to 0

	// Enough bands saying doubletalk?
	if(nr_atalk_subbands > ATALK_NRSUBBANDS_THRESH) {
		atalk_hold_count = ATALK_HOLD;
		bAtalk = true;
	}
}

// Calculates subband gain to suppress residual acoustic echo
static void resdamp_tx_gains_org(q15 gain_set[]) {
	unsigned int i;
	q31 temp32, speechandecho_est;
	q15 temp16;

	// Step through subbands
	for(i=0;i<NUM_ACTIVE_SUB;i++) {

		// Minimum of AErr_bg, AErr_fg and MIC (to obtain the "best" estimate of speech + echo (-echo))
		temp32 = MIN(pq31AErr_fg_ave[i], pq31AErr_bg_ave[i]);
		temp32 = MIN(temp32, pq31MIC_ave[i]);

		pq31SpeechEst[i] = MAX(temp32 - pq31echoEst[i], 0);
		//pq31SpeechEst[i] = MIN(pq31SpeechEst[i], MAX(pq31MIC_ave[i] - pq31MIChat_fg_ave[i], 0));
		//pq31SpeechEst[i] = add_q31(pq31SpeechEst[i], shr_q31(pq31SpeechEst[i], 1));
		speechandecho_est = pq31AErr_fg_ave[i];

		// Residual echo suppression and turbo-duplex
		// "Wiener-type" gain (gain = speech / (speech + residual_echo))
		temp16 = q31_to_q15(div_q31x31(pq31SpeechEst[i], speechandecho_est));
		gain_set[i] = MIN(temp16, gain_set[i]);
	}
}

// Delay signals
static void resdamp_delay_tx(complex_q31 *in, complex_q31 *out) {
	static short counter = 0;
	unsigned int i;

	// Store input in circular buffer
	for(i=0;i<NUM_ACTIVE_SUB;i++) {
		ppcq31TX_delay[i][counter] = in[i];
	}
	// Circular buffering
	counter = circindex(counter, 1, TX_DELAY);
	// Get delayed input
	for(i=0;i<NUM_ACTIVE_SUB;i++) {
		out[i] = ppcq31TX_delay[i][counter];
	}
}

#else // USE_LEC

void internal_lec(q15* line_in, q15* line_out, q15* line_in_processed) {};
void internal_lec_init(bool reset_all) {};

#endif // USE_LEC
