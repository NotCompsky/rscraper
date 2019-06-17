/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#define RSET_STR(a, b)   a = b.GetString()
#define RSET_INT(a, b)   a = b.GetInt()
#define RSET_FLT(a, b)   a = b.GetFloat()
#define RSET_BOOL(a, b)  a = b.GetBool()
#define RSET(a, b, fnct)  a = b.fnct()

#ifdef DEBUG
  #define RSET_DBG_STR(a, b)   RSET_STR(a, b); printf(#a ":\t%s\n", b.GetString())
  #define RSET_DBG_INT(a, b)   RSET_INT(a, b);    printf(#a ":\t%d\n", b.GetInt())
  #define RSET_DBG_FLT(a, b)   RSET_FLT(a, b);  printf(#a ":\t%f\n", b.GetFloat())
  #define RSET_DBG_BOOL(a, b)  RSET_BOOL(a, b);   printf(#a ":\t%s\n", b.GetBool() ? "true" : "false")
  #define RSET_DBG(a, b, fnct, fmt)  RSET(a, b, fnct); printf(#a ":\t" #fmt "\n", a)
#else
  #define RSET_DBG_STR(a, b)   RSET_STR(a, b)
  #define RSET_DBG_INT(a, b)   RSET_INT(a, b)
  #define RSET_DBG_FLT(a, b)   RSET_FLT(a, b)
  #define RSET_DBG_BOOL(a, b)  RSET_BOOL(a, b)
  #define RSET_DBG(a, b, fnct, fmt)  RSET(a, b, fnct)
#endif

#define SET_DBG_STR(a, b)    const char* RSET_DBG_STR(a, b)
#define SET_DBG_INT(a, b)    const int   RSET_DBG_INT(a, b)
#define SET_DBG_FLT(a, b)    const float RSET_DBG_FLT(a, b)
#define SET_DBG_BOOL(a, b)   const bool  RSET_DBG_BOOL(a, b)
#define SET_DBG(type, a, b, fnct, fmt)   type RSET_DBG(a, b, fnct, fmt)

#define SET_STR(a, b)    const char* RSET_STR(a, b)
#define SET_INT(a, b)    const int   RSET_INT(a, b)
#define SET_FLT(a, b)    const float RSET_FLT(a, b)
#define SET_BOOL(a, b)   const bool  RSET_BOOL(a, b)
#define SET(type, a, b, fnct, fmt)   type RSET(a, b, fnct)
