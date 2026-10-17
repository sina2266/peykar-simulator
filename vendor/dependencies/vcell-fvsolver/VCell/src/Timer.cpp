/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;
using std::setw;
using std::setprecision;

#include <VCELL/Timer.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#define TimerResolution 1E-6
#define DoubleToSec(t)       ((long)(floor((double)(t))))
#define DoubleToUSec(t)      ((long)((((double)(t)) - DoubleToSec(t))/TimerResolution))
#define TimeToDouble(t)      (((double)((t).tv_sec)) + ((double)((t).tv_usec))*TimerResolution)

Timer::Timer()
{
	for (int i=0;i<MAX_NUM_TIMER_ID;i++){
		timerIDList[i]=NULL;
	}
	numTimerID=0;

#if ( !defined(WIN32) && !defined(WIN64) )
	which = ITIMER_VIRTUAL;  
	intervalCount = 0;    // number of cycles of timer
	interval = 40000000.0;    // initialize interval timer to 1000 seconds
	value = 40000000.0;       // initialize value to timer that hasn't started
#else
	clockCount = 0;
#endif
	resetTimer();
}

void Timer::resetTimer()
{
#if ( !defined(WIN32) && !defined(WIN64) )
	time.it_interval.tv_sec  = DoubleToSec(interval);
	time.it_interval.tv_usec = DoubleToUSec(interval);
	time.it_value.tv_sec     = DoubleToSec(value);
	time.it_value.tv_usec    = DoubleToUSec(value);
	if (setitimer(which, &time, NULL)) {
		throw "Timer::resetTimer(), failed";
	}
#else
	clockCount = clock();
#endif
}

TimerHandle Timer::registerID(string& identifier)
{
    //
    // check to see if already exists
    //
    for (int i=0;i<numTimerID;i++){
	if (timerIDList[i]->identifier == identifier){
	    return i;
	}
    }
    //
    // add to list
    //
	if (numTimerID >= MAX_NUM_TIMER_ID) {
		throw "Timer::registerID(), too many timers";
	}
    timerIDList[numTimerID++] = new TimerID(identifier);

    return numTimerID-1;
}

bool Timer::start(TimerHandle ID)
{
	if (ID < 0 || ID >= numTimerID) {
		throw "Timer::start(), timer index out of bound";
	}

	double startTime = readTimer();

	timerIDList[ID]->start(startTime);

	return true;
}

bool Timer::getElapsedTimeSec(TimerHandle ID, double &time)
{
	if (ID < 0 || ID >= numTimerID) {
		throw "Timer::getElapsedTimeSec(), timer index out of bound";
	}

	time =  timerIDList[ID]->getElapsedTimeSec();

	return true;
}

bool Timer::stop(TimerHandle ID)
{
	if (ID < 0 || ID >= numTimerID) {
		throw "Timer::stop(), timer index out of bound";
	}

	double stopTime = readTimer();

	timerIDList[ID]->stop(stopTime);

	return true;
}


void Timer::show()
{
	double totalElapsedTime=0.0;

	for (int i = 0; i < numTimerID; i ++){
		totalElapsedTime += timerIDList[i]->getElapsedTimeSec();
	}
	cout << endl << endl << "Timer::show()" << endl << endl;	
	cout << setw(20) << "identifier" << setw(20) << "average time" << setw(20) << "elapsed time" << setw(20) << "percent usage" << endl;

	for (int i = 0; i < numTimerID; i ++){
		cout << setprecision(5);
		cout << setw(20) << timerIDList[i]->identifier << setw(20) << timerIDList[i]->getAvgElapsedTimeSec() 
			<< setw(20) <<	timerIDList[i]->getElapsedTimeSec() << setw(20) << timerIDList[i]->getElapsedTimeSec()/totalElapsedTime*100.0 << endl;
	}
	cout << endl << endl;
}

void Timer::checkForOverflow()
{
#if ( !defined(WIN32) && !defined(WIN64) )
	getitimer(which, &time);
	double currValue = TimeToDouble(time.it_value);
	if (currValue > value){
		intervalCount++;
		printf("Timer::checkForOverflow(), intervalCount=%d\n",intervalCount);
	}
	value = currValue;
	assert(0);
#endif
}

double Timer::readTimer()
{
#if ( !defined(WIN32) && !defined(WIN64) )
	getitimer(which, &time);
	value = TimeToDouble(time.it_value);
#ifndef VCELL_JTC
	//assert(interval == TimeToDouble(time.it_interval));
#endif
	return ((interval-value));
#else
	clockCount = clock();
	return ((double)clockCount)/CLOCKS_PER_SEC;
#endif
}
//
// user               getrusage
// user + system      getrusage
// wallclock          gettimeofday
// realtime timer .. ticks..    o
// 
//======================================================================
//
//  class TimerID
//
//======================================================================
TimerID::TimerID(string& id)
{
	identifier = id;
	isRunning = false; 
	count = 0;
	reset();
}

void TimerID::reset()
{
	elapsedTime = 0.0;
	startTime = 0.0;
}

void TimerID::start(double currTime)
{
	if (isRunning) {
		throw "TimerID::start(), already started";
	}

	isRunning = true;
	startTime = currTime;
}

void TimerID::stop(double currTime)
{
	if (!isRunning) {
		throw "TimerID::stop(), already stopped";
	}

	isRunning = false;
    
#ifndef VCELL_JTC
	if (currTime < startTime) {
		throw "TimerID::stop(), time out of order";
	}
#endif

	elapsedTime += currTime - startTime;
	count++;
}

double TimerID::getElapsedTimeSec()
{
	return elapsedTime;
}

double TimerID::getAvgElapsedTimeSec()
{
	if (count==0)
		return elapsedTime;
	else
		return elapsedTime/count;
}

void TimerID::show()
{
	cout << "Timer(" << identifier << ") - running=" << isRunning << ", start=" << startTime << ", elapsed=" << elapsedTime << endl; 
}
