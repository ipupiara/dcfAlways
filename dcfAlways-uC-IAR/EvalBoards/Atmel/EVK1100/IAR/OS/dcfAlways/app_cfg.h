/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                            ATMEL  AVR32 UC3  Application Configuration File
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
*  originally
* File    : APP.C
* By      : Fabiano Kovalski
*
* extended
* By      :  Peter Niederer
*
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE CONFIGURATION
*********************************************************************************************************
*/

#include <ttcp_cfg.h>


/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  uC_TCPIP_MODULE                 DEF_ENABLED                    /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
//#define  uC_TCPIP_MODULE                 DEF_DISABLED                    /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */

#define  uC_TTCP_MODULE                  DEF_DISABLED                    /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
//#warning might be changed
/*
**************************************************************************************************************
*                                           STACK SIZES
**************************************************************************************************************
*/

/*
**************************************************************************************************************
*                                               STACK SIZES
**************************************************************************************************************
*/

//#define unique_STK_SIZE          2048
#define unique_STK_SIZE          256

#define  CalcControllerMethod_STK_SIZE          unique_STK_SIZE
#define  Sec100CalculatorMethod_STK_SIZE          unique_STK_SIZE
#define  ScoreCalculatorMethod_STK_SIZE          unique_STK_SIZE
#define  SerialQMethod_STK_SIZE          unique_STK_SIZE
#define  tcp_ip_Thread_Method_STK_SIZE          unique_STK_SIZE
#define  rtcPid_Thread_Method_STK_SIZE          unique_STK_SIZE

#define  NET_OS_CFG_TMR_TASK_STK_SIZE        172
#define  NET_OS_CFG_IF_RX_TASK_STK_SIZE      384

/*
**************************************************************************************************************
*                                             TASK PRIORITIES
**************************************************************************************************************
*/



#define  OS_TASK_TMR_PRIO                   5
#define  SerialQ_TASK_PRIO                   6

#define  CalculationController_TASK_PRIO     10
#define  Sec100Calculator_TASK_PRIO          11
#define tcp_ip_Thread_TASK_PRIO              12
#define  ScoreCalculator_TASK_PRIO           13
#define  rtcPid_TASK_PRIO                    14

#define  NET_OS_CFG_IF_RX_TASK_PRIO            20   // lower prio as recommended in tcp-ip manuals, pn 21 aug 12
#define  NET_OS_CFG_TMR_TASK_PRIO             21

//#warning might be changed .. check amount processed and prios for all processes in application -integraton of net processes


#define BOARD EVK1100

