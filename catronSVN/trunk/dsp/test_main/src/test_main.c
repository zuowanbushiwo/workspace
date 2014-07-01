/*****************************************************************************
 *	\file		test_main.c
 *
 *	\date		2013-May-15
 *
 *	\brief		functions for testing of LUF
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
//#include "dsplib.h"
//#include "true_voice.h"
#include "test_main.h"

#include <string.h>								 // memset
#include <math.h>								 // sin()
#include "../../true_voice/include/plot.h"		 // Should not be included here but used for testing of mixer in truevoice
#include "../../true_voice/include/fract_math.h" // Should not be included here but used for testing of limiter
#include "../../true_voice/include/subband.h" 	 // Should not be included here but used for testing of subband functions
#include "../../true_voice/include/timers.h"	 // Should not be included here but used for testing of mute.
#include "bufferedFileReader.h" 				 // for creating debug data
#include <stdint.h>
#include "../../true_voice/include/dumpdata.h"
#include <time.h> 							   //This should be removed

//Includes needed for Line in/Out
#include "stdio.h"
#define TYPES_H //Don't wanna include types.h
#include "types.h"
#include "evmomapl138.h"
#include "evmomapl138_timer.h"
#include "evmomapl138_gpio.h"
#include "evmomapl138_i2c.h"

#include "stdio.h"
#include "types.h"
#include "evmomapl138.h"
#include "evmomapl138_mcasp.h"
#include "evmomapl138_aic3106.h"
#include "test_audio.h"



#include "test_audio.h"

#pragma  DATA_SECTION (maleSpeech, "extvar")
#pragma  DATA_ALIGN(maleSpeech, 64)
short maleSpeech[11311]={
//#include"../../true_voice/include/whiteNoise.inc"
#include "../../true_voice/include/malespeech8000.inc" // Should not be included here but used for testing of mixer
};

#pragma  DATA_SECTION (femaleSpeech, "extvar")
#pragma  DATA_ALIGN(femaleSpeech, 64)
short femaleSpeech[12700]={
#include "../../true_voice/include/femalespeech8000.inc" // Should not be included here but used for testing of mixer
};

#pragma  DATA_SECTION (speech_and_noise_headset, "extvar")
#pragma  DATA_ALIGN(speech_and_noise_headset, 64)
short speech_and_noise_headset[46004]={
#include "../../../matlab/PESQ_simulation/speech_and_noise_headset_8kHz.inc" // used for creating PESQ simulation files
};

#pragma  DATA_SECTION (speech_and_noise_ref, "extvar")
#pragma  DATA_ALIGN(speech_and_noise_ref, 64)
short speech_and_noise_ref[46004]={
#include "../../../matlab/PESQ_simulation/speech_and_noise_ref_8kHz.inc" // used for creating PESQ simulation files
};

// ---< Defines >---
#define RECORD_LENGTH (48000*50)
// ---< Global variabels >---


//Struct with status of noise reduction algorithm
//nr_status_t nr_status;

//#pragma  DATA_SECTION (micData, "extvar")
//#pragma  DATA_ALIGN(micData, 64)
//volatile q15 micData[467496];
#pragma  DATA_SECTION (soundRight, "extvar")
#pragma  DATA_SECTION (soundLeft, "extvar")
#pragma  DATA_ALIGN(soundRight, 64)
#pragma  DATA_ALIGN(soundLeft, 64)
int16_t soundRight[RECORD_LENGTH]; //Store 3 seconds of data
int16_t soundLeft[RECORD_LENGTH];

//extern float fProb;

// Time indices for sine wave
unsigned int uiSineTimeIndex[N_RX + N_MIC];

// ---< Local function prototypes >---

// ---< Functions >---

/*****************************************************************************
 *	\brief		test main function to run in main.c
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
void test_main(){
	unsigned int n, m;
	// Initializes rx and mic buffers to zeros
	memset(audioRx, 0, N_RX*BUFLEN_8KHZ*sizeof(short));
	memset(audioMic, 0, N_MIC*BUFLEN_8KHZ*sizeof(short));
	memset(audioTx, 0, N_TX*BUFLEN_8KHZ*sizeof(short));
	memset(audioLs, 0, N_LS*BUFLEN_8KHZ*sizeof(short));

	USTIMER_init();
	I2C_init(I2C0, I2C_CLK_400K);

	// Init and config of TrueVoice
	config_true_voice_test();

	// Setup of pointers to audio buffers
	init_audio_signals_test();

	if(USE_DEBUG_PLOT) {

		//create_PESQ_files();

		// DEBUG of mixer. Run TrueVoice for TV_PLOT_LENGTH samples and dump output to files
		test_mixer();

		//test_pilot_connections();
		// DEBUG of DTMF.
		//test_dtmf();
		//DEBUG of noise reduction
		//test_noise_reduction();
		//test_fileInput();
		//DEBUG of subband functions
		//test_subband();
		//DEBUG of LEC
		//test_lec();
		//test_all_lec();
		//DEBUG of mute
		//test_mute();

		//Line In/Out
		//test_audio_loop();
//		TEST_audio();
//		mitt_test();
//		for(m=0;m<666;m++){ //Infinite loop
//			//Update inputBuffer
//			for(n=0; n<BUFLEN_8KHZ; n++){
//				audioMic[1][n] = n;
//			}
//
//			//Run true voice
//			true_voice(rx, mic, tx, ls);
//		}
	} else {
		// Run TrueVoice once
		//while(1){
			//simulate_capture_audio();
			//true_voice(rx, mic, tx, ls);
			//simulate_send_audio();
		//}

	}
}

/*****************************************************************************
 *	\brief		Initialization and configuration of TrueVoice
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
void config_true_voice_test() { // Catron-controlled function
	rx_channel_enum_t nr;

	// Init TrueVoice
	//true_voice_init(TV_CONFIG_PT);
	true_voice_init(TV_CONFIG_BS); //should always be called before any additional settings

	bool bfirstTech = true; // should be removed

	if(!bfirstTech) {
		// 1st tech channel is from PT1
		true_voice_mixer_1st_tech_channel(RX_CH_TECH_1);
		// Unless channel 8 is tech this should be 0
		true_voice_mixer_mode(0);
	}
	else {
		// This is 1st tech unit
		true_voice_mixer_1st_tech_channel(RX_CH_TECH_8);
		// This unit can only be first tech if channel 8 is
		// first tech since this unit is ALWAYS channel 8
		true_voice_mixer_mode(1);
	}
	true_voice_pilot_mode(0); 			// 0 = Pilot aircraft cable, 1 = Pilot headset
	true_voice_clt_mode(1); 			// 0 = 4-wire, 1 = 2-wire
	//true_voice_push_to_talk(1); 		//No Vad, open channel

	true_voice_volume_speaker(1, 0);	// 1 = Technician headset
	true_voice_volume_speaker(2, 0);	// 2 = Pilot headset (pilot mono)
	true_voice_dtmf_settings(80, 80, 0);

	true_voice_module_enable(AGC, 1);
	true_voice_module_enable(DIRECTIVE_AUDIO, 1);
	true_voice_module_enable(LEC, 1);
	true_voice_module_enable(LS_LIMITER, 1);
	true_voice_module_enable(NOISE_REDUCTION, 1);

	true_voice_1st_tech_active_channel_out(TV_TECH1_TO_LMR, 1);
	true_voice_1st_tech_active_channel_out(TV_TECH1_TO_FTN, 1);
	true_voice_1st_tech_active_channel_out(TV_TECH1_TO_LS_TECH, 1);
	true_voice_1st_tech_active_channel_out(TV_TECH1_TO_LS_PILOT, 1);

//	true_voice_mixer_active_channel(RX_CH_FTN, 1);

	// Disable some channels in mixer...
//	for(nr=RX_CH_TECH_1; nr<NUM_RX_CHANNELS; nr++) {
//		printf("Mixer input active channel %d status is: %d\n", nr, true_voice_mixer_active_channel(nr,-1));
//	}
//	true_voice_mixer_active_channel(RX_CH_TECH_1, 0);
//	true_voice_mixer_active_channel(RX_CH_TECH_3, 0);
//	true_voice_mixer_active_channel(RX_CH_TECH_5, 0);
//	true_voice_mixer_active_channel(RX_CH_TECH_6, 0);
//	true_voice_mixer_active_channel(RX_CH_FTN, 0);
//	for(nr=RX_CH_TECH_1; nr<NUM_RX_CHANNELS; nr++) {
//		printf("Mixer input active channel %d status is: %d\n", nr, true_voice_mixer_active_channel(nr,-1));
//	}

	// Display version number for TrueVoice
	display_version_number();
}

/*****************************************************************************
 *	\brief		Display TrueVoice version number
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
static void display_version_number() {
	short major, minor, build;
	char date[12], time[9], s[100];

	true_voice_get_version_info(&major, &minor, &build, date, time);
	sprintf(s, "Limes Audio TrueVoice library v%d.%02d.%03d\tBuild date:\t%s\t%s\n", major, minor, build, date, time);
	printf("%s\n", s);
}

/*****************************************************************************
 *	\brief		Init audio signals
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
static void init_audio_signals_test() {
	int channel;

	// === Allocate memory for Truevoice struct


	// === Setup of pointers to TrueVoice struct ===
	channel = 0;
	//receive channels trough radio
	rx.psTech1		= audioRx[channel++];// channel 0
	rx.psTech2		= audioRx[channel++];
	rx.psTech3		= audioRx[channel++];
	rx.psTech4		= audioRx[channel++];
	rx.psTech5		= audioRx[channel++];
	rx.psTech6		= audioRx[channel++];// channel 5
	rx.psTech7		= audioRx[channel++];
	// if PT:This should be empty since this channel is supposed to be the "own" channel (i.e the TechHeadset)
	// if BS: this is not empty
	rx.psTech8		= audioRx[channel++];
	// generated by Application
	rx.psMmiSounds	= audioRx[channel++];
	// other channels
	rx.psLmr		= audioRx[channel++];
	rx.psFtn		= audioRx[channel++];// channel 10
	rx.psPilot		= audioRx[channel++];
	rx.psClt		= audioRx[channel++];
	rx.psClt4wire	= audioRx[channel++];
	rx.psClt2wire	= audioRx[channel++];

	channel = 0;
	mic.psMicNoise		= audioMic[channel++];	//channel 0
	mic.psTechHeadset	= audioMic[channel++];
	// Only if BS and CA:
	mic.psPilotHeadset	= audioMic[channel++];
	mic.psPilotCable	= audioMic[channel++];
	mic.psPilotCltCable	= audioMic[channel++];

	channel = 0;
	tx.psTechToRadio	= audioTx[channel++]; //channel 0
	//only CA and BS
	tx.psPilotAndCltToRadio	= audioTx[channel++];
	//only BS
	tx.psFtnToLine		= audioTx[channel++];
	tx.psCltToLine		= audioTx[channel++];
	tx.psLmrToLine		= audioTx[channel++];
	tx.psFtnToRadio		= audioTx[channel++];
	tx.psCltToRadio		= audioTx[channel++];
	tx.psLmrToRadio		= audioTx[channel++];

	channel = 0;
	// Only PT and CA
	ls.psTechLeft	= audioLs[channel++];
	ls.psTechRight	= audioLs[channel++];
	//only BS
	ls.psTechMono	= audioLs[channel++];
	//only BS and CA
	ls.psPilotMono	= audioLs[channel++];
	ls.psPilotMonoCable	= audioLs[channel++];
	//only BS
	ls.psPilotMonoCltCable	= audioLs[channel++];

}

/*****************************************************************************
 *	\brief		Sine wave generation for a signal block
 *	\parameters	x[]			- Output signal buffer
 * 				uiFreq		- Desired frequency [Hz]
 * 				puiIndex	- Time index counter
 *	\return		None
 *****************************************************************************/
void sin_block(short x[], const unsigned int uiFreq, unsigned int *puiIndex) {
	const unsigned int fs = 8000; // Sample rate
	float fTemp;
	float fPi = 3.14159265359;
	int i;

	for(i=0; i<BUFLEN_8KHZ; i++) {
		fTemp = 2*fPi*uiFreq * *puiIndex;
		x[i] = 32767*sin(fTemp / fs); // sin(2*pi*f*t) in q15 format
		if(++*puiIndex > 3*fs) {*puiIndex = 0;} // Prevent overflow
	}
}

/*****************************************************************************
 *	\brief		Function used for creating files for PESQ simulaition
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
void create_PESQ_files(void){
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i;
	int temp;
	int speechIndex;

	// Initialize time indices
	if(iFirstTime) {
		plot_init();
	}

	speechIndex = 0;
	temp=0;

	while(temp < TV_PLOT_LENGTH){
		temp+=BUFLEN_8KHZ;
		//Update input buffer
		if (temp>=2*(46004-BUFLEN_8KHZ)){
			for(i=0; i<BUFLEN_8KHZ; i++){
				mic.psTechHeadset[i] = 0;
				mic.psMicNoise[i] = 0;
			}
		}
		else {
			for(i=0; i<BUFLEN_8KHZ; i++){
				mic.psTechHeadset[i] = speech_and_noise_headset[i+speechIndex];
				mic.psMicNoise[i] = speech_and_noise_ref[i+speechIndex];
			}
		}
		speechIndex+=BUFLEN_8KHZ;

		if (speechIndex>=(46004-BUFLEN_8KHZ)){
			speechIndex=0;
		}

		//true_voice_push_to_talk(0);
		true_voice(rx, mic, tx, ls);

		write_plot_data(mic.psTechHeadset, &plot1, BUFLEN_8KHZ);

	}
	// Write stored data to output file
	write_plot_buffer_to_file("post_trueVoice", &plot1);
#endif // USE_DEBUG_PLOT
}

/*****************************************************************************
 *	\brief		Testing mixer by creating sine waves for all input channels
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
void test_mixer(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i, k,n;
	int temp;
	int speechIndex;
	bool bUseSpeech = false;
	bool bUseCltCable = false;

	// Initialize time indices
	if(iFirstTime) {
		for(i=0; i<(N_RX + N_MIC); i++) {
			uiSineTimeIndex[i] = 0;
		}
		plot_init();
		iFirstTime = 0;
	//	true_voice_pilot_mode(0);
	//	true_voice_clt_mode(1);

		if (bUseSpeech) {
			true_voice_dtmf_queue('A');
			true_voice_dtmf_queue('A');
			true_voice_dtmf_remove_last_added();
			true_voice_dtmf_queue('7');
			true_voice_dtmf_send_queue();
		}
	}

	if (!bUseSpeech){
	// Use sine waves
		while(uiSineTimeIndex[0] < TV_PLOT_LENGTH) {
			// Create sine wave for each channel with frequency 100 Hz, 200 Hz, 300 Hz, ...
			for(i=0; i<N_RX; i++) {
				sin_block(audioRx[i], (i+1)*100, &uiSineTimeIndex[i]);
			}
			for(i=0; i<N_MIC; i++) {
				if (tvSettings.bPilotConnection == AIRCRAFT_CABLE) {
					sin_block(audioMic[i], (i+1+N_RX)*100, &uiSineTimeIndex[i+N_RX]);
					if (bUseCltCable) {
						if ((i==(N_MIC-2))) {
							sin_block(audioMic[i], (i+1+N_RX)*0, &uiSineTimeIndex[i+N_RX]);
						}
					}
					else {
						if ((i==(N_MIC-1))) {
							sin_block(audioMic[i], (i+1+N_RX)*0, &uiSineTimeIndex[i+N_RX]);
						}
					}
				}
				else {
					sin_block(audioMic[i], (i+1+N_RX)*100, &uiSineTimeIndex[i+N_RX]);
				}
			}

			//TODO: eventually remove temporary signals for limiter testing
			// TEMPORARY: change signals for limiter testing
//			for(i=0; i<N_RX; i++) {
//				if(uiSineTimeIndex[0] < TV_PLOT_LENGTH*0.1) {
//					for (k=0; k<BUFLEN_8KHZ; k++){
//						audioRx[i][k]=mult_q15(audioRx[i][k], 8000);
//					}
//				}
//				else if (uiSineTimeIndex[0] < (TV_PLOT_LENGTH)*0.3) {
//					for (k=0; k<BUFLEN_8KHZ; k++){
//						audioRx[i][k]=mult_q15(audioRx[i][k], 32767);
//					}
//				}
//				else{
//					for (k=0; k<BUFLEN_8KHZ; k++){
//						audioRx[i][k]=mult_q15(audioRx[i][k], 8000);
//					}
//				}
//			}

//			if (uiSineTimeIndex[0]>400) {
//				true_voice_volume_speaker(1, 10);
//			}
//			if (uiSineTimeIndex[0]>500) {
//				true_voice_volume_speaker(2, 5);
//			}

			// Mixer
			true_voice(rx, mic, tx, ls);

			// Store data (see plot.c in true_voice)
			write_plot_data(tx.psTechToRadio, 		 &plot1, BUFLEN_8KHZ);
			write_plot_data(tx.psPilotAndCltToRadio, &plot2, BUFLEN_8KHZ);
			write_plot_data(tx.psFtnToLine,   		 &plot3, BUFLEN_8KHZ);
			write_plot_data(tx.psCltToLine,   		 &plot4, BUFLEN_8KHZ);
			write_plot_data(tx.psLmrToLine,   		 &plot5, BUFLEN_8KHZ);
			write_plot_data(tx.psFtnToRadio,  		 &plot6, BUFLEN_8KHZ);
			write_plot_data(tx.psCltToRadio	, 		 &plot7, BUFLEN_8KHZ);
			write_plot_data(tx.psLmrToRadio,  		 &plot8, BUFLEN_8KHZ);
			write_plot_data(ls.psTechLeft, 	 		 &plot9,  BUFLEN_8KHZ);
			write_plot_data(ls.psTechRight, 		&plot10, BUFLEN_8KHZ);
			write_plot_data(ls.psTechMono,  		&plot11, BUFLEN_8KHZ);
			write_plot_data(ls.psPilotMono, 		&plot12, BUFLEN_8KHZ);
			write_plot_data(ls.psPilotMonoCable,  	&plot13, BUFLEN_8KHZ);
			write_plot_data(ls.psPilotMonoCltCable, &plot14, BUFLEN_8KHZ);

		}
	}// end sin waves

	if (bUseSpeech) {
		// Send male or female speech through a channel
		speechIndex = 0;
		temp=0;

		while(temp < TV_PLOT_LENGTH){
			temp += BUFLEN_8KHZ;

			//Update input buffer
			for(i=0; i<BUFLEN_8KHZ; i++){
				rx.psTech1[i] = maleSpeech[i+speechIndex]; //mic.psTechHeadset = audioMic[1]
				rx.psPilot[i] = femaleSpeech[i+speechIndex];
				rx.psLmr[i] = femaleSpeech[i+speechIndex];
			}
			speechIndex+=BUFLEN_8KHZ;
			if (speechIndex>=11311-BUFLEN_8KHZ){
				speechIndex=0;
			}

//			if(speechIndex == (32*35)){
//				true_voice_dtmf_send_queue();
//			}
/*
			true_voice_volume_speaker(1, 0);
			true_voice_volume_speaker(2, 10);

			if (speechIndex>5000) {
					true_voice_volume_speaker(1, 10);
					true_voice_volume_speaker(2, 5);
			}
			if (speechIndex>10000) {
					true_voice_volume_speaker(1, 6);
					true_voice_volume_speaker(2, 3);
			}
*/
			// Mixer

//			clock_t t_start, t_stop, t_overhead;
//			t_start = clock();
//			t_stop = clock();
//			t_overhead = t_stop - t_start;
//			t_start = clock();
			true_voice(rx, mic, tx, ls);
//			t_stop  = clock();
//			printf("True Voice took: %f ms\n", (t_stop-t_start-t_overhead)/300E6*1000); //Enable clock first while in debug: Run->Clock->Enable

			// Store data (see plot.c in true_voice)
			write_plot_data(tx.psTechToRadio,  		&plot1,  BUFLEN_8KHZ);
			write_plot_data(tx.psPilotAndCltToRadio,&plot2,  BUFLEN_8KHZ);
			write_plot_data(tx.psFtnToLine,    		&plot3,  BUFLEN_8KHZ);
			write_plot_data(tx.psCltToLine,    		&plot4,  BUFLEN_8KHZ);
			write_plot_data(tx.psLmrToLine,    		&plot5,  BUFLEN_8KHZ);
			write_plot_data(tx.psFtnToRadio,   		&plot6,  BUFLEN_8KHZ);
			write_plot_data(tx.psCltToRadio	,  		&plot7,  BUFLEN_8KHZ);
			write_plot_data(tx.psLmrToRadio,  		&plot8,  BUFLEN_8KHZ);
			write_plot_data(ls.psTechLeft,   		&plot9,  BUFLEN_8KHZ);
			write_plot_data(ls.psTechRight,	   		&plot10, BUFLEN_8KHZ);
			write_plot_data(ls.psTechMono,   		&plot11, BUFLEN_8KHZ);
			write_plot_data(ls.psPilotMono,    		&plot12, BUFLEN_8KHZ);
			write_plot_data(ls.psPilotMonoCable,  	&plot13, BUFLEN_8KHZ);
			write_plot_data(ls.psPilotMonoCltCable, &plot14, BUFLEN_8KHZ);

		}

	}//end speech

	// Write stored data to output file
	write_plot_buffer_to_file("Tx0_TechToRadio.dat",  &plot1);
	write_plot_buffer_to_file("Tx1_PilotAndCltToRadio.dat", &plot2);
	write_plot_buffer_to_file("Tx2_FtnToLine.dat",	  &plot3);
	write_plot_buffer_to_file("Tx3_CltToLine.dat", 	  &plot4);
	write_plot_buffer_to_file("Tx4_LmrToLine.dat",	  &plot5);
	write_plot_buffer_to_file("Tx5_FtnToRadio.dat",   &plot6);
	write_plot_buffer_to_file("Tx6_CltToRadio.dat",   &plot7);
	write_plot_buffer_to_file("Tx7_LmrToRadio.dat",   &plot8);
	write_plot_buffer_to_file("Ls0_TechLeft.dat",  	  &plot9);
	write_plot_buffer_to_file("Ls1_TechRight.dat", 	  &plot10);
	write_plot_buffer_to_file("Ls2_TechMono.dat",  	  &plot11);
	write_plot_buffer_to_file("Ls3_PilotMono.dat", 	  &plot12);
	write_plot_buffer_to_file("Ls4_PilotCable.dat",   &plot13);
	write_plot_buffer_to_file("Ls5_PilotCltCable.dat",&plot14);

	write_plot_buffer_to_file("plotVar1.dat", &plotVar1);
//	write_plot_buffer_to_file("plotVar2.dat", &plotVar2);
//	write_plot_buffer_to_file("plotVar3.dat", &plotVar3);

#endif // USE_DEBUG_PLOT
}

/*****************************************************************************
 *	\brief		Simulates the use and switch between the three different pilot
 *				connections in real time.
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
void test_pilot_connections(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i, k,n;
	int temp;
	int speechIndex;
	int speechIndex2;
	bool bUseSpeech = true;

	// Initialize time indices
	if(iFirstTime) {
		plot_init();
		iFirstTime = 0;

		if (bUseSpeech) {
			true_voice_dtmf_queue('A');
			true_voice_dtmf_queue('A');
			true_voice_dtmf_remove_last_added();
			true_voice_dtmf_queue('7');
			//true_voice_dtmf_send_queue();
		}
	}
	// Send male or female speech through a channel
	speechIndex = 0;
	speechIndex2 = 0;
	temp=0;

	while(temp < TV_PLOT_LENGTH){
		temp += BUFLEN_8KHZ;
		//Update input buffer
		for(i=0; i<BUFLEN_8KHZ; i++){
			//mic.psTechHeadset[i] = femaleSpeech[i+speechIndex]; //mic.psTechHeadset = audioMic[1]
			rx.psFtn[i] = femaleSpeech[i+speechIndex];
			rx.psClt2wire[i] = femaleSpeech[i+speechIndex];
			rx.psLmr[i] = maleSpeech[i+speechIndex];

			// Start with pilot in aircraft with switch to Clt StriC
			if (temp<11000){
				mic.psPilotCable[i] = 0;
				mic.psPilotCltCable[i] = maleSpeech[i+speechIndex];
				mic.psPilotHeadset[i] = 0;

				rx.psClt2wire[i] = femaleSpeech[i+speechIndex];
				speechIndex2 = 0;
			}
			// Pilot in aircraft change switch to talk to 1st tech
			else if(temp<22000){
				mic.psPilotCable[i] = maleSpeech[i+speechIndex2];
				mic.psPilotCltCable[i] = 0; //mic.psTechHeadset = audioMic[1]
				mic.psPilotHeadset[i] = 0;

				rx.psClt2wire[i] = femaleSpeech[i+speechIndex2];
				rx.psFtn[i] = femaleSpeech[i+speechIndex2];
				speechIndex = 0;
			}
			// Pilot use headset jack on BS
			else{
				true_voice_pilot_mode(1); 			// 0 = Pilot aircraft cable, 1 = Pilot headset
				mic.psPilotCable[i] = 0;
				mic.psPilotCltCable[i] = 0;
				mic.psPilotHeadset[i] = maleSpeech[i+speechIndex];
			}
		}
		speechIndex+=BUFLEN_8KHZ;
		speechIndex2+=BUFLEN_8KHZ;

		if (speechIndex>=11311-BUFLEN_8KHZ){
			speechIndex=0;
		}
		if (speechIndex2>=11311-BUFLEN_8KHZ){
			speechIndex2=0;
		}

		if (temp == (32*30)){
			true_voice_dtmf_send_queue();
		}

		// Mixer
		true_voice(rx, mic, tx, ls);

		// Store data (see plot.c in true_voice)
		write_plot_data(tx.psTechToRadio,  		&plot1,  BUFLEN_8KHZ);
		write_plot_data(tx.psPilotAndCltToRadio, &plot2,  BUFLEN_8KHZ);
		write_plot_data(tx.psFtnToLine,    		&plot3,  BUFLEN_8KHZ);
		write_plot_data(tx.psCltToLine,    		&plot4,  BUFLEN_8KHZ);
		write_plot_data(tx.psLmrToLine,    		&plot5,  BUFLEN_8KHZ);
		write_plot_data(tx.psFtnToRadio,   		&plot6,  BUFLEN_8KHZ);
		write_plot_data(tx.psCltToRadio	,  		&plot7,  BUFLEN_8KHZ);
		write_plot_data(tx.psLmrToRadio,  		&plot8,  BUFLEN_8KHZ);
		write_plot_data(ls.psTechLeft,   		&plot9,  BUFLEN_8KHZ);
		write_plot_data(ls.psTechRight,	   		&plot10, BUFLEN_8KHZ);
		write_plot_data(ls.psTechMono,   		&plot11, BUFLEN_8KHZ);
		write_plot_data(ls.psPilotMono,    		&plot12, BUFLEN_8KHZ);
		write_plot_data(ls.psPilotMonoCable,  	&plot13, BUFLEN_8KHZ);
		write_plot_data(ls.psPilotMonoCltCable, &plot14, BUFLEN_8KHZ);

	}

	// Write stored data to output file
	write_plot_buffer_to_file("Tx0_TechToRadio.dat",  &plot1);
	write_plot_buffer_to_file("Tx1_PilotAndCltToRadio.dat", &plot2);
	write_plot_buffer_to_file("Tx2_FtnToLine.dat",	  &plot3);
	write_plot_buffer_to_file("Tx3_CltToLine.dat", 	  &plot4);
	write_plot_buffer_to_file("Tx4_LmrToLine.dat",	  &plot5);
	write_plot_buffer_to_file("Tx5_FtnToRadio.dat",   &plot6);
	write_plot_buffer_to_file("Tx6_CltToRadio.dat",   &plot7);
	write_plot_buffer_to_file("Tx7_LmrToRadio.dat",   &plot8);
	write_plot_buffer_to_file("Ls0_TechLeft.dat",  	  &plot9);
	write_plot_buffer_to_file("Ls1_TechRight.dat", 	  &plot10);
	write_plot_buffer_to_file("Ls2_TechMono.dat",  	  &plot11);
	write_plot_buffer_to_file("Ls3_PilotMono.dat", 	  &plot12);
	write_plot_buffer_to_file("Ls4_PilotCable.dat",   &plot13);
	write_plot_buffer_to_file("Ls5_PilotCltCable.dat",&plot14);
#endif //USE_DEBUG_PLOT
}

/*****************************************************************************
 *	\brief		Testing DTMF using interface functions
 *	\parameters	None
 *	\return		None
 *****************************************************************************/
void test_dtmf(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int k;

	// Initialize time indices
	if(iFirstTime) {
		plot_init();
		iFirstTime = 0;
		//true_voice_dtmf_settings(60, 70);
		true_voice_dtmf_queue('A');
		true_voice_dtmf_queue('A');
		true_voice_dtmf_queue('A');
		true_voice_dtmf_remove_last_added();
		true_voice_dtmf_queue('7');
		//true_voice_dtmf_send_queue();
	}
	k = 0;
	while(k < TV_PLOT_LENGTH){
		k=k+BUFLEN_8KHZ;

		//true_voice_dtmf_clear_queue();
		//true_voice_dtmf_remove_last_added();
		if(k == (32*35)){
			true_voice_dtmf_send_queue();
		}
		if (k == (32*60)){
			true_voice_dtmf_clear_queue();
		}
		/*if(k%(32*50)){
			true_voice_dtmf_queue('7');
		}*/

		true_voice(rx, mic, tx, ls);
	}

	// Write stored data to output file
	write_plot_buffer_to_file("DTMFline.dat", &plotVar1);
	write_plot_buffer_to_file("DTMFls.dat", &plotVar2);
#endif // USE_DEBUG_PLOT
}


void test_audio_loop(void){
	unsigned int n;
	clock_t t_start, t_stop;

//	while(1){
//		getAudioBlock(soundRight, soundLeft, RECORD_LENGTH);
		printf("Now processing received sound\n");
		t_start = clock();
		for(n=0; n<RECORD_LENGTH/BUFLEN_8KHZ; n++){
			memcpy(mic.psTechHeadset, &soundRight[n*BUFLEN_8KHZ], BUFLEN_8KHZ*sizeof(short));
			memcpy(mic.psMicNoise, &soundLeft[n*BUFLEN_8KHZ], BUFLEN_8KHZ*sizeof(short));
			true_voice(rx, mic, tx, ls);
			memcpy(&soundRight[n*BUFLEN_8KHZ], mic.psTechHeadset, BUFLEN_8KHZ*sizeof(short));
		}
		t_stop = clock();
		printf("algorithm took: %f s\n", (t_stop-t_start)/300E6); //Enable clock first while in debug: Run->Clock->Enable
		printf("which is: %f ms per loop\n", (t_stop-t_start)/300E6/n*1000);

//		sendAudioBlock(&soundRight[BUFLEN_8KHZ], &soundLeft[BUFLEN_8KHZ], RECORD_LENGTH);
//		sendAudioBlock(soundRight, soundLeft, RECORD_LENGTH);
//		sendAudioBlock(soundRight, soundRight, RECORD_LENGTH);
		printf("\n\n");
//	}

}

void test_noise_reduction(void)  {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i;

	// Initialize time indices
	if(iFirstTime) {
		for(i=0; i<(N_RX + N_MIC); i++) {
			uiSineTimeIndex[i] = 0;
		}
		plot_init();
		iFirstTime = 0;
	}

	while(uiSineTimeIndex[0] < TV_PLOT_LENGTH) {
		// Create sine wave for each channel with frequency 100 Hz, 200 Hz, 300 Hz, ...
		for(i=0; i<N_RX; i++) {
			sin_block(audioRx[i], (i+1)*100, &uiSineTimeIndex[i]);
		}
		for(i=0; i<N_MIC; i++) {
			sin_block(audioMic[i], (i+1+N_RX)*100, &uiSineTimeIndex[i+N_RX]);
		}
//
//		write_plot_data(mic.psPilotHeadset, &plot1, BUFLEN_8KHZ);
//		write_plot_buffer_to_file("micPilot.dat", &plot1);
		// where all the shit is
		true_voice(rx, mic, tx, ls);
		// Store data (see plot.c in true_voice)

		write_plot_data(mic.psPilotHeadset, &plot1, BUFLEN_8KHZ);
//		write_plot_buffer_to_file("fftMag.dat", &plot1);
	}

	// Write stored data to output file
	write_plot_buffer_to_file("micPilot.dat", &plot1);
//	write_plot_buffer_to_file("micNoise.dat", &plot2);
//	write_plot_buffer_to_file("lsPilot.dat", &plot3);
#endif // USE_DEBUG_PLOT
}



void test_fileInput(void)  {
#if USE_DEBUG_PLOT
	long int i;
	plot_init();

	bufferedFileHandle_t input_file_handle;
	bufferedFileHandle_t noise_file_handle;
	short resI = createBufferedFileReader("..\\..\\data\\input_startMotorPrat8k20.bin", &input_file_handle);
	short resN = createBufferedFileReader("..\\..\\data\\noise_startMotorPrat8k5.bin", &noise_file_handle);

	fseek(input_file_handle.file, sizeof(short)*8000*4, SEEK_SET);
	fseek(noise_file_handle.file, sizeof(short)*8000*4, SEEK_SET);

	true_voice_module_enable(NOISE_REDUCTION, 1);
	true_voice_module_enable(AGC, 0);

	clock_t t_start, t_stop;
	t_start = clock();
	for(i=0; i*BUFLEN_8KHZ<TV_PLOT_LENGTH; i++){
		// where all the shit is
		short bytesReadI = readData(mic.psPilotHeadset, sizeof(short)*BUFLEN_8KHZ, &input_file_handle);
		short bytesReadN = readData(mic.psMicNoise, sizeof(short)*BUFLEN_8KHZ, &noise_file_handle);
		int n;
//		for(n=0; n<BUFLEN_8KHZ; n++){
//			mic.psMicNoise[n] >>= 6;
//		}

		if (bytesReadI != sizeof(short)*BUFLEN_8KHZ || bytesReadN != sizeof(short)*BUFLEN_8KHZ)
			break;

		int hej = 0;
		if(i==19){
			hej = hej+1;
		}
		write_plot_data(mic.psPilotHeadset, &plot2, BUFLEN_8KHZ);
		true_voice(rx, mic, tx, ls);
		// Store data (see plot.c in true_voice)
		write_plot_data(mic.psPilotHeadset, &plot1, BUFLEN_8KHZ);

		/*
		short test;
		if(fProb*10 >MAX_16)
			test = MAX_16;
		else
			test = 10*fProb;

//		write_plot_data(&test, &plot2, 1);
		*/
	}
	t_stop = clock();
	printf("algorithm took: %f s\n", (t_stop-t_start)/300E6); //Enable clock first while in debug: Run->Clock->Enable
	printf("which is: %f ms per loop\n", (t_stop-t_start)/300E6/i*1000); //Enable clock first while in debug: Run->Clock->Enable
	closeBufferedFileReader(&input_file_handle);
	closeBufferedFileReader(&noise_file_handle);

	// Write stored data to output file
	write_plot_buffer_to_file("micPilot.dat", &plot1);
	write_plot_buffer_to_file("micPilotOrg.dat", &plot2);
//	write_plot_buffer_to_file("voxProb.dat", &plot2);
//	write_plot_buffer_to_file("micNoise.dat", &plot2);
//	write_plot_buffer_to_file("lsPilot.dat", &plot3);
#endif // USE_DEBUG_PLOT
}


complex_q31 pcq31out[DEC_INDEX+1];
q31 pq31res[BUFLEN_8KHZ];

void test_subband(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i;


	// Initialize time indices
	if(iFirstTime) {
		for(i=0; i<(N_RX + N_MIC); i++) {
			uiSineTimeIndex[i] = 0;
		}
		plot_init();
		iFirstTime = 0;
		subband_init();
		//dumpdata_dump_init();
	}

	while(uiSineTimeIndex[0] < TV_PLOT_LENGTH) {

		// Create sine wave for each channel with frequency 100 Hz, 200 Hz, 300 Hz, ...
		for(i=0; i<N_RX; i++) {
			sin_block(audioRx[i], (i+1)*100, &uiSineTimeIndex[i]);
		}
		for(i=0; i<N_MIC; i++) {
			sin_block(audioMic[i], (i+1+N_RX)*100, &uiSineTimeIndex[i+N_RX]);
		}

		vecmultq15xScalar(audioRx[0], audioRx[0], (q15)31139/*0.95*/, 15, BUFLEN_8KHZ);

		q15 pq15res[BUFLEN_8KHZ];
		//subband_analysis(audioRx[0], pcq31out, pq15Prot_delayRX, &iProt_delayIndexRX);
		//subband_synthesis(pcq31out, pq31res, pq31Prot_delayLS, &iProt_delayIndexLS, 0);

		int i;
		for (i=0; i<BUFLEN_8KHZ; i++) {
			pq15res[i] = pq31res[i];
		}

		// Store data (see plot.c in true_voice)
		write_plot_data(audioRx[0], &plot1, BUFLEN_8KHZ);
		write_plot_data(pq15res, &plot2, BUFLEN_8KHZ);
		//dumpdata_post_dump();
	}

	// Write stored data to output file
	write_plot_buffer_to_file("beforeSub.dat", &plot1);
	write_plot_buffer_to_file("afterSub.dat", &plot2);

#endif //USE_DEBUG_PLOT
}

typedef struct
{
#define ECHO_BUF_SIZE 3
	q15 q15DelayBuff[ECHO_BUF_SIZE*BUFLEN_8KHZ];
	int dlyIndex;
} t_echo_handler;

t_echo_handler createEchoHandler()
{
	t_echo_handler h;
	memset(h.q15DelayBuff, 0, ECHO_BUF_SIZE*BUFLEN_8KHZ*sizeof(q15));
	h.dlyIndex = 0;
	return h;
}

void create_echo(short* src, short* src2, short* dest, bool useSrc2, t_echo_handler* handler) {
	int i;

	//fake echo path with 3*BUFFLEN delay and -6dB amplification
	memcpy(dest,&handler->q15DelayBuff[handler->dlyIndex*BUFLEN_8KHZ], BUFLEN_8KHZ * sizeof(q15));
	memcpy(&handler->q15DelayBuff[handler->dlyIndex*BUFLEN_8KHZ],src, BUFLEN_8KHZ * sizeof(q15));
	if (useSrc2) {
		for(i=0;i<BUFLEN_8KHZ;i++) {
			dest[i]=dest[i]/2+src2[i];
		}
	}
	else {
		for(i=0;i<BUFLEN_8KHZ;i++){
			dest[i]=dest[i]/2;
		}
	}
	handler->dlyIndex++;
	if(handler->dlyIndex >= ECHO_BUF_SIZE)
		handler->dlyIndex=0;
}

void test_lec(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i,j;
	int speechIndex=0;
	bool bUseSpeech = true;
	bool bUseTrueVoice = true;
	t_echo_handler clt_echo_handler;
	t_echo_handler ftn_echo_handler;

	// Initialize time indices
	if(iFirstTime) {
		subband_init();
		plot_init();
		iFirstTime = 0;
		for(i=0; i<(N_RX + N_MIC); i++) {
			uiSineTimeIndex[i] = 0;
		}

		clt_echo_handler = createEchoHandler();
		ftn_echo_handler = createEchoHandler();

		true_voice_clt_mode(1);
		//true_voice_module_enable(LEC, 0);
	}

	short* pMicIn = bUseTrueVoice ? mic.psPilotCable : tx.psCltToLine;
	short* pMicIn2 = bUseTrueVoice ? 0 : tx.psFtnToLine;
	for(j=0; j<7; j++) {
		speechIndex = 0;
		while(speechIndex < TV_PLOT_LENGTH/7) {
		if (bUseSpeech) {
			if (bUseTrueVoice)
			{
				for (i=0; i<BUFLEN_8KHZ; i++){
					if (i+speechIndex<11311 && j != 0){
						pMicIn[i] =maleSpeech[i+speechIndex];
					}
					else{
						pMicIn[i] = 0;
					}
				}
			}
			else
			{
				for (i=0; i<BUFLEN_8KHZ; i++){
					if (i+speechIndex<11311 && j != 0){
						pMicIn[i] =maleSpeech[i+speechIndex];
						pMicIn2[i] =femaleSpeech[i+speechIndex];
					}
					else{
						pMicIn[i] = 0;
						pMicIn2[i] = 0;
					}
				}
			}

		}
		else {
			sin_block(pMicIn, (0+1)*200, &uiSineTimeIndex[0]);
			if (!bUseTrueVoice) sin_block(pMicIn2, (0+1)*200, &uiSineTimeIndex[1]);
			vecmultq15xScalar(pMicIn, pMicIn, (q15)(INT16_MAX*0.99), 15 ,BUFLEN_8KHZ);
			if (!bUseTrueVoice) vecmultq15xScalar(pMicIn2, pMicIn2, (q15)(INT16_MAX*0.99), 15 ,BUFLEN_8KHZ);
		}

		if (bUseTrueVoice) {
			true_voice(rx, mic, tx, ls);
			write_plot_data(rx.psFtn,    &plot6, BUFLEN_8KHZ);
			if (j>=4) {
				create_echo(tx.psCltToLine, &femaleSpeech[speechIndex], rx.psClt2wire, true, &clt_echo_handler);
				create_echo(tx.psFtnToLine, &femaleSpeech[speechIndex], rx.psFtn, true, &ftn_echo_handler);
			}
			else {
				create_echo(tx.psCltToLine, 0, rx.psClt2wire, false, &clt_echo_handler);
				create_echo(tx.psFtnToLine, 0, rx.psFtn, false, &ftn_echo_handler);
			}

			write_plot_data(rx.psFtn,    &plot4, BUFLEN_8KHZ);
		}
		else {
			if (j>=4) {
				create_echo(tx.psCltToLine, &femaleSpeech[speechIndex], rx.psClt2wire, true, &clt_echo_handler);
				create_echo(tx.psFtnToLine, &maleSpeech[speechIndex], rx.psFtn, true, &ftn_echo_handler);
			}
			else {
				create_echo(tx.psCltToLine, 0, rx.psClt2wire, false, &clt_echo_handler);
				create_echo(tx.psFtnToLine, 0, rx.psFtn, false, &ftn_echo_handler);
			}

			write_plot_data(rx.psFtn,    &plot4, BUFLEN_8KHZ);

			internal_lec(rx.psClt2wire, tx.psCltToLine, rx.psClt);
			internal_lec_2(rx.psFtn, tx.psFtnToLine, rx.psFtn);

			write_plot_data(rx.psFtn,    &plot6, BUFLEN_8KHZ);
		}

		write_plot_data(rx.psClt2wire,    &plot1, BUFLEN_8KHZ);
		write_plot_data(tx.psCltToLine,    &plot2, BUFLEN_8KHZ);
		write_plot_data(rx.psClt,    &plot3, BUFLEN_8KHZ);

		write_plot_data(tx.psFtnToLine,    &plot5, BUFLEN_8KHZ);

		speechIndex=speechIndex+BUFLEN_8KHZ;
	}
	}
	write_plot_buffer_to_file("lineIn.dat",  &plot1);
	write_plot_buffer_to_file("lineOut.dat",  &plot2);
	write_plot_buffer_to_file("lineInProcessed.dat", &plot3);

	write_plot_buffer_to_file("FtnIn.dat",  &plot4);
	write_plot_buffer_to_file("FtnOut.dat",  &plot5);
	write_plot_buffer_to_file("FtnInProcessed.dat", &plot6);

#endif //USE_DEBUG_PLOT
}

void test_all_lec(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	int i,j,k;
	int speechIndex=0;
	t_echo_handler clt_echo_handler;
	t_echo_handler ftn_echo_handler;
	t_echo_handler pilot_echo_handler;

	// Initialize time indices
	if(iFirstTime) {
		subband_init();
		plot_init();
		iFirstTime = 0;
		for(i=0; i<(N_RX + N_MIC); i++) {
			uiSineTimeIndex[i] = 0;
		}

		clt_echo_handler = createEchoHandler();
		ftn_echo_handler = createEchoHandler();
		pilot_echo_handler = createEchoHandler();

		//true_voice_module_enable(AGC, 0);
		//true_voice_module_enable(LS_LIMITER, 0);
		//true_voice_module_enable(NOISE_REDUCTION, 0);
		//true_voice_module_enable(LEC, 0);

		true_voice_module_enable(LEC, 1);
		true_voice_clt_mode(1);
	}

	for (j=0; j<2; j++) {
		memset(mic.psPilotCltCable,0,sizeof(short)*BUFLEN_8KHZ);
		memset(mic.psPilotCable,0,sizeof(short)*BUFLEN_8KHZ);
		memset(mic.psTechHeadset,0,sizeof(short)*BUFLEN_8KHZ);

		short* pMicIn = j==0 ? mic.psPilotCltCable : mic.psPilotCable;
		short* pMicIn2 = mic.psTechHeadset;

		for(i=0; i<7; i++) {
			speechIndex = 0;
			while(speechIndex < TV_PLOT_LENGTH/(2*7)) {
				for (k=0; k<BUFLEN_8KHZ; k++){
					if (k+speechIndex<11311 && i != 0){
						pMicIn[k] = j==0 ? maleSpeech[k+speechIndex] : femaleSpeech[k+speechIndex];
						pMicIn2[k] =femaleSpeech[k+speechIndex];
					}
					else{
						pMicIn[k] = 0;
						pMicIn2[k] = 0;
					}
				}

				if (i>=4) {
					create_echo(tx.psCltToLine, &femaleSpeech[speechIndex], rx.psClt2wire, true, &clt_echo_handler);
					create_echo(tx.psFtnToLine, &maleSpeech[speechIndex], rx.psFtn, true, &ftn_echo_handler);
					if (j==1) {
						short temp[BUFLEN_8KHZ];
						memcpy(temp, mic.psPilotCable, sizeof(short)*BUFLEN_8KHZ);
						create_echo(ls.psPilotMonoCable, temp, mic.psPilotCable, true, &pilot_echo_handler);
					}
				}
				else {
					create_echo(tx.psCltToLine, 0, rx.psClt2wire, false, &clt_echo_handler);
					create_echo(tx.psFtnToLine, 0, rx.psFtn, false, &ftn_echo_handler);
					if (j==1) {
						short temp[BUFLEN_8KHZ];
						memcpy(temp, mic.psPilotCable, sizeof(short)*BUFLEN_8KHZ);
						create_echo(ls.psPilotMonoCable, temp, mic.psPilotCable, true, &pilot_echo_handler);
					}
				}

				write_plot_data(rx.psFtn,    &plot5, BUFLEN_8KHZ);

				write_plot_data(ls.psPilotMonoCable,    &plot7, BUFLEN_8KHZ);
				write_plot_data(mic.psPilotCable,    &plot8, BUFLEN_8KHZ);

				true_voice(rx, mic, tx, ls);

				write_plot_data(mic.psPilotCltCable,    &plot1, BUFLEN_8KHZ);
				write_plot_data(rx.psClt2wire,    &plot2, BUFLEN_8KHZ);
				write_plot_data(rx.psClt,    &plot3, BUFLEN_8KHZ);

				write_plot_data(mic.psTechHeadset,    &plot4, BUFLEN_8KHZ);
				write_plot_data(rx.psFtn,    &plot6, BUFLEN_8KHZ);

				write_plot_data(mic.psPilotCable,    &plot9, BUFLEN_8KHZ);

				speechIndex=speechIndex+BUFLEN_8KHZ;
			}
		}
	}

	write_plot_buffer_to_file("lineIn.dat",  &plot1);
	write_plot_buffer_to_file("lineOut.dat",  &plot2);
	write_plot_buffer_to_file("lineInProcessed.dat", &plot3);

	write_plot_buffer_to_file("FtnIn.dat",  &plot4);
	write_plot_buffer_to_file("FtnOut.dat",  &plot5);
	write_plot_buffer_to_file("FtnInProcessed.dat", &plot6);

	write_plot_buffer_to_file("PilotIn.dat",  &plot7);
	write_plot_buffer_to_file("PilotOut.dat",  &plot8);
	write_plot_buffer_to_file("PilotInProcessed.dat", &plot9);

#endif //USE_DEBUG_PLOT
}

void test_mute(void) {
#if USE_DEBUG_PLOT
	static int iFirstTime = 1;
	static int counter = 0;
	int i;
	static timer_handle_t hTimer;
	static fade_handle_t hFade;
	static fade_handle_t hRise;

	// Initialize time indices
	if(iFirstTime) {
		for(i=0; i<(N_RX + N_MIC); i++) {
			uiSineTimeIndex[i] = 0;
		}
		plot_init();
		iFirstTime = 0;

		resetTimerHandle(&hTimer, 20);
		resetFadeHandle(&hFade, true);
		resetFadeHandle(&hRise, false);
	}

	while(uiSineTimeIndex[0] < TV_PLOT_LENGTH) {

		// Create sine wave for each channel with frequency 100 Hz, 200 Hz, 300 Hz, ...
		for(i=0; i<N_RX; i++) {
			sin_block(audioRx[i], (i+1)*100, &uiSineTimeIndex[i]);
		}
		for(i=0; i<N_MIC; i++) {
			sin_block(audioMic[i], (i+1+N_RX)*100, &uiSineTimeIndex[i+N_RX]);
		}

		q15* usedFadeBuffer = audioRx[0];
		q15* usedRiseBuffer = audioRx[1];

		bool startFading = isFinished(&hTimer);
		fade(!startFading, usedFadeBuffer, &hFade);
		fade(startFading, usedRiseBuffer, &hRise);

		// Store data (see plot.c in true_voice)
		write_plot_data(usedFadeBuffer, &plot1, BUFLEN_8KHZ);
		write_plot_data(usedRiseBuffer, &plot2, BUFLEN_8KHZ);
	}

	// Write stored data to output file
	write_plot_buffer_to_file("muted.dat", &plot1);
	write_plot_buffer_to_file("unmuted.dat", &plot2);

#endif //USE_DEBUG_PLOT
}


//void fillArrayFromFile(q15 *data, char filename[100]){
//	fp = fopen(filename, "r"); // Open file for dumping
//	for(ui=0; ui<TV_PLOT_LENGTH; ui++) {
//		fprintf(fp, "%d\n", psTemp[ui]);
//	}
//	fclose(fp);
//}

/*
simulate_capture_audio(){
	in(1)[i]=malespeech[index]

}*/
