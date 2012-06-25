#ifndef INC_config_hpp__
#define INC_config_hpp__

// Just a simple configuration file to adjust between
// various compiler quirks.


/*******************************************
 *   Microsoft Visual C++
 *******************************************/
#if defined(_MSC_VER) && !defined(__ICL) && !defined(__MWERKS__)
    // we don't care to know about name trucation in the debug data.
#   pragma warning (disable:4786)
#endif


/*******************************************
 * Intel C++ Compiler
 *******************************************/
#if defined(__ICL)
#endif


/*******************************************
 * Mingw32 (gcc)
 *   (egcs compiler using the M$ C runtime)
 *******************************************/
#if defined(__MINGW32__)
#endif


/*******************************************
 * Cygwin32 (gcc)
 *   (egcs cross-compiler on M$ Windows)
 *******************************************/
#if defined(__CYGWIN__)
#endif


#if defined(__sgi) && !defined(__GNUC__)
#   if (_COMPILER_VERSION >= 721) && !defined(_NAMESPACES)
#	define CPPTCL_NO_NAMESPACES
#   endif
#endif

/*******************************************
 * SunPro Compiler
 *     (Using OBJECTSPACE STL)
 *******************************************/
#ifdef __SUNPRO_CC
#   if (__SUNPRO_CC == 0x420)
#	define CPPTCL_NO_NAMESPACES
#   endif
#endif


/*******************************************
 * SCO UDK 7 compiler
 *     (UnixWare 7x, OSR 5, UnixWare 2x)
 *******************************************/
#if defined(__USLC__)
#endif


/*******************************************
 *   Inprise (Borland) C++ Builder 3.0
 *******************************************/
#ifdef __BCPLUSPLUS__
#endif


/*******************************************
 * IBM VisualAge C++ 
 *   ( uses the Dinkumware C++ Library )
 *******************************************/
#ifdef __IBMCPP__
#endif


/*******************************************
 * Metrowerks Codewarrior
 *******************************************/
#ifdef __MWERKS__
#endif



#if !defined(CPPTCL_NO_NAMESPACES)
#   define CPPTCL_USE_NAMESPACE(_x_)	    _x_::
#   define CPPTCL_USING_NAMESPACE(_x_)	    using namespace _x_;
#   define CPPTCL_BEGIN_NAMESPACE(_x_)	    namespace _x_ {
#   define CPPTCL_END_NAMESPACE 	    }
#else
#   define CPPTCL_USE_NAMESPACE(_x_)
#   define CPPTCL_USING_NAMESPACE(_x_)
#   define CPPTCL_BEGIN_NAMESPACE(_x_)
#   define CPPTCL_END_NAMESPACE
#endif


#endif // #ifndef INC_config_hpp__
