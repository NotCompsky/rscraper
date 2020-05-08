/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#pragma once

#include "mysql_declarations.hpp"
#include <compsky/mysql/query.hpp>
#include <QMessageBox>


namespace safe_mysql {

	template<typename... Args>
	bool query(Args... args){
		try {
			compsky::mysql::query(args...);
			return true;
		} catch (compsky::mysql::except::SQLExec& e){
			QMessageBox::warning(nullptr, "SQL Error", (unlikely(mysql_ping(_mysql::obj))) ? "Cannot connect to server" : e.what());
			// If auto-reconnect is enabled, mysql_ping() performs a reconnect. Otherwise, it returns an error.
			return false;
		}
	}
	
	template<typename... Args>
	bool query_buffer(Args... args){
		try {
			compsky::mysql::query_buffer(args...);
			return true;
		} catch (compsky::mysql::except::SQLExec& e){
			QMessageBox::warning(nullptr, "SQL Error", (unlikely(mysql_ping(_mysql::obj))) ? "Cannot connect to server" : e.what());
			return false;
		}
	}

}
