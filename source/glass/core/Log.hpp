/*
* This file is part of Glass.
*
* Glass is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Glass is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Glass. If not, see <http://www.gnu.org/licenses/>.
*
* Copyright 2014-2015 Chris Foster
*/

#ifndef GLASS_CORE_LOG
#define GLASS_CORE_LOG

#include <iostream>

#define CLEAR	"\033[0m"
#define BOLD	"\033[1m"
#define CYAN	"\033[36m"
#define YELLOW	"\033[33m"
#define RED		"\033[31m"

#define LOG_INFO		std::cout << BOLD << CYAN << "[Glass]" << CLEAR << ": "
#define LOG_WARNING		std::cout << BOLD << YELLOW << "[Glass Warning]" << CLEAR << ": "
#define LOG_ERROR		std::cerr << BOLD << RED << "[Glass Error]" << CLEAR << ": "
#define LOG_FATAL		std::cerr << BOLD << RED << "[Glass Fatal Error]" << CLEAR << ": "

#define LOG_INFO_NOHEADER		std::cout
#define LOG_WARNING_NOHEADER	std::cout
#define LOG_ERROR_NOHEADER		std::cerr
#define LOG_FATAL_NOHEADER		std::cerr

#ifdef GLASS_DEBUG
	#define LOG_DEBUG_INFO		std::cout << BOLD << CYAN << "[Glass Debug]" << CLEAR << ": "
	#define LOG_DEBUG_WARNING	std::cout << BOLD << YELLOW << "[Glass Debug Warning]" << CLEAR << ": "
	#define LOG_DEBUG_ERROR		std::cerr << BOLD << RED << "[Glass Debug Error]" << CLEAR << ": "

	#define LOG_DEBUG_INFO_NOHEADER		std::cout
	#define LOG_DEBUG_WARNING_NOHEADER	std::cout
	#define LOG_DEBUG_ERROR_NOHEADER	std::cerr
#else
	#define LOG_DEBUG_INFO		if (false) std::cout
	#define LOG_DEBUG_WARNING	LOG_DEBUG_INFO
	#define LOG_DEBUG_ERROR		LOG_DEBUG_INFO

	#define LOG_DEBUG_INFO_NOHEADER		LOG_DEBUG_INFO
	#define LOG_DEBUG_WARNING_NOHEADER	LOG_DEBUG_INFO
	#define LOG_DEBUG_ERROR_NOHEADER	LOG_DEBUG_INFO
#endif

#endif
