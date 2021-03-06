/*****************************************************************************
* linker command file for OMAP-L138 test code.
*
* © Copyright 2009, Logic Product Development Company. All Rights Reserved.
******************************************************************************/

-stack           0x00002500
-heap            0x00000800

MEMORY
{
   dsp_l2_ram:      ORIGIN = 0x11800000  LENGTH = 0x00040000
   shared_ram:      ORIGIN = 0x80000000  LENGTH = 0x00020000
   external_ram:    ORIGIN = 0xC0000000  LENGTH = 0x08000000
   arm_local_ram:   ORIGIN = 0xFFFF0000  LENGTH = 0x00002000
}

SECTIONS
{
   .text       > dsp_l2_ram
   .const      > shared_ram
   .bss        > shared_ram
   .far        > dsp_l2_ram
   .switch     > shared_ram
   .stack      > dsp_l2_ram
   .data       > shared_ram
   .cinit      > external_ram
   .sysmem     > shared_ram//TODO: change back to shared_ram
   .cio        > shared_ram
    extvar	   > external_ram
}

