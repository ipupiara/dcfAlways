
#include <includes.h>
#include "rtcPid.h"
#include "UartPrint.h"

OS_STK  rtcPid_Thread_MethodStk[rtcPid_Thread_Method_STK_SIZE];
OS_EVENT*  rtcPidSema;



////  
////   pn 29. Aug 12,  pid methods to justify cycle time with rtc-clock

#ifdef AdjustCycleToRTC

#define rtcCrystalCyclesPerSec  32768      // 0x8000
#define psel             0       // rtcCyclePerSec = rtcCyclesPerSec * 1/(2 ^ (1 + psel) = rtcCylclesPerSec shr (1+psel)

typedef float real;
int8_t m_started;
real m_kPTot, m_kP, m_kI, m_kD, m_stepTime, m_inv_stepTime, m_error_I_thresh, m_integral;
real corrCarryOver;     // carry amount if correction in float gives zero correction in int

CPU_INT16U intervalVal;
CPU_INT32S   cycleCorrection; 
CPU_INT32S nextCycleCorrection;
CPU_INT32S   m_prev_error; 
//CPU_INT32U  lastRTC;
CPU_INT32U rtcCyclesPerSec;
CPU_INT32U intervalDurationRTC;


/*

void initRTCPid(real kpTot,real kpP, real ki, real kd, real error_I_thresh, real step_time, int interval)
{
    // Initialize controller parameters
	// PN 3.Oct 2011, added m_kP for better setting of proportional factor only
	// though these 4 factors will be linearly dependent
    intervalCnt = 0;
    intervalVal = interval;
    cycleCorrection = 0;
    m_prev_error = 0;
    
	m_kP   = kpP;
    m_kPTot = kpTot;
    m_kI = ki;
    m_kD = kd;
    m_error_I_thresh = error_I_thresh;

    // Controller step time and its inverse
    m_stepTime = step_time * interval;
    m_inv_stepTime = 1 / step_time;

    // Initialize integral and derivative calculations
    m_integral = 0;
    m_started = 0;
	
//	 updateGradAmps();

	 corrCarryOver = 0;
     lastRTC = 0;
     
     if (!rtc_init(&AVR32_RTC, RTC_OSC_32KHZ, psel))   // RTC_PSEL_32KHZ_1HZ))
      {
//        usart_write_line(&AVR32_USART0, "Error initializing the RTC\r\n");
//        while(1);   // pn 30. aug 12, just while debugging
      }
      if (psel == 0) rtc_prescalerClear(&AVR32_RTC);  
     rtcCyclesPerSec =  rtcCrystalCyclesPerSec >> ( 1+psel);
      rtc_enable(&AVR32_RTC);
      lastRTC = rtc_get_value(&AVR32_RTC);
}

#define correctionThreshold  30
#define printfPID

real nextCorrection(CPU_INT32S error)
{
    // Set q_fact to 1 if the error magnitude is below
    // the threshold and 0 otherwise
    real q_fact;
	real res;
    if (abs(error) < m_error_I_thresh)
        q_fact = 1.0;
    else  {
        q_fact = 0.0;
		m_integral = 0.0;
	}

    // Update the error integral
    m_integral += m_stepTime*q_fact*error;

    // Compute the error derivative
    real deriv;
    if (!m_started)
    {
        m_started = 1;
        deriv = 0;
    }
    else
    {
        deriv = (error - m_prev_error) * m_inv_stepTime;
    }    
        // Return the PID controller actuator command
	res = m_kPTot*(m_kP*error + m_kI*m_integral + m_kD*deriv);
    
    m_prev_error = error;
	if (res > correctionThreshold) {
		res = correctionThreshold;
	} else if (res < -1*correctionThreshold) {
		res = -1* correctionThreshold;
	}

#ifdef printfPID
//	double errD = error;
//	double intD = m_integral;
//	double derivD = deriv;
//    info_printf("err %f \n",error);
	info_printf("err %f int %f deriv %e \n",error, m_integral, deriv);
    info_printf("errCor %f  intCor  %f  derivCor   %f\n",m_kPTot*m_kP*error,m_kPTot* m_kI*m_integral,m_kPTot*m_kD*deriv);
#endif
    return res;
}
*/

CPU_INT32S getCycleCorrection()
{  
    
    CPU_SR   cpu_sr;
    CPU_INT32S res;
    OS_ENTER_CRITICAL();
    res = cycleCorrection;
    OS_EXIT_CRITICAL();
    return res;

}



#endif







// must be called before bsp init because timetick accessed these objects
void createRtcPidItems()
{
    rtcPidSema  = OSSemCreate(waitingForNewRTCValue);
    if (NULL == rtcPidSema) {
//            err_printf("error creating Job sema timetick\n");  not yet available
    }
/*    OSSemSet(rtcPidSema, switchedOff , &OErr);
    if (OErr != OS_NO_ERR) {
//            err_printf("error setting Job sema timetick\n");
    }
    */
}

void  rtcPidMethod(void * pData)
{  
    CPU_INT08U OErr; 
    while (1)  {
        OSSemPend(rtcPidSema, 0, &OErr);
        if (OErr == OS_NO_ERR){

            /*
#ifdef  AdjustCycleToRTC 
   
       
    ++intervalCnt; 
    if (intervalCnt == intervalVal) {
        intervalCnt = 0;
        currentRTC = rtc_get_value(&AVR32_RTC);
//        rtcDiffGoal = (rtcCyclesPerSec/OS_TICKS_PER_SEC) * intervalVal;
          rtcDiffGoal = (rtcCyclesPerSec * intervalVal)/OS_TICKS_PER_SEC;  // pn 31.aug 12 will make less errors than above rounding multiplied
        if ((lastRTC < currentRTC) && ((currentRTC - rtcDiffGoal ) < currentRTC)) {    // avoid overflow situation
            rtcError = currentRTC - ( lastRTC + rtcDiffGoal);
            cycleCorrStep  = (CPU_INT32S)  (nextCorrection(rtcError) +0.5);
            cycleCorrection += cycleCorrStep;
            
//            info_printf("rtcError %i cycPT %i, cycCorr %i cyStep %i\n",rtcError,cyclesPerTick, cycleCorrection,cycleCorrStep);
        }
        
        info_printf("currRTC %i   goal %i  rtcDif %i  \n",currentRTC,rtcDiffGoal, (currentRTC-lastRTC));
        
        lastRTC = currentRTC;
    }
        cycle += cycleCorrection;
#endif
                
            
                    
            
        }  else
        {
//            info_printf("rtcPid timeout (resp.  != OS_NO_ERR\n"); 
            */
        }
    
    }
}

// to be called generally within AppTaskCreate()
void createRtcPidThread()
{  
    CPU_INT08U  retVal;
    
      retVal = OSTaskCreateExt(rtcPidMethod,
                    (void *)0,
                    (OS_STK *)&rtcPid_Thread_MethodStk[rtcPid_Thread_Method_STK_SIZE - 1],
                    rtcPid_TASK_PRIO,
                    rtcPid_TASK_PRIO,
                    (OS_STK *)&rtcPid_Thread_MethodStk[0],
                    rtcPid_Thread_Method_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    if (retVal !=  OS_NO_ERR  ) {
        err_printf("error create score calc Thread\n");
    }  
    #if (OS_TASK_NAME_SIZE > 14)
        OSTaskNameSet(rtcPid_TASK_PRIO, (CPU_CHAR *)"rtcPidThread", &retVal);
    #endif

}