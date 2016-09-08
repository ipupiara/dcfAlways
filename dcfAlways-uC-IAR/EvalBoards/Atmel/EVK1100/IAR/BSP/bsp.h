/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                 ATMEL  AVR32 UC3  Board Support Package
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*
* File    : BSP.H
* By      : Fabiano Kovalski
*********************************************************************************************************
*/

#include <cpu.h>

/*
**************************************************************************************************************
*                                            CPU CLOCK FREQUENCY
**************************************************************************************************************
*/

#define     CPU_CONST_FREQ                     1                        /* Enable automatic CPU operating frequency check           */

#if CPU_CONST_FREQ > 0
  #define   CPU_CLK_FREQ()              BSP_CPU_ClkFreq()
#else
  #define   CPU_CLK_FREQ()             (48000000)                       /* CPU operating frequency                                  */
#endif


//#define AdjustCycleToRTC
// pn 5.sept 12, rtcaPid control is still in wip- state,so better don't use and currently I work on another project (embedded linux)


/*
**************************************************************************************************************
*                                            INTERRUPT CONTROLLER
**************************************************************************************************************
*/

#define     BSP_INTC_INT_LEVELS                4                        /* Interrupt priority levels                                */

#define     BSP_INTC_ERR_NONE                  0                        /* BSP_INTC_IntReg is successful                            */
#define     BSP_INTC_ERR_INVALID_IRQ           1                        /* BSP_INTC_IntReg received an invalid IRQ #                */

#define     BSP_INT_PIN_CHANGE                 0
#define     BSP_INT_RISING_EDGE                1
#define     BSP_INT_FALLING_EDGE               2

/*
**************************************************************************************************************
*                                            EXTERNALS
**************************************************************************************************************
*/

extern  const  CPU_INT32U  OSIntPrioReg[BSP_INTC_INT_LEVELS];           /* Holds INT level & INT vector address for each prio level */
extern CPU_INT32U _evba;


/*$PAGE*/
/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         BSP_Init(void);

CPU_INT32U   BSP_INTC_IntReg(CPU_FNCT_VOID handler, CPU_INT32U irq, CPU_INT32U int_level);
//CPU_INT32U   BSP_INTC_IntUnReg(CPU_INT32U irq);

CPU_INT32U   BSP_INTC_FastIntReg(CPU_FNCT_VOID handler, CPU_INT32U irq, CPU_INT32U int_level);

void         BSP_GPIO_SetIntMode(CPU_INT16U pin, CPU_INT08U mode);
CPU_BOOLEAN  BSP_GPIO_IntFlagRd(CPU_INT16U pin);
void         BSP_GPIO_IntFlagClr(CPU_INT16U pin);
void         BSP_GPIO_SetFnct(CPU_INT16U pin, CPU_INT08U function);
CPU_BOOLEAN  BSP_GPIO_PinGet(CPU_INT16U pin);
void         BSP_GPIO_PinSet(CPU_INT16U pin);
void         BSP_GPIO_PinClr(CPU_INT16U pin);
void         BSP_GPIO_PinTgl(CPU_INT16U pin);

CPU_INT32U   BSP_CPU_ClkFreq(void);
CPU_INT32U   BSP_PBA_ClkFreq(void);
CPU_INT32U   BSP_PBB_ClkFreq(void);


void         BSP_USART_Init(CPU_INT08U com, CPU_INT32U baud_rate);
void         BSP_USART_ByteWr(CPU_INT08U com, CPU_INT08U b);
CPU_INT08U   BSP_USART_ByteRd(CPU_INT08U com);
void         BSP_USART_StrWr(CPU_INT08U com, CPU_INT08U *s);
void         BSP_USART_printf(CPU_INT08U com, CPU_CHAR *format, ...);
void         BSP_USART_PrintHex(CPU_INT08U com, CPU_INT32U value, CPU_INT08U digits);
void         BSP_USART_PrintDec(CPU_INT08U com, CPU_INT32U value, CPU_INT08U digits);
void         BSP_USART_IntEn(CPU_INT08U com, CPU_INT32U mask);
void         BSP_USART_IntDis(CPU_INT08U com, CPU_INT32U mask);


/*
*********************************************************************************************************
*                                             LED SERVICES
*********************************************************************************************************
*/

#define LED0  0x01
#define LED1  0x02
#define LED2  0x03
#define LED3  0x04
/*#define LED4  0x10   not available on evk3a board
#define LED5  0x20
#define LED6  0x40
#define LED7  0x80
*/

void         LED_On(CPU_INT08U led);
void         LED_Off(CPU_INT08U led);
void         LED_Toggle(CPU_INT08U led);

void        incCycleDiff( CPU_INT32U*  cycle, CPU_INT32S  cdiff);    
                        // pn 26. aug 12, added to enable exact setting of cycle by comparing 
                        //the time with 32 kHz real time counter RTC


/*********************************************************************************************************
*                                            MISCELLANEOUS
*********************************************************************************************************
*/

#ifndef CPU_CONST_FREQ
  #error   "BSP.H, Missing CPU_CONST_FREQ: Enable (1) or Disable (0) automatic CPU frequency check at CPU_CLK_FREQ()"
#endif

#ifdef AdjustCycleToRTC
CPU_INT32S getCycleDiff();
#endif


