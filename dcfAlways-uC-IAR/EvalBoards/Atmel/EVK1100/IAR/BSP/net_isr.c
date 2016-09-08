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
*                                BOARD SUPPORT PACKAGE (BSP) FUNCTIONS
*
*                                  TCP/IP STACK ISR SUPPORT FUNCTIONS
*                                              AVR32 UC3
*
* Filename      : net_isr.c
* Programmer(s) : Fabiano Kovalski
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>

/*
*********************************************************************************************************
*                                   NETWORK INTERRUPT INITIALIZATION
*
* Description : This function is called to initialize the interrupt controller associated with the NIC.
*
* Arguments   : None.
*
* Return(s)   : None.
*********************************************************************************************************
*/

void  NetNIC_IntInit  (void)
{
    BSP_INTC_IntReg(&NetNIC_ISR_Handler, AVR32_MACB_IRQ, 2);
}

/*
*********************************************************************************************************
*                                        NETWORK INTERRUPT CLEAR
*
* Description : This function is called to clear the interrupt controller associated with the NIC.
*
* Arguments   : None.
*
* Return(s)   : None.
*********************************************************************************************************
*/

void  NetNIC_IntClr (void)
{
}

/*
*********************************************************************************************************
*                                NETWORK INTERRUPT SERVICE ROUTINE ENTER
*
* Description : This function is called when the ISR begins.
*
* Arguments   : None.
*
* Return(s)   : None.
*********************************************************************************************************
*/


void  NetNIC_IntEnter (void)
{
}

/*
*********************************************************************************************************
*                                   NETWORK INTERRUPT INITIALIZATION
*
* Description : This function is called to initialize the interrupt controller associated with the NIC.
*
* Arguments   : None.
*
* Return(s)   : None.
*********************************************************************************************************
*/

#if (EMAC_CFG_PHY_INT == DEF_ON)
void  NetNIC_PhyIntInit  (void)
{
    BSP_GPIO_SetIntMode(MACB_PHY_INT_PIN, BSP_INT_FALLING_EDGE);
    BSP_INTC_IntReg(&NetNIC_PhyISR_Handler, AVR32_GPIO_IRQ_0 + (MACB_PHY_INT_PIN / 8), 2);
}
#endif

/*
*********************************************************************************************************
*                                        NETWORK INTERRUPT CLEAR
*
* Description : This function is called to clear the interrupt controller associated with the NIC.
*
* Arguments   : None.
*
* Return(s)   : None.
*********************************************************************************************************
*/

#if (EMAC_CFG_PHY_INT == DEF_ON)
void  NetNIC_PhyIntClr (void)
{
    BSP_GPIO_IntFlagClr(MACB_PHY_INT_PIN);
}
#endif

/*
*********************************************************************************************************
*                                NETWORK INTERRUPT SERVICE ROUTINE ENTER
*
* Description : This function is called when the ISR begins.
*
* Arguments   : None.
*
* Return(s)   : None.
*********************************************************************************************************
*/

#if (EMAC_CFG_PHY_INT == DEF_ON)
void  NetNIC_PhyIntEnter (void)
{
}
#endif
