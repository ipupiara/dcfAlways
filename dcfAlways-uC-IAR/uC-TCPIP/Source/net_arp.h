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
*                                          NETWORK ARP LAYER
*                                    (ADDRESS RESOLUTION PROTOCOL)
*
* Filename      : net_arp.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
* Note(s)       : (1) Address Resolution Protocol ONLY required for network interfaces that require
*                     network-address-to-hardware-address bindings (see RFC #826 'Abstract').
*
*                 (2) Supports Address Resolution Protocol as described in RFC #826 with the following
*                     restrictions/constraints :
*
*                     (a) ONLY supports the following hardware types :
*                         (1) 48-bit Ethernet
*
*                     (b) ONLY supports the following protocol types :
*                         (1) 32-bit IP
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) ARP Layer is required for some network interfaces (see 'net_arp.h  Note #1).
*
*           (2) The following ARP-module-present configuration value MUST be pre-#define'd in 'net_cfg_net.h'
*               PRIOR to all other network modules that require ARP Layer Configuration (see  'net_cfg_net.h
*               ARP LAYER CONFIGURATION  Note #2b' & 'net_cfg_net.h  NETWORK INTERFACE LAYER CONFIGURATION
*               Note #4') :
*
*                   NET_ARP_MODULE_PRESENT
*********************************************************************************************************
*/

#ifdef   NET_ARP_MODULE_PRESENT                                 /* See Note #2.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_ARP_MODULE
#define  NET_ARP_EXT
#else
#define  NET_ARP_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          ARP CACHE DEFINES
*********************************************************************************************************
*/

#define  NET_ARP_CACHE_TIMEOUT_MIN_SEC             ( 1L * 60L)  /* Cache timeout min  =  60 seconds                     */
#define  NET_ARP_CACHE_TIMEOUT_MAX_SEC             (10L * 60L)  /* Cache timeout max  = 600 seconds                     */
#define  NET_ARP_CACHE_TIMEOUT_DFLT_SEC            (10L * 60L)  /* Cache timeout dflt = 600 seconds                     */

#define  NET_ARP_CACHE_ACCESSED_TH_MIN                    10u
#define  NET_ARP_CACHE_ACCESSED_TH_MAX                 65000u
#define  NET_ARP_CACHE_ACCESSED_TH_DFLT                  100u


/*
*********************************************************************************************************
*                                         ARP REQUEST DEFINES
*
* Note(s) : (1) RFC #1122, Section 2.3.2.1 states that "a mechanism to prevent ARP flooding (repeatedly
*               sending an ARP Request for the same IP address, at a high rate) MUST be included.  The
*               recommended maximum rate is 1 per second per destination".
*********************************************************************************************************
*/

#define  NET_ARP_REQ_RETRY_MIN                             0
#define  NET_ARP_REQ_RETRY_MAX                             5
#define  NET_ARP_REQ_RETRY_DFLT                            3
                                                                /* ARP req retry timeouts (see Note #1).                */
#define  NET_ARP_REQ_RETRY_TIMEOUT_MIN_SEC                 1L   /* ARP req retry timeout min  =  1 seconds              */
#define  NET_ARP_REQ_RETRY_TIMEOUT_MAX_SEC                10L   /* ARP req retry timeout max  = 10 seconds              */
#define  NET_ARP_REQ_RETRY_TIMEOUT_DFLT_SEC                5L   /* ARP req retry timeout dflt =  5 seconds              */


/*$PAGE*/
/*
*********************************************************************************************************
*                                          ARP FLAG DEFINES
*********************************************************************************************************
*/

                                                                /* ------------------ NET ARP FLAGS ------------------- */
#define  NET_ARP_FLAG_NONE                        DEF_BIT_NONE
#define  NET_ARP_FLAG_USED                        DEF_BIT_00    /* ARP cache cur used; i.e. NOT in free cache pool.     */


/*
*********************************************************************************************************
*                                          ARP TYPE DEFINES
*
* Note(s) : (1) NET_ARP_TYPE_??? #define values specifically chosen as ASCII representations of the ARP
*               types.  Memory displays of ARP types will display the ARP TYPE with the chosen ASCII name.
*********************************************************************************************************
*/

                                                                /* ------------------ NET ARP TYPES ------------------- */
#if     (CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG)
#define  NET_ARP_TYPE_NONE                        0x4E4F4E45    /* "NONE" in ASCII.                                     */
#define  NET_ARP_TYPE_CACHE                       0x41525020    /* "ARP " in ASCII.                                     */

#else

#if     (CPU_CFG_DATA_SIZE   == CPU_WORD_SIZE_32)
#define  NET_ARP_TYPE_NONE                        0x454E4F4E    /* "NONE" in ASCII.                                     */
#define  NET_ARP_TYPE_CACHE                       0x20505241    /* "ARP " in ASCII.                                     */

#elif   (CPU_CFG_DATA_SIZE   == CPU_WORD_SIZE_16)
#define  NET_ARP_TYPE_NONE                        0x4F4E454E    /* "NONE" in ASCII.                                     */
#define  NET_ARP_TYPE_CACHE                       0x52412050    /* "ARP " in ASCII.                                     */

#else                                                           /* Dflt CPU_WORD_SIZE_08.                               */
#define  NET_ARP_TYPE_NONE                        0x4E4F4E45    /* "NONE" in ASCII.                                     */
#define  NET_ARP_TYPE_CACHE                       0x41525020    /* "ARP " in ASCII.                                     */

#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                   ARP HARDWARE & PROTOCOL DEFINES
*
* Note(s) : (1) The following ARP values MUST be pre-#define'd in 'net_def.h' PRIOR to 'net_cfg.h' so 
*               that the developer can configure ARP for the correct network interface link layer &
*               network protocol layer (see 'net_def.h  ARP LAYER DEFINES' & 'net_cfg_net.h  ARP LAYER
*               CONFIGURATION') :
*
*                                                                       ---- SUPPORTED ARP HW TYPES ----
*                   NET_ARP_HW_TYPE_NONE                      0x0000
*
*                   NET_ARP_HW_TYPE_ETHER                     0x0001
*                   NET_ARP_HW_ADDR_LEN_ETHER                      6
*
*                                                                   
*                                                                       - SUPPORTED ARP PROTOCOL TYPES -
*                   NET_ARP_PROTOCOL_TYPE_NONE                0x0000
*                                                             
*                   NET_ARP_PROTOCOL_TYPE_IP_V4               0x0800
*                   NET_ARP_PROTOCOL_ADDR_LEN_IP_V4                4
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                    ARP HEADER / MESSAGE DEFINES
*
* Note(s) : (1) See RFC #826, Section 'Packet Format' for ARP packet header format.
*
*               (a) ARP header includes two pairs of hardware & protocol type addresses--one each for the 
*                   sender & the target.
*********************************************************************************************************
*/

#define  NET_ARP_HDR_SIZE_MIN                             12
#define  NET_ARP_HDR_SIZE_MAX                             68
#define  NET_ARP_HDR_SIZE_BASE                             8
                                                                /* See Note #1a.                                        */
#define  NET_ARP_HDR_SIZE                               (NET_ARP_HDR_SIZE_BASE              + \
                                                        (NET_ARP_CFG_HW_ADDR_LEN       * 2) + \
                                                        (NET_ARP_CFG_PROTOCOL_ADDR_LEN * 2))


#define  NET_ARP_HDR_OP_REQ                           0x0001
#define  NET_ARP_HDR_OP_REPLY                         0x0002


#define  NET_ARP_MSG_LEN                                 NET_ARP_HDR_SIZE
#define  NET_ARP_MSG_LEN_DATA                              0


/*$PAGE*/
/*
*********************************************************************************************************
*                                          ARP CACHE STATES
*
*                          (1)                             (2)
*                            ARP REQUEST                     ARP REPLY
*              ----------    FETCHES NEW     -----------     RESOLVES       ------------
*              |        |     ARP CACHE      |         |     ARP CACHE      |          |
*              |  FREE  | -----------------> | PENDING | -----------------> | RESOLVED |
*              |        |                    |         |                    |          |
*              ----------  (3)               -----------                    ------------
*                 ^  ^       ARP REQUEST          |                               |
*                 |  |        TIMES OUT           |                               |
*                 |  |      AFTER RETRIES         |        (4)                    |
*                 |  ------------------------------          ARP CACHE            |
*                 |                                          TIMES OUT            |
*                 -----------------------------------------------------------------
*
*
* Note(s) : (1) ARP cache lookup fails to find the ARP cache with corresponding protocol address.  A new
*               ARP cache is allocated from the ARP cache pool & inserted into the ARP Cache List in the
*               'PENDING' state.  An ARP Request is generated & transmitted to resolve the pending ARP cache.
*
*           (2) An ARP Reply resolves the pending ARP cache's hardware address.
*
*           (3) Alternatively, no corresponding ARP Reply is received after the maximum number of ARP
*               Request retries & the ARP cache is freed.
*
*           (4) ARP cache times out & is freed.
*********************************************************************************************************
*/

#define  NET_ARP_CACHE_STATE_NONE                  0
#define  NET_ARP_CACHE_STATE_FREE                  1
#define  NET_ARP_CACHE_STATE_PEND                  2
#define  NET_ARP_CACHE_STATE_RESOLVED              3


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                    ARP CACHE QUANTITY DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_ARP_CACHE_QTY;                         /* Defines max qty of ARP caches to support.            */


/*
*********************************************************************************************************
*                                     ARP CACHE STATE DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT08U  NET_ARP_CACHE_STATE;


/*
*********************************************************************************************************
*                                    ARP ADDRESS LENGTH DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT08U  NET_ARP_ADDR_LEN;


/*$PAGE*/
/*
*********************************************************************************************************
*                                            ARP HEADER
*
* Note(s) : (1) See RFC #826, Section 'Packet Format' for ARP packet header format.
*
*           (2) See 'ARP HEADER / MESSAGE DEFINES'.
*********************************************************************************************************
*/

                                                                        /* --------------- NET ARP HDR ---------------- */
typedef  struct  net_arp_hdr {
    CPU_INT16U      HW_Type;                                            /* ARP pkt hw       type.                       */
    CPU_INT16U      ProtocolType;                                       /* ARP pkt protocol type.                       */

    CPU_INT08U      HW_AddrLen;                                         /* ARP pkt hw       addr len (in octets).       */
    CPU_INT08U      ProtocolAddrLen;                                    /* ARP pkt protocol addr len (in octets).       */

    CPU_INT16U      OpCode;                                             /* ARP op code (see Note #2).                   */

    CPU_INT08U      HW_AddrSender[NET_ARP_CFG_HW_ADDR_LEN];             /* Sender hw       addr.                        */
    CPU_INT08U      ProtocolAddrSender[NET_ARP_CFG_PROTOCOL_ADDR_LEN];  /* Sender protocol addr.                        */

    CPU_INT08U      HW_AddrTarget[NET_ARP_CFG_HW_ADDR_LEN];             /* Target hw       addr.                        */
    CPU_INT08U      ProtocolAddrTarget[NET_ARP_CFG_PROTOCOL_ADDR_LEN];  /* Target protocol addr.                        */
} NET_ARP_HDR;


/*$PAGE*/
/*
*********************************************************************************************************
*                                      ARP CACHE ENTRY DATA TYPE
*
*                                  NET_ARP_CACHE
*                                 |-------------|
*                                 | Cache Type  |
*                      Previous   |-------------|
*                       Cache <----------O      |
*                                 |-------------|     Next
*                                 |      O----------> Cache      Buffer Queue
*                                 |-------------|                    Head       -------
*                                 |      O------------------------------------> |     |
*                                 |-------------|                               |     |
*                                 |      O----------------------                -------
*                                 |-------------|              |                  | ^
*                                 |      O----------> Cache    |                  v |
*                                 |-------------|     Timer    |                -------
*                                 |  Hardware:  |              |                |     |
*                                 |    Type     |              |                |     |
*                                 |   Length    |              |                -------
*                                 |   Address   |              |                  | ^
*                                 |-------------|              | Buffer Queue     v |
*                                 |  Protocol:  |              |     Tail       -------
*                                 |    Type     |              ---------------> |     |
*                                 |   Length    |                               |     |
*                                 |   Address   |                               -------
*                                 |-------------|
*                                 |    Flags    |
*                                 |-------------|
*                                 |   Accessed  |
*                                 |   Counter   |
*                                 |-------------|
*                                 |   Request   |
*                                 |   Counter   |
*                                 |-------------|
*                                 |    State    |
*                                 |-------------|
*
*
* Note(s) : (1) Forced word-alignment at start of ARP cache NOT required since first data member 
*               'Type' is declared as 'CPU_INT32U'.
*
*           (2) 'PrevPtr'/'NextPtr' ideally declared as 'NET_ARP_CACHE' pointers; declared as 'void' 
*               pointers because 'NET_ARP_CACHE' NOT fully defined at time of declaration.
*
*           (3) These fields are configured by the developer via 'net_cfg_net.h' at compile time.
*
*               See 'net_arp.h  Note #2' & 'ARP HARDWARE & PROTOCOL DEFINES  Note #1' for supported
*               hardware & protocol types.
*********************************************************************************************************
*/

                                                                        /* --------------- NET ARP CACHE -------------- */
typedef  struct  net_arp_cache {
    NET_TYPE              Type;                                         /* Type cfg'd @ init : NET_ARP_TYPE_CACHE.      */

    void                 *PrevPtr;                                      /* Ptr to PREV ARP cache.                       */
    void                 *NextPtr;                                      /* Ptr to NEXT ARP cache.                       */

    NET_BUF              *BufQ_Head;                                    /* Ptr to head of cache's buf Q.                */
    NET_BUF              *BufQ_Tail;                                    /* Ptr to tail of cache's buf Q.                */

    NET_TMR              *TmrPtr;                                       /* Ptr to cache TMR.                            */


    CPU_INT16U            HW_Type;                                      /* HW type           (see Note #3).             */
    CPU_INT08U            HW_AddrLen;                                   /* HW addr len       (see Note #3).             */
    CPU_INT08U            HW_Addr[NET_ARP_CFG_HW_ADDR_LEN];             /* HW addr.                                     */

    CPU_INT16U            ProtocolType;                                 /* Protocol type     (see Note #3).             */
    CPU_INT08U            ProtocolAddrLen;                              /* Protocol addr len (see Note #3).             */
    CPU_INT08U            ProtocolAddr[NET_ARP_CFG_PROTOCOL_ADDR_LEN];  /* Protocol addr.                               */


    CPU_INT16U            Flags;                                        /* ARP cache flags.                             */


    CPU_INT16U            AccessedCtr;                                  /* Nbr successful srchs.                        */
    CPU_INT08U            ReqAttemptsCtr;                               /* ARP req attempts ctr.                        */


    NET_ARP_CACHE_STATE   State;                                        /* Cur cache state.                             */
} NET_ARP_CACHE;


/*$PAGE*/
/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

NET_ARP_EXT  CPU_INT08U     *NetARP_HostAddrPtrHW;                      /* Ptr to this host's hw       addr.            */
NET_ARP_EXT  CPU_INT08U     *NetARP_HostAddrPtrProtocol;                /* Ptr to this host's protocol addr.            */

NET_ARP_EXT  CPU_BOOLEAN     NetARP_HostAddrProtocolConflict;           /* Indicates a protocol addr conflict between   */
                                                                        /* ... this host & other host(s) on local net.  */

NET_ARP_EXT  NET_ARP_CACHE   NetARP_CacheTbl[NET_ARP_CFG_NBR_CACHE];
NET_ARP_EXT  NET_ARP_CACHE  *NetARP_CachePoolPtr;                       /* Ptr to pool of free ARP caches.              */
NET_ARP_EXT  NET_STAT_POOL   NetARP_CachePoolStat;

#if (NET_CTR_CFG_STAT_EN  == DEF_ENABLED)
NET_ARP_EXT  NET_CTR         NetARP_CacheSrchCtr_lo;                    /* Inc'd for EVERY cache lookup (lo ctr).       */
NET_ARP_EXT  NET_CTR         NetARP_CacheSrchCtr_hi;                    /*                 Cache lookup  hi ctr.        */
NET_ARP_EXT  NET_CTR         NetARP_CacheFoundCtr_lo;                   /* Inc'd for EVERY cache found  (lo ctr).       */
NET_ARP_EXT  NET_CTR         NetARP_CacheFoundCtr_hi;                   /*                 Cache found   hi ctr.        */
NET_ARP_EXT  CPU_INT08U      NetARP_CacheFoundPct;                      /* Found % = (FoundCtr / SrchCtr) * 100%        */
#endif

NET_ARP_EXT  NET_ARP_CACHE  *NetARP_CacheListHead;                      /* Ptr to head of ARP Cache List.               */
NET_ARP_EXT  NET_ARP_CACHE  *NetARP_CacheListTail;                      /* Ptr to tail of ARP Cache List.               */


NET_ARP_EXT  CPU_INT16U      NetARP_CacheTimeout_sec;                   /* ARP cache timeout              (in secs ).   */
NET_ARP_EXT  NET_TMR_TICK    NetARP_CacheTimeout_tick;                  /* ARP cache timeout              (in ticks).   */

NET_ARP_EXT  CPU_INT16U      NetARP_CacheAccessedTh_nbr;                /* Nbr successful srch's to promote ARP cache.  */

NET_ARP_EXT  CPU_INT08U      NetARP_ReqMaxAttempts_nbr;                 /* ARP req max nbr of attempts.                 */
NET_ARP_EXT  CPU_INT08U      NetARP_ReqTimeout_sec;                     /* ARP req wait-for-reply timeout (in secs ).   */
NET_ARP_EXT  NET_TMR_TICK    NetARP_ReqTimeout_tick;                    /* ARP req wait-for-reply timeout (in ticks).   */


#if (NET_CTR_CFG_STAT_EN  == DEF_ENABLED)                               /* --------------- NET ARP STATS -------------- */
NET_ARP_EXT  NET_CTR         NetARP_StatRxPktCtr;                       /* Nbr rx'd       pkts.                         */
NET_ARP_EXT  NET_CTR         NetARP_StatRxMsgProcessedCtr;              /* Nbr rx'd       msgs successfully processed.  */
NET_ARP_EXT  NET_CTR         NetARP_StatRxMsgReqProcessedCtr;           /* Nbr rx'd req   msgs successfully processed.  */
NET_ARP_EXT  NET_CTR         NetARP_StatRxMsgReplyProcessedCtr;         /* Nbr rx'd reply msgs successfully processed.  */

NET_ARP_EXT  NET_CTR         NetARP_StatTxMsgCtr;                       /* Nbr tx'd       msgs.                         */
NET_ARP_EXT  NET_CTR         NetARP_StatTxMsgReqCtr;                    /* Nbr tx'd req   msgs.                         */
NET_ARP_EXT  NET_CTR         NetARP_StatTxMsgReplyCtr;                  /* Nbr tx'd reply msgs.                         */
#endif


#if (NET_CTR_CFG_ERR_EN   == DEF_ENABLED)                               /* --------------- NET ARP ERRS --------------- */
NET_ARP_EXT  NET_CTR         NetARP_ErrNoneAvailCtr;                    /* Nbr ARP cache unavail accesses.              */


NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrHW_TypeCtr;                 /* Nbr rx'd msgs with invalid hw       type.    */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrHW_AddrLenCtr;              /* Nbr rx'd msgs with invalid hw       addr len.*/
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrHW_AddrCtr;                 /* Nbr rx'd msgs with invalid hw       addr.    */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrProtocolTypeCtr;            /* Nbr rx'd msgs with invalid protocol type.    */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrProtocolAddrLenCtr;         /* Nbr rx'd msgs with invalid protocol addr len.*/
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrProtocolAddrCtr;            /* Nbr rx'd msgs with invalid protocol addr.    */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrOpCodeCtr;                  /* Nbr rx'd msgs with invalid op code.          */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrOpAddrCtr;                  /* Nbr rx'd msgs with invalid op code/addr.     */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxHdrMsgLenCtr;                  /* Nbr rx'd msgs with invalid msg len.          */

NET_ARP_EXT  NET_CTR         NetARP_ErrRxPktTargetReplyCtr;             /* Nbr rx'd msgs for  invalid reply msg dest.   */
NET_ARP_EXT  NET_CTR         NetARP_ErrRxPktTargetNotThisHostCtr;       /* Nbr rx'd msgs for  invalid           dest.   */

NET_ARP_EXT  NET_CTR         NetARP_ErrRxPktDiscardedCtr;               /* Nbr rx'd msgs discarded.                     */


NET_ARP_EXT  NET_CTR         NetARP_ErrTxPktDiscardedCtr;               /* Nbr tx   msgs discarded.                     */


#if ((NET_ERR_CFG_ARG_CHK_EXT_EN == DEF_ENABLED) || \
     (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED))
NET_ARP_EXT  NET_CTR         NetARP_ErrNullPtrCtr;                      /* Nbr null   ARP ptr   accesses.               */
#endif

#if  (NET_ERR_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
NET_ARP_EXT  NET_CTR         NetARP_ErrInvalidAddrLenHW_Ctr;            /* Nbr accesses with invalid hw       addr len. */
NET_ARP_EXT  NET_CTR         NetARP_ErrInvalidAddrLenProtocolCtr;       /* Nbr accesses with invalid protocol addr len. */
#endif

#if  (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
NET_ARP_EXT  NET_CTR         NetARP_ErrNotUsedCtr;                      /* Nbr unused ARP cache       accesses.         */

NET_ARP_EXT  NET_CTR         NetARP_ErrInvalidTypeCtr;                  /* Nbr ARP cache invalid type accesses.         */


NET_ARP_EXT  NET_CTR         NetARP_ErrRxInvalidBufIxCtr;               /* Nbr rx   pkts with invalid buf ix.           */

NET_ARP_EXT  NET_CTR         NetARP_ErrTxHdrOpCodeCtr;                  /* Nbr tx   msgs with invalid op code.          */
#endif

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void               NetARP_Init                     (void);


                                                                                    /* ---------- CFG FNCTS ----------- */
CPU_BOOLEAN        NetARP_CfgCacheTimeout          (CPU_INT16U         timeout_sec);

CPU_BOOLEAN        NetARP_CfgCacheAccessedTh       (CPU_INT16U         nbr_access);

CPU_BOOLEAN        NetARP_CfgReqTimeout            (CPU_INT08U         timeout_sec);

CPU_BOOLEAN        NetARP_CfgReqMaxRetries         (CPU_INT08U         max_nbr_retries);


                                                                                    /* --------- STATUS FNCTS --------- */
void               NetARP_ProbeAddrOnNet           (CPU_INT08U        *paddr_protocol,
                                                    NET_ARP_ADDR_LEN   addr_protocol_len,
                                                    NET_ERR           *perr);

CPU_BOOLEAN        NetARP_IsAddrProtocolConflict   (void);



                                                                                    /* ------- ARP CACHE FNCTS -------- */
void               NetARP_CacheHandler             (NET_BUF           *pbuf,
                                                    NET_ERR           *perr);

                                                                                    /* ---- ARP CACHE STATUS FNCTS ---- */
NET_ARP_ADDR_LEN   NetARP_CacheGetAddrHW           (CPU_INT08U        *paddr_hw,
                                                    NET_ARP_ADDR_LEN   addr_hw_len_buf,
                                                    CPU_INT08U        *paddr_protocol,
                                                    NET_ARP_ADDR_LEN   addr_protocol_len,
                                                    NET_ERR           *perr);


CPU_INT08U         NetARP_CacheCalcStat            (void);


NET_STAT_POOL      NetARP_CachePoolStatGet         (void);

void               NetARP_CachePoolStatResetMaxUsed(void);



                                                                                    /* ----------- RX FNCTS ----------- */
void               NetARP_Rx                       (NET_BUF           *pbuf,
                                                    NET_ERR           *perr);


                                                                                    /* ----------- TX FNCTS ----------- */
void               NetARP_TxReqGratuitous          (NET_ERR           *perr);


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                       DEFINED IN NETWORK INTERFACE & NETWORK PROTOCOL LAYERS
*********************************************************************************************************
*/

CPU_INT08U        *NetARP_GetHostAddrPtrHW         (void);

CPU_INT08U        *NetARP_GetHostAddrPtrProtocol   (void);


CPU_BOOLEAN        NetARP_IsValidAddrHW            (CPU_INT08U        *paddr_hw);

CPU_BOOLEAN        NetARP_IsValidAddrProtocol      (CPU_INT08U        *paddr_protocol);


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  NET_ARP_CFG_HW_TYPE
#error  "NET_ARP_CFG_HW_TYPE                  not #define'd in 'net_cfg.h'"
#error  "                               [MUST be  NET_ARP_HW_TYPE_ETHER]  "
#elif   (NET_ARP_CFG_HW_TYPE != NET_ARP_HW_TYPE_ETHER)
#error  "NET_ARP_CFG_HW_TYPE            illegally #define'd in 'net_cfg.h'"
#error  "                               [MUST be  NET_ARP_HW_TYPE_ETHER]  "

#else
                                                            /* Correctly configured in 'net_cfg_net.h'; DO NOT MODIFY.  */
#ifndef  NET_ARP_CFG_HW_ADDR_LEN
#error  "NET_ARP_CFG_HW_ADDR_LEN              not #define'd in 'net_cfg_net.h'"
#error  "                               [MUST be  NET_ARP_HW_ADDR_LEN_ETHER]  "
#elif   (NET_ARP_CFG_HW_ADDR_LEN != NET_ARP_HW_ADDR_LEN_ETHER)
#error  "NET_ARP_CFG_HW_ADDR_LEN        illegally #define'd in 'net_cfg_net.h'"
#error  "                               [MUST be  NET_ARP_HW_ADDR_LEN_ETHER]  "
#endif

#endif



#ifndef  NET_ARP_CFG_PROTOCOL_TYPE
#error  "NET_ARP_CFG_PROTOCOL_TYPE            not #define'd in 'net_cfg.h'    "
#error  "                               [MUST be  NET_ARP_PROTOCOL_TYPE_IP_V4]"
#elif   (NET_ARP_CFG_PROTOCOL_TYPE != NET_ARP_PROTOCOL_TYPE_IP_V4)
#error  "NET_ARP_CFG_PROTOCOL_TYPE      illegally #define'd in 'net_cfg.h'    "
#error  "                               [MUST be  NET_ARP_PROTOCOL_TYPE_IP_V4]"

#else
                                                            /* Correctly configured in 'net_cfg_net.h'; DO NOT MODIFY.  */
#ifndef  NET_ARP_CFG_PROTOCOL_ADDR_LEN
#error  "NET_ARP_CFG_PROTOCOL_ADDR_LEN        not #define'd in 'net_cfg_net.h'    "
#error  "                               [MUST be  NET_ARP_PROTOCOL_ADDR_LEN_IP_V4]"
#elif   (NET_ARP_CFG_PROTOCOL_ADDR_LEN != NET_ARP_PROTOCOL_ADDR_LEN_IP_V4)
#error  "NET_ARP_CFG_PROTOCOL_ADDR_LEN  illegally #define'd in 'net_cfg_net.h'    "
#error  "                               [MUST be  NET_ARP_PROTOCOL_ADDR_LEN_IP_V4]"
#endif

#endif




#ifndef  NET_ARP_CFG_NBR_CACHE
#error  "NET_ARP_CFG_NBR_CACHE                not #define'd in 'net_cfg.h'"
#error  "                               [MUST be  >     0]                "
#error  "                               [     &&  < 65536]                "
                                                                            /* See 'ARP CACHE QUANTITY DATA TYPE'.      */
#elif  ((NET_ARP_CFG_NBR_CACHE <                   1) || \
        (NET_ARP_CFG_NBR_CACHE > DEF_INT_16U_MAX_VAL))
#error  "NET_ARP_CFG_NBR_CACHE          illegally #define'd in 'net_cfg.h'"
#error  "                               [MUST be  >     0]                "
#error  "                               [     &&  < 65536]                "
#endif




#ifndef  NET_ARP_CFG_ADDR_FLTR_EN
#error  "NET_ARP_CFG_ADDR_FLTR_EN             not #define'd in 'net_cfg.h'"
#error  "                               [MUST be  DEF_DISABLED]           "
#error  "                               [     ||  DEF_ENABLED ]           "
#elif  ((NET_ARP_CFG_ADDR_FLTR_EN != DEF_DISABLED) && \
        (NET_ARP_CFG_ADDR_FLTR_EN != DEF_ENABLED ))
#error  "NET_ARP_CFG_ADDR_FLTR_EN       illegally #define'd in 'net_cfg.h'"
#error  "                               [MUST be  DEF_DISABLED]           "
#error  "                               [     ||  DEF_ENABLED ]           "
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of ARP module include (see Note #1).             */

