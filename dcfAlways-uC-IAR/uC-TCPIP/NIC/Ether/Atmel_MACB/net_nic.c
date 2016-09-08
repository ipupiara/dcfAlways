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
* Filename      : net_nic.c
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

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define    NET_NIC_MODULE
#include  <net.h>

/*
*********************************************************************************************************
*                                    ATMEL MACB RECEIVE BUFFER DESCRIPTOR
*********************************************************************************************************
*/

#define  EMAC_RXBUF_ADDRESS_MASK    (0xFFFFFFFC)                        /* Addr of Rx Descriptor Buf's (wrap/ownership ignored  */
#define  EMAC_RXBUF_ADD_WRAP         DEF_BIT_01                         /* This is the last buffer in the ring.                 */
#define  EMAC_RXBUF_SW_OWNED         DEF_BIT_00                         /* Software owns the buffer.                            */
#define  EMAC_RXBUF_LEN_MASK        (0x00000FFF)                        /* Length of frame including FCS. (12 bits)             */
#define  EMAC_RXBUF_SOF_MASK         DEF_BIT_14                         /* Start of frame mask                                  */
#define  EMAC_RXBUF_EOF_MASK         DEF_BIT_15                         /* End of frame mask                                    */
#define  EMAC_RXBUF_OFF_MASK        (0x03 << 12)                        /* Data offset mask (up to three bytes possible)        */

#define  EMAC_TXBUF_ADD_WRAP         DEF_BIT_30                         /* This is the last buffer in the ring.                 */
#define  EMAC_TXBUF_TX_SIZE_MASK    (0x000007FF)                        /* Length of frame including FCS. (11 bits)             */
#define  EMAC_TXBUF_ADD_LAST         DEF_BIT_15                         /* This is the last buffer for the current frame        */

#define  EMAC_TXBUF_STATUS_USED      DEF_BIT_31                         /* Status Used Bit. Indicates when a buf has been read  */

/*
*********************************************************************************************************
*                                          ATMEL MACB REGISTERS
*
* Note(s) : (1) See ATMEL AT91SAM7X256, AT91SAM9260, or AVR32 documentation for register summary.
*
*           (2) Since the MACB module is integrated into either an ARM or AVR32 microcontroller,
*               the endianness of the registers is the same as the CPU, which is little-endian by default.
*
*           (3) The base address of the MACB registers, MACB_BASE, must be defined in net_bsp.h
*
*           (4) To protect MACB register accesses during multi-thread operation, registers MUST be
*               accessed in critical sections &/or mutual exclusion.
*
*               (a) For read-modify-write register access OR multiple-register-access sequences; critical
*                   sections &/or mutual exclusion enclosing the entire register access/sequence is REQUIRED.
*********************************************************************************************************
*/

                                                                            /* -------- CONFIGURATION REGISTERS --------------- */
#define  MACB_NCR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x00))      /* EMAC Control Register                            */
#define  MACB_NCFGR       (*(volatile CPU_INT32U *)(MACB_BASE + 0x04))      /* EMAC Configuration Register                      */
#define  MACB_NSR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x08))      /* EMAC Status Register                             */
#define  MACB_TSR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x14))      /* EMAC Transmit Status Register                    */
#define  MACB_RBQP        (*(volatile CPU_INT32U *)(MACB_BASE + 0x18))      /* EMAC Receive  Buffer Queue Pointer               */
#define  MACB_TBQP        (*(volatile CPU_INT32U *)(MACB_BASE + 0x1C))      /* EMAC Transmit Buffer Queue Pointer               */
#define  MACB_RSR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x20))      /* EMAC Receive Status Register                     */
#define  MACB_ISR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x24))      /* EMAC Interrupt Status  Register                  */
#define  MACB_IER         (*(volatile CPU_INT32U *)(MACB_BASE + 0x28))      /* EMAC Interrupt Enable  Register                  */
#define  MACB_IDR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x2C))      /* EMAC Interrupt Disable Register                  */
#define  MACB_IMR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x30))      /* EMAC Interrupt Mask    Register                  */
#define  MACB_MAN         (*(volatile CPU_INT32U *)(MACB_BASE + 0x34))      /* EMAC PHY Maintenance   Register                  */
#define  MACB_PTR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x38))      /* EMAC Pause Time        Register                  */

                                                                            /* -------- STATISTIC REGISTERS ------------------- */
#define  MACB_PFR         (*(volatile CPU_INT32U *)(MACB_BASE + 0x3C))      /* Pause Frames Received Register                   */
#define  MACB_FTO         (*(volatile CPU_INT32U *)(MACB_BASE + 0x40))      /* Frames Transmitted OK Register                   */
#define  MACB_SCF         (*(volatile CPU_INT32U *)(MACB_BASE + 0x44))      /* Single   Collision Frame Register                */
#define  MACB_MCF         (*(volatile CPU_INT32U *)(MACB_BASE + 0x48))      /* Multiple Collision Frame Register                */
#define  MACB_FRO         (*(volatile CPU_INT32U *)(MACB_BASE + 0x4C))      /* Frames Received    OK Register                   */
#define  MACB_FCSE        (*(volatile CPU_INT32U *)(MACB_BASE + 0x50))      /* Frame Check Sequence Error Register              */
#define  MACB_ALE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x54))      /* Alignment Error Register                         */
#define  MACB_DTF         (*(volatile CPU_INT32U *)(MACB_BASE + 0x58))      /* Deferred Transmission Frame Register             */
#define  MACB_LCOL        (*(volatile CPU_INT32U *)(MACB_BASE + 0x5C))      /* Late      Collision Register                     */
#define  MACB_ECOL        (*(volatile CPU_INT32U *)(MACB_BASE + 0x60))      /* Excessive Collision Register                     */
#define  MACB_TUND        (*(volatile CPU_INT32U *)(MACB_BASE + 0x64))      /* Transmit Underrun Error Register                 */
#define  MACB_CSE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x68))      /* Carrier Sense     Error Register                 */
#define  MACB_RRE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x6C))      /* Receive Resource  Error Register                 */
#define  MACB_ROV         (*(volatile CPU_INT32U *)(MACB_BASE + 0x70))      /* Receive Jabber Register                          */
#define  MACB_RSE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x74))      /* Undersize Frame Register                         */
#define  MACB_ELE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x78))      /* Excessive Length  Error Register                 */
#define  MACB_RJA         (*(volatile CPU_INT32U *)(MACB_BASE + 0x7C))      /* Receive Jabbers Register                         */
#define  MACB_USF         (*(volatile CPU_INT32U *)(MACB_BASE + 0x80))      /* Undersize Frame Register                         */
#define  MACB_STE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x84))      /* SQE Test          Error Register                 */
#define  MACB_RLE         (*(volatile CPU_INT32U *)(MACB_BASE + 0x88))      /* Received Length Field Mismatch Register          */

                                                                            /* -------- ADDRESS REGISTERS --------------------- */
#define  MACB_HRB         (*(volatile CPU_INT32U *)(MACB_BASE + 0x90))      /* Hash Register Bottom [31: 0] Register            */
#define  MACB_HRT         (*(volatile CPU_INT32U *)(MACB_BASE + 0x94))      /* Hash Register Top    [63:32] Register            */
#define  MACB_SA1L        (*(volatile CPU_INT32U *)(MACB_BASE + 0x98))      /* Specific Address 1 Bottom Register               */
#define  MACB_SA1H        (*(volatile CPU_INT32U *)(MACB_BASE + 0x9C))      /* Specific Address 1 Top    Register               */
#define  MACB_SA2L        (*(volatile CPU_INT32U *)(MACB_BASE + 0xA0))      /* Specific Address 2 Bottom Register               */
#define  MACB_SA2H        (*(volatile CPU_INT32U *)(MACB_BASE + 0xA4))      /* Specific Address 2 Top    Register               */
#define  MACB_SA3L        (*(volatile CPU_INT32U *)(MACB_BASE + 0xA8))      /* Specific Address 3 Bottom Register               */
#define  MACB_SA3H        (*(volatile CPU_INT32U *)(MACB_BASE + 0xAC))      /* Specific Address 3 Top    Register               */
#define  MACB_SA4L        (*(volatile CPU_INT32U *)(MACB_BASE + 0xB0))      /* Specific Address 4 Bottom Register               */
#define  MACB_SA4H        (*(volatile CPU_INT32U *)(MACB_BASE + 0xB4))      /* Specific Address 4 Top    Register               */

                                                                            /* ----- ADDITIONAL CONFIGURATION REGISTERS ------- */
#define  MACB_TID         (*(volatile CPU_INT32U *)(MACB_BASE + 0xB8))      /* Type ID Checking Register                        */
#define  MACB_USRIO       (*(volatile CPU_INT32U *)(MACB_BASE + 0xC0))      /* User Input/Output Register                       */
#define  MACB_WOL         (*(volatile CPU_INT32U *)(MACB_BASE + 0xC4))      /* Wake on LAN                                      */

/*
*********************************************************************************************************
*                                       ATMEL MACB REGISTER BITS
*********************************************************************************************************
*/

                                                                /* ---------------------- MACB_NCR bits ----------------------- */
#define  MACB_NCR_LB                      DEF_BIT_00            /* Loopback. When set, loopback signal is at high level.        */
#define  MACB_NCR_LLB                     DEF_BIT_01            /* Loopback local.                                              */
#define  MACB_NCR_RE                      DEF_BIT_02            /* Receive enable.                                              */
#define  MACB_NCR_TE                      DEF_BIT_03            /* Transmit enable.                                             */
#define  MACB_NCR_MPE                     DEF_BIT_04            /* Management port enable.                                      */
#define  MACB_NCR_CLRSTAT                 DEF_BIT_05            /* Clear statistics registers.                                  */
#define  MACB_NCR_INCSTAT                 DEF_BIT_06            /* Increment statistics registers.                              */
#define  MACB_NCR_WESTAT                  DEF_BIT_07            /* Write enable for statistics registers.                       */
#define  MACB_NCR_BP                      DEF_BIT_08            /* Back pressure.                                               */
#define  MACB_NCR_TSTART                  DEF_BIT_09            /* Start Transmission.                                          */
#define  MACB_NCR_THALT                   DEF_BIT_10            /* Transmission Halt.                                           */
#define  MACB_NCR_TPFR                    DEF_BIT_11            /* Transmit pause frame                                         */
#define  MACB_NCR_TZQ                     DEF_BIT_12            /* Transmit zero quantum pause frame                            */

                                                                /* ---------------------- MACB_NCFGR bits --------------------- */
#define  MACB_NCFGR_SPD                   DEF_BIT_00            /* Speed.                                                       */
#define  MACB_NCFGR_FD                    DEF_BIT_01            /* Full duplex.                                                 */
#define  MACB_NCFGR_JFRAME                DEF_BIT_03            /* Jumbo Frames.                                                */
#define  MACB_NCFGR_CAF                   DEF_BIT_04            /* Copy all frames.                                             */
#define  MACB_NCFGR_NBC                   DEF_BIT_05            /* No broadcast.                                                */
#define  MACB_NCFGR_MTI                   DEF_BIT_06            /* Multicast hash event enable                                  */
#define  MACB_NCFGR_UNI                   DEF_BIT_07            /* Unicast hash enable.                                         */
#define  MACB_NCFGR_BIG                   DEF_BIT_08            /* Receive 1522 bytes.                                          */
#define  MACB_NCFGR_EAE                   DEF_BIT_09            /* External address match enable.                               */
#define  MACB_NCFGR_CLK             ((CPU_INT32U) 3 << 10)
#define  MACB_NCFGR_CLK_HCLK_8      ((CPU_INT32U) 0 << 10)      /* HCLK divided by 8                                            */
#define  MACB_NCFGR_CLK_HCLK_16     ((CPU_INT32U) 1 << 10)      /* HCLK divided by 16                                           */
#define  MACB_NCFGR_CLK_HCLK_32     ((CPU_INT32U) 2 << 10)      /* HCLK divided by 32                                           */
#define  MACB_NCFGR_CLK_HCLK_64     ((CPU_INT32U) 3 << 10)      /* HCLK divided by 64                                           */
#define  MACB_NCFGR_RTY                   DEF_BIT_12
#define  MACB_NCFGR_PAE                   DEF_BIT_13
#define  MACB_NCFGR_RBOF            ((CPU_INT32U) 3 << 14)
#define  MACB_NCFGR_RBOF_OFFSET_0   ((CPU_INT32U) 0 << 14)      /* no offset from start of receive buffer                       */
#define  MACB_NCFGR_RBOF_OFFSET_1   ((CPU_INT32U) 1 << 14)      /* one byte offset from start of receive buffer                 */
#define  MACB_NCFGR_RBOF_OFFSET_2   ((CPU_INT32U) 2 << 14)      /* two bytes offset from start of receive buffer                */
#define  MACB_NCFGR_RBOF_OFFSET_3   ((CPU_INT32U) 3 << 14)      /* three bytes offset from start of receive buffer              */
#define  MACB_NCFGR_RLCE                  DEF_BIT_16            /* Receive Length field Checking Enable                         */
#define  MACB_NCFGR_DRFCS                 DEF_BIT_17            /* Discard Receive FCS                                          */
#define  MACB_NCFGR_EFRHD                 DEF_BIT_18
#define  MACB_NCFGR_IRXFCS                DEF_BIT_19            /* Ignore RX FCS                                                */

                                                                /* ---------------------- MACB_NSR bits ----------------------- */
#define  MACB_NSR_LINKR                   DEF_BIT_00
#define  MACB_NSR_MDIO                    DEF_BIT_01
#define  MACB_NSR_IDLE                    DEF_BIT_02

                                                                /* ---------------------- MACB_TSR bits ----------------------- */
#define  MACB_TSR_UBR                     DEF_BIT_00
#define  MACB_TSR_COL                     DEF_BIT_01
#define  MACB_TSR_RLES                    DEF_BIT_02
#define  MACB_TSR_TGO                     DEF_BIT_03            /* Transmit Go                                                  */
#define  MACB_TSR_BEX                     DEF_BIT_04            /* Buffers exhausted mid frame                                  */
#define  MACB_TSR_COMP                    DEF_BIT_05
#define  MACB_TSR_UND                     DEF_BIT_06

                                                                /* ---------------------- MACB_RSR bits ----------------------- */
#define  MACB_RSR_BNA                     DEF_BIT_00
#define  MACB_RSR_REC                     DEF_BIT_01
#define  MACB_RSR_OVR                     DEF_BIT_02

                                                                /* ---------------------- MACB_ISR bits ----------------------- */
                                                                /* ---------------------- MACB_IER bits ----------------------- */
                                                                /* ---------------------- MACB_IDR bits ----------------------- */
                                                                /* ---------------------- MACB_IMR bits ----------------------- */
#define  MACB_ISR_MFD                     DEF_BIT_00
#define  MACB_ISR_RCOMP                   DEF_BIT_01
#define  MACB_ISR_RXUBR                   DEF_BIT_02
#define  MACB_ISR_TXUBR                   DEF_BIT_03
#define  MACB_ISR_TUNDR                   DEF_BIT_04
#define  MACB_ISR_RLEX                    DEF_BIT_05
#define  MACB_ISR_TXERR                   DEF_BIT_06
#define  MACB_ISR_TCOMP                   DEF_BIT_07
#define  MACB_ISR_LINK                    DEF_BIT_09
#define  MACB_ISR_ROVR                    DEF_BIT_10
#define  MACB_ISR_HRESP                   DEF_BIT_11
#define  MACB_ISR_PFRE                    DEF_BIT_12
#define  MACB_ISR_PTZ                     DEF_BIT_13

                                                                /* ---------------------- MACB_MAN bits ----------------------- */
#define  MACB_MAN_CODE             ((CPU_INT32U)   0x02 << 16)  /* IEEE Code. MUST have value of 10.                            */
#define  MACB_MAN_RW               ((CPU_INT32U)   0x03 << 28)
#define  MACB_MAN_WRITE            ((CPU_INT32U)   0x01 << 28)  /* 01: Transfer is a write.                                     */
#define  MACB_MAN_READ             ((CPU_INT32U)   0x02 << 28)  /* 10: Transfer is a read.                                      */
#define  MACB_MAN_SOF              ((CPU_INT32U)   0x03 << 30)
#define  MACB_MAN_REGA(_x_)         (_x_         << 18)         /* Specifies the register in the PHY to access.                 */
#define  MACB_MAN_PHYA(_x_)        ((_x_ & 0x1F) << 23)         /* PHY address. Normally 0.                                     */
#define  MACB_MAN_DATA(_x_)         (_x_ & 0xFFFF)              /* PHY Read/Write Data Mask.                                    */

                                                                /* --------------------- MACB_USRIO bits ---------------------- */
#define  MACB_USRIO_RMII                  DEF_BIT_00            /* Reduce MII                                                   */
#define  MACB_USRIO_CLKEN                 DEF_BIT_01            /* Clock Enable                                                 */

                                                                /* ---------------------- MACB_WOL bits ----------------------- */
#define  MACB_WOL_IP                ((CPU_INT32U) 0xFFFF <<  0) /* ARP request IP address                                       */
#define  MACB_WOL_MAG               ((CPU_INT32U)      1 << 16) /* Magic packet event enable                                    */
#define  MACB_WOL_ARP               ((CPU_INT32U)      1 << 17) /* ARP request event enable                                     */
#define  MACB_WOL_SA1               ((CPU_INT32U)      1 << 18) /* Specific address register 1 event enable                     */


/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/

typedef struct {
    CPU_INT32U  addr;                                                   /* Address of RX buffer.                                */
    CPU_INT32U  status;                                                 /* Status of RX buffer.                                 */
}  NIC_BUF_DESC;

/*
*********************************************************************************************************
*                                               GLOBALS
*********************************************************************************************************
*/

NIC_BUF_DESC  *NIC_RxBufDescPtrStart;                                   /* Pointer to the aligned list of Rx buffer descriptors */
NIC_BUF_DESC  *NIC_RxBufDescPtrEnd;                                     /* Pointer to the END of the Rx descriptor list         */
NIC_BUF_DESC  *NIC_TxBufDescPtr;                                        /* Pointer to the only Tx buffer descriptor (aligned)   */

NIC_BUF_DESC  *NIC_CurrentFrameStart;                                   /* Pointer to the 1st descriptor for the current frame  */
NIC_BUF_DESC  *NIC_CurrentFrameEnd;                                     /* Pointer to the last descriptor for the current frame */

NIC_BUF_DESC  *NIC_ExpectedFrameStart;                                  /* Pointer to the next expected SOF descriptor          */
                                                                        /* This is used to improve search / detection of frames */
                                                                        /* that need to be read from the descriptor buffers     */

CPU_INT16U     NIC_RxNRdyCtr;                                           /* Track # of frames are need to be read from memory    */

                                                                        /* MUST be aligned on a 32 byte boundaries              */
CPU_INT08U     NIC_RxBuf[NIC_RX_BUF_SIZE * NIC_RX_N_BUFS + 32];         /* RxBuffers & descriptors, Tx descriptors declarations */
CPU_INT08U     NIC_RxBufDesc[sizeof(NIC_BUF_DESC) * NIC_RX_N_BUFS + 32];
CPU_INT08U     NIC_TxBufDesc[sizeof(NIC_BUF_DESC) * 1 + 32];

/*
*********************************************************************************************************
*                                       LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void        NetNIC_RxISR_Handler            (void);             /* ISR for RX interrupts.                               */
static  void        NetNIC_TxISR_Handler            (void);             /* ISR for TX interrupts.                               */
static  void        NetNIC_TxPktDiscard             (NET_ERR     *perr);

static  CPU_INT16U  NIC_GetNRdy                     (void);

                                                                        /* -------------------- MACB FNCTS -------------------  */
static  void        MACB_Init                       (NET_ERR     *perr);


                                                                        /* ------------------ MACB RX FNCTS ------------------  */
static  void        MACB_RxBufInit                  (void);
static  void        MACB_RxEn                       (void);
static  void        MACB_RxDis                      (void);
static  void        MACB_RxIntEn                    (void);

static  void        MACB_RxPktDiscard               (CPU_INT16U   size);

static  void        MACB_RxPkt                      (void        *ppkt,
                                                     CPU_INT16U   size,
                                                     NET_ERR     *perr);

                                                                        /* ------------------ MACB TX FNCTS ------------------  */
static  void        MACB_TxBufInit                  (void);
static  void        MACB_TxEn                       (void);
static  void        MACB_TxDis                      (void);
static  void        MACB_TxIntEn                    (void);
static  void        MACB_TxPkt                      (void        *ppkt,
                                                     CPU_INT16U   size,
                                                     NET_ERR     *perr);

static  void        MACB_TxPktPrepare  (void        *ppkt,
                                                     CPU_INT16U   size,
                                                     NET_ERR     *perr);

/*
*********************************************************************************************************
*********************************************************************************************************
*                                            GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             NetNIC_Init()
*
* Description : (1) Initialize Network Interface Card :
*                   (a) Perform NIC Layer OS initialization
*                   (b) Initialize NIC status
*                   (c) Initialize NIC statistics & error counters
*                   (d) Initialize MACB
*
* Argument(s) : perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                    Network interface card successfully initialized.
*
*                                                                   -------- RETURNED BY NetOS_NIC_Init() : --------
*                               NET_OS_ERR_INIT_NIC_TX_RDY          NIC transmit ready signal NOT successfully
*                                                                       initialized.
*                               NET_OS_ERR_INIT_NIC_TX_RDY_NAME     NIC transmit ready name   NOT successfully
*                                                                       configured.
* Return(s)   : none.
*
* Caller(s)   : Net_Init().
*********************************************************************************************************
*/

void  NetNIC_Init (NET_ERR  *perr)
{
                                                                        /* --------------- PERFORM NIC/OS INIT ---------------  */
    NetOS_NIC_Init(perr);                                               /* Create NIC objs.                                     */
    if (*perr != NET_OS_ERR_NONE) {
         return;
    }

                                                                        /* ----------------- INIT NIC STATUS -----------------  */
    NetNIC_ConnStatus           =  DEF_OFF;

                                                                        /* ------------- INIT NIC STAT & ERR CTRS ------------  */
#if (NET_CTR_CFG_STAT_EN        == DEF_ENABLED)
    NetNIC_StatRxPktCtr         =  0;
    NetNIC_StatTxPktCtr         =  0;
#endif

#if (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)
    NetNIC_ErrRxPktDiscardedCtr =  0;
    NetNIC_ErrTxPktDiscardedCtr =  0;
#endif

                                                                        /* -------------------- INIT MACB---------------------  */
    MACB_Init(perr);
}


/*
*********************************************************************************************************
*                                            NetNIC_IntEn()
*
* Description : Enable NIC interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Net_Init().
*********************************************************************************************************
*/

void  NetNIC_IntEn (NET_ERR  *perr)
{
    MACB_RxIntEn();
    MACB_TxIntEn();

    *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                        EMAC Link Settings Update
*
* Description : This function is called by NetNIC_Init and the PHY ISR in order to update the
*             : speed and duplex settings for the EMAC.
*
* Arguments   : link_speed      Indicates link speed.  This can be one of
*                                   NET_PHY_SPD_0
*                                   NET_PHY_SPD_10
*                                   NET_PHY_SPD_100
*                                   NET_PHY_SPD_1000
*
*               link_duplex     Indicates link duplex.  This can be one of
*                                   NET_PHY_DUPLEX_UNKNOWN
*                                   NET_PHY_DUPLEX_HALF
*                                   NET_PHY_DUPLEX_FULL
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  NetNIC_LinkChange (CPU_INT32U link_speed, CPU_INT32U link_duplex)
{
    CPU_INT32U reg_val;


                                                                        /* Get EMAC config, clear speed & duplex bits           */
    reg_val         =   MACB_NCFGR & ~(MACB_NCFGR_SPD | MACB_NCFGR_FD);

    if (link_speed ==  NET_PHY_SPD_100) {
        reg_val    |=   MACB_NCFGR_SPD;                                 /* If 100mbps, set the 100mbps bit in reg_val           */
    }

    if (link_duplex == NET_PHY_DUPLEX_FULL) {
        reg_val    |=   MACB_NCFGR_FD;                                  /* If Full Duplex, set the FD bit in reg_val            */
    }
                                                                        /* Write the Link speed and duplex values to the        */
    MACB_NCFGR      = reg_val;                                          /* EMAC control network configuration register          */
}


/*
*********************************************************************************************************
*                                        NetNIC_ConnStatusGet()
*
* Description : Get NIC's network connection status.
*
*               (1) Obtaining the NIC's network connection status is encapsulated in this function for the
*                   possibility that obtaining a NIC's connection status requires a non-trivial procedure.
*
*
* Argument(s) : none.
*
* Return(s)   : NIC network connection status :
*
*                   DEF_OFF     Network connection DOWN.
*                   DEF_ON      Network connection UP.
*
* Caller(s)   : NetIF_Pkt_Tx().
*********************************************************************************************************
*/

CPU_BOOLEAN  NetNIC_ConnStatusGet (void)
{
    return (NetNIC_ConnStatus);
}

/*
*********************************************************************************************************
*                                         NetNIC_ISR_Handler()
*
* Description : (1) Decode ISR & call appropriate ISR handler :
*
*                   (a) MACB  Receive Buffer Not Available ISR     NetNIC_RxPktDiscard().
*                   (b) MACB  Receive  ISR                         NetNIC_RxISR_Handler().
*                   (c) MACB  Transmit ISR                         NetNIC_TxISR_Handler().
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR
*
* Note(s)     : (3) The functions NetNIC_IntEnter() and NetNIC_IntClr() inform the external interrupt
*                   controller that the ISR is being entered and clear the interrupt source(s) on the
*                   external interrupt controller, respectively. These MUST be developer-implemented in
*
*                       \<Your Product Application>\net_isr.c
*
*                           where
*                                   <Your Product Application>    directory path for Your Product's Application.
*********************************************************************************************************
*/

void  NetNIC_ISR_Handler (void)
{
    volatile  CPU_INT32U  isr_status;
              CPU_INT32U  rsr_status;
              CPU_INT32U  tsr_status;


    NetNIC_IntEnter();

                                                                        /* Read the interrupt status register                   */
    isr_status      =  MACB_ISR;                                        /* This read clears the bits in the ISR register        */

                                                                        /* Check for 'frame received' interrupt                 */
    rsr_status      = MACB_RSR;                                         /* Read the Receive Status Register                     */
    if ((rsr_status & MACB_RSR_REC) == MACB_RSR_REC) {                  /* Check if we have recieved a frame                    */
        NetNIC_RxISR_Handler();                                         /* Call the Rx Handler                                  */
        MACB_RSR    = MACB_RSR_REC;		                                /* Clear the Receive Status Register REC bit            */
    }
                                                                        /* Check for 'transmit complete' interrupt              */
    tsr_status      = MACB_TSR;                                         /* Read the Transmit Status Register                    */
    if ((tsr_status & MACB_TSR_COMP) == MACB_TSR_COMP) {                /* Check if we have completed transmitting a frame      */
        MACB_TSR    = MACB_TSR_COMP;                                    /* Clear the Transmit Status Register COMP bit          */
        NetNIC_TxISR_Handler();                                         /* Call the Tx Handler                                  */
    }
                                                                        /* If a transmit buffer was read, clear the Used bit    */
    if ((tsr_status &  MACB_TSR_UBR) == MACB_TSR_UBR) {                 /* Check if 'Used Bit Read' is set                      */
        MACB_TSR    = MACB_TSR_UBR;	                                    /* Clear 'Used Bit Read'                                */
    }   	

    if ((rsr_status & (MACB_RSR_BNA | MACB_RSR_OVR)) > 0) {             /* Clear all receiver errors if they have occured       */
        MACB_RSR    = (MACB_RSR_BNA | MACB_RSR_OVR);
        NetNIC_RxISR_Handler();                                         /* If any error occured, we call NetNIC_RxISR_Handler() */
    }                                                                   /* to ensure that we havent missed any events           */

    NetNIC_IntClr();                                                    /* Clr int ctrl'r  int (see Note #3).                   */
}

/*
*********************************************************************************************************
*                                         NetNIC_RxPktGetSize()
*
* Description : Get network packet size from NIC.
*
* Argument(s) : none.
*
* Return(s)   : Size, in octets, of next frame to be read from the EMAC buffers
*
* Caller(s)   : NetIF_RxTaskHandler()
*
* Notes       : 1) On occassion, it is possible to have frame fragments in the descriptor list. As mentioned
*                  above in Note 1) of NIC_GetNRdy(), should 2 start of frame descriptors be found without
*                  corresponding EOF descriptors , then softwares must free the rogue descriptors. This
*                  function will free rogue descriptors if they are found while searching for the next
*                  frame and its corresponding size
*
*               2) This routine causes the TCP/IP stack to service received frames in a random order.
*                  In other words, frames that arrive before other frames are not guarrenteed to be
*                  serviced first. This is a result of always searching through the descriptor list from
*                  top to bottom, and not from an expected start point and wrapping around. The benefit to
*                  this approach however is that you dont have to keep state information about where the
*                  next expected SOF flag will be. Nor do you have check if the expected SOF pointer is
*                  correct and make changes should Note 1) effect the correctness of that pointer.
*
*               3) There MUST be at least 1 SOF / EOF combination in the descriptor list, otherwise
*                  interrupt generation would not have occured since interrupt generation takes place
*                  after the complete frame has been trasferred via DMA to RAM.
*********************************************************************************************************
*/

CPU_INT16U  NetNIC_RxPktGetSize (void)
{                                                                       /* WARNING: No code should never be inserted between a  */
    CPU_INT16U     size;                                                /* ptr increment, and the following end of list check   */
    NIC_BUF_DESC  *search_ptr;                                          /* This function contains several pointer increments    */


    NIC_CurrentFrameStart  =  (NIC_BUF_DESC *)0;                        /* Initialize the Current Frame Start pointer to NULL   */
    NIC_CurrentFrameEnd    =  (NIC_BUF_DESC *)0;                        /* Initialize the Current Frame Start pointer to NULL   */

    search_ptr             =  NIC_ExpectedFrameStart;                   /* Begin searching from the expected SOF descriptor     */

    while (((search_ptr->status & EMAC_RXBUF_SOF_MASK) == 0) ||         /* While the expected SOF is NOT pointing to a software */
           ((search_ptr->addr   & EMAC_RXBUF_SW_OWNED) == 0)) {         /* owned SOF descriptor (precautionary code)            */
        search_ptr++;                                                   /* Move the next descriptor in the list                 */

        if (search_ptr    ==  NIC_RxBufDescPtrEnd) {                    /* If we are at the end of the list                     */
            search_ptr     =  NIC_RxBufDescPtrStart;                    /* Wrap around to the beginning                         */
        }
    }
                                                                        /* We now point to a SOF desc that software ownes       */
    NIC_CurrentFrameStart  =  search_ptr;                               /* Save the location of the SOF descriptor              */

    while (((search_ptr->status & EMAC_RXBUF_EOF_MASK) == 0) ||         /* Loop until we find the matching EOF descriptor       */
           ((search_ptr->addr   & EMAC_RXBUF_SW_OWNED) == 0)) {         /* that is software owned                               */
            search_ptr++;

        if (search_ptr    ==  NIC_RxBufDescPtrEnd) {                    /* If we are at the end of the list                     */
            search_ptr     =  NIC_RxBufDescPtrStart;                    /* Wrap around to the beginning                         */
        }

        if (((search_ptr->status & EMAC_RXBUF_SOF_MASK)  > 0) &&        /* If while searching for EOF we find another SW owned  */
            ((search_ptr->addr   & EMAC_RXBUF_SW_OWNED)  > 0)) {        /* SOF, then we must free the incomplete frame desc's   */
            while (NIC_CurrentFrameStart != search_ptr) {               /* Loop from the last found SOF, to the newly found SOF */
                NIC_CurrentFrameStart->addr &= ~EMAC_RXBUF_SW_OWNED;    /* Free a descriptor belonging to the incomplete frame  */
                NIC_CurrentFrameStart++;                                /* Point to the next descriptor that needs to be freed  */
                if (NIC_CurrentFrameStart ==  NIC_RxBufDescPtrEnd) {    /* If we are at the end of the descriptor list          */
                    NIC_CurrentFrameStart  =  NIC_RxBufDescPtrStart;    /* Wrap around to the beginning of the list             */
                }
            }
                                                                        /* Done freeing incomplete frame descriptors            */
            NIC_CurrentFrameStart          =  search_ptr;               /* However, we must now save the new SOF location       */
        }
    }
                                                                        /* We now point to a SW owned  EOF descriptor           */
    NIC_CurrentFrameEnd    =  search_ptr;                               /* Save the location of the EOF descriptor              */

    NIC_ExpectedFrameStart = (search_ptr + 1);                          /* The next expected SOF is in the next descriptor      */
                                                                        /* We store this information now to improve search      */
                                                                        /* performance on the next NetNIC_RxPktGetSize()        */

    if (NIC_ExpectedFrameStart == NIC_RxBufDescPtrEnd) {                /* If the next expected frame points after the end of   */
        NIC_ExpectedFrameStart  = NIC_RxBufDescPtrStart;                /* the list, wrap back around to the beginning          */
    }
                                                                        /* Retrieve the frame size for the frame to be read     */
                                                                        /* next from the buffers using the 'Current' pointers   */
    size                   =  NIC_CurrentFrameEnd->status & EMAC_RXBUF_LEN_MASK;

    return (size);
}

/*
*********************************************************************************************************
*                                            NetNIC_RxPkt()
*
* Description : Read network packet from NIC into buffer.
*
* Argument(s) : ppkt        Pointer to memory buffer to receive NIC packet.
*
*               size        Number of packet frame octets to read into buffer.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE            Packet successfully read.
*                               NET_ERR_INIT_INCOMPLETE     Network initialization NOT complete.
*                               NET_NIC_ERR_NULL_PTR        Argument 'ppkt' passed a NULL pointer.
*                               NET_NIC_ERR_INVALID_SIZE    Invalid size.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_RxTaskHandler().
*
* Note(s)     : 1) NetNIC_RxPkt() blocked until network initialization completes; perform NO action.
*********************************************************************************************************
*/

void  NetNIC_RxPkt (void        *ppkt,
                    CPU_INT16U   size,
                    NET_ERR     *perr)
{
#if (NET_CTR_CFG_STAT_EN     == DEF_ENABLED)
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
#endif


    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit rx (see Note #1).         */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                         /* ------------------- VALIDATE PTR ------------------  */
    if (ppkt == (void *)0) {
       *perr = NET_NIC_ERR_NULL_PTR;
        return;
    }

                                                                        /* ------------------- VALIDATE SIZE -----------------  */
    if (size < NET_IF_FRAME_MIN_SIZE) {
       *perr = NET_NIC_ERR_INVALID_SIZE;
        return;
    }
#endif

    MACB_RxPkt(ppkt, size, perr);                                       /* Read the received Frame from the EMAC buffers        */
    if (*perr != NET_NIC_ERR_NONE) {
        return;
    }

    NET_CTR_STAT_INC(NetNIC_StatRxPktCtr);

    *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                         NetNIC_RxPktDiscard()
*
* Description : Discard network packet from NIC to free NIC packet frames for new receive packets.
*
* Argument(s) : size        Number of packet frame octets.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE            Packet successfully discarded.
*                               NET_ERR_INIT_INCOMPLETE     Network initialization NOT complete.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_RxTaskHandler().
*
* Note(s)     : (1) NetNIC_RxPktDiscard() blocked until network initialization completes; perform NO action.
*********************************************************************************************************
*/

void  NetNIC_RxPktDiscard (CPU_INT16U   size,
                           NET_ERR     *perr)
{
#if (NET_CTR_CFG_STAT_EN     == DEF_ENABLED)
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
#endif


    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit discard (see Note #1).    */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }

    MACB_RxPktDiscard(size);

    NET_CTR_ERR_INC(NetNIC_ErrRxPktDiscardedCtr);

   *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                         NetNIC_TxPktPrepare()
*
* Description : Prepare to transmit data packets from network driver layer to network interface card.
*
* Argument(s) : ppkt        Pointer to memory buffer to transmit NIC packet.
*
*               size        Number of packet frame octets to write to frame.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE            Packet successfully transmitted.
*                               NET_ERR_INIT_INCOMPLETE     Network initialization NOT complete.
*
*                                                           - RETURNED BY NetNIC_TxPktDiscard() : -
*                               NET_ERR_TX                  Transmit error; packet discarded.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_Pkt_Tx().
*
* Note(s)     : (1) NetNIC_TxPktPrepare() blocked until network initialization completes; perform NO action.
*********************************************************************************************************
*/

void  NetNIC_TxPktPrepare (void        *ppkt,
                           CPU_INT16U   size,
                           NET_ERR     *perr)
{
    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit tx (see Note #1).         */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                         /* ------------------- VALIDATE PTR ------------------  */
    if (ppkt == (void *)0) {
        NetNIC_TxPktDiscard(perr);
        return;
    }
#endif

    MACB_TxPktPrepare(ppkt, size, perr);                                /* Prepare to tx pkt to the EMAC.                       */

    if (*perr != NET_NIC_ERR_NONE) {
        NetNIC_TxPktDiscard(perr);
        return;
    }

    *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                            NetNIC_TxPkt()
*
* Description : Transmit data packets from network driver layer to network interface card.
*
* Argument(s) : ppkt        Pointer to memory buffer to transmit NIC packet.
*
*               size        Number of packet frame octets to write to frame.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE            Packet successfully transmitted.
*                               NET_ERR_INIT_INCOMPLETE     Network initialization NOT complete.
*
*                                                           - RETURNED BY NetNIC_TxPktDiscard() : -
*                               NET_ERR_TX                  Transmit error; packet discarded.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_Pkt_Tx().
*
* Note(s)     : (1) NetNIC_TxPkt() blocked until network initialization completes; perform NO action.
*********************************************************************************************************
*/

void  NetNIC_TxPkt (void        *ppkt,
                    CPU_INT16U   size,
                    NET_ERR     *perr)
{
#if (NET_CTR_CFG_STAT_EN     == DEF_ENABLED)
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
#endif


    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit tx (see Note #1).         */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                         /* ------------------- VALIDATE PTR ------------------  */
    if (ppkt == (void *)0) {
        NetNIC_TxPktDiscard(perr);
        return;
    }
#endif

    MACB_TxPkt(ppkt, size, perr);                                       /* Tx pkt to processor.                                 */

    if (*perr != NET_NIC_ERR_NONE) {
        NetNIC_TxPktDiscard(perr);
        return;
    }

    NET_CTR_STAT_INC(NetNIC_StatTxPktCtr);

    *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                           NetNIC_PhyRegRd()
*
* Description : Read PHY data value.
*
* Argument(s) : phy         PHY address, normally 0.
*               reg         PHY register.
*               perr        Pointer to variable that will hold the return error code from this function
*
* Return(s)   : Data read from PHY register.
*
* Caller(s)   : NetNIC_PhyInit(),
*               NetNIC_PhyIsLinkUp().
*
* Note(s)     : Register ALWAYS reads 16-bit data values.
*********************************************************************************************************
*/

CPU_INT16U  NetNIC_PhyRegRd (CPU_INT08U  phy,
                             CPU_INT08U  reg,
                             NET_ERR    *perr)
{
              CPU_INT32U  reg_val;
              CPU_INT08U  retries;
    volatile  CPU_INT32U  i;


    MACB_MAN        = (MACB_MAN_SOF & (1 << 30))                        /* Start of frame code (must be 0x01                    */
                    |  MACB_MAN_READ                                    /* RW flags, must be 0x10 for Read                      */
                    |  MACB_MAN_PHYA(phy)                               /* Specify the address of the PHY to read from          */
                    |  MACB_MAN_REGA(reg)                               /* Supply the addres of the PHY register to read        */
                    |  MACB_MAN_CODE                                    /* IEEE code, must be 0x10                              */
                    |  MACB_MAN_DATA(0);                                /* Dummy data, 0x0000                                   */

    retries         = 50;
                                                                        /* Wait for commmand to finish                          */
    while (((MACB_NSR & MACB_NSR_IDLE) == 0) && retries > 0) {

        for (i = 0; i < 1000; i++) {                                    /* Delay while the read is in progress                  */
            ;
        }
        retries--;
    }

    if (retries == 0) {
       *perr        =  NET_PHY_ERR_REGRD_TIMEOUT;                       /* If a timeout occured, return net nic err timeout    */
        reg_val     = (MACB_MAN & 0x0000FFFF);                          /* Read the management register data                   */
        reg_val     =  0;
    } else {
       *perr        =  NET_PHY_ERR_NONE;
        reg_val     = (MACB_MAN & 0x0000FFFF);                          /* Read the management register data                   */
    }

    return (reg_val);
}

/*
*********************************************************************************************************
*                                           NetNIC_PhyRegWr()
*
* Description : Write PHY data value.
*
* Argument(s) : phy             PHY address, normally 0.
*               reg             PHY register.
*               val             Data to write to PHY register.
*               perr            Pointer to variable that will hold the return error code from this function
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_PhyInit(),
*               NetNIC_PhyIsLinkUp().
*
* Note(s)     : Register ALWAYS writes 16-bit data values.
*********************************************************************************************************
*/

void  NetNIC_PhyRegWr (CPU_INT08U  phy,
                       CPU_INT08U  reg,
                       CPU_INT16U  val,
                       NET_ERR    *perr)
{
              CPU_INT08U   retries;
    volatile  CPU_INT32U  i;


    MACB_MAN        =   (MACB_MAN_SOF & (1 << 30))                      /* Start of frame code (must be 0x01                    */
                    |    MACB_MAN_WRITE                                 /* RW flags, must be 0x01 for Read                      */
                    |    MACB_MAN_PHYA(phy)                             /* Specify the address of the PHY to read from          */
                    |    MACB_MAN_REGA(reg)                             /* Supply the addres of the PHY register to read        */
                    |    MACB_MAN_CODE                                  /* IEEE code, must be 0x10                              */
                    |    MACB_MAN_DATA(val);                            /* Specify the data to be written                       */

    retries         = 3;
                                                                        /* Wait for commmand to finish                          */
    while (((MACB_NSR & MACB_NSR_IDLE) == 0) && retries > 0) {

        for (i = 0; i < 1000; i++) {                                    /* Delay while the write is in progress                 */
            ;
        }
        retries--;
    }

    if (retries == 0) {
       *perr        = NET_PHY_ERR_REGWR_TIMEOUT;                        /* If a timeout occured, return net nic err timeout     */
    } else {
       *perr        = NET_PHY_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        NetNIC_RxISR_Handler()
*
* Description : Signal Network Interface Receive Task that a receive packet is available.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_ISR_Handler().
*
* Note(s)     : (1) NetNIC_ISR_Handler() decodes MACB  Receive ISR & calls NetNIC_RxISR_Handler().
*********************************************************************************************************
*/

static  void  NetNIC_RxISR_Handler (void)
{
    CPU_INT16S  n_new;
    NET_ERR     err;


    n_new = NIC_GetNRdy() - NIC_RxNRdyCtr;                              /* Determine how many NEW packets have been received    */

    while (n_new > 0) {
        NetOS_IF_RxTaskSignal(&err);                                    /* Signal Net IF Rx Task of NIC rx pkt.                 */

        switch (err) {
            case NET_IF_ERR_NONE:
                 if (NIC_RxNRdyCtr < NIC_RX_N_BUFS) {
                     NIC_RxNRdyCtr++;
                 }
                 break;

            case NET_IF_ERR_RX_Q_FULL:
            case NET_IF_ERR_RX_Q_POST_FAIL:
            default:
                 NetNIC_RxPktDiscard(0, &err);                          /* If any net drv signal err, discard rx pkt.           */
                 break;
        }
        n_new--;
    }
}

/*
*********************************************************************************************************
*                                        NetNIC_TxISR_Handler()
*
* Description : (1) Clear transmit interrupt &/or transmit errors :
*
*                   (a) Acknowledge transmit interrupt.
*                   (b) Post transmit FIFO empty signal.
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_ISR_Handler().
*
* Note(s)     : (2) NetNIC_ISR_Handler() decodes AT91SAM9260  Transmit ISR & calls NetNIC_TxISR_Handler().
*********************************************************************************************************
*/

static  void  NetNIC_TxISR_Handler (void)
{
                                                                        /* Interrupts are acknowledged when ISR read.           */
                                                                        /* ISR are previously read in NetNIC_ISR_Handler().     */

                                                                        /* ------------- POST TX COMPLETE SIGNAL -------------  */
    NetOS_NIC_TxRdySignal();
}

/*
*********************************************************************************************************
*                                         NetNIC_TxPktDiscard()
*
* Description : On any Transmit errors, set error.
*
* Argument(s) : pbuf        Pointer to network buffer.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_ERR_TX                  Transmit error; packet discarded.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_TxPkt().
*********************************************************************************************************
*/

static  void  NetNIC_TxPktDiscard (NET_ERR  *perr)
{
#if (NET_CTR_CFG_STAT_EN     == DEF_ENABLED)
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
#endif


    NET_CTR_ERR_INC(NetNIC_ErrTxPktDiscardedCtr);

   *perr = NET_ERR_TX;
}

/*
*********************************************************************************************************
*                                        MACB_Init()
*
* Description : (1) Initialize & start MACB  :
*
*                   (a) Initialize Interrupts.
*                   (b) Initialize Registers.
*                   (c) Initialize MAC Address.
*                   (d) Initialize RX Buffer descriptors.
*                   (e) Initialize Auto Negotiation.
*                   (f) Enable     Receiver/Transmitter.
*                   (g) Initialize External Interrupt Controller.  See Note #4.
*
*
* Argument(s) : perr        Pointer to variable that will hold the return error code from this function :
*
*
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_Init().
*
* Note(s)     : (2) See this 'net_nic.c  MACB REGISTER BITS' for register bit summary.
*
*               (3) (a) Assumes MAC address to set has previously been initialized by
*
*                       (1) MACB's EEPROM                     for EMAC_MAC_ADDR_SEL_EEPROM
*                       (2) Application code                  for EMAC_MAC_ADDR_SEL_CFG
*
*                   (b) The MACB module allow configuration of six MAC addresses that will be
*                       used to accept or reject an ethernet packet.  This driver uses one of these.
*
*               (4) Interrupts MUST be enabled ONLY after ALL network initialization is complete (see also
*                   'net.c  Net_Init()  Note #4c').
*
*********************************************************************************************************
*/

static  void  MACB_Init (NET_ERR *perr)
{
    CPU_INT32U  mck_frq;


    MACB_RxDis();                                                       /* Disable the EMAC receiver                            */
    MACB_TxDis();                                                       /* Disable the EMAC transmitter                         */

    NetBSP_Phy_HW_Init();                                               /* User implemented in net_bsp.c, reset / init the PHY  */
                                                                        /* and configure the necessary IO pins that it req's    */

    MACB_NCR               |=  MACB_NCR_MPE;                            /* Enable the management port. Comm between EMAC/PHY    */

    mck_frq                 =  NetBSP_EMAC_ClkFreq() / 1000;            /* Get the clock frequency                              */

    if (mck_frq        <= 20000) {                                      /* If MCK <= 20MHZ, set MDC to MCK / 8                  */
        mck_frq             =  MACB_NCFGR_CLK_HCLK_8;
    } else if (mck_frq <= 40000) {                                      /* If 20MHZ < MCK <= 40MHZ, set MDC to MCK / 16         */
        mck_frq             =  MACB_NCFGR_CLK_HCLK_16;
    } else if (mck_frq <= 80000) {                                      /* If 40MHZ < MCK <= 80MHZ, set MDC to MCK / 32         */
        mck_frq             =  MACB_NCFGR_CLK_HCLK_32;
    } else {                                                            /* If 80MHZ < MCK <= 160MHZ, set MDC to MCK / 64        */
        mck_frq             =  MACB_NCFGR_CLK_HCLK_64;
    }

    MACB_NCFGR              =  mck_frq;                                 /* Set the MDC CLk divider                              */
    MACB_NCFGR             |=  MACB_NCFGR_DRFCS;                        /* Do NOT copy the FCS to memory during DMA transfer    */

    NetNIC_PhyInit(perr);                                               /* Configure the PHY & get the link state (net_phy.c)   */

#ifdef EMAC_SEL_AVR32                                                   /* If using the AVR32                                   */

#if (EMAC_CFG_RMII == DEF_YES)
    MACB_USRIO              =   (0x02);                                 /* ... Enable RMII mode and set EAM                     */
#else
    MACB_USRIO              =   MACB_USRIO_RMII | (0x02);               /* ... Enable MII mode, and set EAM                     */
#endif

#else                                                                   /* Otherwise (on the AT91SAMs)                          */

#if (EMAC_CFG_RMII == DEF_YES)
    MACB_USRIO              =   MACB_USRIO_RMII | (0x02);               /* ... Enable RMII mode and RMII clk on ERFCK           */
#else
    MACB_USRIO              =  (0x02);                                  /* ... Enable MII mode, and clk ERXCK and ETXCK         */
#endif

#endif
    MACB_RxBufInit();                                                   /* Init Rx buffer descriptors                           */
    MACB_TxBufInit();                                                   /* Init Tx buffer descriptors                           */

                                                                        /* Init MAC Address                                     */
                                                                        /* See Note #3.                                         */
                                                                        /* MAC address is fetched from EMAC EEPROM.             */
#if (EMAC_CFG_MAC_ADDR_SEL == EMAC_MAC_ADDR_SEL_EEPROM)
    reg_val                 = MACB_SA1L;
    NetIF_MAC_Addr[0]       = ((reg_val >>  0) & 0xFF);
    NetIF_MAC_Addr[1]       = ((reg_val >>  8) & 0xFF);
    NetIF_MAC_Addr[2]       = ((reg_val >> 16) & 0xFF);
    NetIF_MAC_Addr[3]       = ((reg_val >> 24) & 0xFF);

    reg_val                 = MACB_SA1H;
    NetIF_MAC_Addr[4]       = ((reg_val >>  0) & 0xFF);
    NetIF_MAC_Addr[5]       = ((reg_val >>  8) & 0xFF);
#else                                                                   /* MAC address is application-configured.               */
    MACB_SA1L               = (NetIF_MAC_Addr[3] << 24) |
                              (NetIF_MAC_Addr[2] << 16) |
                              (NetIF_MAC_Addr[1] <<  8) |
                              (NetIF_MAC_Addr[0]      );
    MACB_SA1H               = (NetIF_MAC_Addr[5] <<  8) |
                              (NetIF_MAC_Addr[4]      );
#endif

    NetIF_MAC_AddrValid     =  DEF_YES;

                                                                        /* Clear receive status register                        */
    MACB_RSR                = (MACB_RSR_OVR | MACB_RSR_REC | MACB_RSR_BNA);

    MACB_IDR                =  0x3FFF;                                  /* Disable all EMAC interrupts                          */
    NetNIC_IntInit();                                                   /* Configure and enable AIC, not EMAC interrupts        */
                                                                        /* EMAC int's are enabled by NetInit -> NetNIC_IntEn    */

    MACB_TxEn();                                                        /* Enable the EMAC transmitter                          */
    MACB_RxEn();                                                        /* Enable the EMAC receiver                             */

    NIC_RxNRdyCtr           =  0;                                       /* No packets require servicing yet                     */

    *perr                   =  NET_NIC_ERR_NONE;                        /* If we made it this far, then no fatal errors         */
}

/*
*********************************************************************************************************
*                                         NIC_GetNRdy()
*
* Description : Determines how many packets we are ready to be processed.
*               This is determined by looping through the entire list of Rx Descriptors
*               and counting the number of present EOF descriptors that are currently owned
*               by software.
*
* Parameters  : None.
*
* Returns     : Number of NIC buffers that are ready to be processed by the stack.
*
* Notes       : 1) In some situations, it is possible to have extra SOF descriptors. This
*                  can happen whenever an EMAC error occurs and cannot complete the write
*                  of additional descriptors into RAM. The EMAC will ONLY recover the last
*                  descriptor that it has attempted to utilize, however, it will not
*                  recover previous descriptors filled with frame data.
*********************************************************************************************************
*/

static  CPU_INT16U  NIC_GetNRdy (void)
{
    CPU_INT16U     n_rdy;
    NIC_BUF_DESC  *search_ptr;


    n_rdy           = 0;
    search_ptr      = NIC_RxBufDescPtrStart;

    while (search_ptr != NIC_RxBufDescPtrEnd) {
        if (((search_ptr->status & EMAC_RXBUF_EOF_MASK) > 0) &&
            ((search_ptr->addr   & EMAC_RXBUF_SW_OWNED) > 0)) {
            n_rdy++;
        }
        search_ptr++;
    }

    return (n_rdy);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                              LOCAL FUNCTIONS: MACB RX FUNCTIONS
*
* Notes: (1) During the processing of a single RX frame, MACB_RxPktGetSize() is first called, followed by
*            either MACB_RxPkt() or MACB_RxPktDiscard().
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            MACB_RxBufInit()
*
* Description: Initializes the Rx buffer descriptors
*
* Parameters : None
*
* Returns    : None
*
* Notes      : 1) Buffer descriptors MUST start on a 32-byte boundary.
*********************************************************************************************************
*/

static  void  MACB_RxBufInit (void)
{
    CPU_INT16U     i;
    NIC_BUF_DESC  *pbufdesc;


    pbufdesc                = (NIC_BUF_DESC *)(((CPU_INT32U)&NIC_RxBufDesc[0] + 32) & 0xFFFFFFE0);
    NIC_RxBufDescPtrStart   = pbufdesc;
    NIC_ExpectedFrameStart  = pbufdesc;

    for (i = 0; i < NIC_RX_N_BUFS; i++) {                               /* Initialize Rx descriptor ring                        */
        pbufdesc->addr      = (((CPU_INT32U)&NIC_RxBuf[i * NIC_RX_BUF_SIZE] + 32) & 0xFFFFFFE0);
        pbufdesc->addr     &= ~EMAC_RXBUF_SW_OWNED;
        pbufdesc->status    =  0;
        pbufdesc++;
    }

    NIC_RxBufDescPtrEnd     =  pbufdesc;                                /* Get the last buffer descriptor in the ring (plus 1)  */
    pbufdesc--;                                                         /* Move back to the last descriptor                     */
    pbufdesc->addr         |=  EMAC_RXBUF_ADD_WRAP;                     /* Set the Wrap bit on the last descriptor in the ring  */

    MACB_RBQP               = (CPU_INT32U)NIC_RxBufDescPtrStart;        /* Configure the EMAC's Rx Descriptor Pointer Register  */
}

/*
*********************************************************************************************************
*                                        MACB_RxEn()
*
* Description : Enable MACB Receiver.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MACB_Init().
*********************************************************************************************************
*/

static  void  MACB_RxEn (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();                                               /* See 'MACB REGISTERS  Note #5b'.                      */

    MACB_NCR      |= MACB_NCR_RE;

    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                        MACB_RxDis()
*
* Description : Disable MACB Receiver.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MACB_Init().
*********************************************************************************************************
*/

static  void  MACB_RxDis (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();                                               /* See 'MACB REGISTERS  Note #5b'.                      */

    MACB_NCR      &= ~MACB_NCR_RE;

    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                       MACB_RxIntEn()
*
* Description : Enable MACB Receiver Interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_IntEn(), NetNIC_RxPktGetSize(), NetNIC_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  MACB_RxIntEn (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();                                               /* See 'MACB REGISTERS  Note #5b'.                     */

    MACB_IER      =  MACB_ISR_RCOMP                                     /* Enable 'Reception complete' interrupt.               */
                  |  MACB_ISR_ROVR;                                     /* Enable 'Receiver overrun' interrupt.                 */

    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                        MACB_RxPkt()
*
* Description : (1) Read network packet from NIC into buffer :
*                   (a) Read received frame from MACB DMA memory space.
*                   (b) Release descriptors once data has been read
*
* Argument(s) : ppkt        Pointer to memory buffer to receive NIC packet.
*               ----        Argument checked in NetNIC_RxPkt().
*
*               size        Number of packet frame octets to read into buffer.
*               ----        Argument checked in NetNIC_RxPkt().
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                Packet successfully transmitted.
*                               NET_ERR_RX                      Error Reading Buffers (EOF not found)
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_RxPkt().
*
* Note(s)     : 1) See 'MACB RECEIVE BUFFER DESCRIPTOR' for packet frame format.
*               2) Operational Description:
*                   a) If we are operating on a descriptor that is both SOF and EOF, we only
*                      execute the code below the while loop.
*
*                   b) If the received frame is contained in 2 descriptors, we ONLY execute the
*                      outside 'if' block within the while loop, AND, the code below the while loop.
*
*                   c) If the received frame fills 3 or more descriptors, we first execute code
*                      within the outside 'if' statement if the while loop once, followed by
*                      (numberOfFilledDescriptors - 1) iterations of the outside 'else' block
*                      within the while loop, and for the very last descriptor, the code below
*                      the while loop.
*********************************************************************************************************
*/

static  void  MACB_RxPkt (void        *ppkt,
                          CPU_INT16U   size,
                          NET_ERR     *perr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR       cpu_sr;
#endif
    CPU_INT16U    sizeLeftToCopy;
    CPU_INT08U    dataOffset;
    CPU_INT08U   *pStackBuf;
    CPU_INT08U   *pEMACBuf;
    NIC_BUF_DESC *searchPtr;
    CPU_BOOLEAN   doMemCpy;


   *perr               =  NET_NIC_ERR_NONE;                             /* Begin with no error code being returned              */

    doMemCpy           =  DEF_TRUE;                                     /* Error Control. If an error occurs, we can stop       */
                                                                        /* performing mem copies, will still executing the loop */
                                                                        /* freeing the remaining descriptors without            */
                                                                        /* accidently overwriting other memory                  */

    sizeLeftToCopy     =  size;                                         /* Make a copy of size for debugging purposes           */
    searchPtr          =  NIC_CurrentFrameStart;                        /* Point to the SOF descriptor. Start copy from here    */
    pStackBuf          = (CPU_INT08U *)ppkt;                            /* Point to the TCP/IP stacks buffer, copy into here    */

                                                                        /* Determine the data offset for the first descriptor   */
    dataOffset         = (searchPtr->status & EMAC_RXBUF_OFF_MASK) >> 12;

    while (searchPtr  !=  NIC_CurrentFrameEnd) {                        /* Copy from SOF to EOF descriptor (EOF not inclusive)  */
                                                                        /* Get the address of the buffer we wish to copy        */
        pEMACBuf       = (CPU_INT08U *)((searchPtr->addr & EMAC_RXBUF_ADDRESS_MASK));		

        if (searchPtr ==  NIC_CurrentFrameStart) {                      /* If its the SOF, include the offset in the copy size  */
            if (doMemCpy == DEF_TRUE) {
                Mem_Copy((void *)(pStackBuf), (void *)(pEMACBuf + dataOffset), NIC_RX_BUF_SIZE - dataOffset);
                pStackBuf += (NIC_RX_BUF_SIZE - dataOffset);            /* Advance the pointer into the TCP/IP stacks buffer    */
            }
            if ((sizeLeftToCopy - NIC_RX_BUF_SIZE - dataOffset) >= 0) { /* Subtract the amount copied from the total left       */
                sizeLeftToCopy -= (NIC_RX_BUF_SIZE - dataOffset);
                dataOffset      =   0;                                  /* Set the offset to zero since we processed the SOF    */
            } else {
                sizeLeftToCopy  =  0;                                   /* An error occured!                                    */
               *perr            =  NET_ERR_RX;                          /* Return an error code!                                */
                doMemCpy        =  DEF_FALSE;                           /* Disable mem copies, so we can free descriptors       */
            }
        } else {                                                        /* If its not SOF, copy a full buffer to the stacks buf */
            if (doMemCpy == DEF_TRUE) {
                Mem_Copy((void *)(pStackBuf), (void *)(pEMACBuf), NIC_RX_BUF_SIZE);
                pStackBuf += (NIC_RX_BUF_SIZE);                         /* Advance the pointer into the TCP/IP stacks buffer    */
            }
            if ((sizeLeftToCopy - NIC_RX_BUF_SIZE) >= 0) {              /* Subtract the amount copied from the total left       */
                sizeLeftToCopy -= (NIC_RX_BUF_SIZE);
            } else {
                sizeLeftToCopy  = 0;                                    /* An error occured!                                    */
               *perr            =  NET_ERR_RX;                          /* Return an error code!                                */
                doMemCpy        =  DEF_FALSE;                           /* Disable mem copies, so we can free descriptors       */
            }
        }

        searchPtr->addr &= ~EMAC_RXBUF_SW_OWNED;                        /* Free the descriptor we just copied from              */

        searchPtr++;                                                    /* Go to the next descriptor                            */

        if (searchPtr == NIC_RxBufDescPtrEnd) {                         /* If we have reached the end of the list, wrap around  */
            searchPtr  = NIC_RxBufDescPtrStart;                         /* to the top of the list and continue searching        */
        }
    }
                                                                        /* COPY THE LAST DESCRIPTOR BUFFER DATA                 */
                                                                        /* Get the address of the buffer we wish to copy        */
    pEMACBuf       = (CPU_INT08U *)((searchPtr->addr & EMAC_RXBUF_ADDRESS_MASK));		

    if (doMemCpy == DEF_TRUE) {                                         /* If no errors have occured, copy the remaining data   */
        if (dataOffset > 0) {                                           /* If this is both the SOF and EOF descriptor           */
            Mem_Copy((void *)(pStackBuf), (void *)(pEMACBuf + dataOffset), sizeLeftToCopy - dataOffset);
        } else {                                                        /* If this is the last descriptor, and NOT also SOF     */
            Mem_Copy((void *)(pStackBuf), (void *)(pEMACBuf), sizeLeftToCopy);
        }
    }

    searchPtr->addr &= ~EMAC_RXBUF_SW_OWNED;                            /* Free the last descriptor                             */

    CPU_CRITICAL_ENTER();
    if (NIC_RxNRdyCtr > 0) {                                            /* One less packet to process                           */
        NIC_RxNRdyCtr--;
    }
    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                    MACB_RxPktDiscard()
*
* Description : Discard network packet from NIC to free NIC packet frames for new receive packets.
*
* Argument(s) : Size: how much data to discard. However, we will discard an entire frame
*               so this information is not necessary.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_RxPktDiscard().
*********************************************************************************************************
*/

static  void  MACB_RxPktDiscard (CPU_INT16U  size)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR  cpu_sr = 0;
#endif
    NIC_BUF_DESC  *search_ptr;


    (void)size;

    if ((NIC_CurrentFrameStart == (NIC_BUF_DESC *)0) || (NIC_CurrentFrameEnd == (NIC_BUF_DESC *)0)) {
        CPU_CRITICAL_ENTER();
        if (NIC_RxNRdyCtr > 0) {                                        /* One less packet to process                           */
            NIC_RxNRdyCtr--;
        }
        CPU_CRITICAL_EXIT();
        return;
    }

    search_ptr         =  NIC_CurrentFrameStart;                        /* Point to the SOF descriptor. Start copy from here    */

    while (search_ptr  !=  NIC_CurrentFrameEnd) {                       /* Copy from SOF to EOF descriptor (EOF not inclusive)  */
        search_ptr->addr &= ~EMAC_RXBUF_SW_OWNED;                       /* Free the descriptor                                  */
        search_ptr++;                                                   /* Go to the next descriptor                            */
        if (search_ptr == NIC_RxBufDescPtrEnd) {                        /* If we have reached the end of the list, wrap around  */
            search_ptr  = NIC_RxBufDescPtrStart;                        /* to the top of the list and continue searching        */
        }
    }

    search_ptr->addr &= ~EMAC_RXBUF_SW_OWNED;                           /* Free the last descriptor                             */

    CPU_CRITICAL_ENTER();
    if (NIC_RxNRdyCtr > 0) {                                            /* One less packet to process                           */
        NIC_RxNRdyCtr--;
    }
    CPU_CRITICAL_EXIT();
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                              LOCAL FUNCTIONS: MACB TX FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             MACB_TxBufInit()
*
* Description: Initializes the Tx buffer descriptors
*
* Parameters : None
*
* Returns    : None
*
* Notes      : 1) Buffer descriptors MUST start on a 32-byte boundary.
*********************************************************************************************************
*/

static  void  MACB_TxBufInit (void)
{
    NIC_TxBufDescPtr            = (NIC_BUF_DESC *)(((CPU_INT32U)&NIC_TxBufDesc[0] + 32) & 0xFFFFFFE0);

    NIC_TxBufDescPtr->status   |=  EMAC_TXBUF_STATUS_USED;              /* The descriptor is ready for software use after init  */
    NIC_TxBufDescPtr->status   |=  EMAC_TXBUF_ADD_WRAP;                 /* Set the Wrap bit on the last (only) descriptor       */

    MACB_TBQP                  = (CPU_INT32U)NIC_TxBufDescPtr;         /* Configure the EMAC's Tx Descriptor Pointer Register  */
}

/*
*********************************************************************************************************
*                                        MACB_TxEn()
*
* Description : Enable MACB Transmitter.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : EMAC_Init().
*********************************************************************************************************
*/

static  void  MACB_TxEn (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();                                               /* See 'MACB REGISTERS  Note #5b'.                     */

    MACB_NCR      |= MACB_NCR_TE;                                       /* Enable the transmitter                               */

    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                        MACB_TxDis()
*
* Description : Disable MACB Transmitter.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MACB_Init().
*********************************************************************************************************
*/

static  void  MACB_TxDis (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();                                              /* See 'MACB REGISTERS  Note #5b'.                       */

    MACB_NCR      &= ~MACB_NCR_TE;                                      /* Disable the transmitter                               */

    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                       MACB_TxIntEn()
*
* Description : Enable MACB Transmit Interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_IntEn().
*********************************************************************************************************
*/

static  void  MACB_TxIntEn (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();                                               /* See 'MACB REGISTERS  Note #5b'.                     */

    MACB_IER      =  MACB_ISR_TCOMP                                     /* Enable Tx complete interrupt                         */
                  |  MACB_ISR_TXUBR;                                    /* Enable Tx bit used interrupt                         */

    CPU_CRITICAL_EXIT();
}

/*
*********************************************************************************************************
*                                    EMAC_TxPktPrepare()
*
* Description : (1) This funciton is empty
*
* Argument(s) : ppkt        Pointer to memory buffer to transmit NIC packet.
*               ----        Argument checked in NetNIC_TxPkt().
*
*               size        Number of packet frame octets to write to frame.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                Packet successfully transmitted.
* Return(s)   : none.
*
* Caller(s)   : NetNIC_TxPktPrepare().
*********************************************************************************************************
*/

static void  MACB_TxPktPrepare (void        *ppkt,
                                CPU_INT16U   size,
                                NET_ERR     *perr)
{
   *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                        MACB_TxPkt()
*
* Description : (1) Instruct MACB to send network packet :
*                   (a) Check is transmitter ready.
*                   (b) Clear all transmitter errors.
*                   (c) Inform transmitter about buffer address and size.
*                       This starts actual transmission of the packet.
*
* Argument(s) : ppkt        Pointer to memory buffer to transmit NIC packet.
*               ----        Argument checked in NetNIC_TxPkt().
*
*               size        Number of packet frame octets to write to frame.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                Packet successfully transmitted.
*                               EMAC_ERR_TX_BUSY                Transmitter not ready.
* Return(s)   : none.
*
* Caller(s)   : NetNIC_TxPkt().
*
* Notes       : (1) If the Tx descriptor is NOT available,  a BUSY return code will be sent to the stack
*                   and retransmission will be attemped at a later time. - Transmitting the descriptor
*                   content is VERY fast, therefore, the descriptor should always be available.
*********************************************************************************************************
*/

static  void  MACB_TxPkt (void        *ppkt,
                          CPU_INT16U   size,
                          NET_ERR     *perr)
{
    CPU_INT32U    reg_val;

                                                                        /* Check if transmitter ready.                          */
    reg_val             = MACB_TSR;

    if ((reg_val & MACB_TSR_TGO) == MACB_TSR_TGO) {                     /* If bit AT91C_EMAC_TGO == 1, then tranmitter is busy  */
        *perr           = EMAC_ERR_TX_BUSY;
        return;
    }
                                                                        /* Clear all transmitter errors.                        */
    MACB_TSR           = MACB_TSR_UBR
                       | MACB_TSR_COL
                       | MACB_TSR_RLES
                       | MACB_TSR_BEX
                       | MACB_TSR_COMP
                       | MACB_TSR_UND;


    if ((NIC_TxBufDescPtr->status & EMAC_TXBUF_STATUS_USED) > 0) {      /* If the Tx descriptor is availble for use             */
        NIC_TxBufDescPtr->addr          = (CPU_INT32U)ppkt;             /* Set the address of the buffer to transmit            */
                                                                        /* Set the descriptor status field to the buffer size   */
                                                                        /* and set the 'last buffer for this frame' flag        */
                                                                        /* Since there is one descriptor, set the WRAP bit      */
        NIC_TxBufDescPtr->status        = EMAC_TXBUF_ADD_LAST | EMAC_TXBUF_ADD_WRAP | (size & EMAC_TXBUF_TX_SIZE_MASK);

        MACB_NCR                       |= MACB_NCR_TSTART;              /* Start the transmission                               */
       *perr                            = NET_NIC_ERR_NONE;             /* Return no error, transmission in progress            */
    } else {
       *perr                            = EMAC_ERR_TX_BUSY;             /* If the descriptor is not available, return BUSY      */
    }                                                                   /* See Note 1) above.                                   */
}
