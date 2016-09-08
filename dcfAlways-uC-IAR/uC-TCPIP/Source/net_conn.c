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
*                                    NETWORK CONNECTION MANAGEMENT
*
* Filename      : net_conn.c
* Version       : V1.90
* Programmer(s) : ITJ
*********************************************************************************************************
* Note(s)       : (1) Supports network connections for local & remote addresses of the following :
*
*                     (a) Families :
*                         (1) IPv4 Connections
*                             (A) BSD 4.x Sockets
*
*                     (b) Connection types :
*                         (1) Datagram
*                         (2) Stream
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    NET_CONN_MODULE
#include  <net.h>


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) See 'net_conn.h  MODULE'.
*********************************************************************************************************
*/

#ifdef  NET_CONN_MODULE_PRESENT


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


/*$PAGE*/
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void       NetConn_Close                 (NET_CONN             *pconn);

static  void       NetConn_CloseApp              (NET_CONN             *pconn);

static  void       NetConn_CloseTransport        (NET_CONN             *pconn);


static  void       NetConn_CloseAllConnListConns (NET_CONN            **pconn_list);



static  void       NetConn_FreeHandler           (NET_CONN             *pconn);


static  void       NetConn_Clr                   (NET_CONN             *pconn);

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
static  void       NetConn_Discard               (NET_CONN             *pconn);
#endif



static  NET_CONN  *NetConn_ListSrch              (NET_CONN            **pconn_list,
                                                  NET_CONN_LIST_TYPE    conn_list_type,
                                                  CPU_BOOLEAN           sec_srch_addr_reqd,
                                                  CPU_INT08U           *paddr_local,
                                                  CPU_INT08U           *paddr_remote);


static  void       NetConn_ChainInsert           (NET_CONN            **pconn_list,
                                                  NET_CONN             *pconn_chain);

static  void       NetConn_ChainUnlink           (NET_CONN            **pconn_list,
                                                  NET_CONN             *pconn_chain);


static  void       NetConn_Add                   (NET_CONN            **pconn_list,
                                                  NET_CONN             *pconn_chain,
                                                  NET_CONN             *pconn,
                                                  NET_CONN_LIST_TYPE    conn_list_type);

static  void       NetConn_Unlink                (NET_CONN             *pconn);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                           NetConn_Init()
*
* Description : (1) Initialize Network Connection Management Module :
*
*                   (a) Initialize network connection pool
*                   (b) Initialize network connection table
*                   (c) Initialize network connection lists
*                   (d) Initialize network connection wildcard address(s)
*                   (e) Initialize network connection statistics & error counters
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Net_Init().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (2) Network connection pool MUST be initialized PRIOR to initializing the pool with 
*                   pointers to network connections.
*
*               (3) Network connection  addresses maintained in network-order.  Therefore, network 
*                   connection wildcard address(es) MUST be configured in network-order.
*
*               (4) Ideally, network connection address configuration comparison(s) would be calculated 
*                   at compile-time.  However, some compilers do NOT allow pre-processor directives to 
*                   include run-time macro's -- e.g. 'sizeof()'.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_Init (void)
{
    NET_CONN            *pconn;
    NET_CONN           **pconn_list;
    NET_CONN_QTY         i;
    NET_CONN_LIST_QTY    j;
    CPU_INT16U           k;
    CPU_INT16U           ix;
    CPU_INT16U           val_len;
    CPU_INT08U           val;
    NET_ERR              stat_err;


                                                                /* ------------- INIT NET CONN POOL/STATS ------------- */
    NetConn_PoolPtr = (NET_CONN *)0;                            /* Init-clr net conn pool (see Note #2).                */

    NetStat_PoolInit((NET_STAT_POOL   *)&NetConn_PoolStat,
                     (NET_STAT_POOL_QTY) NET_CONN_CFG_NBR_CONN,
                     (NET_ERR         *)&stat_err);


                                                                /* ---------------- INIT NET CONN TBL ----------------- */
    pconn = &NetConn_Tbl[0];
    for (i = 0; i < NET_CONN_CFG_NBR_CONN; i++) {
        pconn->Type  =  NET_CONN_TYPE_CONN;                     /* Init each net conn type/id--NEVER modify.            */
        pconn->ID    = (NET_CONN_ID)i;

        pconn->Flags =  NET_CONN_FLAG_NONE;                     /* Init each net conn as NOT used.                      */

#if (NET_DBG_CFG_MEM_CLR_EN == DEF_ENABLED)
        NetConn_Clr(pconn);
#endif

        pconn->NextConnPtr = (void     *)NetConn_PoolPtr;       /* Free each net conn to net conn pool (see Note #2).   */
        NetConn_PoolPtr    = (NET_CONN *)pconn;

        pconn++;
    }


                                                                /* --------------- INIT NET CONN LISTS ---------------- */
                                                                /* Init net conn lists.                                 */
    pconn_list = &NetConn_ServerConnListHead[0];
    for (j = 0; j < NET_CONN_CFG_PROTOCOL_MAX; j++) {
       *pconn_list = (NET_CONN *)0;
        pconn_list++;
    }

    pconn_list = &NetConn_ClientConnListHead[0];
    for (j = 0; j < NET_CONN_CFG_PROTOCOL_MAX; j++) {
       *pconn_list = (NET_CONN *)0;
        pconn_list++;
    }

                                                                /* Init net conn list ptrs.                             */
    NetConn_ConnListChainPtr     = (NET_CONN *)0;
    NetConn_ConnListConnPtr      = (NET_CONN *)0;
    NetConn_ConnListNextChainPtr = (NET_CONN *)0;
    NetConn_ConnListNextConnPtr  = (NET_CONN *)0;



/*$PAGE*/
                                                                /* ----------- INIT NET CONN WILDCARD ADDRS ----------- */
                                                                /* See Note #3.                                         */
    Mem_Clr((void     *)&NetConn_AddrWildCard[0],
            (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);

#if (NET_CONN_CFG_FAMILY == NET_CONN_FAMILY_IP_V4_SOCK)
    if (NET_CONN_ADDR_IP_LEN_ADDR <= NET_CONN_CFG_ADDR_LEN) {   /* See Note #4.                                         */
        NetConn_AddrWildCardAvail  = DEF_YES;

        val_len = NET_CONN_ADDR_IP_LEN_ADDR;
        for (k = 0; k < val_len; k++) {
            val                      = (NET_CONN_ADDR_IP_WILD_CARD >> (k * DEF_OCTET_NBR_BITS)) & DEF_OCTET_MASK;
            ix                       =  val_len - k - 1;
            NetConn_AddrWildCard[ix] =  val;
        }

    } else {
        NetConn_AddrWildCardAvail = DEF_NO;
    }


#else
    NetConn_AddrWildCardAvail = DEF_NO;
   (void)&k;                                                    /* Prevent compiler warnings.                           */
   (void)&ix;
   (void)&val;
   (void)&val_len;
#endif



                                                                /* ---------- INIT NET CONN STAT & ERR CTRS ----------- */
#if (NET_CTR_CFG_ERR_EN                == DEF_ENABLED)
    NetConn_ErrNoneAvailCtr            =  0;
    NetConn_ErrNotUsedCtr              =  0;

    NetConn_ErrCloseCtr                =  0;

    NetConn_ErrInvalidConnCtr          =  0;
    NetConn_ErrInvalidConnAddrLenCtr   =  0;
    NetConn_ErrInvalidConnAddrInUseCtr =  0;

    NetConn_ErrInvalidFamilyCtr        =  0;

    NetConn_ErrInvalidListTypeCtr      =  0;
    NetConn_ErrInvalidListIxCtr        =  0;

#if (NET_ERR_CFG_ARG_CHK_DBG_EN        == DEF_ENABLED)
    NetConn_ErrNullPtrCtr              =  0;

    NetConn_ErrInvalidTypeCtr          =  0;
#endif
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_CfgAccessedTh()
*
* Description : Configure network connection accessed-promotion threshold.
*
* Argument(s) : nbr_access      Desired number of accesses before network connection is promoted.
*
* Return(s)   : DEF_OK, network connection promotion threshold configured.
*
* Caller(s)   : Net_InitDflt(),
*               Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  NetConn_CfgAccessedTh (CPU_INT16U  nbr_access)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR  cpu_sr;
#endif


#if (NET_CONN_ACCESSED_TH_MIN > DEF_INT_16U_MIN_VAL)
    if (nbr_access < NET_CONN_ACCESSED_TH_MIN) {
        nbr_access = NET_CONN_ACCESSED_TH_MIN;
    }
#endif
#if (NET_CONN_ACCESSED_TH_MAX < DEF_INT_16U_MAX_VAL)
    if (nbr_access > NET_CONN_ACCESSED_TH_MAX) {
        nbr_access = NET_CONN_ACCESSED_TH_MAX;
    }
#endif

    CPU_CRITICAL_ENTER();
    NetConn_AccessedTh_nbr = nbr_access;
    CPU_CRITICAL_EXIT();

    return (DEF_OK);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            NetConn_Get()
*
* Description : (1) Allocate & initialize a network connection :
*
*                   (a) Get      a network connection
*                   (b) Validate   network connection
*                   (c) Initialize network connection
*                   (d) Update     network connection pool statistics
*                   (e) Return network connection handle identifier
*                         OR
*                       Null identifier & error code, on failure
*
*               (2) The network connection pool is implemented as a stack :
*
*                   (a) 'NetConn_PoolPtr' points to the head of the network connection pool.
*
*                   (b) Connections' 'NextConnPtr's link each connection to form the connection pool stack.
*
*                   (c) Connections are inserted & removed at the head of        the connection pool stack.
*
*
*                                     Connections are
*                                    inserted & removed
*                                        at the head
*                                      (see Note #2c)
*
*                                             |               NextConnPtr
*                                             |             (see Note #2b)
*                                             v                    |
*                                                                  |
*                                          -------       -------   v   -------       -------
*                     Connection Pool ---->|     |------>|     |------>|     |------>|     |
*                         Pointer          |     |       |     |       |     |       |     |
*                                          |     |       |     |       |     |       |     |
*                      (see Note #2a)      -------       -------       -------       -------
*
*                                          |                                               |
*                                          |<--------- Pool of Free Connections ---------->|
*                                          |                (see Note #2)                  |
*
*
* Argument(s) : perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection successfully allocated &
*                                                                   initialized.
*                               NET_CONN_ERR_NONE_AVAIL         NO available network connections to allocate.
*                               NET_CONN_ERR_INVALID_FAMILY     Invalid network connection family.
*                               NET_CONN_ERR_INVALID_TYPE       Invalid network connection type.
*                               NET_CONN_ERR_INVALID_LIST_IX    Invalid network connection list index.
*
* Return(s)   : Connection handle identifier, if NO errors.
*
*               NET_CONN_ID_NONE,             otherwise.
*
* Caller(s)   : NetSock_BindHandler(),
*               NetTCP_RxPktConnHandlerListen().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (3) (a) Network connection pool is accessed by 'NetConn_PoolPtr' during execution of
*
*                       (1) NetConn_Init()
*                       (2) NetConn_Get()
*                       (3) NetConn_FreeHandler()
*
*                   (b) Since the primary tasks of the network protocol suite are prevented from running 
*                       concurrently (see 'net.h  Note #2'), it is NOT necessary to protect the shared 
*                       resources of the network connection pool since no asynchronous access from other 
*                       network tasks is possible.
*********************************************************************************************************
*/
/*$PAGE*/
NET_CONN_ID  NetConn_Get (NET_CONN_FAMILY    family,
                          NET_CONN_LIST_IX   protocol_ix,
                          NET_ERR           *perr)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR        cpu_sr;
#endif
    NET_CONN     *pconn;
    NET_CONN_ID   conn_id;
    NET_ERR       stat_err;


                                                                /* -------------- VALIDATE NET CONN ARGS -------------- */
    switch (family) {
#if (NET_CONN_CFG_FAMILY == NET_CONN_FAMILY_IP_V4_SOCK)
        case NET_CONN_FAMILY_IP_V4_SOCK:
             switch (protocol_ix) {
                 case NET_CONN_LIST_IX_IP_V4_SOCK_UDP:
#ifdef  NET_TCP_MODULE_PRESENT
                 case NET_CONN_LIST_IX_IP_V4_SOCK_TCP:
#endif
                      break;


                 default:
                      NET_CTR_ERR_INC(NetConn_ErrInvalidListIxCtr);
                     *perr =  NET_CONN_ERR_INVALID_LIST_IX;
                      return (NET_CONN_ID_NONE);                /* Prevent 'break NOT reachable' compiler warning.      */
             }
             break;
#endif

        case NET_CONN_FAMILY_NONE:
        default:
             NET_CTR_ERR_INC(NetConn_ErrInvalidFamilyCtr);
            *perr =  NET_CONN_ERR_INVALID_FAMILY;
             return (NET_CONN_ID_NONE);                         /* Prevent 'break NOT reachable' compiler warning.      */
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (protocol_ix >= NET_CONN_CFG_PROTOCOL_MAX) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidListIxCtr);
       *perr =  NET_CONN_ERR_INVALID_LIST_IX;
        return (NET_CONN_ID_NONE);
    }
#endif


                                                                /* ------------------- GET NET CONN ------------------- */
    if (NetConn_PoolPtr != (NET_CONN *)0) {                     /* If net conn pool NOT empty, get net conn from pool.  */
        pconn            = (NET_CONN *)NetConn_PoolPtr;
        NetConn_PoolPtr  = (NET_CONN *)pconn->NextConnPtr;

    } else {                                                    /* If none avail, rtn err.                              */
        NET_CTR_ERR_INC(NetConn_ErrNoneAvailCtr);
       *perr =  NET_CONN_ERR_NONE_AVAIL;
        return (NET_CONN_ID_NONE);
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ---------------- VALIDATE NET CONN ----------------- */
    if (pconn->Type != NET_CONN_TYPE_CONN) {
        NetConn_Discard(pconn);
        NET_CTR_ERR_INC(NetConn_ErrInvalidTypeCtr);
       *perr =  NET_CONN_ERR_INVALID_TYPE;
        return (NET_CONN_ID_NONE);
    }
#endif

                                                                /* ------------------ INIT NET CONN ------------------- */
    NetConn_Clr(pconn);
    DEF_BIT_SET(pconn->Flags, NET_CONN_FLAG_USED);              /* Set net conn as used.                                */
    pconn->Family         = family;
    pconn->ProtocolListIx = protocol_ix;

                                                                /* ------------ UPDATE NET CONN POOL STATS ------------ */
    NetStat_PoolEntryUsedInc(&NetConn_PoolStat, &stat_err);

                                                                /* ----------------- RTN NET CONN ID ------------------ */
    conn_id = pconn->ID;
   *perr    = NET_CONN_ERR_NONE;

    return (conn_id);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           NetConn_Free()
*
* Description : Free a network connection.
*
*               (1) Network connection free ONLY frees but does NOT close any connections.
*
*
* Argument(s) : conn_id     Handle identifier of network connection to free.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (2) #### To prevent freeing a network connection already freed via previous network
*                   connection free, NetConn_Free() checks if the network connection is used BEFORE 
*                   freeing the network connection.
*
*                   This prevention is only best-effort since any invalid duplicate network connection 
*                   frees MAY be asynchronous to potentially valid network connection gets.  Thus the 
*                   invalid network connection free(s) MAY corrupt the network connection's valid
*                   operation(s).
*
*                   However, since the primary tasks of the network protocol suite are prevented from
*                   running concurrently (see 'net.h  Note #2'), it is NOT necessary to protect network
*                   connection resources from possible corruption since no asynchronous access from
*                   other network tasks is possible.
*********************************************************************************************************
*/

void  NetConn_Free (NET_CONN_ID  conn_id)
{
#if  (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    NET_ERR    err;
#endif
    NET_CONN  *pconn;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, &err);
    if (err != NET_CONN_ERR_NONE) {                             /* If net conn NOT used, ...                            */
        return;                                                 /* ... rtn but do NOT free (see Note #2).               */
    }
#endif

                                                                /* ------------------ FREE NET CONN ------------------- */
    pconn = &NetConn_Tbl[conn_id];
    NetConn_FreeHandler(pconn);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_CloseFromApp()
*
* Description : (1) Close a network connection from application layer :
*
*                   (a) Close transport connection, if requested                            See Note #3c
*                   (b) Clear network   connection's reference to application connection    See Note #3c
*                   (c) Free  network   connection, if necessary
*
*
* Argument(s) : conn_id                 Handle identifier of network connection to close.
*
*               close_conn_transport    Indicate whether to close transport connection :
*
*                                           DEF_YES                    Close transport connection.
*                                           DEF_NO              Do NOT close transport connection.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_CloseConn(),
*               NetSock_CloseSockHandler(),
*               NetSock_ConnAcceptQ_Clr().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (2) #### To prevent closing a network connection already closed via previous network
*                   connection close, NetConn_CloseFromApp() checks if the network connection is used 
*                   BEFORE closing the network connection.
*
*                   This prevention is only best-effort since any invalid duplicate network connection 
*                   closes MAY be asynchronous to potentially valid network connection gets.  Thus the 
*                   invalid network connection close(s) MAY corrupt the network connection's valid
*                   operation(s).
*
*                   However, since the primary tasks of the network protocol suite are prevented from
*                   running concurrently (see 'net.h  Note #2'), it is NOT necessary to protect network
*                   connection resources from possible corruption since no asynchronous access from
*                   other network tasks is possible.
*
*                   (a) Network connection(s) MAY already be closed AFTER other network connection 
*                       close operations & MUST be validated as used BEFORE any subsequent network 
*                       connection close operation(s).
*
*               (3) (a) Network connections are considered connected if any of the following network 
*                       connections are valid :
*
*                       (1) Application layer connection
*                       (2) Transport   layer connection
*
*                               (A) Network connections which ONLY reference application layer clone 
*                                   connection(s) are NOT considered connected since the actual non-
*                                   cloned application connection MAY or MAY NOT reference the cloned
*                                   network connection.
*
*                   (b) Since NetConn_CloseFromApp() actively closes the application layer connection,
*                       network connections need only validate the remaining transport layer connection
*                       as connected.
*
*                   (c) Since network connection(s) connection validation determines, in part, when to
*                       close the network connection (see Note #3a), & since NetConn_CloseFromTransport() 
*                       may indirectly call NetConn_CloseFromApp(); clearing the network connection's 
*                       application layer connection handle identifier MUST follow the closing of the 
*                       transport   layer connection to prevent re-closing the network connection.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_CloseFromApp (NET_CONN_ID  conn_id,
                            CPU_BOOLEAN  close_conn_transport)
{
    CPU_BOOLEAN   used;
    CPU_BOOLEAN   connd;
    CPU_BOOLEAN   free;
    NET_CONN     *pconn;
    NET_ERR       err;

                                                                    /* -------------- VALIDATE NET CONN --------------- */
    if (conn_id == NET_CONN_ID_NONE) {
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                     /* ------------ VALIDATE NET CONN USED ------------ */
   (void)NetConn_IsUsed(conn_id, &err);
    if (err != NET_CONN_ERR_NONE) {                                 /* If net conn NOT used, ...                        */
        return;                                                     /* ... rtn but do NOT close (see Note #2).          */
    }
#endif


    pconn = &NetConn_Tbl[conn_id];

                                                                    /* ------------- CLOSE TRANSPORT CONN ------------- */
    if (close_conn_transport == DEF_YES) {                          
        NetConn_CloseTransport(pconn);                              /* Close transport conn, if req'd.                  */
        used  =  DEF_BIT_IS_SET(pconn->Flags, NET_CONN_FLAG_USED);  /* If still used (see Note #2a), free net conn ...  */
        free  = (used == DEF_YES) ? DEF_YES : DEF_NO;               /* ... since app & transport conns closed.          */

    } else {                                                        /* Else chk net conn conn'd (see Note #3b).         */
        connd = (pconn->ID_Transport != NET_CONN_ID_NONE) ? DEF_YES : DEF_NO;
        free  =  (connd != DEF_YES) ? DEF_YES : DEF_NO;             /* Free net conn, if NOT conn'd.                    */
    }

                                                                    /* ---------------- CLOSE APP CONN ---------------- */
    NetConn_ID_AppSet(conn_id, NET_CONN_ID_NONE, &err);             /* Clr net conn's app conn id (see Note #3c).       */

                                                                    /* ---------------- FREE NET CONN ----------------- */
    if (free == DEF_YES) {
        NetConn_FreeHandler(pconn);                                 /* Free net conn, if req'd.                         */
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                    NetConn_CloseFromTransport()
*
* Description : (1) Close a network connection from transport layer :
*
*                   (a) Close application connection, if requested                          See Note #3c
*                   (b) Clear network     connection's reference to transport connection    See Note #3c
*                   (c) Free  network     connection, if necessary
*
*
* Argument(s) : conn_id             Handle identifier of network connection to close.
*
*               close_conn_app      Indicate whether to close application connection :
*
*                                       DEF_YES                        Close application connection.
*                                       DEF_NO                  Do NOT close application connection.
*
* Return(s)   : none.
*
* Caller(s)   : NetTCP_ConnClose().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (2) #### To prevent closing a network connection already closed via previous network
*                   connection close, NetConn_CloseFromTransport() checks if the network connection 
*                   is used BEFORE closing the network connection.
*
*                   This prevention is only best-effort since any invalid duplicate network connection 
*                   closes MAY be asynchronous to potentially valid network connection gets.  Thus the 
*                   invalid network connection close(s) MAY corrupt the network connection's valid
*                   operation(s).
*
*                   However, since the primary tasks of the network protocol suite are prevented from
*                   running concurrently (see 'net.h  Note #2'), it is NOT necessary to protect network
*                   connection resources from possible corruption since no asynchronous access from
*                   other network tasks is possible.
*
*                   (a) Network connection(s) MAY already be closed AFTER other network connection 
*                       close operations & MUST be validated as used BEFORE any subsequent network 
*                       connection close operation(s).
*
*               (3) (a) Network connections are considered connected if any of the following network 
*                       connections are valid :
*
*                       (1) Application layer connection
*                       (2) Transport   layer connection
*
*                               (A) Network connections which ONLY reference application layer clone 
*                                   connection(s) are NOT considered connected since the actual non-
*                                   cloned application connection MAY or MAY NOT reference the cloned
*                                   network connection.
*
*                   (b) Since NetConn_CloseFromTransport() actively closes the transport layer connection,
*                       network connections need only validate the remaining application layer connection
*                       as connected.
*
*                   (c) Since network connection(s) connection validation determines, in part, when to
*                       close the network connection (see Note #3a), & since NetConn_CloseFromApp() may
*                       indirectly call NetConn_CloseFromTransport(); clearing the network connection's 
*                       transport   layer connection handle identifier MUST follow the closing of the 
*                       application layer connection to prevent re-closing the network connection.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_CloseFromTransport (NET_CONN_ID  conn_id,
                                  CPU_BOOLEAN  close_conn_app)
{
    CPU_BOOLEAN   used;
    CPU_BOOLEAN   connd;
    CPU_BOOLEAN   free;
    NET_CONN     *pconn;
    NET_ERR       err;

                                                                    /* -------------- VALIDATE NET CONN --------------- */
    if (conn_id == NET_CONN_ID_NONE) {
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                     /* ------------ VALIDATE NET CONN USED ------------ */
   (void)NetConn_IsUsed(conn_id, &err);
    if (err != NET_CONN_ERR_NONE) {                                 /* If net conn NOT used, ...                        */
        return;                                                     /* ... rtn but do NOT close (see Note #2).          */
    }
#endif


    pconn = &NetConn_Tbl[conn_id];

                                                                    /* ---------------- CLOSE APP CONN ---------------- */
    if (close_conn_app == DEF_YES) {                          
        NetConn_CloseApp(pconn);                                    /* Close app conn, if req'd.                        */
        used  =  DEF_BIT_IS_SET(pconn->Flags, NET_CONN_FLAG_USED);  /* If still used (see Note #2a), free net conn ...  */
        free  = (used == DEF_YES) ? DEF_YES : DEF_NO;               /* ... since app & transport conns closed.          */

    } else {                                                        /* Else chk net conn conn'd (see Note #3b).         */
        connd = (pconn->ID_App != NET_CONN_ID_NONE) ? DEF_YES : DEF_NO;
        free  =  (connd != DEF_YES) ? DEF_YES : DEF_NO;             /* Free net conn, if NOT conn'd.                    */
    }

                                                                    /* ------------- CLOSE TRANSPORT CONN ------------- */
    NetConn_ID_TransportSet(conn_id, NET_CONN_ID_NONE, &err);       /* Clr net conn's transport conn id (see Note #3c). */

                                                                    /* ---------------- FREE NET CONN ----------------- */
    if (free == DEF_YES) {
        NetConn_FreeHandler(pconn);                                 /* Free net conn, if req'd.                         */
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_CloseAllConns()
*
* Description : Close ALL network connections.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY
*               be called by application function(s) [see also Note #2].
*
* Note(s)     : (1) (a) Certain circumstances may require that :
*
*                       (1) ALL network protocol suite connections close; ...
*                       (2) All pending network &/or application connection function(s) SHOULD :
*                           (A) Abort, immediately if possible; ...
*                           (B) Return appropriate closed error code(s).
*
*                   (b) The following example(s) list when to close all network connections :
*
*                       (1) RFC #2131, Section 4.4.5 states that "if the [DHCP] client is given a
*                           new network address, it MUST NOT continue using the previous network
*                           address and SHOULD notify the local users of the problem".
*
*                           Therefore, ALL network connections based on a previous network address
*                           MUST be closed.
*
*               (2) NetConn_CloseAllConns() is called by application function(s) & ... :
*
*                   (a) MUST NOT be called with the global network lock already acquired; ...
*                   (b) MUST block ALL other network protocol tasks by pending on & acquiring the global
*                       network lock (see 'net.h  Note #2').
*
*                   This is required since an application's network protocol suite API function access is 
*                   asynchronous to other network protocol tasks.
*
*                   See also 'NetConn_CloseAllConnsHandler()  Note #1'.
*********************************************************************************************************
*/

void  NetConn_CloseAllConns (void)
{
    NET_ERR  err;


    NetOS_Lock(&err);                                           /* Acquire net lock (see Note #2b).                     */
    if (err != NET_OS_ERR_NONE) {
        return;
    }

    NetConn_CloseAllConnsHandler();                             /* Close all net conns.                                 */

    NetOS_Unlock();                                             /* Release net lock.                                    */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                   NetConn_CloseAllConnsHandler()
*
* Description : Close ALL network connections in ALL network connection lists.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_CloseAllConns().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s) [see also Note #1].
*
* Note(s)     : (1) NetConn_CloseAllConnsHandler() is called by network protocol suite function(s) &
*                   MUST be called with the global network lock already acquired.
*
*                   See also 'NetConn_CloseAllConns()  Note #2'.
*********************************************************************************************************
*/

void  NetConn_CloseAllConnsHandler (void)
{
    NET_CONN           **pconn_list;
    NET_CONN_LIST_QTY    i;

                                                                /* -------- CLOSE SERVER CONN LIST'S NET CONNS -------- */
    pconn_list = &NetConn_ServerConnListHead[0];
    for (i = 0; i < NET_CONN_CFG_PROTOCOL_MAX; i++) {
        NetConn_CloseAllConnListConns(pconn_list);
       *pconn_list = (NET_CONN *)0;
        pconn_list++;
    }
                                                                /* -------- CLOSE CLIENT CONN LIST'S NET CONNS -------- */
    pconn_list = &NetConn_ClientConnListHead[0];
    for (i = 0; i < NET_CONN_CFG_PROTOCOL_MAX; i++) {
        NetConn_CloseAllConnListConns(pconn_list);
       *pconn_list = (NET_CONN *)0;
        pconn_list++;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         NetConn_ID_AppGet()
*
* Description : Get a network connection's application layer handle identifier.
*
* Argument(s) : conn_id     Handle identifier of network connection to get application layer handle identifier.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection handle identifier get
*                                                                   successful.
*
*                                                               ---- RETURNED BY NetConn_IsUsed() : ----
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : Network connection's application layer handle identifier, if NO errors.
*
*               NET_CONN_ID_NONE,                                         otherwise.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

NET_CONN_ID  NetConn_ID_AppGet (NET_CONN_ID   conn_id,
                                NET_ERR      *perr)
{
    NET_CONN     *pconn;
    NET_CONN_ID   conn_id_app;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
       *perr =  NET_CONN_ERR_NONE;
        return (NET_CONN_ID_NONE);
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return (NET_CONN_ID_NONE);
    }
#endif


    pconn       = &NetConn_Tbl[conn_id];
    conn_id_app =  pconn->ID_App;                               /* Get net conn's app id.                               */


   *perr =  NET_CONN_ERR_NONE;

    return (conn_id_app);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         NetConn_ID_AppSet()
*
* Description : Set a network connection's application layer handle identifier.
*
* Argument(s) : conn_id         Handle identifier of network connection to set application layer handle
*                                   identifier.
*
*               conn_id_app     Connection's application layer handle identifier.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection's handle identifier
*                                                                   successfully set.
*
*                                                               --- RETURNED BY NetConn_IsUsed() : ---
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetConn_ID_AppSet (NET_CONN_ID   conn_id,
                         NET_CONN_ID   conn_id_app,
                         NET_ERR      *perr)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR     cpu_sr;
#endif
    NET_CONN  *pconn;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
       *perr = NET_CONN_ERR_NONE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

                                                                /* --------------- VALIDATE APP CONN ID --------------- */
    if (conn_id_app < NET_CONN_ID_NONE) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnCtr);
       *perr = NET_CONN_ERR_INVALID_CONN;
        return;
    }

    pconn         = &NetConn_Tbl[conn_id];
    pconn->ID_App =  conn_id_app;                               /* Set net conn's app id.                               */


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NetConn_ID_AppCloneGet()
*
* Description : Get a network connection's application layer clone handle identifier.
*
* Argument(s) : conn_id     Handle identifier of network connection to get application layer clone handle
*                               identifier.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection handle identifier get
*                                                                   successful.
*
*                                                               ---- RETURNED BY NetConn_IsUsed() : ----
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : Network connection's application layer clone handle identifier, if NO errors.
*
*               NET_CONN_ID_NONE,                                               otherwise.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

NET_CONN_ID  NetConn_ID_AppCloneGet (NET_CONN_ID   conn_id,
                                     NET_ERR      *perr)
{
    NET_CONN     *pconn;
    NET_CONN_ID   conn_id_app_clone;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
       *perr =  NET_CONN_ERR_NONE;
        return (NET_CONN_ID_NONE);
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return (NET_CONN_ID_NONE);
    }
#endif


    pconn             = &NetConn_Tbl[conn_id];
    conn_id_app_clone =  pconn->ID_AppClone;                    /* Get net conn's app clone id.                         */


   *perr =  NET_CONN_ERR_NONE;

    return (conn_id_app_clone);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NetConn_ID_AppCloneSet()
*
* Description : Set a network connection's application layer clone handle identifier.
*
* Argument(s) : conn_id         Handle identifier of network connection to set application layer clone 
*                                   handle identifier.
*
*               conn_id_app     Connection's application layer handle identifier.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection's handle identifier
*                                                                   successfully set.
*
*                                                               --- RETURNED BY NetConn_IsUsed() : ---
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetConn_ID_AppCloneSet (NET_CONN_ID   conn_id,
                              NET_CONN_ID   conn_id_app,
                              NET_ERR      *perr)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR     cpu_sr;
#endif
    NET_CONN  *pconn;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
       *perr = NET_CONN_ERR_NONE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

                                                                /* --------------- VALIDATE APP CONN ID --------------- */
    if (conn_id_app < NET_CONN_ID_NONE) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnCtr);
       *perr = NET_CONN_ERR_INVALID_CONN;
        return;
    }

    pconn              = &NetConn_Tbl[conn_id];
    pconn->ID_AppClone =  conn_id_app;                          /* Set net conn's app clone id.                         */


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NetConn_ID_TransportGet()
*
* Description : Get a network connection's transport layer handle identifier.
*
* Argument(s) : conn_id     Handle identifier of network connection to get transport layer handle identifier.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection handle identifier get
*                                                                   successful.
*
*                                                               ---- RETURNED BY NetConn_IsUsed() : ----
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : Network connection's transport layer handle identifier, if NO errors.
*
*               NET_CONN_ID_NONE,                                       otherwise.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

NET_CONN_ID  NetConn_ID_TransportGet (NET_CONN_ID   conn_id,
                                      NET_ERR      *perr)
{
    NET_CONN     *pconn;
    NET_CONN_ID   conn_id_transport;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
       *perr =  NET_CONN_ERR_NONE;
        return (NET_CONN_ID_NONE);
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return (NET_CONN_ID_NONE);
    }
#endif


    pconn             = &NetConn_Tbl[conn_id];
    conn_id_transport =  pconn->ID_Transport;                   /* Get net conn's transport id.                         */


   *perr =  NET_CONN_ERR_NONE;

    return (conn_id_transport);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NetConn_ID_TransportSet()
*
* Description : Set a network connection's transport layer handle identifier.
*
* Argument(s) : conn_id             Handle identifier of network connection to set transport layer handle
*                                       identifier.
*
*               conn_id_transport   Connection's transport layer handle identifier.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection's handle identifier
*                                                                   successfully set.
*
*                                                               --- RETURNED BY NetConn_IsUsed() : ---
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetConn_ID_TransportSet (NET_CONN_ID   conn_id,
                               NET_CONN_ID   conn_id_transport,
                               NET_ERR      *perr)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR     cpu_sr;
#endif
    NET_CONN  *pconn;

                                                                /* ---------------- VALIDATE NET CONN ----------------- */
    if (conn_id == NET_CONN_ID_NONE) {
       *perr = NET_CONN_ERR_NONE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

                                                                /* ------------ VALIDATE TRANSPORT CONN ID ------------ */
    if (conn_id_transport < NET_CONN_ID_NONE) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnCtr);
       *perr = NET_CONN_ERR_INVALID_CONN;
        return;
    }

    pconn               = &NetConn_Tbl[conn_id];
    pconn->ID_Transport =  conn_id_transport;                   /* Set net conn's transport id.                         */


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_AddrLocalGet()
*
* Description : Get a network connection's local address.
*
* Argument(s) : conn_id         Handle identifier of network connection to get local address.
*
*               paddr_local     Pointer to variable that will receive the return  local address (see Note #1),
*                                   if NO errors.
*
*               paddr_len       Pointer to a variable to ... :
*                              
*                                   (a) Pass the size of the address buffer pointed to by 'paddr_local'.
*                                   (b) Return the actual local address length, if NO errors;
*                                       Return 0,                               otherwise.
*
*                               See also Note #2.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Network connection address get successful.
*                               NET_CONN_ERR_NULL_PTR               Argument 'paddr_local'/'paddr_len'
*                                                                       passed a NULL pointer.
*                               NET_CONN_ERR_INVALID_ADDR_LEN       Invalid network connection address length.
*                               NET_CONN_ERR_ADDR_NOT_USED          Network connection address NOT in use.
*
*                                                                   ----- RETURNED BY NetConn_IsUsed() : -----
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_ConnHandlerStream(),
*               NetSock_ConnHandlerAddrRemoteValidate(),
*               NetSock_TxDataHandlerDatagram(),
*               NetSock_FreeAddr(),
*               NetTCP_TxConnPrepareSegAddrs().
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (1) Network connection addresses maintained in network-order.
*
*               (2) (a) Since 'paddr_len' parameter is both an input & output parameter (see 'Argument(s) :
*                       paddr_len'), ...
*
*                       (1) its input value, prior to use, MUST be ...
*                           (A) saved, &                           ...
*                           (B) validated;                         ...
*
*                       (2) while its output value MUST be initially configured to return a default error
*                           value in case of any function exit due to error/fault conditions.
*
*                   (b) In the case that the 'paddr_len' parameter is passed a null pointer, ...
*
*                       (1) NO input value is saved, ...
*                       (2) NO input value is validated or used.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_AddrLocalGet (NET_CONN_ID         conn_id,
                            CPU_INT08U         *paddr_local,
                            NET_CONN_ADDR_LEN  *paddr_len,
                            NET_ERR            *perr)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR              cpu_sr;
#endif
#if  (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    NET_CONN_ADDR_LEN   addr_len;
#endif
    NET_CONN           *pconn;

                                                                /* ------------------ VALIDATE ADDR ------------------- */
#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (paddr_len == (NET_CONN_ADDR_LEN *)0) {                  /* See Note #2b.                                        */
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr = NET_CONN_ERR_NULL_PTR;
        return;
    }

     addr_len = *paddr_len;                                     /* Save     initial addr len         (see Note #2a1A).  */
#endif
   *paddr_len =  0;                                             /* Cfg      dflt    addr len for err (see Note #2a2).   */

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (addr_len < NET_CONN_CFG_ADDR_LEN) {                     /* Validate initial addr len         (see Note #2a1B).  */
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrLenCtr);
       *perr = NET_CONN_ERR_INVALID_ADDR_LEN;
        return;
    }
    if (paddr_local == (CPU_INT08U *)0) {
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr = NET_CONN_ERR_NULL_PTR;
        return;
    }

                                                                /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif


    pconn = &NetConn_Tbl[conn_id];

                                                                /* -------------- VALIDATE NET CONN ADDR -------------- */
    if (pconn->AddrLocalValid != DEF_YES) {                     /* If net conn local addr NOT avail, rtn err.           */
       *perr = NET_CONN_ERR_ADDR_NOT_USED;
        return;
    }


    Mem_Copy((void     *) paddr_local,                          /* Copy & rtn net conn local addr.                      */
             (void     *)&pconn->AddrLocal[0],
             (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);

   *paddr_len = NET_CONN_CFG_ADDR_LEN;


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_AddrLocalSet()
*
* Description : Set a network connection's local address.
*
* Argument(s) : conn_id         Handle identifier of network connection to set local address.
*
*               paddr_local     Pointer to local address (see Note #1).
*
*               addr_len        Length  of local address (in octets).
*
*               addr_over_wr    Allow      local address overwrite :
*
*                                   DEF_NO                          Do NOT overwrite local address.
*                                   DEF_YES                                Overwrite local address.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Network connection address successfully set.
*                               NET_CONN_ERR_NULL_PTR               Argument 'paddr_local' passed a NULL
*                                                                       pointer.
*                               NET_CONN_ERR_INVALID_ADDR_LEN       Invalid network connection address length.
*                               NET_CONN_ERR_ADDR_IN_USE            Network connection address already in use.
*
*                                                                   ------ RETURNED BY NetConn_IsUsed() : ------
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_BindHandler(),
*               NetTCP_RxPktConnHandlerListen().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (1) Network connection addresses maintained in network-order.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_AddrLocalSet (NET_CONN_ID         conn_id,
                            CPU_INT08U         *paddr_local,
                            NET_CONN_ADDR_LEN   addr_len,
                            CPU_BOOLEAN         addr_over_wr,
                            NET_ERR            *perr)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR     cpu_sr;
#endif
    NET_CONN  *pconn;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

    pconn = &NetConn_Tbl[conn_id];

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ------------------ VALIDATE ADDR ------------------- */
    if (paddr_local == (CPU_INT08U *)0) {
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr = NET_CONN_ERR_NULL_PTR;
        return;
    }
    if (addr_len != NET_CONN_CFG_ADDR_LEN) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrLenCtr);
       *perr = NET_CONN_ERR_INVALID_ADDR_LEN;
        return;
    }
    if (addr_over_wr != DEF_YES) {                              /* If addr over-wr NOT req'd ...                        */
        if (pconn->AddrLocalValid != DEF_NO) {                  /* ... & local addr valid,   ...                        */
            NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrInUseCtr);
           *perr = NET_CONN_ERR_ADDR_IN_USE;                    /* ... rtn err.                                         */
            return;
        }
    }

#else                                                           /* Prevent compiler warnings.                           */
   (void)&addr_len;
   (void)&addr_over_wr;
#endif


    Mem_Copy((void     *)&pconn->AddrLocal[0],                  /* Copy local addr to net conn addr.                    */
             (void     *) paddr_local,
             (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);

    pconn->AddrLocalValid = DEF_YES;


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_AddrRemoteGet()
*
* Description : Get a network connection's remote address.
*
* Argument(s) : conn_id         Handle identifier of network connection to get remote address.
*
*               paddr_remote    Pointer to variable that will receive the return  remote address (see Note #1),
*                                   if NO errors.
*
*               paddr_len       Pointer to a variable to ... :
*                              
*                                   (a) Pass the size of the address buffer pointed to by 'paddr_remote'.
*                                   (b) Return the actual local address length, if NO errors;
*                                       Return 0,                               otherwise.
*
*                               See also Note #2.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Network connection address get successful.
*                               NET_CONN_ERR_NULL_PTR               Argument 'paddr_local'/'paddr_len'
*                                                                       passed a NULL pointer.
*                               NET_CONN_ERR_INVALID_ADDR_LEN       Invalid network connection address length.
*                               NET_CONN_ERR_ADDR_NOT_USED          Network connection address NOT in use.
*
*                                                                   ----- RETURNED BY NetConn_IsUsed() : -----
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_Accept(),
*               NetSock_BindHandler(),
*               NetSock_RxDataHandlerStream(),
*               NetSock_TxDataHandlerDatagram(),
*               NetTCP_TxConnPrepareSegAddrs().
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (1) Network connection addresses maintained in network-order.
*
*               (2) (a) Since 'paddr_len' parameter is both an input & output parameter (see 'Argument(s) :
*                       paddr_len'), ...
*
*                       (1) its input value, prior to use, MUST be ...
*                           (A) saved, &                           ...
*                           (B) validated;                         ...
*
*                       (2) while its output value MUST be initially configured to return a default error
*                           value in case of any function exit due to error/fault conditions.
*
*                   (b) In the case that the 'paddr_len' parameter is passed a null pointer, ...
*
*                       (1) NO input value is saved, ...
*                       (2) NO input value is validated or used.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_AddrRemoteGet (NET_CONN_ID         conn_id,
                             CPU_INT08U         *paddr_remote,
                             NET_CONN_ADDR_LEN  *paddr_len,
                             NET_ERR            *perr)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR              cpu_sr;
#endif
#if  (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    NET_CONN_ADDR_LEN   addr_len;
#endif
    NET_CONN           *pconn;

                                                                /* ------------------ VALIDATE ADDR ------------------- */
#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (paddr_len == (NET_CONN_ADDR_LEN *)0) {                  /* See Note #2b.                                        */
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr = NET_CONN_ERR_NULL_PTR;
        return;
    }

     addr_len = *paddr_len;                                     /* Save     initial addr len         (see Note #2a1A).  */
#endif
   *paddr_len =  0;                                             /* Cfg      dflt    addr len for err (see Note #2a2).   */

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (addr_len < NET_CONN_CFG_ADDR_LEN) {                     /* Validate initial addr len         (see Note #2a1B).  */
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrLenCtr);
       *perr = NET_CONN_ERR_INVALID_ADDR_LEN;
        return;
    }
    if (paddr_remote == (CPU_INT08U *)0) {
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr = NET_CONN_ERR_NULL_PTR;
        return;
    }

                                                                /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif


    pconn = &NetConn_Tbl[conn_id];

                                                                /* -------------- VALIDATE NET CONN ADDR -------------- */
    if (pconn->AddrRemoteValid != DEF_YES) {                    /* If net conn remote addr NOT avail, rtn err.          */
       *perr = NET_CONN_ERR_ADDR_NOT_USED;
        return;
    }


    Mem_Copy((void     *) paddr_remote,                         /* Copy & rtn net conn remote addr.                     */
             (void     *)&pconn->AddrRemote[0],
             (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);

   *paddr_len = NET_CONN_CFG_ADDR_LEN;


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_AddrRemoteSet()
*
* Description : Set a network connection's remote address.
*
* Argument(s) : conn_id         Handle identifier of network connection to set remote address.
*
*               paddr_remote    Pointer to remote address (see Note #1).
*
*               addr_len        Length  of remote address (in octets).
*
*               addr_over_wr    Allow      remote address overwrite :
*
*                                   DEF_NO                          Do NOT overwrite remote address.
*                                   DEF_YES                                Overwrite remote address.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Network connection address successfully set.
*                               NET_CONN_ERR_NULL_PTR               Argument 'paddr_local' passed a NULL
*                                                                       pointer.
*                               NET_CONN_ERR_INVALID_ADDR_LEN       Invalid network connection address length.
*                               NET_CONN_ERR_ADDR_IN_USE            Network connection address already in use.
*
*                                                                   ------ RETURNED BY NetConn_IsUsed() : ------
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_ConnHandlerAddrRemoteSet(),
*               NetTCP_RxPktConnHandlerListen().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (1) Network connection addresses maintained in network-order.
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_AddrRemoteSet (NET_CONN_ID         conn_id,
                             CPU_INT08U         *paddr_remote,
                             NET_CONN_ADDR_LEN   addr_len,
                             CPU_BOOLEAN         addr_over_wr,
                             NET_ERR            *perr)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR     cpu_sr;
#endif
    NET_CONN  *pconn;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

    pconn = &NetConn_Tbl[conn_id];

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ------------------ VALIDATE ADDR ------------------- */
    if (paddr_remote == (CPU_INT08U *)0) {
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr = NET_CONN_ERR_NULL_PTR;
        return;
    }
    if (addr_len != NET_CONN_CFG_ADDR_LEN) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrLenCtr);
       *perr = NET_CONN_ERR_INVALID_ADDR_LEN;
        return;
    }
    if (addr_over_wr != DEF_YES) {                              /* If addr over-wr NOT req'd ...                        */
        if (pconn->AddrRemoteValid != DEF_NO) {                 /* ... & remote addr valid,  ...                        */
            NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrInUseCtr);
           *perr = NET_CONN_ERR_ADDR_IN_USE;                    /* ... rtn err.                                         */
            return;
        }
    }

#else                                                           /* Prevent compiler warnings.                           */
   (void)&addr_len;
   (void)&addr_over_wr;
#endif


    Mem_Copy((void     *)&pconn->AddrRemote[0],                 /* Copy remote addr to net conn addr.                   */
             (void     *) paddr_remote,
             (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);

    pconn->AddrRemoteValid = DEF_YES;


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       NetConn_AddrRemoteCmp()
*
* Description : Compare an address to a network connection's remote address.
*
* Argument(s) : conn_id         Handle identifier of connection to compare.
*
*               paddr_remote    Pointer to remote address to compare.
*
*               addr_len        Length  of remote address (in octets).
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Remote address successfully compared to 
*                                                                       network connection's remote address;
*                                                                       check return value.
*                               NET_CONN_ERR_NULL_PTR               Argument 'paddr_remote' passed a NULL
*                                                                       pointer.
*                               NET_CONN_ERR_INVALID_ADDR_LEN       Invalid network connection address length.
*                               NET_CONN_ERR_ADDR_NOT_USED          Network connection address NOT in use.
*
*                                                                   ----- RETURNED BY NetConn_IsUsed() : -----
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : DEF_YES, if addresses successfully compare.
*
*               DEF_NO,  otherwise.
*
* Caller(s)   : NetSock_IsValidAddrRemote().
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/
/*$PAGE*/
CPU_BOOLEAN  NetConn_AddrRemoteCmp (NET_CONN_ID         conn_id,
                                    CPU_INT08U         *paddr_remote,
                                    NET_CONN_ADDR_LEN   addr_len,
                                    NET_ERR            *perr)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR        cpu_sr;
#endif
    NET_CONN     *pconn;
    CPU_BOOLEAN   cmp;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
                                                                /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return (DEF_NO);
    }

                                                                /* ------------------ VALIDATE ADDR ------------------- */
    if (paddr_remote == (CPU_INT08U *)0) {
        NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
       *perr =  NET_CONN_ERR_NULL_PTR;
        return (DEF_NO);
    }
    if (addr_len != NET_CONN_CFG_ADDR_LEN) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrLenCtr);
       *perr =  NET_CONN_ERR_INVALID_ADDR_LEN;
        return (DEF_NO);
    }

#else                                                           /* Prevent compiler warning.                            */
   (void)&addr_len;
#endif


    pconn = &NetConn_Tbl[conn_id];

                                                                /* -------------- VALIDATE NET CONN ADDR -------------- */
    if (pconn->AddrRemoteValid != DEF_YES) {                    /* If conn local addr NOT avail, rtn err.               */
       *perr =  NET_CONN_ERR_ADDR_NOT_USED;
        return (DEF_NO);
    }

                                                                
    cmp = Mem_Cmp((void     *) paddr_remote,                    /* Cmp remote addr to conn addr.                        */
                  (void     *)&pconn->AddrRemote[0],
                  (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);

   *perr =  NET_CONN_ERR_NONE;

    return (cmp);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          NetConn_IsUsed()
*
* Description : Validate network connection in use.
*
* Argument(s) : conn_id     Handle identifier of network connection to validate.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection successfully validated
*                                                                   as in use.
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : DEF_YES, network connection   valid &      in use.
*
*               DEF_NO,  network connection invalid or NOT in use.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (1) NetConn_IsUsed() blocked until network initialization completes.
*
*               (2) NetConn_IsUsed() MUST be called with the global network lock already acquired.
*********************************************************************************************************
*/

CPU_BOOLEAN  NetConn_IsUsed (NET_CONN_ID   conn_id,
                             NET_ERR      *perr)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR        cpu_sr;
#endif
    NET_CONN     *pconn;
    CPU_BOOLEAN   used;


#if (NET_ERR_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
    if (Net_InitDone != DEF_YES) {                              /* If init NOT complete, exit (see Note #1).            */
       *perr =  NET_ERR_INIT_INCOMPLETE;
        return (DEF_NO);
    }
#endif

                                                                /* --------------- VALIDATE NET CONN ID --------------- */
    if (conn_id < NET_CONN_ID_MIN) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnCtr);
       *perr =  NET_CONN_ERR_INVALID_CONN;
        return (DEF_NO);
    }
    if (conn_id > NET_CONN_ID_MAX) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnCtr);
       *perr =  NET_CONN_ERR_INVALID_CONN;
        return (DEF_NO);
    }

                                                                /* -------------- VALIDATE NET CONN USED -------------- */
    pconn = &NetConn_Tbl[conn_id];
    used  =  DEF_BIT_IS_SET(pconn->Flags, NET_CONN_FLAG_USED);
    if (used != DEF_YES) {
        NET_CTR_ERR_INC(NetConn_ErrNotUsedCtr);
       *perr =  NET_CONN_ERR_NOT_USED;
        return (DEF_NO);
    }


   *perr =  NET_CONN_ERR_NONE;

    return (DEF_YES);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          NetConn_IsConn()
*
* Description : Determine network connection status.
*
* Argument(s) : conn_id     Handle identifier of network connection to check.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_CONN_NONE          NO   network connection.
*                               NET_CONN_ERR_CONN_HALF          Half network connection --
*                                                                   local OR  remote address valid.
*                               NET_CONN_ERR_CONN_FULL          Full network connection --
*                                                                   local AND remote address valid.
*
*                                                               --- RETURNED BY NetConn_IsUsed() : ---
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (1) NetConn_IsConn() MUST be called with the global network lock already acquired.
*********************************************************************************************************
*/

void  NetConn_IsConn (NET_CONN_ID   conn_id,
                      NET_ERR      *perr)
{
    NET_CONN  *pconn;

                                                                /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }

                                                                /* -------------- DETERMINE CONN STATUS --------------- */
    pconn = &NetConn_Tbl[conn_id];
    if (pconn->AddrLocalValid == DEF_YES) {
        if (pconn->AddrRemoteValid == DEF_YES) {
           *perr = NET_CONN_ERR_CONN_FULL;
        } else {
           *perr = NET_CONN_ERR_CONN_HALF;
        }
    } else {
        if (pconn->AddrRemoteValid == DEF_YES) {
           *perr = NET_CONN_ERR_CONN_HALF;
        } else {
           *perr = NET_CONN_ERR_CONN_NONE;
        }
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NetConn_GetConnListType()
*
* Description : Get network connection's current connection list type.
*
* Argument(s) : conn_id     Handle identifier of network connection to check.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Network connection list type successfully
*                                                                       returned; check return value.
*                               NET_CONN_ERR_INVALID_LIST           No current connection list.
*                               NET_CONN_ERR_INVALID_LIST_TYPE      Invalid network connection list type.
*
*                                                                   ---- RETURNED BY NetConn_IsUsed() : -----
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : Network connection's connection list type, if NO errors.
*
*               NET_CONN_LIST_TYPE_NONE,                   otherwise.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (1) Default case already invalidated in earlier network connection functions.  However,
*                   the default case is included as an extra precaution in case 'ConnListType' is 
*                   incorrectly modified.
*********************************************************************************************************
*/
/*$PAGE*/
NET_CONN_LIST_TYPE  NetConn_GetConnListType (NET_CONN_ID   conn_id,
                                             NET_ERR      *perr)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR               cpu_sr;
#endif
    NET_CONN            *pconn;
    NET_CONN_LIST_TYPE   list_type;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ------------- VALIDATE NET CONN USED --------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return (NET_CONN_LIST_TYPE_NONE);
    }
#endif

    pconn = &NetConn_Tbl[conn_id];

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ----------- VALIDATE NET CONN LIST USED ------------ */
    if (pconn->ConnList == (void **)0) {
       *perr =  NET_CONN_ERR_INVALID_LIST;
        return (NET_CONN_LIST_TYPE_NONE);
    }
#endif

                                                                /* ---------- GET NET CONN'S CONN LIST TYPE ----------- */
    list_type = pconn->ConnListType;

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    switch (list_type) {
        case NET_CONN_LIST_TYPE_NONE:
        case NET_CONN_LIST_TYPE_SERVER:
        case NET_CONN_LIST_TYPE_CLIENT:
             break;


        default:                                                /* See Note #1.                                         */
             NET_CTR_ERR_INC(NetConn_ErrInvalidListTypeCtr);
            *perr =  NET_CONN_ERR_INVALID_LIST_TYPE;
             return (NET_CONN_LIST_TYPE_NONE);                  /* Prevent 'break NOT reachable' compiler warning.      */
    }
#endif


   *perr =  NET_CONN_ERR_NONE;

    return (list_type);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NetConn_PoolStatGet()
*
* Description : Get network connection statistics pool.
*
* Argument(s) : none.
*
* Return(s)   : Network connection statistics pool, if NO errors.
*
*               NULL               statistics pool, otherwise.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : (1) NetConn_PoolStatGet() blocked until network initialization completes; return NULL
*                   statistics pool.
*********************************************************************************************************
*/

NET_STAT_POOL  NetConn_PoolStatGet (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
#if (NET_ERR_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
    NET_ERR        err;
#endif
    NET_STAT_POOL  stat_pool;


#if (NET_ERR_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
    if (Net_InitDone != DEF_YES) {                              /* If init NOT complete, ...                            */
        NetStat_PoolClr(&stat_pool, &err);
        return (stat_pool);                                     /* ... rtn NULL stat pool (see Note #1).                */
    }
#endif


    CPU_CRITICAL_ENTER();
    stat_pool = NetConn_PoolStat;
    CPU_CRITICAL_EXIT();

    return (stat_pool);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                   NetConn_PoolStatResetMaxUsed()
*
* Description : Reset network connection statistics pool's maximum number of entries used.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
*               This function is a network protocol suite application interface (API) function & MAY be 
*               called by application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetConn_PoolStatResetMaxUsed (void)
{
    NET_ERR  err;


    NetStat_PoolResetUsedMax(&NetConn_PoolStat, &err);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           NetConn_Srch()
*
* Description : (1) Search connection lists for network connection with specific local & remote addresses :
*
*                   (a) Get    network connection list table index
*                   (b) Search network connection lists :                                   See Note #2
*                       (1) Search with specified search addresses
*                           (A) Search server connection list first
*                           (B) Search client connection list second
*                       (2) Search with wildcard  search address(s), if available
*                           (A) Search server connection list first
*                           (B) Search client connection list second
*                   (c) Return network connection handle identifier, if network connection     found
*                         OR
*                       Null identifier & error code,                if network connection NOT found
*
*               (2) Network connection searches are resolved in order :
*
*                   (a) From greatest number of identical connection address fields to
*                            least    number of identical connection address fields.
*
*                   (b) (1) Server network connection lists first;
*                       (2) Client network connection lists second.
*
*
* Argument(s) : family                  Network connection family type.
*
*               protocol_ix             Network connection list protocol index.
*
*               conn_list_type          Network connection list type :
*
*                                           NET_CONN_LIST_TYPE_ALL          Search ALL    connection lists.
*                                           NET_CONN_LIST_TYPE_SERVER       Search Server Connection List ONLY.
*                                           NET_CONN_LIST_TYPE_CLIENT       Search Client Connection List ONLY.
*
*               sec_srch_addr_reqd      Indicate whether secondary search address is required.
*
*               paddr_local             Pointer to local  address (see Note #3).
*
*               paddr_remote            Pointer to remote address (see Note #3).
*
*               addr_len                Length  of search addresses (in octets).
*
*               pconn_id_transport      Pointer to variable that will receive the returned network
*                                           connection's transport   handle identifier.
*
*               pconn_id_app            Pointer to variable that will receive the returned network
*                                           connection's application handle identifier.
*
* Return(s)   : Handle identifier of network connection with specific local & remote address, if found.
*
*               NET_CONN_ID_NONE,                                                             otherwise.
*
* Caller(s)   : NetSock_RxPktDemux(),
*               NetSock_BindHandler(),
*               NetSock_ConnHandlerAddrRemoteValidate().
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by 
*               application function(s).
*
* Note(s)     : (3) Network connection addresses maintained in network-order.
*********************************************************************************************************
*/

NET_CONN_ID  NetConn_Srch (NET_CONN_FAMILY      family,
                           NET_CONN_LIST_IX     protocol_ix,
                           NET_CONN_LIST_TYPE   conn_list_type,
                           CPU_BOOLEAN          sec_srch_addr_reqd,
                           CPU_INT08U          *paddr_local,
                           CPU_INT08U          *paddr_remote,
                           NET_CONN_ADDR_LEN    addr_len,
                           NET_CONN_ID         *pconn_id_transport,
                           NET_CONN_ID         *pconn_id_app)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR         cpu_sr;
#endif
    NET_CONN     **pconn_list;
    NET_CONN      *pconn;
    NET_CONN_ID    conn_id;
    CPU_INT08U     addr_wildcard[NET_CONN_CFG_ADDR_LEN];


/*$PAGE*/
                                                                /* Init conn id's for err or NOT found srch's.          */
    if (pconn_id_transport != (NET_CONN_ID *)0) {
       *pconn_id_transport  =  NET_CONN_ID_NONE;
    }
    if (pconn_id_app       != (NET_CONN_ID *)0) {
       *pconn_id_app        =  NET_CONN_ID_NONE;
    }



#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
                                                                /* ----------- VALIDATE NET CONN LIST TYPE ------------ */
    switch (conn_list_type) {
        case NET_CONN_LIST_TYPE_ALL:
        case NET_CONN_LIST_TYPE_SERVER:
        case NET_CONN_LIST_TYPE_CLIENT:
             break;


        default:
             NET_CTR_ERR_INC(NetConn_ErrInvalidListTypeCtr);
             return (NET_CONN_ID_NONE);                         /* Prevent 'break NOT reachable' compiler warning.      */
    }

                                                                /* ---------------- VALIDATE ADDR LEN ----------------- */
    if (addr_len != NET_CONN_CFG_ADDR_LEN) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidConnAddrLenCtr);
        return (NET_CONN_ID_NONE);
    }
#else
   (void)&addr_len;                                             /* Prevent compiler warning.                            */
#endif


                                                                /* ------ VALIDATE/CFG CONN LIST FAMILY/PROTOCOL ------ */
    switch (family) {
#if (NET_CONN_CFG_FAMILY == NET_CONN_FAMILY_IP_V4_SOCK)
        case NET_CONN_FAMILY_IP_V4_SOCK:
#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
             switch (protocol_ix) {
                 case NET_CONN_LIST_IX_IP_V4_SOCK_UDP:
#ifdef  NET_TCP_MODULE_PRESENT
                 case NET_CONN_LIST_IX_IP_V4_SOCK_TCP:
#endif
                      break;


                 default:
                      NET_CTR_ERR_INC(NetConn_ErrInvalidListIxCtr);
                      return (NET_CONN_ID_NONE);                /* Prevent 'break NOT reachable' compiler warning.      */
             }
#endif

             if (NetConn_AddrWildCardAvail == DEF_YES) {        /* Cfg wildcard addr.                                   */
                 if (paddr_local != (CPU_INT08U *)0) {
                     Mem_Copy((void     *)&addr_wildcard[0],
                              (void     *)paddr_local,
                              (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);
                 } else {                                       /* If local addr NOT avail, clr wildcard addr.          */
                     Mem_Clr ((void     *)&addr_wildcard[0],
                              (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);
                 }

                 Mem_Copy((void     *)&addr_wildcard[NET_CONN_ADDR_IP_IX_ADDR],
                          (void     *)&NetConn_AddrWildCard[0],
                          (CPU_SIZE_T) NET_CONN_ADDR_IP_LEN_ADDR);
             }
             break;
#endif

        case NET_CONN_FAMILY_NONE:
        default:
             NET_CTR_ERR_INC(NetConn_ErrInvalidFamilyCtr);
             return (NET_CONN_ID_NONE);                         /* Prevent 'break NOT reachable' compiler warning.      */
    }
#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (protocol_ix >= NET_CONN_CFG_PROTOCOL_MAX) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidListIxCtr);
        return (NET_CONN_ID_NONE);
    }
#endif


/*$PAGE*/
                                                                /* --------------- SRCH NET CONN LISTS ---------------- */
                                                                /* ------ SRCH CONN LISTS WITH SPECIFIED ADDR(S) ------ */
    if ((conn_list_type == NET_CONN_LIST_TYPE_ALL   ) ||
        (conn_list_type == NET_CONN_LIST_TYPE_SERVER)) {        /* Srch server conn list with req'd addrs.              */

        pconn_list = &NetConn_ServerConnListHead[protocol_ix];
        pconn      =  NetConn_ListSrch((NET_CONN        **)pconn_list,
                                       (NET_CONN_LIST_TYPE)NET_CONN_LIST_TYPE_SERVER,
                                       (CPU_BOOLEAN       )sec_srch_addr_reqd,
                                       (CPU_INT08U       *)paddr_local,
                                       (CPU_INT08U       *)paddr_remote);
        if (pconn != (NET_CONN *)0) {                           /* If net conn found, rtn conn id's.                    */
            if (pconn_id_transport != (NET_CONN_ID *)0) {
               *pconn_id_transport  =  pconn->ID_Transport;
            }
            if (pconn_id_app       != (NET_CONN_ID *)0) {
               *pconn_id_app        =  pconn->ID_App;
            }
            conn_id = pconn->ID;
            return (conn_id);
        }
    }

    if ((conn_list_type == NET_CONN_LIST_TYPE_ALL   ) ||
        (conn_list_type == NET_CONN_LIST_TYPE_CLIENT)) {        /* Srch client conn list with req'd addrs.              */

        pconn_list = &NetConn_ClientConnListHead[protocol_ix];
        pconn      =  NetConn_ListSrch((NET_CONN        **)pconn_list,
                                       (NET_CONN_LIST_TYPE)NET_CONN_LIST_TYPE_CLIENT,
                                       (CPU_BOOLEAN       )sec_srch_addr_reqd,
                                       (CPU_INT08U       *)paddr_local,
                                       (CPU_INT08U       *)paddr_remote);
        if (pconn != (NET_CONN *)0) {                           /* If net conn found, rtn conn id's.                    */
            if (pconn_id_transport != (NET_CONN_ID *)0) {
               *pconn_id_transport  =  pconn->ID_Transport;
            }
            if (pconn_id_app       != (NET_CONN_ID *)0) {
               *pconn_id_app        =  pconn->ID_App;
            }
            conn_id = pconn->ID;
            return (conn_id);
        }
    }

/*$PAGE*/
                                                                /* -------- SRCH CONN LISTS WITH WILDCARD ADDR -------- */
    if (NetConn_AddrWildCardAvail == DEF_YES) {
        if ((conn_list_type == NET_CONN_LIST_TYPE_ALL   ) ||
            (conn_list_type == NET_CONN_LIST_TYPE_SERVER)) {    /* Srch server conn list with wildcard addr.            */

            pconn_list = &NetConn_ServerConnListHead[protocol_ix];
            pconn      =  NetConn_ListSrch((NET_CONN        **) pconn_list,
                                           (NET_CONN_LIST_TYPE) NET_CONN_LIST_TYPE_SERVER,
                                           (CPU_BOOLEAN       ) sec_srch_addr_reqd,
                                           (CPU_INT08U       *)&addr_wildcard[0],
                                           (CPU_INT08U       *)paddr_remote);
            if (pconn != (NET_CONN *)0) {                       /* If net conn found, rtn conn id's.                    */
                if (pconn_id_transport != (NET_CONN_ID *)0) {
                   *pconn_id_transport  =  pconn->ID_Transport;
                }
                if (pconn_id_app       != (NET_CONN_ID *)0) {
                   *pconn_id_app        =  pconn->ID_App;
                }
                conn_id = pconn->ID;
                return (conn_id);
            }
        }

        if ((conn_list_type == NET_CONN_LIST_TYPE_ALL   ) ||
            (conn_list_type == NET_CONN_LIST_TYPE_CLIENT)) {    /* Srch client conn list with wildcard addr.            */
                                                                
            pconn_list = &NetConn_ClientConnListHead[protocol_ix];
            pconn      =  NetConn_ListSrch((NET_CONN        **) pconn_list,
                                           (NET_CONN_LIST_TYPE) NET_CONN_LIST_TYPE_CLIENT,
                                           (CPU_BOOLEAN       ) sec_srch_addr_reqd,
                                           (CPU_INT08U       *)&addr_wildcard[0],
                                           (CPU_INT08U       *)paddr_remote);
            if (pconn != (NET_CONN *)0) {                       /* If net conn found, rtn conn id's.                    */
                if (pconn_id_transport != (NET_CONN_ID *)0) {
                   *pconn_id_transport  =  pconn->ID_Transport;
                }
                if (pconn_id_app       != (NET_CONN_ID *)0) {
                   *pconn_id_app        =  pconn->ID_App;
                }
                conn_id = pconn->ID;
                return (conn_id);
            }
        }
    }


    return (NET_CONN_ID_NONE);                                  /* NO net conn found.                                   */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          NetConn_ListAdd()
*
* Description : (1) Add a network connection into a connection list :
*
*                   (a) Get network connection's appropriate connection list
*                   (b) Get network connection's appropriate connection chain
*                   (c) Add network connection into          connection list
*
*
* Argument(s) : conn_id             Handle identifier of network connection to add.
*
*               conn_list_type      Network connection list type :
*
*                                       NET_CONN_LIST_TYPE_SERVER       Connection list organized primarily
*                                                                           by local  address.
*                                       NET_CONN_LIST_TYPE_CLIENT       Connection list organized primarily
*                                                                           by remote address.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE                   Network connection successfully
*                                                                       added to connection list.
*                               NET_CONN_ERR_INVALID_FAMILY         Invalid network connection family.
*                               NET_CONN_ERR_INVALID_LIST_TYPE      Invalid network connection list type.
*                               NET_CONN_ERR_INVALID_LIST_IX        Invalid network connection list index.
*
*                                                                   --- RETURNED BY NetConn_IsUsed() : ---
*                               NET_ERR_INIT_INCOMPLETE             Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN           Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED               Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_BindHandler(),
*               NetSock_ConnHandlerDatagram(),
*               NetSock_ConnHandlerStream(),
*               NetTCP_RxPktConnHandlerListen().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : (2) Network connection addresses maintained in network-order.
*
*               (3) Local &/or remote addresses SHOULD be set prior to network connection list add.
*
*               (4) #### Search for connection list chain ONLY performed if connection's remote address
*                   is available.  However, this assumes ALL connections have unique local addresses.
*
*                   #### If local addresses are NOT necessarily unique, should connection list chain
*                   always be searched?
*********************************************************************************************************
*/
/*$PAGE*/
void  NetConn_ListAdd (NET_CONN_ID          conn_id,
                       NET_CONN_LIST_TYPE   conn_list_type,
                       NET_ERR             *perr)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR              cpu_sr;
#endif
    NET_CONN           *pconn;
    NET_CONN           *pconn_chain;
    NET_CONN          **pconn_list;
    NET_CONN_LIST_IX    protocol_ix;
    CPU_INT08U          addr_wildcard[NET_CONN_CFG_ADDR_LEN];


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

    pconn = &NetConn_Tbl[conn_id];
                                                                /* ------ VALIDATE/CFG CONN LIST FAMILY/PROTOCOL ------ */
    switch (pconn->Family) {
#if (NET_CONN_CFG_FAMILY == NET_CONN_FAMILY_IP_V4_SOCK)
        case NET_CONN_FAMILY_IP_V4_SOCK:
             if (NetConn_AddrWildCardAvail == DEF_YES) {        /* Cfg wildcard addr.                                   */
                 if (pconn->AddrLocalValid == DEF_YES) {
                     Mem_Copy((void     *)&addr_wildcard[0],
                              (void     *)&pconn->AddrLocal[0],
                              (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);
                 } else {
                     Mem_Clr ((void     *)&addr_wildcard[0],
                              (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);
                 }

                 Mem_Copy((void     *)&addr_wildcard[NET_CONN_ADDR_IP_IX_ADDR],
                          (void     *)&NetConn_AddrWildCard[0],
                          (CPU_SIZE_T) NET_CONN_ADDR_IP_LEN_ADDR);
             }
             break;
#endif

        case NET_CONN_FAMILY_NONE:
        default:
             NET_CTR_ERR_INC(NetConn_ErrInvalidFamilyCtr);
            *perr = NET_CONN_ERR_INVALID_FAMILY;
             return;                                            /* Prevent 'break NOT reachable' compiler warning.      */
    }


/*$PAGE*/
                                                                /* ---------------- GET NET CONN LIST ----------------- */
    protocol_ix = pconn->ProtocolListIx;
#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
    if (protocol_ix >= NET_CONN_CFG_PROTOCOL_MAX) {
        NET_CTR_ERR_INC(NetConn_ErrInvalidListIxCtr);
       *perr = NET_CONN_ERR_INVALID_LIST_IX;
        return;
    }
#endif

    switch (conn_list_type) {
        case NET_CONN_LIST_TYPE_SERVER:
             pconn_list = &NetConn_ServerConnListHead[protocol_ix];
             break;


        case NET_CONN_LIST_TYPE_CLIENT:
             pconn_list = &NetConn_ClientConnListHead[protocol_ix];
             break;


        case NET_CONN_LIST_TYPE_NONE:
        default:
             NET_CTR_ERR_INC(NetConn_ErrInvalidListTypeCtr);
            *perr = NET_CONN_ERR_INVALID_LIST_TYPE;
             return;                                            /* Prevent 'break NOT reachable' compiler warning.      */
    }


                                                                /* ---------- GET NET CONN'S CONN LIST CHAIN ---------- */
    if (pconn->AddrRemoteValid == DEF_YES) {                    /* If remote addr avail, ...                            */
                                                                /* ... srch conn list for conn chain (see Note #4).     */
        pconn_chain = NetConn_ListSrch((NET_CONN        **) pconn_list,
                                       (NET_CONN_LIST_TYPE) conn_list_type,
                                       (CPU_BOOLEAN       ) DEF_NO,
                                       (CPU_INT08U       *)&pconn->AddrLocal[0],
                                       (CPU_INT08U       *)&pconn->AddrRemote[0]);

        if (pconn_chain == (NET_CONN *)0) {                     /* If specified local addr conn chain NOT found, ...    */
            if (NetConn_AddrWildCardAvail == DEF_YES) {         /* ... srch conn list with wildcard addr.               */
                pconn_chain = NetConn_ListSrch((NET_CONN        **) pconn_list,
                                               (NET_CONN_LIST_TYPE) conn_list_type,
                                               (CPU_BOOLEAN       ) DEF_NO,
                                               (CPU_INT08U       *)&addr_wildcard[0],
                                               (CPU_INT08U       *)&pconn->AddrRemote[0]);
            }
        }
        
    } else {
        pconn_chain = (NET_CONN *)0;
    }


                                                                /* --------- ADD NET CONN INTO NET CONN LIST ---------- */
    NetConn_Add(pconn_list,
                pconn_chain,
                pconn,
                conn_list_type);


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NetConn_ListUnlink()
*
* Description : Unlink a network connection from a connection list.
*
* Argument(s) : conn_id     Handle identifier of network connection to unlink.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_CONN_ERR_NONE               Network connection successfully unlinked
*                                                                   from connection list.
*
*                                                               ---- RETURNED BY NetConn_IsUsed() : ----
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*                               NET_CONN_ERR_INVALID_CONN       Invalid network connection number.
*                               NET_CONN_ERR_NOT_USED           Network connection NOT currently used.
*
* Return(s)   : none.
*
* Caller(s)   : NetSock_BindHandler(),
*               NetSock_ConnHandlerDatagram(),
*               NetSock_ConnHandlerStream().
*
*               This function is an INTERNAL network protocol suite function & MUST NOT be called by 
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetConn_ListUnlink (NET_CONN_ID   conn_id,
                          NET_ERR      *perr)
{
    NET_CONN  *pconn;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE NET CONN USED -------------- */
   (void)NetConn_IsUsed(conn_id, perr);
    if (*perr != NET_CONN_ERR_NONE) {
         return;
    }
#endif

                                                                /* -------- UNLINK NET CONN FROM NET CONN LIST -------- */
    pconn = &NetConn_Tbl[conn_id];
    NetConn_Unlink(pconn);


   *perr = NET_CONN_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           NetConn_Close()
*
* Description : (1) Close a network connection :
*
*                   (a) Update network connection close statistic(s)
*
*                   (b) Close  network connection(s) :
*                       (1) Close application connection
*                       (2) Close transport   connection
*
*                   (c) Free   network connection, if necessary
*
*
* Argument(s) : pconn       Pointer to a network connection.
*               -----       Argument checked in NetConn_CloseAllConnListConns(),
*                                               NetConn_ListValidateConn().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_CloseAllConnListConns(),
*               NetConn_ListValidateConn().
*
* Note(s)     : (2) Network connection's handle identifier MUST be obtained PRIOR to any network 
*                   connection validation &/or close operations.
*
*               (3) (a) Network connection(s) MAY  already be closed BEFORE any  network connection 
*                       close operations & MUST be validated as used BEFORE each network connection 
*                       close operation.
*
*                   (b) Network connection SHOULD already be closed AFTER application & transport 
*                       layer connections have both closed.
*
*                       See also 'NetConn_CloseFromApp()        Note #1c'
*                              & 'NetConn_CloseFromTransport()  Note #1c'.
*********************************************************************************************************
*/

static  void  NetConn_Close (NET_CONN  *pconn)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR       cpu_sr;
#endif
    NET_CONN_ID  conn_id;
    NET_ERR      err;
    CPU_BOOLEAN  used;

                                                                /* --------------- UPDATE CLOSE STATS ----------------- */
    NET_CTR_ERR_INC(NetConn_ErrCloseCtr);

                                                                /* ------------------ CLOSE CONN(S) ------------------- */
    conn_id = pconn->ID;                                        /* Get net conn id (see Note #2).                       */

    used = NetConn_IsUsed(conn_id, &err);
    if (used == DEF_YES) {                                      /* If net conn used (see Note #3a), ...                 */
        NetConn_CloseApp(pconn);                                /* ... close app conn.                                  */
    }

    used = NetConn_IsUsed(conn_id, &err);
    if (used == DEF_YES) {                                      /* If net conn used (see Note #3a), ...                 */
        NetConn_CloseTransport(pconn);                          /* ... close transport conn.                            */
    }


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ------------------ FREE NET CONN ------------------- */
    used = NetConn_IsUsed(conn_id, &err);
    if (used == DEF_YES) {                                      /* If net conn used (see Note #3b), ...                 */
        NetConn_FreeHandler(pconn);                             /* ... free net conn.                                   */
    }
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         NetConn_CloseApp()
*
* Description : (1) Close a network connection's application connection :
*
*                   (a) Free  network     connection from application clone connection, if necessary
*                   (b) Close application connection
*
*
* Argument(s) : pconn       Pointer to a network connection.
*               -----       Argument validated in NetConn_Close(),
*                                                 NetConn_CloseFromTransport().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_Close(),
*               NetConn_CloseFromTransport().
*
* Note(s)     : (2) The 'NET_CONN_CFG_FAMILY' pre-processor 'else'-conditional code will never be compiled/linked
*                   since 'net_conn.h' ensures that the family type configuration constant (NET_CONN_CFG_FAMILY)
*                   is configured with an appropriate family type value (see 'net_conn.h  CONFIGURATION ERRORS').
*                   The 'else'-conditional code is included for completeness & as an extra precaution in case
*                   'net_conn.h' is incorrectly modified.
*********************************************************************************************************
*/

static  void  NetConn_CloseApp (NET_CONN  *pconn)
{
                                                                /* ---------------- CLOSE APP CONN(S) ----------------- */
#if (NET_CONN_CFG_FAMILY == NET_CONN_FAMILY_IP_V4_SOCK)         
    NetSock_FreeConnFromSock((NET_SOCK_ID)pconn->ID_AppClone,   /* Free net conn from clone sock conn (see Note #1a).   */
                             (NET_CONN_ID)pconn->ID);    

    NetSock_CloseFromConn((NET_SOCK_ID)pconn->ID_App);          /* Close sock app conn (see Note #1b).                  */

#else                                                           /* See Note #2.                                         */
   (void)&pconn;                                                /* Prevent compiler warning.                            */
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                      NetConn_CloseTransport()
*
* Description : Close a network connection's transport connection.
*
* Argument(s) : pconn       Pointer to a network connection.
*               -----       Argument validated in NetConn_Close(),
*                                                 NetConn_CloseFromApp().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_Close(),
*               NetConn_CloseFromApp().
*
* Note(s)     : (1) The 'NET_CONN_CFG_FAMILY' pre-processor 'else'-conditional code will never be compiled/linked
*                   since 'net_conn.h' ensures that the family type configuration constant (NET_CONN_CFG_FAMILY)
*                   is configured with an appropriate family type value (see 'net_conn.h  CONFIGURATION ERRORS').
*                   The 'else'-conditional code is included for completeness & as an extra precaution in case
*                   'net_conn.h' is incorrectly modified.
*********************************************************************************************************
*/

static  void  NetConn_CloseTransport (NET_CONN  *pconn)
{
                                                                /* --------------- CLOSE TRANSPORT CONN -------------- */
#if    (NET_CONN_CFG_FAMILY == NET_CONN_FAMILY_IP_V4_SOCK)
#ifdef  NET_TCP_MODULE_PRESENT
    NetTCP_ConnCloseFromConn((NET_TCP_CONN_ID)pconn->ID_Transport);
#endif

#else                                                           /* See Note #1.                                         */
   (void)&pconn;                                                /* Prevent compiler warning.                            */
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                   NetConn_CloseAllConnListConns()
*
* Description : Close ALL network connections in a network connection list.
*
* Argument(s) : pconn_list      Pointer to a network connection list.
*               ----------      Argument validated in NetConn_CloseAllConnsHandler().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_CloseAllConnsHandler().
*
* Note(s)     : (1) Invalid network connection(s) in the connection list MAY already be closed in other
*                   validation functions.
*
*                   See also 'NetConn_ListValidateConn()  Note #3'.
*
*               (2) Since network connection close handlers execute asynchronously to NetConn_FreeHandler(),
*                   network connection list pointers ('NetConn_ConnListChainPtr', 'NetConn_ConnListConnPtr',
*                   etc.) MUST be coordinated with NetConn_FreeHandler() to avoid network connection list
*                   corruption :
*
*                   (a) (1) Network connection list pointers typically advance to the next network connection
*                           in a network connection list.
*
*                       (2) However, whenever a network connection list pointer connection is freed by an
*                           asynchronous network connection close, the network connection list pointer(s)
*                           MUST be advanced to the next valid & available network connection in the
*                           network connection list.
*
*                           See also 'NetConn_FreeHandler()  Note #2a'.
*
*                   (b) Network connection list pointers MUST be cleared after handling the network
*                       connection list.
*
*                       (1) However, network connection list pointers are implicitly cleared after 
*                           handling the network connection list.
*********************************************************************************************************
*/

static  void  NetConn_CloseAllConnListConns (NET_CONN  **pconn_list)
{
    NetConn_ConnListChainPtr = (NET_CONN *)*pconn_list;                 /* Start close @ conn list head.                */

    while (NetConn_ConnListChainPtr != (NET_CONN *)0) {                 /* Close ALL net conn chains ...                */
        NetConn_ConnListNextChainPtr = (NET_CONN *)NetConn_ConnListChainPtr->NextChainPtr;
        NetConn_ConnListConnPtr      = (NET_CONN *)NetConn_ConnListChainPtr;

        while (NetConn_ConnListConnPtr != (NET_CONN *)0) {              /* ... & ALL net conns.                         */
            NetConn_ConnListNextConnPtr = (NET_CONN *)NetConn_ConnListConnPtr->NextConnPtr;

            NetConn_Close(NetConn_ConnListConnPtr);                     /* Close net conn.                              */

            NetConn_ConnListConnPtr = NetConn_ConnListNextConnPtr;      /* Adv to next net conn       (see Note #2a1).  */
        }

        NetConn_ConnListChainPtr    = NetConn_ConnListNextChainPtr;     /* Adv to next net conn chain (see Note #2a1).  */
    }


#if  (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                        /* Clr net conn list ptrs (see Note #2b1).      */
    NetConn_ConnListChainPtr     = (NET_CONN *)0;
    NetConn_ConnListConnPtr      = (NET_CONN *)0;
    NetConn_ConnListNextChainPtr = (NET_CONN *)0;
    NetConn_ConnListNextConnPtr  = (NET_CONN *)0;
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NetConn_FreeHandler()
*
* Description : (1) Free a network connection :
*
*                   (a) Remove network connection from a   network connection list :
*                       (1) Update network connection list pointer(s)                       See Note #2
*                       (2) Unlink network connection from network connection list
*
*                   (b) Clear  network connection values
*                   (c) Free   network connection back to network connection pool
*                   (d) Update network connection pool statistics
*
*
* Argument(s) : pconn       Pointer to a network connection.
*               -----       Argument validated in NetConn_Free(),
*                                                 NetConn_Close(),
*                                                 NetConn_CloseFromApp(),
*                                                 NetConn_CloseFromTransport(),
*                                                 NetConn_ListValidateConn().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_Free(),
*               NetConn_Close(),
*               NetConn_CloseFromApp(),
*               NetConn_CloseFromTransport(),
*               NetConn_ListValidateConn().
*
* Note(s)     : (2) Since network connection close handlers execute asynchronously to NetConn_FreeHandler(),
*                   network connection list pointers ('NetConn_ConnListChainPtr', 'NetConn_ConnListConnPtr',
*                   etc.) MUST be coordinated with NetConn_FreeHandler() to avoid network connection list
*                   corruption :
*
*                   (a) Whenever a network connection list pointer connection is freed, network connection
*                       list pointers MUST be advanced to the next valid & available network connection in
*                       the appropriate network connection list.
*
*                   See also 'NetConn_CloseAllConnListConns()  Note #2'.
*********************************************************************************************************
*/

static  void  NetConn_FreeHandler (NET_CONN  *pconn)
{
#if ((NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                    && \
     (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD    == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR     cpu_sr;
#endif
    NET_CONN  *pnext;
    NET_ERR    err;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* ------------------ VALIDATE TYPE ------------------- */
    if (pconn->Type != NET_CONN_TYPE_CONN) {
        NetConn_Discard(pconn);
        NET_CTR_ERR_INC(NetConn_ErrInvalidTypeCtr);
        return;
    }
#endif
                                                                /* -------- REMOVE NET CONN FROM NET CONN LIST -------- */
                                                                /* If net conn is next conn list conn to update, ...    */
                                                                /* ... adv to skip this net conn (see Note #2a).        */
    if (pconn == NetConn_ConnListNextChainPtr) {
        pnext                        = (NET_CONN *)NetConn_ConnListNextChainPtr->NextChainPtr;
        NetConn_ConnListNextChainPtr = (NET_CONN *)pnext;
    }
    if (pconn == NetConn_ConnListNextConnPtr) {
        pnext                        = (NET_CONN *)NetConn_ConnListNextConnPtr->NextConnPtr;
        NetConn_ConnListNextConnPtr  = (NET_CONN *)pnext;
    }

    NetConn_Unlink(pconn);                                      /* Unlink net conn from net conn list.                  */


                                                                /* ------------------- CLR NET CONN ------------------- */
    DEF_BIT_CLR(pconn->Flags, NET_CONN_FLAG_USED);              /* Set net conn as NOT used.                            */
#if (NET_DBG_CFG_MEM_CLR_EN == DEF_ENABLED)
    NetConn_Clr(pconn);
#endif

                                                                /* ------------------ FREE NET CONN ------------------- */
    pconn->NextConnPtr = (void     *)NetConn_PoolPtr;
    NetConn_PoolPtr    = (NET_CONN *)pconn;

                                                                /* ------------ UPDATE NET CONN POOL STATS ------------ */
    NetStat_PoolEntryUsedDec(&NetConn_PoolStat, &err);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            NetConn_Clr()
*
* Description : Clear network connection controls.
*
* Argument(s) : pconn       Pointer to a network connection.
*               -----       Argument validated in NetConn_Init(),
*                                                 NetConn_Get(),
*                                                 NetConn_FreeHandler().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_Init(),
*               NetConn_Get(),
*               NetConn_FreeHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  NetConn_Clr (NET_CONN  *pconn)
{
    pconn->PrevChainPtr         = (void  *)0;
    pconn->NextChainPtr         = (void  *)0;
    pconn->PrevConnPtr          = (void  *)0;
    pconn->NextConnPtr          = (void  *)0;
    pconn->ConnList             = (void **)0;
    pconn->Family               =  NET_CONN_FAMILY_NONE;
    pconn->ProtocolListIx       =  NET_CONN_LIST_IX_NONE;
    pconn->ConnListType         =  NET_CONN_LIST_TYPE_NONE;
    pconn->ID_App               =  NET_CONN_ID_NONE;
    pconn->ID_AppClone          =  NET_CONN_ID_NONE;
    pconn->ID_Transport         =  NET_CONN_ID_NONE;
    pconn->AddrLocalValid       =  DEF_NO;
    pconn->AddrRemoteValid      =  DEF_NO;
    pconn->ConnChainAccessedCtr =  0;
    pconn->ConnAccessedCtr      =  0;
    pconn->Flags                =  NET_CONN_FLAG_NONE;

    Mem_Clr((void     *)&pconn->AddrLocal[0],
            (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);
    Mem_Clr((void     *)&pconn->AddrRemote[0],
            (CPU_SIZE_T) NET_CONN_CFG_ADDR_LEN);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          NetConn_Discard()
*
* Description : (1) Discard an invalid/corrupted network  connection :
*
*                   (a) Discard network connection from available network connection pool       See Note #2
*                   (b) Update  network connection pool statistics
*
*               (2) Assumes network connection is invalid/corrupt & MUST be removed.  Network 
*                   connection removed simply by NOT returning the network connection back to
*                   the network connection pool.
*
*
* Argument(s) : pconn       Pointer to an invalid/corrupted network connection.
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_Get(),
*               NetConn_FreeHandler(),
*               NetConn_ListValidateConn().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
static  void  NetConn_Discard (NET_CONN  *pconn)
{
    NET_ERR  stat_err;

                                                                /* ----------------- DISCARD NET CONN ----------------- */
   (void)&pconn;                                                /* Prevent compiler warning (see Note #2).              */

                                                                /* --------------- UPDATE DISCARD STATS --------------- */
    NetStat_PoolEntryLostInc(&NetConn_PoolStat, &stat_err);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         NetConn_ListSrch()
*
* Description : Search a network connection list for network connection with specific local & remote addresses.
*
*               (1) Connections are organized into connection chains to expedite connection searches.
*
*                   (a) For each connection family & protocol type, two connection lists are maintained :
*                       
*                       (1) A server connection list organized primarily by each connection's local  address
*                       (2) A client connection list organized primarily by each connection's remote address
*                       
*                   (b) (1) Each connection list is maintained as a list of connection lists.  Each connection 
*                           list is uniquely organized by one of the connection addresses -- either the local 
*                           or the remote address depending on the connection list type (see Note #1a).
*
*                           In other words, a connection list's top-level connections are each the heads of 
*                           their own connection list & each have a primary address (either local or remote) 
*                           that is unique from all the other connection lists.
*
*                       (2) Each connection list maintained within a connection list is also referred to as
*                           a connection chain.
*
*                   (c) Each connection chain is an organization of connections that all share the same 
*                       primary connection address [either local or remote (see Note #1a)] but different 
*                       secondary addresses.
*
*                       In other words, each connection in a connection chain shares the connection chain's 
*                       primary connection address but has its own unique secondary connection address.
*
*                   (d) Therefore, connections in a connection chain may share the same local OR remote
*                       connection addresses; however, no two connections in a connection chain share
*                       the same local AND remote connection address.
*
*                   (e) In the diagram below, ...
*
*                       (1) The top horizontal row  represents the connection list's list of connection chains.
*
*                       (2) Each    vertical column represents the connections in unique     connection chains.
*
*                       (3) Each connection list is a pointer that points to the head of the connection list.
*
*                       (4) Connections' 'PrevChainPtr' & 'NextChainPtr' doubly-link each connection chain to 
*                           form the list of connection chains.
*
*                       (5) Connections' 'PrevConnPtr'  & 'NextConnPtr'  doubly-link each connection to form a
*                           connection chain.
*
*                   (f) For any connection search, each connection list is searched in order by the primary
*                       address (either local or remote) to find the appropriate connection chain.  If a
*                       connection chain with the primary search address is found, then each connection in
*                       the connection chain is searched in order to find the connection with the secondary
*                       search address.
*
*                   (g) To expedite faster connection searches for recently added network connections :
*
*                       (1) New network connection chains are added at the head of a network connection list.
*
*                       (2) New network connections are added at or immediately following the head of a network 
*                           connection chain :
*
*                           (A) New connections in a server connection list are added ...
*
*                               (1) As the second connection immediately following the connection chain head,
*                                   if the connection chain head connection is an unconnected server connection
*                                   (i.e. no remote connection address is available).
*
*                               (2) At the head of a connection chain, if the current connection chain head is 
*                                   NOT an unconnected server connection.
*
*                           (B) New network connections in a client connection list are added at the head of a 
*                               network connection chain.
*
*                       See also 'NetConn_Add()  Note #1'.
*$PAGE*
*
*                                            |                                                             |
*                                            |<---------------- List of Connection Chains ---------------->|
*                                            |                      (see Note #1e1)                        |
*
*                                New connection chains inserted
*                                  at head of connection list
*                                       (see Note #1g1)
*
*                                               |              NextChainPtr
*                                               |             (see Note #1e4)
*                                               v                    |
*                                                                    |
*                           Head of          -------       -------   v   -------       -------       -------
*                       Connection List ---->|     |------>|     |------>|     |------>|     |------>|     |
*                                            |     |       |     |       |     |       |     |       |     |
*                       (see Note #1e3)      |     |<------|     |<------|     |<------|     |<------|     |
*                                            -------       -------   ^   -------       -------       -------
*                                              | ^                   |                   | ^
*                                              | |                   |                   | |
*                                              v |             PrevChainPtr              v |
*               ---                          -------          (see Note #1e4)          -------
*                ^                           |     |                                   |     |
*                |                           |     |                                   |     |
*                |                           |     |                                   |     |
*                |                           -------                                   -------
*                |                             | ^                                       | ^
*                |           NextConnPtr ----> | |                                       | | <----  PrevConnPtr
*                |         (see Note #1e5)     v |                                       v |      (see Note #1e5)
*                                            -------                                   -------
*      Connections organized                 |     |                                   |     |
*     into a connection chain                |     |                                   |     |
*         (see Note #1e2)                    |     |                                   |     |
*                                            -------                                   -------
*                |                             | ^
*                |                             | |
*                |                             v |
*                |                           -------
*                |                           |     |
*                |                           |     |
*                v                           |     |
*               ---                          -------
*
*
* Argument(s) : pconn_list              Pointer to a connection list.
*               ----------              Argument validated in NetConn_Srch().
*
*               conn_list_type          Network connection list type :
*
*                                           NET_CONN_LIST_TYPE_SERVER       Connection list organized primarily
*                                                                               by local  address.
*                                           NET_CONN_LIST_TYPE_CLIENT       Connection list organized primarily
*                                                                               by remote address.
*
*               sec_srch_addr_reqd      Indicate whether secondary search address is required.
*
*               paddr_local             Pointer to local  address (see Note #2).
*
*               paddr_remote            Pointer to remote address (see Note #2).
*
* Return(s)   : Pointer to connection with specific local & remote address, if found.
*
*               Pointer to NULL,                                            otherwise.
*
* Caller(s)   : NetConn_Srch().
*
* Note(s)     : (2) Network connection addresses maintained in network-order.
*
*               (3) (a) Assumes ALL connections' local   address is valid.
*
*                   (b) Assumes ALL connections' primary address is valid; i.e. for connections on server
*                       connection lists, the connections' remote addresses are assumed to be valid.
*
*               (4) Default case already invalidated in NetConn_Srch().  However, the default case is
*                   included as an extra precaution in case 'conn_list_type' is corrupted.
*********************************************************************************************************
*/
/*$PAGE*/
static  NET_CONN  *NetConn_ListSrch (NET_CONN            **pconn_list,
                                     NET_CONN_LIST_TYPE    conn_list_type,
                                     CPU_BOOLEAN           sec_srch_addr_reqd,
                                     CPU_INT08U           *paddr_local,
                                     CPU_INT08U           *paddr_remote)
{
#if ((NET_CTR_CFG_ERR_EN      == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR        cpu_sr;
#endif
    NET_CONN     *pconn;
    NET_CONN     *pconn_next;
    NET_CONN     *pconn_chain;
    NET_CONN     *pconn_chain_next;
    CPU_INT08U   *pconn_addr_local;
    CPU_INT08U   *pconn_addr_remote;
    CPU_BOOLEAN   found;
    CPU_BOOLEAN   found_prim_addr;


#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                 /* -------------- VALIDATE SRCH REQ OPT --------------- */
    switch (sec_srch_addr_reqd) {
        case DEF_NO:
        case DEF_YES:
             break;


        default:
             sec_srch_addr_reqd = DEF_YES;
             break;
    }
#endif

                                                                /* ---------- VALIDATE CONN LIST TYPE / PTRS ---------- */
    switch (conn_list_type) {
        case NET_CONN_LIST_TYPE_SERVER:
             if (paddr_local == (CPU_INT08U *)0) {
                 return ((NET_CONN *)0);
             }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
             if (sec_srch_addr_reqd == DEF_YES) {               /* If conn sec  addr req'd,   ...                       */
                 if (paddr_remote == (CPU_INT08U *)0) {         /* ... & remote addr ptr NULL ...                       */
                     NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
                     return ((NET_CONN *)0);                    /* ... rtn NULL.                                        */
                 }
             }
#endif
             break;


        case NET_CONN_LIST_TYPE_CLIENT:
             if (paddr_remote == (CPU_INT08U *)0) {
                 return ((NET_CONN *)0);
             }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)
             if (sec_srch_addr_reqd == DEF_YES) {               /* If conn sec  addr req'd,   ...                       */
                 if (paddr_local == (CPU_INT08U *)0) {          /* ... & local  addr ptr NULL ...                       */
                     NET_CTR_ERR_INC(NetConn_ErrNullPtrCtr);
                     return ((NET_CONN *)0);                    /* ... rtn NULL.                                        */
                 }
             }
#endif
             break;


        case NET_CONN_LIST_TYPE_NONE:
        default:
             NET_CTR_ERR_INC(NetConn_ErrInvalidListTypeCtr);
             return ((NET_CONN *)0);                            /* Prevent 'break NOT reachable' compiler warning.      */
    }


/*$PAGE*/
    pconn_chain = (NET_CONN *)*pconn_list;                                  /* Start @ conn list head.                  */
    pconn       = (NET_CONN *) 0;
    found       =  DEF_NO;

    while ((pconn_chain != (NET_CONN *)0) &&                                /* Srch ALL net conn chains ..              */
           (found       ==  DEF_NO)) {                                      /* .. until net conn found.                 */

        pconn_chain_next = (NET_CONN *)pconn_chain->NextChainPtr;

        switch (conn_list_type) {
            case NET_CONN_LIST_TYPE_SERVER:
                                                                            /* Cmp conn local addr (see Note #1a1).     */
                 pconn_addr_local = (CPU_INT08U *)&pconn_chain->AddrLocal[0];
                 found_prim_addr  =  Mem_Cmp((void     *)paddr_local,
                                             (void     *)pconn_addr_local,
                                             (CPU_SIZE_T)NET_CONN_CFG_ADDR_LEN);

                 if (found_prim_addr == DEF_YES) {                          /* If local addr found,       ...           */

                     pconn = pconn_chain;

                     while ((pconn != (NET_CONN *)0) &&                     /* ... srch server conn chain ...           */
                            (found ==  DEF_NO)) {                           /* ... until conn found.                    */

                         pconn_next = (NET_CONN *)pconn->NextConnPtr;

                         if (sec_srch_addr_reqd == DEF_YES) {               /* If conn sec addr req'd, ...              */
                                                                            /* ... cmp conn remote addr.                */
                             if (pconn->AddrRemoteValid == DEF_YES) {
                                 pconn_addr_remote = (CPU_INT08U *)&pconn->AddrRemote[0];
                                 found             =  Mem_Cmp((void     *)paddr_remote,
                                                              (void     *)pconn_addr_remote,
                                                              (CPU_SIZE_T)NET_CONN_CFG_ADDR_LEN);
                             }

                         } else if (pconn->AddrRemoteValid == DEF_NO) {     /* Else if conn sec addr NOT avail, ...     */
                             found = DEF_YES;                               /* ... rtn conn.                            */
                         }

                         if (found != DEF_YES) {                            /* If NOT found, adv to next conn.          */
                             pconn  = pconn_next;
                         }
                     }
                 }
                 break;

/*$PAGE*/
            case NET_CONN_LIST_TYPE_CLIENT:
                                                                            /* Cmp conn remote addr (see Note #1a2).    */
                 pconn_addr_remote = (CPU_INT08U *)&pconn_chain->AddrRemote[0];
                 found_prim_addr   =  Mem_Cmp((void     *)paddr_remote,
                                              (void     *)pconn_addr_remote,
                                              (CPU_SIZE_T)NET_CONN_CFG_ADDR_LEN);

                 if (found_prim_addr == DEF_YES) {                          /* If remote addr found,      ...           */

                     pconn = pconn_chain;

                     while ((pconn != (NET_CONN *)0) &&                     /* ... srch client conn chain ...           */
                            (found ==  DEF_NO)) {                           /* ... until conn found.                    */

                         pconn_next = (NET_CONN *)pconn->NextConnPtr;

                         if (pconn->AddrLocalValid  == DEF_YES) {           /* If conn's local addr valid & ...         */
                             if (sec_srch_addr_reqd == DEF_YES) {           /* .. conn sec addr req'd,      ...         */
                                                                            /* .. cmp conn local addr.                  */
                                 pconn_addr_local   = (CPU_INT08U *)&pconn->AddrLocal[0];
                                 found              =  Mem_Cmp((void     *)paddr_local,
                                                               (void     *)pconn_addr_local,
                                                               (CPU_SIZE_T)NET_CONN_CFG_ADDR_LEN);
                             } else {                                       /* Else if conn sec addr NOT req'd, ...     */
                                 found              =  DEF_YES;             /* ... rtn conn.                            */
                             }
                         } else {                                           /* Else conn's local addr NOT valid, ...    */
                             NetConn_Unlink(pconn);                         /* ... unlink conn from conn list    ...    */
                                                                            /* ... (see Notes #3a & #3c).               */
                         }

                         if (found != DEF_YES) {                            /* If NOT found, adv to next conn.          */
                             pconn  = pconn_next;
                         }
                     }
                 }
                 break;


            case NET_CONN_LIST_TYPE_NONE:
            default:                                                        /* See Note #4.                             */
                 NET_CTR_ERR_INC(NetConn_ErrInvalidListTypeCtr);
                 return ((NET_CONN *)0);                                    /* Prevent 'break NOT reachable' warning.   */
        }

        if (found != DEF_YES) {                                             /* If net conn NOT found, ...               */
            pconn_chain = pconn_chain_next;                                 /* ... adv to next net conn chain.          */
        }
    }


    if (found == DEF_YES) {                                                 /* If net conn found, inc conn access ctrs. */
        pconn_chain->ConnChainAccessedCtr++;
        if (pconn_chain->ConnChainAccessedCtr > NetConn_AccessedTh_nbr) {   /* If conn chain accessed > th           .. */
            pconn_chain->ConnChainAccessedCtr = 0;
            if (pconn_chain != *pconn_list) {                               /* .. & conn chain NOT    @ conn list head, */
                                                                            /* .. promote conn chain to conn list head. */
                NetConn_ChainUnlink(pconn_list, pconn_chain);
                NetConn_ChainInsert(pconn_list, pconn_chain);                        
            }
        }

        pconn->ConnAccessedCtr++;
        if (pconn->ConnAccessedCtr > NetConn_AccessedTh_nbr) {              /* If conn accessed > th               ..   */
            pconn->ConnAccessedCtr = 0;
            if (pconn != pconn_chain) {                                     /* .. & conn  NOT   @ conn chain head, ..   */
                                                                            /* .. promote conn to conn chain head.      */
                NetConn_Unlink(pconn);
                NetConn_Add(pconn_list, pconn_chain, pconn, conn_list_type);
            }
        }
    }


    return (pconn);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NetConn_ChainInsert()
*
* Description : (1) Insert a connection chain into a connection list :
*
*                   (a) Insert connection chain at the head of the connection list
*                   (b) Set each chain connection's connection list
*
*
* Argument(s) : pconn_list      Pointer to a connection list.
*               ----------      Argument validated in NetConn_ListSrch().
*
*               pconn_chain     Pointer to a connection chain.
*               -----------     Argument validated in NetConn_ListSrch().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_ListSrch().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  NetConn_ChainInsert (NET_CONN  **pconn_list,
                                   NET_CONN   *pconn_chain)
{
    NET_CONN  *pconn;
    NET_CONN  *pconn_next;

                                                                /* ------- INSERT CONN CHAIN AT CONN LIST HEAD -------- */
    pconn_chain->PrevChainPtr = (void *)0;
    pconn_chain->NextChainPtr = (void *)(*pconn_list);

    if (*pconn_list != (NET_CONN *)0) {                         /* If conn list NOT empty,                         ...  */
       (*pconn_list)->PrevChainPtr = (void *)pconn_chain;       /* ... insert conn chain before cur conn list head ...  */
    }
   *pconn_list = pconn_chain;                                   /*     Set    conn chain as     new conn list head.     */

                                                                /* ------------ SET CHAIN CONNS' CONN LIST ------------ */
    pconn = pconn_chain;
    while (pconn  != (NET_CONN *)0) {
        pconn->ConnList = (void    **)pconn_list;
        pconn_next      = (NET_CONN *)pconn->NextConnPtr;
        pconn           =  pconn_next;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        NetConn_ChainUnlink()
*
* Description : (1) Unlink a connection chain from a connection list :
*
*                   (a) Unlink connection chain
*                   (b) Clear each chain connection's connection list
*
*
* Argument(s) : pconn_list      Pointer to a connection list.
*               ----------      Argument validated in NetConn_ListSrch().
*
*               pconn_chain     Pointer to a connection chain.
*               -----------     Argument validated in NetConn_ListSrch().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_ListSrch().
*
* Note(s)     : (2) Since NetConn_ChainUnlink() called ONLY to remove & then re-link connection chains,
*                   it is NOT necessary to clear the entry's previous & next chain pointers.  However,
*                   pointers cleared to NULL shown for correctness & completeness.
*********************************************************************************************************
*/

static  void  NetConn_ChainUnlink (NET_CONN  **pconn_list,
                                   NET_CONN   *pconn_chain)
{
    NET_CONN  *pconn_chain_prev;
    NET_CONN  *pconn_chain_next;
    NET_CONN  *pconn;
    NET_CONN  *pconn_next;

                                                                /* --------- UNLINK CONN CHAIN FROM CONN LIST --------- */
    pconn_chain_prev = (NET_CONN *)pconn_chain->PrevChainPtr;
    pconn_chain_next = (NET_CONN *)pconn_chain->NextChainPtr;
                                                                /* Point prev conn chain to next conn chain.            */
    if (pconn_chain_prev != (NET_CONN *)0) {
        pconn_chain_prev->NextChainPtr = (void     *)pconn_chain_next;
    } else {
       *pconn_list                     = (NET_CONN *)pconn_chain_next;
    }
                                                                /* Point next conn chain to prev conn chain.            */
    if (pconn_chain_next != (NET_CONN *)0) {
        pconn_chain_next->PrevChainPtr = (void     *)pconn_chain_prev;
    }

#if (NET_DBG_CFG_MEM_CLR_EN == DEF_ENABLED)                     /* Clr conn chain's chain ptrs (see Note #2).           */
    pconn_chain->PrevChainPtr = (void *)0;
    pconn_chain->NextChainPtr = (void *)0;
#endif

                                                                /* ------------ CLR CHAIN CONNS' CONN LIST ------------ */
    pconn = pconn_chain;
    while (pconn  != (NET_CONN *)0) {
        pconn->ConnList = (void    **)0;
        pconn_next      = (NET_CONN *)pconn->NextConnPtr;
        pconn           =  pconn_next;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                            NetConn_Add()
*
* Description : (1) Add a network connection into a network connection list :
*
*                   (a) New network connection chains are added at the head of a network connection list.
*
*                   (b) New network connections are added at or immediately following the head of a network 
*                       connection chain :
*
*                       (1) New network connections in a server connection list are added ...
*
*                           (A) As the second connection immediately following the connection chain head,
*                               if the connection chain head connection is an unconnected server connection
*                               (i.e. no remote connection address is available).
*
*                           (B) At the head of a connection chain, if the current connection chain head is 
*                               NOT an unconnected server connection.
*
*                       (2) New network connections in a client connection list are added at the head of a 
*                           network connection chain.
*
*
* Argument(s) : pconn_list          Pointer to a network connection list.
*               ----------          Argument validated in NetConn_ListAdd(),
*                                                         NetConn_ListSrch().
*
*               pconn_chain         Pointer to a network connection chain.
*
*               pconn               Pointer to a network connection.
*               -----               Argument validated in NetConn_ListAdd(),
*                                                         NetConn_ListSrch().
*
*               conn_list_type      Network connection list type :
*
*                                       NET_CONN_LIST_TYPE_SERVER       Connection list organized primarily
*                                                                           by local  address.
*                                       NET_CONN_LIST_TYPE_CLIENT       Connection list organized primarily
*                                                                           by remote address.
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_ListAdd(),
*               NetConn_ListSrch().
*
* Note(s)     : none.  
*********************************************************************************************************
*/
/*$PAGE*/
static  void  NetConn_Add (NET_CONN            **pconn_list,
                           NET_CONN             *pconn_chain,
                           NET_CONN             *pconn,
                           NET_CONN_LIST_TYPE    conn_list_type)
{
    CPU_BOOLEAN   insert_conn_list_head;
    NET_CONN     *pconn_next;
    NET_CONN     *pconn_chain_prev;
    NET_CONN     *pconn_chain_next;


    if (pconn_chain == (NET_CONN *)0) {                         /* If conn chain empty, ...                             */
                                                                /* ... insert net conn as new conn chain (see Note #1a).*/
        pconn->PrevChainPtr = (void *)0;
        pconn->NextChainPtr = (void *)(*pconn_list);
        pconn->PrevConnPtr  = (void *)0;
        pconn->NextConnPtr  = (void *)0;

        if (*pconn_list != (NET_CONN *)0) {                     /* If conn list NOT empty,                   ...        */
           (*pconn_list)->PrevChainPtr = (void *)pconn;         /* ... insert conn before cur conn list head ...        */
        }
       *pconn_list = pconn;                                     /*     Set    conn as     new conn list head.           */

    } else {                                                    /* Else add net conn into existing conn chain.          */

        insert_conn_list_head = ((conn_list_type               == NET_CONN_LIST_TYPE_SERVER) &&
                                 (pconn_chain->AddrRemoteValid != DEF_YES)) ? DEF_NO : DEF_YES;

        if (insert_conn_list_head != DEF_NO) {                  /* If conn chain head NOT an unconnected server conn,   */
                                                                /* ... add net conn as new conn chain head          ... */
                                                                /* ... (see Notes #1b1B & #1b2).                        */
            pconn_chain_prev          = (NET_CONN *)pconn_chain->PrevChainPtr;
            pconn_chain_next          = (NET_CONN *)pconn_chain->NextChainPtr;
            pconn_chain->PrevChainPtr = (void     *)0;
            pconn_chain->NextChainPtr = (void     *)0;
            pconn_chain->PrevConnPtr  = (void     *)pconn;

            pconn->PrevConnPtr        = (void     *)0;
            pconn->NextConnPtr        = (void     *)pconn_chain;

            pconn->PrevChainPtr       = (void     *)pconn_chain_prev;
            if (pconn_chain_prev != (NET_CONN *)0) {
                pconn_chain_prev->NextChainPtr = (void     *)pconn;
            } else {
               *pconn_list                     = (NET_CONN *)pconn;
            }

            pconn->NextChainPtr   = (void     *)pconn_chain_next;
            if (pconn_chain_next != (NET_CONN *)0) {
                pconn_chain_next->PrevChainPtr = (void     *)pconn;
            }
                                                                /* Inherit conn chain accessed ctr.                     */
            pconn->ConnChainAccessedCtr       = pconn_chain->ConnChainAccessedCtr;
            pconn_chain->ConnChainAccessedCtr = 0;

        } else {                                                /* Else add net conn after conn chain head ...          */
                                                                /* ... (see Note #1b1A).                                */
            pconn->PrevChainPtr      = (void     *)0;
            pconn->NextChainPtr      = (void     *)0;

            pconn->PrevConnPtr       = (void     *)pconn_chain;

            pconn_next               = (NET_CONN *)pconn_chain->NextConnPtr;
            pconn->NextConnPtr       = (void     *)pconn_next;
            pconn_chain->NextConnPtr = (void     *)pconn;
            if (pconn_next != (NET_CONN *)0) {
                pconn_next->PrevConnPtr = (void *)pconn;
            }
        }
    }

    pconn->ConnList     = (void **)pconn_list;                  /* Mark conn's conn list ownership.                     */
    pconn->ConnListType =  conn_list_type;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          NetConn_Unlink()
*
* Description : Unlink a network connection from its network connection list.
*
* Argument(s) : pconn       Pointer to a network connection.
*               -----       Argument validated in NetConn_ListUnlink(),
*                                                 NetConn_ListSrch(),
*                                                 NetConn_FreeHandler().
*
* Return(s)   : none.
*
* Caller(s)   : NetConn_ListUnlink(),
*               NetConn_ListSrch(),
*               NetConn_FreeHandler().
*
* Note(s)     : (1) Since NetConn_Unlink() called ONLY to remove & then re-link or free network
*                   connections, it is NOT necessary to clear the entry's previous & next chain/
*                   connection pointers.  However, pointers cleared to NULL shown for correctness
*                   & completeness.
*********************************************************************************************************
*/
/*$PAGE*/
static  void  NetConn_Unlink (NET_CONN  *pconn)
{
    NET_CONN  **pconn_list;
    NET_CONN   *pconn_prev;
    NET_CONN   *pconn_next;
    NET_CONN   *pconn_chain_prev;
    NET_CONN   *pconn_chain_next;


    pconn_list = (NET_CONN **)pconn->ConnList;
    if (pconn_list == (NET_CONN **)0) {                                 /* If net conn NOT in conn list, ...            */
        return;                                                         /* ... exit unlink.                             */
    }

                                                                        /* ----- UNLINK NET CONN FROM CONN CHAIN ------ */
    pconn_prev = (NET_CONN *)pconn->PrevConnPtr;
    pconn_next = (NET_CONN *)pconn->NextConnPtr;
                                                                
    if (pconn_prev != (NET_CONN *)0) {                                  /* If prev net conn non-NULL, ...               */
                                                                        /* ... unlink conn from middle of conn chain.   */
        pconn_prev->NextConnPtr = (void *)pconn_next;                   /* Point prev conn to next conn.                */

        if (pconn_next != (NET_CONN *)0) {                              /* If next net conn non-NULL, ...               */
            pconn_next->PrevConnPtr = (void *)pconn_prev;               /* ... point next conn to prev conn.            */
        }

    } else {                                                            /* Else unlink conn chain head.                 */
        pconn_chain_prev = (NET_CONN *)pconn->PrevChainPtr;
        pconn_chain_next = (NET_CONN *)pconn->NextChainPtr;

        if (pconn_next  != (NET_CONN *)0) {                             /* If next conn in conn chain non-NULL, ...     */
                                                                        /* ... promote next conn to conn chain head.    */
            pconn_next->PrevChainPtr = (void  *)pconn_chain_prev;
            if (pconn_chain_prev != (NET_CONN *)0) {
                pconn_chain_prev->NextChainPtr = (void     *)pconn_next;
            } else {
               *pconn_list                     = (NET_CONN *)pconn_next;
            }

            pconn_next->NextChainPtr = (void  *)pconn_chain_next;
            if (pconn_chain_next != (NET_CONN *)0) {
                pconn_chain_next->PrevChainPtr = (void     *)pconn_next;
            }

            pconn_next->PrevConnPtr          = (void *)0;
                                                                        /* Inherit conn chain accessed ctr.             */
            pconn_next->ConnChainAccessedCtr =  pconn->ConnChainAccessedCtr;
            pconn->ConnChainAccessedCtr      =  0;

        } else {                                                        /* Else remove conn list entirely.              */
            if (pconn_chain_prev != (NET_CONN *)0) {
                pconn_chain_prev->NextChainPtr = (void     *)pconn_chain_next;
            } else {
               *pconn_list                     = (NET_CONN *)pconn_chain_next;
            }

            if (pconn_chain_next != (NET_CONN *)0) {
                pconn_chain_next->PrevChainPtr = (void     *)pconn_chain_prev;
            }
        }
    }

    pconn->ConnList     = (void **)0;                                   /* Clr net conn's conn list ownership.          */
    pconn->ConnListType =  NET_CONN_LIST_TYPE_NONE;

#if (NET_DBG_CFG_MEM_CLR_EN == DEF_ENABLED)                             /* Clr net conn's chain/conn ptrs (see Note #1).*/
    pconn->PrevChainPtr = (void  *)0;
    pconn->NextChainPtr = (void  *)0;
    pconn->PrevConnPtr  = (void  *)0;
    pconn->NextConnPtr  = (void  *)0;
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'MODULE  Note #1' & 'net_conn.h  MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of net conn module include (see Note #1).        */

