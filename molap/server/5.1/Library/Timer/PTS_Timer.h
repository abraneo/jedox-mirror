#ifndef   PTS_TIMER_H   /*-*- C++ -*-*/
#define   PTS_TIMER_H

#include  "PTDebug.h"

/** STOPWATCH

    instantiate with a class that has the Interface of a PTS_Resource,
    in particular a double now() member function.

    <pre>
    include "PTS_System/PTS_Timer.h"
    include "PTS_System/PTS_Resource.h"
    // or include "PTS_System/PTS_TSCount.h"

    main(int argc, char **argv)
    {
      PTSTimer<PTS_CPUTime> a;
      // or PTSTimer<PTS_TSCount> a;

      a.start();
      // do sthg
      a.stop();
      cout << "elapsed time " << a.elapsed();
      a.reset();
      a.start();

      // do something else
      a.stop();
      cout << "elapsed time " << a.elapsed();
      a.reset();
    }
    </pre>
*/
namespace p3t {

template<class T>
class PTS_Timer : private T {

public:
  PTS_Timer(): t_elapsed(0.), t_last_start(0.), state(stopped) {}

  /** start: set elapsed time to 0 */
  void start(void) { state = running; t_last_start = T::now(); }
  
  /** stop: add to elapsed time the interval since last start */
  void stop(void) { 
    t_elapsed += state == running ? T::now() - t_last_start: 0.;
    state = stopped;
  }

  /** reset: set elapased time to 0, timer can be either started or stopped */
  void reset(void) { t_last_start = T::now(); t_elapsed=0.;  }

  /** @return accumulated elapsed time */ 
  double elapsed(void) const { 
    return state == stopped ? t_elapsed 
                            : t_elapsed + T::now() - t_last_start; 
  }
private:
  enum { running, stopped }; // dont increment elapsed time when stopped
  double t_elapsed;
  double t_last_start;
  int    state;
};  

}   // - ns p3t

#endif










