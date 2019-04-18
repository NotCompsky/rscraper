#ifdef DEBUG
  #define SET_DBG_STR(a, b)  const char* a = b.GetString(); printf(#a ":\t%s\n", b.GetString());
  #define SET_DBG_INT(a, b)  const int   a = b.GetInt();    printf(#a ":\t%d\n", b.GetInt());
  #define SET_DBG_FLT(a, b)  const float a = b.GetFloat();  printf(#a ":\t%f\n", b.GetFloat());
  #define SET_DBG_BOOL(a, b) const bool  a = b.GetBool();   printf(#a ":\t%s\n", b.GetBool() ? "true" : "false");
#else
  #define SET_DBG_STR(a, b)  const char* a = b.GetString();
  #define SET_DBG_INT(a, b)  const int   a = b.GetInt();
  #define SET_DBG_FLT(a, b)  const float a = b.GetFloat();
  #define SET_DBG_BOOL(a, b) const bool  a = b.GetBool();
#endif
