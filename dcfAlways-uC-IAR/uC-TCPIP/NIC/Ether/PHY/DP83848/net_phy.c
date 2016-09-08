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
* Filename      : net_phy.c
* Version       : V1.90
* Programmer(s) : EHS
*                 FGK (Added PHY interrupt support for Auto-negotiation Complete, and Link Status)
*********************************************************************************************************
* Note(s)       : (1) Supports National Semiconductor DP83848 10/100 PHY
*
*                 (2) The MII interface port is assumed to be part of the host EMAC; consequently,
*                     reads from and writes to the PHY are made through the EMAC.  The functions
*                     NetNIC_PhyRegRd() and NetNIC_PhyRegWr(), which are used to access the PHY, should
*                     be provided in the EMAC driver.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <net.h>
#include  <net_phy.h>
#include  <net_phy_def.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         NetNIC_PhyInit()
*
* Description : Initialize phyter (ethernet link controller)
*
* Argument(s) : none.
*
* Return(s)   : 1 for OK, 0 for error
*
* Caller(s)   : EMAC_Init()
*
* Note(s)     : Assumes the MDI port as already been enabled for the PHY.
*********************************************************************************************************
*/

void  NetNIC_PhyInit (NET_ERR *perr)
{
    CPU_INT32U   reg_val;
    CPU_INT16U   i;
    CPU_INT16U   oui_msb;
    CPU_INT16U   oui_lsb;


    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, MII_BMCR, BMCR_RESET, perr);     /* Reset the PHY                                            */
    i               = DP83848_INIT_RESET_RETRIES;
    reg_val         = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_BMCR, perr) & BMCR_RESET;

    while (reg_val == BMCR_RESET && i > 0) {
        NetBSP_DlyMs(10);                                               /* Delay while reset completes > (1uS + 3uS) for soft reset */
                                                                        /* Read the control register                                */
        reg_val     =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_BMCR, perr) & BMCR_RESET;
        i--;
    }

    if (i == 0) {                                                       /* If reset has not completed and no retries remain         */
       *perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return a reset timeout error                             */
        return;
    }


    oui_msb         = (DP83848_OUI >>   6) & 0xFFFF;
    oui_lsb         = (DP83848_OUI & 0x3F) <<    10;
                                                                        /* Read the PHY ID, ensure the PHY has reset                */
    reg_val         =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_PHYSID1, perr);
    if (reg_val != oui_msb) {
       *perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return an error  if the PHY is not properly reset        */
        return;
    }
                                                                        /* Read the PHY ID, ensure the PHY has reset                */
    reg_val         =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_PHYSID2, perr);
    if ((reg_val & 0xFC00) != oui_lsb) {
       *perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return an error  if the PHY is not properly reset        */
        return;
    }

                                                                        /* Read RBR for current mode of operation                   */
    reg_val         =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, DP83848_RBR, perr);
    reg_val        &=  0x33;                                            /* Ignore read-only fields of RBR                           */
#if (!defined(EMAC_CFG_RMII)) || (EMAC_CFG_RMII <= 0)
    if ((reg_val & RBR_RMII_MODE) == RBR_RMII_MODE) {
        reg_val    &= ~RBR_RMII_MODE;                                   /* Activate MII mode of operation                           */
    }
#else
    if ((reg_val & RBR_RMII_MODE) != RBR_RMII_MODE) {
        reg_val    |=  RBR_RMII_MODE;                                   /* Activate RMII mode of operation                          */
    }
#endif
    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, DP83848_RBR, reg_val, perr);     /* Write RBR with proper mode of operation                  */


#if (EMAC_CFG_PHY_INT == DEF_ON)
    NetNIC_PhyIntInit();

                                                                        /* Enable interrupts on PHY INT pin                         */
    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, DP83848_MICR, MICR_INTEN | MICR_INTOE, perr);
                                                                        /* Enable Link Status & Auto-negotiation interrupts         */
    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, DP83848_MISR, MISR_LINK_INT_EN | MISR_ANC_INT_EN, perr);

                                                                        /* Restart Auto-Negotiation (INT updates NetNIC_ConnStatus) */
    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART, perr);

#else

    NetNIC_PhyAutoNeg();                                                /* Attempt Auto-Negotiation                                 */

    NetNIC_ConnStatus = NetNIC_PhyLinkState();                          /* Set NetNIC_ConnStatus according to link state            */

    if (NetNIC_ConnStatus == DEF_ON) {
        NetNIC_LinkUp();
    } else {
        NetNIC_LinkDown();
    }
#endif
}


/*
*********************************************************************************************************
*                                        NetNIC_PhyAutoNeg()
*
* Description : Do link auto-negotiation
*
* Argument(s) : none.
*
* Return(s)   : 1 = no error, 0 = error
*
* Caller(s)   : NetNIC_PhyInit.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetNIC_PhyAutoNeg (void)
{
    CPU_BOOLEAN  link_state;
    CPU_INT16U   i;
    NET_ERR      err;

                                                                        /* Restart Auto-Negotiation                                 */
    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART, &err);

    i          = DP83848_INIT_AUTO_NEG_RETRIES;
    link_state = NetNIC_PhyAutoNegState();

    while ((link_state != DEF_ON) && (i > 0)) {
        NetBSP_DlyMs(1500);
        link_state = NetNIC_PhyAutoNegState();
        i--;
    }
}

/*
*********************************************************************************************************
*                                    NetNIC_PhyAutoNegState()
*
* Description : Returns state of auto-negotiation
*
* Argument(s) : none.
*
* Return(s)   : State of auto-negociation (DEF_OFF = not completed, DEF_ON = completed).
*
* Caller(s)   : NetNIC_PhyInit().
*
* Note(s)     : If any error is encountered while reading the PHY, this function
*               will return Auto Negotiation State = DEF_OFF (incomplete).
*********************************************************************************************************
*/

CPU_BOOLEAN  NetNIC_PhyAutoNegState (void)
{
    CPU_INT32U  reg_val;
    NET_ERR     err;


    reg_val     = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_BMSR, &err);

    if (err   != NET_PHY_ERR_NONE) {
        reg_val = 0;
    }

    if ((reg_val & BMSR_ANEGCOMPLETE) == BMSR_ANEGCOMPLETE) {
        return (DEF_ON);
    } else {
        return (DEF_OFF);
    }
}

/*
*********************************************************************************************************
*                                     NetNIC_PhyLinkState()
*
* Description : Returns state of ethernet link
*
* Argument(s) : none.
*
* Return(s)   : State of ethernet link (DEF_OFF = link down, DEF_ON = link up).
*
* Note(s)     : If any error is encountered while reading the PHY, this function
*               will return link state = DEF_OFF.
*********************************************************************************************************
*/

CPU_BOOLEAN  NetNIC_PhyLinkState (void)
{
    NET_ERR     err;
    CPU_INT16U  reg_val;


    reg_val      = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, DP83848_PHYSTS, &err);

    if (err == NET_PHY_ERR_NONE) {

        if ((reg_val & PHYSTS_LINK_STATUS) != 0) {

            return (DEF_ON);
        } else {

            return (DEF_OFF);
        }

    } else {

        return (DEF_OFF);
    }
}

/*
*********************************************************************************************************
*                                     NetPHY_GetLinkSpeed()
*
* Description : Returns the speed of the current Ethernet link
*
* Argument(s) : none.
*
* Return(s)   : 0 = No Link, 10 = 10mbps, 100 = 100mbps
*
* Caller(s)   : EMAC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  NetNIC_PhyLinkSpeed (void)
{
    NET_ERR     err;
    CPU_INT16U  reg_val;


    if (NetNIC_PhyLinkState() == DEF_OFF) {

        return (NET_PHY_SPD_0);

    } else {

        reg_val = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, DP83848_PHYSTS, &err);

        if (err == NET_PHY_ERR_NONE) {

            if ((reg_val & PHYSTS_SPEED_STATUS) != 0) {


                return (NET_PHY_SPD_10);
            } else {

                return (NET_PHY_SPD_100);
            }
        } else {

            return (NET_PHY_SPD_0);
        }
    }
}

/*
*********************************************************************************************************
*                                     NetPHY_GetDuplex()
*
* Description : Returns the duplex mode of the current Ethernet link
*
* Argument(s) : none.
*
* Return(s)   : 0 = Unknown (Auto-Neg in progress), 1 = Half Duplex, 2 = Full Duplex
*
* Caller(s)   : EMAC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  NetNIC_PhyLinkDuplex (void)
{
    CPU_INT16U  reg_val;
    NET_ERR     err;


    if (NetNIC_PhyLinkState() == DEF_OFF) {

        return (NET_PHY_DUPLEX_UNKNOWN);

    } else {

        reg_val = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, DP83848_PHYSTS, &err);

        if (err == NET_PHY_ERR_NONE) {

            if ((reg_val & PHYSTS_DUPLEX_STATUS) != 0) {


                return (NET_PHY_DUPLEX_FULL);
            } else {

                return (NET_PHY_DUPLEX_HALF);
            }
        } else {

            return (NET_PHY_DUPLEX_UNKNOWN);
        }
    }
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                    GLOBAL FUNCTIONS: PHY INTERRUPTS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        NetNIC_ISR_Handler()
*
* Description : (1) Update NetNIC_ConnStatus according to link state
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (EMAC_CFG_PHY_INT == DEF_ON)
void  NetNIC_PhyISR_Handler (void)
{
    volatile  CPU_INT16U  reg_val;
              NET_ERR     err;


    NetNIC_PhyIntEnter();
                                                                        /* Retrieve status & clear interrupt                        */
    reg_val     = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, DP83848_MISR, &err);

    if (((reg_val & (MISR_ANC_INT | MISR_ANC_INT_EN)) != 0) ||          /* If Auto-negotiation Complete interrupt or,               */
        ((reg_val & (MISR_LINK_INT | MISR_LINK_INT_EN)) != 0)) {        /*    Change of Link Status interrupt, then                 */
        NetNIC_ConnStatus = NetNIC_PhyLinkState();                      /* Set NetNIC_ConnStatus according to link state            */

        if (NetNIC_ConnStatus == DEF_ON) {
            NetNIC_LinkUp();
        } else {
            NetNIC_LinkDown();
        }
    }

    NetNIC_PhyIntClr();
}
#endif

