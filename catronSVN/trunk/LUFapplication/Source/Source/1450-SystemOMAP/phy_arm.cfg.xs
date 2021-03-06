/* --COPYRIGHT--,BSD
 * Copyright (c) $(CPYYEAR), Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//var BIOS = xdc.useModule('ti.sysbios.BIOS'); 

var MultiProc = xdc.useModule('ti.sdo.utils.MultiProc');
var HeapMem = xdc.useModule('ti.sysbios.heaps.HeapMem');
var HeapBuf = xdc.useModule('ti.sysbios.heaps.HeapBuf');
//var HeapBufMP = xdc.useModule('ti.sdo.ipc.heaps.HeapBufMP');
var Task = xdc.useModule('ti.sysbios.knl.Task');
var Mailbox = xdc.useModule('ti.sysbios.knl.Mailbox');
var Swi = xdc.useModule('ti.sysbios.knl.Swi');
var BIOS = xdc.useModule('ti.sysbios.BIOS');
var Timer = xdc.useModule('ti.sysbios.hal.Timer');
var SemaphorePhy = xdc.useModule('ti.sysbios.knl.Semaphore');
//var Mmu = xdc.useModule('ti.sysbios.family.arm.arm9.Mmu');
//MultiProc.setConfig("HOST", ["HOST", "DSP"]);

/* Load the common configuration */
//xdc.loadCapsule("message_common.cfg.xs");
/*var heapBuf0Params = new HeapBuf.Params();
heapBuf0Params.instance.name = "heapBuf0";
heapBuf0Params.blockSize = 128;
heapBuf0Params.numBlocks = 10;
Program.global.heapBuf0 = HeapBuf.create(heapBuf0Params);
//HeapBufMP.maxRuntimeEntries = 10;*/

var timer1Params = new Timer.Params();

timer1Params.instance.name = "TXTimer";
timer1Params.startMode = xdc.module("ti.sysbios.interfaces.ITimer").StartMode_USER;
timer1Params.runMode = xdc.module("ti.sysbios.interfaces.ITimer").RunMode_ONESHOT;
timer1Params.period = 1000;
Program.global.TXTimer = Timer.create(1, "&SendTxTimerIsr", timer1Params);
var task1Params = new Task.Params();
task1Params.instance.name = "task1";
//Program.global.task1 = Task.create("&taskStub", task1Params);
var swi1Params = new Swi.Params();
swi1Params.instance.name = "trace_flush_swi_hdl";
swi1Params.priority = 1;
swi1Params.trigger = 1;
Program.global.trace_flush_swi_hdl = Swi.create("&trace_flush_swi", swi1Params);
var semaphore0Params = new SemaphorePhy.Params();
semaphore0Params.instance.name = "radioSemaphore0";
semaphore0Params.mode = SemaphorePhy.Mode_BINARY;
Program.global.radioSemaphore0 = SemaphorePhy.create(null, semaphore0Params);
var task2Params = new Task.Params();
task2Params.instance.name = "radiotask";
task2Params.priority = 10;
Program.global.radiotask = Task.create("&radioTskFnc", task2Params);
/*
var semaphore1Params = new SemaphorePhy.Params();
semaphore1Params.instance.name = "traceSem";
Program.global.traceSem = SemaphorePhy.create(null, semaphore1Params);
/*
// Force peripheral section to be NON cacheable 
 var peripheralAttrs = { type : Mmu.FirstLevelDesc_SECTION, // SECTION descriptor 
 bufferable : false, // bufferable 
 cacheable : false, // cacheable 
 };

 // Define the base address of the 1 Meg page 
 // the peripheral resides in.  
 var peripheralBaseAddr = 0x01d0d000; /* replace with the 1 meg page address of your peripheral */  

 // Configure the corresponding MMU page descriptor accordingly 
 //Mmu.setFirstLevelDescMeta(peripheralBaseAddr, peripheralBaseAddr, peripheralAttrs); 
var timer1Params0 = new Timer.Params();
timer1Params0.instance.name = "slotTimer";
timer1Params0.period = 2000;
timer1Params0.startMode = xdc.module("ti.sysbios.interfaces.ITimer").StartMode_USER;
Program.global.slotTimer = Timer.create(0, "&myIsr", timer1Params0);
var timer2Params = new Timer.Params();
timer2Params.instance.name = "FrameTimer";
timer2Params.startMode = xdc.module("ti.sysbios.interfaces.ITimer").StartMode_USER;
timer2Params.period = 20000;
Program.global.FrameTimer = Timer.create(3, "&frameTimerTimeoutMaster", timer2Params);
var timer3Params = new Timer.Params();
timer3Params.instance.name = "RadioDelay";
timer3Params.startMode = xdc.module("ti.sysbios.interfaces.ITimer").StartMode_USER;
timer3Params.period = 50;
Program.global.RadioDelay = Timer.create(2, "&radiodelay", timer3Params);
var semaphorePhy1Params = new SemaphorePhy.Params();
semaphorePhy1Params.instance.name = "radiosem";
semaphorePhy1Params.mode = SemaphorePhy.Mode_BINARY;
Program.global.radiosem = SemaphorePhy.create(null, semaphorePhy1Params);
