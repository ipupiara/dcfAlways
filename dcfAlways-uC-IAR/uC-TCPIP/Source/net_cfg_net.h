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
*                                        NETWORK CONFIGURATION
*
* Filename      : net_cfg_net.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     NETWORK DEBUG CONFIGURATION
*********************************************************************************************************
*/

#if     (NET_ICMP_CFG_TX_SRC_QUENCH_EN   == DEF_ENABLED)
#define  NET_DBG_CFG_MON_TASK_EN            DEF_ENABLED
#else
#define  NET_DBG_CFG_MON_TASK_EN            DEF_DISABLED
#endif


#if    ((NET_DBG_CFG_MON_TASK_EN         == DEF_ENABLED) || \
        (NET_DBG_CFG_STATUS_EN           == DEF_ENABLED))
#define  NET_DBG_CFG_STATUS_RSRC_LOST_EN    DEF_ENABLED
#else
#define  NET_DBG_CFG_STATUS_RSRC_LOST_EN    DEF_DISABLED
#endif


#if    ((NET_DBG_CFG_MON_TASK_EN         == DEF_ENABLED) || \
        (NET_DBG_CFG_STATUS_EN           == DEF_ENABLED))
#define  NET_DBG_CFG_STATUS_RSRC_LO_EN      DEF_ENABLED
#else
#define  NET_DBG_CFG_STATUS_RSRC_LO_EN      DEF_DISABLED
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                NETWORK INTERFACE LAYER CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure network interface parameters.
*               DO NOT MODIFY.
*
*           (2) Network interface maximum header size MUST be #define'd based on network interface type
*               configured in 'net_cfg.h'.  Assumes header sizes are fixed based on configured network
*               interface type (see any 'net_if.h  NETWORK INTERFACE HEADER DEFINES  Note #1').
*
*           (3) Ideally, the Packet-based Network Interface Layer would configure the network protocol
*               suite for the inclusion of the Packet-based Network Interface Layer via the
*               NET_IF_PKT_MODULE_PRESENT #define (see 'net_if_pkt.h  MODULE  Note #2').
*
*               However, the presence of the Packet-based Network Interface Layer MUST be configured ...
*
*               (a) By each network interface that requires the     Packet-based Network Interface Layer
*                     AND
*               (b) PRIOR to all other network modules that require Packet-based Network Interface Layer
*                       Configuration
*
*           (4) Some network interfaces require network-address-to-hardware-address bindings, implemented
*               in an Address Resolution Protocol Layer (see 'net_arp.h  Note #1').
*
*               Ideally, the ARP Layer would configure the network protocol suite for the inclusion of
*               the ARP Layer via the NET_ARP_MODULE_PRESENT #define (see 'net_arp.h  MODULE  Note #2' &
*               'ARP LAYER CONFIGURATION  Note #2b').
*
*               However, since the ARP Layer is required only for SOME network interfaces, the presence of 
*               the ARP Layer MUST be configured ...
*
*               (a) By each network interface that requires the     ARP Layer
*                     AND
*               (b) PRIOR to all other network modules that require ARP Layer Configuration
*********************************************************************************************************
*/

                                                                /* ---------------- CFG NET IF PARAMS ----------------- */
#ifdef   NET_IF_HDR_SIZE_MAX
#undef   NET_IF_HDR_SIZE_MAX
#endif
                                                                /* See Note #2.                                         */
#if     (NET_IF_CFG_TYPE                 == NET_IF_TYPE_ETHER)
#define  NET_IF_HDR_SIZE_MAX                NET_IF_HDR_SIZE_ETHER

#define  NET_IF_PKT_MODULE_PRESENT                              /* See Note #3.                                         */
#define  NET_ARP_MODULE_PRESENT                                 /* See Note #4.                                         */
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                       ARP LAYER CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure ARP parameters.  DO NOT MODIFY.
*
*           (2) (a) Address Resolution Protocol required only for some network interfaces (see 'net_arp.h
*                   Note #1').
*
*               (b) ARP Layer Configuration included only when NET_ARP_MODULE_PRESENT is #define'd.
*
*                   See 'net_arp.h  MODULE  Note #2' & 'NETWORK INTERFACE LAYER CONFIGURATION  Note #4'.
*
*           (3) ARP hardware & protocol address lengths MUST be #define'd based on ARP hardware & protocol
*               types configured in 'net_cfg.h'.  Assumes hardware & protocol address lengths are fixed
*               based on configured ARP hardware & protocol types (see 'net_arp.h  Notes #2a & #2b').
*********************************************************************************************************
*/

#ifdef   NET_ARP_MODULE_PRESENT                                 /* See Note #2b.                                        */
                                                                /* ------------------ CFG ARP PARAMS ------------------ */

#ifdef   NET_ARP_CFG_HW_ADDR_LEN
#undef   NET_ARP_CFG_HW_ADDR_LEN
#endif
                                                                /* See Note #3.                                         */
#if     (NET_ARP_CFG_HW_TYPE             == NET_ARP_HW_TYPE_ETHER)
#define  NET_ARP_CFG_HW_ADDR_LEN            NET_ARP_HW_ADDR_LEN_ETHER
#endif


#ifdef   NET_ARP_CFG_PROTOCOL_ADDR_LEN
#undef   NET_ARP_CFG_PROTOCOL_ADDR_LEN
#endif
                                                                /* See Note #3.                                         */
#if     (NET_ARP_CFG_PROTOCOL_TYPE       == NET_ARP_PROTOCOL_TYPE_IP_V4)
#define  NET_ARP_CFG_PROTOCOL_ADDR_LEN      NET_ARP_PROTOCOL_ADDR_LEN_IP_V4
#endif


#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                    TRANSPORT LAYER CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure transport layer parameters.
*               DO NOT MODIFY.
*********************************************************************************************************
*/

#ifdef   NET_CFG_TRANSPORT_TYPE_STREAM_EN
#undef   NET_CFG_TRANSPORT_TYPE_STREAM_EN
#endif

#if     (NET_CFG_TRANSPORT_LAYER_SEL     == NET_TRANSPORT_LAYER_SEL_UDP_TCP)
#define  NET_CFG_TRANSPORT_TYPE_STREAM_EN   DEF_ENABLED
#else
#define  NET_CFG_TRANSPORT_TYPE_STREAM_EN   DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                       TCP LAYER CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure TCP parameters.  DO NOT MODIFY.
*
*           (2) (a) TCP Layer required only for some application interfaces (see 'net_tcp.h  MODULE
*                   Note #1').
*
*               (b) Ideally, the TCP Layer would configure the network protocol suite for the inclusion of
*                   the TCP Layer via the NET_TCP_MODULE_PRESENT #define (see 'net_tcp.h  MODULE  Note #2').
*                   However, the presence of the TCP Layer MUST be configured PRIOR to all other network
*                   modules that require TCP Layer Configuration.
*********************************************************************************************************
*/

#if     (NET_CFG_TRANSPORT_LAYER_SEL == NET_TRANSPORT_LAYER_SEL_UDP_TCP)

#define  NET_TCP_MODULE_PRESENT                                 /* See Note #2.                                         */

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                 NETWORK SOCKET LAYER CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure network socket parameters.
*               DO NOT MODIFY.
*
*           (2) (a) Network Socket Layer required only for some application interfaces (see 'net_sock.h
*                   Note #1').
*
*               (b) Ideally, the Network Socket Layer would configure the network protocol suite for the 
*                   inclusion of the Socket Layer via the NET_SOCK_MODULE_PRESENT #define (see 'net_sock.h
*                   MODULE  Note #2').  However, the presence of the Socket Layer MUST be configured PRIOR 
*                   to all other network modules that require Socket Layer Configuration.
*
*           (3) Network socket family configuration values MUST be #define'd based on Network Socket
*               Family type configured in 'net_cfg.h'.
*********************************************************************************************************
*/

                                                                /* ------------ CFG SOCK MODULE INCLUSION ------------- */
#if    ((NET_CFG_TRANSPORT_LAYER_SEL == NET_TRANSPORT_LAYER_SEL_UDP_TCP) || \
       ((NET_CFG_TRANSPORT_LAYER_SEL == NET_TRANSPORT_LAYER_SEL_UDP    ) && \
       ((NET_UDP_CFG_APP_API_SEL     == NET_UDP_APP_API_SEL_SOCK    )    || \
        (NET_UDP_CFG_APP_API_SEL     == NET_UDP_APP_API_SEL_SOCK_APP))))

#define  NET_SOCK_MODULE_PRESENT                                /* See Note #2.                                         */


                                                                /* ----------------- CFG SOCK PARAMS ------------------ */
#ifdef   NET_SOCK_CFG_ADDR_LEN
#undef   NET_SOCK_CFG_ADDR_LEN
#endif

#ifdef   NET_SOCK_CFG_PROTOCOL_MAX
#undef   NET_SOCK_CFG_PROTOCOL_MAX
#endif


                                                                /* See Note #3.                                         */
#if     (NET_SOCK_CFG_FAMILY             == NET_SOCK_FAMILY_IP_V4)

#define  NET_SOCK_CFG_ADDR_LEN              NET_SOCK_ADDR_LEN_IP_V4

#if     (NET_CFG_TRANSPORT_LAYER_SEL     == NET_TRANSPORT_LAYER_SEL_UDP)
#define  NET_SOCK_CFG_PROTOCOL_MAX          NET_SOCK_PROTOCOL_MAX_IP_V4_UDP

#elif   (NET_CFG_TRANSPORT_LAYER_SEL     == NET_TRANSPORT_LAYER_SEL_UDP_TCP)
#define  NET_SOCK_CFG_PROTOCOL_MAX          NET_SOCK_PROTOCOL_MAX_IP_V4_UDP_TCP

#endif

#endif



#ifdef   NET_SOCK_CFG_TYPE_STREAM_EN
#undef   NET_SOCK_CFG_TYPE_STREAM_EN
#endif

#define  NET_SOCK_CFG_TYPE_STREAM_EN        NET_CFG_TRANSPORT_TYPE_STREAM_EN


#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                     BSD 4.x LAYER CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure BSD 4.x layer parameters.
*               DO NOT MODIFY.
*
*           (2) (a) BSD 4.x Layer required only for some application configurations (see 'net_bsd.h
*                   MODULE  Note #1').
*
*               (b) Ideally, the BSD 4.x Layer would configure the network protocol suite for the 
*                   inclusion of the BSD 4.x Layer via the NET_BSD_MODULE_PRESENT #define (see 
*                   'net_bsd.h  MODULE  Note #2').  However, the presence of the BSD 4.x Layer 
*                   MUST be configured PRIOR to all other network modules that require BSD 4.x
*                   Layer Configuration.
*********************************************************************************************************
*/

                                                                /* ------------ CFG BSD MODULE INCLUSION -------------- */
#if    ((NET_BSD_CFG_API_EN == DEF_ENABLED) || \
        (defined(NET_SOCK_MODULE_PRESENT)))

#define  NET_BSD_MODULE_PRESENT                                 /* See Note #2.                                         */

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                               NETWORK BUFFER MANAGEMENT CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure network buffer module parameters.
*               DO NOT MODIFY.
*
*           (2) (a) NET_BUF_DATA_PROTOCOL_HDR_SIZE_MAX's ideal #define'tion :
*
*                       (A) IF Hdr  +  max(Protocol Headers)
*
*               (b) NET_BUF_DATA_PROTOCOL_HDR_SIZE_MAX  #define'd with hard-coded knowledge that IF, IP &
*                   TCP headers have the largest combined maximum size of all the protocol headers :
*
*                             IP Hdr   60     ARP Hdr   68      IP Hdr   60      IP Hdr   60
*                           ICMP Hdr    0                      UDP Hdr    8     TCP Hdr   60
*                           -------------     ------------     ------------     ------------
*                           Total      60     Total     68     Total     68     Total    120
*
*               See also 'net_buf.h  NETWORK BUFFER INDEX & SIZE DEFINES  Note #1'.
*********************************************************************************************************
*/

                                                                /* -------- CFG BUF DATA PROTOCOL HDR SIZE MAX -------- */
#ifdef   NET_TCP_MODULE_PRESENT

#define  NET_BUF_DATA_PROTOCOL_HDR_SIZE_MAX             (NET_IF_HDR_SIZE_MAX + \
                                                         NET_IP_HDR_SIZE_MAX + \
                                                         NET_TCP_HDR_SIZE_MAX)

#else

#define  NET_BUF_DATA_PROTOCOL_HDR_SIZE_MAX             (NET_IF_HDR_SIZE_MAX + \
                                                         NET_IP_HDR_SIZE_MAX + \
                                                         NET_UDP_HDR_SIZE_MAX)

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                             NETWORK CONNECTION MANAGEMENT CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure network connection module 
*               parameters.  DO NOT MODIFY.
*
*           (2) (a) Connection Management Module required only for some application configurations
*                   (see 'net_conn.h  MODULE  Note #1').
*
*               (b) Ideally, the Connection Management Module would configure the network protocol suite 
*                   for the inclusion of the Connection Management Module via the NET_CONN_MODULE_PRESENT
*                   #define (see 'net_conn.h  MODULE  Note #2').  However, the presence of the Connection
*                   Management Module MUST be configured PRIOR to all other network modules that require
*                   Connection Management Module Configuration.
*
*           (3) Connection Management family configuration values MUST be #define'd based on Connection
*               Management Family type configured in 'net_cfg.h'.
*
*           (4) If the socket &/or TCP modules are mutually included (see 'net_sock.h  MODULE  Note #1' &
*               'net_tcp.h  MODULE  Note #1'), then the configured number of connections MUST be greater
*               than the configured number of sockets PLUS the configured number of TCP connections.
*********************************************************************************************************
*/

                                                                /* ------------ CFG CONN MODULE INCLUSION ------------- */
#if    ((defined(NET_TCP_MODULE_PRESENT )) || \
        (defined(NET_SOCK_MODULE_PRESENT)))

#define  NET_CONN_MODULE_PRESENT                                /* See Note #2.                                         */


                                                                /* ----------------- CFG CONN PARAMS ------------------ */
#ifdef   NET_CONN_CFG_ADDR_LEN
#undef   NET_CONN_CFG_ADDR_LEN
#endif

#ifdef   NET_CONN_CFG_PROTOCOL_MAX
#undef   NET_CONN_CFG_PROTOCOL_MAX
#endif
                                                                /* See Note #3.                                         */
#if     (NET_CONN_CFG_FAMILY             == NET_CONN_FAMILY_IP_V4_SOCK)
#define  NET_CONN_CFG_ADDR_LEN              NET_SOCK_CFG_ADDR_LEN
#define  NET_CONN_CFG_PROTOCOL_MAX          NET_SOCK_CFG_PROTOCOL_MAX
#endif


#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                  NETWORK CONNECTION CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure network communication 
*               parameters.  DO NOT MODIFY.
*
*           (2) To balance network receive versus transmit packet loads for certain network connection
*               types (e.g. stream-type connections), network receive & transmit packets SHOULD be 
*               handled in an APPROXIMATELY balanced ratio.
*
*               See also 'net.h  NETWORK RECEIVE PACKET MACRO'S  Note #1'.
*********************************************************************************************************
*/

                                                                /* See Note 2.                                          */
#ifdef   NET_CFG_LOAD_BAL_EN
#undef   NET_CFG_LOAD_BAL_EN
#endif

#define  NET_CFG_LOAD_BAL_EN                NET_CFG_TRANSPORT_TYPE_STREAM_EN


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifdef   NET_IF_PKT_MODULE_PRESENT

#ifndef  NET_NIC_CFG_TX_PKT_PREPARE_EN
#error  "NET_NIC_CFG_TX_PKT_PREPARE_EN        not #define'd in 'net_cfg.h'"
#error  "                               [MUST be  DEF_DISABLED]           "
#error  "                               [     ||  DEF_ENABLED ]           "
#elif  ((NET_NIC_CFG_TX_PKT_PREPARE_EN != DEF_DISABLED) && \
        (NET_NIC_CFG_TX_PKT_PREPARE_EN != DEF_ENABLED ))
#error  "NET_NIC_CFG_TX_PKT_PREPARE_EN  illegally #define'd in 'net_cfg.h'"
#error  "                               [MUST be  DEF_DISABLED]           "
#error  "                               [     ||  DEF_ENABLED ]           "
#endif

#endif




#ifndef  NET_CFG_TRANSPORT_LAYER_SEL
#error  "NET_CFG_TRANSPORT_LAYER_SEL          not #define'd in 'net_cfg.h'        "
#error  "                               [MUST be  NET_TRANSPORT_LAYER_SEL_UDP    ]"
#error  "                               [     ||  NET_TRANSPORT_LAYER_SEL_UDP_TCP]"
#elif  ((NET_CFG_TRANSPORT_LAYER_SEL != NET_TRANSPORT_LAYER_SEL_UDP    ) && \
        (NET_CFG_TRANSPORT_LAYER_SEL != NET_TRANSPORT_LAYER_SEL_UDP_TCP))
#error  "NET_CFG_TRANSPORT_LAYER_SEL    illegally #define'd in 'net_cfg.h'        "
#error  "                               [MUST be  NET_TRANSPORT_LAYER_SEL_UDP    ]"
#error  "                               [     ||  NET_TRANSPORT_LAYER_SEL_UDP_TCP]"
#endif



                                                        /* See 'NETWORK CONNECTION MANAGEMENT CONFIGURATION  Note #4'.  */
#ifdef   NET_CONN_MODULE_PRESENT

#ifdef   NET_TCP_MODULE_PRESENT

#if     (NET_CONN_CFG_NBR_CONN < (NET_SOCK_CFG_NBR_SOCK \
                               +  NET_TCP_CFG_NBR_CONN ))
#error  "NET_CONN_CFG_NBR_CONN          illegally #define'd in 'net_cfg.h'   "
#error  "                               [MUST be  >= (NET_SOCK_CFG_NBR_SOCK ]"
#error  "                               [         +   NET_TCP_CFG_NBR_CONN )]"
#endif

#else

#if     (NET_CONN_CFG_NBR_CONN <  NET_SOCK_CFG_NBR_SOCK)
#error  "NET_CONN_CFG_NBR_CONN          illegally #define'd in 'net_cfg.h' "
#error  "                               [MUST be  >= NET_SOCK_CFG_NBR_SOCK]"
#endif

#endif

#endif




#ifndef  NET_BSD_CFG_API_EN
#error  "NET_BSD_CFG_API_EN                   not #define'd in 'net_cfg.h'"
#error  "                               [MUST be  DEF_DISABLED]           "
#error  "                               [     ||  DEF_ENABLED ]           "
#elif  ((NET_BSD_CFG_API_EN != DEF_DISABLED) && \
        (NET_BSD_CFG_API_EN != DEF_ENABLED ))
#error  "NET_BSD_CFG_API_EN             illegally #define'd in 'net_cfg.h'"
#error  "                               [MUST be  DEF_DISABLED]           "
#error  "                               [     ||  DEF_ENABLED ]           "
#endif

