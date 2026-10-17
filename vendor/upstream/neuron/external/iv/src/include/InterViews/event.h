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

/*
 * Input events.
 */

#ifndef iv_event_h
#define iv_event_h

#if !defined(WIN32) && !MAC
#define UNIX 1
#endif

#include <InterViews/coord.h>

#include <InterViews/_enter.h>

class Display;
class EventRep;
class Interactor;
class Handler;
class Window;
class World;

typedef unsigned int EventType;
typedef unsigned int EventButton;

#if UNIX
/* anachronism */
enum {
    MotionEvent,	/* mouse moved */
    DownEvent,		/* button pressed */
    UpEvent,		/* button released */
    KeyEvent,		/* key pressed, intepreted as ascii */
    EnterEvent,		/* mouse enters canvas */
    LeaveEvent,		/* mouse leaves canvas */
    FocusInEvent,	/* focus for keyboard events */
    FocusOutEvent 	/* lose keyboard focus */
};

/* mouse button anachronisms */
static const int LEFTMOUSE = 0;
static const int MIDDLEMOUSE = 1;
static const int RIGHTMOUSE = 2;
#endif

class Event {
public:
    enum { undefined, motion, down, up, key, other_event };
    enum { none, any, left, middle, right, other_button };

    Event();
    Event(const Event&);
    virtual ~Event();

    virtual Event& operator =(const Event&);

    virtual void display(Display*);
    virtual Display* display() const;

    virtual void window(Window*);
    virtual Window* window() const;

    virtual bool pending() const;
    virtual void read();
    virtual bool read(long sec, long usec);
    virtual void unread();
    virtual void poll();

    virtual Handler* handler() const;
    virtual void handle();
    virtual void grab(Handler*) const;
    virtual void ungrab(Handler*) const;
    virtual Handler* grabber() const;
    virtual bool is_grabbing(Handler*) const;

    virtual EventType type() const;
    virtual unsigned long time() const;
    virtual Coord pointer_x() const;
    virtual Coord pointer_y() const;
    virtual Coord pointer_root_x() const;
    virtual Coord pointer_root_y() const;
    virtual EventButton pointer_button() const;
    virtual unsigned int keymask() const;
    virtual bool control_is_down() const;
    virtual bool meta_is_down() const;
    virtual bool shift_is_down() const;
    virtual bool capslock_is_down() const;
    virtual bool left_is_down() const;
    virtual bool middle_is_down() const;
    virtual bool right_is_down() const;
    virtual unsigned char keycode() const;
    virtual unsigned long keysym() const;
    virtual unsigned int mapkey(char*, unsigned int len) const;

    EventRep* rep() const;
private:
    EventRep* rep_;
    char free_store_[200];

    void copy_rep(const Event&);

#if UNIX
    /*
     * Old members for backward compatibility
     */
public:
    Interactor* target;
    unsigned long timestamp;
    EventType eventType;
    IntCoord x, y;		/* mouse position relative to target */
    bool control : 1;	/* true if down */
    bool meta : 1;
    bool shift : 1;
    bool shiftlock : 1;
    bool leftmouse : 1;
    bool middlemouse : 1;
    bool rightmouse : 1;
    unsigned char button;	/* button pressed or released, if any */
    unsigned short len;		/* length of ASCII string */
    char* keystring;		/* ASCII interpretation of event, if any */

    void GetAbsolute(IntCoord&, IntCoord&);
    void GetAbsolute(World*&, IntCoord&, IntCoord&);
    EventRep* Rep() const;
private:
    World* w;
    _lib_iv2_6(Coord) wx, wy;
    char keydata[sizeof(int)];

    friend class Interactor;

    void GetInfo();
    void GetMotionInfo();
    void GetButtonInfo(EventType);
    void GetKeyInfo();
    void GetKeyState(unsigned);
    void GetCrossingInfo(EventType);
#endif
};

inline EventRep* Event::rep() const { return rep_; }
#if UNIX
inline EventRep* Event::Rep() const { return rep(); }
#endif

#include <InterViews/_leave.h>

#endif
