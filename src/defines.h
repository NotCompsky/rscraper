#ifdef DEBUG
  #define PRINTF(...) printf(__VA_ARGS__);
#else
  #define PRINTF(...) ;
#endif
