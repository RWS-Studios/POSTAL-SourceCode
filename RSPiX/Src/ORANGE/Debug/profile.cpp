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
//		07/07/97 MJR	Switched to S64 instead of __int64.
//							Changed printf() stuff to use %g instead of %lg.
//
//		07/08/97 JRD	Removed destructor reporting when define is off.
//
//////////////////////////////////////////////////////////////////////////////
// Here's RSPiX's answer to controlled profiling.
// It currently uses Microseconds....
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "profile.h"
#include "ORANGE/str/str.h"

#if WIN32
	S64 i64GetTimeSpeed=7;	// how long does the time command take?
#else
	S64 i64GetTimeSpeed=8;	// Mike's guess on the mac
#endif


// The new profiler has one global established instance:
RProfile	rspProfileInstance;


//////////////////////////////////////////////////////////////////////////////
//
//	Start Profile => Begin a range!
//
//////////////////////////////////////////////////////////////////////////////
void	RProfile::StartProfile(char* pszFieldName)
	{
	short sKey;

	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : START ******
	//*****************************************************************************
	S64 i64EntryTime = rspGetAppMicroseconds() - i64GetTimeSpeed; // Track Overhead
	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : .END. ******
	//*****************************************************************************
	if (!m_sActive) return; // Do not activate if in error state!

	m_sLastUnaccounted = m_lFastTimeOut - m_lFastTimeIn; // Get last overhead time
	m_lFastTimeIn = i64EntryTime;


	//------------------------------------------------------
	// See if this name currently exists:
	//------------------------------------------------------
	short sMatch = FALSE;
	// Do the same length regardless (for consistency)
	short i;
	for (i=0; i < m_sNumTracked /*PF_MAX_FIELDS*/;i++)
		{
		if (!rspStricmp(pszFieldName,m_aList[i].m_szFieldName))
			{
			sMatch = TRUE;
			sKey = i;
			}
		}
	
	if (!sMatch) // first time!
		{
		//------------------------------------------------------
		// Create a new field!
		//------------------------------------------------------
		if (m_sNumTracked >= PF_MAX_FIELDS)
			{
			// All timing is void in an error scenario!
			//STRACE("RProfile::StartProfile: out of FIELDS, increase PF_MAX_FIELDS or like it!\n");
			m_sInternalError = TRUE;
			ProfilingOff();	// Turn off the whole thing on error, but report it!

			return;
			}
		
		// Note the time if it's the first one:
		if (!m_sNumTracked)	
			{
			m_lFirstTime = i64EntryTime;
			m_sLastUnaccounted = 0; // Assume previous overhead was zero!
			}

		m_aList[m_sNumTracked].Init();
		strcpy(m_aList[m_sNumTracked].m_szFieldName,pszFieldName);
		sKey = m_sNumTracked++; // the new handle
		}

	if (m_aList[sKey].m_eState == Timing)
		{
		// All timing is void in an error scenario!
		//STRACE("RProfile::StartProfile: Hit multiple starts without an end [%s]!\n",m_aList[sKey].m_szFieldName);
		m_aList[sKey].m_sInError |= PF_START_ERR;
		m_sCommandError = TRUE;
		ProfilingOff();	// Turn off the whole thing on error, but report it!

		return;
		}

	//------------------------------------------------------
	// Begin the profile:

	// We shall use the time at the end of the function to maximize accuracy.
	// Because of this, we will not start timing until after the adjustment's made.

	m_sCurDepth++; // You have descended down one level!
	if (m_sCurDepth > m_sMaxDepth) m_sMaxDepth = m_sCurDepth;

	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : START ******
	//*****************************************************************************
	// Track Overhead
	m_lCount++;
	m_lTotTime += m_sLastUnaccounted; // Total system overhead, lag one

	// Now remove this overhead from all currently begin timed!
	// We do all entries (slow) just to keep it consistent
	for (i=0;i < m_sNumTracked /*PF_MAX_FIELDS*/; i++)
		{	
		// Give the timing a boost by bumping up the start time!

		// NOTE: This part is slow so it can be consistant:
		// NOTE::: It assumes a boolean true has a value of +1!!!
		m_aList[i].m_lLastTime += m_sLastUnaccounted*(m_aList[i].m_eState == Timing);
		}

	m_aList[sKey].m_eState = Timing; // Now, about to begin timing:

	// Start the timing! (For Overhead and section.)
	//------------------------------------------------------
	m_lFastTimeOut = m_aList[sKey].m_lLastTime = rspGetAppMicroseconds();

	return;
	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : .END. ******
	//*****************************************************************************
	}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Very inefficient for now!
//
void	RProfile::EndProfile(char* pszFieldName)
	{
	short sKey;
	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : START ******
	//*****************************************************************************
	S64 i64EntryTime = rspGetAppMicroseconds() - i64GetTimeSpeed; // Track Overhead
	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : .END. ******
	//*****************************************************************************

	if (!m_sActive) return; // Do not activate if in error state!

	m_sLastUnaccounted = m_lFastTimeOut - m_lFastTimeIn; // Get last overhead time
	m_lFastTimeIn = i64EntryTime;

	//------------------------------------------------------
	// See if this name currently exists:
	//------------------------------------------------------

	short sMatch = FALSE;
	// Do the same length regardless (for consistency)
	short i;
	for (i=0; i < m_sNumTracked /*PF_MAX_FIELDS*/;i++)
		{
		if (!rspStricmp(pszFieldName,m_aList[i].m_szFieldName))
			{
			sMatch = TRUE;
			sKey = i;
			}
		}
	
	if (!sMatch) // First Time!
		{
		//------------------------------------------------------
		// Create a new field!
		//------------------------------------------------------
		if (m_sNumTracked >= PF_MAX_FIELDS)
			{
			// All timing is void in an error scenario!
			//STRACE("RProfile::StartProfile: out of FIELDS, increase PF_MAX_FIELDS or like it!\n");
			m_sInternalError = TRUE;
			ProfilingOff();	// Turn off the whole thing on error, but report it!

			return;
			}

		m_aList[m_sNumTracked].Init();
		strcpy(m_aList[m_sNumTracked].m_szFieldName,pszFieldName);
		sKey = m_sNumTracked++; // the new handle
		}

	if (m_aList[sKey].m_eState == DoneTiming)
		{
		// All timing is void in an error scenario!
		//STRACE("RProfile::EndProfile: Hit multiple ends without a start [%s]!\n",
		//	m_aList[sKey].m_szFieldName);
		m_aList[sKey].m_sInError |= PF_END_ERR;
		m_sCommandError = TRUE;
		ProfilingOff();	// Turn off the whole thing on error, but report it!

		return;
		}

	// In this case, use the timing at entry, since we will NOT be subtracting the overhead
	// from this run...

	// Allow poosibility of hitting this first:
	if (m_aList[sKey].m_eState == Timing) // you're in progress
		{
		S64 i64Diff = i64EntryTime - m_aList[sKey].m_lLastTime;
		m_aList[sKey].m_lTotTime += i64Diff;
		m_aList[sKey].m_lNumCalls++;	// A successful time
		m_sCurDepth--; // You have risen up one level!

		// Give current time as now - profile session start:
		// Remove conditions for consitency!
		if (i64Diff < m_aList[sKey].m_lMinTime) 
			{ 
			m_aList[sKey].m_lMinTime = i64Diff; 
			m_aList[sKey].m_lWhenMin = i64EntryTime - m_lBeginTime; 
			}

		if (i64Diff > m_aList[sKey].m_lMaxTime) 
			{ 
			m_aList[sKey].m_lMaxTime = i64Diff; 
			m_aList[sKey].m_lWhenMax = i64EntryTime - m_lBeginTime; 
			}
		}

	m_aList[sKey].m_eState = DoneTiming;
	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : START ******
	//*****************************************************************************
	// Track Overhead
	m_lCount++; 
	m_lTotTime += m_sLastUnaccounted; // Total system overhead, lag one

	// Now remove this overhead from all currently begin timed!
	// We do all entries (slow) just to keep it consistent
	for (i=0;i < m_sNumTracked /*PF_MAX_FIELDS*/; i++)
		{	
		// Give the timing a boost by bumping up the start time!

		// NOTE: This part is slow so it can be consistant:
		// NOTE::: It assumes a boolean true has a value of +1!!!
		m_aList[i].m_lLastTime += m_sLastUnaccounted*(m_aList[i].m_eState == Timing);
		}

	// Store this overhead to be lagged one use:
	m_lFastTimeOut = rspGetAppMicroseconds();

	return;
	//*****************************************************************************
	//************************************ TIME NOT BILLED CORRECTLY : .END. ******
	//*****************************************************************************
	}

short	gsReportNumber = 0;
//===============================================
void RProfile::Report()
	{
	FILE* fp;

	if (gsReportNumber == 0)
		{
		fp = fopen(m_szOutputFile,"w");
		}
	else
		{
		fp = fopen(m_szOutputFile,"a"); // append!
		}

	if (!m_lDeceasedTime) // degenerate report at end:
		{
		//=============================================== THE REAL REPORT ====================
		if (!gsReportNumber)
			{
			fprintf(fp,"Welcome to RSPiX Profiling!  Measurement error typically ranges from 60ns to 75us.\n\n");
			fprintf(fp,"SYMBOL KEY:  '@' - refers to the time at which something occurred relative to the START of the REPORT.\n");
			fprintf(fp,"Ex: '@+(s)' means the time in seconds after the start of the current report.\n");
			}

		fprintf(fp,"\n=======================\nREPORT #%d\n=======================\n\n",
			gsReportNumber + 1);
		fprintf(fp,"Activation occurred %g seconds into execution.\n",
						double(m_lBeginTime - m_lInitTime)/1000000.0);
		fprintf(fp,"Total report time was %g seconds.\n\n",
						double(m_lFastTimeOut - m_lBeginTime)/1000000.0);

		if (m_lCount) // safety
		fprintf(fp,"Profiler overhead: Tot(ms) = %g, # of calls = %ld, Avg(ms) = %g\n\n",
			double(m_lTotTime)/1000.0,long(m_lCount),double(m_lTotTime)/double(m_lCount * S64(1000)));
		
		fprintf(fp,"Number of profile ranges was %hd.\n",m_sNumTracked);

		fprintf(fp,"Maximum nesting depth was %hd\n\n",m_sMaxDepth-1);
		//--------------------------------------------------------------------------------------------
		// See if there are any typos which make this run invalid:

		if (m_sInternalError || m_sCommandError)
			{
			fprintf(fp,"\n************  PROFILING ERROR - DATA INVALID  *************\n\n");

			if (m_sInternalError)
				{
				fprintf(fp,"Internal memory exceeded - the profiler was designed to only\n"
					"handle %hd timing sets.  Please increase the number or get rid of old ones.\n\n",
					short(PF_MAX_FIELDS));

				return;
				}
			// Must be a user error:
			fprintf(fp,"You have made a command typo.  You cannot nest timing runs of the \n"
				"same name.  Nor can you have 'unbalanced parenthesis'.\n\n"
				"Here was the first command in error:\n");

			for (short i=0; i < m_sNumTracked;i++)
				{
				if (m_aList[i].m_sInError)
					{
					fprintf(fp,"%s: \n      ",m_aList[i].m_szFieldName);
					if (m_aList[i].m_sInError & PF_START_ERR) 
						fprintf(fp,"missing rspEndProfile\n");
					if (m_aList[i].m_sInError & PF_END_ERR) 
						fprintf(fp,"missing rspStartProfile\n");
					}
				}

			fclose(fp);	// since we're ducking out early, better sure up the file!
			gsReportNumber++;

			return;
			}

		//--------------------------------------------------------------------------------------------
		// See if there is a relative request:
		double dRel = 0.0;

		short i;
		for (i=0; i < m_sNumTracked;i++)
			{
			if (!strncmp(m_aList[i].m_szFieldName,"100%",4))
				{
				if (m_aList[i].m_lNumCalls)
					dRel = double(m_aList[i].m_lTotTime)/100.0;
				break;
				}
			}

		for (i=0; i < m_sNumTracked;i++)
			{
			if (m_aList[i].m_lNumCalls) // safety
				{
				fprintf(fp,"-------------------------------------------------------\n%s:\n",
					m_aList[i].m_szFieldName);
				fprintf(fp,"				# of passes = %ld, Tot(ms) = %g, Avg(ms) = %g",
					long(m_aList[i].m_lNumCalls),
					double(m_aList[i].m_lTotTime)/1000.0,
					double(m_aList[i].m_lTotTime)/double(m_aList[i].m_lNumCalls * S64(1000)));

				if (dRel)
					{
					double dRat = double(m_aList[i].m_lTotTime)/dRel;

					fprintf(fp,"  [%+1.6g %%]\n",double( long(dRat*10000.0) )/10000.0);
					}
				else
					fprintf(fp,"\n");

				//=========================
				fprintf(fp,"				MIN(ms): %+8.6g  @+(s): %+8.6g\n",
					double(m_aList[i].m_lMinTime)/1000.0,double(m_aList[i].m_lWhenMin)/1000000.0);
				fprintf(fp,"				MAX(ms): %+8.6g  @+(s): %+8.6g\n",
					double(m_aList[i].m_lMaxTime)/1000.0,double(m_aList[i].m_lWhenMax)/1000000.0);
				}
			}
		fprintf(fp,"-------------------------------------------------------\n");
		//=======================================================================================
		}
	else		// Only for the final time:
		{
		fprintf(fp,"\n==========================================\n");
		fprintf(fp,"\nTOTAL PROGRAM TIME = %g seconds\n",double(m_lDeceasedTime - m_lInitTime)/1000000.0);
		fprintf(fp,"Last timing run finished %g seconds before program exit.\n",
						double(m_lDeceasedTime - m_lFastTimeOut)/1000000.0);
		}

	fclose(fp);
	gsReportNumber++;
	}

RProfile::RProfile() 
	{ 
	m_sNumTracked = 0; 
	strcpy(m_szOutputFile,"rspProfile.txt");
	m_sActive = m_sCommandError = m_sInternalError = FALSE;
	m_sMaxDepth = 0;

	m_lCount = 0;
	m_lTotTime = m_lFirstTime = m_lBeginTime = m_lDeceasedTime = 
		m_lFastTimeIn = m_lFastTimeOut = 0;

	m_lInitTime = rspGetAppMicroseconds(); // the First element
	m_lInitTime = 0; // Blue will reset this to zero before I can reach it!

	m_sLastUnaccounted = 0; // perhaps this is for the best (sigh)
	}
