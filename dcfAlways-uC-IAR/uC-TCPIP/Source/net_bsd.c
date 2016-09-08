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
* Filename      : net_bsd.c
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
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    NET_BSD_MODULE
#include  <net.h>


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) See 'net_bsd.h  MODULE'.
*********************************************************************************************************
*/

#ifdef  NET_BSD_MODULE_PRESENT


/*$PAGE*/
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                     STANDARD BSD 4.x FUNCTIONS
*
* Note(s) : (1) BSD 4.x function definitions are required only for applications that call BSD 4.x functions.
*
*               See 'net_bsd.h  MODULE  Note #1b3'
*                 & 'net_bsd.h  STANDARD BSD 4.x FUNCTION PROTOTYPES  Note #1'.
*********************************************************************************************************
*/

#if (NET_BSD_CFG_API_EN == DEF_ENABLED)                         /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                              socket()
*
* Description : Create a socket.
*
* Argument(s) : protocol_family     Socket protocol family (see 'net_sock.c  Note #1a').
*
*               sock_type           Socket type            (see 'net_sock.c  Note #1b').
*
*               protocol            Socket protocol        (see 'net_sock.c  Note #1c').
*
* Return(s)   : Socket descriptor/handle identifier, if NO errors.
*
*               -1,                                  otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  socket (int  protocol_family,
             int  sock_type,
             int  protocol)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_Open((CPU_INT16S) protocol_family,
                                 (CPU_INT16S) sock_type,
                                 (CPU_INT16S) protocol,
                                 (NET_ERR  *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               close()
*
* Description : Close a socket.
*
* Argument(s) : sock_id     Socket descriptor/handle identifier of socket to close.
*
* Return(s)   :  0, if NO errors.
*
*               -1, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) Once an application closes its socket, NO further operations on the socket are allowed
*                   & the application MUST NOT continue to access the socket.
*                   
*                   #### Continued access to the closed socket by the application layer will likely corrupt
*                   the network socket layer.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  close (int  sock_id)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_Close((NET_SOCK_ID) sock_id,
                                  (NET_ERR   *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               bind()
*
* Description : Bind a socket to a local address.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to bind to a local address.
*
*               paddr_local     Pointer to socket address structure (see Note #1).
*
*               addr_len        Length  of socket address structure (in octets).
*
* Return(s)   :  0, if NO errors.
*
*               -1, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) Socket address structure 'Family' member MUST be configured in host-order & MUST
*                       NOT be converted to/from network-order.
*
*                   (b) Socket address structure addresses MUST be configured/converted from host-order
*                       to network-order.
*
*                   See also 'net_sock.h  NETWORK SOCKET ADDRESS DATA TYPES  Note #2'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  bind (        int        sock_id,
           struct  sockaddr  *paddr_local,
                   int        addr_len)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_Bind((NET_SOCK_ID      ) sock_id,
                                 (NET_SOCK_ADDR   *) paddr_local,
                                 (NET_SOCK_ADDR_LEN) addr_len,
                                 (NET_ERR         *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                              connect()
*
* Description : Connect a socket to a remote server.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to connect.
*
*               paddr_remote    Pointer to socket address structure (see Note #1).
*
*               addr_len        Length  of socket address structure (in octets).
*
* Return(s)   :  0, if NO errors.
*
*               -1, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) Socket address structure 'Family' member MUST be configured in host-order & MUST
*                       NOT be converted to/from network-order.
*
*                   (b) Socket address structure addresses MUST be configured/converted from host-order
*                       to network-order.
*
*                   See also 'net_sock.h  NETWORK SOCKET ADDRESS DATA TYPES  Note #2'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  connect (        int        sock_id,
              struct  sockaddr  *paddr_remote,
                      int        addr_len)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_Conn((NET_SOCK_ID      ) sock_id,
                                 (NET_SOCK_ADDR   *) paddr_remote,
                                 (NET_SOCK_ADDR_LEN) addr_len,
                                 (NET_ERR         *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                              listen()
*
* Description : Set socket to listen for connection requests.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to listen.
*
*               sock_q_size     Number of connection requests to queue on listen socket.
*
* Return(s)   :  0, if NO errors.
*
*               -1, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
#if    (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
int  listen (int  sock_id,
             int  sock_q_size)
{
    NET_ERR  err;
    int      rtn_code;


    rtn_code = (int)NetSock_Listen((NET_SOCK_ID    ) sock_id,
                                   (NET_SOCK_Q_SIZE) sock_q_size,
                                   (NET_ERR       *)&err);

    return (rtn_code);
}
#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                              accept()
*
* Description : Get a new socket accepted from a socket set to listen for connection requests.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of listen socket.
*
*               paddr_remote    Pointer to an address buffer that will receive the socket address structure
*                                   of the accepted socket's remote address (see Note #1), if NO errors.
*
*               paddr_len       Pointer to a variable to ... :
*
*                                   (a) Pass the size of the address buffer pointed to by 'paddr_remote'.
*                                   (b) (1) Return the actual size of socket address structure with the 
*                                               accepted socket's remote address, if NO errors;
*                                       (2) Return 0,                             otherwise.
*
* Return(s)   : Socket descriptor/handle identifier of new accepted socket, if NO errors.
*
*               -1,                                                         otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) Socket address structure 'Family' member returned in host-order & SHOULD NOT be
*                       converted to network-order.
*
*                   (b) Socket address structure addresses MUST be converted from host-order to network-
*                       order.
*
*                   See also 'net_sock.h  NETWORK SOCKET ADDRESS DATA TYPES  Note #2'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
#if    (NET_SOCK_CFG_TYPE_STREAM_EN == DEF_ENABLED)
int  accept (        int        sock_id,
             struct  sockaddr  *paddr_remote,
                     int       *paddr_len)
{
    int                rtn_code;
    NET_SOCK_ADDR_LEN  addr_len;
    NET_ERR            err;


    addr_len  = (NET_SOCK_ADDR_LEN)*paddr_len;
    rtn_code  = (int)NetSock_Accept((NET_SOCK_ID        ) sock_id,
                                    (NET_SOCK_ADDR     *) paddr_remote,
                                    (NET_SOCK_ADDR_LEN *)&addr_len,
                                    (NET_ERR           *)&err);

   *paddr_len = (int)addr_len;

    return (rtn_code);
}
#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             recvfrom()
*
* Description : Receive data from a socket.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to receive data.
*
*               pdata_buf       Pointer to an application data buffer that will receive the socket's received
*                                   data.
*
*               data_buf_len    Size of the   application data buffer (in octets) [see Note #1].
*
*               flags           Flags to select receive options (see Note #2); bit-field flags logically OR'd :
*
*                                   0                           No socket flags selected.
*                                   MSG_PEEK                    Receive socket data without consuming
*                                                                   the socket data.
*                                   MSG_DONTWAIT                Receive socket data without blocking.
*
*               paddr_remote    Pointer to an address buffer that will receive the socket address structure 
*                                   with the received data's remote address (see Note #3), if NO errors.
*
*               paddr_len       Pointer to a variable to ... :
*
*                                   (a) Pass the size of the address buffer pointed to by 'paddr_remote'.
*                                   (b) (1) Return the actual size of socket address structure with the 
*                                               received data's remote address, if NO errors;
*                                       (2) Return 0,                           otherwise.
*
* Return(s)   : Number of positive data octets received, if NO errors                (see Note #4a).
*
*                0,                                      if socket connection closed (see Note #4b).
*
*               -1,                                      otherwise                   (see Note #4c1).
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) (1) (A) Datagram-type sockets send & receive all data atomically -- i.e. every single, 
*                               complete datagram transmitted MUST be received as a single, complete datagram.
*
*                           (B) IEEE Std 1003.1, 2004 Edition, Section 'recvfrom() : DESCRIPTION' summarizes 
*                               that "for message-based sockets, such as ... SOCK_DGRAM ... the entire message 
*                               shall be read in a single operation.  If a message is too long to fit in the 
*                               supplied buffer, and MSG_PEEK is not set in the flags argument, the excess 
*                               bytes shall be discarded".
*
*                       (2) Thus, if the socket's type is datagram & the receive data buffer size is NOT
*                           large enough for the received data, the receive data buffer is maximally filled
*                           with receive data but the remaining data octets are silently discarded & NO 
*                           error is returned.
*
*                           (A) IEEE Std 1003.1, 2004 Edition, Section 'send() : ERRORS' states to return an
*                               'EMSGSIZE' error when "the message is too large to be sent all at once".
*
*                               ???? Similarly, a socket receive whose receive data buffer size is NOT large
*                               enough for the received data could return an 'EMSGSIZE' error.
*
*                   (b) (1) (A) (1) Stream-type sockets send & receive all data octets in one or more non-
*                                   distinct packets.  In other words, the application data is NOT bounded
*                                   by any specific packet(s); rather, it is contiguous & sequenced from 
*                                   one packet to the next.
*
*                               (2) IEEE Std 1003.1, 2004 Edition, Section 'recv() : DESCRIPTION' summarizes
*                                   that "for stream-based sockets, such as SOCK_STREAM, message boundaries 
*                                   shall be ignored.  In this case, data shall be returned to the user as
*                                   soon as it becomes available, and no data shall be discarded".
*
*                           (B) Thus, if the socket's type is stream & the receive data buffer size is NOT 
*                               large enough for the received data, the receive data buffer is maximally 
*                               filled with receive data & the remaining data octets remain queued for 
*                               later application-socket receives.
*                   
*                       (2) Thus, it is typical -- but NOT absolutely required -- that a single application
*                           task ONLY receive or request to receive data from a stream-type socket.
*
*                   See also 'net_sock.c  NetSock_RxDataHandler()  Note #2'.
*$PAGE*
*               (2) Only some socket receive flag options are implemented.  If other flag options are requested,
*                   socket receive handler function(s) abort & return appropriate error codes so that requested
*                   flag options are NOT silently ignored.
*
*               (3) (a) Socket address structure 'Family' member returned in host-order & SHOULD NOT be
*                       converted to network-order.
*
*                   (b) Socket address structure addresses MUST be converted from host-order to network-
*                       order.
*
*                   See also 'net_sock.h  NETWORK SOCKET ADDRESS DATA TYPES  Note #2'.
*
*               (4) IEEE Std 1003.1, 2004 Edition, Section 'recvfrom() : RETURN VALUE' states that :
*
*                   (a) "Upon successful completion, recvfrom() shall return the length of the message in
*                        bytes."
*
*                   (b) "If no messages are available to be received and the peer has performed an orderly
*                        shutdown, recvfrom() shall return 0."
*
*                   (c) (1) "Otherwise, [-1 shall be returned]" ...
*                       (2) "and 'errno' set to indicate the error."
*
*                   See also 'net_sock.c  NetSock_RxDataHandler()  Note #7'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  recvfrom (        int        sock_id,
                       void      *pdata_buf,
                       int        data_buf_len,
                       int        flags,
               struct  sockaddr  *paddr_remote,
                       int       *paddr_len)
{
    int                rtn_code;
    NET_SOCK_ADDR_LEN  addr_len;
    NET_ERR            err;


    addr_len  = (NET_SOCK_ADDR_LEN)*paddr_len;
    rtn_code  = (int)NetSock_RxDataFrom((NET_SOCK_ID        ) sock_id,
                                        (void              *) pdata_buf,
                                        (CPU_INT16S         ) data_buf_len,
                                        (CPU_INT16S         ) flags,
                                        (NET_SOCK_ADDR     *) paddr_remote,
                                        (NET_SOCK_ADDR_LEN *)&addr_len,
                                        (void              *) 0,
                                        (CPU_INT08U         ) 0,
                                        (CPU_INT08U        *) 0,
                                        (NET_ERR           *)&err);

   *paddr_len = (int)addr_len;

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               recv()
*
* Description : Receive data from a socket.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to receive data.
*
*               pdata_buf       Pointer to an application data buffer that will receive the socket's received
*                                   data.
*
*               data_buf_len    Size of the   application data buffer (in octets) [see Note #1].
*
*               flags           Flags to select receive options (see Note #2); bit-field flags logically OR'd :
*
*                                   0                           No socket flags selected.
*                                   MSG_PEEK                    Receive socket data without consuming
*                                                                   the socket data.
*                                   MSG_DONTWAIT                Receive socket data without blocking.
*
* Return(s)   : Number of positive data octets received, if NO errors                (see Note #3a).
*
*                0,                                      if socket connection closed (see Note #3b).
*
*               -1,                                      otherwise                   (see Note #3c1).
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) (1) (A) Datagram-type sockets send & receive all data atomically -- i.e. every single, 
*                               complete datagram transmitted MUST be received as a single, complete datagram.
*
*                           (B) IEEE Std 1003.1, 2004 Edition, Section 'recv() : DESCRIPTION' summarizes that
*                               "for message-based sockets, such as SOCK_DGRAM ... the entire message shall be
*                               read in a single operation.  If a message is too long to fit in the supplied
*                               buffer, and MSG_PEEK is not set in the flags argument, the excess bytes shall
*                               be discarded".
*
*                       (2) Thus, if the socket's type is datagram & the receive data buffer size is NOT 
*                           large enough for the received data, the receive data buffer is maximally filled
*                           with receive data but the remaining data octets are silently discarded & NO 
*                           error is returned.
*
*                           (A) IEEE Std 1003.1, 2004 Edition, Section 'send() : ERRORS' states to return an
*                               'EMSGSIZE' error when "the message is too large to be sent all at once".
*
*                               ???? Similarly, a socket receive whose receive data buffer size is NOT large
*                               enough for the received data could return an 'EMSGSIZE' error.
*
*                   (b) (1) (A) (1) Stream-type sockets send & receive all data octets in one or more non-
*                                   distinct packets.  In other words, the application data is NOT bounded
*                                   by any specific packet(s); rather, it is contiguous & sequenced from 
*                                   one packet to the next.
*
*                               (2) IEEE Std 1003.1, 2004 Edition, Section 'recv() : DESCRIPTION' summarizes
*                                   that "for stream-based sockets, such as SOCK_STREAM, message boundaries 
*                                   shall be ignored.  In this case, data shall be returned to the user as
*                                   soon as it becomes available, and no data shall be discarded".
*
*                           (B) Thus, if the socket's type is stream & the receive data buffer size is NOT 
*                               large enough for the received data, the receive data buffer is maximally 
*                               filled with receive data & the remaining data octets remain queued for 
*                               later application-socket receives.
*                   
*                       (2) Thus, it is typical -- but NOT absolutely required -- that a single application
*                           task ONLY receive or request to receive data from a stream-type socket.
*
*                   See also 'net_sock.c  NetSock_RxDataHandler()  Note #2'.
*$PAGE*
*               (2) Only some socket receive flag options are implemented.  If other flag options are requested,
*                   socket receive handler function(s) abort & return appropriate error codes so that requested
*                   flag options are NOT silently ignored.
*
*               (3) IEEE Std 1003.1, 2004 Edition, Section 'recv() : RETURN VALUE' states that :
*
*                   (a) "Upon successful completion, recv() shall return the length of the message in bytes."
*
*                   (b) "If no messages are available to be received and the peer has performed an orderly
*                        shutdown, recv() shall return 0."
*
*                   (c) (1) "Otherwise, -1 shall be returned" ...
*                       (2) "and 'errno' set to indicate the error."
*
*                   See also 'net_sock.c  NetSock_RxDataHandler()  Note #7'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  recv (int    sock_id,
           void  *pdata_buf,
           int    data_buf_len,
           int    flags)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_RxData((NET_SOCK_ID  ) sock_id,
                                   (void        *) pdata_buf,
                                   (CPU_INT16S   ) data_buf_len,
                                   (CPU_INT16S   ) flags,
                                   (NET_ERR     *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                              sendto()
*
* Description : Send data through a socket.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to send data.
*
*               p_data          Pointer to application data to send.
*
*               data_len        Length of  application data to send (in octets) [see Note #1].
*
*               flags           Flags to select send options (see Note #2); bit-field flags logically OR'd :
*
*                                   0                           No socket flags selected.
*                                   MSG_DONTWAIT                Send socket data without blocking.
*
*               paddr_remote    Pointer to destination address buffer (see Note #3).
*
*               addr_len        Length of  destination address buffer (in octets).
*
* Return(s)   : Number of positive data octets sent, if NO errors                (see Note #4a1).
*
*                0,                                  if socket connection closed (see Note #4b).
*
*               -1,                                  otherwise                   (see Note #4a2A).
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) (1) (A) Datagram-type sockets send & receive all data atomically -- i.e. every single, 
*                               complete datagram  sent      MUST be received    as a single, complete datagram.
*                               Thus, each call to send data MUST be transmitted in a single, complete datagram.
*
*                           (B) (1) IEEE Std 1003.1, 2004 Edition, Section 'send() : DESCRIPTION' states that
*                                   "if the message is too long to pass through the underlying protocol, send() 
*                                   shall fail and no data shall be transmitted".
*
*                               (2) Since IP transmit fragmentation is NOT currently supported (see 'net_ip.h 
*                                   Note #1e'), if the socket's type is datagram & the requested send data
*                                   length is greater than the socket/transport layer MTU, then NO data is 
*                                   sent & NET_SOCK_ERR_INVALID_DATA_SIZE error is returned.
*
*                       (2) (A) (1) Stream-type sockets send & receive all data octets in one or more non-
*                                   distinct packets.  In other words, the application data is NOT bounded 
*                                   by any specific packet(s); rather, it is contiguous & sequenced from 
*                                   one packet to the next.
*
*                               (2) Thus, if the socket's type is stream & the socket's send data queue(s) are
*                                   NOT large enough for the send data, the send data queue(s) are maximally
*                                   filled with send data & the remaining data octets are discarded but may be
*                                   re-sent by later application-socket sends.
*
*                               (3) Therefore, NO stream-type socket send data length should be "too long to
*                                   pass through the underlying protocol" & cause the socket send to "fail ... 
*                                   [with] no data ... transmitted" (see Note #1a1B1).
*
*                           (B) Thus, it is typical -- but NOT absolutely required -- that a single application
*                               task ONLY send or request to send data to a stream-type socket.
*
*                   (b) 'data_len' of 0 octets NOT allowed.
*
*                   See also 'net_sock.c  NetSock_TxDataHandler()  Note #2'.
*$PAGE*
*               (2) Only some socket send flag options are implemented.  If other flag options are requested,
*                   socket send handler function(s) abort & return appropriate error codes so that requested
*                   flag options are NOT silently ignored.
*
*               (3) (a) Socket address structure 'Family' member MUST be configured in host-order & MUST
*                       NOT be converted to/from network-order.
*
*                   (b) Socket address structure addresses MUST be configured/converted from host-order
*                       to network-order.
*
*                   See also 'net_sock.h  NETWORK SOCKET ADDRESS DATA TYPES  Note #2'.
*
*               (4) (a) IEEE Std 1003.1, 2004 Edition, Section 'sendto() : RETURN VALUE' states that :
*
*                       (1) "Upon successful completion, sendto() shall return the number of bytes sent."
*
*                           (A) Section 'sendto() : DESCRIPTION' elaborates that "successful completion 
*                               of a call to sendto() does not guarantee delivery of the message".
*
*                           (B) Thus, applications SHOULD verify the actual returned number of data 
*                               octets transmitted &/or prepared for transmission.
*
*                       (2) (A) "Otherwise, -1 shall be returned" ...
*                               (1) Section 'sendto() : DESCRIPTION' elaborates that "a return value of 
*                                   -1 indicates only locally-detected errors".
*
*                           (B) "and 'errno' set to indicate the error."
*
*                   (b) ???? Although NO socket send() specification states to return '0' when the socket's
*                       connection is closed, it seems reasonable to return '0' since it is possible for the
*                       socket connection to be close()'d or shutdown() by the remote host.
*
*                   See also 'net_sock.c  NetSock_TxDataHandler()  Note #5'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  sendto (        int        sock_id,
                     void      *p_data,
                     int        data_len,
                     int        flags,
             struct  sockaddr  *paddr_remote,
                     int        addr_len)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_TxDataTo((NET_SOCK_ID      ) sock_id,
                                     (void            *) p_data,
                                     (CPU_INT16S       ) data_len,
                                     (CPU_INT16S       ) flags,
                                     (NET_SOCK_ADDR   *) paddr_remote,
                                     (NET_SOCK_ADDR_LEN) addr_len,
                                     (NET_ERR         *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               send()
*
* Description : Send data through a socket.
*
* Argument(s) : sock_id         Socket descriptor/handle identifier of socket to send data.
*
*               p_data          Pointer to application data to send.
*
*               data_len        Length of  application data to send (in octets) [see Note #1].
*
*               flags           Flags to select send options (see Note #2); bit-field flags logically OR'd :
*
*                                   0                           No socket flags selected.
*                                   MSG_DONTWAIT                Send socket data without blocking.
*
* Return(s)   : Number of positive data octets sent, if NO errors                (see Note #3a1).
*
*                0,                                  if socket connection closed (see Note #3b).
*
*               -1,                                  otherwise                   (see Note #3a2A).
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) (a) (1) (A) Datagram-type sockets send & receive all data atomically -- i.e. every single, 
*                               complete datagram  sent      MUST be received    as a single, complete datagram.
*                               Thus, each call to send data MUST be transmitted in a single, complete datagram.
*
*                           (B) (1) IEEE Std 1003.1, 2004 Edition, Section 'send() : DESCRIPTION' states that
*                                   "if the message is too long to pass through the underlying protocol, send() 
*                                   shall fail and no data shall be transmitted".
*
*                               (2) Since IP transmit fragmentation is NOT currently supported (see 'net_ip.h 
*                                   Note #1e'), if the socket's type is datagram & the requested send data
*                                   length is greater than the socket/transport layer MTU, then NO data is 
*                                   sent & NET_SOCK_ERR_INVALID_DATA_SIZE error is returned.
*
*                       (2) (A) (1) Stream-type sockets send & receive all data octets in one or more non-
*                                   distinct packets.  In other words, the application data is NOT bounded 
*                                   by any specific packet(s); rather, it is contiguous & sequenced from 
*                                   one packet to the next.
*
*                               (2) Thus, if the socket's type is stream & the socket's send data queue(s) are
*                                   NOT large enough for the send data, the send data queue(s) are maximally
*                                   filled with send data & the remaining data octets are discarded but may be
*                                   re-sent by later application-socket sends.
*
*                               (3) Therefore, NO stream-type socket send data length should be "too long to
*                                   pass through the underlying protocol" & cause the socket send to "fail ... 
*                                   [with] no data ... transmitted" (see Note #1a1B1).
*
*                           (B) Thus, it is typical -- but NOT absolutely required -- that a single application
*                               task ONLY send or request to send data to a stream-type socket.
*
*                   (b) 'data_len' of 0 octets NOT allowed.
*
*                   See also 'net_sock.c  NetSock_TxDataHandler()  Note #2'.
*$PAGE*
*               (2) Only some socket send flag options are implemented.  If other flag options are requested,
*                   socket send handler function(s) abort & return appropriate error codes so that requested
*                   flag options are NOT silently ignored.
*
*               (3) (a) IEEE Std 1003.1, 2004 Edition, Section 'send() : RETURN VALUE' states that :
*
*                       (1) "Upon successful completion, send() shall return the number of bytes sent."
*
*                           (A) Section 'send() : DESCRIPTION' elaborates that "successful completion 
*                               of a call to sendto() does not guarantee delivery of the message".
*
*                           (B) Thus, applications SHOULD verify the actual returned number of data 
*                               octets transmitted &/or prepared for transmission.
*
*                       (2) (A) "Otherwise, -1 shall be returned" ...
*                               (1) Section 'send() : DESCRIPTION' elaborates that "a return value of 
*                                   -1 indicates only locally-detected errors".
*
*                           (B) "and 'errno' set to indicate the error."
*
*                   (b) ???? Although NO socket send() specification states to return '0' when the socket's
*                       connection is closed, it seems reasonable to return '0' since it is possible for the
*                       socket connection to be close()'d or shutdown() by the remote host.
*
*                   See also 'net_sock.c  NetSock_TxDataHandler()  Note #5'.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
int  send (int    sock_id,
           void  *p_data,
           int    data_len,
           int    flags)
{
    int      rtn_code;
    NET_ERR  err;


    rtn_code = (int)NetSock_TxData((NET_SOCK_ID  ) sock_id,
                                   (void        *) p_data,
                                   (CPU_INT16S   ) data_len,
                                   (CPU_INT16S   ) flags,
                                   (NET_ERR     *)&err);

    return (rtn_code);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             inet_addr()
*
* Description : Convert an IPv4 address in ASCII dotted-decimal notation to a network protocol IPv4 address
*                   in network-order.
*
* Argument(s) : paddr       Pointer to an ASCII string that contains a dotted-decimal IPv4 address (see Note #2).
*
* Return(s)   : Network-order IPv4 address represented by ASCII string, if NO errors.
*
*               -1,                                                     otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) RFC #1983 states that "dotted decimal notation ... refers [to] IP addresses of the
*                   form A.B.C.D; where each letter represents, in decimal, one byte of a four byte IP
*                   address".
*
*                   In other words, the dotted-decimal notation separates four decimal octet values by
*                   the dot, or period, character ('.').  Each decimal value represents one octet of 
*                   the IP address starting with the most significant octet in network order.
*
*                       IP Address Examples :
*
*                              DOTTED DECIMAL NOTATION     HEXADECIMAL EQUIVALENT
*
*                                   127.0.0.1           =       0x7F000001
*                                   192.168.1.64        =       0xC0A80140
*                                   255.255.255.0       =       0xFFFFFF00
*                                   ---         -                 --    --
*                                    ^          ^                 ^      ^
*                                    |          |                 |      |
*                                   MSO        LSO               MSO    LSO
*
*                           where
*
*                               MSO             Most  Significant Octet in Dotted Decimal IP Address
*                               LSO             Least Significant Octet in Dotted Decimal IP Address
*
*               (2) The dotted-decimal ASCII string MUST :
*
*                   (a) Include ONLY decimal values & the dot, or period, character ('.') ; ALL other
*                       characters trapped as invalid, including any leading or trailing characters.
*
*                   (b) (1) Include EXACTLY four decimal values ...
*                       (2) ... separated ...
*                       (3) ... by EXACTLY three dot characters.
*
*                   (c) Ensure that each decimal value does NOT exceed the maximum octet value (i.e. 255).
*
*                   (d) Ensure that each decimal value does NOT include leading zeros.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
#if    (NET_SOCK_CFG_FAMILY == NET_SOCK_FAMILY_IP_V4)
in_addr_t  inet_addr (char  *paddr)
{
    in_addr_t  addr;
    NET_ERR    err;


    addr = (in_addr_t)NetASCII_Str_to_IP((CPU_CHAR *) paddr,
                                         (NET_ERR  *)&err);
    if (err !=  NET_ASCII_ERR_NONE) {
        addr = (in_addr_t)NET_BSD_ERR_DFLT;
    }
    addr =  NET_UTIL_HOST_TO_NET_32(addr);

    return (addr);
}
#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             inet_ntoa()
*
* Description : Convert a network protocol IPv4 address into a dotted-decimal notation ASCII string.
*
* Argument(s) : addr        IPv4 address.
*
* Return(s)   : Pointer to ASCII string of converted IPv4 address (see Note #2), if NO errors.
*
*               Pointer to NULL,                                                 otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) RFC #1983 states that "dotted decimal notation ... refers [to] IP addresses of the
*                   form A.B.C.D; where each letter represents, in decimal, one byte of a four byte IP
*                   address".
*
*                   In other words, the dotted-decimal notation separates four decimal octet values by
*                   the dot, or period, character ('.').  Each decimal value represents one octet of 
*                   the IP address starting with the most significant octet in network order.
*
*                       IP Address Examples :
*
*                              DOTTED DECIMAL NOTATION     HEXADECIMAL EQUIVALENT
*
*                                   127.0.0.1           =       0x7F000001
*                                   192.168.1.64        =       0xC0A80140
*                                   255.255.255.0       =       0xFFFFFF00
*                                   ---         -                 --    --
*                                    ^          ^                 ^      ^
*                                    |          |                 |      |
*                                   MSO        LSO               MSO    LSO
*
*                           where
*
*                               MSO             Most  Significant Octet in Dotted Decimal IP Address
*                               LSO             Least Significant Octet in Dotted Decimal IP Address
*
*               (2) IEEE Std 1003.1, 2004 Edition, Section 'inet_ntoa() : DESCRIPTION' states that
*                   "inet_ntoa() ... need not be reentrant ... [and] is not required to be thread-safe".
*
*                   Since the character string is returned in a single, global character string array,
*                   this conversion function is NOT re-entrant.
*********************************************************************************************************
*/

#ifdef  NET_SOCK_MODULE_PRESENT
#if    (NET_SOCK_CFG_FAMILY == NET_SOCK_FAMILY_IP_V4)
char  *inet_ntoa (struct  in_addr  addr)
{
    in_addr_t  addr_ip;
    NET_ERR    err;


    addr_ip = addr.s_addr;
    addr_ip = NET_UTIL_NET_TO_HOST_32(addr_ip);

    NetASCII_IP_to_Str((NET_IP_ADDR) addr_ip,
                       (CPU_CHAR  *)&NetBSD_IP_to_Str_Array[0],
                       (CPU_BOOLEAN) DEF_NO,
                       (NET_ERR   *)&err);
    if (err != NET_ASCII_ERR_NONE) {
        return ((char *)0);
    }

    return ((char *)&NetBSD_IP_to_Str_Array[0]);
}
#endif
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                    STANDARD BSD 4.x FUNCTIONS END
*
* Note(s) : (1) See 'STANDARD BSD 4.x FUNCTIONS  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of BSD fncts (see Note #1).                      */


/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'MODULE  Note #1' & 'net_bsd.h  MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of BSD module include (see Note #1).             */

