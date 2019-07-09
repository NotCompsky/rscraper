/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef RSCRAPER_TAGGER_INCLUDE_RSCRAPER_TAGGER_HPP
#define RSCRAPER_TAGGER_INCLUDE_RSCRAPER_TAGGER_HPP


extern "C" char* DST;

extern "C"
void init();

extern "C"
void exit_mysql();

extern "C"
void csv2cls(const char* csv);


#endif
