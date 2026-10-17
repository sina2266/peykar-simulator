// for gcc-2.96 (but not 3.1 or 3.2) including sys/wait.h in its normal place
// below when HAVE_SSTREAM there is a problem with __WAIT_STATUS and WNOHANG
#include <sys/wait.h>

#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#define __USE_XOPEN 1		// Necessary for compilation with glibc 2.1.
#ifndef _LANGUAGE_C_PLUS_PLUS
#define _LANGUAGE_C_PLUS_PLUS 1 //gcc-2.8.1 mips-sgi-irix6.2
#endif

// Dispatcher provides an interface to the "select" system call.

#include <Dispatch/dispatcher.h>
#include <Dispatch/iohandler.h>
#include <OS/memory.h>
#include <OS/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#undef NULL
#include <sys/param.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

/* no standard place for this */

extern "C" {
#ifdef NEED_GETTIMEOFDAY_PROTOTYPE // #if (defined(__DECCXX) || (defined(__GNUC__) && !defined(SVR4)))
    extern int gettimeofday(struct timeval*, struct timezone*);
#endif
}

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif

#ifdef LINUX
// This doesn't appear to be necessary any more, since LINUX is
// no longer defined and it still compiles fine on linux.
#ifndef howmany
#define howmany(x,y) ((((u_int)(x))+(((u_int)(y))-1))/((u_int)(y)))
#endif
#endif

Dispatcher* Dispatcher::_instance;

class FdMask : public fd_set {
public:
    FdMask();
    void zero();
    void setBit(int);
    void clrBit(int);
    bool isSet(int) const;
    bool anySet() const;
    int numSet() const;
};

FdMask::FdMask() {
    zero();
}

void FdMask::zero() { Memory::zero(this, sizeof(FdMask)); }
void FdMask::setBit(int fd) { FD_SET(fd,(fd_set*)this); }
void FdMask::clrBit(int fd) { FD_CLR(fd,(fd_set*)this); }
bool FdMask::isSet(int fd) const { return FD_ISSET(fd,(fd_set*)this); }

bool FdMask::anySet() const {

#if 0 && defined(_XOPEN_SOURCE)
    const int mskcnt = howmany(FD_SETSIZE,NFDBITS);
    for (int i = 0; i < mskcnt; i++) {
	if (fds_bits[i]) {
	    return true;
	}
    }
#else
//
// Redo using entirely standard macros.  This is slightly less efficient.
//
    for (int fd_idx = 0; fd_idx < FD_SETSIZE; ++fd_idx)
      if (FD_ISSET(fd_idx, (fd_set*)this))
	return true;
#endif
    return false;
}

int FdMask::numSet() const {
    int n = 0;
//
// Redone using entirely standard macros.
//
    for (int i = 0; i < FD_SETSIZE; ++i)
      if (FD_ISSET(i, (fd_set*)this))
	++n;

//    const int mskcnt = howmany(FD_SETSIZE,NFDBITS);
//    for (int i = 0; i < mskcnt; i++) {
//	if (fds_bits[i]) {
//	    for (int j = 0; j < NFDBITS; j++) {
//		if ((fds_bits[i] & (1 << j)) != 0) {
//		    n += 1;
//		}
//	    }
//	}
//    }
    return n;
}

/*
 * Operations on timeval structures.
 */

const long ONE_SECOND = 1000000;

timeval operator+(timeval src1, timeval src2) {
    timeval sum;
    sum.tv_sec = src1.tv_sec + src2.tv_sec;
    sum.tv_usec = src1.tv_usec + src2.tv_usec;
    if (sum.tv_usec >= ONE_SECOND) {
	sum.tv_usec -= ONE_SECOND;
	sum.tv_sec++;
    } else if (sum.tv_sec >= 1 && sum.tv_usec < 0) {
	sum.tv_usec += ONE_SECOND;
	sum.tv_sec--;
    }
    return sum;
}

timeval operator-(timeval src1, timeval src2) {
    timeval delta;
    delta.tv_sec = src1.tv_sec - src2.tv_sec;
    delta.tv_usec = src1.tv_usec - src2.tv_usec;
    if (delta.tv_usec < 0) {
	delta.tv_usec += ONE_SECOND;
	delta.tv_sec--;
    } else if (delta.tv_usec >= ONE_SECOND) {
	delta.tv_usec -= ONE_SECOND;
	delta.tv_sec++;
    }
    return delta;
}

bool operator>(timeval src1, timeval src2) {
    if (src1.tv_sec > src2.tv_sec) {
	return true;
    } else if (src1.tv_sec == src2.tv_sec && src1.tv_usec > src2.tv_usec) {
	return true;
    } else {
	return false;
    }
}

bool operator<(timeval src1, timeval src2) {
    if (src1.tv_sec < src2.tv_sec) {
	return true;
    } else if (src1.tv_sec == src2.tv_sec && src1.tv_usec < src2.tv_usec) {
	return true;
    } else {
	return false;
    }
}

/*
 * Interface to timers.
 */

struct Timer {
    Timer(timeval t, IOHandler* h, Timer* n);

    timeval timerValue;
    IOHandler* handler;
    Timer* next;
};

class TimerQueue {
public:
    TimerQueue();
    virtual ~TimerQueue();

    bool isEmpty() const;
    static timeval zeroTime();
    timeval earliestTime() const;
    static timeval currentTime();

    void insert(timeval, IOHandler*);
    void remove(IOHandler*);
    void expire(timeval);
private:
    Timer* _first;
    static timeval _zeroTime;
};

Timer::Timer(timeval t, IOHandler* h, Timer* n) :
    timerValue(t),
    handler(h),
    next(n) {}

timeval TimerQueue::_zeroTime;

TimerQueue::TimerQueue() :
    _first(nil) {}

TimerQueue::~TimerQueue() {
    Timer* doomed = _first;
    while (doomed != nil) {
	Timer* next = doomed->next;
	delete doomed;
	doomed = next;
    }
}

inline bool TimerQueue::isEmpty() const {
    return _first == nil;
}

inline timeval TimerQueue::zeroTime() {
    return _zeroTime;
}

inline timeval TimerQueue::earliestTime() const {
    return _first->timerValue;
}

timeval TimerQueue::currentTime() {
    timeval curTime;
#if 0 && defined(SVR4) && !defined(__GNUC__)
    gettimeofday(&curTime);
#else
    struct timezone curZone;
    gettimeofday(&curTime, &curZone);
#endif
    return curTime;
}

void TimerQueue::insert(timeval futureTime, IOHandler* handler) {
    if (isEmpty() || futureTime < earliestTime()) {
	_first = new Timer(futureTime, handler, _first);
    } else {
	Timer* before = _first;
	Timer* after = _first->next;
	while (after != nil && futureTime > after->timerValue) {
	    before = after;
	    after = after->next;
	}
	before->next = new Timer(futureTime, handler, after);
    }
}

void TimerQueue::remove(IOHandler* handler) {
    Timer* before = nil;
    Timer* doomed = _first;
    while (doomed != nil && doomed->handler != handler) {
	before = doomed;
	doomed = doomed->next;
    }
    if (doomed != nil) {
	if (before == nil) {
	    _first = doomed->next;
	} else {
	    before->next = doomed->next;
	}
	delete doomed;
    }
}

void TimerQueue::expire(timeval curTime) {
    while (!isEmpty() && earliestTime() < curTime) {
	Timer* expired = _first;
	_first = _first->next;
	expired->handler->timerExpired(curTime.tv_sec, curTime.tv_usec);
	delete expired;
    }
}

/*
 * Interface to child process handling.
 */

struct Child {
    Child(pid_t pid, IOHandler* h, Child* n);

    pid_t pid;		// process's PID
    int status;		// wait status
    IOHandler* handler;	// associated handler
    Child* next;
};

class ChildQueue {
public:
    ChildQueue();
    virtual ~ChildQueue();

    bool isEmpty() const;
    bool isReady() const;

    void insert(pid_t, IOHandler*);
    void remove(IOHandler*);
    void notify();
    void setStatus(pid_t, int status);
private:
    Child* _first;            // queue head
    bool _ready;           // something is ready
};

Child::Child(pid_t p, IOHandler* h, Child* n) {
    pid = p;
    status = -1;
    handler = h;
    next = n;
}

ChildQueue::ChildQueue() {
    _first = nil;
    _ready = false;
}

ChildQueue::~ChildQueue() {
    Child* doomed = _first;
    while (doomed != nil) {
	Child* next = doomed->next;
	delete doomed;
	doomed = next;
    }
}

inline bool ChildQueue::isEmpty() const { return _first == nil; }
inline bool ChildQueue::isReady() const { return _ready; }

void ChildQueue::insert(pid_t p, IOHandler* handler) {
    if (isEmpty()) {
	_first = new Child(p, handler, _first);
    } else {
	Child* before = _first;
	Child* after = _first->next;
	while (after != nil && p > after->pid) {
	    before = after;
	    after = after->next;
	}
	before->next = new Child(p, handler, after);
    }
}

void ChildQueue::remove(IOHandler* handler) {
    Child* before = nil;
    Child* doomed = _first;
    while (doomed != nil && doomed->handler != handler) {
	before = doomed;
	doomed = doomed->next;
    }
    if (doomed != nil) {
	if (before == nil) {
	    _first = doomed->next;
	} else {
	    before->next = doomed->next;
	}
	delete doomed;
    }
}

void ChildQueue::setStatus(pid_t p, int status) {
    for (Child* c = _first; c != nil; c = c->next) {
	if (c->pid == p) {
	    c->status = status;
	    _ready = true;
	    break;
	}
    }
}

void ChildQueue::notify() {
    Child** prev = &_first;
    Child* c;

    while ((c = *prev) != nil) {
	if (c->status != -1) {
	    c->handler->childStatus(c->pid, c->status);
	    *prev = c->next;
	    delete c;
	} else {
	    prev = &c->next;
	}
    }
    _ready = false;
}

Dispatcher::Dispatcher() {
    _nfds = 0;
    _rmask = new FdMask;
    _wmask = new FdMask;
    _emask = new FdMask;
    _rmaskready = new FdMask;
    _wmaskready = new FdMask;
    _emaskready = new FdMask;
    _rtable = new IOHandler*[NOFILE];
    _wtable = new IOHandler*[NOFILE];
    _etable = new IOHandler*[NOFILE];
    _queue = new TimerQueue;
    _cqueue = new ChildQueue;
    for (int i = 0; i < NOFILE; i++) {
	_rtable[i] = nil;
	_wtable[i] = nil;
	_etable[i] = nil;
    }
}

Dispatcher::~Dispatcher() {
    delete _rmask;
    delete _wmask;
    delete _emask;
    delete _rmaskready;
    delete _wmaskready;
    delete _emaskready;
    delete [] _rtable;
    delete [] _wtable;
    delete [] _etable;
    delete _queue;
    delete _cqueue;
}

Dispatcher& Dispatcher::instance() {
    if (_instance == nil) {
	_instance = new Dispatcher;
    }
    return *_instance;
}

void Dispatcher::instance(Dispatcher* d) { _instance = d; }

IOHandler* Dispatcher::handler(int fd, DispatcherMask mask) const {
    if (fd < 0 || fd >= NOFILE) {
	abort();
    }
    IOHandler* cur = nil;
    if (mask == ReadMask) {
	cur = _rtable[fd];
    } else if (mask == WriteMask) {
	cur = _wtable[fd];
    } else if (mask == ExceptMask) {
	cur = _etable[fd];
    } else {
	abort();
    }
    return cur;
}

void Dispatcher::link(int fd, DispatcherMask mask, IOHandler* handler) {
    if (fd < 0 || fd >= NOFILE) {
	abort();
    }
    attach(fd, mask, handler);
}

void Dispatcher::unlink(int fd) {
    if (fd < 0 || fd >= NOFILE) {
	abort();
    }
    detach(fd);
}

void Dispatcher::attach(int fd, DispatcherMask mask, IOHandler* handler) {
    if (mask == ReadMask) {
	_rmask->setBit(fd);
	_rtable[fd] = handler;
    } else if (mask == WriteMask) {
	_wmask->setBit(fd);
	_wtable[fd] = handler;
    } else if (mask == ExceptMask) {
	_emask->setBit(fd);
	_etable[fd] = handler;
    } else {
	abort();
    }
    if (_nfds < fd+1) {
	_nfds = fd+1;
    }
}

void Dispatcher::detach(int fd) {
    _rmask->clrBit(fd);
    _rtable[fd] = nil;
    _wmask->clrBit(fd);
    _wtable[fd] = nil;
    _emask->clrBit(fd);
    _etable[fd] = nil;
    if (_nfds == fd+1) {
	while (_nfds > 0 && _rtable[_nfds-1] == nil &&
	       _wtable[_nfds-1] == nil && _etable[_nfds-1] == nil
	) {
	    _nfds--;
	}
    }
}

void Dispatcher::startTimer(long sec, long usec, IOHandler* handler) {
    timeval deltaTime;
    deltaTime.tv_sec = sec;
    deltaTime.tv_usec = usec;
    _queue->insert(TimerQueue::currentTime() + deltaTime, handler);
}

void Dispatcher::stopTimer(IOHandler* handler) {
    _queue->remove(handler);
}

void Dispatcher::startChild(int pid, IOHandler* handler) {
    _cqueue->insert(pid, handler);
}

void Dispatcher::stopChild(IOHandler* handler) {
    _cqueue->remove(handler);
}

bool Dispatcher::setReady(int fd, DispatcherMask mask) {
    if (handler(fd, mask) == nil) {
	return false;
    }
    if (mask == ReadMask) {
	_rmaskready->setBit(fd);
    } else if (mask == WriteMask) {
	_wmaskready->setBit(fd);
    } else if (mask == ExceptMask) {
	_emaskready->setBit(fd);
    } else {
	return false;
    }
    return true;
}

void Dispatcher::dispatch() {
    dispatch(nil);
}

bool Dispatcher::dispatch(long& sec, long& usec) {
    timeval howlong;
    timeval prevTime;
    timeval elapsedTime;

    howlong.tv_sec = sec;
    howlong.tv_usec = usec;
    prevTime = TimerQueue::currentTime();

    bool success = dispatch(&howlong);

    elapsedTime = TimerQueue::currentTime() - prevTime;
    if (howlong > elapsedTime) {
	howlong = howlong - elapsedTime;
    } else {
	howlong = TimerQueue::zeroTime(); /* Used all of timeout */
    }

    sec = howlong.tv_sec;
    usec = howlong.tv_usec;
    return success;
}

bool Dispatcher::dispatch(timeval* howlong) {
    FdMask rmaskret;
    FdMask wmaskret;
    FdMask emaskret;
    int nfound;

    if (anyReady()) {
	nfound = fillInReady(rmaskret, wmaskret, emaskret);
    } else {
	nfound = waitFor(rmaskret, wmaskret, emaskret, howlong);
    }

    notify(nfound, rmaskret, wmaskret, emaskret);

    return (nfound != 0);
}

bool Dispatcher::anyReady() const {
    return
       _rmaskready->anySet() || _wmaskready->anySet() || _emaskready->anySet();
}

int Dispatcher::fillInReady(
    FdMask& rmaskret, FdMask& wmaskret, FdMask& emaskret
) {
    rmaskret = *_rmaskready;
    wmaskret = *_wmaskready;
    emaskret = *_emaskready;
    _rmaskready->zero();
    _wmaskready->zero();
    _emaskready->zero();
    return rmaskret.numSet() + wmaskret.numSet() + emaskret.numSet();
}

//
// From what I can see from other packages that have been ported to a lot
// of systems, it looks like you can get away with declaring your signal
// handlers to have an argument of int.  I'm not entirely sure this is
// true, however.  Apparently you had some problems in the past with this.
// The actual type could be guessed by autoconf, I suppose, if it's
// worth it to devise a test.
//
void Dispatcher::sigCLD(int)
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
	Dispatcher::instance()._cqueue->setStatus(pid, status);
    }
}

#define fxSIGHANDLER 
#define fxSIGVECHANDLER
#define fxSIGACTIONHANDLER
				// Shouldn't be necessary to cast if the 
				// compiler is ANSI conformant since it
				// already knows the type of the routine.
				// (Unless I've overlooked something.)
//#ifndef fxSIGHANDLER
//#define fxSIGHANDLER
//#endif
//#ifndef fxSIGVECHANDLER
//#define fxSIGVECHANDLER	(void(*)(int))
//#endif

//#if !defined(fxSIGACTIONHANDLER) && (defined(__GNUC__) || (defined(SGI) && SGI == 5))
//#define fxSIGACTIONHANDLER (void(*)(...))
//#endif
//#if !defined(fxSIGACTIONHANDLER) && (defined(SVR4) || defined(sgi))
//#define fxSIGACTIONHANDLER (void(*)(int))
//#endif
//#if !defined(fxSIGACTIONHANDLER)
//#define fxSIGACTIONHANDLER (void(*)())
//#endif

#ifndef SA_INTERRUPT
#define SA_INTERRUPT 0
#endif

#if defined(sun) && defined(__GNUC__) && !defined(SVR4)
extern "C" { int sigvec(int, struct sigvec*, struct sigvec*);}
#endif

#if defined(AIXV3)
extern "C" { int sigvec(int, struct sigvec*, struct sigvec*);}
#endif

int Dispatcher::waitFor(
    FdMask& rmaskret, FdMask& wmaskret, FdMask& emaskret, timeval* howlong
) {
    int nfound;
#if defined(HAVE_BSD_SIGNALS) // ifdef SV_INTERRUPT /* BSD-style */
    static struct sigvec sv, osv;
#elif defined(HAVE_POSIX_SIGNALS) // #ifdef SA_NOCLDSTOP /* POSIX */
    static struct sigaction sa, osa;
#else                                 /* System V-style */
    void (*osig)();
#endif

    if (!_cqueue->isEmpty()) {
#if defined(HAVE_BSD_SIGNALS) // #ifdef SV_INTERRUPT /* BSD-style */
	sv.sv_handler = fxSIGVECHANDLER(&Dispatcher::sigCLD);
	sv.sv_flags = SV_INTERRUPT;
	sigvec(SIGCLD, &sv, &osv);
#elif defined(HAVE_POSIX_SIGNALS) // #ifdef SA_NOCLDSTOP /* POSIX */
	sa.sa_handler = fxSIGACTIONHANDLER(&Dispatcher::sigCLD);
	sa.sa_flags = SA_INTERRUPT;
	sigaction(SIGCLD, &sa, &osa);
#else                                 /* System V-style */
	osig = (void (*)())signal(SIGCLD, fxSIGHANDLER(&Dispatcher::sigCLD));
#endif
    }

    do {
	rmaskret = *_rmask;
	wmaskret = *_wmask;
	emaskret = *_emask;
	howlong = calculateTimeout(howlong);

//#if 0 && defined(hpux)
// 	nfound = select(
//	    _nfds, (int*)&rmaskret, (int*)&wmaskret, (int*)&emaskret, howlong
//	);
//#else
 	nfound = select(_nfds, &rmaskret, &wmaskret, &emaskret, howlong);
//#endif
    } while (nfound < 0 && !handleError());
    if (!_cqueue->isEmpty()) {
#if defined(HAVE_BSD_SIGNALS) // #ifdef SV_INTERRUPT  /* BSD-style */
	sigvec(SIGCLD, &osv, (struct sigvec*) 0);
#elif defined(HAVE_POSIX_SIGNALS) // #ifdef SA_NOCLDSTOP /* POSIX */
	sigaction(SIGCLD, &osa, (struct sigaction*) 0);
#else                                 /* System V-style */
	(void) signal(SIGCLD, fxSIGHANDLER(osig));
#endif
    }

    return nfound;		/* Timed out or input available */
}

void Dispatcher::notify(
    int nfound, FdMask& rmaskret, FdMask& wmaskret, FdMask& emaskret
) {
    for (int i = 0; i < _nfds && nfound > 0; i++) {
	if (rmaskret.isSet(i)) {
	    int status = _rtable[i]->inputReady(i);
	    if (status < 0) {
		detach(i);
	    } else if (status > 0) {
		_rmaskready->setBit(i);
	    }
	    nfound--;
	}
	if (wmaskret.isSet(i)) {
	    int status = _wtable[i]->outputReady(i);
	    if (status < 0) {
		detach(i);
	    } else if (status > 0) {
		_wmaskready->setBit(i);
	    }
	    nfound--;
	}
	if (emaskret.isSet(i)) {
	    int status = _etable[i]->exceptionRaised(i);
	    if (status < 0) {
		detach(i);
	    } else if (status > 0) {
		_emaskready->setBit(i);
	    }
	    nfound--;
	}
    }

    if (!_queue->isEmpty()) {
	_queue->expire(TimerQueue::currentTime());
    }
    if (_cqueue->isReady()) {
	_cqueue->notify();
    }
}

timeval* Dispatcher::calculateTimeout(timeval* howlong) const {
    static timeval timeout;

    if (!_queue->isEmpty()) {
	timeval curTime;

	curTime = TimerQueue::currentTime();
	if (_queue->earliestTime() > curTime) {
	    timeout = _queue->earliestTime() - curTime;
	    if (howlong == nil || *howlong > timeout) {
		howlong = &timeout;
	    }
	} else {
	    timeout = TimerQueue::zeroTime();
	    howlong = &timeout;
	}
    }
    return howlong;
}

bool Dispatcher::handleError() {
    switch (errno) {
    case EBADF:
	checkConnections();
	break;
    case EINTR:
	if (_cqueue->isReady()) {
	    return true;
	}
	break;
#if defined(CYGWIN)
    case 0:
	break;
#endif
    default:
	printf("errno=%d\n", errno);
	exit(1);
	/*NOTREACHED*/
    }
    return false;	// retry select;
}

void Dispatcher::checkConnections() {
    FdMask rmask;
    timeval poll = TimerQueue::zeroTime();

    for (int fd = 0; fd < _nfds; fd++) {
	if (_rtable[fd] != nil) {
	    rmask.setBit(fd);
//#if 0 && defined(hpux)
//	    if (select(fd+1, (int*)&rmask, nil, nil, &poll) < 0) {
//#else
	    if (select(fd+1, &rmask, nil, nil, &poll) < 0) {
//#endif
		detach(fd);
	    }
	    rmask.clrBit(fd);
	}
    }
}
