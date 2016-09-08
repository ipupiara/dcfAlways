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
*                                        NETWORK PHYSICAL LAYER
*
*                                           National DP83848
*
* Filename      : net_phy.h
* Version       : V1.90
* Programmer(s) : EHS
*********************************************************************************************************
* Note(s)       : (1) Supports National Semiconductor DP83848 10/100 PHY
*
*                 (2) The MII interface port is assumed to be part of the host EMAC; consequently,
*                     reads from and writes to the PHY are made through the EMAC.  The functions
*                     NetNIC_PhyRegRd() and NetNIC_PhyRegWr(), which are used to access the PHY, should
*                     be provided in the EMAC driver.
*********************************************************************************************************
*/

#ifndef _NET_PHY_H
#define _NET_PHY_H

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define  DP83848_INIT_AUTO_NEG_RETRIES         3                /* Attempt Auto-Negotiation 3 times         */
#define  DP83848_INIT_RESET_RETRIES           16                /* Check for successful reset 16 times      */

#define  DP83848_OUI                    0x080017
#define  DP83848_VNDR_MDL                   0x09

/*
*********************************************************************************************************
*                                       DP83848 REGISTER DEFINES
*********************************************************************************************************
*/
                                                                /* ------- Generic MII registers ---------- */
#define  MII_BMCR                           0x00                /* Basic mode control register              */
#define  MII_BMSR                           0x01                /* Basic mode status register               */
#define  MII_PHYSID1                        0x02                /* PHYS ID 1                                */
#define  MII_PHYSID2                        0x03                /* PHYS ID 2                                */
#define  MII_ANAR                           0x04                /* Advertisement control reg                */
#define  MII_ANLPAR                         0x05                /* Link partner ability reg                 */
#define  MII_ANER                           0x06                /* Expansion register                       */
#define  MII_ANNPTR                         0x07                /* Next page transmit register              */

                                                                /* --------- Extended registers ----------- */
#define  DP83848_PHYSTS                     0x10                /* PHY Status Register                      */
#define  DP83848_MICR		                0x11                /* MII Interrupt Control Register           */
#define  DP83848_MISR		                0x12                /* MII Interrupt Status/Misc. Control Reg.  */
#define  DP83848_FCSCR		                0x14                /* False Carrier Sense Counter Register     */
#define  DP83848_RECR		                0x15                /* Receiver Error Counter Register          */
#define  DP83848_PCSR		                0x16                /* 100 Mb/s PCS Config. & Status Register   */
#define  DP83848_RBR			            0x17                /* RMII and Bypass Register                 */
#define  DP83848_LEDCR		                0x18                /* LED Direct Control Register              */
#define  DP83848_PHYCR		                0x19                /* PHY Control Register                     */
#define  DP83848_10BTSCR		            0x1A                /* 10Base-T Status/Control Register         */
#define  DP83848_CDCTRL1		            0x1B                /* CD Test & BIST Extensions Register       */
#define  DP83848_EDCR		                0x1D                /* Energy Detect Control                    */

/*
*********************************************************************************************************
*                                         DP83848 REGISTER BITS
*********************************************************************************************************
*/
                                                                /* -------- MII_BMCR Register Bits -------- */
#define  BMCR_RESV                         0x007F               /* Unused...                                */
#define  BMCR_CTST                       DEF_BIT_07             /* Collision test                           */
#define  BMCR_FULLDPLX                   DEF_BIT_08             /* Full duplex                              */
#define  BMCR_ANRESTART                  DEF_BIT_09             /* Auto negotiation restart                 */
#define  BMCR_ISOLATE                    DEF_BIT_10             /* Disconnect DP83840 from MII              */
#define  BMCR_PDOWN                      DEF_BIT_11             /* Powerdown the DP83840                    */
#define  BMCR_ANENABLE                   DEF_BIT_12             /* Enable auto negotiation                  */
#define  BMCR_SPEED100                   DEF_BIT_13             /* Select 100Mbps                           */
#define  BMCR_LOOPBACK                   DEF_BIT_14             /* TXD loopback bits                        */
#define  BMCR_RESET                      DEF_BIT_15             /* Reset the DP83840                        */

                                                                /* -------- MII_BMSR Register Bits -------- */
#define  BMSR_ERCAP                      DEF_BIT_00             /* Ext-reg capability                       */
#define  BMSR_JCD                        DEF_BIT_01             /* Jabber detected                          */
#define  BMSR_LSTATUS                    DEF_BIT_02             /* Link status                              */
#define  BMSR_ANEGCAPABLE                DEF_BIT_03             /* Able to do auto-negotiation              */
#define  BMSR_RFAULT                     DEF_BIT_04             /* Remote fault detected                    */
#define  BMSR_ANEGCOMPLETE               DEF_BIT_05             /* Auto-negotiation complete                */
#define  BMSR_RESV                         0x07C0               /* Unused...                                */
#define  BMSR_10HALF                     DEF_BIT_11             /* Can do 10mbps, half-duplex               */
#define  BMSR_10FULL                     DEF_BIT_12             /* Can do 10mbps, full-duplex               */
#define  BMSR_100HALF                    DEF_BIT_13             /* Can do 100mbps, half-duplex              */
#define  BMSR_100FULL                    DEF_BIT_14             /* Can do 100mbps, full-duplex              */
#define  BMSR_100BASE4                   DEF_BIT_15             /* Can do 100mbps, 4k packets               */

                                                                /* -------- MII_ANAR Register Bits -------- */
#define  ANAR_SLCT                         0x001F               /* Selector bits                            */
#define  ANAR_CSMA                       DEF_BIT_04             /* Only selector supported                  */
#define  ANAR_10HALF                     DEF_BIT_05             /* Try for 10mbps half-duplex               */
#define  ANAR_10FULL                     DEF_BIT_06             /* Try for 10mbps full-duplex               */
#define  ANAR_100HALF                    DEF_BIT_07             /* Try for 100mbps half-duplex              */
#define  ANAR_100FULL                    DEF_BIT_08             /* Try for 100mbps full-duplex              */
#define  ANAR_100BASE4                   DEF_BIT_09             /* Try for 100mbps 4k packets               */
#define  ANAR_RESV                         0x1C00               /* Unused...                                */
#define  ANAR_RFAULT                     DEF_BIT_13             /* Say we can detect faults                 */
#define  ANAR_LPACK                      DEF_BIT_14             /* Ack link partners response               */
#define  ANAR_NPAGE                      DEF_BIT_15             /* Next page bit                            */

#define  ANAR_FULL       (ANAR_100FULL | ANAR_10FULL | ANAR_CSMA)
#define  ANAR_ALL        (ANAR_100FULL | ANAR_10FULL | ANAR_100HALF | ANAR_10HALF)

                                                                /* ------- MII_ANLPAR Register Bits ------- */
#define  ANLPAR_SLCT                       0x001F               /* Same as advertise selector               */
#define  ANLPAR_10HALF                   DEF_BIT_05             /* Can do 10mbps half-duplex                */
#define  ANLPAR_10FULL                   DEF_BIT_06             /* Can do 10mbps full-duplex                */
#define  ANLPAR_100HALF                  DEF_BIT_07             /* Can do 100mbps half-duplex               */
#define  ANLPAR_100FULL                  DEF_BIT_08             /* Can do 100mbps full-duplex               */
#define  ANLPAR_100BASE4                 DEF_BIT_09             /* Can do 100mbps 4k packets                */
#define  ANLPAR_RESV                       0x1C00               /* Unused...                                */
#define  ANLPAR_RFAULT                   DEF_BIT_13             /* Link partner faulted                     */
#define  ANLPAR_LPACK                    DEF_BIT_14             /* Link partner acked us                    */
#define  ANLPAR_NPAGE                    DEF_BIT_15             /* Next page bit                            */

#define  ANLPAR_DUPLEX   (ANLPAR_10FULL  | ANLPAR_100FULL)
#define  ANLPAR_100	     (ANLPAR_100FULL | ANLPAR_100HALF | ANLPAR_100BASE4)

                                                                /* -------- MII_ANER Register Bits -------- */
#define  ANER_NWAY                       DEF_BIT_00             /* Can do N-way auto-nego                   */
#define  ANER_LCWP                       DEF_BIT_01             /* Got new RX page code word                */
#define  ANER_ENABLENPAGE                DEF_BIT_02             /* This enables npage words                 */
#define  ANER_NPCAPABLE                  DEF_BIT_03             /* Link partner supports npage              */
#define  ANER_MFAULTS                    DEF_BIT_04             /* Multiple faults detected                 */
#define  ANER_RESV                         0xFFE0               /* Unused...                                */

                                                                /* ----- DP83848_PHYSTS Register Bits ----- */
#define  PHYSTS_LINK_STATUS              DEF_BIT_00
#define  PHYSTS_SPEED_STATUS             DEF_BIT_01
#define  PHYSTS_DUPLEX_STATUS            DEF_BIT_02
#define  PHYSTS_LOOPBACK_STATUS          DEF_BIT_03
#define  PHYSTS_AN_COMPLETE              DEF_BIT_04
#define  PHYSTS_JABBER_DETECT            DEF_BIT_05
#define  PHYSTS_REMOTE_FAULT             DEF_BIT_06
#define  PHYSTS_MII_INTERRUPT            DEF_BIT_07
#define  PHYSTS_PAGE_RCVD                DEF_BIT_08
#define  PHYSTS_DESCRAMBLER_LOCK         DEF_BIT_09
#define  PHYSTS_SIGNAL_DETECT            DEF_BIT_10
#define  PHYSTS_FALSE_CARRIER_SENSE      DEF_BIT_11
#define  PHYSTS_POLARITY_STATUS          DEF_BIT_12
#define  PHYSTS_RECEIVE_ERROR_LATCH      DEF_BIT_13
#define  PHYSTS_MDIX_MODE                DEF_BIT_14

                                                                /* ----- DP83848_MICR Register Bits ------- */
#define  MICR_INTOE                      DEF_BIT_00
#define  MICR_INTEN                      DEF_BIT_01

                                                                /* ----- DP83848_MISR Register Bits ------- */
#define  MISR_ANC_INT_EN                 DEF_BIT_02             /* Autonegotiation complete interrupt en    */
#define  MISR_LINK_INT_EN                DEF_BIT_05             /* Link Status interrupt enable             */
#define  MISR_ANC_INT                    DEF_BIT_10             /* Autonegotiation complete event           */
#define  MISR_LINK_INT                   DEF_BIT_13             /* Link Status event                        */

                                                                /* ----- DP83848_RBR Register Bits -------- */
#define  RBR_RX_UNF_STATUS               DEF_BIT_03
#define  RBR_RX_OVL_STATUS               DEF_BIT_03
#define  RBR_RMII_REV1_0                 DEF_BIT_04
#define  RBR_RMII_MODE                   DEF_BIT_05

/*
*********************************************************************************************************
*                                   PHY ERROR CODES 12,000 -> 13,000
*********************************************************************************************************
*/

#define  NET_PHY_ERR_NONE                  12000
#define  NET_PHY_ERR_REGRD_TIMEOUT         12010
#define  NET_PHY_ERR_REGWR_TIMEOUT         12020
#define  NET_PHY_ERR_AUTONEG_TIMEOUT       12030
#define  NET_PHY_ERR_RESET_TIMEOUT         12040

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         NetNIC_PhyInit          (NET_ERR      *perr);

void         NetNIC_PhyAutoNeg       (void);                    /* Do link auto-negotiation                             */

                                                                /* -----------------PHY STATUS FNCTS ------------------ */
CPU_BOOLEAN  NetNIC_PhyAutoNegState  (void);                    /* Get PHY auto-negotiation state                       */

CPU_BOOLEAN  NetNIC_PhyLinkState     (void);                    /* Get PHY link state                                   */

CPU_INT32U   NetNIC_PhyLinkSpeed     (void);                    /* Get PHY link speed                                   */

CPU_INT32U   NetNIC_PhyLinkDuplex    (void);                    /* Get PHY duplex mode                                  */

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                    DEFINED IN PRODUCT'S  net_bsp.c
*********************************************************************************************************
*/

void         NetNIC_LinkUp           (void);                    /* Message from NIC that the ethernet link is up.       */
                                                                /* Called in interruption context most of the time.     */

void         NetNIC_LinkDown         (void);                    /* Message from NIC that the ethernet link is down.     */
                                                                /* Called in interruption context most of the time.     */


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                    DEFINED IN PRODUCT'S  net_isr.c
*********************************************************************************************************
*/

#if (NET_NIC_CFG_INT_CTRL_EN == DEF_ENABLED)
#if (EMAC_CFG_PHY_INT == DEF_ON)
void         NetNIC_PhyIntInit       (void);                    /* Init int ctrl'r.                                     */

void         NetNIC_PhyIntClr        (void);                    /* Clr  int ctrl'r src(s).                              */

void         NetNIC_PhyIntEnter      (void);                    /* Enter ISR              [see Note #2].                */
#endif
#endif

void         NetNIC_PhyISR_Handler   (void);

/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  NET_NIC_CFG_INT_CTRL_EN
#error  "NET_NIC_CFG_INT_CTRL_EN           not #define'd in 'net_cfg.h'"
#error  "                            [MUST be  DEF_DISABLED]           "
#error  "                            [     ||  DEF_ENABLED ]           "
#elif  ((NET_NIC_CFG_INT_CTRL_EN != DEF_DISABLED) && \
        (NET_NIC_CFG_INT_CTRL_EN != DEF_ENABLED ))
#error  "NET_NIC_CFG_INT_CTRL_EN     illegally #define'd in 'net_cfg.h'"
#error  "                            [MUST be  DEF_DISABLED]           "
#error  "                            [     ||  DEF_ENABLED ]           "
#endif

#ifndef   EMAC_CFG_PHY_ADDR
#error   "EMAC_CFG_PHY_ADDR           not #define'd in 'net_bsp.h'"
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




#endif
