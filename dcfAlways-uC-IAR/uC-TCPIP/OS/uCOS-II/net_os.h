/*
*********************************************************************************************************
*                                              uC/TCP-IP
*                                      The Embedded TCP/IP Suite
*
*                          (c) Copyright 2003-2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/TCP-IP is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/TCP-IP in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to help
*               you experience uC/TCP-IP.  The fact that the source code is provided does
*               NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   NETWORK OPERATING SYSTEM LAYER
*
*                                          Micrium uC/OS-II
*
* Filename      : net_os.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
* Note(s)       : (1) Assumes uC/OS-II V2.84 is included in the product build.
*
*                 (2) REQUIREs the following uC/OS-II features to be ENABLED :
*
*                         ------- FEATURE --------    ---------- MINIMUM CONFIGURATION FOR NET/OS PORT -----------
*
*                     (a) OS Events                   OS_MAX_EVENTS >= NET_OS_NBR_EVENTS (see 'OS OBJECT DEFINES')
*
*                     (b) Semaphores                                   NET_OS_NBR_SEM    (see Note #2a)
*                         (1) OS_SEM_EN                   Enabled
*                         (2) OS_SEM_SET_EN               Enabled
*                         (3) OS_SEM_PEND_ABORT_EN        Enabled
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <ucos_ii.h>                                           /* See this 'net_os.h  Note #1'.                        */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_OS_MODULE
#define  NET_OS_EXT
#else
#define  NET_OS_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                     OS TASK/OBJECT NAME DEFINES
*********************************************************************************************************
*/

                                                                /* -------------------- TASK NAMES -------------------- */
#define  NET_TMR_TASK_NAME                  "Net Timer Task"
#define  NET_IF_RX_TASK_NAME                "Net IF Rx Task"


#define  NET_TMR_TASK_NAME_SIZE                  (sizeof(NET_TMR_TASK_NAME))
#define  NET_IF_RX_TASK_NAME_SIZE                (sizeof(NET_IF_RX_TASK_NAME))

#define  NET_TASK_NAME_SIZE_MAX                           15    /* Max of ALL net task name sizes.                      */



                                                                /* -------------------- OBJ NAMES --------------------- */
#define  NET_INIT_NAME                      "Net Init Signal"
#define  NET_LOCK_NAME                      "Net Global Lock"
#define  NET_TX_SUSPEND_NAME                "Net Tx Suspend"

#define  NET_NIC_TX_RDY_NAME                "Net NIC Tx Rdy"

#define  NET_IF_RX_Q_NAME                   "Net IF Rx Queue"



#define  NET_INIT_NAME_SIZE                      (sizeof(NET_INIT_NAME))
#define  NET_LOCK_NAME_SIZE                      (sizeof(NET_LOCK_NAME))
#define  NET_TX_SUSPEND_NAME_SIZE                (sizeof(NET_TX_SUSPEND_NAME))

#define  NET_NIC_TX_RDY_NAME_SIZE                (sizeof(NET_NIC_TX_RDY_NAME))

#define  NET_IF_RX_Q_NAME_SIZE                   (sizeof(NET_IF_RX_Q_NAME))


#define  NET_OBJ_NAME_SIZE_MAX                            16    /* Max of ALL net obj name sizes.                        */


/*
*********************************************************************************************************
*                                          OS OBJECT DEFINES
*********************************************************************************************************
*/

#define  NET_OS_NBR_SEM_NET_INIT                           1
#define  NET_OS_NBR_SEM_NET_LOCK                           1

#if     (NET_CFG_LOAD_BAL_EN == DEF_ENABLED)
#define  NET_OS_NBR_SEM_NET_TX_SUSPEND                     1
#else
#define  NET_OS_NBR_SEM_NET_TX_SUSPEND                     0
#endif

#define  NET_OS_NBR_SEM_NET                             (NET_OS_NBR_SEM_NET_INIT      + \
                                                         NET_OS_NBR_SEM_NET_LOCK      + \
                                                         NET_OS_NBR_SEM_NET_TX_SUSPEND)


#define  NET_OS_NBR_SEM_NIC                                1
#define  NET_OS_NBR_SEM_IF                                 1
#define  NET_OS_NBR_SEM_TCP                               (2 * NET_TCP_CFG_NBR_CONN )
#define  NET_OS_NBR_SEM_SOCK                              (4 * NET_SOCK_CFG_NBR_SOCK)


#define  NET_OS_NBR_SEM                                 (NET_OS_NBR_SEM_NET + \
                                                         NET_OS_NBR_SEM_NIC + \
                                                         NET_OS_NBR_SEM_IF  + \
                                                         NET_OS_NBR_SEM_TCP + \
                                                         NET_OS_NBR_SEM_SOCK)

#define  NET_OS_NBR_EVENTS                             NET_OS_NBR_SEM          // NET_OS_NBR_SEM
                              

/*$PAGE*/
/*
*********************************************************************************************************
*                                         OS TIMEOUT DEFINES
*********************************************************************************************************
*/

#define  NET_OS_TIMEOUT_MIN_TICK                           1uL
#define  NET_OS_TIMEOUT_MAX_TICK                         DEF_INT_16U_MAX_VAL

#define  NET_OS_TIMEOUT_MIN_MS                             0uL
#if     (OS_TICKS_PER_SEC > 0)
#define  NET_OS_TIMEOUT_MAX_MS                         ((NET_OS_TIMEOUT_MAX_TICK * DEF_TIME_NBR_mS_PER_SEC) / OS_TICKS_PER_SEC)
#else
#define  NET_OS_TIMEOUT_MAX_MS                             0uL
#endif

#define  NET_OS_TIMEOUT_MIN_SEC                         (NET_OS_TIMEOUT_MIN_MS   / DEF_TIME_NBR_mS_PER_SEC)
#define  NET_OS_TIMEOUT_MAX_SEC                         (NET_OS_TIMEOUT_MAX_MS   / DEF_TIME_NBR_mS_PER_SEC)


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if    ((OS_TASK_NAME_SIZE >                      0) && \
        (OS_TASK_NAME_SIZE < NET_TASK_NAME_SIZE_MAX))
#error  "OS_TASK_NAME_SIZE      illegally #define'd in 'os_cfg.h'   "
#error  "                       [MUST be  <                       1]"
#error  "                       [     ||  >= NET_TASK_NAME_SIZE_MAX]"
#endif



#if     (NET_TMR_CFG_TASK_FREQ > OS_TICKS_PER_SEC)
#error  "NET_TMR_CFG_TASK_FREQ  illegally #define'd in 'net_cfg.h'"
#error  "                       [MUST be  < OS_TICKS_PER_SEC]     "
#endif




#if    ((OS_EVENT_NAME_SIZE >                      0) && \
        (OS_EVENT_NAME_SIZE < NET_OBJ_NAME_SIZE_MAX))
#error  "OS_EVENT_NAME_SIZE     illegally #define'd in 'os_cfg.h'  "
#error  "                       [MUST be  <                      1]"
#error  "                       [     ||  >= NET_OBJ_NAME_SIZE_MAX]"
#endif


#if     (OS_MAX_EVENTS < NET_OS_NBR_EVENTS)
#error  "OS_MAX_EVENTS          illegally #define'd in 'os_cfg.h'    "
#error  "                       [MUST be  >= NET_OS_NBR_EVENTS      ]"
#error  "                       [(see 'net_os.h  OS OBJECT DEFINES')]"
#endif



#if     (OS_SEM_EN != 1)
#error  "OS_SEM_EN              illegally #define'd in 'os_cfg.h'"
#error  "                       [MUST be  1]                     "
#endif

