/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                     ATMEL  AVR32 UC3 Specific code
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : INCLUDES.H
* By      : Fabiano Kovalski
*********************************************************************************************************
*/

#include      <stdio.h>
#include      <stdarg.h>

#include      <cpu.h>
#include      <app_cfg.h>
#include      <ucos_ii.h>

#include      <lib_def.h>
#include      <lib_mem.h>
#include      <lib_str.h>

#include      <bsp.h>

#include      <avr32/io.h>

#if  uC_TCPIP_MODULE > 0
    #include  <net_cfg.h>
    #include  <net.h>
    #include  <net_bsp.h>
#endif

#if  uC_TTCP_MODULE > 0
    #include  <ttcp_cfg.h>
    #include  <ttcp.h>
#endif

#include      <pdca.h>
#include      <sdramc.h>
#include      <conf_usb.h>
#include      <appl_usb.h>





