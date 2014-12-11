#ifndef PTS_RESOURCE_H  /* -*- C++ -*- */
#define PTS_RESOURCE_H

/*  $Id: PTSResource.h,v 1.15 2001-11-30 18:52:55 tobi Exp $

    SYSTEM RESOURCES

    DESCRIPTION:
    A base class for everything that has an instantanteous value 
    which can be expressed as double (time/disk/memory use) and which
    might also have a limit (as disk/memory usage). No limit 
    will be indicated by the largest possible double.
    
    INTERFACE:

    // query current value
    double now();
    // query limit (returns max double if none exists)
    double limit();

    // debug option
    void print_state(ostream &out=cout)

    SO FAR:
    PTS_Resource         base class
    PTS_CpuTime          process cputime usage in seconds from getrusage call
    PTS_WallTime         total wall clock time (as returned by gettimeofday)
    (PTS_TSCount (i386, ppc) read time stamp counter to get timing)
    
    TODO: memory, ... limits, split the code into cpp file
*/

#if defined (_WIN32)
# include <windows.h>
# include <time.h>
#elif defined(PT_RS6000)

# ifndef _ALL_SOURCE
#  define _ALL_SOURCE 
# endif
# include <sys/time.h>
# undef  _ALL_SOURCE

#else     /* other architectures */
# include  <sys/time.h>     /* for gettimeofday call */
#endif

#if defined(PT_PARAGON)
# include  <nx.h>
#elif defined(PT_CRAY_T3E) || defined(PT_NEC_SX)
//DO NOT TOUCH THIS!! Ask <matthias@ica1.uni-stuttgart.de> or <sts@..> why !
# include  <sys/times.h>
          /* may need also times.h to define  CLK_TCK */ 
# include <time.h>       /* for rtclock call */
# include <unistd.h>
#elif defined(__EMX__)
# include <time.h>
#elif !defined(_WIN32)
# include <sys/resource.h>
#endif 

#include <iostream>
#include <iomanip>
#include <limits>

#include  "PTDebug.h"

namespace p3t {

/** base class for things that are poss. limited, change, etc.
 */ 
class PTS_Resource
{
public:

  /** virtual fct to be overloaded in derived classes to query the 
      absolute value of a resource. Should return a 'current absolute'
      value (i.e. total CPU time used by the process).
   */
  virtual double now() const = 0;

#if defined(_WIN32)
# undef max
#endif
  /** limit value, defaults to DBL_MAX */
  virtual double limit() const { return std::numeric_limits<double>::max(); }

  /** prints internal state, mostly for debugging */
  virtual void print_state( std::ostream &out = std::cout ) const { 
    out << "# PTS_Resource: now = " << now();
    double lim = limit();
    if ( lim != PTS_Resource::limit() )
      out << " [limit = " << lim << "]";
    out << std::endl;
  }

  /** support complex stuff in derived classes */
  virtual ~PTS_Resource() {}
};



/** total CPU time used by the process */ 
class PTS_CPUTime: public PTS_Resource
{
public:
  virtual double now() const 
  {  
#ifdef   PT_PARAGON
      // this is a fix for the paragon, because all other calls 
      //  seem to create communication with the 'mother' node
      return dclock();
#elif defined(PT_CRAY_T3E) || defined(PT_NEC_SX)
      struct tms tb;
      times (&tb);
      return (tb.tms_utime + tb.tms_cutime + tb.tms_stime + 
	      tb.tms_cstime) / (double) CLK_TCK;     
#elif defined(__EMX__)
      return clock(); // is this correct - there should be a scale factor
#elif defined(_WIN32)
      // sts: there does seem to be  simple way of finding CPU usage on
      // WIN32. I have seen stuff using ThreadTime, but it seem to
      // be necessary to switch Hyperthreading off or to do similar
      // hacks. For now, it is probabably not worth the trouble.
      //PT_SEVERE("CPU time measurement for Win32 not yet implemented");
      return clock() / double(CLK_TCK);
#else          /* SYS V call */
      struct rusage t;
      getrusage ( RUSAGE_SELF, &t );
      return t.ru_utime.tv_sec  + t.ru_stime.tv_sec + 
	    (t.ru_utime.tv_usec + t.ru_stime.tv_usec) / 1.e6; 
#endif
  }

  virtual void print_state( std::ostream &out = std::cout ) const 
  { 
    out << "# PTS_CPUTime : ";
    PTS_Resource::print_state(out);
  }

};

/**  wall clock time */
class PTS_WallClock: public PTS_Resource
{
public:

  double now() const { 
#ifdef   PT_PARAGON
      //  same fix as for CpuTime
      return dclock();
#elif defined(PT_CRAY_T3E)
      double clockspeed = 1.0/sysconf(_SC_CLK_TCK);
      return rtclock()  * clockspeed;
#elif defined(_WIN32)
      // GetTickCount64() only on Vista
      // time since start of system in ms, wraps after 49 days
      return 0.001 * GetTickCount();
#else     /* stdc lib call */
      struct timeval t;

      gettimeofday( &t, NULL);
      return t.tv_sec + 1.e-6 * t.tv_usec;
#endif
  }

  virtual void print_state( std::ostream &out = std::cout ) const 
  {
    out << "### PTS_WallClock : ";
    PTS_Resource::print_state(out);
  }

};


}  // -*- ns p3t

// for compatiblity with programs that expect time stamp counter in 
// PTS_Resource.h (must be outside namespace definitions):
#include "PTS_TSCount.h"


#endif  /* conditional include */














