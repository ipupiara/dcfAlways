/*
*********************************************************************************************************
*                                                uC/TTCP
*                                  TCP-IP Transfer Measurement Utility
*
*                          (c) Copyright 2004-2007; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                  TCP-IP TRANSFER MEASUREMENT UTILITY
*
* Filename      : ttcp.c
* Version       : V1.90
* Programmer(s) : CL
*                 JDH
*                 ITJ
*********************************************************************************************************
* Note(s)       : This application is a Micrium implementation of a publically available TCP test tool: TTCP.
*                 This implementation uses uC/OS-II and uC/TCP-IP.
*                 The tool was adapted to Micrium uC/TCP-IP and coding standards.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define   TTCP_MODULE

#include  <includes.h>


/*
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*/

#define  TTCP_INPUT_STRING_LEN                            80
#define  TTCP_OUTPUT_STRING_LEN                           50


/*
*********************************************************************************************************
*                                               DEFAULTS
*********************************************************************************************************
*/

                                                                /* TTCP parameters for defaults and initialization.     */
#define  TTCP_REMOTE_IP                (CPU_CHAR*)("0.0.0.0")   /* Remote IP address.                                   */
#define  TTCP_NUM_BUF                                   2048    /* Number of buffers used.                              */
#define  TTCP_TRANSPORT                                    0    /* Transport layer mode set : 0 = TCP; 1 = UDP.         */
#define  TTCP_SOCK_OPT                                     0    /* No socket options.                                   */
#define  TTCP_PORT                                      5001    /* TCP or UDP port number to use.                       */
#define  TTCP_CLI_SRVR                                     0    /* 0 = Rx; 1 = Tx.                                      */
#define  TTCP_SINK_MODE                                    1    /* Sink / source mode.                                  */
#define  TTCP_NO_DELAY                                     0    /* No TCP_NODELAY socket option.                        */
#define  TTCP_BLOCK                                        0    /* -B option to read full blocks only.                  */
#define  TTCP_SOCK_BUF_SIZE                                0    /* Socket buffer size (not currently supported).        */
#define  TTCP_FMT                                         'K'   /* Output format: K = kilobytes.                        */
#define  TTCP_TOUCH_DATA                                   0    /* Do not access data after reading.                    */
#define  TTCP_LOOP_MODE                                    0    /* Infinite loop the current operation.                 */

#define  TTCP_NUM_CONNECTIONS                              1    /* Number of socket connections.                        */
#define  TTCP_MAX_CONNS                                   10    /* Max number of socket connections.                    */


/*$PAGE*/
/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

                                                                /* ---- TTCP Global Variables ------------------------- */
    CPU_BOOLEAN        TTCP_NetInitOK;                          /* Flag for successful uC/TCP-IP initialization.        */
    NET_SOCK_ID        TTCP_ListenSockID;                       /* Socket ID for the TCP listening socket.              */
    NET_SOCK_ID        TTCP_ConnSockID[TTCP_MAX_CONNS];         /* Socket IDs of the connected or accepted socket used. */
    NET_SOCK_ADDR_LEN  TTCP_RemoteAddLen;                       /* Length of remote address.                            */
    NET_SOCK_ADDR_IP   TTCP_RemoteAddrPort;                     /* Socket parameter for the IP address and port number  */
                                                                /* for the connections.                                 */

    CPU_INT08U         TTCP_NumConns;                           /* Number of socket connections.                        */

    CPU_CHAR           TTCP_RemoteIP[16];                       /* Remote host IP address.                              */
    CPU_INT16U         TTCP_BufLen;                             /* Length of buffer to use for TTCP.                    */
    CPU_CHAR           TTCP_Buf[TTCP_CFG_BUF_LEN];              /* Buffer used for TTCP, maximum size of 64K.           */
    CPU_INT16U         TTCP_NumBufInit;                         /* Number of buffers initially to send.                 */
    CPU_INT16U         TTCP_NumBuf;                             /* Number of buffers           to send in sink mode.    */
    CPU_INT08U         TTCP_Transport;                          /* Layer 4 protocol to use : 0 = tcp, !0 = udp.         */
    CPU_INT08U         TTCP_SockOpt;                            /* Socket options are possible.                         */
    CPU_INT16U         TTCP_Port;                               /* Port number of the TTCP server.                      */
    CPU_INT08U         TTCP_Cli_Srvr;                           /* Determine if this device will be a client or server. */
                                                                /* 0 = Receive (server), !0 = Transmit (client).        */
    CPU_INT08U         TTCP_SinkMode;                           /* Sinkmode set automatic operation with a pattern.     */
                                                                /* buffer.  0 = normal I/O via serial terminal,         */
                                                                /* !0 = sink/source mode.                               */
    CPU_INT08U         TTCP_NoDelay;                            /* Set TCP_NODELAY socket option.                       */
    CPU_INT08U         TTCP_Block;                              /* Use mread() - Multiple read when using fixed buffers.*/
    CPU_INT16U         TTCP_SockBufSize;                        /* Socket buffer size to use.                           */
    CPU_CHAR           TTCP_Fmt;                                /* Output format: k = kilobits, K = kilobytes,          */
                                                                /*                m = megabits, M = megabytes,          */
                                                                /*                g = gigabits, G = gigabytes.          */
    CPU_INT08U         TTCP_TouchData;                          /* Access data after reading.                           */
    CPU_INT08U         TTCP_LoopMode;                           /* Infinite loop the current operation.                 */
    CPU_CHAR           TTCP_Stats[128];                         /* String used to ouput the test results.               */
    CPU_INT32U         TTCP_NumBytes;                           /* Number of bytes carried on the network.              */

    CPU_INT32U         TTCP_NumIntvl[TTCP_MAX_CONNS];           /* Number of interval bytes sent.                       */
    CPU_INT32U         TTCP_NumBytesStream[TTCP_MAX_CONNS];     /* Number of interval bytes sent.                       */

    CPU_INT32U         TTCP_NumCalls;                           /* Number of I/O system calls.                          */
    CPU_INT32U         TTCP_NumTxErrs;                          /* Number of transmit errors.                           */
    CPU_INT32U         TTCP_NumRcvErrs;                         /* Number of receive errors.                            */

    CPU_INT32U         TTCP_NumUnknownTxRet;                    /* Number of "default" returns from Tx.                 */

    CPU_INT32U         TTCP_RcvTimeOutCnts;                     /* Number of Rcv calls returning a -1.                  */

    CPU_CHAR           TTCP_OutBuf[TTCP_OUTPUT_STRING_LEN];     /* String used for serial port formatted output.        */

    CPU_INT32U         TTCP_PerfTimerStart;                     /* Timer used to calculate the performance statistics.  */
    CPU_INT32U         TTCP_PerfTimerEnd;                       /* Timer used to calculate the performance statistics.  */

    CPU_BOOLEAN        TTCP_Abort;                              /* Abort Flag.                                          */


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  NET_SOCK_ID  TTCP_TaskTCP_Init (void);
static  NET_SOCK_ID  TTCP_TaskUDP_Init (void);


static  void         TTCP_Pattern      (CPU_CHAR    *cp,
                                        CPU_INT16U   cnt);
static  void         TTCP_OutFmt       (CPU_FP32     b);
static  CPU_BOOLEAN  TTCP_Parse        (CPU_CHAR    *TTCPCmdLine);


static  void         TTCP_TxTCP        (CPU_CHAR    *cp,
                                        CPU_INT16U   cnt);
static  void         TTCP_TxUDP        (CPU_CHAR    *cp,
                                        CPU_INT16U   cnt);

static  void         TTCP_RxTCP        (void);
static  void         TTCP_RxUDP        (void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                              TTCP_Init()
*
* Description: This function initializes the TTCP application.
*
* Arguments :
*
* Returns   :  DEF_FAIL     TTCP failed to initialized.
*              DEF_OS       TTCP initialized correctly.
*********************************************************************************************************
*/

CPU_BOOLEAN  TTCP_Init (void)
{
    CPU_BOOLEAN  err;


    err = TTCP_OS_Init((void *)0);

    return (err);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              TTCP_Task()
*
* Description : This function is the main menu for the application.
*
* Arguments   : None.
*********************************************************************************************************
*/

void  TTCP_Task (void  *p_arg)
{
    NET_ERR            TTCP_Err;
    CPU_INT32U         TTCP_DataSent;                           /* Tells how many bytes were sent by the TCP echo       */
                                                                /* server.  Always one in our case.                     */

    NET_SOCK_ID        TTCP_ListenSockID;                       /* Socket ID used for the Listening socket when in      */
                                                                /* Receive-Server mode.                                 */
    NET_IP_ADDR        TTCP_RemoteAddr;                         /* IP address of the remote host to conduct the test.   */

    NET_SOCK_RTN_CODE  TTCP_RtnCode;                            /* Used to store the stack fucntion calls return code.  */
    CPU_INT32U         TTCP_PerfTimerDelta;                     /* Timer used to calculate the performance statisitics. */
    CPU_FP32           TTCP_PerfTimerSecs;                      /* Timer used to calculate the performance statisitics  */
                                                                /* in seconds.                                          */
    CPU_FP32           TTCP_BytesPerSec;
    CPU_FP32           TTCP_BitsPerSec;
    CPU_FP32           TTCP_CallsPerSec;
    CPU_FP32           TTCP_msPerCall;

    CPU_INT16U         TTCP_TCP_UDP_End;                        /* Counter to send 4 UDP 4 bytes datagram to end the    */
                                                                /* statistics.                                          */
    CPU_INT32U         TTCP_Total;                              /* Number of bytes expected to receive when in Server   */
                                                                /* Mode.                                                */

    CPU_CHAR           TTCP_Str[TTCP_INPUT_STRING_LEN];         /* Uses as the input string for the TTCP arguments and  */
                                                                /* options.                                             */
    CPU_BOOLEAN        TTCP_ParseErr;                           /* Records the success for parameter input.             */

    CPU_INT32U         i;


   (void)p_arg;                                                 /* Prevent compiler warning.                            */

    TTCP_ParseErr = DEF_YES;
    for (i = 0; i < TTCP_MAX_CONNS; i++) {
        TTCP_ConnSockID[i] = -1;                                /* No Client connection active.                         */
    }

    while (DEF_TRUE) {
        if (TTCP_LoopMode == 0) {
            while (TTCP_ParseErr) {
                                                                /* Initialize test parameters to defaults.              */
                Str_Copy(TTCP_RemoteIP, TTCP_REMOTE_IP);        /* Set-up the Remote IP address.                        */
                TTCP_BufLen      = TTCP_CFG_BUF_LEN;            /* Buffer size.                                         */
                TTCP_NumBufInit  = TTCP_NUM_BUF;                /* Number of buffers used.                              */
                TTCP_Transport   = TTCP_TRANSPORT;              /* Transport layer mode set 0 = TCP (default), 1 = UDP. */
                TTCP_SockOpt     = TTCP_SOCK_OPT;               /* Socket options.                                      */
                TTCP_Port        = TTCP_PORT;                   /* TCP port number.                                     */
                TTCP_Cli_Srvr    = TTCP_CLI_SRVR;               /* Set the device mode.                                 */
                TTCP_SinkMode    = TTCP_SINK_MODE;              /* Sink/source mode.                                    */
                TTCP_NoDelay     = TTCP_NO_DELAY;               /* TCP_NODELAY socket option.                           */
                TTCP_Block       = TTCP_BLOCK;                  /* -B option of the receiver (mread()) setting.         */
                TTCP_SockBufSize = TTCP_SOCK_BUF_SIZE;          /* Socket buffer size not currently supported by        */
                                                                /* uC/TCP-IP.                                           */
                TTCP_Fmt         = TTCP_FMT;                    /* Output format: K = kilobytes.                        */
                TTCP_TouchData   = TTCP_TOUCH_DATA;             /* Setting to access or not data after reading.         */
                TTCP_LoopMode    = TTCP_LOOP_MODE;              /* Infinite loop the current operation.                 */

                TTCP_NumConns    = TTCP_NUM_CONNECTIONS;        /* Number of socket connections.                        */

                TTCP_TRACE_INFO(((CPU_CHAR*)"uC/TCP-IP Performance measurements\r\n"));
                TTCP_TRACE_INFO(((CPU_CHAR*)"Type the TTCP parameters, parameter values and press Enter\r\n"));
                TTCP_TRACE_INFO(((CPU_CHAR*)">"));
                Ser_RdStr(TTCP_Str, TTCP_INPUT_STRING_LEN);

                TTCP_ParseErr    = TTCP_Parse(TTCP_Str);        /* Parse the input string for parameters.               */
            }
        }

        TTCP_NumCalls        = 0;                               /* Reset the system call counters to 0.                 */
        TTCP_RcvTimeOutCnts  = 0;
        TTCP_NumRcvErrs      = 0;
        TTCP_NumTxErrs       = 0;
        TTCP_NumUnknownTxRet = 0;
        TTCP_NumBuf          = TTCP_NumBufInit;
        TTCP_Abort           = DEF_NO;

        if (TTCP_Transport) {
                                                                /* Maximum UDP datagram because uC/TCP-IP does not      */
                                                                /*    fragment.                                         */
                                                                /*    NET_IP_MTU   = Minimum(1500, 1596) - 60           */
                                                                /*                 = 1500 - 60                          */
                                                                /*                 = 1440                               */

                                                                /*    NET_UDP MTU  = IP MTU - UDP header size           */
                                                                /*    NET_UDP_MTU  = 1440 - 8                           */
                                                                /*                 = 1432                               */
            if (TTCP_BufLen > NET_UDP_MTU) {
                TTCP_BufLen = NET_UDP_MTU;
            }
        }

        for (i = 0; i < TTCP_MAX_CONNS; i++) {
            TTCP_NumIntvl[i]       = 0;
            TTCP_NumBytesStream[i] = 0;
        }


        if (TTCP_Cli_Srvr) {                                    /* Output test parameters to serial port.               */
            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp-t: BufLen=%d, NumBuf=%d, port=%d, NumConns=%d",
                              TTCP_BufLen, TTCP_NumBuf, TTCP_Port, TTCP_NumConns));
            if (TTCP_SockBufSize) {
                TTCP_TRACE_INFO(((CPU_CHAR*)", sockbufsize=%d", TTCP_SockBufSize));
            }
            TTCP_TRACE_INFO(((CPU_CHAR*)"  %s  -> %s\r\n", TTCP_Transport ? "udp" : "tcp", TTCP_RemoteIP));

        } else {
            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp-r: BufLen=%d, NumBuf=%d, port=%d, NumConns=%d",
                              TTCP_BufLen, TTCP_NumBuf, TTCP_Port, TTCP_NumConns));

            if (TTCP_SockBufSize) {
                TTCP_TRACE_INFO(((CPU_CHAR*)", sockbufsize=%d", TTCP_SockBufSize));
            }
            TTCP_TRACE_INFO(((CPU_CHAR*)"  %s\r\n", TTCP_Transport ? "udp" : "tcp"));
        }


        if (TTCP_Transport && (TTCP_BufLen < 5)) {
            TTCP_BufLen = 5;                                    /* Send more than the UDP sentinel size of 4.           */
        }

                                                                /* IP address for socket operations.                    */
        TTCP_RemoteAddrPort.Family = AF_INET;
        TTCP_RemoteAddr            = NetASCII_Str_to_IP(TTCP_RemoteIP, &TTCP_Err);
        TTCP_RemoteAddrPort.Addr   = NET_UTIL_HOST_TO_NET_32(TTCP_RemoteAddr);

        TTCP_TRACE_INFO(((CPU_CHAR*)"TTCP_RemoteIP = %s, TTCP_RemoteAddr = %x\n\r",
                          TTCP_RemoteIP, TTCP_RemoteAddr));

        TTCP_RemoteAddrPort.Port   = NET_UTIL_HOST_TO_NET_16(TTCP_Port);
        TTCP_RemoteAddLen          = sizeof(TTCP_RemoteAddrPort);


        if (TTCP_Cli_Srvr) {                                    /* Client mode.  Acts as a Transmitter.                 */

            /* Add code for multiple socket connections. */
            for (i = 0; i < TTCP_NumConns; i++) {
                if (TTCP_Transport) {                           /* Layer 4 protocol is UDP.                             */
                                                                /* Open a UDP socket to send data to the remote host.   */
                    TTCP_ConnSockID[i] = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
                                                       NET_SOCK_TYPE_DATAGRAM,
                                                       NET_SOCK_PROTOCOL_UDP,
                                                      &TTCP_Err);

                } else {                                        /* Layer 4 protocol is TCP.                             */
                                                                /* Open a TCP socket to initiate connection with remote */
                                                                /* host.                                                */
                    TTCP_ConnSockID[i] = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
                                                       NET_SOCK_TYPE_STREAM,
                                                       NET_SOCK_PROTOCOL_TCP,
                                                      &TTCP_Err);
                }

                if (TTCP_ConnSockID[i] > -1) {                  /* Socket opened successfully.                          */
                                                                /* Set blocking timeout until we 'connect'.             */
                    NetSock_CfgTimeoutConnReqSet(TTCP_ConnSockID[i], TTCP_CFG_MAX_CONN_TIMEOUT_MS, &TTCP_Err);

                    TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Client socket %d opened\r\n",
                                      TTCP_Cli_Srvr?"-t":"-r", TTCP_ConnSockID[i]));

                    TTCP_TRACE_INFO(((CPU_CHAR*)"Connecting to addr: %x, port: %x\r\n",
                                      TTCP_RemoteAddrPort.Addr,
                                      TTCP_RemoteAddrPort.Port));

                    TTCP_RtnCode = NetSock_Conn( TTCP_ConnSockID[i],
                                                (NET_SOCK_ADDR *)&TTCP_RemoteAddrPort,
                                                 TTCP_RemoteAddLen,
                                                &TTCP_Err);

                    if (TTCP_RtnCode != NET_SOCK_BSD_ERR_CONN) {
                        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Client socket %d connected\r\n",
                                          TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ConnSockID[i]));
                    } else {
                        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Unable to connect Client socket. Server may not be running. Error: %d\r\n",
                                          TTCP_Cli_Srvr ? "-t" : "-r", TTCP_Err));
                        NetSock_Close(TTCP_ConnSockID[i], &TTCP_Err);
                        TTCP_ConnSockID[i] = -1;
                        TTCP_Abort = DEF_YES;
                    }
                } else {
                    TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Unable to open socket.\r\n",
                                      TTCP_Cli_Srvr ? "-t" : "-r"));
                }
            }

        } else {                                                /* Server mode.  Acts as a Receiver.                    */

            if (TTCP_Transport) {                               /* Layer 4 protocol is UDP.                             */

                i = 0;

                TTCP_ConnSockID[i] = TTCP_TaskUDP_Init();       /* Open a socket for incoming connections.              */
                TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Waiting for client to send UDP datagrams.\r\n",
                                  TTCP_Cli_Srvr?"-t":"-r"));

                                                                /* Set blocking timeout to infinite until the Client    */
                                                                /* connects.                                            */
                NetSock_CfgTimeoutRxQ_Set(TTCP_ConnSockID[i], NET_TMR_TIME_INFINITE, &TTCP_Err);

            } else {                                            /* Layer 4 protocol is TCP.                             */
                TTCP_ListenSockID = TTCP_TaskTCP_Init();        /* Open a socket to listen for incoming connections.    */

                if (TTCP_ListenSockID > -1) {
                    TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Waiting for %d client to request connection.\r\n",
                                      TTCP_Cli_Srvr ? "-t" : "-r", TTCP_NumConns));

                    TTCP_TRACE_INFO(((CPU_CHAR*)"TTCP_RemoteAddLen = %d, NET_SOCK_ADDR_SIZE=%d.\r\n",
                                      TTCP_RemoteAddLen, NET_SOCK_ADDR_SIZE));

                    /* ------------------------------------------------------------- */
                    /* Wait for all of our TCP connections.                          */
                    /* ------------------------------------------------------------- */

                    i = 0;

                    while (i < TTCP_NumConns) {
                        /* --------------------------------------------------------- */
                        /* PJC - Needed to add the following line, if the            */
                        /* NET_ERR_CFG_ARG_CHK_EXT_EN argument in net_cfg.h is set   */
                        /* to DEF_ENABLE then the NetSock_Accept will check the      */
                        /* arguments being passed in.  After the first call to       */
                        /* NetSock_Accept, if a timeout occurs (500 ms) before the   */
                        /* client connects then the TTCP_RemoteAddLen is set to 0.   */
                        /* Thus unless we reset the variable to the correct size     */
                        /* NetSock_Accept will return with an Invalid sock addr len  */
                        /* Error.                                                    */
                        /* --------------------------------------------------------- */

                        TTCP_RemoteAddLen = sizeof (TTCP_RemoteAddrPort);

                        TTCP_ConnSockID[i] = NetSock_Accept( TTCP_ListenSockID,
                                                            (NET_SOCK_ADDR *)&TTCP_RemoteAddrPort,
                                                            &TTCP_RemoteAddLen,
                                                            &TTCP_Err);

                        if (TTCP_Err != NET_SOCK_ERR_NONE) {            /* Could not accept socket on the TCP port.                      */
                            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Unable to accept socket. Client may not be running. Error: %d\r\n",
                                              TTCP_Cli_Srvr ? "-t" : "-r", TTCP_Err));
                            NetSock_Close(TTCP_ListenSockID, &TTCP_Err);
                            TTCP_Abort = DEF_YES;
                            break;
                        }

                        if (TTCP_ConnSockID[i] >= 0) {
                            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Server socket %d active \r\n",
                                              TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ConnSockID[i]));
                        }

                        i++;
                    }
                }
            }

            /* ---------------------------------------------------------------- */
            /* See if we have all of the sockets connected.  If so proceed with */
            /* the data transfer.                                               */
            /* ---------------------------------------------------------------- */

            for (i = 0;  i < TTCP_NumConns; i++) {
                if (TTCP_ConnSockID[i] < 0) {
                    TTCP_Abort = DEF_YES;
                    TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp: Connection Timeout Detected - Retrying...\r\n"));
                    break;
                }
            }
        }

        if (TTCP_Abort != DEF_YES) {                            /* Proceed with the transmission/reception of the test  */
                                                                /* data.                                                */

            TTCP_PerfTimerStart = OSTimeGet();                  /* Initialize test timer.                               */

            if (TTCP_SinkMode) {

                if (TTCP_Cli_Srvr) {                            /* Client acts as Transmitter.                          */
                    TTCP_Pattern(TTCP_Buf, TTCP_BufLen);        /* Fill the buffer with printable characters.           */
                    if (TTCP_Transport) {                       /* Layer 4 is UDP.                                      */

                        i = 0;
                                                                /* Send a 4 bytes datagram to trigger the start of the  */
                                                                /* test.                                                */
                        TTCP_DataSent = NetSock_TxDataTo( TTCP_ConnSockID[i],
                                                          TTCP_Buf,
                                                          4,
                                                          0,
                                                         (NET_SOCK_ADDR *)&TTCP_RemoteAddrPort,
                                                          TTCP_RemoteAddLen,
                                                         &TTCP_Err);

                        TTCP_Total    = TTCP_BufLen * TTCP_NumBuf;
                        TTCP_NumBytes = 0;

                        while (TTCP_NumBytes < TTCP_Total) {
                            TTCP_TxUDP(TTCP_Buf, TTCP_BufLen);
                            if (TTCP_ConnSockID[i] == -1) {        /* Socket was closed.  Terminate test.                  */
                                break;
                            }
                        }

                        for (TTCP_TCP_UDP_End = 0; TTCP_TCP_UDP_End < 4; TTCP_TCP_UDP_End++) {
                                                                /* Send a 4 bytes datagram to signal the end of the     */
                                                                /* test.                                                */
                            TTCP_DataSent = NetSock_TxDataTo( TTCP_ConnSockID[i],
                                                              TTCP_Buf,
                                                              4,
                                                              0,
                                                             (NET_SOCK_ADDR *)&TTCP_RemoteAddrPort,
                                                              TTCP_RemoteAddLen,
                                                             &TTCP_Err);
                        }
                    } else {                                    /* Layer 4 is TCP.                                      */
                        TTCP_NumBytes = 0;
                        while (TTCP_NumBuf != 0) {
                                                                /* Send a buffer.                                       */
                            TTCP_TxTCP(TTCP_Buf, TTCP_BufLen);
                            TTCP_NumBuf--;
                            if (TTCP_Abort == DEF_YES) {        /* Sokcet was closed. Terminate test.                   */
                                break;
                            }
                        }
                    }

                } else {                                        /* Server Mode. Acts as Receiver.                       */

                    if (TTCP_Transport) {                       /* Layer 4 protocol is UDP.                             */
                        TTCP_NumBytes = 0;
                        TTCP_RxUDP();
                    } else {                                    /* Layer 4 protocol is TCP.                             */
                        TTCP_NumBytes = 0;
                        TTCP_RxTCP();
                    }
               }
            } else {                                            /* Not Sink Mode.                                       */
                TTCP_NumBytes = 0;
                if (TTCP_Cli_Srvr) {
                    Ser_RdStr(TTCP_Str, TTCP_INPUT_STRING_LEN);
                                                                /* Add code to send via TCP or UDP.                     */
                    TTCP_NumBytes += TTCP_DataSent;
                    TTCP_NumCalls++;
                } else {
                    if (TTCP_Transport) {                       /* Layer 4 protocol is UDP.                             */
                        TTCP_NumBytes = 0;
                        TTCP_RxUDP();
                    } else {                                    /* Layer 4 protocol is TCP.                             */
                        TTCP_NumBytes = 0;
                        TTCP_RxTCP();
                    }
                }
            }

                                                                /* Take the final time for the statistics.              */
            TTCP_PerfTimerEnd   = OSTimeGet();
                                                                /* Calculate the elapsed time.                          */
            TTCP_PerfTimerDelta = TTCP_PerfTimerEnd - TTCP_PerfTimerStart;
            TTCP_PerfTimerSecs  = TTCP_PerfTimerDelta / 1000.0;

            if (TTCP_Cli_Srvr) {                                /* Close all used sockets.                              */
                for (i = 0; i < TTCP_NumConns; i++) {
                    if (TTCP_ConnSockID[i] > -1) {
                        NetSock_Close(TTCP_ConnSockID[i], &TTCP_Err);
                        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Client socket %d closed\r\n",
                                          TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ConnSockID[i]));
                    }
                }
            } else {
                for (i = 0; i < TTCP_NumConns; i++) {
                    TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Client socket %d closed\r\n",
                                      TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ConnSockID[i]));
                                                                /* Close the connected sockets.                          */
                    NetSock_Close(TTCP_ConnSockID[i], &TTCP_Err);
                }
                if (!TTCP_Transport) {
                    TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Listen socket %d closed\r\n",
                                      TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ListenSockID));
                                                                /* Close the listening socket.                          */
                    NetSock_Close(TTCP_ListenSockID, &TTCP_Err);
                }
            }

            TTCP_BytesPerSec =  TTCP_NumBytes   / TTCP_PerfTimerSecs;
            TTCP_BitsPerSec  =  TTCP_BytesPerSec * DEF_OCTET_NBR_BITS;
            TTCP_msPerCall   = (DEF_TIME_NBR_mS_PER_SEC * TTCP_PerfTimerSecs) / (CPU_FP32)TTCP_NumCalls;
            TTCP_CallsPerSec = (CPU_FP32)TTCP_NumCalls  / TTCP_PerfTimerSecs;

            TTCP_OutFmt(TTCP_BytesPerSec);

                                                                /* Outputs test results.                                */
            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: %u bytes in %u.%u real sec = %s/sec (%u.%u bps)",
                              TTCP_Cli_Srvr ? "-t" : "-r",
                             (CPU_INT32U) TTCP_NumBytes,
                             (CPU_INT32U) TTCP_PerfTimerSecs,
                            ((CPU_INT32U)(TTCP_PerfTimerSecs * 1000) % 1000),
                              TTCP_OutBuf,
                             (CPU_INT32U) TTCP_BitsPerSec,
                            ((CPU_INT32U)(TTCP_BitsPerSec    * 1000) % 1000)));
            TTCP_TRACE_INFO(((CPU_CHAR*)"\r\n"));

            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: %d I/O calls, msec/call = %u.%u, calls/sec = %u.%u\r\n",
                              TTCP_Cli_Srvr ? "-t" : "-r",
                             (CPU_INT32U) TTCP_NumCalls,
                             (CPU_INT32U) TTCP_msPerCall,
                            ((CPU_INT32U)(TTCP_msPerCall   * 1000) % 1000),
                             (CPU_INT32U) TTCP_CallsPerSec,
                            ((CPU_INT32U)(TTCP_CallsPerSec * 1000) % 1000)));
            TTCP_TRACE_INFO(((CPU_CHAR*)"\r\n"));
            TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: %ld Rcv Timeouts, %d Rcv Errs %d Tx Errs, %d Unknown TX Rets\n\r",
                              TTCP_Cli_Srvr ? "-t" : "-r",
                             (CPU_INT32U) TTCP_RcvTimeOutCnts,
                             (CPU_INT32U) TTCP_NumRcvErrs,
                             (CPU_INT32U) TTCP_NumTxErrs,
                             (CPU_INT32U) TTCP_NumUnknownTxRet));
            TTCP_TRACE_INFO(((CPU_CHAR*)"\r\n"));
        }

        OSTimeDlyHMSM(0, 0, 0, 250);
        TTCP_ParseErr = DEF_YES;                                /* Set to re-enter the input menu.                      */
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                               TTCP_TaskTCP_Init()
*
* Description: This function initializes a socket to be used as a TCP server on port TTCP_Port.
*
* Arguments  : None.
*********************************************************************************************************
*/

static  NET_SOCK_ID  TTCP_TaskTCP_Init (void)
{
    CPU_INT32S        bind_status;
    CPU_INT32S        listen_status;
    NET_ERR           err;
    NET_SOCK_ADDR_IP  TTCPTaskTCP_SockAddr;
    NET_SOCK_ID       TTCPTaskTCP_SockID;
    NET_SOCK_Q_SIZE   TTCPTaskTCP_Q;                            /* Cur Q size to accept rx'd conn reqs.                 */
    CPU_INT08U        i;


                                                                /* Open a socket to listen for incoming connections.    */
    TTCPTaskTCP_SockID = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
                                       NET_SOCK_TYPE_STREAM,
                                       NET_SOCK_PROTOCOL_TCP,
                                      &err);

    if (TTCPTaskTCP_SockID < 0) {
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Could not open a socket\r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r"));
        return (TTCPTaskTCP_SockID);                            /* Could not open a socket.                             */
    } else {
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Listening socket opened = %d for PORT: %d\r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r", TTCPTaskTCP_SockID, TTCP_Port));
    }

                                                                /* Bind a local address so the client can send to us.   */
    Mem_Set((char *)&TTCPTaskTCP_SockAddr, 0, NET_SOCK_ADDR_SIZE);
    TTCPTaskTCP_SockAddr.Family = NET_SOCK_ADDR_FAMILY_IP_V4;
    TTCPTaskTCP_SockAddr.Port   = NET_UTIL_HOST_TO_NET_16(TTCP_Port);
    TTCPTaskTCP_SockAddr.Addr   = NET_UTIL_HOST_TO_NET_32(INADDR_ANY);

    bind_status                 = NetSock_Bind( TTCPTaskTCP_SockID,
                                               (NET_SOCK_ADDR *)&TTCPTaskTCP_SockAddr,
                                                NET_SOCK_ADDR_SIZE,
                                               &err);

    if (bind_status != NET_SOCK_BSD_ERR_NONE) {
        NetSock_Close(TTCPTaskTCP_SockID, &err);                /* Could not bind to the TCP port.                      */
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Could not bind to the TCP port\r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r"));
        return (-1);
    }

                                                                /* Set blocking timeout until we 'connect'.             */
    NetSock_CfgTimeoutConnAcceptSet(TTCPTaskTCP_SockID, TTCP_CFG_MAX_ACCEPT_TIMEOUT_MS, &err);

    TTCPTaskTCP_Q = 5;                                          /* Reserve 5 buffers for every socket created from this */
                                                                /* listening socket.                                    */
    listen_status = NetSock_Listen( TTCPTaskTCP_SockID,
                                    TTCPTaskTCP_Q,
                                   &err);

    if (listen_status != NET_SOCK_BSD_ERR_NONE) {
        NetSock_Close(TTCPTaskTCP_SockID, &err);                /* Could not bind open the listen socket.               */
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Could not open listening socket\r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r"));
        return (-1);
    }

    for (i = 0; i < NET_TCP_CFG_NBR_CONN; i++) {
        NetSock_CfgTimeoutRxQ_Set(i, TTCP_CFG_MAX_RX_TIMEOUT_MS, &err);
    }

    return (TTCPTaskTCP_SockID);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                               TTCP_TaskUDP_Init()
*
* Description: This function initializes a scoket to be used as a UDP server on port TTCP_Port.
*
* Arguments  : None.
*********************************************************************************************************
*/

static  NET_SOCK_ID  TTCP_TaskUDP_Init (void)
{
    CPU_INT32S        bind_status;
    NET_ERR           err;
    NET_SOCK_ADDR_IP  TTCPTaskUDP_SockAddr;
    NET_SOCK_ID       TTCPTaskUDP_SockID;


    TTCPTaskUDP_SockID = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
                                       NET_SOCK_TYPE_DATAGRAM,
                                       NET_SOCK_PROTOCOL_UDP,
                                      &err);

    if (TTCPTaskUDP_SockID < 0) {
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Could not open a socket\r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r"));
        return (TTCPTaskUDP_SockID);                            /* Could not open a socket.                             */
    } else {
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: UDP socket %d opened \r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r", TTCPTaskUDP_SockID));
    }

                                                                /* Bind a local address so the client can send to us.   */
    Mem_Set((char *)&TTCPTaskUDP_SockAddr, 0, NET_SOCK_ADDR_SIZE);
    TTCPTaskUDP_SockAddr.Family = NET_SOCK_ADDR_FAMILY_IP_V4;
    TTCPTaskUDP_SockAddr.Port   = NET_UTIL_HOST_TO_NET_16(TTCP_Port);
    TTCPTaskUDP_SockAddr.Addr   = NET_UTIL_HOST_TO_NET_32(INADDR_ANY);

    bind_status                 = NetSock_Bind( TTCPTaskUDP_SockID,
                                               (NET_SOCK_ADDR *)&TTCPTaskUDP_SockAddr,
                                                NET_SOCK_ADDR_SIZE,
                                               &err);

    if (bind_status != NET_SOCK_BSD_ERR_NONE) {
        NetSock_Close(TTCPTaskUDP_SockID, &err);                /* Could not bind to the TTCP port.                     */
        TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Could not bind to the UDP port\r\n",
                          TTCP_Cli_Srvr ? "-t" : "-r"));
        return (-1);
    }

    return (TTCPTaskUDP_SockID);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            TTCP_Pattern()
*
* Description: This function fills a buffer with printable characters.
*
* Arguments  : cp       buffer to fill.
*              cnt      number of printable characters the buffer will contain.
*
*********************************************************************************************************
*/

static  void  TTCP_Pattern (CPU_CHAR    *cp,
                            CPU_INT16U   cnt)
{
    CPU_CHAR     c;
    CPU_BOOLEAN  printable;


    c = 0;

    while (cnt-- > 0)  {
        printable = Str_IsPrint((c & 0x7F));

        while (!printable) {
            c++;
            printable = Str_IsPrint((c & 0x7F));
        }

         c    &= 0x7F;
        *cp++  = c;
         c++;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             TTCP_OutFmt()
*
* Description: This function formats the statistics for output according to the selected format.
*
* Arguments  : b        byte rate achieved during the test.
*
*********************************************************************************************************
*/


static  void  TTCP_OutFmt (CPU_FP32  b)
{
    switch (TTCP_Fmt) {
        default:
        case 'k':
             Str_FmtPrint((char *)TTCP_OutBuf, TTCP_OUTPUT_STRING_LEN,
                           "%.2f Kbit", b * 8.0 / 1000.0);
             break;

        case 'K':
             Str_FmtPrint((char *)TTCP_OutBuf, TTCP_OUTPUT_STRING_LEN,
                           "%.2f KB",   b / 1024.0);
             break;

        case 'm':
             Str_FmtPrint((char *)TTCP_OutBuf, TTCP_OUTPUT_STRING_LEN,
                           "%.2f Mbit", b * 8.0 / (1000.0 * 1000.0));
             break;

        case 'M':
             Str_FmtPrint((char *)TTCP_OutBuf, TTCP_OUTPUT_STRING_LEN,
                           "%.2f MB",   b / (1024.0 * 1024.0));
             break;

        case 'g':
             Str_FmtPrint((char *)TTCP_OutBuf, TTCP_OUTPUT_STRING_LEN,
                           "%.2f Gbit", b * 8.0 / (1000.0 * 1000.0 * 1000.0));
             break;

        case 'G':
             Str_FmtPrint((char *)TTCP_OutBuf, TTCP_OUTPUT_STRING_LEN,
                           "%.2f GB",   b / (1024.0 * 1024.0 * 1024.0));
             break;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             TTCP_Parse()
*
* Description: This function parses the input string gathered via the serial interface.
*
* Arguments  : Socket_ID        ID of the socket used for this TTCP session.
*              TTCP_Buf         array that contains the data to be transmitted.
*              TTCP_BufLen      number of bytes to be transmitted.
*
* Returns    : A boolean indicating if there was an error in the parsing of the command line.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  TTCP_Parse (CPU_CHAR  *TTCPCmdLine)
{
    CPU_CHAR     TTCPPar[14];                                   /* Used to parse parameters from the command line.      */
    CPU_INT08U   TTCPParptr;
    CPU_CHAR     TTCPParVal[4];                                 /* Used to read parameter value from the command line.  */
    CPU_INT08U   TTCP_Strptr;
    CPU_INT08U   TTCP_StrLen;
    CPU_BOOLEAN  TTCP_StrErr;                                   /* Record command line parsing error.                   */
    CPU_BOOLEAN  printable;


    TTCP_Strptr = 0;
    TTCP_StrLen = Str_Len(TTCPCmdLine);

    if (TTCP_StrLen == 0) {
        TTCP_StrErr = DEF_YES;                                  /* No parameters, make sur the help is displayed.       */
    } else {
        TTCP_StrErr = DEF_NO;                                   /* Clear all parsing errors.                            */
    }

    while (TTCP_Strptr < TTCP_StrLen) {
        printable = isprint(TTCPCmdLine[TTCP_Strptr]);

        while (((TTCPCmdLine[TTCP_Strptr] == ' ') |             /* Remove spaces from the command line                  */
                (!printable)) &
                (TTCP_Strptr < TTCP_StrLen)) {                  /* until end of string.                                 */

            TTCP_Strptr++;
            printable = isprint(TTCPCmdLine[TTCP_Strptr]);
        }

        TTCPParptr          = 0;                                /* Initialize parameter string pointer.                 */
        TTCPPar[TTCPParptr] = '\0';

        while ((TTCPCmdLine[TTCP_Strptr] != ' ') &              /* Extract the parameter until next space character or  */
               (TTCPCmdLine[TTCP_Strptr] != '\n') &             /* line feed character or                               */
               (TTCPCmdLine[TTCP_Strptr] != '\r') &             /* carriage return character or                         */
               (TTCP_Strptr < TTCP_StrLen)) {                   /* end of string.                                       */

            TTCPPar[TTCPParptr] = TTCPCmdLine[TTCP_Strptr];
            TTCP_Strptr++;
            TTCPParptr++;
        }

        TTCPPar[TTCPParptr] = '\0';                             /* Terminate parameter string.                          */

        if (TTCPPar[0] == '-') {
            switch (TTCPPar[1]) {                               /* Determine which parameter.                           */
                case 'r':                                       /* -r      Receiver Mode = Server Mode.                 */
                     TTCP_Cli_Srvr = 0;
                     break;

                case 't':                                       /* -t      Trasnmitter Mode = Client Mode.              */
                     TTCP_Cli_Srvr = 1;
                     break;

                case 'd':                                       /* -d      set SO_DEBUG socket option - not supported.  */
                     TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp: -d option ignored: SOCKET options not supported\r\n"));
                     break;

                case 'D':                                       /* -D      don't buffer TCP writes (sets TCP_NODELAY    */
                                                                /*         socket option).                              */
                     TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp: -D option ignored: TCP_NODELAY option not supported\r\n"));
                     break;

                case 'n':
                     TTCPParptr = 0;                            /* -n ##   number of source bufs written to network     */
                                                                /*         (default 2048).                              */
                     while (TTCPCmdLine[TTCP_Strptr] == ' '){
                         TTCP_Strptr++;
                     }
                     TTCPParptr = 0;
                     while ((TTCPCmdLine[TTCP_Strptr] != ' ') &
                            (TTCP_Strptr < TTCP_StrLen)       &
                            (!TTCP_StrErr)) {                   /* Get parameter value.                                 */
                         TTCPParVal[TTCPParptr] = TTCPCmdLine[TTCP_Strptr];
                         if (TTCPParVal[TTCPParptr] == '-') {   /* If not a parameter value, Flag error.                */
                             TTCP_StrErr = DEF_YES;
                         }
                         TTCP_Strptr++;
                         TTCPParptr++;
                     }
                     if (!TTCP_StrErr) {
                         TTCPParVal[TTCPParptr] = '\0';
                                                                /* Convert the parameter value from string.             */
                         TTCP_NumBufInit = (CPU_INT16U) Str_ToLong(TTCPParVal, 0, 10);
                         if (TTCP_NumBufInit == 0) {            /* If invalid parameter value, Flag error.              */
                             TTCP_StrErr = DEF_YES;
                         }
                     }
                     break;

                case 'l':
                     TTCPParptr = 0;
                     while (TTCPCmdLine[TTCP_Strptr] == ' '){
                         TTCP_Strptr++;
                     }
                     TTCPParptr = 0;
                     while ((TTCPCmdLine[TTCP_Strptr] != ' ') &
                            (TTCP_Strptr < TTCP_StrLen)       &
                            (!TTCP_StrErr)) {                   /* Get parameter value.                                 */
                         TTCPParVal[TTCPParptr] = TTCPCmdLine[TTCP_Strptr];
                         if (TTCPParVal[TTCPParptr] == '-') {
                             TTCP_StrErr = DEF_YES;             /* If not a parameter value, Flag error.                */
                         }
                         TTCP_Strptr++;
                         TTCPParptr++;
                     }
                     if (!TTCP_StrErr) {
                         TTCPParVal[TTCPParptr] = '\0';
                                                                /* Convert the parameter value from string.             */
                         TTCP_BufLen = (CPU_INT16U) Str_ToLong(TTCPParVal, 0, 10);
                         if (TTCP_BufLen == 0) {                /* If invalid parameter value, Flag error.              */
                             TTCP_StrErr  = DEF_YES;
                         }
                     }
                     break;

                case 'p':
                     TTCPParptr = 0;
                     while (TTCPCmdLine[TTCP_Strptr] == ' ') {
                         TTCP_Strptr++;
                     }
                     TTCPParptr = 0;
                     while ((TTCPCmdLine[TTCP_Strptr] != ' ') &
                            (TTCP_Strptr < TTCP_StrLen)       &
                            (!TTCP_StrErr)) {                   /* Get parameter value.                                 */
                         TTCPParVal[TTCPParptr] = TTCPCmdLine[TTCP_Strptr];
                         if (TTCPParVal[TTCPParptr] == '-') {
                             TTCP_StrErr = DEF_YES;             /* If not a parameter value, Flag error.                */
                         }
                         TTCP_Strptr++;
                         TTCPParptr++;
                     }
                     if (!TTCP_StrErr) {
                         TTCPParVal[TTCPParptr] = '\0';
                                                                /* Convert the parameter value from string.             */
                         TTCP_Port = (CPU_INT16U) Str_ToLong(TTCPParVal, 0, 10);
                         if (TTCP_Port  == 0) {                 /* If invalid parameter value, Flag error.              */
                             TTCP_StrErr = DEF_YES;
                         }
                     }
                     break;

                case 'k':                                       /* Number of connections                                */
                     TTCPParptr = 0;
                     while (TTCPCmdLine[TTCP_Strptr] == ' ') {
                         TTCP_Strptr++;
                     }
                     TTCPParptr = 0;
                     while ((TTCPCmdLine[TTCP_Strptr] != ' ') &
                            (TTCP_Strptr < TTCP_StrLen)       &
                            (!TTCP_StrErr)) {                   /* Get parameter value.                                 */
                         TTCPParVal[TTCPParptr] = TTCPCmdLine[TTCP_Strptr];
                         if (TTCPParVal[TTCPParptr] == '-') {
                             TTCP_StrErr = DEF_YES;             /* If not a parameter value, Flag error.                */
                         }
                         TTCP_Strptr++;
                         TTCPParptr++;
                     }
                     if (!TTCP_StrErr) {
                         TTCPParVal[TTCPParptr] = '\0';
                                                                /* Convert the parameter value from string.             */
                         TTCP_NumConns = (CPU_INT08U) Str_ToLong(TTCPParVal, 0, 10);
                         if ((TTCP_NumConns  <= 0) || (TTCP_NumConns > TTCP_MAX_CONNS))
                         {   /* If invalid parameter value, Flag error.              */
                             TTCP_StrErr = DEF_YES;
                         }
                     }
                     break;

                case 's':                                       /*  -s     -t: source a pattern to network.             */
                                                                /*         -r: sink (discard) all data from network.    */
                     TTCP_SinkMode = !TTCP_SinkMode;
                     break;

                case 'u':                                       /*  -u     use UDP instead of TCP.                      */
                     TTCP_Transport = 1;
                     break;

                case 'b':                                       /*  -b ##  set socket buffer size (if supported).       */
                     TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp: -b option ignored: SOCKET buffer size option not supported\r\n"));
                     break;

                case 'f':
                     TTCPParptr = 0;                            /*  -f X   format for rate: k, K = kilo {bit,byte};     */
                                                                /*                          m, M = mega;                */
                                                                /*                          g, G = giga.                */
                     while (TTCPCmdLine[TTCP_Strptr] == ' ') {
                         TTCP_Strptr++;
                     }
                     TTCPParptr = 0;
                     while ((TTCPCmdLine[TTCP_Strptr] != ' ') &
                            (TTCP_Strptr < TTCP_StrLen)       &
                            (!TTCP_StrErr)) {                   /* Get parameter value.                                 */
                         TTCPParVal[TTCPParptr] = TTCPCmdLine[TTCP_Strptr];
                         if (TTCPParVal[TTCPParptr] == '-') {
                             TTCP_StrErr = DEF_YES;             /* If not a parameter value, Flag error.                */
                         }
                         TTCP_Strptr++;
                         TTCPParptr++;
                     }
                     if (!TTCP_StrErr) {
                         TTCP_Fmt = TTCPParVal[0];              /* Retrieve the parameter.                              */
                     }
                     break;

                case 'B':                                       /* -B   For -s, only output full blocks as specified    */
                                                                /*      by -l.                                          */
                     if (TTCP_Cli_Srvr == 0) {
                        TTCP_Block = 1;
                     }
                     break;

                case 'T':                                       /* -T   Read data on reception to add processing time   */
                                                                /*      to the test.                                    */
                     TTCP_TouchData = 1;
                     TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Touch Data. Data processed as received\r\n",
                                       TTCP_Cli_Srvr?"-t":"-r"));
                     break;

                case 'L':
                     TTCP_LoopMode = 1;
                     TTCP_TRACE_INFO(((CPU_CHAR*)"LOOP mode ACTIVATED.\r\n"));
                     break;


                default:
                     TTCP_StrErr = DEF_YES;                     /* Any unknown paramter, Flag error.                    */
                     break;
            }

        } else {
            switch (TTCPPar[0]) {                               /* Only non-alphabetical parameter is the remote host   */
                                                                /* IP address.                                          */
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                     Str_Copy(TTCP_RemoteIP, TTCPPar);
                     break;

                default:
                     if (TTCPPar[0]  != '\0') {
                         TTCP_StrErr  = DEF_YES;
                     }
                     break;
            }
        }
    }

    if (TTCP_StrErr) {                                          /* On parsing error, outputs the command line menu.     */
                                                                /* Definition of the -h option of the TTCP command      */
                                                                /* line input.                                          */
        TTCP_TRACE_INFO(((CPU_CHAR*)"Usage: -t [-options] host\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"       -r [-options]\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"Common options: \r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -l ##   length of buffers read from or written to network (default %d)\r\n", TTCP_CFG_BUF_LEN));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -u      use UDP instead of TCP\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -p ##   port number to send to or listen at (default %d)\r\n", TTCP_PORT));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -s      disable sinkmode (enabled by default)\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"              sinkmode enabled:\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"                -t: source (transmit) a pattern to network\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"                -r: sink (discard) all data from network\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"              sinkmode disabled:\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"                -t: take data to be transmitted from serial port\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"                -r: write data received to the serial port\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -b ##   set socket buffer size (not supported)\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -k ##   number of simultaneous connections\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -f X    format for rate: k,K = kilo{bit,byte}; m,M = mega; g,G = giga\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -L      enable LOOP mode. Infinite loop the current operation\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"Options specific to -t:\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -n ##   number of source buffers written to network (default %d)\r\n", TTCP_NUM_BUF));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -D      don't buffer TCP writes (sets TCP_NODELAY socket option)\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"Options specific to -r:\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -B      for -s, only output full blocks as specified by -l\r\n"));
        TTCP_TRACE_INFO(((CPU_CHAR*)"    -T      \"touch\": access each byte as it is read\r\n\n"));
    }

    return (TTCP_StrErr);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             TTCP_TxTCP()
*
* Description: This function sends data via a TCP socket.
*
* Arguments  : cp       buffer containing the data to send.
*              cnt      number of characters to send from the buffer.
*
*********************************************************************************************************
*/

static  void  TTCP_TxTCP (CPU_CHAR    *cp,
                          CPU_INT16U   num)
{
    CPU_INT32U  transmit;                                       /* Number of bytes to transmit = TTCP_BufLen on first   */
                                                                /* TXData.                                              */
    CPU_INT32U  count;                                          /* Used to count the number of bytes transmistted.      */
    CPU_INT32U  data_sent;                                      /* Tells how many bytes were sent by the TCP echo.      */
                                                                /* server.  Always one in our case.                     */
    CPU_INT16U  sock_flags;                                     /* Socket flags.                                        */
    NET_ERR     err;                                            /* Socket error parameter.                              */

    CPU_INT32U  ConnIdx;                                        /* Index into the array of sockets.                     */


    sock_flags = 0;                                             /* No sock flags used.                                  */
    count      = 0;
    ConnIdx    = 0;

    while (ConnIdx < TTCP_NumConns) {
        transmit  = num - count;

        LED_On(1);                                              /* Turned off at the end.                               */

        data_sent = NetSock_TxData( TTCP_ConnSockID[ConnIdx],
                                   &cp[count],
                                    transmit,
                                    sock_flags,
                                   &err);

        LED_Off(1);                                             /* Turned on right before NetSock_TxData().             */

        TTCP_NumCalls++;

        switch (err) {                                          /* Socket error handling.                               */
            case NET_SOCK_ERR_NONE:
                 count                        += data_sent;
                 TTCP_NumBytes                += data_sent;
                 TTCP_NumIntvl[ConnIdx]       += data_sent;
                 TTCP_NumBytesStream[ConnIdx] += data_sent;

                 if (TTCP_NumIntvl[ConnIdx] > 1000000) {
                     TTCP_TRACE_INFO(((CPU_CHAR*)"Stream %d: Bytes Sent So Far: %11d\n\r",
                                      ConnIdx, TTCP_NumBytesStream[ConnIdx]));

                     TTCP_NumIntvl[ConnIdx]   -= 1000000;
                 }
                 break;

            case NET_SOCK_ERR_INVALID_FAMILY:
            case NET_SOCK_ERR_INVALID_TYPE:
            case NET_SOCK_ERR_INVALID_STATE:
            case NET_SOCK_ERR_INVALID_OP:
            case NET_SOCK_ERR_FAULT:
                 TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Closing socket %d Fatal socket error number %d.\r\n",
                                   TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ConnSockID[ConnIdx], err));
                 NetSock_Close(TTCP_ConnSockID[ConnIdx], &err);
                 TTCP_ConnSockID[ConnIdx] = -1;
                 break;

            case NET_SOCK_ERR_NOT_USED:
                 TTCP_ConnSockID[ConnIdx] = -1;
                 break;

            case NET_ERR_TX:
                 TTCP_NumTxErrs++;
                 OSTimeDlyHMSM(0, 0, 0, 20);
                 break;

            default:
                 TTCP_NumUnknownTxRet++;
                 OSTimeDlyHMSM(0, 0, 0, 20);
                 break;
        }

        if (TTCP_ConnSockID[ConnIdx] == -1) {                            /* Socket was closed. Terminate test.                  */
            TTCP_Abort = DEF_YES;
            break;
        } else {
           /* ------------------------------------------------------------- */
           /* See if all of the data has been sent for this socket.         */
           /* ------------------------------------------------------------- */
           if (count == num) {
               ConnIdx++;
               count = 0;
           }
        }
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             TTCP_TxUDP()
*
* Description: This function send data via a UDP socket.
*
* Arguments  : cp       buffer containing the data to send.
*              num      number of characters to send from the buffer.
*
*********************************************************************************************************
*/

static  void  TTCP_TxUDP (CPU_CHAR    *cp,
                          CPU_INT16U   num)
{
    CPU_INT32U  transmit;                                       /* Number of bytes to transmit = TTCP_BufLen on first   */
                                                                /* TXData.                                              */
    CPU_INT32U  count;                                          /* Used to count the number of bytes transmistted.      */
    CPU_INT32U  data_sent;                                      /* Tells how many bytes were sent by the TCP echo.      */
                                                                /* server.  Always one in our case.                     */
    NET_ERR     err;                                            /* Socket error parameter.                              */

    CPU_INT32U  ConnIdx;                                        /* Index into the array of sockets.                     */


    count      = 0;
    ConnIdx    = 0;

    while (ConnIdx < TTCP_NumConns) {
        transmit  = num - count;
                                                                /* Maximum UDP datagram because uC/TCP-IP does not      */
                                                                /*    fragment.                                         */
                                                                /*    NET_IP_MTU)  = Minimum(1500, 1596) - 60           */
                                                                /*                 = 1500 - 60                          */
                                                                /*                 = 1440                               */

                                                                /*    NET_UDP MTU  = IP MTU - UDP header size           */
                                                                /*    NET_UDP_MTU  = 1440   - 8                         */
                                                                /*                 = 1432                               */
        if (transmit > NET_UDP_MTU) {
            transmit = NET_UDP_MTU;
        }

        LED_On(1);                                              /* Turned off at the end.                               */

        data_sent = NetSock_TxDataTo( TTCP_ConnSockID[ConnIdx],          /* Send a datagram to max UDP length.                   */
                                     &cp[count],
                                      transmit,
                                      0,
                                     (NET_SOCK_ADDR *)&TTCP_RemoteAddrPort,
                                      TTCP_RemoteAddLen,
                                     &err);

        LED_Off(1);                                             /* Turned on right before NetSock_TxData().             */

        TTCP_NumCalls++;

        switch (err) {                                          /* Socket error handling.                               */
            case NET_SOCK_ERR_NONE:
                 count                        += data_sent;
                 TTCP_NumBytes                += data_sent;
                 TTCP_NumIntvl[ConnIdx]       += data_sent;
                 TTCP_NumBytesStream[ConnIdx] += data_sent;

                 if (TTCP_NumIntvl[ConnIdx] > 1000000) {
                     TTCP_TRACE_INFO(((CPU_CHAR*)"Stream %d: Bytes Sent So Far: %11d\n\r",
                                      ConnIdx, TTCP_NumBytesStream[ConnIdx]));

                     TTCP_NumIntvl[ConnIdx]   -= 1000000;
                 }
                 break;

            case NET_SOCK_ERR_INVALID_FAMILY:
            case NET_SOCK_ERR_INVALID_TYPE:
            case NET_SOCK_ERR_INVALID_STATE:
            case NET_SOCK_ERR_INVALID_OP:
            case NET_SOCK_ERR_FAULT:
                 TTCP_TRACE_INFO(((CPU_CHAR*)"ttcp%s: Closing socket %d Fatal socket error number %d. \r\n",
                                   TTCP_Cli_Srvr ? "-t" : "-r", TTCP_ConnSockID[ConnIdx], err));
                 NetSock_Close(TTCP_ConnSockID[ConnIdx], &err);
                 TTCP_ConnSockID[ConnIdx] = -1;
                 break;

            case NET_SOCK_ERR_NOT_USED:
                 TTCP_ConnSockID[ConnIdx] = -1;
                 break;

            case NET_ERR_TX:
                 TTCP_NumTxErrs++;
                 OSTimeDlyHMSM(0, 0, 0, 20);
                 break;

            default:
                 TTCP_NumUnknownTxRet++;
                 OSTimeDlyHMSM(0, 0, 0, 20);
                 break;
        }

        if (TTCP_ConnSockID[ConnIdx] == -1) {                            /* Socket was closed. Terminate test.                  */
            TTCP_Abort = DEF_YES;
            break;
        } else {
           /* ------------------------------------------------------------- */
           /* See if all of the data has been sent for this socket.         */
           /* ------------------------------------------------------------- */
           if (count == num) {
               ConnIdx++;
               count = 0;
           }
        }
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                               TTCP_RxTCP()
*
* Description: This function receives the test data via a TCP socket.
*
* Arguments  : None.
*
*********************************************************************************************************
*/

static  void  TTCP_RxTCP (void)
{
    CPU_INT32U  receive;                                        /* Number of bytes to receive.                          */
    CPU_INT32U  count;                                          /* Used to count the number of bytes transmistted.      */
    CPU_INT32U  print_count;                                    /* Used to keep the count for printed received          */
                                                                /* characters.                                          */
    CPU_INT32U  touch_count;                                    /* Used to keep the count for processed received        */
                                                                /* characters.                                          */
    CPU_INT32U  sum;                                            /* Used to simulate data processing wiht the -T option. */
    CPU_INT32S  data_received;                                  /* Tells when data has been received by the TCP echo    */
                                                                /* server.                                              */
    CPU_INT32U  total;                                          /* Number of bytes expected to receive when in Server   */
                                                                /* Mode.                                                */
    NET_ERR     err;                                            /* Socket error parameter.                              */

    CPU_INT32U  ConnIdx;                                        /* Index into socket array.                             */

    CPU_INT32U  NumReceived[TTCP_MAX_CONNS];                    /* Bytes each rcvd by each socket.                      */

    CPU_INT32U  NumBufsRcvd;                                    /* Count of the number of buffers.                      */

    CPU_INT32U  ActiveConns;                                    /* count of num active connections.                     */


    sum   = 0;
    total = TTCP_BufLen * TTCP_NumBuf;                          /* Number of total bytes to receive.                    */

    if (TTCP_BLOCK) {                                           /* Receive only full blocks.                            */
        /* ---------------------------------------------------------------- */
        /* Loop through for the number of buffers to receive.  Get the      */
        /* buffer for each of the connections.                              */
        /* ---------------------------------------------------------------- */

        for (NumBufsRcvd = 0; NumBufsRcvd < TTCP_NumBuf; NumBufsRcvd++) {
            for (ConnIdx = 0; ConnIdx < TTCP_NumConns; ConnIdx++) {
                count       = 0;
                print_count = 0;
                touch_count = 0;

                while (count < TTCP_BufLen) {
                    receive = TTCP_BufLen - count;

                    LED_On(1);                                  /* Turned off after the call.                           */

                    data_received = NetSock_RxData( TTCP_ConnSockID[ConnIdx],
                                                   &TTCP_Buf[count],
                                                    receive,
                                                    0,
                                                   &err);

                    LED_Off(1);                                 /* Turned On right before NetSock_RxData().             */

                    TTCP_NumCalls++;

                    if (data_received > 0) {
                        count += data_received;

                        TTCP_NumIntvl[ConnIdx] += data_received;

                        TTCP_NumBytesStream[ConnIdx] += data_received;

                        if (TTCP_NumIntvl[ConnIdx] > 1000000) {
                            TTCP_TRACE ((CPU_CHAR*)"Stream %d: Bytes RCVD So Far: %11d\n\r",
                                ConnIdx, TTCP_NumBytesStream[ConnIdx]);

                            TTCP_NumIntvl[ConnIdx] -= 1000000;
                        }

                                                                /* Not Sink Mode => output data to serial port.         */
                        if (TTCP_SinkMode == 0) {
                            while (print_count < count) {
                                                                /* Output each character to the serial port.            */
                                TTCP_TRACE_INFO(((CPU_CHAR*)"%c", TTCP_Buf[print_count]));
                                print_count++;
                            }
                        }

                                                                /* -T option is enabled.                                */
                        if (TTCP_TouchData != 0) {
                            while (touch_count < count) {
                                                                /* Process received data.                               */
                                sum += (CPU_INT08U) TTCP_Buf[touch_count];
                                touch_count++;
                            }
                        }

                        TTCP_NumBytes += count;
                    } else if (data_received == 0) {            /* Socket closed */
                        TTCP_TRACE((CPU_CHAR*)"Socket Closed Received\r\n");
                        return;
                    }
                }        /* While loop */
            }            /* For Loop */
        }                /* For Loop */
    } else { /* Receive whatever the stack was able to retrieve.     */
        /* ---------------------------------------------------------------- */
        /* Loop through reading the buffers for each connection until all   */
        /* of the connections have received their full allotment.           */
        /* ---------------------------------------------------------------- */

        ActiveConns = TTCP_NumConns;

        for (ConnIdx = 0; ConnIdx < TTCP_NumConns; ConnIdx++) {
           NumReceived[ConnIdx] = 0;
        }

        while (ActiveConns > 0) {
            for (ConnIdx = 0; ConnIdx < TTCP_NumConns; ConnIdx++) {
                /* -------------------------------------------------------- */
                /* See if this connection has reached its full amount.      */
                /* -------------------------------------------------------- */

                if (NumReceived[ConnIdx] < total) {

                    LED_On (1);   // Turned On

                    data_received = NetSock_RxData( TTCP_ConnSockID[ConnIdx],
                                                    TTCP_Buf,
                                                    TTCP_BufLen,
                                                    0,
                                                   &err);

                    LED_Off (1);    // Turned On right before NetSock_RxData

                    TTCP_NumCalls++;
                    if (data_received > 0) {
                        TTCP_NumBytes += data_received;

                        NumReceived[ConnIdx] += data_received;

                        /* ------------------------------------------------- */
                        /* If we this connection has reached it's total then */
                        /* reduce one from the Active Conns counter.         */
                        /* ------------------------------------------------- */

                        if (NumReceived[ConnIdx] >= total) {
                            if (ActiveConns > 0) {
                                ActiveConns--;
                            }
                        }

                        TTCP_NumIntvl[ConnIdx] += data_received;
                        TTCP_NumBytesStream[ConnIdx] += data_received;

                        if (TTCP_NumIntvl[ConnIdx] > 1000000) {
                            TTCP_TRACE ((CPU_CHAR*)"Stream %d: Bytes RCVD So Far: %11d\n\r",
                                ConnIdx, TTCP_NumBytesStream[ConnIdx]);

                            TTCP_NumIntvl[ConnIdx] -= 1000000;
                        }

                        if (TTCP_SinkMode == 0) {               /* Not Sink Mode => output data to serial port.         */
                            TTCP_Buf[TTCP_BufLen] = '\0';       /* Terminate the string.                                */
                            TTCP_TRACE_INFO(((CPU_CHAR*)"%s", TTCP_Buf));  /* Output the string to the serial port.                */
                        }

                        if (TTCP_TouchData != 0) {              /* -T option is enabled.                                */
                            touch_count     = 0;
                            while (touch_count < (CPU_INT16U) data_received) {
                                                                /* Process received data.                               */
                                sum += (CPU_INT08U) TTCP_Buf[touch_count];
                                touch_count++;
                            }
                        }
                    } else if (data_received == -1) {
                        if (err == NET_SOCK_ERR_RX_Q_EMPTY) {
                            TTCP_RcvTimeOutCnts ++;
                        } else if (err == NET_SOCK_ERR_INVALID_TYPE) {
                            TTCP_TRACE_INFO(((CPU_CHAR*)"Stream %d: Invalid Type, reducing activeConns\r\n", ConnIdx));

                            if (ActiveConns > 0) {
                                ActiveConns--;
                            }
                        } else {
                            TTCP_NumRcvErrs ++;
                        }
                    } else if (data_received == 0) {
                        TTCP_TRACE((CPU_CHAR*)"Stream %d: Close socket detected\n\r",
                                    ConnIdx);
                        return;
                    }
                }
            }
        }
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             TTCP_RxUDP()
*
* Description: This function receives the test data via a UDP socket.
*
* Arguments  : None.
*
*********************************************************************************************************
*/

static  void  TTCP_RxUDP (void)
{
    CPU_INT32S   data_received;                                 /* Tells when data has been received by the TCP echo    */
                                                                /* server.                                              */
    CPU_BOOLEAN  udp_start;                                     /* Monitors the first 4 bytes datagram to start the     */
                                                                /* statistics.                                          */
    CPU_INT32U   receive;                                       /* Number of bytes to receive.                          */
    CPU_INT32U   touch_count;                                   /* Used to keep the count for processed received        */
                                                                /* characters.                                          */
    CPU_INT32U   sum;                                           /* Used to simulate data processing wiht the -T option. */
    CPU_INT32U   total;                                         /* Number of bytes expected to receive when in Server   */
                                                                /* Mode.                                                */
    NET_ERR      err;                                           /* Socket error parameter.                              */


    total     = TTCP_BufLen * TTCP_NumBuf;                      /* Number of total bytes to receive.                    */
    udp_start = DEF_NO;

                                                                /* Maximum UDP datagram because uC/TCP-IP does not      */
                                                                /*    fragment.                                         */
                                                                /*    NET_IP_MTU   = Minimum(1500, 1596) - 60           */
                                                                /*                 = 1500 - 60                          */
                                                                /*                 = 1440                               */

                                                                /*    NET_UDP MTU  = IP MTU - UDP header size           */
                                                                /*    NET_UDP_MTU  = 1440 - 8                           */
                                                                /*                 = 1432                               */
    receive = NET_UDP_MTU;
    sum     = 0;

    while (TTCP_NumBytes < total) {
                                                                /* Wait for message from client (blocking).             */
        data_received = NetSock_RxDataFrom( TTCP_ConnSockID[0],
                                            TTCP_Buf,
                                            receive,
                                            0,
                                           (NET_SOCK_ADDR *)&TTCP_RemoteAddrPort,
                                           &TTCP_RemoteAddLen,
                                            0,
                                            0,
                                            0,
                                            &err);

        if (data_received > 0) {
            if (data_received <= 4) {                               /* A second UDP datagram of 4 bytes or less, signals    */
                                                                    /* the end of the test.                                 */
                if (udp_start) {
                    break;
                } else {
                    udp_start = DEF_YES;                            /* The first UDP datagram of 4 bytes or less, signals   */
                                                                    /* the start of transmission.                           */
                    TTCP_PerfTimerStart = OSTimeGet();
                }

            } else {
                TTCP_NumBytes += data_received;
                TTCP_NumCalls++;
                if (TTCP_SinkMode == 0) {                           /* Not Sink Mode => output data to serial port.         */
                    TTCP_Buf[data_received] = '\0';                 /* Terminate the string.                                */
                    TTCP_TRACE_INFO(((CPU_CHAR*)"%s", TTCP_Buf));              /* Output the string to the serial port.                */
                }

                if (TTCP_TouchData != 0) {                          /* -T option is enabled.                                */
                    touch_count     = 0;
                    while (touch_count < (CPU_INT16U) data_received) {
                        sum += (CPU_INT08U) TTCP_Buf[touch_count];  /* Process received data.                               */
                        touch_count++;
                    }
                }
            }
        } else {
            break;                                                  /* Terminate test on error from NetSock_RxDataFrom      */
        }
    }
}
