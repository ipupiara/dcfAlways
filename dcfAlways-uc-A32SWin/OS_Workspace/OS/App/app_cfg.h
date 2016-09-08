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
**************************************************************************************************************
*                                               STACK SIZES
**************************************************************************************************************
*/

#define unique_STK_SIZE          2048

#define  CalcControllerMethod_STK_SIZE          unique_STK_SIZE
#define  Sec100CalculatorMethod_STK_SIZE          unique_STK_SIZE
#define  ScoreCalculatorMethod_STK_SIZE          unique_STK_SIZE
#define  SerialQMethod_STK_SIZE          unique_STK_SIZE
/*
**************************************************************************************************************
*                                             TASK PRIORITIES
**************************************************************************************************************
*/

#define  CalculationController_TASK_PRIO     7
#define  Sec100Calculator_TASK_PRIO          8
#define  ScoreCalculator_TASK_PRIO           9
#define  SerialQ_TASK_PRIO                   6

#define  OS_TASK_TMR_PRIO                   5

#define BOARD EVK1100

