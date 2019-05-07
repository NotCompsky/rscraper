#ifndef __MYERR__
#define __MYERR__

namespace myerr {
enum {
    NONE,
    UNKNOWN,
    CANNOT_INIT_CURL,
    CANNOT_WRITE_RES,
    CURL_PERFORM,
    CANNOT_SET_PROXY,
    INVALID_PJ,
    JSON_PARSING,
    BAD_ARGUMENT
};
}
#endif
