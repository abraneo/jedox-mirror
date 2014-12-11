#ifndef PT_DEBUG_H  /* -*- C++ -*- protects only the invariant parts of this file */
#define PT_DEBUG_H

/** 
    Two dimensional scheme for 'verbosity' and 'speed/safety' of a class
    PT_VERBOSITY value is used for 'verbosity' - higher level= more info
    PT_SAFETY  value is used for 'speed/safety' control 
                                               - higer level=more safety

    Makefile:
    setting -DNDEBUG corresponds to setting verbosity to 'LSEVERE' and 
    safety to 'LSOMECHECKS'                     
 
    if nothing is set you get LINFO, LSAFE levels

    to control safety/verbosity globally:
    -DPT_VERBOSITY=900 -DPT_SAFETY=1

    logging with level less or equal warning goes to stderr/cerr, otherwise to stdout/cout
    adjust by redefining PT_COUT and PT_CERR to appropriate streams (you can use any stream
    with the same interface as std::ostream)

    as an exception, loggin to IPM_Logger::addError/Info/Warning is supported instead, if 
    the macro PT_USE_IPMLOGGER is defined before the header is included

   WARNING: 
    the lowest 'blessed' value for these defines is 1, the largest meaningful setting is 1000 

*/ 

// #include <iostream>
#include <cstdlib>

/* predefined SAFETY LEVELS   _L in the name because the printing macros 
   recycle these names 
*/

#define PT_LNOCHECKS      1  /* all the speed you can get */  
#define PT_LSOMECHECKS   10  /* some - not runtime relevant - checks */
#define PT_LSAFE         20  /* all reasonable safety checks, asserts, ...
                                    array bounds, etc.. */

#define PT_LTEST        100  /* internal consistency checks, comprehensive 
	                            tests, ... */
#define PT_LDEBUG      1000  /* all the previous and more... */


/* predefined VERBOSITY levels */

#define PT_LFATAL         1  /* absolutely nothing, apart from fatal err. */
#define PT_LSEVERE       10  /* severe errors (inconsistencies, 
	                            no memory,... */
#define PT_LWARNING      20  /* possible problems, like oiut of 
	                            bounds access, div. by zero, etc...     */
#define PT_LINFO         30  /* important protocol information, 
				    internally calculated parameters, used
	                            current parameter values, etc... */
#define PT_LDETAILLED    40  /* + more, possible hints for improvements.. */ 
	                             
#define PT_LDEBUGINFO   1000  /* highest level, like above */
#define PT_LDEBUGINFO1  PT_LDEBUGINFO
#define PT_LDEBUGINFO2   900  /* get rid of previous info but leave these */
#define PT_LDEBUGINFO3   800  /* as above */
#define PT_LDEBUGINFO4   700  /* as above */
#define PT_LDEBUGINFO5   600  /* as above */


/* location of error and normal output stream */

#ifndef PT_COUT
#define PT_COUT std::cerr
#endif
#ifndef PT_CERR
#define PT_CERR std::cerr
#endif 
#ifndef PT_ENDL
#define PT_ENDL std::endl
#endif 


#ifndef PT_DBG_LOCATION
#define PT_DBG_LOCATION "["__FILE__ " L" << __LINE__ << "]: "
#endif

/*
    set PT_SAFETY and VERBOSITY if they haven't been set before
    use NDEBUG as a hint if possible
 */

#ifndef PT_SAFETY
#ifndef NDEBUG 
#define PT_SAFETY PT_LSAFE          /* 'secure' level */
#else 
#define PT_SAFETY PT_LSOMECHECKS    /* 'reasonably fast' level   */
#endif                                 
#endif

#ifndef PT_VERBOSITY
#ifndef NDEBUG 
#define PT_VERBOSITY PT_LINFO
#else 
#define PT_VERBOSITY PT_LSEVERE  
#endif                                 
#endif

/* if you get problems with one or two commas in your macro arguments 
   (they might after all contain templates <A1,A2>, etc) you could 
   probably wrap them in the appropriate  'protect' macro: 

   template<class T, class U> 
   PT_BINARYOUT(PTX_STONEOFWISDOM<T,U>)               // problem
   PT_BINARYOUT(PT_PROTECT1(PTX_STONEOFWISDOM<T,U>))  // ok
 */

#define PT_PROTECT1(A1,A2)     A1 ## , ## A2
#define PT_PROTECT2(A1,A2,A3)  A1 ## , ## A2 ## , ## A3


#endif   /* =========================================  conditional include */

/* on file basis stuff:
   set PT_SAFETY_LEVEL and VERBOSITY_LEVEL 
   if they haven't been set before the include, use PT_SAFETY and _VERBOSITY
 */

#ifndef  PT_SAFETY_LEVEL
#define  PT_SAFETY_LEVEL    PT_SAFETY
#endif
  
#ifndef  PT_VERBOSITY_LEVEL
#define  PT_VERBOSITY_LEVEL PT_VERBOSITY
#endif

/* error/warning/info messages - treat macro like C statement, i.e. add ; */

#if defined(PT_USE_IPMLOGGER)

#include "IPM_Logging/IPM_QLogger.h"
#include <sstream>

/* convert expression with << into a string, then pass to a routine 
   logging strings, use only locals for thread safety:
   - cast is safe, because we know that we point to a stringstream
   - lifetime is ok, at least as long as the routine expects const & or temp.
   - std::flush is necessary to force the stringstream() temporary to act as 
     ostream &, otherwise the first call is matched to a ostream & 
     member function which interprets a string EXPRESSION as (void *)
   - PREFIX contains '# WARNING', '# FATAL' etc and would repeat what IPM_Logger does
 */

#define PTDEBUG_FUNCTION_ERROR(PREFIX,EXPRESSION) \
   IPM_QLogger::addError( (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << EXPRESSION))->str().c_str()))
#define PTDEBUG_FUNCTION_WARN(PREFIX,EXPRESSION) \
   IPM_QLogger::addWarning( (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << EXPRESSION))->str().c_str()))
#define PTDEBUG_FUNCTION_INFO(PREFIX,EXPRESSION) \
   IPM_QLogger::addInfo( (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << EXPRESSION))->str().c_str()))

#elif defined(PT_USE_QDEBUG)

#include <sstream>
#include <QDebug>

#define PTDEBUG_FUNCTION_ERROR(PREFIX,EXPRESSION) \
   qDebug( (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << PREFIX << EXPRESSION))->str().c_str()))
#define PTDEBUG_FUNCTION_WARN(PREFIX,EXPRESSION) \
   qDebug( (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << PREFIX << EXPRESSION))->str().c_str()))
#define PTDEBUG_FUNCTION_INFO(PREFIX,EXPRESSION) \
   qDebug( (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << PREFIX << EXPRESSION))->str().c_str()))

#elif defined(PT_USE_PMFLOGGER) 

/* defines for Portalscanner project, logging via PMF_log is mandated by 
   project coding standards, the PMF_common.h header looks like plain C 
   but is in facto compiled with the c++ compiler.
   Thus we do not worry about linkage (extern "C") issues.
 */ 

#include "PMF_common.h"
#include <sstream>

#define PTDEBUG_FUNCTION_ERROR(PREFIX,EXPRESSION) \
   PMF_log("ERR","%s", (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << EXPRESSION))->str().c_str()))
#define PTDEBUG_FUNCTION_WARN(PREFIX,EXPRESSION) \
   PMF_log("WRN","%s", (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << EXPRESSION))->str().c_str()))
#define PTDEBUG_FUNCTION_INFO(PREFIX,EXPRESSION) \
   PMF_log("TRC","%s", (dynamic_cast<std::stringstream*>(& \
          (std::stringstream() << std::flush << EXPRESSION))->str().c_str()))

#else

#include <iostream>

#define PTDEBUG_FUNCTION_ERROR(PREFIX,EXPRESSION) PT_CERR << PREFIX << EXPRESSION << PT_ENDL
#define PTDEBUG_FUNCTION_WARN(PREFIX,EXPRESSION)  PT_CERR << PREFIX << EXPRESSION << PT_ENDL
#define PTDEBUG_FUNCTION_INFO(PREFIX,EXPRESSION)  PT_COUT << PREFIX << EXPRESSION << PT_ENDL

#endif

#undef  PT_FATAL
#define PT_FATAL(STRING)  PTDEBUG_FUNCTION_ERROR( \
        "# FATAL ", PT_DBG_LOCATION << STRING )

#undef  PT_SEVERE
#if PT_VERBOSITY_LEVEL < PT_LSEVERE
#define PT_SEVERE(STRING)
#else 
#define PT_SEVERE(STRING) PTDEBUG_FUNCTION_ERROR( \
        "# SEVERE ", PT_DBG_LOCATION << STRING )
#endif

#undef  PT_WARNING
#if PT_VERBOSITY_LEVEL < PT_LWARNING
#define PT_WARNING(STRING) 
#else
#define PT_WARNING(STRING) PTDEBUG_FUNCTION_WARN( \
        "# WARNING ", PT_DBG_LOCATION << STRING )
#endif

#undef  PT_INFO
#if PT_VERBOSITY_LEVEL < PT_LINFO
#define PT_INFO(STRING)
#else
#define PT_INFO(STRING)   PTDEBUG_FUNCTION_INFO( \
        "# Info ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DETAILLED
#if PT_VERBOSITY_LEVEL < PT_LDETAILLED
#define PT_DETAILLED(STRING)
#else
#define PT_DETAILLED(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Det. Info ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DEBUGINFO
#if PT_VERBOSITY_LEVEL < PT_LDEBUGINFO
#define PT_DEBUGINFO(STRING)
#else
#define PT_DEBUGINFO(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Debug Info ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DEBUGINFO1
#if PT_VERBOSITY_LEVEL < PT_LDEBUGINFO1
#define PT_DEBUGINFO1(STRING)
#else
#define PT_DEBUGINFO1(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Debug Info(1) ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DEBUGINFO2
#if PT_VERBOSITY_LEVEL < PT_LDEBUGINFO2
#define PT_DEBUGINFO2(STRING)
#else
#define PT_DEBUGINFO2(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Debug Info(2) ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DEBUGINFO3
#if PT_VERBOSITY_LEVEL < PT_LDEBUGINFO3
#define PT_DEBUGINFO3(STRING)
#else
#define PT_DEBUGINFO3(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Debug Info(3) ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DEBUGINFO4
#if PT_VERBOSITY_LEVEL < PT_LDEBUGINFO4
#define PT_DEBUGINFO4(STRING)
#else
#define PT_DEBUGINFO4(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Debug Info(4) ", PT_DBG_LOCATION << STRING )
#endif

#undef PT_DEBUGINFO5
#if PT_VERBOSITY_LEVEL < PT_LDEBUGINFO5
#define PT_DEBUGINFO5(STRING)
#else
#define PT_DEBUGINFO5(STRING) PTDEBUG_FUNCTION_INFO( \
        "# Debug Info(5) ", PT_DBG_LOCATION << STRING )
#endif


/* safety checks - macro acts like switch, may or may not require added ;
   depending on code location, take some care in the body of if's where
   the syntax may be altered if a ; is suddenly missing:  
   if () ; else ; if () ...  */

#undef  PT_NOCHECKS
#define PT_NOCHECKS(CODE)   CODE      /* always generate code */

#undef  PT_SOMECHECKS
#if PT_SAFETY_LEVEL < PT_LSOMECHECKS
#define PT_SOMECHECKS(CODE)  
#else
#define PT_SOMECHECKS(CODE) CODE 
#endif

#undef  PT_SAFE
#if PT_SAFETY_LEVEL < PT_LSAFE
#define PT_SAFE(CODE) 
#else
#define PT_SAFE(CODE) CODE
#endif

#undef  PT_TEST
#if PT_SAFETY_LEVEL < PT_LTEST
#define PT_TEST(CODE) 
#else
#define PT_TEST(CODE) CODE
#endif

#undef  PT_DEBUG
#if PT_SAFETY_LEVEL < PT_LDEBUG
#define PT_DEBUG(CODE) 
#else
#define PT_DEBUG(CODE) CODE
#endif



/* something in the assert class */

#undef PT_VERIFY
#ifdef NDEBUG
#define PT_VERIFY(EXPRESSION)  ((void)(EXPRESSION))
#else
#define PT_VERIFY(EXPRESSION)\
      ((void)((EXPRESSION)?  \
               1 :           \
               (PT_WARNING("'" #EXPRESSION "' evaluated to false"), 0))) 
#endif	


#undef PT_FALSIFY
#ifdef NDEBUG
#define PT_FALSIFY(EXPRESSION) ((void)(EXPRESSION)); 
#else
#define PT_FALSIFY(EXPRESSION)\
      ((void)((EXPRESSION)?   \
              (PT_WARNING("'" #EXPRESSION "' evaluated to true"), 1) : 0))
#endif	

#undef PT_ASSERT
#ifdef NDEBUG
#define PT_ASSERT(EX)  ((void) 0) 
#else 
#define PT_ASSERT(EXPRESSION)\
      ((void)((EXPRESSION)? 1 : \
              (PT_FATAL( "assertion '" #EXPRESSION "' failed"), \
	       std::abort(), 0) ))
#endif

/* #ifdef PTM_MEMDEBUG */
/* #include "PT/PTM_Memory.h" */
/* //  PTM_Memory set __FILELINE__ if supported by the preprocessor */
/* #ifdef __FILELINE__    */
/* // this define should be the last before we enter the .cc file to compile */
/* // it appears here instead of in PTM_Memory.h in order to avoid problems */
/* // with defined new when PTDebug includes its headers in turn... */
/* #define new new(__FILE__,__LINE__) */
/* #endif */
/* #endif  /\* inclusion of PTM_Memory.h *\/ */

#undef PT_SAFETY_LEVEL
#undef PT_VERBOSITY_LEVEL

