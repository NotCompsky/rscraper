#ifndef __FILTER_USER__
#define __FILTER_USER__

#include <inttypes.h> // for uint64_t


namespace filter_user {

int matches_id(const uint64_t id);

bool to_count(const uint64_t id);

}

#endif
