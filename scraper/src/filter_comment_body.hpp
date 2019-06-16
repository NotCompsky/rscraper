#ifndef __FILTER_COMMENT_BODY__
#define __FILTER_COMMENT_BODY__

#include "structs.h" // for cmnt_meta


namespace filter_comment_body {

namespace reason {
    enum {
        NONE,
        UNCONDITIONAL, // i.e. unknown reason
        PROGRAMMING,
        LINUX,
        EGG
    };
}

namespace wl { // Whitelist
unsigned int match(struct cmnt_meta metadata, const char* str, const int str_len);
}


} // end namespace

#endif
