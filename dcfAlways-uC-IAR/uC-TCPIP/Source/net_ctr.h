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
*                                      NETWORK COUNTER MANAGEMENT
*
* Filename      : net_ctr.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_CTR_MODULE
#define  NET_CTR_EXT
#else
#define  NET_CTR_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       NETWORK COUNTER MACRO'S
*
* Description : Increment network counter(s).
*
* Argument(s) : Various network counter variable(s) to increment.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               These macro's are INTERNAL network protocol suite macro's & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (1) ALL functions which call network counter macro's MUST declare local variable 'cpu_sr' if
*                   critical section method is configured as CPU_CRITICAL_METHOD_STATUS_LOCAL :
*
*                       #if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
*                            CPU_SR  cpu_sr;
*                       #endif
*
*                   (a) #### Macro's require critical section for exclusive access?
*********************************************************************************************************
*/

#define  NET_CTR_INC(ctr)                                       { CPU_CRITICAL_ENTER(); \
                                                                 (ctr)++;               \
                                                                  CPU_CRITICAL_EXIT();  }

#define  NET_CTR_INC_LARGE(ctr_hi, ctr_lo)                      { CPU_CRITICAL_ENTER(); \
                                                                 (ctr_lo)++;            \
                                                                  if ((ctr_lo) == 0) {  \
                                                                      (ctr_hi)++;       \
                                                                  }                     \
                                                                  CPU_CRITICAL_EXIT();  }


#if (NET_CTR_CFG_STAT_EN == DEF_ENABLED)
#define  NET_CTR_STAT_INC(stat_ctr)                             { NET_CTR_INC(stat_ctr);                       }
#define  NET_CTR_STAT_INC_LARGE(stat_ctr_hi, stat_ctr_lo)       { NET_CTR_INC_LARGE(stat_ctr_hi, stat_ctr_lo); }
#else
#define  NET_CTR_STAT_INC(stat_ctr)
#define  NET_CTR_STAT_INC_LARGE(stat_ctr_hi, stat_ctr_lo)
#endif


#if (NET_CTR_CFG_ERR_EN == DEF_ENABLED)
#define  NET_CTR_ERR_INC(err_ctr)                               { NET_CTR_INC(err_ctr);                        }
#define  NET_CTR_ERR_INC_LARGE(err_ctr_hi, err_ctr_lo)          { NET_CTR_INC_LARGE(err_ctr_hi,  err_ctr_lo);  }
#else
#define  NET_CTR_ERR_INC(err_ctr)
#define  NET_CTR_ERR_INC_LARGE(err_ctr_hi, err_ctr_lo)
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      NETWORK COUNTER DATA TYPE
*
* Note(s) : (1) NET_CTR_MAX  SHOULD be globally #define'd AFTER 'NET_CTR' data type declared.
*
*           (2) NET_CTR_PCT_NUMER_MAX_TH, NET_CTR_BIT_LO, & NET_CTR_BIT_HI  MUST be globally #define'd
*               AFTER 'NET_CTR' data type declared.
*
*           (3) NET_CTR_PCT_NUMER_MAX_TH  #define's the maximum value for a counter that is used as a 
*               numerator in a percentage calculation.  This threshold is required since the numerator in
*               a percentage calculation is multiplied by 100 (%) BEFORE division with the denominator :
*
*                                    Numerator * 100%
*                   Percentage (%) = ----------------
*                                      Denominator
*
*               Therefore, the numerator MUST be constrained by this threshold to prevent integer overflow
*               from the multiplication BEFORE the division.
*
*               (a) The percentage threshold value is calculated as follows :
*
*                                                N  
*                                               2  
*                       Percentage Threshold = ---
*                                              100
*
*               (b) To avoid macro integer overflow, the threshold value is modified by one less "divide-by-2"
*                   left-shift compensated by dividing the numerator by 50, instead of 100 :
*
*                                               N-1     N
*                                              2       2  / 2
*                       Percentage Threshold = ---- = -------
*                                               50    100 / 2
*********************************************************************************************************
*/
                                                                
typedef  CPU_INT32U  NET_CTR;                                   /* Defines   max nbr of errs/stats to cnt.              */

#define  NET_CTR_MAX                        DEF_INT_32U_MAX_VAL /* Define as max unsigned val (see Note #1).            */


#define  NET_CTR_PCT_NUMER_MAX_TH        (((NET_CTR)1 << ((sizeof(NET_CTR) * DEF_OCTET_NBR_BITS) - 1)) / (NET_CTR)50)

#define  NET_CTR_BIT_LO                   ((NET_CTR)DEF_BIT_00)
#define  NET_CTR_BIT_HI                   ((NET_CTR)1 << ((sizeof(NET_CTR) * DEF_OCTET_NBR_BITS) - 1))


/*$PAGE*/
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

CPU_INT08U  NetCtr_CalcPctLarge(NET_CTR  pct_numer_hi,
                                NET_CTR  pct_numer_lo,
                                NET_CTR  pct_denom_hi,
                                NET_CTR  pct_denom_lo);


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  NET_CTR_CFG_STAT_EN
#error  "NET_CTR_CFG_STAT_EN        not #define'd in 'net_cfg.h'"
#error  "                     [MUST be  DEF_DISABLED]           "
#error  "                     [     ||  DEF_ENABLED ]           "
#elif  ((NET_CTR_CFG_STAT_EN != DEF_DISABLED) && \
        (NET_CTR_CFG_STAT_EN != DEF_ENABLED ))
#error  "NET_CTR_CFG_STAT_EN  illegally #define'd in 'net_cfg.h'"
#error  "                     [MUST be  DEF_DISABLED]           "
#error  "                     [     ||  DEF_ENABLED ]           "
#endif



#ifndef  NET_CTR_CFG_ERR_EN
#error  "NET_CTR_CFG_ERR_EN        not #define'd in 'net_cfg.h'"
#error  "                    [MUST be  DEF_DISABLED]           "
#error  "                    [     ||  DEF_ENABLED ]           "
#elif  ((NET_CTR_CFG_ERR_EN != DEF_DISABLED) && \
        (NET_CTR_CFG_ERR_EN != DEF_ENABLED ))
#error  "NET_CTR_CFG_ERR_EN  illegally #define'd in 'net_cfg.h'"
#error  "                    [MUST be  DEF_DISABLED]           "
#error  "                    [     ||  DEF_ENABLED ]           "
#endif

