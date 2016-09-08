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
//#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

//#include "sdramc.h"

#include "UartPrint.h"
#include "rtcPid.h"


/*
**************************************************************************************************************
*                                               VARIABLES
**************************************************************************************************************
*/

#if uC_TCPIP_MODULE > 0
        NET_IP_ADDR   ip;
        NET_IP_ADDR   msk;
        NET_IP_ADDR   gateway;
        NET_ERR       err;
#endif

OS_STK  CalcControllerMethodStk[CalcControllerMethod_STK_SIZE];
OS_STK  Sec100CalculatorMethodStk[Sec100CalculatorMethod_STK_SIZE];
OS_STK  ScoreCalculatorMethodStk[ScoreCalculatorMethod_STK_SIZE];
OS_STK  tcp_ip_Thread_MethodStk[tcp_ip_Thread_Method_STK_SIZE];

/*
**************************************************************************************************************
*                                           FUNCTION PROTOTYPES
**************************************************************************************************************
*/

#if uC_TCPIP_MODULE > 0
static  void  AppInit_TCPIP(void);
#endif

static  void  CalcControllerMethod(void *p_arg);
static  void  AppTaskCreate(void);
static  void  Sec100CalculatorMethod (void *p_arg);
static  void  ScoreCalculatorMethod (void *p_arg);
void resetSec100Data();
void CalculationTable01_close();
void CalculationTable01_reset();
void resetCalculationData();

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
	add10cs,
    add1cs,
    addSecs
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
*   tcp-ip_Thread Variables and Types
*/


#if uC_TCPIP_MODULE > 0
#define tcp_ip_Buffer_Size   256

char   tcp_ip_RecvBuffer [tcp_ip_Buffer_Size];
char   tcp_ip_SendBuffer [tcp_ip_Buffer_Size];
#endif


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

CPU_INT08U   sec100Array [201];
sec100Info   lastSec100Info;
CPU_INT08U   sec100State;
CPU_INT08U   lastFxyValue, lastFy2Value;
CPU_INT32U   cycle10cs, cycle1cs;


/*
 *
 * Calculation Table types and global variable
 *
 * 
 */

/*

// pn 14. june 2012: might be done with OO-Technology... but wont change now from C to C++ - might be done in somewhen future

class  CalcTable 
{
public:
  CalcTable();
  ~CalcTable();
  
public:
  abstract virtual void addTo(sec100Info *  pSec100Info, CPU_INT08U  sFxFy, CPU_INT08U sFy2) = 0;
private:
  void getMaskCoord(sec100Info *  pSec100Info, CPU_INT08U* sec100Ind,  CPU_INT08U* secInd) = 0;
};

CalcTable::CalcTable() {};
~CalcTable::CalcTable() {};

class  CalcTableAvAr : CalcTable
{
};

class CalcTableTWA()
{
};

*/

#define artihmetic
#ifndef arithmetic
#define  TWA    //time weighted average
#endif

typedef CPU_INT16S sumE2Type;

#ifdef  arithmetic

typedef struct Sec100value01 {
	sumE2Type  sExEy;
	sumE2Type  sEy2;
        sumE2Type  amtSteps;
        float rho;
}Sec100value01;

#endif  

#ifdef  TWA

#define twaArraySize   12

typedef struct Sec100value01 {
	sumE2Type  sExEy[twaArraySize];
	sumE2Type  sEy2[twaArraySize];
        sumE2Type  amtSteps;
        CPU_INT08U  insertIndex;
        float rho;
}Sec100value01;

#endif


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
 *    score and  snapshot Table  Types and Variables
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
CPU_INT08U secondsToAdd;

/*
 *
 * Score Result, CalcController Types and Variables
 */

#define CalcControllerMsgSize 3

typedef struct ZeroPointCoord {
	CPU_INT08U dT;
	CPU_INT08U tZero;
} ZeroPointCoord;

typedef union ScoreResultContent {
	ZeroPointCoord  zeroPointCoord;
	CPU_INT08U minuteValue;
	CPU_INT08U hourValue;
} ScoreResultContent;

typedef struct ScoreResult {
	CPU_INT08U  dummyresi [5];   // take car of minimum rec size, depending on the size of this array
                                //  osMemGet will crash during initialization of its linked list ....????????
	CPU_INT08U  resultType;
	ScoreResultContent content;    
}ScoreResult;





ScoreResult CalcControllerTaskMsgs [CalcControllerMsgSize];
void *   CalcControllerTaskPMsgs  [CalcControllerMsgSize];
OS_MEM *  CalcControllerTaskMsgMem ;
OS_EVENT* CalcControllerTaskMsgQ;


/*
// an old implementation of above:

#define CalcControllerMsgSize 3

typedef struct TimeDiff {
CPU_INT08U dT;
CPU_INT08U tZero;
} TimeDiff;


typedef union ScoreResultContent {
TimeDiff timeDiff;
CPU_INT08U minuteValue;
CPU_INT08U hourValue;
} ScoreResultContent;

typedef struct ScoreResult {
CPU_INT08U resi [5];
CPU_INT08U resultType;
ScoreResultContent content;
}ScoreResult;

ScoreResult CalcControllerTaskMsgs [CalcControllerMsgSize];
void * CalcControllerTaskPMsgs [CalcControllerMsgSize];
OS_MEM * CalcControllerTaskMsgMem ;
OS_EVENT* CalcControllerTaskMsgQ;

*/




/*
*
*   score types
*
*/

#define amtBars 10
#define amtTopHiz 5

typedef struct topHit {
    float rho;
    CPU_INT08U   atSec;
    CPU_INT08U   atSec100;
}  topHit;

typedef struct TopHisto   {
    float       maxRho, minRho, avgRho, varRho;
    CPU_INT08U  bars[amtBars];
    topHit      topHiz[amtTopHiz];
//    float       barBoarders [amtBars + 1];
}TopHisto;

// data on sdram
typedef struct sdramSpace{
	CalculationTable01   calcTable01;
	Snapshot01  snapshots [scoreQMsgSize];
    TopHisto     topHisto;
} sdramSpace;

typedef volatile sdramSpace* sdP;

sdP sdp = SDRAM;


/*  
*    tcp-ip Methods
*/

#if uC_TCPIP_MODULE > 0

#define UDP_SERVER_PORT 10001

static  void  tcp_ip_Thread_Method (void *p_arg)
{
    NET_SOCK_ID sock;
    NET_SOCK_ADDR_IP server_sock_addr_ip;
    NET_SOCK_ADDR_LEN server_sock_addr_ip_size;
    NET_SOCK_ADDR_IP client_sock_addr_ip;
    NET_SOCK_ADDR_LEN client_sock_addr_ip_size;
    NET_SOCK_RTN_CODE rx_size, tx_size;
    CPU_BOOLEAN attempt_rx;
    NET_ERR err;
    CPU_INT08U secB;
	CPU_INT16S minB;
    CPU_INT08U continueInt;
    
    continueInt = 1;  
    sock = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4, 
                            NET_SOCK_TYPE_DATAGRAM,
                            NET_SOCK_PROTOCOL_UDP,
                            &err);
    if (err != NET_SOCK_ERR_NONE) {
        err_printf("NetSock Open failed\n");
    }
      
    server_sock_addr_ip_size = sizeof(server_sock_addr_ip); 
    Mem_Clr((void *)&server_sock_addr_ip,   (CPU_SIZE_T) server_sock_addr_ip_size);
    
    server_sock_addr_ip.Family = NET_SOCK_ADDR_FAMILY_IP_V4;
    server_sock_addr_ip.Addr = NET_UTIL_HOST_TO_NET_32(NET_SOCK_ADDR_IP_WILD_CARD);
    server_sock_addr_ip.Port = NET_UTIL_HOST_TO_NET_16(UDP_SERVER_PORT);
    
    NetSock_Bind((NET_SOCK_ID ) sock, 
                    (NET_SOCK_ADDR *)&server_sock_addr_ip,
                    (NET_SOCK_ADDR_LEN) NET_SOCK_ADDR_SIZE,
                    (NET_ERR *)&err);
    if (err != NET_SOCK_ERR_NONE) {
         NetSock_Close(sock, &err);
         err_printf("Net sock Bind failed\n");
    }
          
    while (continueInt)  {
        do {
                client_sock_addr_ip_size = sizeof(client_sock_addr_ip);
                
                rx_size = NetSock_RxDataFrom((NET_SOCK_ID ) sock, 
                                                (void *) tcp_ip_RecvBuffer,
                                                (CPU_INT16S ) sizeof(tcp_ip_RecvBuffer),
                                                (CPU_INT16S ) NET_SOCK_FLAG_NONE,
                                                (NET_SOCK_ADDR *)&client_sock_addr_ip,
                                                (NET_SOCK_ADDR_LEN *)&client_sock_addr_ip_size,
                                                (void *) 0,
                                                (CPU_INT08U ) 0,
                                                (CPU_INT08U *) 0,
                                                (NET_ERR *)&err);
                switch (err) {
                    case NET_SOCK_ERR_NONE:
                        attempt_rx = DEF_NO;
                        break;
                    case NET_SOCK_ERR_RX_Q_EMPTY:
                    case NET_OS_ERR_LOCK:
                        attempt_rx = DEF_YES;
                        break;
                    default:
                        attempt_rx = DEF_NO;
                    break;
                }
        } while (attempt_rx == DEF_YES);
        info_printf("Net received %i  bytes : %s\n",rx_size,tcp_ip_RecvBuffer);
       
        secB = currentSec100Info.sec;
        minB = currentSec100Info.min;
        //   not threadsave, but if ever an accident happens, it is of no consequence in this case.... murphis law...  :-)
        
        snprintf((char*)&tcp_ip_SendBuffer,sizeof(tcp_ip_SendBuffer),"dcfAlways Server at min %i sec %i - still prototye",minB,secB);
  
        tx_size = NetSock_TxDataTo((NET_SOCK_ID ) sock, 
                                    (void *) tcp_ip_SendBuffer,
                                    (CPU_INT16S ) strlen((char*) tcp_ip_SendBuffer)+ 1,
                                    (CPU_INT16S ) NET_SOCK_FLAG_NONE,
                                    (NET_SOCK_ADDR *)&client_sock_addr_ip,
                                    (NET_SOCK_ADDR_LEN) client_sock_addr_ip_size,
                                    (NET_ERR *)&err);
                                            
        info_printf("net sent (ret error code %i) %i bytes : &s\n",err,tx_size, tcp_ip_SendBuffer);                                   
    }
    NetSock_Close(sock, &err); 
    if (err != NET_SOCK_ERR_NONE) {
        err_printf("Net Sock Close error\n");
    }      
}
#endif


/*
 * score Methods
 *
 */


void  Calculator01_establishHisto(PSnapshot01 pSnapshot01)
{
    CPU_INT08S  i1, i2, i3;
    float avgRho = 0;
    float sigm = 0;
    float boarder;
    float maxRho, minRho, rho, barWith;
    memset((void *) &sdp->topHisto,0, sizeof(TopHisto));
    minRho = 1.0; maxRho = -1.0;
    for (i1 = 0; i1 <= 59; ++i1) {
        avgRho += (rho = pSnapshot01->secItems[i1].maxRho);
        if (maxRho < rho) maxRho = rho;
        if (minRho > rho) minRho = rho;
    }
    sdp->topHisto.avgRho =  (avgRho /= 60);    // calculate avg
    sdp->topHisto.minRho = minRho;
    sdp->topHisto.maxRho = maxRho;
    barWith = (maxRho - minRho) / amtBars;
    for (i1 = 0; i1 <= 59; ++i1) {
        sigm += pow(avgRho - (rho= pSnapshot01->secItems[i1].maxRho), 2);        //  calculate variance
        i2 = 0;
        boarder = minRho + barWith;
        while((rho > boarder) && (i2 < amtBars)) {                         // populate histo
            ++ i2;
            boarder += barWith;
        }
        sdp->topHisto.bars[i2]++;
        i2 = amtTopHiz-1;
                                                           // && is shortcut, therefor no illegal data access at -1
        while( (i2 >= -1) && (rho <= sdp->topHisto.topHiz[i2].rho) ) {            // populate topHiz
            i2 --;
        }
        if (i2 > -1) {
          for (i3 = 0 ; i3 <= i2-1; ++ i3) {
              sdp->topHisto.topHiz[i3] = sdp->topHisto.topHiz[i3+1];
          }
            sdp->topHisto.topHiz[i2].rho = rho;
            sdp->topHisto.topHiz[i2].atSec = i1;
            sdp->topHisto.topHiz[i2].atSec100 = pSnapshot01->secItems[i1].dT_atMaxRho;
        }
    }
    sigm = sqrt(sigm / 60);                                           // calculate variance
    sdp->topHisto.varRho = sigm;
}

/// scoring methods: these are just first prototypes, to be refind later during tests   (pn, 19. june 12)

#define mitDistRelTreshold 1.0
#define topPointsDistandThreshold 0.6
#define topPointsSameMaxRelRholDistThreshold  0.1
#define topPointsSameMaxSec100DistThreshold 20


CPU_INT08U topPointsDistant(CPU_INT08U p1, CPU_INT08U p2)
{
    float diff, relDiff, p1Rho, p2Rho;
    p1Rho = sdp->topHisto.topHiz[p1].rho;
    p2Rho = sdp->topHisto.topHiz[p2].rho;
    diff = fabs(p1Rho - p2Rho);
    relDiff = diff / sdp->topHisto.varRho;
    
    info_printf("topPoints p1 %i  p2 %i  Distant relDiff %f tresh %f\n",p1,p2,relDiff,topPointsDistandThreshold );
  
    return (relDiff > topPointsDistandThreshold);
}

CPU_INT08U topPointsSameMax(CPU_INT08U p1, CPU_INT08U p2)
{
    CPU_INT08U res = 0;
    CPU_INT08U s1, s2, s1c, s2c;
    s1 = sdp->topHisto.topHiz[p1].atSec;
    s2 = sdp->topHisto.topHiz[p1].atSec;
    
    info_printf("top Points %i, %i sameMax ",p1,p2);
    
    if (labs( s1 - s2 ) == 1) {
        float diff, relDiff, p1Rho, p2Rho;
        CPU_INT08S distS100;
        p1Rho = sdp->topHisto.topHiz[p1].rho;
        p2Rho = sdp->topHisto.topHiz[p2].rho;
        diff = p1Rho - p2Rho;
        relDiff = diff / sdp->topHisto.varRho;
        info_printf(" neigbours relDiff %f, treshold %f ",relDiff, topPointsSameMaxRelRholDistThreshold);
        if (relDiff < topPointsSameMaxRelRholDistThreshold) {                        // p1 and p2 lie together (in rho value)
            s1c = sdp->topHisto.topHiz[p1].atSec100;
            s2c = sdp->topHisto.topHiz[p1].atSec100;
            if (s1 > s2) distS100 = s1c+ (100 - s2c); else 
                distS100 = s2c + (100 - s1c);                   
            if (distS100 < topPointsSameMaxSec100DistThreshold) res = 1;            // assume they represent the same maximum correlation (correlation on boarder)    
        info_printf(" distS100 %i tresh %i",distS100,topPointsSameMaxSec100DistThreshold);
        }
    }
    info_printf("returns res %i",res);
    return (res);
}

CPU_INT08U midDistant(CPU_INT08U p1)
{   CPU_INT08U res = 0;
    float avgR = sdp->topHisto.avgRho;
    float dist = fabs( avgR - sdp->topHisto.topHiz[p1].rho);
    float relDist = dist / sdp->topHisto.varRho;
    if (relDist > mitDistRelTreshold) res = 1;
    info_printf("midDistant %i  %f",p1, relDist);
    return (res);
}


void Calculator01_scoreMinute(PSnapshot01 pSnapshot01)
{
      CPU_INT08U   err;
      ScoreResult*  pScoreRes;
      info_printf("\n\nSnapshot min %i\n",pSnapshot01->min);
 
       Calculator01_establishHisto(pSnapshot01);
       
       // just a first prototype for a very first testing,  pn 19. june 2012
       if  (midDistant(1) & (topPointsDistant(1,2) || (topPointsSameMax(1,2) && topPointsDistant(1,3)) )) {
           
         
            info_printf("01 correlation found Zero Point at %i ",      sdp->topHisto.topHiz[1].atSec, sdp->topHisto.topHiz[1].atSec100);      
            
            
            pScoreRes = OSMemGet(CalcControllerTaskMsgMem, &err);
            if (err != OS_NO_ERR) {
                 err_printf("mem get problem score minute\n");
            }
            
            
            err = OSQPost(CalcControllerTaskMsgQ, (void *)pScoreRes);
            if ( err != OS_NO_ERR) {
                err_printf("Q post err pSnapshot01\n");
            }
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

	       	 err = OSMemPut(ScoreTaskMsgMem, (void *)pSnapshot01Box);
			 if (err != OS_NO_ERR) {
				 err_printf("mem put problem score method\n");
			 }
		} else  {
			// probable timeout ok....
//			info_printf("msg timeout scoreQ\n");
		}
	}
}

void initSnapshotData()
{
    CPU_INT08U cnt;
    for (cnt = 0; cnt < scoreQMsgSize; ++cnt)  {
        scoreTaskMsgs[cnt].pSnapshot = (Snapshot01 *)&sdp->snapshots[cnt];
    }

}

void createScoreThreadItems()
{
	CPU_INT08U err;
    ScoreTaskMsgQ = OSQCreate(&scoreTaskPMsgs[0], scoreQMsgSize);
      if (ScoreTaskMsgQ == 0) {
      	err_printf("error msgQ3\n");   // at least a printf in case of this fatal error
      }
      ScoreTaskMsgMem = OSMemCreate(&scoreTaskMsgs[0], scoreQMsgSize, sizeof(Snapshot01Box), &err);
      if (err != OS_NO_ERR){
     	 err_printf("err create mem block3\n");
      }
      OSMemNameSet(CommSec100MsgMem, (INT8U*)"ScoreTaskMsgMem", &err);  
      initSnapshotData();
}




// end score methods

/*
 *
 *
 *  snapshot creation, RTPC-CYCLE PID, and sec100 methods
 *
 */

void createApp_TimeTickData()
{
    CPU_INT08U OErr;
    JobStateSema  = OSSemCreate(switchedOff);
    if (NULL == JobStateSema) {
            err_printf("error creating Job sema timetick\n");
    }
    OSSemSet(JobStateSema, switchedOff , &OErr);
    if (OErr != OS_NO_ERR) {
            err_printf("error setting Job sema timetick\n");
    }
}



void resetApp_TimeTickData()
{
  CPU_INT08U err;
    OSSemSet(JobStateSema, switchedOff , &err);
    if (err != OS_NO_ERR) {
            err_printf("error setting Job sema timetick\n");
    }
    memset(&currentSec100Info,0x00,sizeof(sec100Info));	
    cycle10cs = (CPU_CLK_FREQ() / 10);
    cycle1cs  = (CPU_CLK_FREQ() / 100);
    info_printf("clk running at: %i, 10cs= %i  1cs=%i\n",CPU_CLK_FREQ(),cycle10cs,cycle1cs);
}

// enable App_TimeTickHook to get the dcf pin value immediately and forward an event to the sec100 Timer thread


void          App_TimeTickHook        (void)
{
	CPU_INT08U val;
	sec100Info * pSec100Info;
	CPU_INT08U err;
	val = BSP_GPIO_PinGet(AVR32_PIN_PB20);
	OS_SEM_DATA sem_data;

	err = OSSemQuery(JobStateSema, &sem_data);
	if (err != OS_NO_ERR) {
		err_printf("error query sem timetick\n");
	}

    if (sem_data.OSCnt == idleCount ) {
        switch (sem_data.OSCnt)  {  // pn 29. Aug 12, Time adjust still tobe tested
            case add10cs:  
                CPU_SysReg_Set_Compare(CPU_SysReg_Get_Compare() + cycle10cs);
                OSSemSet(JobStateSema, idleCount , &err);
                if (err != OS_NO_ERR) {
                    err_printf("error setting Job sema add10ms timetick\n");
                }
                break;
             case add1cs:  
                CPU_SysReg_Set_Compare(CPU_SysReg_Get_Compare() + cycle1cs);
                OSSemSet(JobStateSema, idleCount , &err);
                if (err != OS_NO_ERR) {
                    err_printf("error setting Job sema add1ms timetick\n");
                }
                break;              
        }
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
                ++currentSec100Info.index;
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
//			err_printf("no more mem to get in tickHook\n");
		}
	}
}


void CalculationTable01_close()
{

}

void CalculationTable01_reset()
{   
//    CPU_INT32U sz= sizeof(CalculationTable01);
//    memset((void *)&sdp->calcTable01,0x00,sizeof(CalculationTable01));
//    info_printf("CalcTable01_reset with size %i \n",sz);
}

void CalculationTable01_sendMinuteSnapShot(sec100Info *  pSec100Info)
{
	CPU_INT08U  i1, err;
	PSnapshot01 pSnapshot01;
	PSnapshot01Box pSnapshot01Box;


	pSnapshot01Box = (PSnapshot01Box) OSMemGet(ScoreTaskMsgMem, &err);

	if (pSnapshot01Box != 0 ) {
		pSnapshot01 = pSnapshot01Box->pSnapshot;

		pSnapshot01->min = pSec100Info->min - 1;  // actually this value is the number of the next minute,
											  
		for(i1 = 0; i1< 60;++i1) {
			pSnapshot01->secItems[i1].maxRho = sdp->calcTable01.minValue.secValues[i1].maxRho;
			pSnapshot01->secItems[i1].dT_atMaxRho = sdp->calcTable01.minValue.secValues[i1].dT_atMaxRho;
		}

		err = OSQPost(ScoreTaskMsgQ, (void *)pSnapshot01Box);

		if ( err != OS_NO_ERR) {
			err_printf("Q post err pSnapshot01\n");
		}
	}	else{
		err_printf("no memory got snapshot\n");
	}
}

#define secStringSz 80
char  secString1 [secStringSz];
char  secString2 [secStringSz];

void resetSec100Data()
{
	memset(&lastSec100Info,0x00,sizeof(sec100Info));
	memset(sec100Array,0x00,sizeof(sec100Array));
	sec100State = firstSec100;
	lastFxyValue = lastFy2Value = 0;
    memset(secString1,0,sizeof(secString1));
     memset(secString2,0,sizeof(secString2));
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

void getMaskCoord(sec100Info *  pSec100Info, CPU_INT08U* sec100Ind,  CPU_INT08U* secInd)
{
	if (pSec100Info->sec100 == 99) {  // a 01-mask ending at sec100==99 represents a 0-point
		*sec100Ind = 0;	          // at sec100 == 0 of the same second ... and so on... always -99
		*secInd = pSec100Info->sec;
	}  else  {
		*sec100Ind = pSec100Info->sec100 + 1;
		if (pSec100Info->sec > 0) {
			*secInd = pSec100Info->sec -1;
		} else  {
			*secInd = 59;
		}
	}
                
    if (pSec100Info->pinVal == 0) {
      LED_Off(LED0);  
      if (pSec100Info->sec100 < 60)
        strncat(secString1,".",secStringSz - 1);
      else 
        strncat(secString2,".",secStringSz - 1);
    }  else {
        LED_On(LED0);
      if (pSec100Info->sec100 < 60)
        strncat(secString1,"X",secStringSz - 1);
      else
        strncat(secString2,"X",secStringSz - 1);
    }
    if ( pSec100Info->sec100 == 99) {
        if (pSec100Info->sec100 == 99) strcat(secString2,"\n"); // pn 27. jul 12, no size check needed because strlen will be at about 40 and thats ok 
        info_printf(secString1); //info_printf("->sec100 %i\n",pSec100Info->sec100);
        info_printf(secString2);
        memset(secString1,0,sizeof(secString1));
         memset(secString2,0,sizeof(secString2));
    }
}

#ifdef average

void CalculatorTable01_addFxyValues( CPU_INT16U sFxFy, CPU_INT16U sFy2, CPU_INT08U  sec100Ind, CPU_INT08U  secInd)
{
    CPU_INT16U  totSfy2, totSfxy, totSteps;  
    float   rhoV;
	float   divisor2;
        
    totSfy2 = (sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sEy2 +=  sFy2);
	totSfxy = (sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sExEy += sFxFy);
	totSteps =(++sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].amtSteps);
        
	divisor2 = totSfy2 * 10 * totSteps;      // fx2 = 10 constant
        if (divisor2 > 1E-5) {
	    rhoV = totSfxy / (sqrt(divisor2 ));
        } else {
            rhoV = 0;                           // set rho 0 if totSfy2 is 0 (= no correlation)
        }
        sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].rho = rhoV;
}

#endif

#ifdef TWA



void CalculatorTable01_addFxyValues( CPU_INT16U sFxFy, CPU_INT16U sFy2, CPU_INT08U  sec100Ind, CPU_INT08U  secInd)
{
    CPU_INT16U  totSfy2, totSfxy, totSfx2;  
    float   rhoV;
    float   divisor2;
    CPU_INT08U   insertInd, cnt, steps,  aSteps ;
    CPU_INT08S ind;
    
    if ((sec100Ind == 0) &&(secInd == 3)) {
 //       info_printf("debug break");
    }
    
    
    insertInd  = sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].insertIndex;
    sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sEy2[insertInd] = sFy2;
    sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sExEy[insertInd]  = sFxFy;
    
    aSteps = (++sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].amtSteps);
    
    ind = insertInd;
    steps = 0;  totSfy2= 0; totSfxy = 0; totSfx2 = 0;
    for (cnt = 0; cnt < twaArraySize; ++ cnt)  {
//        CPU_INT16U fy2 = sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sEy2[ind];
        if (cnt < aSteps) {
            totSfy2 += sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sEy2[ind] * (twaArraySize - cnt);
            totSfxy += sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].sExEy[ind] * (twaArraySize - cnt);
            totSfx2 += 10 * (twaArraySize - cnt);
            ++steps;
        }   
        --ind;
        if (ind < 0) ind = twaArraySize -1;
    
    }  
    if ((++sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].insertIndex) >= twaArraySize) {
        sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].insertIndex =0;   
    }  
    
    divisor2 = totSfy2 * totSfx2 ;      
    if (divisor2 > 1E-5) {
        rhoV = totSfxy / (sqrt(divisor2 ));    // divisor (n/2)*(n-1) of TWA formula cancels down
    } else {
        rhoV = 0;                           // set rho 0 if totSfy2 is 0 (= no correlation)
    }
    sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].rho = rhoV;
        
}

#endif

float CalculatorTable01_getRho(CPU_INT08U  sec100Ind, CPU_INT08U  secInd )
{
  float   rhoV;
  rhoV =  sdp->calcTable01.minValue.secValues[secInd].sec100Values[sec100Ind].rho;
  return rhoV;
}



void addToCalctable01(sec100Info *  pSec100Info, CPU_INT08U  sFxFy, CPU_INT08U sFy2)
{
	CPU_INT08U  sec100Ind, secInd;
//    CPU_INT32U  aI, aNSI;
	float   rho;

        getMaskCoord(pSec100Info, &sec100Ind, &secInd);        
	
        CalculatorTable01_addFxyValues(sFxFy, sFy2,   sec100Ind, secInd);
               
        rho =  CalculatorTable01_getRho( sec100Ind,   secInd );
        
	if (sec100Ind == 0) {     
            sdp->calcTable01.minValue.secValues[secInd].maxRho = 0;
            sdp->calcTable01.minValue.secValues[secInd].dT_atMaxRho = 0;        
	}        
	if (rho > sdp->calcTable01.minValue.secValues[secInd].maxRho)  {
		  sdp->calcTable01.minValue.secValues[secInd].maxRho = rho;
		  sdp->calcTable01.minValue.secValues[secInd].dT_atMaxRho = sec100Ind;
	}       
        if (sec100Ind == 99) {
            float  mr = sdp->calcTable01.minValue.secValues[secInd].maxRho;
            INT8U mrAt = sdp->calcTable01.minValue.secValues[secInd].dT_atMaxRho;
            
/*            CPU_IntDis();
            aI = amtUSBInterrupts;
            aNSI = amtUSBNonSofInterrupts;
            CPU_IntEn();
            info_printf("  sec %i mRho %f dtAtmr  %i amtUSBInt %i amtNonSofI %i\n", secInd,mr, mrAt,aI, aNSI );
*/            
             info_printf("  sec %i mRho %f dtAtmr  %i\n", secInd,mr, mrAt );
//             info_printf("mask sec100Ind %i  clock->sec100Ind %i \n",sec100Ind, pSec100Info->sec100 );
        } 
	if ((secInd == 59) && (sec100Ind == 99)){
		      CalculationTable01_sendMinuteSnapShot(pSec100Info);
              info_printf("minute snapshot %i \n",pSec100Info->min-1);     
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
            if (sec100State == firstSec100)  {dy2InInd = 200;}

                 if (++dy2InInd > 200) dy2InInd = 0;
                 if (++dy2OutInd > 200) dy2OutInd = 0;
                 if (++dxyInInd > 200) dxyInInd = 0;
                 if (++dxyOutInd > 200) dxyOutInd = 0;

    		 if ((sec100State != firstSec100) && (pSec100Info->index != lastSec100Info.index + 1)){
    			 // missed event Error
                      err_printf("missed event sec100\n");
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



void advanceClock(CPU_INT08U  secs, CPU_INT08U cs)
{
    // idleCount state is assumed
    CPU_INT08U err;
    secondsToAdd = secs;
    OS_SEM_DATA sem_data;
    OSSemSet(JobStateSema, addSecs , &err);
    if (err != OS_NO_ERR) {
            err_printf("error setting Job sema addSecs AdvCl\n");
    }
    do {
        OSTimeDly(2);
        err = OSSemQuery(JobStateSema, &sem_data);
        if (err != OS_NO_ERR) {
            err_printf("error query sem timetick\n");
        }
    } while (sem_data.OSCnt != idleCount);  
    while (cs > 0)  {
        if (cs > 9) {
            OSSemSet(JobStateSema, add10cs , &err);
            if (err != OS_NO_ERR) {
                    err_printf("error setting Job sema addSecs AdvCl\n");
            }
            cs -= 10;
        } else {
            OSSemSet(JobStateSema, add1cs , &err);
            if (err != OS_NO_ERR) {
                    err_printf("error setting Job sema addSecs AdvCl\n");
            }
            cs -= 1;
        }
        do {
            OSTimeDly(2);
            err = OSSemQuery(JobStateSema, &sem_data);
            if (err != OS_NO_ERR) {
                err_printf("error query sem timetick\n");
            }
        } while (sem_data.OSCnt != idleCount);  
    }
}

void resetCalculationData()
{
    resetSec100Data();
    CalculationTable01_reset(); 
    
}

#if uC_TCPIP_MODULE > 0

static void AppInit_TCPIP (void)
{
#if EMAC_CFG_MAC_ADDR_SEL == EMAC_MAC_ADDR_SEL_CFG
    NetIF_MAC_Addr[0] = 0x00;
    NetIF_MAC_Addr[1] = 0x50;
    NetIF_MAC_Addr[2] = 0xC2;
    NetIF_MAC_Addr[3] = 0x25;
    NetIF_MAC_Addr[4] = 0x60;
    NetIF_MAC_Addr[5] = 0x01;
#endif

#if uC_TTCP_MODULE > 0
    BSP_USART_Init(TTCP_COMM_SEL, 38400);                               /* Initialize the TTCP UART for command entry               */
#endif

    err = Net_Init();                                                   /* Initialize uC/TCP-IP                                     */

    ip      = NetASCII_Str_to_IP((CPU_CHAR *)"192.168.1.155",  &err);
    msk     = NetASCII_Str_to_IP((CPU_CHAR *)"255.255.255.0", &err);
    gateway = NetASCII_Str_to_IP((CPU_CHAR *)"192.168.1.1",   &err);

    err     = NetIP_CfgAddrThisHost(ip, msk);
    err     = NetIP_CfgAddrDfltGateway(gateway);

#if uC_TTCP_MODULE > 0
    TTCP_Init();                                                        /* Initialize uC/TTCP                                       */
#endif
    info_printf("tcp/ip network initialized ...\n");
}
#endif


static  void  CalcControllerMethod (void *p_arg)
{
	CPU_INT08U  err;
	ScoreResult * pScoreResult;
    
     createRtcPidItems();         //  these two must be
     createApp_TimeTickData();   // called before timetick starts   
     BSP_Init();

#if OS_TASK_STAT_EN > 0
    OSStatInit();                      /* Determine CPU capacity                                  */
#endif
       
    init_err_printf();

#if uC_TCPIP_MODULE > 0
    AppInit_TCPIP();                                                    /* Initialize uC/TCP-IP and associated appliations          */
#endif 
    
//    initUSB();      // pn 31 aug 12 temporarely disabled
    
    createSec100ThreadItems();      // create the queues, sema etc. before OS/thread is  started
    createScoreThreadItems();  
    createCalcControllerThreadItems();
      // initialize all of the micrium os data before any micrium os activity starts
                        // since using any uninitialized data means misbehaviour of the whole application, pn, 22 .june 2012
    
    resetApp_TimeTickData();      // must be after BSP_Init, resp after clock freqency is set, before job begins
    resetCalculationData();   // initialize data before BSP_init to initialize appTimeTick data as well

    AppTaskCreate();     
    
 /* Create application tasks                                */

    OSSemSet(JobStateSema, prepare01 , &err);
    if (err != OS_NO_ERR) {
    	err_printf("error setting Job sema calcContMeth\n");
    }

    while (1) {                                                         /* Task body, always written as an infinite loop.          */
    	pScoreResult = (ScoreResult *)OSQPend(CalcControllerTaskMsgQ, 6823, &err);
		 if (err == OS_NO_ERR) {
           if (pScoreResult->resultType == foundTdt) {
                OSSemSet(JobStateSema, idleCount , &err);       
                if (err != OS_NO_ERR) {
                        err_printf("error setting Job sema addSecs AdvCl\n");
                }
                OSTimeDly(2);    // let the system become idle
           
//                advanceClock(pScoreResult->content.zeroPointCoord.dT,pScoreResult->content.zeroPointCoord.tZero);
           }


//			take action

	       	 err = OSMemPut(CalcControllerTaskMsgMem, (void *)pScoreResult);
			 if (err != OS_NO_ERR) {
				 err_printf("mem put problem calcCont method\n");
			 }
		} else  {
			// probable timeout ok....
//			info_printf("msg timeout CalcCont Q\n");
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
#if (OS_TASK_NAME_SIZE > 10) 
    CPU_INT08U  err;
#endif
    CPU_INT08U retVal;

#if (OS_TASK_NAME_SIZE > 10) 
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
    if (retVal !=  OS_NO_ERR  ) {
        err_printf("error create score calc Thread\n");
    }

#if (OS_TASK_NAME_SIZE > 10) 
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
    
    if (retVal !=  OS_NO_ERR  ) {
        err_printf("error create sec100 Calc Thread\n");
    }

#if (OS_TASK_NAME_SIZE > 10) 
    OSTaskNameSet(Sec100Calculator_TASK_PRIO, (CPU_CHAR *)"Sec100Calc", &err);
#endif
    
    createRtcPidThread();   

#if uC_TCPIP_MODULE > 0   
    
    retVal = OSTaskCreateExt(tcp_ip_Thread_Method,
                    (void *)0,
                    (OS_STK *)&tcp_ip_Thread_MethodStk[tcp_ip_Thread_Method_STK_SIZE - 1],
                    tcp_ip_Thread_TASK_PRIO,
                    tcp_ip_Thread_TASK_PRIO,
                    (OS_STK *)&tcp_ip_Thread_MethodStk[0],
                    tcp_ip_Thread_Method_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    
    if (retVal !=  OS_NO_ERR  ) {
        err_printf("error create sec100 tcp_ip Thread\n");
    }

#if (OS_TASK_NAME_SIZE > 10) 
    OSTaskNameSet(tcp_ip_Thread_TASK_PRIO, (CPU_CHAR *)"tcp_ip", &err);
#endif
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
    serialOn = 0;    // some kind of C++ constructor code, should be done at application-load

    CPU_INT08U retVal;

    CPU_IntDis();               /* Disable all interrupts until we are ready to accept them */

    OSInit();

    retVal = OSTaskCreateExt(CalcControllerMethod,                                       /* Create the start task                                    */
                    (void *)0,
                    (OS_STK *)&CalcControllerMethodStk[CalcControllerMethod_STK_SIZE - 1],
                    CalculationController_TASK_PRIO,
                    CalculationController_TASK_PRIO,
                    (OS_STK *)&CalcControllerMethodStk[0],
                    CalcControllerMethod_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    
    if (retVal !=  OS_NO_ERR  ) {
        err_printf("error create calcController Thread\n");
    }

#if (OS_TASK_NAME_SIZE > 11) && (OS_TASK_STAT_EN > 0)
    OSTaskNameSet(CalculationController_TASK_PRIO, (CPU_CHAR *)"CalcControl", &err);
#endif

    OSStart();                                                          /* Start multitasking (i.e. give control to uC/OS-II)       */

    return (0);                                                         /* Avoid warnings for GCC std prototype (int main (void))   */
}
