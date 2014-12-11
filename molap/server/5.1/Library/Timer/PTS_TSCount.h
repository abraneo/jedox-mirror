
#ifndef PTS_TSCOUNT_H
#define PTS_TSCOUNT_H


#include "PTS_Resource.h"
#include "PT_MiscSystem.h"   // sleep

namespace p3t {

/** high res counter, partly portable
    TSC is kind of a raw variant of wall clock time 
    code taken from PACC Timer 1.14 LGPL
 */
struct PTS_TSCount: public PTS_Resource 
{
  PTS_TSCount(int runs=10, int udelay=10000) {

#if defined(WIN32)
    (void) udelay; 
    (void) runs; // make compiler happy
    LARGE_INTEGER lFreq;
    QueryPerformanceFrequency(&lFreq);
    calib = 1. / lFreq.QuadPart;

#else
    calib = 1.;
    long long total_cnt(0);
    double total_tm(0.);

    for ( int i=0; i < runs; ++i ) {

      timeval startTime, time;
      ::gettimeofday(&startTime,0);
      unsigned long long startCount = count();

      // a busy loop is probably more realistic since it
      // forces CPU frequency up
      if ( i % 3 == 0 ) 
	p3t::delay_loop(udelay*1e-6);
      else 
	p3t::sleep_nsres(udelay*1e-6);
      //::usleep(udelay);

      ::gettimeofday(&time,0);
      total_cnt += count() - startCount;
      time.tv_sec -= startTime.tv_sec;
      time.tv_usec -= startTime.tv_usec;
      total_tm += time.tv_sec + 0.000001 * time.tv_usec;
 
      PT_DEBUGINFO(" time: " << total_tm
	        << " tsc : " << total_cnt << " / "
		<< total_tm/total_cnt);
    }
    calib = total_tm / total_cnt;

    PT_DEBUGINFO( " calib: " << calib );
#endif
  }

  double now() const { 
	#if defined(__GNUC__) 
	  //return double(clock()) / CLOCKS_PER_SEC;
      timeval time;
      ::gettimeofday(&time,0);
	  return time.tv_sec + 0.000001 * time.tv_usec;
	#elif defined(WIN32)
	  return calib * count();
	#else
	  return 0;
	#endif
  }

  virtual void print_state( std::ostream &out = std::cout ) const 
  {
    out << "### PTS_TSCount - freq " << 1. / calib << " : ";
    PTS_Resource::print_state(out);
  }

private:
  static unsigned long long count() {
    unsigned long long lCount = 0; 
#if defined(WIN32)
    LARGE_INTEGER lCurrent;
    QueryPerformanceCounter(&lCurrent);
    return lCurrent.QuadPart;
#elif defined(__GNUC__) 
#  if defined (__i386__)
    __asm__ volatile("rdtsc" : "=A" (lCount));
#  elif defined(__ppc__) || defined(__PPC405__)
    register unsigned int lLow, lHigh1, lHigh2;
    do {
      // make sue that high bits have not changed
      __asm__ volatile ( "mftbu %0" : "=r" (lHigh1) );
      __asm__ volatile ( "mftb %0"  : "=r" (lLow) );
      __asm__ volatile ( "mftbu %0" : "=r" (lHigh2) );
    } while ( lHigh1 != lHigh2 );
    // transfer to lCount
    unsigned int *lPtr = (unsigned int *) &lCount;
    *lPtr++ = lHigh1; *lPtr = lLow;
#  else
	struct timeval t;

    gettimeofday( &t, NULL);
    lCount = t.tv_sec + 1.e-6 * t.tv_usec;
//#    error "hardware/compiler combination not supported, see, e.g., PTS_CPUTime"
#  endif
#else
#  error "hardware/compiler combination not supported, see, e.g., PTS_CPUTime"
#endif 
    return lCount;
  }

  double calib; // conversion count to seconds
};

} // ns p3t


#endif
