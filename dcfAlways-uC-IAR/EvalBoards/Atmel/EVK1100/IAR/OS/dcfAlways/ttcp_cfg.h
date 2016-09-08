/*
*********************************************************************************************************
*                                                uC/TTCP
*                                  TCP-IP Transfer Measurement Utility
*
*                          (c) Copyright 2003-2007; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                  TCP-IP TRANSFER MEASUREMENT UTILITY
*
*                                      CONFIGURATION TEMPLATE FILE
*
* Filename      : ttcp_cfg.h
* Version       : V1.90
* Programmer(s) : JDH
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/

#define  TTCP_OS_CFG_TASK_NAME                  "TTCP"


/*
*********************************************************************************************************
*                                           TASKS PRIORITIES
*********************************************************************************************************
*/

#define  TTCP_OS_CFG_TASK_PRIO                  (OS_LOWEST_PRIO - 2)


/*
*********************************************************************************************************
*                                              STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  TTCP_OS_CFG_TASK_STK_SIZE                       448


/*
*********************************************************************************************************
*                                                 TTCP
*********************************************************************************************************
*/

#define  TTCP_CFG_MAX_ACCEPT_TIMEOUT_MS         NET_TMR_TIME_INFINITE   /* Maximum inactivity time (ms) on ACCEPT.              */
#define  TTCP_CFG_MAX_CONN_TIMEOUT_MS                   5000            /* Maximum inactivity time (ms) on CONNECT.             */
#define  TTCP_CFG_MAX_RX_TIMEOUT_MS                     5000            /* Maximum inactivity time (ms) on RX.                  */

#define  TTCP_CFG_BUF_LEN                               2048            /* Buffer length.                                       */


/*
*********************************************************************************************************
*                                                TERMINAL
*********************************************************************************************************
*/

#define  TTCP_UART_0                                       0
#define  TTCP_UART_1                                       1
#define  TTCP_UART_2                                       2
#define  TTCP_UART_3                                       3

#define  TTCP_COMM_SEL                          TTCP_UART_0


/*
*********************************************************************************************************
*                                                TRACING
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                                   0
#define  TRACE_LEVEL_INFO                                  1
#define  TRACE_LEVEL_DBG                                   2

#define  TTCP_TRACE_LEVEL                       TRACE_LEVEL_DBG
#define  TTCP_TRACE(...)                        BSP_USART_printf(TTCP_COMM_SEL, __VA_ARGS__)

#define  Ser_RdStr(...)                         BSP_USART_StrRd(TTCP_COMM_SEL,  __VA_ARGS__)
