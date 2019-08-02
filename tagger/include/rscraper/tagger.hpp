/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_TAGGER_INCLUDE_RSCRAPER_TAGGER_HPP
#define RSCRAPER_TAGGER_INCLUDE_RSCRAPER_TAGGER_HPP

#include <inttypes.h> // for uint64_t


extern "C" char* DST;

extern "C"
void init();

extern "C"
void exit_mysql();

/*
extern "C"
void generate_id_list(const char* tblname,  const char** names,  uint64_t* ls);

extern "C"
uint64_t* generate_id_list_string(const char* tblname,  const char** names);
*/

extern "C"
const char* generate_id_list_string(const char* tblname,  const char** names);

extern "C"
void csv2cls(const char* csv,  const char* tagcondition,  const char* reasoncondition);

extern "C"
void user_summary(const char* const name);


#endif
