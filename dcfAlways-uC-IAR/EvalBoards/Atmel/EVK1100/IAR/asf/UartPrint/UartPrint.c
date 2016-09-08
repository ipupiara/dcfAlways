/*
 * UartPrint.c
 *
 *  Created on: 30.05.2012
 *      Author: duenda
 */

#include <string.h>
//#include <malloc.h>
#include <math.h>
#include <stdarg.h>



#include "includes.h"
#include "UartPrint.h"
#include "udc.h"



CPU_INT32U  debugUartLoopCnt, debugUartPosFlag;

/*
 *
 *    Serial  printing stuff
 *
 *
 */

OS_STK  SerialQMethodStk[SerialQMethod_STK_SIZE];
//static  void  SerialQMethod (void *p_arg);


#define serialStrBufSz    14
#define serialStrSz    80    // pn 9.June2012 attention: a value of e.g. 123 will give an unhandled exception.... ????
typedef struct serialMem {
  char serialStr [serialStrSz];  // ATTENTION serialStr must be at first place, so that &(serialMem) == &(serialStr)
} serialMem;
#define serialTaskQMsgSz  serialStrBufSz

CPU_INT08U  serialOn;
CPU_INT32U   amtUSBInterrupts;
CPU_INT32U   amtUSBNonSofInterrupts;

OS_EVENT *SerialQSem;
OS_MEM *serialMsgMem;
OS_EVENT*  serialMsgTaskQ;
serialMem serialStrBuf[serialStrBufSz];
void* serialTaskQMsg[serialTaskQMsgSz];

// error print methods, just try to work a bit with hardware and BSP
//

#ifdef usePDCA

#define PDCA_CHANNEL_USART_PRINTF 0
#define AVR32_PDCA_PID_USART_TX       AVR32_PDCA_PID_USART0_TX
#define App_IRQ_PDCA  AVR32_PDCA_IRQ_0

CPU_INT32U amtPdcaErrors;

OS_FLAG_GRP *pdcaStatus;
#define TransferFinished (OS_FLAGS)0x0001
#define ReloadCounterZero (OS_FLAGS)0x0002

void* reloadDataP;
void* transferDataP;

#endif

#define   APP_USART_COM               0
#define   APP_USART                   AVR32_USART0;

#ifdef use_USART_TxRdyInt

#define   App_IRQ_USART               AVR32_USART0_IRQ

#endif




CPU_INT32U  amtErr;			// amt calls to err_printf
CPU_INT32U  amtPrintErr;   // errors during print out, where err_printf should not be called
CPU_INT08U  err_init_print;


void OnPrintError()
{
  ++amtPrintErr;  // at least keep the number for debugging, maybe later an led could be switched on or so...
}


#ifdef usePDCA


// PDCA channel options
 static const pdca_channel_options_t PDCA_OPTIONS =
 {
   .addr = NULL,  //(void *)ascii_anim1,              // memory address
   .pid = AVR32_PDCA_PID_USART_TX,           // select peripheral - data are transmit on USART TX line.
   .size =  0 ,//sizeof(ascii_anim1),              // transfer counter
   .r_addr = NULL,                           // next memory address
   .r_size = 0,                              // next transfer counter
   .transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer
 };


char* errorResetStr = "Error reset\n";

/*
 * actually an error should never happen, but if it does, just try to signalise it somehow
 * and reset it if posssible
 * communicate transferComplete and reloadCounterZero to the application 

    pn, 9.7.12
    pdca interrupt was implemented specifically only for uart, while in a non-prototype design,
    pdca interrupt would be held generally for an arbitrary amount of pdca-channels working on a
    notification method per pdca-channel call design or a similar design, eg. an
    OSFlag registry per channel

*/

void APP_PDCA_ISR (void)
{
	INT32U isr;
	INT8U err;
	isr = pdca_get_interrupt_status(PDCA_CHANNEL_USART_PRINTF);
	if (isr & AVR32_PDCA_ISR_TERR_MASK)  {
		amtPdcaErrors ++;
//		pdca_load_channel(PDCA_CHANNEL_USART_PRINTF, errorResetStr, sizeof(errorResetStr));
        // errorResetStr may not be mixed into SerialMsgTaskQ! so let method handle errors
		// load method clears also error state, try communicate the error if possible
	}
	if (isr & AVR32_PDCA_ISR_RCZ_MASK) {
		OSFlagPost(pdcaStatus, ReloadCounterZero, OS_FLAG_SET, &err);
		pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_PRINTF);
	}
	if (isr & AVR32_PDCA_IMR_TRC_MASK) {
		OSFlagPost(pdcaStatus, TransferFinished, OS_FLAG_SET, &err);
		 pdca_disable_interrupt_transfer_complete(PDCA_CHANNEL_USART_PRINTF);
	}
}
//#warning is pdca isr needed for tcp/ip ?
// pn 21. Aug 12   seems that tcp-ip doesnt need pdca, seems to work all with macb interrupts 

void initPDCA(INT8U *perr)
{
    
    debugUartLoopCnt = debugUartPosFlag = 0;
    
	transferDataP = reloadDataP = NULL;
	  pdcaStatus = OSFlagCreate( 0, perr);  // will obviousely be set by interrupts, as seen when debugging
	  if (*perr == OS_ERR_NONE)  {
		  OSFlagNameSet(pdcaStatus, (INT8U *) "pdcaStatusFlags", perr);
	  }
	  if (*perr == OS_ERR_NONE)  {
		  if (BSP_INTC_IntReg(&APP_PDCA_ISR, App_IRQ_PDCA, 1) == BSP_INTC_ERR_NONE ) {
			*perr = OS_NO_ERR;
		} else {
			*perr = 0xFF;
		}
	  }
	  if (*perr ==  OS_ERR_NONE) {
		  pdca_init_channel(PDCA_CHANNEL_USART_PRINTF, &PDCA_OPTIONS);
		  pdca_enable_interrupt_transfer_error(PDCA_CHANNEL_USART_PRINTF);
		  pdca_enable_interrupt_transfer_complete(PDCA_CHANNEL_USART_PRINTF);
		  pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_PRINTF);
		  pdca_enable(PDCA_CHANNEL_USART_PRINTF);
	  }
}

static  void  SerialQMethod (void *p_arg)
{
	INT8U err;
	serialMem* sm;
//	volatile  avr32_usart_t  *usart;
	OS_FLAGS flags;

//	usart = &APP_USART;
     while (1) {
         ++ debugUartLoopCnt;
         debugUartPosFlag = 1;
    	 sm = (serialMem *)OSQPend(serialMsgTaskQ, 3167, &err);
         debugUartPosFlag = 2;
    	 if (err == OS_NO_ERR) {
    		 flags = OSFlagPend(pdcaStatus, ReloadCounterZero,
						 OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 1619, &err);
             debugUartPosFlag = 3;
    		 if (err == OS_NO_ERR)  {
    			 if (transferDataP != NULL) {
    				 err = OSMemPut(serialMsgMem, transferDataP);
					 if (err != OS_NO_ERR) {
						 OnPrintError();
					 }
    			 }
    			 transferDataP = reloadDataP;
    			 reloadDataP = (void*) sm;
    			 pdca_reload_channel(PDCA_CHANNEL_USART_PRINTF,sm,strlen(sm->serialStr));
    			 OSFlagPost(pdcaStatus, TransferFinished, OS_FLAG_CLR, &err);  // no more valid
    			 pdca_enable_interrupt_transfer_complete(PDCA_CHANNEL_USART_PRINTF);
    			 pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_PRINTF);
    			 // prevent loop on pdca interrupts that otherwise obivousely happens
    		 } else  {
                 // 16 secs no reload counter zero... something with pdca must be wrong
                 // reset everything
                 pdca_disable(PDCA_CHANNEL_USART_PRINTF);
                 if (transferDataP != NULL) {
                     err = OSMemPut(serialMsgMem, transferDataP);
                     if (err != OS_NO_ERR) {
                             OnPrintError();
                     }
                     transferDataP = NULL;
                 }
                 if (reloadDataP != NULL) {                                     
                     err = OSMemPut(serialMsgMem, reloadDataP);
                     if (err != OS_NO_ERR) {
                             OnPrintError();
                     }
                     reloadDataP = NULL;
                 }
                 if (sm != NULL) {                                     
                     err = OSMemPut(serialMsgMem, sm);
                     if (err != OS_NO_ERR) {
                             OnPrintError();
                     }
                     sm = NULL;
                 }
                 pdca_resetErrorFlag(PDCA_CHANNEL_USART_PRINTF);
                 pdca_enable(PDCA_CHANNEL_USART_PRINTF);
                    
                 OSFlagPost(pdcaStatus, ReloadCounterZero, OS_FLAG_SET, &err);  // pn 4. sept 12 probable needed to continue, still tobe tested
    			 OnPrintError();
#warning:  the timeout wait for reloadCounterZero in UartPrint.c is still tobe tested
    		 }
		 } else {  // normally will be timeout  .. see it optimistic :-)
             debugUartPosFlag = 4;
			 flags = OSFlagQuery(pdcaStatus, &err);
             debugUartPosFlag = 5;
                         if (err == OS_NO_ERR)  {
                             if (flags & TransferFinished ) {
                                     if (transferDataP) {
                                         err = OSMemPut(serialMsgMem, transferDataP);
                                         if (err != OS_NO_ERR) {
                                                 OnPrintError();
                                         }
                                         transferDataP = NULL;
                                     }
                                     if (reloadDataP) {
                                         err = OSMemPut(serialMsgMem, reloadDataP);
                                         if (err != OS_NO_ERR) {
                                                 OnPrintError();
                                         }
                                         reloadDataP = NULL;
                                     }
                                     OSFlagPost(pdcaStatus, TransferFinished, OS_FLAG_CLR, &err);
                                     // if (err != OS_NO_ERR) ...
                              } // transfer finished
                         } // flags queried ok .. else {  error ...}
		 }  // timeout
	 } // while 1
}



#endif


#ifdef use_USART_TxRdyInt

void  APP_USARTRxTxISR (void)
{
    volatile  avr32_usart_t  *usart;
    INT8U err;

    usart = &APP_USART;
//    if (usart->CSR.rxrdy == 1) {
//        APP_RxISRHandler();
//    }
    if (usart->CSR.txrdy == 1) {
    	err = OSSemPost(SerialQSem);
    	if (err != OS_NO_ERR){
//    		err_printf("couldnt signal sema in USART ISR\n");
    	}
    	BSP_USART_IntDis (APP_USART_COM, (1<< AVR32_USART_IER_TXRDY));
    	// PN 24. May 2012
    	// contrary to other AVR processors, it seems necessary to disable TXRDY interrupts
    	// manually, while other processors ATMega32/64 just trigger TXRDY interrupt once
    	// further testing will take place.
    }
}

/*
 * On a first attempt we try to send char after char to the uart for transmit
 * as soon a TX is ready, signalled by an interrupt.
 * Later we should try to use DMA/PDCA instead to transfer a whole buffer at once.
 */

static  void  SerialQMethod (void *p_arg)
{
	INT8U err;
	serialMem* sm;
	INT8U strInd;
	volatile  avr32_usart_t  *usart;

	usart = &APP_USART;
     while (1) {
    	 sm = (serialMem *)OSQPend(serialMsgTaskQ, 1097, &err);
    	 if (err == OS_NO_ERR) {
    		 for (strInd = 0; strInd < (strlen(sm->serialStr) -1 ); ++ strInd){
    			 OSSemPend(SerialQSem, 107, &err);
    			 if (err == OS_NO_ERR){
    				 OSSemSet(SerialQSem, 0, &err);  // by initalization to value of 1, the count
												 // might get higher than one, what does not make sense
												 // and next char would be lost
    				 if (err  != OS_NO_ERR) {
    					 err_printf("Sem set 0 error, SerialQMeth");
    				 }
    				 usart->thr      = sm->serialStr[strInd];
    				 BSP_USART_IntEn (APP_USART_COM, (1<< AVR32_USART_IER_TXRDY));
    			    	// PN 24. May 2012
    			    	// contrary to other AVR processors, it seems necessary to disable TXRDY interrupts
    			    	// manually, while other processors ATMega32/64 just trigger TXRDY interrupt once
    			    	// further testing will take place.
    			 } else {
					 if ((err == OS_TIMEOUT)
							 && (!((usart->csr & AVR32_USART_CSR_TXRDY_MASK) == 0)))
					 {   // ready to send byte, but sema accidentally not set, recover situation
						 usart->thr      = sm->serialStr[strInd];
					 }
    			 }
    		 }

        	 err = OSMemPut(serialMsgMem, (void *)sm);
        	 if (err != OS_NO_ERR) {
        	    OnPrintError();   // err_printf("mem put problem sec100 method\n");
        	 }
    	 } else {
    	  // Message not received, must have timed out
    	// better not err_printf("serial Q timeout\n");
    		 // but maybe set a pin, or other message on different device
    	 }
    }
}

#endif

void init_err_printf()
{
	CPU_INT08U retVal;
    amtUSBInterrupts = 0;
    amtUSBNonSofInterrupts = 0;
	serialOn = 0;
	amtErr = 0;
	amtPrintErr = 0;
	err_init_print = OS_NO_ERR;

	if (err_init_print == OS_NO_ERR) {
		BSP_USART_Init (APP_USART_COM, 57600);  // start with something slow, later increase

		if (!(SerialQSem = OSSemCreate(0)))
		// after initialisation, probable a character can be sent
		// worst case we just loose it
		{
			//err_printf("could not create serialQSem\n");
			err_init_print = 0xFF;
		}
	}

#ifdef usePDCA
	initPDCA(&err_init_print);
#endif

	memset(serialStrBuf,0x00,sizeof(serialStrBuf));
	if (err_init_print == OS_NO_ERR) {
		serialMsgMem = OSMemCreate(&serialStrBuf[0], serialStrBufSz, sizeof(serialMem), &err_init_print);
	}
	if (err_init_print == OS_NO_ERR) {
		OSMemNameSet(serialMsgMem, (INT8U*)"serialMsgMem", &err_init_print);
	}
	if (err_init_print == OS_NO_ERR) {
		serialMsgTaskQ = OSQCreate(&serialTaskQMsg[0], serialTaskQMsgSz);
		if (! serialMsgTaskQ) err_init_print = 0xFF;
	}

#ifdef use_USART_TxRdyInt

	if (err_init_print == OS_NO_ERR) {
		if (BSP_INTC_IntReg(&APP_USARTRxTxISR, App_IRQ_USART, 1) == BSP_INTC_ERR_NONE ) {
			err_init_print = OS_NO_ERR;
		} else {
			err_init_print = 0xFF;
		}

	}
	BSP_USART_IntEn (APP_USART_COM, (1<< AVR32_USART_IER_TXRDY));
#endif
        
        if (err_init_print != OS_NO_ERR ) {OnPrintError(); }

    retVal = OSTaskCreateExt(SerialQMethod,                                       /* Create the start task                                    */
                    (void *)0,
                    (OS_STK *)&SerialQMethodStk[SerialQMethod_STK_SIZE - 1],
                    SerialQ_TASK_PRIO,
                    SerialQ_TASK_PRIO,
                    (OS_STK *)&SerialQMethodStk[0],
                    SerialQMethod_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    
    if (retVal !=  OS_NO_ERR  ) {OnPrintError(); }                // at least count the errros for debugging reasons
    

    OSTaskNameSet(CalculationController_TASK_PRIO, (CPU_CHAR *)"SerQ", &retVal);

	serialOn = (err_init_print == OS_NO_ERR);
    info_printf("serial print ready for use\n");
    info_printf("second test ok\n");
     info_printf("third test ok\n");
}


//char bufferS [serialStrSz] ;

void info_printf( char *emsg, ...)
{
	va_list ap;
	CPU_INT08S  res;
	CPU_INT08U	err;
	serialMem* sm;

	va_start(ap, emsg);

	if (serialOn == 1) {
		sm = (serialMem *) OSMemGet(serialMsgMem, &err);
		if( sm != 0 ) {
			res = vsnprintf(sm->serialStr, serialStrSz-1,  emsg, ap);
			if (res < 0) res = serialStrSz;

			sm->serialStr[serialStrSz-1] = 0;

//            attemptSendUSBString(sm->serialStr);     // temporarely commented out, pn 31 aug 12
            
			err = OSQPost(serialMsgTaskQ, (void *)sm);
			if ( err != OS_NO_ERR) {
			//	do something but dont loop--->> err_printf("Q post err tickHook\n");
				OnPrintError();
			}

		} else	{  //OSMemGet
			// do somethingg ???
			OnPrintError();
		}
	} else {   // serialOn
		// might turn on an led or so
		OnPrintError();
	}
	va_end(ap);
	//	printf(emsg, ap);
}

// just for usage with short strings, otherwise sizes of buffers need to be increased
void  err_printf ( char *emsg, ...)
{
	va_list ap;

	va_start(ap, emsg);
	++ amtErr;
	info_printf(emsg, ap);
	va_end(ap);
}


/*
 * end of serial printing stuff
 */


