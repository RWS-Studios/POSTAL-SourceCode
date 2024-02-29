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
// COMMAND SUMMARY: - simple to use!

// rspStartProfile("label");				// mark beginning of a timed code segment
// rspEndProfile("label");					// mark the end of a time code segment
// rspProfileOn();							// Start a profiling session
// rspProfileOff();							// Suspend the Profiler
// rspSetProfileOutput("filename.ext");// direct the output
// #define RSP_PROFILE_ON					// activate profiling in this module,
													// or set in project settings

///////////////////////////////////////////////////////////////////////////////
//
//	Profile.h
// 
// History:
//		06/11/97 JRD	Started.
//
//		06/12/97 JRD	Revamped user interfaceChanged to appear as though
//							it's not really a class instance.  Worked on timing refinement
//		
//		06/13/97 JRD	Finally got 2nd order self timing working - accurate to 60ns.
//							Added ability to have inside out (exclusions) loops.
//
//		06/14/97 JRD	Added multiple report sessions, min/max reporting,
//							percentage of target (100%) reporting, error reporting,
//							and error recovery.
//
//		06/16/97	JRD	Optimized speed over consistency.
//
//		07/08/97	JRD	Moved destructor into header so it can be turned on and
//							off based on RSP_PROFILE_ON
//
//////////////////////////////////////////////////////////////////////////////
//***********************  TUTORIAL and INFORMATION  *************************
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// 1)  WHY WOULD I WANT TO USE THE RSPiX PROFILER?
//----------------------------------------------------------------------------
// (a) - The RSPiX profiler profides fully controllable, detailed, INTERACTIVE,
// multi session profiling under control of the program itself.  
//
// (b) - This profiler has almost NO OVERHEAD.  Your program will run at
// FULL SPEED, and in it's native state of caching.  You are measuring it's
// REAL behaviour, not it's altered behavior.
//
// (c) - It can be used to pin down WHEN certain slow downs occur as well as 
// where they occur.  
//
// (d) - It can specify the areas you wish to time with much more flexibility
// than commerical profilers.
//
// (e) - You don't have to rebuild your whole project every time you wish
// to turn profiling on or off.
//
// (f) - Timing more accurate than conventional profilers, so very small
//			non-repeating functions can be measured.
//
// - DISADVANTAGES OF USING THE RSPiX PROFILER - 
// 
// (a) - Since you tell it what to compare, it is better to first use a 
// general profiler to find the areas most suspect, than use the RSPiX
// profiler to really investigate.
//
// (b) - You must modify your source code to debug with it.  Luckily, 
// once in your code, it is COMPLETELY harmless when RSP_PROFILE_ON is
// NOT defined.  Like a TRACE or ASSERT statement, if the profiler's
// turned off, your code is COMPLETELY normal, so you can leave profile
// instructionsin if it is convenient.
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// 2)  HOW DO I OPERATE THE RSPiX PROFILER?
//----------------------------------------------------------------------------
//
//********************** A - Profile Pairs ***************************
//
//  You essentially define timing pairs you wish to profile.  The rest is magic.
//  Each profile pair you identify with a descriptive string: 
/*
rspStartProfile("memcpy in scene");

memcpy(.....);

rspEndProfile("memcpy in scene");
*/
// Take care that the text is the same in both commands, and there is a start and
// an end.  If you screw up, you will get a detailed error report telling you
// about it.
//
//********************** B - Exclusion Pairs ***************************
//
// Putting the rspEndProfile BEFORE the rspStartProfile is 100% legal.  It
// allows you to provide an EXCLUSIONARY profile, i.e., everything BUT a
// certain area of code:
/*
rspEndProfile("scene without memcpy");

memcpy(.....);

rspStartProfile("scene without memcpy");
*/
//
//********************** C - Disjoint pairs ***************************
//
// If you have more than one pair of the SAME name NOT NESTED, it will act
// as a group.  The total time will act as the total time of the group,
// but the average time will be the average time of EACH PART of the group,
// and not the combined average:
/*
rspStartProfile("memcpys in scene");

memcpy(.....);

rspEndProfile("memcpys in scene");

MyCode();

rspStartProfile("memcpys in scene");

memcpy(.....);

rspEndProfile("memcpys in scene");
*/
//
//********************** D - PERCENTAGE ***************************
//
// You can use any single pair in a given report at a baseline for 
// comparison.  Just start a pair with the phrase "100%", and all other
// timing pairs in that report will also give a relative percentage to
// your chosen base:
/*
rspStartProfile("100% - the scene"); // starting the name with 100% is the key
MyCode();

rspStartProfile("memcpys in scene"); // will show percentage relative to scene
memcpy(.....);
rspEndProfile("memcpys in scene");

rspEndProfile("100% - the scene");
*/
//
//********************** E - START and END TIME ***************************
//
// You can interactively control when timing begins in your program with
// the commands:
/*
rspProfileOn();
rspProfileOff();
*/
// Until the first rspProfileOn(); is encountered, none of your rspStartProfile or
// rspEndProfile tags will activate.  Similarly, once rspProfileOff() is used,
// all timing will cease and your report will be written.
//
//********************** F - MULTIPLE TIMING RUNS ***************************
//
// Perhaps the most important feature of the RSPiX profiler is that you can
// do an unlimited amount of rspProfileOn() and rspProfileOff() sessions 
// during the SAME run of your program, and ALL will be separately tallied
// and compared for you in your report.  You could, for example, use a 
// function ey to start and stop profiling during times in a game you think
// are significant.
//
// Independent profiles do NOT need to cover the SAME areas of your code.  They
// are, in fact, separate contexts, so you can freely repeat names between 
// them without conflict.  
//
// When an rspEndProfile() occurs, all profile pairs in progress have their final
// pass dropped from the record (no partial timing).
// Similarly, if an rspStartProfile occurs in the middle of timing pairs, they
// will not START timing until they have one FULL iteration. (again, no partial
// timing.)
//
//********************** G - Deluxe Reporting ***************************
//
// Once you get a report, you will see all remaining features, such as
// minimum and maximum speeds, and when they occurred.  You will see the
// relative time line of all your profiling against the program execution
// time.  You can get relaive percentages of timing.
//
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// 3)  HOW DO I "INSTALL" THE RSPiX PROFILER IN MY CURRENT PROJECT?
//----------------------------------------------------------------------------
// (a) - make sure "profile.cpp" is included in your project, either directly 
// or through the orange library.
//
// (b) - in any of your project files you wish to enable profiling, add an 
// include of "orange/debug/profil.h" or "rspix.lib".  This is harmless.
//
// (c) - in any file you wish to activate the profiler, #define RSP_PROFILE_ON.
// You may also define it as a project level macro, if you don't mind 
// recompiling all of your files.
//
// (d) - in some part of your program outside of the profiling area, you can
// define where you with the profile report to go by specifying 
// rspSetProfileOutput("MyOutputName.ext"); Your report will go here.
// Also, if you are NOT doing multi-session profiling, here is a good point
// to insert your one "rspProfileOn();" command.
//
// (e) - put in all of the profile directives into your code, and compile
// those files with RSP_PROFILE_ON defined.
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// 4)  IMPORTANT PORTABILITY ISSUES
//----------------------------------------------------------------------------
//  (a) - Currently, the RSPiX profiler makes heavy use of 64-bit integer math 
//  because it uses a special blue function, rpsGetAppMicroseconds, which returns 
//  the full 64-bit amount of microseconds elapsed.  To do this, it uses the 
//  non-standard S64 data type.  This needs to be changed on porting.
//
//  (b) - This program relies on FALSE being zero.
//
//  (c) - This program uses boolean algebra for timing consistency and REQUIRES
//  that a boolean value of TRUE is equal to +1.  If not, these sections must be 
//  altered.
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROFILE_H
#define _PROFILE_H

#include <stdlib.h>
#include <string.h>

#include "Blue.h"
//------------------------------------
// If you use global tags to do an
// profile_end, you will reduce overhead!

#define PF_MAX_FIELDS	20
#define PF_MAX_LEN	80 // no names more than this, please!
typedef	enum {Inactive,Timing,DoneTiming,InError}	ProfileState;

#ifdef __GNUC__
#define BIGS64(x) S64(x##ll)
#else
#define BIGS64(x) S64(x)
#endif

class	RProfileNode
	{
public:
	//----------------------------
	void	Init()
		{
		m_szFieldName[0] = 0; 
		m_lNumCalls = m_lTotTime = m_lLastTime = m_lMaxTime = m_lWhenMin = m_lWhenMax = 0; 
		m_lMinTime = BIGS64(999999999999);
		m_eState = Inactive;
		m_sInError = FALSE;
		}
	RProfileNode() 
		{ 
		Init();
		}

	~RProfileNode(){};
	//----------------------------
	char m_szFieldName[PF_MAX_LEN];
	S64	m_lNumCalls;
	S64	m_lTotTime;
	S64  m_lLastTime;

	S64	m_lMinTime;
	S64  m_lWhenMin; // relative to begin
	S64	m_lMaxTime;
	S64	m_lWhenMax; // relative to begin

	ProfileState m_eState;	// parenthesis verification!

#define PF_START_ERR	1
#define PF_END_ERR	2

	short	m_sInError;	// parenthesis mismatch!
	};

class	RProfile
	{
public:
	void	StartProfile(char* pszFieldName);
	void	EndProfile(char* pszFieldName);
	//----------------------------
	void	SetOutput(char *pszOutput)
		{
		if (pszOutput) strcpy(m_szOutputFile,pszOutput);
		}

	void ProfilingOn()
		{
		// Can't try to turn it back on even if there's been a problem!
		// (further profiles can be valid)

		m_sInternalError = m_sCommandError = FALSE;

		m_sActive = TRUE;

		// Reset all profiles in progress:
		m_sNumTracked = 0;

		// Reset th overhead as well:
		m_lCount = m_lTotTime = 0;
		m_lFastTimeIn = m_lFastTimeOut = 0;

		m_sLastUnaccounted = 0;
		m_sMaxDepth = m_sCurDepth = 0;

		// Get a new base time!
		m_lBeginTime = rspGetAppMicroseconds();
		}

	// Each time you turn off profiling, I'll do a report apend:
	//
	void ProfilingOff()
		{
		// don't allow double reports for double stops
		if (m_sActive == FALSE) return;

		m_sActive = FALSE;

		if (m_sNumTracked) Report();	// Give the report!

		// Deactivate all ranges (shouldn't really be needed!)
		/*
		for (short i=0; i < m_sNumTracked;i++)
			{
			if (m_aList[i].m_eState != InError) m_aList[i].m_eState = Inactive;
			}
			*/
		}

	//S64	DetermineTimeError();

	void Report(); // tell it all!
	//----------------------------

	// THIS IS IN THE HEADER SO IT CAN BE AFFECTED BY RSP_PROFILE_ON
	~RProfile()
		{
	#ifdef RSP_PROFILE_ON
		if (m_sActive) Report(); // No closing profile
		m_lDeceasedTime = rspGetAppMicroseconds(); // the Last element
		Report();	// Give the report!
	#endif
		};

	RProfile(); // in cpp file

	//----------------------------
	S64	m_lCount;		// to determine my own overhead!
	S64	m_lTotTime;		// Total overhead used by profiling!

	S64	m_lInitTime;		// start of program
	S64	m_lDeceasedTime;	// end of program

	S64	m_lBeginTime;	// when user kicks off profiling
	S64  m_lFirstTime;	// first time a profile range is entered in active mode

	//===== Let's try to max out a theme here:
	S64	m_lFastTimeIn;
	S64	m_lFastTimeOut;
	//----------------------------
	
	short	m_sLastUnaccounted;// Used for one frame lag timing of unknown overhead...
	short m_sNumTracked;		// how many in count?
	short m_sCommandError;	// mismatched parenthesis
	short m_sMaxDepth;		// Used for third order error estimation
	short	m_sCurDepth;		// Used for third order error estimation
	short	m_sActive;			// suspend / resume profiling...
	short	m_sInternalError;	// Usually memory limits...

	char	m_szOutputFile[256];
	RProfileNode m_aList[PF_MAX_FIELDS];
	};

/////////////////////////////////////////////////////////////////
// The new profiler has one global established instance:
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//  Set up the user interface convention:
/////////////////////////////////////////////////////////////////

extern	RProfile	rspProfileInstance; // The one global instance

inline void rspProfileOptimizeOut(char* a) {} // used to trick the compiler
inline void rspProfileOptimizeOut() {} // used to trick the compiler

#ifdef	RSP_PROFILE_ON
/////////////////////////////////////////////////////////////////
	#define	rspStartProfile(a)		rspProfileInstance.StartProfile(a)
	#define	rspEndProfile(a)			rspProfileInstance.EndProfile(a)
	#define	rspProfileOn()				rspProfileInstance.ProfilingOn()
	#define	rspProfileOff()			rspProfileInstance.ProfilingOff()
	#define	rspSetProfileOutput(a)	rspProfileInstance.SetOutput(a)
/////////////////////////////////////////////////////////////////
#else
/////////////////////////////////////////////////////////////////
	#define	rspStartProfile(a) 		1 ? (void)0 : rspProfileOptimizeOut(a)
	#define	rspEndProfile(a) 			1 ? (void)0 : rspProfileOptimizeOut(a)
	#define	rspProfileOn() 			1 ? (void)0 : rspProfileOptimizeOut()
	#define	rspProfileOff() 			1 ? (void)0 : rspProfileOptimizeOut()
	#define	rspSetProfileOutput(a) 	1 ? (void)0 : rspProfileOptimizeOut(a)
/////////////////////////////////////////////////////////////////
#endif



//------------------------------------
#endif
