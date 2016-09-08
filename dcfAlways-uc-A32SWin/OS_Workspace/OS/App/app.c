/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                      ATMEL  AVR32 UC3 Sample code
*
*                                 (c) Copyright 2007; Micrium; Weston, FL
*                                           All Rights Reserved
*  originally
* File    : APP.C
* By      : Fabiano Kovalski
*
* extended
* By      :  Peter Niederer
*
*********************************************************************************************************
*/

#include <includes.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>

//#include "sdramc.h"

#include "UartPrint.h"

/*
**************************************************************************************************************
*                                               VARIABLES
**************************************************************************************************************
*/

OS_STK  CalcControllerMethodStk[CalcControllerMethod_STK_SIZE];
OS_STK  Sec100CalculatorMethodStk[Sec100CalculatorMethod_STK_SIZE];
OS_STK  ScoreCalculatorMethodStk[ScoreCalculatorMethod_STK_SIZE];


/*
**************************************************************************************************************
*                                           FUNCTION PROTOTYPES
**************************************************************************************************************
*/


static  void  CalcControllerMethod(void *p_arg);
static  void  AppTaskCreate(void);
static  void  Sec100CalculatorMethod (void *p_arg);
static  void  ScoreCalculatorMethod (void *p_arg);
void resetSec100Data();
void CalculationTable01_init();
void CalculationTable01_close();
void CalculationTable01_reset();




/*
 * Global enums
 */


enum JobStates {
	switchedOff,
	idleCount,
	prepare01,
	find01,
	findHypo,
	checkHypo,
	forwardTime
};


enum sec100States {
	firstSec100,
	initArray,
	runningCalc
} sec100States;

enum scoreResultTypes {
	search01OnGoing,
	foundDtDifference,
	foundTdt,
} scoreResultTypes;


/*
 * sec 100 calculation types and variables
 */


#define sec100MsgMemSize  10
#define sec100TaskQMsgSz 10

typedef struct sec100Info{
	CPU_INT08U pinVal;
	CPU_INT08U  sec100;
	CPU_INT08U sec;
	CPU_INT16S min;
	CPU_INT32U index;
}sec100Info;


OS_EVENT *Sec100TaskQ;
void *Sec100TaskQMsg[sec100TaskQMsgSz];
OS_MEM *CommSec100MsgMem;
sec100Info CommSec100Buf[sec100MsgMemSize];

sec100Info currentSec100Info;

CPU_INT08U  sec100Array [201];
sec100Info lastSec100Info;
CPU_INT08U		sec100State;
CPU_INT08U   lastFxyValue, lastFy2Value;


/*
 *
 * Calculation Table types and global variable
 *
 *
 */


typedef CPU_INT16S sumE2Type;

typedef struct Sec100value01 {
	sumE2Type  sExEy;
	sumE2Type  sEy2;
	sumE2Type  amtSteps;
}Sec100value01;

typedef struct SecValue01 {
	Sec100value01   sec100Values [100];
	float   maxRho;
	CPU_INT08S dT_atMaxRho;
}SecValue01;


typedef struct MinValue01{
	SecValue01   secValues[60];
}MinValue01;


typedef struct CalculationTable01  {
	MinValue01    minValue ;
}CalculationTable01;

//void CalculationTable01_getMinuteSnapShot();


/*
 *    score   snapshot Table  Types and Variables
 *
 *
 */

#define scoreQMsgSize    3

typedef struct Snapshot01SecItem {
	float   maxRho;
	CPU_INT08U dT_atMaxRho;
}Snapshot01SecItem;

typedef struct Snapshot01{
	CPU_INT08U min;
	Snapshot01SecItem  secItems[60];
}Snapshot01;

typedef  Snapshot01*  PSnapshot01;

typedef struct Snapshot01Box{
	CPU_INT08U min;
	PSnapshot01  pSnapshot;
} Snapshot01Box;

typedef Snapshot01Box* PSnapshot01Box;


Snapshot01Box  scoreTaskMsgs [scoreQMsgSize];  // snapshot will later be a struct union with the other snapshots
void* scoreTaskPMsgs [scoreQMsgSize];
OS_MEM *ScoreTaskMsgMem;
OS_EVENT* JobStateSema;
OS_EVENT *ScoreTaskMsgQ;

/*
 *
 * Score Result, CalcController Types and Variables
 */

#define CalcControllerMsgSize 3

typedef struct TimeDiff {
	CPU_INT08U dT;
	CPU_INT08U tZero;
} TimeDiff;


typedef union ScoreResultContent {
	TimeDiff  timeDiff;
	CPU_INT08U minuteValue;
	CPU_INT08U hourValue;
} ScoreResultContent;

typedef struct ScoreResult {
	CPU_INT08U  resi [5];
	CPU_INT08U  resultType;
	ScoreResultContent content;
}ScoreResult;

ScoreResult CalcControllerTaskMsgs [CalcControllerMsgSize];
void *   CalcControllerTaskPMsgs  [CalcControllerMsgSize];
OS_MEM *  CalcControllerTaskMsgMem ;
OS_EVENT* CalcControllerTaskMsgQ;


// data on sdram
typedef struct sdramSpace{
	CalculationTable01   calcTable01;
	Snapshot01  snapshots [scoreQMsgSize];
} sdramSpace;

typedef volatile sdramSpace* sdP;

sdP sdp = SDRAM;



/*
 * score Methods
 *
 */

void Calculator01_scoreMinute(PSnapshot01 pSnapshot01)
{
	CPU_INT08U  s1;
	info_printf("\n\nSnapshot min %i\n",pSnapshot01->min);
	for  ( s1 = 0; s1 < 59; ++ s1)  {
		info_printf("sec %i   maxRho  %f  dT %i\n",s1, pSnapshot01->secItems[s1].maxRho, pSnapshot01->secItems[s1].dT_atMaxRho);
	}
	info_printf("\n");
}


static  void  ScoreCalculatorMethod (void *p_arg)
{
	INT8U err;
	PSnapshot01 pSnapshot01;
	PSnapshot01Box pSnapshot01Box;


	while (1)  {
		pSnapshot01Box = (PSnapshot01Box)OSQPend(ScoreTaskMsgQ, 1367, &err);
		if (err == OS_NO_ERR) {
			pSnapshot01 = pSnapshot01Box->pSnapshot;

			Calculator01_scoreMinute(pSnapshot01);

	       	 err = OSMemPut(ScoreTaskMsgMem, (void *)pSnapshot01);
			 if (err != OS_NO_ERR) {
				 err_printf("mem put problem score method\n");
			 }
		} else  {
			// probable timeout ok....
			info_printf("msg timeout scoreQ\n");
		}
	}
}

void createScoreThreadItems()
{
	CPU_INT08U err;
    ScoreTaskMsgQ = OSQCreate(&scoreTaskPMsgs[0], scoreQMsgSize);
      if (ScoreTaskMsgQ == 0) {
      	err_printf("error msgQ3\n");   // at least a printf in case of this fatal error
      }
      ScoreTaskMsgMem = OSMemCreate(&scoreTaskMsgs[0], scoreQMsgSize, sizeof(Snapshot01), &err);
      if (err != OS_NO_ERR){
     	 err_printf("err create mem block3\n");
      }
      OSMemNameSet(CommSec100MsgMem, (INT8U*)"ScoreTaskMsgMem", &err);  // ???????????? why and how?
}


// end score methods

/*
 *
 *
 *  snapshot creation and sec100 methods
 *
 */




void resetApp_TimeTickData()
{
	memset(&currentSec100Info,0,sizeof(sec100Info));
}

// enable App_TimeTickHook to get the dcf pin value immediately and forward an event to the sec100 Timer thread

CPU_INT32U  tcnt;


void          App_TimeTickHook        (void)
{
	CPU_INT08U val;
	sec100Info * pSec100Info;
	CPU_INT08U err;
	val = BSP_GPIO_PinGet(AVR32_PIN_PX13);
	OS_SEM_DATA sem_data;

	err = OSSemQuery(JobStateSema, &sem_data);
	if (err != OS_NO_ERR) {
		err_printf("error query sem timetick\n");
	}


	if (sem_data.OSCnt == prepare01 ){
		currentSec100Info.min = -1;
		currentSec100Info.sec = 58;
		currentSec100Info.sec100 = 99;  // start at -1 sec
		OSSemSet(JobStateSema, find01 , &err);
		if (err != OS_NO_ERR) {
			err_printf("error setting Job sema timetick\n");
		}
		err = OSSemQuery(JobStateSema, &sem_data);
		if (err != OS_NO_ERR) {
			err_printf("error query sem timetick 2\n");
		}
	}
	if (sem_data.OSCnt != switchedOff ) {
		if ((++currentSec100Info.sec100) > 99) {
			currentSec100Info.sec100 = 0;
			if ((++currentSec100Info.sec) > 59) {
				currentSec100Info.sec = 0;
				++ currentSec100Info.min;
			}
		}
		currentSec100Info.pinVal = val;
	}
	if (sem_data.OSCnt == find01 ) {
		pSec100Info = (sec100Info *) OSMemGet(CommSec100MsgMem, &err);
		if (pSec100Info != 0 ) {

			*pSec100Info = currentSec100Info;
			err = OSQPost(Sec100TaskQ, (void *)pSec100Info);

			if ( err != OS_NO_ERR) {
				err_printf("Q post err tickHook\n");
			}
		} else {
			err_printf("no more mem to get in tickHook\n");
		}
	}

}




void CalculationTable01_init()
{
	CalculationTable01_reset();
}

void CalculationTable01_close()
{

}


CPU_INT32U sdramp, sdramSize, CTableSize;


void CalculationTable01_reset()
{
	sdramp = ((CPU_INT32U) sdp);
	sdramSize = sizeof(*sdp);
	CTableSize = sizeof (CalculationTable01);
	memset((void *)&sdp->calcTable01,0,sizeof(CalculationTable01));
}

void CalculationTable01_sendMinuteSnapShot(sec100Info *  pSec100Info)
{
	CPU_INT08U  i1, err;
	PSnapshot01 pSnapshot01;
	PSnapshot01Box pSnapshot01Box;


	pSnapshot01Box = (PSnapshot01Box) OSMemGet(ScoreTaskMsgMem, &err);

	if (pSnapshot01Box != 0 ) {
		pSnapshot01 = pSnapshot01Box->pSnapshot;

		pSnapshot01->min = pSec100Info->min;  // actually this value is the number of the next minute,
											  // but does not matter anyhow, PN 4. june 2012
		for(i1 = 0; i1< 59;++i1) {
			pSnapshot01->secItems[i1].maxRho = sdp->calcTable01.minValue.secValues[i1].maxRho;
			pSnapshot01->secItems[i1].dT_atMaxRho = sdp->calcTable01.minValue.secValues[i1].dT_atMaxRho;
		}

		err = OSQPost(ScoreTaskMsgQ, (void *)pSnapshot01);

		if ( err != OS_NO_ERR) {
			err_printf("Q post err pSnapshot01\n");
		}
	}	else{
		err_printf("no memory got snapshot\n");
	}
}



void resetSec100Data()
{
	memset(&lastSec100Info,0,sizeof(sec100Info));
	memset(sec100Array,0,sizeof(sec100Array));
	sec100State = firstSec100;
	lastFxyValue = lastFy2Value = 0;
}

void calc01Correlation(CPU_INT08U* fxy,CPU_INT08U* fy2)
{
	int i1;
	*fxy = *fy2 = 0;
	for(i1=0; i1 < 200; ++i1){
		if (sec100Array[i1] != 0 ) {
			++*fy2;
			if ((i1 >= 100) && (i1 <= 110)) ++ *fxy;
		}
	}
}

void addToCalctable01(sec100Info *  pSec100Info, CPU_INT08U  sFxFy, CPU_INT08U sFy2)
{
	CPU_INT08U  sec100Ind, secInd;
	CPU_INT16U  totSfy2, totSfxy, totSteps;
	float   rho;
	float   divisor2;


	if (pSec100Info->sec100 == 99) {  // a 01-mask ending at sec100==99 represents a 0-point
		sec100Ind = 0;				  // at sec100 == 0 of the same second ... and so on... always -99
		secInd = pSec100Info->sec;
	}  else  {
		sec100Ind = pSec100Info->sec100 + 1;
		if (pSec100Info->sec > 0) {
			secInd = pSec100Info->sec -1;
		} else  {
			secInd = 59;
		}
	}
	totSfy2 = (sdp->calcTable01.minValue.secValues[pSec100Info->sec].sec100Values[pSec100Info->sec100].sEy2 +=  sFy2);
	totSfxy = (sdp->calcTable01.minValue.secValues[pSec100Info->sec].sec100Values[pSec100Info->sec100].sExEy += sFxFy);
	totSteps =(sdp->calcTable01.minValue.secValues[pSec100Info->sec].sec100Values[pSec100Info->sec100].amtSteps ++);
	if (pSec100Info->sec100 == 0) {
		sdp->calcTable01.minValue.secValues[pSec100Info->sec].maxRho = 0;
	}
	divisor2 = totSfy2 * 10 * totSteps;      // fx2 = 10 constant
	rho = totSfxy / (sqrt(divisor2 ));

	if (rho > sdp->calcTable01.minValue.secValues[pSec100Info->sec].maxRho)  {
		sdp->calcTable01.minValue.secValues[pSec100Info->sec].maxRho = rho;
		sdp->calcTable01.minValue.secValues[pSec100Info->sec].dT_atMaxRho = pSec100Info->sec100;
	}
	if ((secInd == 59) && (sec100Ind == 99)){
		CalculationTable01_sendMinuteSnapShot(pSec100Info);
		// send message to scoreQ
	}
}

static  void  Sec100CalculatorMethod (void *p_arg)
{
	INT8U err;
	INT8U dxyInInd, dxyOutInd, dy2InInd, dy2OutInd;
	sec100Info * pSec100Info;
	CPU_INT08U  fxy, fy2;
	CPU_INT08U  cFxy, cFy2;

	dxyOutInd = dxyInInd = dy2OutInd =  dy2InInd = 200;
     while (1) {
    	 pSec100Info = (sec100Info *)OSQPend(Sec100TaskQ, 1033, &err);
    	 if (err == OS_NO_ERR) {
     	  // Message received within 100 ticks!

			 if (++dy2InInd > 200) dy2InInd = 0;
			 if (++dy2OutInd > 200) dy2OutInd = 0;
			 if (++dxyInInd > 200) dxyInInd = 0;
			 if (++dxyOutInd > 200) dxyOutInd = 0;

    		 if ((sec100State != firstSec100) && (pSec100Info->index != lastSec100Info.index + 1)){
    			 // missed event Error
    		 } else {  // valid data
    			 sec100Array[dy2InInd] = pSec100Info->pinVal;
    			 if (sec100State == firstSec100){
    				 fxy= 0;
    				 fy2= 0;
    				 sec100State = initArray;
    			 }  else
    			 if (sec100State == initArray) {
    				 if (dy2InInd == 199){
    					 calc01Correlation(&fxy,&fy2);
    					 addToCalctable01(pSec100Info, fxy, fy2);

    					 dy2OutInd = 200;
    					 dxyInInd = 110;
    					 dxyOutInd = 99;
    					 sec100State = runningCalc;
    				 }
    			 } else
    			 if (sec100State == runningCalc){
    				 fxy = lastFxyValue + sec100Array[dxyInInd] - sec100Array[dxyOutInd];
    				 fy2 = lastFy2Value + sec100Array[dy2InInd] - sec100Array[dy2OutInd];
    				 addToCalctable01(pSec100Info, fxy, fy2);

    				 if (dy2InInd == 199) {
    					 calc01Correlation(&cFxy,&cFy2);
    					 if ((cFxy != fxy) ||(cFy2 != fy2)){
    						 err_printf("wrong add calc cFxy %i cFy2 %i fxy %i fy2 %i",cFxy,cFy2,fxy, fy2);
    					 }
    				 }
    			 }
    			 lastSec100Info = *pSec100Info;
    			 lastFxyValue = fxy;
    			 lastFy2Value = fy2;
				 err = OSMemPut(CommSec100MsgMem, (void *)pSec100Info);
				 if (err != OS_NO_ERR) {
					 err_printf("mem put problem sec100 method\n");
				 }
    		 }
    	 } else {
    	  // Message not received, must have timed out
    		 info_printf("sec100 ms Q timeout\n");
    	 }
    }
}

void createSec100ThreadItems()
{
	CPU_INT08U err;
	/*
	 *
	 * TODO  init input port
	 */

    JobStateSema  = OSSemCreate(switchedOff);

    Sec100TaskQ = OSQCreate(&Sec100TaskQMsg[0], sec100TaskQMsgSz);
     if (Sec100TaskQ == 0) {
     	err_printf("error msgQ\n");   // at least a printf in case of this fatal error
     }
     CommSec100MsgMem = OSMemCreate(&CommSec100Buf[0], sec100MsgMemSize, sizeof(sec100Info), &err);
     if (err != OS_NO_ERR){
    	 err_printf("err create mem block\n");
     }
     OSMemNameSet(CommSec100MsgMem, (INT8U*)"Sec100MsgMem", &err);  // ???????????? why and how?
}

/*
 *
 *   Calculation controller methods
 *
 *   currently just a prototype, ready for first correlation tests
 *
 */



/*$PAGE*/
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description: This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments  : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Note(s)    : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                 used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/


void createCalcControllerThreadItems()
{
	CPU_INT08U  err;

    CalcControllerTaskMsgQ = OSQCreate(&CalcControllerTaskPMsgs[0], CalcControllerMsgSize);
      if (CalcControllerTaskMsgQ == 0) {
      	err_printf("error msgQ4\n");   // at least a printf in case of this fatal error
      }
      CalcControllerTaskMsgMem = OSMemCreate(&CalcControllerTaskMsgs[0], CalcControllerMsgSize, sizeof(ScoreResult), &err);
      if (err != OS_NO_ERR){
     	 err_printf("err create mem block4\n");
      }
      OSMemNameSet(CommSec100MsgMem, (INT8U*)"CalcControllerMsgMem", &err);  // ???????????? why and how?
}



static  void  CalcControllerMethod (void *p_arg)
{
	CPU_INT08U  err;
	ScoreResult * pScoreResult;

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                       /* Determine CPU capacity                                  */
#endif

    BSP_Init();
    init_err_printf();

    resetApp_TimeTickData();


    // create the queues, sema etc. before OS/thread is  started

    createSec100ThreadItems();
    createScoreThreadItems();



    resetSec100Data();
   CalculationTable01_init();
   CalculationTable01_reset();

    AppTaskCreate();                                                    /* Create application tasks                                */


/*    OSSemSet(JobStateSema, prepare01 , &err);
    if (err != OS_NO_ERR) {
    	err_printf("error setting Job sema calcContMeth\n");
    }
*/
    while (1) {                                                         /* Task body, always written as an infinite loop.          */
    	pScoreResult = (ScoreResult *)OSQPend(CalcControllerTaskMsgQ, 6823, &err);
		 if (err == OS_NO_ERR) {


//			take action

	       	 err = OSMemPut(CalcControllerTaskMsgMem, (void *)pScoreResult);
			 if (err != OS_NO_ERR) {
				 err_printf("mem put problem calcCont method\n");
			 }
		} else  {
			// probable timeout ok....
			info_printf("msg timeout CalcCont Q\n");
		}
    }
}

/*
**************************************************************************************************************
*                                        CREATE APPLICATION TASKS
*
* Description: This function creates the application tasks.
*
* Arguments  : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Note(s)    : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                 used.  The compiler should not generate any code for this statement.
**************************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
#if (OS_TASK_NAME_SIZE > 10) && (OS_TASK_STAT_EN > 0)
    CPU_INT08U  err;
#endif
    CPU_INT08U retVal;

#if (OS_TASK_NAME_SIZE > 10) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(SerialQ_TASK_PRIO, (CPU_CHAR *)"ScoreCalc", &err);
#endif


    retVal = OSTaskCreateExt(ScoreCalculatorMethod,
                    (void *)0,
                    (OS_STK *)&ScoreCalculatorMethodStk[ScoreCalculatorMethod_STK_SIZE - 1],
                    ScoreCalculator_TASK_PRIO,
                    ScoreCalculator_TASK_PRIO,
                    (OS_STK *)&ScoreCalculatorMethodStk[0],
                    ScoreCalculatorMethod_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_SIZE > 10) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(ScoreCalculator_TASK_PRIO, (CPU_CHAR *)"ScoreCalc", &err);
#endif


    retVal = OSTaskCreateExt(Sec100CalculatorMethod,
                    (void *)0,
                    (OS_STK *)&Sec100CalculatorMethodStk[Sec100CalculatorMethod_STK_SIZE - 1],
                    Sec100Calculator_TASK_PRIO,
                    Sec100Calculator_TASK_PRIO,
                    (OS_STK *)&Sec100CalculatorMethodStk[0],
                    Sec100CalculatorMethod_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_SIZE > 10) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(Sec100Calculator_TASK_PRIO, (CPU_CHAR *)"Sec100Calc", &err);
#endif

}



/*$PAGE*/
/*
**************************************************************************************************************
*                                                MAIN
**************************************************************************************************************
*/

// bad luck the following methods need (more or less) to be implemented as well  (or some own defines
// needed to be implemented into
// the framework code, what is not so nice for future possible upgrades

// moreover it is not possible to compile an application with OS_TASK_SW_HOOK_EN set to 0
// because this will give a build error in any case what shows that the
// framework code itself is not so completely finished in its development.
// so it seems the easiest just to implement these method, since probable they get optimized out anyhow

void          App_TaskCreateHook      (OS_TCB          *ptcb){}
void          App_TaskDelHook         (OS_TCB          *ptcb){}
void          App_TaskIdleHook        (void){}
void          App_TaskStatHook        (void){}
void          App_TCBInitHook         (OS_TCB          *ptcb){}
void          App_TaskSwHook          (void){}





int  main (void)
{
	tcnt = 0;  // variable for debug only, remove later

   CPU_INT08U retVal;

    CPU_IntDis();               /* Disable all interrupts until we are ready to accept them */

    OSInit();

    createCalcControllerThreadItems();

    retVal = OSTaskCreateExt(CalcControllerMethod,                                       /* Create the start task                                    */
                    (void *)0,
                    (OS_STK *)&CalcControllerMethodStk[CalcControllerMethod_STK_SIZE - 1],
                    CalculationController_TASK_PRIO,
                    CalculationController_TASK_PRIO,
                    (OS_STK *)&CalcControllerMethodStk[0],
                    CalcControllerMethod_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_SIZE > 11) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(CalculationController_TASK_PRIO, (CPU_CHAR *)"CalcControl", &err);
#endif

    OSStart();                                                          /* Start multitasking (i.e. give control to uC/OS-II)       */

    return (0);                                                         /* Avoid warnings for GCC std prototype (int main (void))   */
}
