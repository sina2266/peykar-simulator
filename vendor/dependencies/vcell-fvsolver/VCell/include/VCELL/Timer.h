/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef TIMER_H
#define TIMER_H

#define MAX_NUM_TIMER_ID 1000

#if ( !defined(WIN32) && !defined(WIN64) )
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <string>
using std::string;

class TimerID {
public:
	TimerID(string& id);
	     		
	void reset();
	void start(double currTime);
	void stop(double currTime);
	double getElapsedTimeSec();
	double getAvgElapsedTimeSec();
	void show();
	
	string identifier;
	double elapsedTime;
	double startTime; 
	int    count; 
	bool   isRunning;
};

typedef int TimerHandle;

class Timer {
public:
	Timer();

	TimerHandle registerID(string& identifier);

	bool start(TimerHandle ID);
	bool stop(TimerHandle ID);
	bool getElapsedTimeSec(TimerHandle ID, double &time);
	void checkForOverflow();    
	void show();
    
protected:
	double readTimer();
	void resetTimer();

#if ( !defined(WIN32) && !defined(WIN64) )
	int which;  // which timer ... ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF 
	struct  itimerval time;
	double  interval;       // value to initialize itimer
	double  value;          // current value of itimer (countdown timer)
	int     intervalCount;  // number of times timer resets.
#else
	clock_t clockCount;     // current value of clock
	//
	// Win32 definitions
	// 
#endif

	TimerID *timerIDList[MAX_NUM_TIMER_ID];
	int      numTimerID;
};

#endif
