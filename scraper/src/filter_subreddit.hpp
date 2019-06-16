#ifndef __FILTER_SUBREDDIT__
#define __FILTER_SUBREDDIT__

#include <inttypes.h> // for uint64_t


namespace filter_subreddit {

int matches_id(const uint64_t id);

bool to_count(const uint64_t id);

}

#endif
