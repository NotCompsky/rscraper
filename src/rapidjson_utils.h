#ifdef DEBUG
  #define RSET_DBG_STR(a, b)   a = b.GetString(); printf(#a ":\t%s\n", b.GetString());
  #define RSET_DBG_INT(a, b)   a = b.GetInt();    printf(#a ":\t%d\n", b.GetInt());
  #define RSET_DBG_FLT(a, b)   a = b.GetFloat();  printf(#a ":\t%f\n", b.GetFloat());
  #define RSET_DBG_BOOL(a, b)  a = b.GetBool();   printf(#a ":\t%s\n", b.GetBool() ? "true" : "false");
  #define RSET_DBG(a, b, fnct, fmt)  a = b.fnct(); printf(#a ":\t" #fmt "\n", a);
#else
  #define RSET_DBG_STR(a, b)   a = b.GetString();
  #define RSET_DBG_INT(a, b)   a = b.GetInt();
  #define RSET_DBG_FLT(a, b)   a = b.GetFloat();
  #define RSET_DBG_BOOL(a, b)  a = b.GetBool();
  #define RSET_DBG(a, b, fnct, fmt)  a = b.fnct();
#endif

#define SET_DBG_STR(a, b)    const char* RSET_DBG_STR(a, b)
#define SET_DBG_INT(a, b)    const int   RSET_DBG_INT(a, b)
#define SET_DBG_FLT(a, b)    const float RSET_DBG_FLT(a, b)
#define SET_DBG_BOOL(a, b)   const bool  RSET_DBG_BOOL(a, b)
#define SET_DBG(type, a, b, fnct, fmt)   type RSET_DBG(a, b, fnct, fmt)
