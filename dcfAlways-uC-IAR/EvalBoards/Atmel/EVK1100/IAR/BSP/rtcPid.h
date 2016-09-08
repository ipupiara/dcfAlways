
#ifndef  rtcPidH
#define rtcPidH

/*
enum rtcPidSemaStates {
	waitingForNewValues,
	nextValuesSet
} rtcPidSemaStates;
causes link error   rrrgggghhhh */
#define waitingForNewRTCValue 0
#define nextRTCValuesSet      1
#define calculating           2
#define nextCorrectionValuesSet  3
#define nextCorrectionValuesReceived 4

// must be called before bsp init because timetick accessed these objects
void createRtcPidItems();

// to be called generally within AppTaskCreate()
void createRtcPidThread();

extern OS_EVENT*  rtcPidSema;
extern CPU_INT32S   cycleCorrection; 
extern CPU_INT16U intervalVal;
extern CPU_INT32U intervalDurationRTC;
extern CPU_INT32S nextCycleCorrection;

#endif