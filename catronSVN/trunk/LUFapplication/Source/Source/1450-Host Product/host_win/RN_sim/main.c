/*
    FreeRTOS V7.1.1 - Copyright (C) 2012 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!
    
    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?                                      *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    
    http://www.FreeRTOS.org - Documentation, training, latest information, 
    license and contact details.
    
    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell 
    the code with commercial support, indemnification, and middleware, under 
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under 
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

/*
 ******************************************************************************
 * -NOTE- The Win32 port is a simulation (or is that emulation?) only!  Do not
 * expect to get real time behaviour from the Win32 port or this demo
 * application.  It is provided as a convenient development and demonstration
 * test bed only.  This was tested using Windows XP on a dual core laptop.
 *
 * In this example, one simulated millisecond will take approximately 40ms to
 * execute, and the timing information in the FreeRTOS+Trace logs have no
 * meaningful units.  See the documentation page for the Windows simulator for
 * an explanation of the slow timing:
 * http://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
 ******************************************************************************
 *
 * This is a simple FreeRTOS Windows simulator project that makes it easy to 
 * evaluate FreeRTOS+CLI and FreeRTOS+Trace on a standard desktop PC, without
 * any external hardware or interfaces being required.
 *
 * To keep everything as simple as possible, the command line interface is 
 * accessed through a UDP socket on the default Windows loopback IP address of
 * 127.0.0.1.  Full instructions are provided on the documentation page
 * referenced above.
 *
 * Commands are provided to both start and stop a FreeRTOS+Trace recording.  
 * Stopping a recording will result in the recorded data being saved to the
 * hard disk, ready for viewing in the FreeRTOS+Trace graphical user interface.
 * Again, full instructions are provided on the documentation page referenced
 * above.
 *
 * A queue send task and a queue receive task are defined in this file.  The
 * queue receive task spends most of its time blocked on the queue waiting for
 * messages to arrive.  The queue send task periodically sends a message to the
 * queue, causing the queue receive task to exit the Blocked state.  The
 * priority of the queue receive task is above that of the queue send task, so
 * it pre-empts the queue send task as soon as it leaves the Blocked state.  It
 * then consumes the message from the queue and prints "message received" to
 * the screen before returning to block on the queue once again.  This 
 * sequencing is clearly visible in the recorded FreeRTOS+Trace data.
 *
 * Finally, a trace monitoring task is also created that prints out a message
 * when it determines that the status of the trace has changed since it last
 * executed.  It prints out a message when the trace has started, when the
 * trace has stopped, and periodically when the trace is executing.
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <stdint.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"

/* FreeRTOS+Trace includes. */
#include "trcUser.h"

#include "rn_manager.h"
#include "phy_sim.h"
#include "ap_sim.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainUDP_CLI_TASK_PRIORITY			( tskIDLE_PRIORITY )

/* The rate at which data is sent to the queue.  The (simulated) 50ms value is 
converted to ticks using the portTICK_RATE_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS			( 50 / portTICK_RATE_MS )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH					( 1 )

/*-----------------------------------------------------------*/

/*
 * The queue send and receive tasks as described in the comments at the top of
 * this file. 
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/*
 * The task that implements the UDP command interpreter using FreeRTOS+CLI.
 */
extern void vUDPCommandInterpreterTask( void *pvParameters );

/*
 * Register commands that can be used with FreeRTOS+CLI through the UDP socket.
 * The commands are defined in CLI-commands.c.
 */
extern void vRegisterCLICommands( void );

/* The queue used by both tasks. */
static xQueueHandle xQueue = NULL;

#define PHY_DOWNLINK_1          (5010)
#define PHY_UPLINK_1            (5011)    
#define CONSOLE_IN_PORT_1       (5001)
#define CONSOLE_OUT_PORT_1      (5002)
#define CONSOLE_IN_PORT_2       (5003)
#define CONSOLE_OUT_PORT_2      (5004)

uint16_t    configured_console_in_port = CONSOLE_IN_PORT_1;
uint16_t    configured_console_out_port = CONSOLE_OUT_PORT_1;

//extern uint16_t         phy_send_port = PHY_DOWNLINK_1;
//extern uint16_t         phy_receive_port = PHY_UPLINK_1;


/*-----------------------------------------------------------*/

int main(int argc,      // Number of strings in array argv
    char *argv[])
{
    const uint32_t ulLongTime_ms = 250UL;
    
    printf("%d %s \n\r",argc, argv[1]);

    if( 2 == argc)
    {
        if (0 == strcmp(argv[1],"0"))
        {
            rn_phy_send_port    (PHY_DOWNLINK_1);
            rn_phy_receive_port (PHY_UPLINK_1);

            configured_console_in_port = CONSOLE_IN_PORT_1;
            configured_console_out_port = CONSOLE_OUT_PORT_1;
        }
        else if (0 == strcmp(argv[1],"1"))
        {
            rn_phy_send_port    (PHY_UPLINK_1);
            rn_phy_receive_port (PHY_DOWNLINK_1);

            configured_console_in_port = CONSOLE_IN_PORT_2;
            configured_console_out_port = CONSOLE_OUT_PORT_2;
        }
    }

    printf("Command port %d \n\r",configured_console_in_port);

    /* Create the queue used to pass messages from the queue send task to the
    queue receive task. */
    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( unsigned long ) );

    /* Give the queue a name for the FreeRTOS+Trace log. */
	vTraceSetQueueName( xQueue, "DemoQ" );

	/* Start the two tasks as described in the comments at the top of this
	file. */
	xTaskCreate( prvQueueReceiveTask,				/* The function that implements the task. */
				( signed char * ) "Rx", 			/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task.  Not actually used as a stack in the Win32 simulator port. */
				NULL,								/* The parameter passed to the task - not used in this example. */
				mainQUEUE_RECEIVE_TASK_PRIORITY, 	/* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

	xTaskCreate( prvQueueSendTask, ( signed char * ) "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

	/* Create the task that handles the CLI on a UDP port.  The port number
	is set using the configUDP_CLI_PORT_NUMBER setting in FreeRTOSConfig.h. */
	xTaskCreate( vUDPCommandInterpreterTask, ( signed char * ) "CLI", configMINIMAL_STACK_SIZE, NULL, mainUDP_CLI_TASK_PRIORITY, NULL );


	/* Create the task that monitors the trace recording status, printing
	periodic information to the display. */
	vTraceStartStatusMonitor();

    rn_freertos_init();
    rn_freertos_start(1);

    rn_module_init();
    rn_module_start(1);
  
    ap_sim_module_init();
    ap_sim_module_start(1);

	/* Register commands with the FreeRTOS+CLI command interpreter. */
	vRegisterCLICommands();

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details (this is standard text that is not not 
	really applicable to the Win32 simulator port). */
	for( ;; )
	{
		Sleep( ulLongTime_ms );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
portTickType xNextWakeTime;
const unsigned long ulValueToSend = 100UL;

	/* Remove warning about unused parameters. */
	( void ) pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again.
		While in the Blocked state this task will not consume any CPU time. */
		vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to unblock and
		write a message to the display.  0 is used as the block time so the 
		sending operation will not block - it shouldn't need to block as the 
		queue should always	be empty at this point in the code, and it is an
		error if it is not. */
		xQueueSend( xQueue, &ulValueToSend, 0U );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
unsigned long ulReceivedValue;

	/* Remove warning about unused parameters. */
	( void ) pvParameters;

	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

		/*  To get here something must have been received from the queue, but
		is it the expected value?  If it is, write the message to the 
		display before looping back to block on the queue again. */
		if( ulReceivedValue == 100UL )
		{
			//printf( "Message received!\r\n" );
			ulReceivedValue = 0U;
		}
	}
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
const unsigned long ulMSToSleep = 5;

	/* This function is called on each cycle of the idle task if
	configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h.  Sleep to reduce CPU 
	load. */
	Sleep( ulMSToSleep );
}
/*-----------------------------------------------------------*/

void vAssertCalled( void )
{
const unsigned long ulLongSleep = 1000UL;

	taskDISABLE_INTERRUPTS();
	for( ;; )
	{
		Sleep( ulLongSleep );
	}
}

/*-----------------------------------------------------------*/
