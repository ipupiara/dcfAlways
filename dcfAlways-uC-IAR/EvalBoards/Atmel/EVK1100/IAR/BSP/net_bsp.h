/*
*********************************************************************************************************
*                                              uC/TCP-IP
*                                      The Embedded TCP/IP Stack
*                                 ATMEL  AVR32 UC3  Board Support Package
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : NET_BSP.C
* By      : Fabiano Kovalski
*********************************************************************************************************
*/

#ifndef _NET_BSP_H
#define _NET_BSP_H


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef     NET_BSP_MODULE
#define    NET_BSP_EXT
#else
#define    NET_BSP_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/
                                                                        /* ----------- EMAC RX BUFFERS ----------                                   */
#define  NIC_RX_BUF_SIZE                                 128            /* Set each buffer                                                          */
#define  NIC_RX_N_BUFS                                   128            /* Number of RX descriptors AND buffers.   */

                                                                        /* Configure MAC address set method :                                       */
#define  EMAC_CFG_MAC_ADDR_SEL          EMAC_MAC_ADDR_SEL_CFG
                                                                        /* EMAC_MAC_ADDR_SEL_EEPROM from NIC's EEPROM                               */
                                                                        /* EMAC_MAC_ADDR_SEL_CFG    from application                                */

#define  EMAC_CFG_RMII                  DEF_YES                         /* Enable RMII mode of operation                                            */
#define  EMAC_CFG_PHY_INT               DEF_YES                         /* Enable PHY interrupt for state changes                                   */
#define  EMAC_CFG_PHY_ADDR                                 1            /* PHY Address                                                              */

#define  EMAC_SEL_AVR32                 DEF_YES                         /* Set USRIO register with AVR32 options                                    */
#define  MACB_BASE                                0xFFFE1800            /* The base address of the EMACB peripheral                                 */

#define  MACB_PHY_INT_PIN               AVR32_PIN_PA24                  /* GPIO pin connected to the PHY INT pin                                    */

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MACRO'S
*
* Note(s) : (1) When NET_NIC_CFG_RD_WR_SEL configured to NET_NIC_RD_WR_SEL_MACRO, implement NIC read/write
*               functionality with macro's.
*
*               (a) The developer MUST implement the NIC read/write macro functions with the correct number
*                   & type of arguments necessary to implement the NIC's read/write functionality.
*
*               (b) The developer SHOULD appropriately name the NIC read/write macro functions by appending
*                   the number of bits that the macro functions read/write to the end of the macro function
*                   name :
*
*                           NetNIC_Rd_xx()
*                           NetNIC_Wr_xx()
*
*                   For example, 16-bit read/write macro functions should be named :
*
*                           NetNIC_Rd_16()
*                           NetNIC_Wr_16()
*
*
*               See also 'net_bsp.c  NetNIC_Rd_xx()  Note #1'
*                      & 'net_bsp.c  NetNIC_Wr_xx()  Note #1'.
*********************************************************************************************************
*/

#if (NET_NIC_CFG_RD_WR_SEL != NET_NIC_RD_WR_SEL_FNCT)

#define  NetNIC_Rd_xx(...)

#define  NetNIC_Wr_xx(..., val)

#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        NetBSP_Phy_HW_Init(void);

CPU_INT32U  NetBSP_EMAC_ClkFreq(void);
void        NetBSP_EMAC_PhyRdWrDly(void);

void        NetBSP_DlyMs(CPU_INT32U ms);

/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#endif
