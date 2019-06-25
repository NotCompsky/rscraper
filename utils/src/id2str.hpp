/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __ID2STR_H__
#define __ID2STR_H__

#include <inttypes.h> // for uint64_t
#include <stddef.h> // for size_t


size_t id2str(uint64_t id_orig,  char* buf);
uint64_t str2id(const char* str);
uint64_t myatoi(const char* str);


#endif
