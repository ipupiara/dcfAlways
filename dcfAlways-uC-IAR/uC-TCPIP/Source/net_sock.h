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
*                                         NETWORK SOCKET LAYER
*
* Filename      : net_sock.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
* Note(s)       : (1) Supports BSD 4.x Socket Layer with the following restrictions/constraints :
*
*                     (a) ONLY supports a single address family from the following families :
*                         (1) IPv4 (AF_INET)
*
*                     (b) ONLY supports the following socket types :
*                         (1) Datagram (SOCK_DGRAM)
*                         (2) Stream   (SOCK_STREAM)
*
*                     (c) ONLY supports a single protocol family from the following families :
*                         (1) IPv4 (PF_INET)
*                             (A) ONLY supports the following protocols :
*                                 (1) UDP (IPPROTO_UDP)
*                                 (2) TCP (IPPROTO_TCP)
*
*                     (d) ONLY supports the following socket options :
*                         (1) Global options
*                             #### ( )
*                         (2) Socket options
*                             #### ( )
*
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) Socket module is required for :
*
*               (a) IPv4 Sockets
*                   (1) UDP Datagram Sockets
*                   (2) TCP Stream   Sockets
*
*
*               See also 'net_sock.h  Note #1c',
*                        'net_cfg.h   TRANSPORT LAYER CONFIGURATION',
*                      & 'net_cfg.h   USER DATAGRAM PROTOCOL LAYER CONFIGURATION'.
*
*           (2) The following socket-module-present configuration value MUST be pre-#define'd in 
*               'net_cfg_net.h' PRIOR to all other network modules that require Network Socket Layer
*               Configuration (see 'net_cfg_net.h  NETWORK SOCKET LAYER CONFIGURATION  Note #2b') :
*
*                   NET_SOCK_MODULE_PRESENT
*********************************************************************************************************
*/

#ifdef   NET_SOCK_MODULE_PRESENT                                /* See Note #2.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_SOCK_MODULE
#define  NET_SOCK_EXT
#else
#define  NET_SOCK_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                             NETWORK SOCKET IDENTIFICATION NUMBER DEFINES
*********************************************************************************************************
*/

#define  NET_SOCK_ID_NONE                                 -1
#define  NET_SOCK_ID_MIN                                   0
#define  NET_SOCK_ID_MAX                                (NET_SOCK_CFG_NBR_SOCK - 1)


/*
*********************************************************************************************************
*                                  NETWORK SOCKET PORT NUMBER DEFINES
*********************************************************************************************************
*/

#define  NET_SOCK_PORT_NBR_RESERVED                      NET_PORT_NBR_RESERVED
#define  NET_SOCK_PORT_NBR_NONE                          NET_SOCK_PORT_NBR_RESERVED


#define  NET_SOCK_PORT_NBR_RANDOM_NBR_OFF                 10

#define  NET_SOCK_PORT_NBR_RANDOM_NBR                   (NET_SOCK_CFG_NBR_SOCK + NET_SOCK_PORT_NBR_RANDOM_NBR_OFF)

#if     (NET_SOCK_PORT_NBR_RANDOM_NBR                  > DEF_INT_16U_MAX_VAL)
#undef   NET_SOCK_PORT_NBR_RANDOM_NBR
#define  NET_SOCK_PORT_NBR_RANDOM_NBR                    DEF_INT_16U_MAX_VAL
#endif

#define  NET_SOCK_PORT_NBR_RANDOM_MIN                    NET_SOCK_CFG_PORT_NBR_RANDOM_BASE
#define  NET_SOCK_PORT_NBR_RANDOM_MAX                   (NET_SOCK_CFG_PORT_NBR_RANDOM_BASE + NET_SOCK_PORT_NBR_RANDOM_NBR - 1)


/*
*********************************************************************************************************
*                                    NETWORK SOCKET TIMEOUT DEFINES
*********************************************************************************************************
*/

#define  NET_SOCK_TIMEOUT_MIN_MS                         NET_OS_TIMEOUT_MIN_MS
#define  NET_SOCK_TIMEOUT_MAX_MS                         NET_OS_TIMEOUT_MAX_MS

#define  NET_SOCK_TIMEOUT_MIN_SEC                        NET_OS_TIMEOUT_MIN_SEC
#define  NET_SOCK_TIMEOUT_MAX_SEC                        NET_OS_TIMEOUT_MAX_SEC


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NETWORK SOCKET STATES
*********************************************************************************************************
*/

#define  NET_SOCK_STATE_NONE                               0
#define  NET_SOCK_STATE_FREE                               1
#define  NET_SOCK_STATE_DISCARD                            2
#define  NET_SOCK_STATE_CLOSED                            10
#define  NET_SOCK_STATE_CLOSED_FAULT                      11
#define  NET_SOCK_STATE_CLOSE_IN_PROGRESS                 15
#define  NET_SOCK_STATE_CLOSING_DATA_AVAIL                16
#define  NET_SOCK_STATE_BOUND                             20
#define  NET_SOCK_STATE_LISTEN                            30
#define  NET_SOCK_STATE_CONN                              40
#define  NET_SOCK_STATE_CONN_IN_PROGRESS                  41
#define  NET_SOCK_STATE_CONN_DONE                         42


/*
*********************************************************************************************************
*                                     NETWORK SOCKET FLAG DEFINES
*********************************************************************************************************
*/

                                                                /* ------------------ NET SOCK FLAGS ------------------ */
#define  NET_SOCK_FLAG_NONE                       DEF_BIT_NONE
#define  NET_SOCK_FLAG_USED                       DEF_BIT_00    /* Sock cur used; i.e. NOT in free sock pool.           */
#define  NET_SOCK_FLAG_BLOCK                      DEF_BIT_01    /* Sock blocking ENABLED.                               */


/*
*********************************************************************************************************
*                                     NETWORK SOCKET TYPE DEFINES
*
* Note(s) : (1) NET_SOCK_TYPE_??? #define values specifically chosen as ASCII representations of the socket
*               types.  Memory displays of socket types will display the socket TYPE with the chosen ASCII 
*               name.
*********************************************************************************************************
*/

                                                                /* ------------------ NET SOCK TYPES ------------------ */
#if     (CPU_CFG_ENDIAN_TYPE == CPU_ENDIAN_TYPE_BIG)
#define  NET_SOCK_TYPE_SOCK_NONE                  0x4E4F4E45    /* "NONE" in ASCII.                                     */
#define  NET_SOCK_TYPE_SOCK                       0x534F434B    /* "SOCK" in ASCII.                                     */

#else

#if     (CPU_CFG_DATA_SIZE   == CPU_WORD_SIZE_32)
#define  NET_SOCK_TYPE_SOCK_NONE                  0x454E4F4E    /* "NONE" in ASCII.                                     */
#define  NET_SOCK_TYPE_SOCK                       0x4B434F53    /* "SOCK" in ASCII.                                     */

#elif   (CPU_CFG_DATA_SIZE   == CPU_WORD_SIZE_16)
#define  NET_SOCK_TYPE_SOCK_NONE                  0x4F4E454E    /* "NONE" in ASCII.                                     */
#define  NET_SOCK_TYPE_SOCK                       0x4F534B43    /* "SOCK" in ASCII.                                     */

#else                                                           /* Dflt CPU_WORD_SIZE_08.                               */
#define  NET_SOCK_TYPE_SOCK_NONE                  0x4E4F4E45    /* "NONE" in ASCII.                                     */
#define  NET_SOCK_TYPE_SOCK                       0x534F434B    /* "SOCK" in ASCII.                                     */

#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                              NETWORK SOCKET FAMILY & PROTOCOL DEFINES
*
* Note(s) : (1) The following socket values MUST be pre-#define'd in 'net_def.h' PRIOR to 'net_cfg.h'
*               so that the developer can configure sockets for the correct socket family values (see
*               'net_def.h  BSD 4.x & NETWORK SOCKET LAYER DEFINES  Note #1' & 'net_cfg_net.h  NETWORK 
*               SOCKET LAYER CONFIGURATION') :
*
*                   AF_INET                                        2
*                   PF_INET                                  AF_INET
*
*                   NET_SOCK_FAMILY_IP_V4                    PF_INET
*                   NET_SOCK_PROTOCOL_MAX_IP_V4                    2
*                   NET_SOCK_ADDR_LEN_IP_V4                        6
*********************************************************************************************************
*/

                                                                /* -------------------- SOCK TYPES -------------------- */
#define  NET_SOCK_TYPE_NONE                                0
#define  NET_SOCK_TYPE_DATAGRAM                         SOCK_DGRAM
#define  NET_SOCK_TYPE_STREAM                           SOCK_STREAM


                                                                /* -------------- TCP/IP SOCK PROTOCOLS --------------- */
#define  NET_SOCK_PROTOCOL_DFLT                            0
#define  NET_SOCK_PROTOCOL_NONE                         NET_SOCK_PROTOCOL_DFLT

#define  NET_SOCK_PROTOCOL_UDP                          IPPROTO_UDP
#define  NET_SOCK_PROTOCOL_TCP                          IPPROTO_TCP


/*
*********************************************************************************************************
*                                   NETWORK SOCKET ADDRESS DEFINES
*********************************************************************************************************
*/

                                                                /* ------------------ SOCK ADDR CFG ------------------- */
#if     (NET_SOCK_CFG_FAMILY == NET_SOCK_FAMILY_IP_V4)

#define  NET_SOCK_ADDR_IP_LEN_PORT               (sizeof(NET_PORT_NBR))
#define  NET_SOCK_ADDR_IP_LEN_ADDR               (sizeof(NET_IP_ADDR))

#define  NET_SOCK_ADDR_IP_IX_BASE                          0
#define  NET_SOCK_ADDR_IP_IX_PORT                        NET_SOCK_ADDR_IP_IX_BASE
#define  NET_SOCK_ADDR_IP_IX_ADDR                       (NET_SOCK_ADDR_IP_IX_PORT + NET_SOCK_ADDR_IP_LEN_PORT) 


#define  NET_SOCK_ADDR_IP_WILD_CARD                      INADDR_ANY
#define  NET_SOCK_ADDR_IP_BROADCAST                      INADDR_BROADCAST

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                     NETWORK SOCKET API DEFINES
*********************************************************************************************************
*/

#define  NET_SOCK_BSD_ADDR_LEN_MAX                       NET_BSD_ADDR_LEN_MAX

#define  NET_SOCK_ADDR_IP_NBR_OCTETS_UNUSED              NET_BSD_ADDR_IP_NBR_OCTETS_UNUSED


#define  NET_SOCK_BSD_SOCK_ID_MIN_VAL                      0
#define  NET_SOCK_BSD_SOCK_ID_MAX_VAL                  32767


#define  NET_SOCK_BSD_ERR_NONE                           NET_BSD_ERR_NONE
#define  NET_SOCK_BSD_ERR_DFLT                           NET_BSD_ERR_DFLT

#define  NET_SOCK_BSD_ERR_OPEN                           NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_CLOSE                          NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_BIND                           NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_CONN                           NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_LISTEN                         NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_ACCEPT                         NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_RX                             NET_SOCK_BSD_ERR_DFLT
#define  NET_SOCK_BSD_ERR_TX                             NET_SOCK_BSD_ERR_DFLT


#define  NET_SOCK_BSD_RTN_CODE_CONN_CLOSED               NET_BSD_RTN_CODE_CONN_CLOSED


/*
*********************************************************************************************************
*                                         SOCKET FLAG DEFINES
*********************************************************************************************************
*/

#define  NET_SOCK_FLAG_RX_DATA_PEEK                      MSG_PEEK

#define  NET_SOCK_FLAG_NO_BLOCK                          MSG_DONTWAIT
#define  NET_SOCK_FLAG_RX_NO_BLOCK                       NET_SOCK_FLAG_NO_BLOCK
#define  NET_SOCK_FLAG_TX_NO_BLOCK                       NET_SOCK_FLAG_NO_BLOCK


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  NETWORK SOCKET QUANTITY DATA TYPE
*
* Note(s) : (1) See also 'NETWORK SOCKET IDENTIFICATION DATA TYPE  Note #1'.
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_SOCK_QTY;                              /* Defines max qty of socks to support.                 */


/*
*********************************************************************************************************
*                                   NETWORK SOCKET FAMILY DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_SOCK_FAMILY;


/*
*********************************************************************************************************
*                               NETWORK SOCKET IDENTIFICATION DATA TYPE
*
* Note(s) : (1) (a) NET_SOCK_NBR_MAX  SHOULD be globally #define'd AFTER 'NET_SOCK_QTY' data type declared.
*
*               (b) Since socket handle identifiers are data-typed as 16-bit signed integers, the maximum
*                   unique number of valid socket identifiers (i.e. non-negative) is the total number of
*                   positive values that 16-bit signed integers support.
*********************************************************************************************************
*/

typedef  CPU_INT16S  NET_SOCK_ID;

#define  NET_SOCK_NBR_MAX               DEF_INT_16S_MAX_VAL     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                               NETWORK SOCKET ADDRESS LENGTH DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16S  NET_SOCK_ADDR_LEN;


/*$PAGE*/
/*
*********************************************************************************************************
*                                 NETWORK SOCKET QUEUE SIZE DATA TYPE
*
* Note(s) : (1) (a) NET_SOCK_Q_SIZE #define's SHOULD be globally #define'd AFTER 'NET_SOCK_Q_SIZE'
*                   data type declared.
*
*               (b) Since socket/connection handle identifiers are data-typed as 16-bit signed 
*                   integers, the maximum unique number of valid identifiers (i.e. non-negative) 
*                   is the total number of positive values that 16-bit signed integers support.
*
*                   See also             'NETWORK SOCKET     IDENTIFICATION DATA TYPE  Note #1b'
*                          & 'net_conn.h  NETWORK CONNECTION IDENTIFICATION DATA TYPE  Note #2b'.
*
*           (2) (a) NET_SOCK_Q_IX   #define's SHOULD be globally #define'd AFTER 'NET_SOCK_Q_SIZE'
*                   data type declared.
*
*               (b) Since socket queue size is data typed as a 16-bit unsigned integer but the
*                   maximum queue sizes are #define'd as 16-bit signed integer values, valid
*                   socket queue indices are #define'd within the range of 16-bit signed integer
*                   values but socket queue indice exception values may be #define'd with 16-bit
*                   unsigned integer values.
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_SOCK_Q_SIZE;

                                                                /* See Note #1.                                         */
#define  NET_SOCK_Q_SIZE_NONE             0
#define  NET_SOCK_Q_SIZE_MIN              1
#define  NET_SOCK_Q_SIZE_MAX            DEF_INT_16S_MAX_VAL     /* See Note #1b.                                        */

                                                                /* See Note #2.                                         */
#define  NET_SOCK_Q_IX_NONE             DEF_INT_16U_MAX_VAL
#define  NET_SOCK_Q_IX_MIN                0
#define  NET_SOCK_Q_IX_MAX             (NET_SOCK_Q_SIZE_MAX - 1)


/*
*********************************************************************************************************
*                                   NETWORK SOCKET STATE DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT08U  NET_SOCK_STATE;


/*
*********************************************************************************************************
*                            NETWORK SOCKET (ERROR) RETURN CODE DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16S  NET_SOCK_RTN_CODE;


/*$PAGE*/
/*
*********************************************************************************************************
*                                  NETWORK SOCKET ADDRESS DATA TYPES
*
* Note(s) : (1) See 'net_sock.h  Note #1a' for supported socket address families.
*
*           (2) (a) Socket address structure 'Family' member MUST be configured in host-order & MUST NOT
*                   be converted to/from network-order.
*
*               (b) Socket address structure addresses MUST be configured/converted from host-order to
*                   network-order.
*********************************************************************************************************
*/

                                                                /* ------------------ NET SOCK ADDR ------------------- */
typedef  struct  net_sock_addr {
    NET_SOCK_FAMILY     Family;                                 /* Sock family type (see Note #2a).                     */
    CPU_INT08U          Addr[NET_SOCK_BSD_ADDR_LEN_MAX];        /* Sock addr        (see Note #2b).                     */
} NET_SOCK_ADDR;

#define  NET_SOCK_ADDR_SIZE                 (sizeof(NET_SOCK_ADDR))



#if (NET_SOCK_CFG_FAMILY == NET_SOCK_FAMILY_IP_V4)
                                                                /* ----------------- NET SOCK ADDR IP ----------------- */
typedef  struct  net_sock_addr_ip {
    NET_SOCK_FAMILY     Family;                                 /* Sock family = NET_SOCK_FAMILY_IP_V4 (AF_INET).       */
    NET_PORT_NBR        Port;                                   /* UDP/TCP port nbr.                                    */
    NET_IP_ADDR         Addr;                                   /* IP addr.                                             */
    CPU_INT08U          Unused[NET_SOCK_ADDR_IP_NBR_OCTETS_UNUSED]; /* Unused (MUST be zero).                           */
} NET_SOCK_ADDR_IP;

#define  NET_SOCK_ADDR_IP_SIZE              (sizeof(NET_SOCK_ADDR_IP))

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NETWORK SOCKET DATA TYPE
*
*                             NET_SOCK
*                          |-------------|
*                          |  Sock Type  |
*                          |-------------|      Next
*                          |      O----------> Socket     Buffer Queue
*                          |-------------|                    Heads      -------
*                          |      O------------------------------------> |     |
*                          |-------------|                               |     |
*                          |      O----------------------                -------
*                          |-------------|              |                  | ^
*                          |  Conn IDs   |              |                  v |
*                          |-------------|              |                -------
*                          |    Sock     |              |                |     |
*                          |   Family/   |              |                |     |
*                          |  Protocol   |              |                -------
*                          |-------------|              |                  | ^
*                          | Conn Ctrls  |              | Buffer Queue     v |
*                          |-------------|              |     Tails      -------
*                          |    Flags    |              ---------------> |     |
*                          |-------------|                               |     |
*                          |    State    |                               -------
*                          |-------------|
*
*
* Note(s) : (1) Forced word-alignment at start of socket NOT required since first data member 'Type'
*               is declared as 'CPU_INT32U'.
*
*           (2) 'NextSockPtr' ideally declared as 'NET_SOCK' pointer; declared as 'void' pointer because
*               'NET_SOCK' NOT fully defined at time of declaration.
*
*           (3) #### 'TxQ_Head'/'TxQ_Tail' may NOT be necessary (remove if unnecessary).
*********************************************************************************************************
*/

typedef  struct  net_sock {
    NET_TYPE          Type;                                         /* Type cfg'd @ init : NET_SOCK_TYPE_SOCK.          */

    void             *NextSockPtr;                                  /* Ptr to NEXT sock.                                */


    NET_BUF          *RxQ_Head;                                     /* Ptr to head of sock's rx buf Q.                  */
    NET_BUF          *RxQ_Tail;                                     /* Ptr to tail of sock's rx buf Q.                  */

#if 0                                                               /* #### See Note #3.                                */
    NET_BUF          *TxQ_Head;                                     /* Ptr to head of sock's tx buf Q.                  */
    NET_BUF          *TxQ_Tail;                                     /* Ptr to tail of sock's tx buf Q.                  */
#endif


    NET_SOCK_ID       ID;                                           /* Sock id.                                         */
    NET_CONN_ID       ID_Conn;                                      /* Conn id.                                         */


    CPU_INT16S        Family;                                       /* Sock protocol family.                            */
    CPU_INT16S        Protocol;                                     /* Sock protocol.                                   */
    CPU_INT16S        SockType;                                     /* Sock type.                                       */


#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
    NET_SOCK_Q_SIZE   ConnAcceptQ_SizeMax;                          /* Max Q size to accept rx'd conn reqs.             */
    NET_SOCK_Q_SIZE   ConnAcceptQ_SizeCur;                          /* Cur Q size to accept rx'd conn reqs.             */
                                                                    /* Conn accept Q (conn id's q'd into array).        */
    NET_CONN_ID       ConnAcceptQ[NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX];
    NET_SOCK_Q_SIZE   ConnAcceptQ_HeadIx;                           /* Conn accept Q head ix.                           */
    NET_SOCK_Q_SIZE   ConnAcceptQ_TailIx;                           /* Conn accept Q tail ix.                           */
#endif


    CPU_INT16U        Flags;                                        /* Sock flags.                                      */

    NET_SOCK_STATE    State;                                        /* Cur sock state.                                  */
} NET_SOCK;


/*$PAGE*/
/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

NET_SOCK_EXT  NET_SOCK           NetSock_Tbl[NET_SOCK_CFG_NBR_SOCK];
NET_SOCK_EXT  NET_SOCK          *NetSock_PoolPtr;                       /* Ptr to pool of free socks.                   */
NET_SOCK_EXT  NET_STAT_POOL      NetSock_PoolStat;


NET_SOCK_EXT  NET_PORT_NBR       NetSock_RandomPortNbrQ[NET_SOCK_PORT_NBR_RANDOM_NBR];
NET_SOCK_EXT  NET_PORT_NBR       NetSock_RandomPortNbrQ_HeadIx;         /* Random port FIFO Q head ix.                  */
NET_SOCK_EXT  NET_PORT_NBR       NetSock_RandomPortNbrQ_TailIx;         /* Random port FIFO Q tail ix.                  */
NET_SOCK_EXT  NET_PORT_NBR_QTY   NetSock_RandomPortNbrQ_NbrUsed;        /* Random port FIFO Q nbr used.                 */



#if (NET_CTR_CFG_STAT_EN         == DEF_ENABLED)                        /* -------------- NET SOCK STATS -------------- */
NET_SOCK_EXT  NET_CTR            NetSock_StatRxPktCtr;                  /* Nbr rx'd pkts.                               */
NET_SOCK_EXT  NET_CTR            NetSock_StatRxPktProcessedCtr;         /* Nbr rx'd pkts delivered to apps.             */
#endif



#if (NET_CTR_CFG_ERR_EN          == DEF_ENABLED)                        /* -------------- NET SOCK ERRS --------------- */
NET_SOCK_EXT  NET_CTR            NetSock_ErrNullPtrCtr;                 /* Nbr null   sock ptr     accesses.            */
NET_SOCK_EXT  NET_CTR            NetSock_ErrNullSizeCtr;

NET_SOCK_EXT  NET_CTR            NetSock_ErrNoneAvailCtr;               /* Nbr        sock unavail accesses.            */
NET_SOCK_EXT  NET_CTR            NetSock_ErrNotUsedCtr;                 /* Nbr unused sock         accesses.            */

NET_SOCK_EXT  NET_CTR            NetSock_ErrCloseCtr;                   /* Nbr fault  sock closes.                      */


NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidFamilyCtr;           /* Nbr socks with invalid sock family.          */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidProtocolCtr;         /* Nbr socks with invalid sock protocol.        */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidSockTypeCtr;         /* Nbr socks with invalid sock type.            */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidSockCtr;             /* Nbr            invalid sock ID accesses.     */

NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidFlagsCtr;            /* Nbr socks with invalid flags.                */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidOpCtr;               /* Nbr socks with invalid op.                   */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidStateCtr;            /* Nbr socks with invalid state.                */

NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidAddrCtr;             /* Nbr socks with invalid addr.                 */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidAddrLenCtr;          /* Nbr socks with invalid addr len.             */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidAddrInUseCtr;        /* Nbr socks with         addr already in use.  */

NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidPortNbrCtr;          /* Nbr socks with invalid port nbr.             */

NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidConnInUseCtr;        /* Nbr socks with         conn already in use.  */



#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
NET_SOCK_EXT  NET_CTR            NetSock_ErrConnAcceptQ_NoneAvailCtr;   /* Nbr accept Q conn   unavail accesses.       */
#endif

NET_SOCK_EXT  NET_CTR            NetSock_ErrRandomPortNbrNoneAvailCtr;  /* Nbr random port nbr unavail accesses.       */



NET_SOCK_EXT  NET_CTR            NetSock_ErrRxDestCtr;                  /* Nbr rx'd pkts for unavail dest.              */

NET_SOCK_EXT  NET_CTR            NetSock_ErrRxPktDiscardedCtr;          /* Nbr rx'd pkts discarded.                     */



#if  (NET_ERR_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
NET_SOCK_EXT  NET_CTR            NetSock_ErrTxInvalidSizeCtr;
#endif

#if  (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidTypeCtr;             /* Nbr sock invalid type accesses.              */
NET_SOCK_EXT  NET_CTR            NetSock_ErrInvalidConnCtr;

NET_SOCK_EXT  NET_CTR            NetSock_ErrRxInvalidBufIxCtr;          /* Nbr rx   pkts with invalid buf ix.           */

#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
NET_SOCK_EXT  NET_CTR            NetSock_ErrConnAcceptQ_MaxCtr;
#endif

NET_SOCK_EXT  NET_CTR            NetSock_ErrRandomPortNbrQ_UsedCtr;
NET_SOCK_EXT  NET_CTR            NetSock_ErrRandomPortNbrQ_NbrInQ_Ctr;
#endif

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void                NetSock_Init                        (NET_ERR            *perr);


                                                                                    /* ----------- RX FNCTS ----------- */
void                NetSock_Rx                          (NET_BUF            *pbuf,
                                                         NET_ERR            *perr);


                                                                                    /* ----------- TX FNCTS ----------- */




                                                                                    /* -------- SOCK API FNCTS -------- */
NET_SOCK_ID         NetSock_Open                        (CPU_INT16S          protocol_family,
                                                         CPU_INT16S          sock_type,
                                                         CPU_INT16S          protocol,
                                                         NET_ERR            *perr);


NET_SOCK_RTN_CODE   NetSock_Close                       (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);

void                NetSock_CloseFromConn               (NET_SOCK_ID         sock_id);


void                NetSock_FreeConnFromSock            (NET_SOCK_ID         sock_id,
                                                         NET_CONN_ID         conn_id);



NET_SOCK_RTN_CODE   NetSock_Bind                        (NET_SOCK_ID         sock_id,
                                                         NET_SOCK_ADDR      *paddr_local,
                                                         NET_SOCK_ADDR_LEN   addr_len,
                                                         NET_ERR            *perr);


NET_SOCK_RTN_CODE   NetSock_Conn                        (NET_SOCK_ID         sock_id,
                                                         NET_SOCK_ADDR      *paddr_remote,
                                                         NET_SOCK_ADDR_LEN   addr_len,
                                                         NET_ERR            *perr);

#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
void                NetSock_ConnSignalReq               (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);

void                NetSock_ConnSignalAccept            (NET_SOCK_ID         sock_id,
                                                         NET_CONN_ID         conn_id,
                                                         NET_ERR            *perr);

void                NetSock_ConnSignalClose             (NET_SOCK_ID         sock_id,
                                                         CPU_BOOLEAN         data_avail,
                                                         NET_ERR            *perr);


NET_SOCK_RTN_CODE   NetSock_Listen                      (NET_SOCK_ID         sock_id,
                                                         NET_SOCK_Q_SIZE     sock_q_size,
                                                         NET_ERR            *perr);

CPU_BOOLEAN         NetSock_ListenQ_IsAvail             (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);


NET_SOCK_ID         NetSock_Accept                      (NET_SOCK_ID         sock_id,
                                                         NET_SOCK_ADDR      *paddr_remote,
                                                         NET_SOCK_ADDR_LEN  *paddr_len,
                                                         NET_ERR            *perr);
#endif

/*$PAGE*/
NET_SOCK_RTN_CODE   NetSock_RxDataFrom                  (NET_SOCK_ID         sock_id,
                                                         void               *pdata_buf,
                                                         CPU_INT16S          data_buf_len,
                                                         CPU_INT16S          flags,
                                                         NET_SOCK_ADDR      *paddr_remote,
                                                         NET_SOCK_ADDR_LEN  *paddr_len,
                                                         void               *pip_opts_buf,
                                                         CPU_INT08U          ip_opts_buf_len,
                                                         CPU_INT08U         *pip_opts_len,
                                                         NET_ERR            *perr);

NET_SOCK_RTN_CODE   NetSock_RxData                      (NET_SOCK_ID         sock_id,
                                                         void               *pdata_buf,
                                                         CPU_INT16S          data_buf_len,
                                                         CPU_INT16S          flags,
                                                         NET_ERR            *perr);


NET_SOCK_RTN_CODE   NetSock_TxDataTo                    (NET_SOCK_ID         sock_id,
                                                         void               *p_data,
                                                         CPU_INT16S          data_len,
                                                         CPU_INT16S          flags,
                                                         NET_SOCK_ADDR      *paddr_remote,
                                                         NET_SOCK_ADDR_LEN   addr_len,
                                                         NET_ERR            *perr);

NET_SOCK_RTN_CODE   NetSock_TxData                      (NET_SOCK_ID         sock_id,
                                                         void               *p_data,
                                                         CPU_INT16S          data_len,
                                                         CPU_INT16S          flags,
                                                         NET_ERR            *perr);


/*$PAGE*/
                                                                                    /* -------- SOCK CFG FNCTS -------- */
                                                                                    /* Cfg dflt sock rx Q   timeout.    */
void                NetSock_CfgTimeoutRxQ_Dflt          (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);
                                                                                    /* Cfg/set  sock rx Q   timeout.    */
void                NetSock_CfgTimeoutRxQ_Set           (NET_SOCK_ID         sock_id,
                                                         CPU_INT32U          timeout_ms,
                                                         NET_ERR            *perr);
                                                                                    /* Get      sock rx Q   timeout.    */
CPU_INT32U          NetSock_CfgTimeoutRxQ_Get_ms        (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);


                                                                                    /* Cfg dflt sock tx Q   timeout.    */
void                NetSock_CfgTimeoutTxQ_Dflt          (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);
                                                                                    /* Cfg/set  sock tx Q   timeout.    */
void                NetSock_CfgTimeoutTxQ_Set           (NET_SOCK_ID         sock_id,
                                                         CPU_INT32U          timeout_ms,
                                                         NET_ERR            *perr);
                                                                                    /* Get      sock tx Q   timeout.    */
CPU_INT32U          NetSock_CfgTimeoutTxQ_Get_ms        (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);


#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
                                                                                    /* Cfg dflt sock conn   timeout.    */
void                NetSock_CfgTimeoutConnReqDflt       (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);
                                                                                    /* Cfg/set  sock conn   timeout.    */
void                NetSock_CfgTimeoutConnReqSet        (NET_SOCK_ID         sock_id,
                                                         CPU_INT32U          timeout_ms,
                                                         NET_ERR            *perr);
                                                                                    /* Get      sock conn   timeout.    */
CPU_INT32U          NetSock_CfgTimeoutConnReqGet_ms     (NET_SOCK_ID         sock_id,
                                                         NET_ERR            *perr);


                                                                                    /* Cfg dflt sock accept timeout.    */
void                NetSock_CfgTimeoutConnAcceptDflt  (NET_SOCK_ID         sock_id,
                                                       NET_ERR            *perr);
                                                                                    /* Cfg/set  sock accept timeout.    */
void                NetSock_CfgTimeoutConnAcceptSet   (NET_SOCK_ID         sock_id,
                                                       CPU_INT32U          timeout_ms,
                                                       NET_ERR            *perr);
                                                                                    /* Get      sock accept timeout.    */
CPU_INT32U          NetSock_CfgTimeoutConnAcceptGet_ms(NET_SOCK_ID         sock_id,
                                                       NET_ERR            *perr);


                                                                                    /* Cfg dflt sock close  timeout.    */
void                NetSock_CfgTimeoutConnCloseDflt   (NET_SOCK_ID         sock_id,
                                                       NET_ERR            *perr);
                                                                                    /* Cfg/set  sock close  timeout.    */
void                NetSock_CfgTimeoutConnCloseSet    (NET_SOCK_ID         sock_id,
                                                       CPU_INT32U          timeout_ms,
                                                       NET_ERR            *perr);
                                                                                    /* Get      sock close  timeout.    */
CPU_INT32U          NetSock_CfgTimeoutConnCloseGet_ms (NET_SOCK_ID         sock_id,
                                                       NET_ERR            *perr);
#endif



                                                                                    /* ------ SOCK STATUS FNCTS ------- */
CPU_BOOLEAN         NetSock_IsUsed                    (NET_SOCK_ID         sock_id,
                                                       NET_ERR            *perr);

CPU_BOOLEAN         NetSock_IsConn                    (NET_SOCK_ID         sock_id,
                                                       NET_ERR            *perr);


NET_STAT_POOL       NetSock_PoolStatGet               (void);

void                NetSock_PoolStatResetMaxUsed      (void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                      DEFINED IN OS'S  net_os.c
*********************************************************************************************************
*/

void        NetOS_Sock_Init                     (NET_ERR      *perr);               /* Create sock objs.                */


                                                                                    /* Clr      sock rx Q signal.       */
void        NetOS_Sock_RxQ_Clr                  (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Wait for sock rx Q signal.       */
void        NetOS_Sock_RxQ_Wait                 (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Signal   sock rx Q.              */
void        NetOS_Sock_RxQ_Signal               (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Abort    sock rx Q.              */
void        NetOS_Sock_RxQ_Abort                (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               

                                                                                    /* Set dflt sock rx Q timeout.      */
void        NetOS_Sock_RxQ_TimeoutDflt          (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Set      sock rx Q timeout.      */
void        NetOS_Sock_RxQ_TimeoutSet           (NET_SOCK_ID   sock_id,             
                                                 CPU_INT32U    timeout_ms,          
                                                 NET_ERR      *perr);               
                                                                                    /* Get      sock rx Q timeout.      */
CPU_INT32U  NetOS_Sock_RxQ_TimeoutGet_ms        (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               



#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
                                                                                    /* Clr      sock conn signal.       */
void        NetOS_Sock_ConnReqClr               (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Wait for sock conn signal.       */
void        NetOS_Sock_ConnReqWait              (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Signal   sock conn.              */
void        NetOS_Sock_ConnReqSignal            (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Abort    sock conn.              */
void        NetOS_Sock_ConnReqAbort             (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               

                                                                                    /* Set dflt sock conn timeout.      */
void        NetOS_Sock_ConnReqTimeoutDflt       (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Set      sock conn timeout.      */
void        NetOS_Sock_ConnReqTimeoutSet        (NET_SOCK_ID   sock_id,             
                                                 CPU_INT32U    timeout_ms,          
                                                 NET_ERR      *perr);               
                                                                                    /* Get      sock conn timeout.      */
CPU_INT32U  NetOS_Sock_ConnReqTimeoutGet_ms     (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               


                                                                                    /* Clr      sock accept Q signal.   */
void        NetOS_Sock_ConnAcceptQ_Clr          (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Wait for sock accept Q signal.   */
void        NetOS_Sock_ConnAcceptQ_Wait         (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Signal   sock accept Q.          */
void        NetOS_Sock_ConnAcceptQ_Signal       (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Abort    sock accept Q wait.     */
void        NetOS_Sock_ConnAcceptQ_Abort        (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               

                                                                                    /* Set dflt sock accept Q timeout.  */
void        NetOS_Sock_ConnAcceptQ_TimeoutDflt  (NET_SOCK_ID   sock_id,             
                                                 NET_ERR      *perr);               
                                                                                    /* Set      sock accept Q timeout.  */
void        NetOS_Sock_ConnAcceptQ_TimeoutSet   (NET_SOCK_ID   sock_id,             
                                                 CPU_INT32U    timeout_ms,          
                                                 NET_ERR      *perr);               
                                                                                    /* Get      sock accept Q timeout.  */
CPU_INT32U  NetOS_Sock_ConnAcceptQ_TimeoutGet_ms(NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);


/*$PAGE*/
                                                                                    /* Clr      sock close signal.      */
void        NetOS_Sock_ConnCloseClr             (NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);
                                                                                    /* Wait for sock close signal.      */
void        NetOS_Sock_ConnCloseWait            (NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);
                                                                                    /* Signal   sock close.             */
void        NetOS_Sock_ConnCloseSignal          (NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);
                                                                                    /* Abort    sock close.             */
void        NetOS_Sock_ConnCloseAbort           (NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);

                                                                                    /* Set dflt sock close timeout.     */
void        NetOS_Sock_ConnCloseTimeoutDflt     (NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);
                                                                                    /* Set      sock close timeout.     */
void        NetOS_Sock_ConnCloseTimeoutSet      (NET_SOCK_ID   sock_id,
                                                 CPU_INT32U    timeout_ms,
                                                 NET_ERR      *perr);
                                                                                    /* Get      sock close timeout.     */
CPU_INT32U  NetOS_Sock_ConnCloseTimeoutGet_ms   (NET_SOCK_ID   sock_id,
                                                 NET_ERR      *perr);
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  NET_SOCK_CFG_FAMILY
#error  "NET_SOCK_CFG_FAMILY                        not #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  NET_SOCK_FAMILY_IP_V4]  "

#elif   (NET_SOCK_CFG_FAMILY != NET_SOCK_FAMILY_IP_V4)
#error  "NET_SOCK_CFG_FAMILY                  illegally #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  NET_SOCK_FAMILY_IP_V4]  "


#else

                                                            /* Correctly configured in 'net_cfg_net.h'; DO NOT MODIFY.  */
#ifndef  NET_SOCK_CFG_ADDR_LEN
#error  "NET_SOCK_CFG_ADDR_LEN                      not #define'd in 'net_cfg_net.h'"
#error  "                                     [MUST be  NET_SOCK_ADDR_LEN_IP_V4]    "

#elif   (NET_SOCK_CFG_ADDR_LEN != NET_SOCK_ADDR_LEN_IP_V4)
#error  "NET_SOCK_CFG_ADDR_LEN                illegally #define'd in 'net_cfg_net.h'"
#error  "                                     [MUST be  NET_SOCK_ADDR_LEN_IP_V4]    "
#endif


#ifndef  NET_SOCK_CFG_PROTOCOL_MAX
#error  "NET_SOCK_CFG_PROTOCOL_MAX                  not #define'd in 'net_cfg_net.h'   "
#error  "                                     [MUST be  >  0                          ]"
#error  "                                     [     &&  <= NET_SOCK_PROTOCOL_MAX_IP_V4]"

#elif  ((NET_SOCK_CFG_PROTOCOL_MAX < 1                          ) || \
        (NET_SOCK_CFG_PROTOCOL_MAX > NET_SOCK_PROTOCOL_MAX_IP_V4))
#error  "NET_SOCK_CFG_PROTOCOL_MAX            illegally #define'd in 'net_cfg_net.h'   "
#error  "                                     [MUST be  >  0                          ]"
#error  "                                     [     &&  <= NET_SOCK_PROTOCOL_MAX_IP_V4]"
#endif


#endif




#ifndef  NET_SOCK_CFG_NBR_SOCK
#error  "NET_SOCK_CFG_NBR_SOCK                      not #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  >  0                   ]"
#error  "                                     [     &&  <= NET_SOCK_NBR_MAX    ]"

#elif  ((NET_SOCK_CFG_NBR_SOCK <                1) || \
        (NET_SOCK_CFG_NBR_SOCK > NET_SOCK_NBR_MAX))
#error  "NET_SOCK_CFG_NBR_SOCK                illegally #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  >  0                   ]"
#error  "                                     [     &&  <= NET_SOCK_NBR_MAX    ]"
#endif




#ifndef  NET_SOCK_CFG_BLOCK_SEL
#error  "NET_SOCK_CFG_BLOCK_SEL                     not #define'd in 'net_cfg.h'    "
#error  "                                     [MUST be  NET_SOCK_BLOCK_SEL_DFLT    ]"
#error  "                                     [     ||  NET_SOCK_BLOCK_SEL_BLOCK   ]"
#error  "                                     [     ||  NET_SOCK_BLOCK_SEL_NO_BLOCK]"

#elif  ((NET_SOCK_CFG_BLOCK_SEL != NET_SOCK_BLOCK_SEL_DFLT    ) && \
        (NET_SOCK_CFG_BLOCK_SEL != NET_SOCK_BLOCK_SEL_BLOCK   ) && \
        (NET_SOCK_CFG_BLOCK_SEL != NET_SOCK_BLOCK_SEL_NO_BLOCK))
#error  "NET_SOCK_CFG_BLOCK_SEL               illegally #define'd in 'net_cfg.h'    "
#error  "                                     [MUST be  NET_SOCK_BLOCK_SEL_DFLT    ]"
#error  "                                     [     ||  NET_SOCK_BLOCK_SEL_BLOCK   ]"
#error  "                                     [     ||  NET_SOCK_BLOCK_SEL_NO_BLOCK]"
#endif




/*$PAGE*/
                                                                /* Correctly configured in 'net_sock.h'; DO NOT MODIFY. */
#ifndef  NET_SOCK_CFG_TYPE_STREAM_EN
#error  "NET_SOCK_CFG_TYPE_STREAM_EN                not #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  DEF_DISABLED]           "
#error  "                                     [     ||  DEF_ENABLED ]           "

#elif  ((NET_SOCK_CFG_TYPE_STREAM_EN != DEF_DISABLED) && \
        (NET_SOCK_CFG_TYPE_STREAM_EN != DEF_ENABLED ))
#error  "NET_SOCK_CFG_TYPE_STREAM_EN          illegally #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  DEF_DISABLED]           "
#error  "                                     [     ||  DEF_ENABLED ]           "


#elif   (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)

#ifndef  NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX
#error  "NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX        not #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  >= NET_SOCK_Q_SIZE_MIN] "
#error  "                                     [     &&  <= NET_SOCK_Q_SIZE_MAX] "

#elif  ((NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX < NET_SOCK_Q_SIZE_MIN) || \
        (NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX > NET_SOCK_Q_SIZE_MAX))
#error  "NET_SOCK_CFG_CONN_ACCEPT_Q_SIZE_MAX  illegally #define'd in 'net_cfg.h'"
#error  "                                     [MUST be  >= NET_SOCK_Q_SIZE_MIN] "
#error  "                                     [     &&  <= NET_SOCK_Q_SIZE_MAX] "
#endif

#endif




#ifndef  NET_SOCK_CFG_PORT_NBR_RANDOM_BASE
#error  "NET_SOCK_CFG_PORT_NBR_RANDOM_BASE          not #define'd in 'net_cfg.h'                                                "
#error  "                                     [MUST be  >       0                                                              ]"
#error  "                                     [     &&  <= (65536 - (NET_SOCK_CFG_NBR_SOCK + NET_SOCK_PORT_NBR_RANDOM_NBR_OFF))]"

#elif  ((NET_SOCK_CFG_PORT_NBR_RANDOM_BASE <                   1) || \
        (NET_SOCK_PORT_NBR_RANDOM_MAX      > DEF_INT_16U_MAX_VAL))
#error  "NET_SOCK_CFG_PORT_NBR_RANDOM_BASE    illegally #define'd in 'net_cfg.h'                                                "
#error  "                                     [MUST be  >       0                                                              ]"
#error  "                                     [     &&  <= (65536 - (NET_SOCK_CFG_NBR_SOCK + NET_SOCK_PORT_NBR_RANDOM_NBR_OFF))]"
#endif


                                                                /* Correctly configured in 'net_sock.h'; DO NOT MODIFY. */
#ifndef  NET_SOCK_PORT_NBR_RANDOM_NBR
#error  "NET_SOCK_PORT_NBR_RANDOM_NBR               not #define'd in 'net_sock.h'"
#error  "                                     [MUST be  >     0]                 "
#error  "                                     [     &&  < 65536]                 "

#elif  ((NET_SOCK_PORT_NBR_RANDOM_NBR <                   1) || \
        (NET_SOCK_PORT_NBR_RANDOM_NBR > DEF_INT_16U_MAX_VAL))
#error  "NET_SOCK_PORT_NBR_RANDOM_NBR         illegally #define'd in 'net_sock.h'"
#error  "                                     [MUST be  >     0]                 "
#error  "                                     [     &&  < 65536]                 "
#endif




/*$PAGE*/
#ifndef  NET_SOCK_CFG_TIMEOUT_RX_Q_MS
#error  "NET_SOCK_CFG_TIMEOUT_RX_Q_MS               not #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"

#elif (((NET_SOCK_CFG_TIMEOUT_RX_Q_MS <  NET_OS_TIMEOUT_MIN_MS)  || \
        (NET_SOCK_CFG_TIMEOUT_RX_Q_MS >  NET_OS_TIMEOUT_MAX_MS)) && \
        (NET_SOCK_CFG_TIMEOUT_RX_Q_MS != NET_TMR_TIME_INFINITE))
#error  "NET_SOCK_CFG_TIMEOUT_RX_Q_MS         illegally #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"
#endif



#if (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)

#ifndef  NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS
#error  "NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS           not #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"

#elif (((NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS <  NET_OS_TIMEOUT_MIN_MS)  || \
        (NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS >  NET_OS_TIMEOUT_MAX_MS)) && \
        (NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS != NET_TMR_TIME_INFINITE))
#error  "NET_SOCK_CFG_TIMEOUT_CONN_REQ_MS     illegally #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"
#endif



#ifndef  NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS
#error  "NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS        not #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"

#elif (((NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS <  NET_OS_TIMEOUT_MIN_MS)  || \
        (NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS >  NET_OS_TIMEOUT_MAX_MS)) && \
        (NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS != NET_TMR_TIME_INFINITE))
#error  "NET_SOCK_CFG_TIMEOUT_CONN_ACCEPT_MS  illegally #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"
#endif



#ifndef  NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS
#error  "NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS         not #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"

#elif (((NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS <  NET_OS_TIMEOUT_MIN_MS)  || \
        (NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS >  NET_OS_TIMEOUT_MAX_MS)) && \
        (NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS != NET_TMR_TIME_INFINITE))
#error  "NET_SOCK_CFG_TIMEOUT_CONN_CLOSE_MS   illegally #define'd in 'net_cfg.h' "
#error  "                                     [MUST be  >= NET_OS_TIMEOUT_MIN_MS]"
#error  "                                     [     &&  <= NET_OS_TIMEOUT_MAX_MS]"
#error  "                                     [     ||  == NET_TMR_TIME_INFINITE]"
#endif

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of net sock module include (see Note #1).        */

