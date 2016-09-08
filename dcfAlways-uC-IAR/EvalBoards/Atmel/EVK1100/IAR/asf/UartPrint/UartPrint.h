/*
 * UartPrint.h
 *
 *  Created on: 30.05.2012
 *      Author: duenda
 */

#ifndef UARTPRINT_H_
#define UARTPRINT_H_

#define usePDCA
#ifndef usePDCA
#define use_USART_TxRdyInt
#endif

void init_err_printf();
void  err_printf ( char *emsg, ...);
void info_printf( char *emsg, ...);

extern CPU_INT08U  serialOn;

extern CPU_INT32U   amtUSBInterrupts;

extern CPU_INT32U   amtUSBNonSofInterrupts;


#endif /* UARTPRINT_H_ */
