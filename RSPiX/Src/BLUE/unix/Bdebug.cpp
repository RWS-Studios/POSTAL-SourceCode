////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
///////////////////////////////////////////////////////////////////////////////
//
//	bdebug.cpp
// 
// History:
//		05/30/96 JMI	Started.
//
//		07/08/96	JMI	Now using exit(0) instead of FatalAppExit so that
//							atexit functions will get called.  Took out asm int 3
//							since I already had DebugBreak().
//
//		07/08/96	JMI	EXIT_SUCCESS now passed to exit instead of 0.
//
//		07/12/96	JMI	Removed excess param sExpr passed rspAssert.
//
//		07/16/96	JMI	Removed #if'd outness of this file in release mode.
//
//		10/18/96	JMI	szOutput in rspTrace is now static so it doesn't use
//							any stack space.  Changed str size for szOutput to
//							1024 bytes.  Changed ASSERT's string to same.
//
//		11/19/97	JMI	Added more debug output options via macros:
//							RSP_DEBUG_OUT_MESSAGEBOX, RSP_DEBUG_OUT_FILE, 
//							RSP_DEBUG_ASSERT_PASSIVE, & RSP_TRACE_LOG_NAME.
//							See below for details.
//
//		11/19/97	JMI	sSem was being decremented in the wrong spot.
//
//////////////////////////////////////////////////////////////////////////////
//
// Does all Windows specific debug stuff.
//
//	NOTE:  Define the following Macros at the compiler settings level to get
// other than default behavior:
//
//	- RSP_DEBUG_OUT_MESSAGEBOX	-- Use an rspMsgBox() for all TRACE calls.
// - RSP_DEBUG_OUT_FILE			-- Use a file for all TRACE calls.
// - RSP_DEBUG_ASSERT_PASSIVE	-- Just TRACE (do NOT show messagebox) for all
//										ASSERT failures -- Assumes user option 'Ignore'.
// - RSP_TRACE_LOG_NAME			-- Override the default name for the TRACE log
//										file.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Includes.
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>

#include "BLUE/Blue.h"

#include "CYAN/cyan.h"	// For rspMsgBox() used by rspTrace().

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
#define MAX_TRACE_STR	1024
#define MAX_ASSERT_STR	1024

#if defined(RSP_DEBUG_OUT_FILE)
	#if !defined(RSP_TRACE_LOG_NAME)
		#define RSP_TRACE_LOG_NAME	"TRACE.txt"
	#endif	// RSP_TRACE_LOG_NAME
#endif	// RSP_DEBUG_OUT_FILE

///////////////////////////////////////////////////////////////////////////////
//
// Used to extract the filename from __FILE__.
//
///////////////////////////////////////////////////////////////////////////////
char* Debug_FileName(char* pszPath)
	{
	// Start at end of string and work toward beginning or '\\'.
	char *p;
	for (p = pszPath + (strlen(pszPath) - 1); p > pszPath && *p != '\\'; p--);

	if (*p == '\\')
		p++;

	return p;
	}

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,"DUKE", __VA_ARGS__))
#endif
///////////////////////////////////////////////////////////////////////////////
//
// Output a formatted debug string to the debug terminal/window.
//
///////////////////////////////////////////////////////////////////////////////
void rspTrace(char *frmt, ... )
	{
	static short	sSem	= 0;

	// If something called by TRACE calls TRACE, we'd be likely to continue
	// forever until stack overflow occurred.  So don't allow re-entrance.
	if (++sSem == 1)
		{
		va_list varp;
		  
		va_start(varp, frmt);    
		  
#ifdef __ANDROID__
		char errortext[512];
		vsnprintf (errortext, 512, frmt, varp);
		va_end (varp);
		LOGI("%s",errortext);
#else
		vfprintf(stderr, frmt, varp);
#endif

#if defined(RSP_DEBUG_OUT_FILE)
		static FILE*	fs	= NULL;	// NOTE that we never fclose this so we can get 
											// EVERY LAST TRACE -- so this may show up as
											// a leak.  The system will close it though.
		// If not yet open . . . 
		if (fs == NULL)
			{
			// Attempt to open (Note that we never close this -- the system does).
			// This will probably show up as a leak.
			fs	= fopen(RSP_TRACE_LOG_NAME, "wt");
			if (fs)
			{
				fprintf(fs, "======== Postal Plus build %s %s ========\n", __DATE__, __TIME__);
				time_t sysTime = time(NULL);
				fprintf(fs, "Debug log file initialized: %s\n", ctime(&sysTime));
			}
			}

		// If open . . .
		if (fs)
			{
			char szOutput[512];
			vsnprintf(szOutput, 512, frmt, varp);
			fprintf(fs, szOutput);
			}
#endif	// RSP_DEBUG_OUT_FILE

		va_end(varp);

#if defined(RSP_DEBUG_OUT_MESSAGEBOX)
		if (rspMsgBox(
			RSP_MB_ICN_INFO | RSP_MB_BUT_YESNO,
			"rspTrace",
			"\"%s\"\n"
			"Continue?",
			szOutput) == RSP_MB_RET_NO)
			{
			DebugBreak();
			exit(EXIT_SUCCESS);
			}
#endif	// RSP_DEBUG_OUT_MESSAGEBOX
		}

	// Remember to reduce.
	sSem--;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
