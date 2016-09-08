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
* Filename      : ttcp.h
* Version       : V1.90
* Programmer(s) : CL
*                 JDH
*                 ITJ
*********************************************************************************************************
* Note(s)       : This application is a Micrium implementation of a publically available TCP test tool: TTCP.
*                 This implementation uses uC/OS-II and uC/TCP-IP.
*                 The tool was adapted to Micrium uC/TCP-IP and coding standards.
**********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               TTCP present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  TTCP_PRESENT                                           /* See Note #1.                                         */
#define  TTCP_PRESENT


/*
*********************************************************************************************************
*                                          TTCP VERSION NUMBER
*
* Note(s) : (1) (a) The TTCP software version is denoted as follows :
*
*                       Vx.yy
*
*                           where
*                                  V     denotes 'Version' label
*                                  x     denotes major software version revision number
*                                  yy    denotes minor software version revision number
*
*               (b) The TTCP software version label #define is formatted as follows :
*
*                       ver = x.yy * 100
*
*                           where
*                                  ver   denotes software version number scaled as
*                                        an integer value
*                                  x.yy  denotes software version number
*********************************************************************************************************
*/

#define  TTCP_VERSION               189u                        /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   TTCP_MODULE
#define  TTCP_EXT
#else
#define  TTCP_EXT  extern
#endif


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

CPU_BOOLEAN  TTCP_Init (void);
void         TTCP_Task (void  *p_arg);


/*
*********************************************************************************************************
*                                       RTOS INTERFACE FUNCTIONS
*                                            (see ttcp_os.c)
*********************************************************************************************************
*/

CPU_BOOLEAN  TTCP_OS_Init (void  *p_arg);


/*
*********************************************************************************************************
*                                              TRACING
*********************************************************************************************************
*/

                                                                /* Trace level, default to TRACE_LEVEL_OFF              */
#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                                 0
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                                1
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                                 2
#endif

#ifndef  TTCP_TRACE_LEVEL
#define  TTCP_TRACE_LEVEL                       TRACE_LEVEL_OFF
#endif

#ifndef  TTCP_TRACE
#define  TTCP_TRACE                             printf
#endif

#define  TTCP_TRACE_INFO(x)                   ((TTCP_TRACE_LEVEL >= TRACE_LEVEL_INFO) ? (void)(TTCP_TRACE x) : (void)0)
#define  TTCP_TRACE_DBG(x)                    ((TTCP_TRACE_LEVEL >= TRACE_LEVEL_DBG)  ? (void)(TTCP_TRACE x) : (void)0)


/*
*********************************************************************************************************
*                                         CONFIGURATION ERRORS
*********************************************************************************************************
*/

                                                                /* Task name for debugging purposes.                    */
#ifndef  TTCP_OS_CFG_TASK_NAME
#error  "TTCP_OS_CFG_TASK_NAME                  illegally #define'd in 'app_cfg.h'"
#error  "                                       see template file in package      "
#error  "                                       named 'ttcp_cfg.h'                "
#endif

                                                                /* Task priority.                                       */
#ifndef  TTCP_OS_CFG_TASK_PRIO
#error  "TTCP_OS_CFG_TASK_PRIO                  illegally #define'd in 'app_cfg.h'"
#error  "                                       see template file in package      "
#error  "                                       named 'ttcp_cfg.h'                "
#endif

                                                                /* Task stack size.                                     */
#ifndef  TTCP_OS_CFG_TASK_STK_SIZE
#error  "TTCP_OS_CFG_TASK_STK_SIZE              illegally #define'd in 'app_cfg.h'"
#error  "                                       see template file in package      "
#error  "                                       named 'ttcp_cfg.h'                "
#endif

                                                                /* Maximum inactivity time (ms) on ACCEPT.              */
#ifndef  TTCP_CFG_MAX_ACCEPT_TIMEOUT_MS
#error  "TTCP_CFG_MAX_ACCEPT_TIMEOUT_MS         illegally #define'd in 'app_cfg.h'"
#error  "                                       see template file in package      "
#error  "                                       named 'ttcp_cfg.h'                "
#endif

                                                                /* Maximum inactivity time (ms) on CONNECT.             */
#ifndef  TTCP_CFG_MAX_CONN_TIMEOUT_MS
#error  "TTCP_CFG_MAX_CONN_TIMEOUT_MS           illegally #define'd in 'app_cfg.h'"
#error  "                                       see template file in package      "
#error  "                                       named 'ttcp_cfg.h'                "
#endif

                                                                /* Maximum inactivity time (ms) on RX.                  */
#ifndef  TTCP_CFG_MAX_RX_TIMEOUT_MS
#error  "TTCP_CFG_MAX_RX_TIMEOUT_MS             illegally #define'd in 'app_cfg.h'"
#error  "                                       see template file in package      "
#error  "                                       named 'ttcp_cfg.h'                "
#endif


/*
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of TTCP module include.                          */
