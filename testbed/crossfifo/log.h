#ifndef _CFF_LOG_H_
#define _CFF_LOG_H_

#include <stdio.h>

#ifdef USE_DEBUG
  void _cff_debug_head(const char * f, int lno);
  void _cff_debug(const char * fmt, ...);
  void _cff_debug_err(const char * fmt, ...);
  #define cff_debug(x) _cff_debug_head(__FILE__, __LINE__);_cff_debug x
  #define cff_debug_err(x) _cff_debug_head(__FILE__, __LINE__);_cff_debug_err x
#else
  #define cff_debug(x)
  #define cff_debug_err(x)
#endif

#endif // _CFF_LOG_H_
