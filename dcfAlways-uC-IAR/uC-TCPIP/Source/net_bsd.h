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
*                                            BSD 4.x LAYER
*
* Filename      : net_bsd.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
* Notice(s)     : (1) The Institute of Electrical and Electronics Engineers and The Open Group, have given 
*                     us permission to reprint portions of their documentation.  Portions of this text are 
*                     reprinted and reproduced in electronic form from the IEEE Std 1003.1, 2004 Edition, 
*                     Standard for Information Technology -- Portable Operating System Interface (POSIX), 
*                     The Open Group Base Specifications Issue 6, Copyright (C) 2001-2004 by the Institute 
*                     of Electrical and Electronics Engineers, Inc and The Open Group.  In the event of any 
*                     discrepancy between these versions and the original IEEE and The Open Group Standard, 
*                     the original IEEE and The Open Group Standard is the referee document.  The original 
*                     Standard can be obtained online at http://www.opengroup.org/unix/online.html.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) BSD 4.x module is required for :
*
*               (a) Network sockets
*               (b) Applications that require BSD 4.x application interface (API) :
*                   (1) Macro's
*                   (2) Data Types
*                   (3) Functions
*
*               See also 'net_sock.h  Note #1'
*                      & 'net_cfg.h   BSD 4.x LAYER CONFIGURATION'.
*
*           (2) The following BSD-module-present configuration value MUST be pre-#define'd in 'net_cfg_net.h'
*               PRIOR to all other network modules that require BSD 4.x Layer Configuration (see 'net_cfg_net.h
*               BSD 4.x LAYER CONFIGURATION  Note #2b') :
*
*                   NET_BSD_MODULE_PRESENT
*********************************************************************************************************
*/

#ifdef   NET_BSD_MODULE_PRESENT                                 /* See Note #2.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_BSD_MODULE
#define  NET_BSD_EXT
#else
#define  NET_BSD_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define  NET_BSD_ADDR_LEN_MAX                             14    /* Sizeof(sockaddr.addr).                               */
#define  NET_BSD_ADDR_IP_NBR_OCTETS_UNUSED                 8


#define  NET_BSD_ASCII_LEN_MAX_ADDR_IP                   NET_ASCII_LEN_MAX_ADDR_IP


/*
*********************************************************************************************************
*                              BSD 4.x SOCKET FAMILY & PROTOCOL DEFINES
*
* Note(s) : (1) The following socket values MUST be pre-#define'd in 'net_def.h' PRIOR to 'net_cfg.h'
*               so that the developer can configure sockets for the correct socket family values (see
*               'net_def.h  BSD 4.x & NETWORK SOCKET LAYER DEFINES  Note #1' & 'net_cfg_net.h  NETWORK 
*               SOCKET LAYER CONFIGURATION') :
*
*                   AF_INET                                        2
*                   PF_INET                                  AF_INET
*********************************************************************************************************
*/

                                                                /* -------------------- SOCK TYPES -------------------- */
#ifdef   SOCK_STREAM
#undef   SOCK_STREAM
#endif
#define  SOCK_STREAM                                       1

#ifdef   SOCK_DGRAM
#undef   SOCK_DGRAM
#endif
#define  SOCK_DGRAM                                        2


                                                                /* -------------- TCP/IP SOCK PROTOCOLS --------------- */
#ifdef   IPPROTO_TCP
#undef   IPPROTO_TCP
#endif
#define  IPPROTO_TCP                                       6    /*  = NET_IP_HDR_PROTOCOL_TCP                           */

#ifdef   IPPROTO_UDP
#undef   IPPROTO_UDP
#endif
#define  IPPROTO_UDP                                      17    /*  = NET_IP_HDR_PROTOCOL_UDP                           */


/*
*********************************************************************************************************
*                                   BSD 4.x SOCKET ADDRESS DEFINES
*********************************************************************************************************
*/

#ifdef   INADDR_ANY
#undef   INADDR_ANY
#endif
#define  INADDR_ANY                                      NET_IP_ADDR_NONE

#ifdef   INADDR_BROADCAST
#undef   INADDR_BROADCAST
#endif
#define  INADDR_BROADCAST                                NET_IP_ADDR_BROADCAST


/*$PAGE*/
/*
*********************************************************************************************************
*                                     BSD 4.x SOCKET FLAG DEFINES
*
* Note(s) : (1) #### Some socket flags NOT currently supported.
*********************************************************************************************************
*/

#ifdef   MSG_OOB
#undef   MSG_OOB
#endif
#define  MSG_OOB                                  DEF_BIT_00    /* See Note #1.                                         */

#ifdef   MSG_PEEK
#undef   MSG_PEEK
#endif
#define  MSG_PEEK                                 DEF_BIT_01

#ifdef   MSG_DONTROUTE
#undef   MSG_DONTROUTE
#endif
#define  MSG_DONTROUTE                            DEF_BIT_02    /* See Note #1.                                         */

#ifdef   MSG_EOR
#undef   MSG_EOR
#endif
#define  MSG_EOR                                  DEF_BIT_03    /* See Note #1.                                         */

#ifdef   MSG_TRUNC
#undef   MSG_TRUNC
#endif
#define  MSG_TRUNC                                DEF_BIT_04    /* See Note #1.                                         */

#ifdef   MSG_CTRUNC
#undef   MSG_CTRUNC
#endif
#define  MSG_CTRUNC                               DEF_BIT_05    /* See Note #1.                                         */

#ifdef   MSG_WAITALL
#undef   MSG_WAITALL
#endif
#define  MSG_WAITALL                              DEF_BIT_06    /* See Note #1.                                         */

#ifdef   MSG_DONTWAIT
#undef   MSG_DONTWAIT
#endif
#define  MSG_DONTWAIT                             DEF_BIT_07


/*$PAGE*/
/*
*********************************************************************************************************
*                                 BSD 4.x RETURN CODE / ERROR DEFINES
*
* Note(s) : (1) (a) (1) (A) IEEE Std 1003.1, 2004 Edition states to "return 0 ... upon successful
*                           completion" ...
*
*                       (B) ... of the following BSD socket functions :
*
*                           (1) close()
*                           (2) shutdown()
*                           (3) bind()
*                           (4) connect()
*                           (5) listen()
*
*                   (2) (A) (1) "Otherwise, -1 shall be returned"       ...
*                           (2) "and 'errno' set to indicate the error" ...
*
*                       (B) ... for the following BSD socket functions :
*
*                           (1) socket()
*                           (2) close()
*                           (3) shutdown()
*                           (4) bind()
*                           (5) listen()
*                           (6) accept()
*                           (7) connect()
*                           (8) recvfrom()
*                           (9) recv()
*                          (10) sendto()
*                          (11) send()
*                          (12) inet_addr()
*                          (13) inet_ntoa()
*
*               (b) (1) IEEE Std 1003.1, 2004 Edition states to "return 0 ... if no messages are 
*                       available ... and the peer has performed an orderly shutdown" ...
*
*                   (2) ... for the following BSD socket functions :
*
*                       (A) recvfrom()
*                       (B) recv()
*********************************************************************************************************
*/

#define  NET_BSD_ERR_NONE                                  0    /* See Note #1a1.                                       */
#define  NET_BSD_ERR_DFLT                                 -1    /* See Note #1ab.                                       */

#define  NET_BSD_RTN_CODE_CONN_CLOSED                      0    /* See Note #1b.                                        */


/*$PAGE*/
/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                        BSD NETWORK WORD ORDER - TO - CPU WORD ORDER MACRO'S
*
* Description : Convert data values to & from network word order to host CPU word order.
*
* Argument(s) : val       Data value to convert (see Note #2).
*
* Return(s)   : Converted data value (see Note #2).
*
* Caller(s)   : Application.
*
*               These macro's are network protocol suite application interface (API) macro's & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) BSD 4.x macro's are required only for applications that call BSD 4.x macro's.
*
*                   See also 'MODULE  Note #1b1'.
*
*               (2) 'val' data value to convert & any variable to receive the returned conversion MUST 
*                   start on appropriate CPU word-aligned addresses.  This is required because most word-
*                   aligned processors are more efficient & may even REQUIRE that multi-octet words start 
*                   on CPU word-aligned addresses.
*
*                   (a) For 16-bit word-aligned processors, this means that
*
*                           all 16- & 32-bit words MUST start on addresses that are multiples of 2 octets
*
*                   (b) For 32-bit word-aligned processors, this means that
*
*                           all 16-bit       words MUST start on addresses that are multiples of 2 octets
*                           all 32-bit       words MUST start on addresses that are multiples of 4 octets
*
*                   See also 'net_util.h  NETWORK WORD ORDER - TO - CPU WORD ORDER MACRO'S  Note #1'.
*********************************************************************************************************
*/

#if (NET_BSD_CFG_API_EN == DEF_ENABLED)                                 /* See Note #1.                                 */

#define  ntohs(val)                     (NET_UTIL_NET_TO_HOST_16(val))
#define  ntohl(val)                     (NET_UTIL_NET_TO_HOST_32(val))

#define  htons(val)                     (NET_UTIL_HOST_TO_NET_16(val))
#define  htonl(val)                     (NET_UTIL_HOST_TO_NET_32(val))

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                   BSD 4.x SOCKET ADDRESS DATA TYPES
*
* Note(s) : (1) BSD 4.x data types are required only for applications that reference BSD 4.x data types.
*
*               See also 'MODULE  Note #1b2'.
*
*           (2) (a) Socket address structure 'Family' member MUST be configured in host-order & MUST NOT
*                   be converted to/from network-order.
*
*               (b) Socket address structure addresses MUST be configured/converted from host-order to
*                   network-order.
*
*               See also 'net_sock.h  NETWORK SOCKET ADDRESS DATA TYPES  Note #2'.
*********************************************************************************************************
*/

#if (NET_BSD_CFG_API_EN == DEF_ENABLED)                                 /* See Note #1.                                 */

                                                                        /* ----------------- SOCKADDR ----------------- */
struct  sockaddr {
    CPU_INT16U          sa_family;                                      /* Sock family (see Note #2a).                  */
    CPU_CHAR            sa_data[NET_BSD_ADDR_LEN_MAX];                  /* Sock addr   (see Note #2b).                  */
};



                                                                        /* ----------- SOCK IPv4 PORT/ADDR ------------ */
typedef  CPU_INT16U     in_port_t;
typedef  CPU_INT32U     in_addr_t;


struct  in_addr {
    in_addr_t           s_addr;
};

                                                                        /* --------------- SOCKADDR_IN ---------------- */
struct  sockaddr_in {
            CPU_INT16U  sin_family;                                     /* AF_INET family (see Note #2a).               */
            CPU_INT16U  sin_port;                                       /* Port nbr       (see Note #2b).               */
    struct  in_addr     sin_addr;                                       /* IP addr        (see Note #2b).               */
            CPU_CHAR    sin_zero[NET_BSD_ADDR_IP_NBR_OCTETS_UNUSED];    /* Unused (MUST be zero).                       */
};


#endif


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*
* Note(s) : (1) BSD 4.x global variables are required only for applications that call BSD 4.x functions.
*
*               See also 'MODULE  Note #1b'
*                      & 'STANDARD BSD 4.x FUNCTION PROTOTYPES  Note #1'.
*********************************************************************************************************
*/

#if    (NET_BSD_CFG_API_EN  == DEF_ENABLED)                             /* See Note #1.                                 */

#ifdef  NET_SOCK_MODULE_PRESENT
#if    (NET_SOCK_CFG_FAMILY == NET_SOCK_FAMILY_IP_V4)
NET_BSD_EXT  CPU_CHAR   NetBSD_IP_to_Str_Array[NET_BSD_ASCII_LEN_MAX_ADDR_IP];
#endif
#endif

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                 STANDARD BSD 4.x FUNCTION PROTOTYPES
*
* Note(s) : (1) BSD 4.x function prototypes are required only for applications that call BSD 4.x functions.
*
*               See also 'MODULE  Note #1b3'.
*********************************************************************************************************
*/

#if (NET_BSD_CFG_API_EN == DEF_ENABLED)                                 /* See Note #1.                                 */

                                                                        /* ------------- SOCK ALLOC FNCTS ------------- */
int         socket   (        int        protocol_family,
                              int        sock_type,
                              int        protocol);

int         close    (        int        sock_id);

int         shutdown (        int        sock_id,
                              int        type);


                                                                        /* ------------- LOCAL CONN FCNTS ------------- */
int         bind     (        int        sock_id,
                      struct  sockaddr  *paddr_local,
                              int        addr_len);


                                                                        /* ------------ CLIENT CONN FCNTS ------------- */
int         connect  (        int        sock_id,
                      struct  sockaddr  *paddr_remote,
                              int        addr_len);


                                                                        /* ------------ SERVER CONN FCNTS ------------- */
int         listen   (        int        sock_id,
                              int        sock_q_size);

int         accept   (        int        sock_id,
                      struct  sockaddr  *paddr_remote,
                              int       *paddr_len);


                                                                        /* ----------------- RX FNCTS ----------------- */
int         recvfrom (        int        sock_id,
                              void      *pdata_buf,
                              int        data_buf_len,
                              int        flags,
                      struct  sockaddr  *paddr_remote,
                              int       *paddr_len);

int         recv     (        int        sock_id,
                              void      *pdata_buf,
                              int        data_buf_len,
                              int        flags);


                                                                        /* ----------------- TX FNCTS ----------------- */
int         sendto   (        int        sock_id,
                              void      *p_data,
                              int        data_len,
                              int        flags,
                      struct  sockaddr  *paddr_remote,
                              int        addr_len);

int         send     (        int        sock_id,
                              void      *p_data,
                              int        data_len,
                              int        flags);


                                                                        /* ---------------- CONV FCNTS ---------------- */
in_addr_t   inet_addr(        char      *paddr);

char       *inet_ntoa(struct  in_addr    addr);


#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of BSD module include (see Note #1).             */

