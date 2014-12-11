#ifndef __PT_MISCSYSTEM_H__
#define __PT_MISCSYSTEM_H__

/* Collection of simple helper function and classes related to 
   OS functionality

   date_time()           // local time and date
   nice(..)
   sleep(double seconds) // using poll to achieve ms resolution sleep

   see PTS_Resource for CPU time and elapsed wall clock time

// ----------------------------------------------------------------------
// BUGS: in linux the success of the nice call must be checked using errno
// ----------------------------------------------------------------------

*/
#ifdef _WIN32
#  include <windows.h>
#  include <cmath>
#  include <time.h>
#else
#  include <unistd.h>
#  include <time.h>
#  include <string>
#  include <cmath>
#  include <poll.h>
#endif

#include "PTDebug.h"

namespace p3t {

  
  /** ms resolution sleep, a lot higher resolution only makes sense
      if very short time slices are used and possibly in real time situations 
      without intervening scheduling
  */
  inline void sleep(double seconds) 
  {
    int timeout = static_cast<int>(seconds * 1000 + 0.5);
#ifdef _WIN32
    ::Sleep(timeout);
#else
    ::poll(0,0,timeout);
#endif
  }

  /** highest possible resolution sleep on each platform,
      @return remaining time if sleep was interrupted 
      @param seconds - time to sleep
   */
  inline double sleep_nsres(double seconds)
  {
    if ( seconds <= 0. ) return seconds;

#ifdef _WIN32
    p3t::sleep(seconds);
    return 0.;
#else
    struct timespec req, rest;
    req.tv_sec  = time_t(seconds);
    req.tv_nsec = (seconds - req.tv_sec) * 1e9;
    if ( req.tv_nsec < 0 ) req.tv_nsec = 0.;
    if ( req.tv_nsec >= 1000000000 )
      req.tv_nsec = 999999999;
    
    if ( ::nanosleep(&req,&rest) ) {
      return rest.tv_sec + 1e-9 * rest.tv_nsec;
    }
    else return 0.;
#endif
  }

  /** approximate delay by given number of (CPU) seconds,
      attn: _busy_ delay, does not take care of 
      interrupts/signals/scheduling delays
      BUG: delay may be shorter if clock() happens to wrap around
      (once every 72 min on a 32 bit system for POSIX impl.)
  */
  inline void delay_loop(double seconds)
  {
    clock_t start(clock()), diff(0);
    volatile double sum(0.);
    
#if defined(_WIN32)
    // this should work for windows - untested so far
    while ( diff >= 0 && diff / double(CLK_TCK) < seconds ) {
#else 
    // this works for Linux
    while ( diff >= 0 && diff / double(CLOCKS_PER_SEC) < seconds ) {
#endif    
      for ( clock_t i = diff; i < diff + 25; i += 1 )
	sum += std::sqrt(double(i));
      diff  = clock() - start;
    }
  }


  inline int nice(int n)
  {
#ifndef _WIN32
    int p = ::nice(0);
    if (p!= -1)  {
      p = ::nice(n-p);
      if (p!= -1) return p;
    }
#endif
    //PT_SEVERE( "renice "<<n<<" failed\n" );
    return -1;
  }
  
  inline std::string date_time()
  {
#ifndef _WIN32 
	// this routine should be threadsafe
    time_t  t(time(0));
    char    buffer[30];
    return  std::string(asctime_r(localtime(&t),buffer));
#else
    return  "not implemented";
#endif
  }
  
} // -ns- p3t

#endif








