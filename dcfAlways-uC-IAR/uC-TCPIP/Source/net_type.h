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
*                                         NETWORK DATA TYPES
*
* Filename      : net_type.h
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       NETWORK PROTOCOL TYPES 
*
* Note(s) : (1) See 'net.h  Note #1'.
*********************************************************************************************************
*/

                                                        /* -------------------- NET PROTOCOL TYPES -------------------- */
#define  NET_PROTOCOL_TYPE_NONE                    0
#define  NET_PROTOCOL_TYPE_ALL                     1

                                                        /* ------------------- LINK LAYER PROTOCOLS ------------------- */
#define  NET_PROTOCOL_TYPE_FRAME                  10

#define  NET_PROTOCOL_TYPE_ARP                    20

                                                        /* ------------------ NETWORK LAYER PROTOCOLS ----------------- */
#define  NET_PROTOCOL_TYPE_IP                     30
#define  NET_PROTOCOL_TYPE_IP_OPT                 31
#define  NET_PROTOCOL_TYPE_ICMP                   32

                                                        /* ----------------- TRANSPORT LAYER PROTOCOLS ---------------- */
#define  NET_PROTOCOL_TYPE_UDP                    40
#define  NET_PROTOCOL_TYPE_TCP                    41

                                                        /* ---------------- APPLICATION LAYER PROTOCOLS --------------- */
#define  NET_PROTOCOL_TYPE_APP                    50
#define  NET_PROTOCOL_TYPE_SOCK                   51


/*$PAGE*/
/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            NETWORK TYPE
*
* Note(s) : (1) 'NET_TYPE' declared as 'CPU_INT32U' & all 'NET_TYPE's #define'd with large, non-trivial 
*               values to trap & discard invalid/corrupted network objects based on 'NET_TYPE'.
*********************************************************************************************************
*/

typedef  CPU_INT32U  NET_TYPE;


/*
*********************************************************************************************************
*                                     NETWORK PROTOCOL DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_PROTOCOL_TYPE;


/*
*********************************************************************************************************
*                                     NETWORK CHECK SUM DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_CHK_SUM;


/*
*********************************************************************************************************
*                                     NETWORK TIMESTAMP DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT32U  NET_TS;


/*
*********************************************************************************************************
*                            NETWORK TRANSPORT LAYER PORT NUMBER DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_PORT_NBR;


/*
*********************************************************************************************************
*                        NETWORK TRANSPORT LAYER PORT NUMBER QUANTITY DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT16U  NET_PORT_NBR_QTY;                          /* Defines max qty of port nbrs to support.             */


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                                NETWORK MODULE & LAYER GLOBAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*
* Note(s) : (1) These module & layer global data types are required PRIOR to network configuration.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                               NETWORK CONNECTION MANAGEMENT DATA TYPES
*
* Note(s) : (1) See 'net_conn.h  NETWORK CONNECTION IDENTIFICATION DATA TYPE  Note #1'.
*********************************************************************************************************
*/

typedef  CPU_INT16S  NET_CONN_ID;

