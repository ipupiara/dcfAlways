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
*               Network Interface Card (NIC) port files provided, as is, for FREE and do
*               NOT require any additional licensing or licensing fee.
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
*                                        NETWORK INTERFACE CARD
*
*                                             Atmel MACB
*                                              for the
*                           AT91SAM7X256, AT91SAM9260, AT91SAM9263, & AVR32
*
* Filename      : net_nic.h
* Version       : V1.90
* Programmer(s) : EHS
*                 BAN (Converted AT91SAM7x256 driver to generic MACB driver)
*********************************************************************************************************
* Note(s)       : (1) This driver supports the Atmel MACB EMAC, often referred to as "MACB" or "EMACB" in 
*                     Atmel documentation, for the following Atmel processors (and possibly others) :
*
*                         Atmel Corporation (ATMEL; http://www.atmel.com)
*                         (a) AVR32         (ATMEL AVR32;       Revision 7684AX-AVR32-Nov06   )
*                         (b) AT91SAM7X256  (ATMEL AT91SAM7X;   Revision 6120F-ATARM-03-Oct-06)
*                         (c) AT91SAM9260   (ATMEL AT91SAM9260; Revision 6221D-ATARM-12-Mar-07)
*                         (d) AT91SAM9263   (ATMEL AT91SAM9263; Revision 6249B-ATARM-14-Dec-06)
*
*                 (2) REQUIREs Ethernet Network Interface Layer located in the following network directory :
*
*                         \<Network Protocol Suite>\IF\Ether\
*
*                              where
*                                      <Network Protocol Suite>    directory path for network protocol suite.
*
*                 (3) All processor- or hardware-specific functions, such as clearing an interrupt, which 
*                     might be identical between any two of these platforms, are now located in net_bsp.c.
*
*                 (4) Since the MACB module is integrated into either an ARM or AVR32 microcontroller,
*                     the endianness of the registers is the same as the CPU, which is little-endian by
*                     default.
*********************************************************************************************************
* TO DO(s)      : (1) Add support for jumbo frames.
*********************************************************************************************************
*/

#ifndef _NET_NIC_H
#define _NET_NIC_H


/*
*********************************************************************************************************
*                                                EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_NIC_MODULE
#define  NET_NIC_EXT
#else
#define  NET_NIC_EXT  extern
#endif


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <net_phy_def.h>
#include  <net_phy.h>


/*
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                NIC DRIVER LAYER / ETHERNET ERROR CODES
*
* Note(s) : (1) ALL NIC-independent          error codes #define'd in      'net_err.h';
*               ALL AT91SAM9260_EMAC-specific error codes #define'd in this 'net_nic.h'.
*
*           (2) Network error code '10,000' series reserved for NIC drivers.
*********************************************************************************************************
*/

#define  EMAC_ERR_RX_BUSY  10700
#define  EMAC_ERR_TX_BUSY  10701

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*/

NET_NIC_EXT  CPU_BOOLEAN  NetNIC_ConnStatus;                            /* NIC's connection status : DEF_ON/DEF_OFF.           */

#if (NET_CTR_CFG_STAT_EN == DEF_ENABLED)                                /* ------------------- NET DRV STATS ----------------- */
NET_NIC_EXT  NET_CTR      NetNIC_StatRxPktCtr;
NET_NIC_EXT  NET_CTR      NetNIC_StatTxPktCtr;
#endif

#if (NET_CTR_CFG_ERR_EN  == DEF_ENABLED)                                /* ------------------- NET DRV ERRS ------------------ */
NET_NIC_EXT  NET_CTR      NetNIC_ErrRxPktDiscardedCtr;
NET_NIC_EXT  NET_CTR      NetNIC_ErrTxPktDiscardedCtr;
#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         NetNIC_Init            (NET_ERR     *perr);

void         NetNIC_IntEn           (NET_ERR     *perr);

                                                                        /* ------------------- STATUS FNCTS ------------------ */
CPU_BOOLEAN  NetNIC_ConnStatusGet   (void);

void         NetNIC_LinkChange      (CPU_INT32U link_speed,
                                     CPU_INT32U link_duplex);

void         NetNIC_ISR_Handler     (void);                             /* Decode & handle rx/tx ISRs.                         */

                                                                        /* --------------------- RX FNCTS -------------------- */
CPU_INT16U   NetNIC_RxPktGetSize    (void);                             /* Get NIC rx pkt size.                                */

void         NetNIC_RxPkt           (void        *ppkt,                 /* Rx pkt from NIC.                                    */
                                     CPU_INT16U   size,
                                     NET_ERR     *perr);

void         NetNIC_RxPktDiscard    (CPU_INT16U   size,                 /* Discard rx pkt in NIC.                              */
                                     NET_ERR     *perr);

                                                                        /* --------------------- TX FNCTS -------------------- */
void         NetNIC_TxPktPrepare    (void        *ppkt,
                                     CPU_INT16U   size,
                                     NET_ERR     *perr);

void         NetNIC_TxPkt           (void        *ppkt,
                                     CPU_INT16U   size,
                                     NET_ERR     *perr);

                                                                        /* ------------------- PHYTER FNCTS ------------------ */
void         NetNIC_PhyInit         (NET_ERR     *perr);

CPU_INT16U   NetNIC_PhyRegRd        (CPU_INT08U   phy,
                                     CPU_INT08U   reg,
                                     NET_ERR    *perr);

void         NetNIC_PhyRegWr        (CPU_INT08U   phy,
                                     CPU_INT08U   reg,
                                     CPU_INT16U   val,
                                     NET_ERR    *perr);


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                       DEFINED IN OS'S  net_os.c
*********************************************************************************************************
*/

void         NetOS_NIC_Init         (NET_ERR     *perr);                /* Create Drv objs & start Drv tasks.                  */

void         NetOS_NIC_TxRdyWait    (NET_ERR     *perr);                /* Wait on drv tx empty signal from NIC.               */

void         NetOS_NIC_TxRdySignal  (void);                             /* Post    drv tx empty signal from NIC.               */


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                    DEFINED IN PRODUCT'S  net_isr.c
*
* Note(s) : (1) See this 'net_nic.c  MACB_Init()'.
*
*           (2) See this 'net_nic.c  NetNIC_ISR_Handler()   Note #3'.
*********************************************************************************************************
*/

#if (NET_NIC_CFG_INT_CTRL_EN == DEF_ENABLED)
void         NetNIC_IntInit         (void);                             /* Init int ctrl'r        [see Note #1].               */

void         NetNIC_IntClr          (void);                             /* Clr  int ctrl'r src(s) [see Note #2].               */

void         NetNIC_IntEnter        (void);                             /* Enter ISR              [see Note #2].               */
#endif


/*
*********************************************************************************************************
*                                         CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef   NET_NIC_CFG_INT_CTRL_EN
#error   "NET_NIC_CFG_INT_CTRL_EN     not #define'd in 'net_cfg.h'"
#error   "                            [MUST be  DEF_DISABLED]"
#error   "                            [     ||  DEF_ENABLED ]"
#elif   ((NET_NIC_CFG_INT_CTRL_EN != DEF_ENABLED ))
#error   "NET_NIC_CFG_INT_CTRL_EN     illegally #define'd in 'net_cfg.h'"
#error   "                            [MUST be  DEF_ENABLED ]"
#endif

#ifndef   EMAC_CFG_MAC_ADDR_SEL
#error   "EMAC_CFG_MAC_ADDR_SEL       not #define'd in 'net_bsp.h'"
#error   "                            [MUST be  EMAC_MAC_ADDR_SEL_CFG   ]"
#error   "                            [     ||  EMAC_MAC_ADDR_SEL_EEPROM]"
#elif   ((EMAC_CFG_MAC_ADDR_SEL != EMAC_MAC_ADDR_SEL_CFG   ) && \
         (EMAC_CFG_MAC_ADDR_SEL != EMAC_MAC_ADDR_SEL_EEPROM))
#error   "EMAC_CFG_MAC_ADDR_SEL       illegally #define'd in 'net_bsp.h'"
#error   "                            [MUST be  EMAC_MAC_ADDR_SEL_CFG   ]"
#error   "                            [     ||  EMAC_MAC_ADDR_SEL_EEPROM]"
#endif

#ifndef   EMAC_CFG_RMII
#error   "EMAC_CFG_RMII               not #define'd in 'net_bsp.h'"
#error   "                            [MUST be  DEF_YES   ]"
#error   "                            [     ||  DEF_NO    ]"
#elif   ((EMAC_CFG_RMII != DEF_YES) && \
         (EMAC_CFG_RMII != DEF_NO ))
#error   "EMAC_CFG_RMII               illegally #define'd in 'net_bsp.h'"
#error   "                            [MUST be  DEF_YES]"
#error   "                            [     ||  DEF_NO ]"
#endif

#ifndef   MACB_BASE
#error   "MACB_BASE          not #define'd in 'net_bsp.h'"
#endif

#ifndef   NIC_RX_N_BUFS
#error   "NIC_RX_N_BUFS       not #define'd in 'net_bsp.h'"
#error   "                            [MUST be  >= 2]"
#elif    (NIC_RX_N_BUFS < 2)
#error   "NIC_RX_N_BUFS       illegally #define'd in 'net_bsp.h'"
#error   "                            [MUST be  >= 2]"
#endif

#ifndef   NIC_RX_N_BUFS
#error   "NIC_RX_N_BUFS       not #define'd in 'net_bsp.h'"
#error   "                            [MUST be  >= 2]"
#elif    (NIC_RX_N_BUFS < 2)
#error   "NIC_RX_N_BUFS       illegally #define'd in 'net_bsp.h'"
#error   "                            [MUST be  >= 2]"
#endif




#endif
