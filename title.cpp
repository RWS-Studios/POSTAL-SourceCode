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
// title.cpp
// Project: Postal
//
// This module deals with displaying the title sequence and the loading of
// assets during that time.
//
// History:
//		12/04/96 MJR	Started.
//
//		01/28/97	JMI	Took out feet and title.bmp and replaced them with a simple
//							progress bar and Pile640K.bmp.
//
//		01/31/97	JMI	Now uses FullPath() to get full path for filespecs.
//
//		02/02/97	JMI	Now, once EndTitle() is called, the progress bar is 
//							guaranteed to be completely filled.  Also, changed
//							static sLastFillPos local to DoTitle() to module static
//							so it can be initialized in StartTitle() and, therefore,
//							the title screen can be restarted.
//
//		02/03/97	JMI	Updated paths to reflect SAK dir.
//
//		02/24/97 MJR	Put in new title screen and modified progress bar.
//
//		04/14/97	JMI	Now shows one BMP for a portion of the title progress and
//							then a different one for the rest.
//							You can select the starting bitmap on the call to 
//							StartTitle().
//							Eventually, I imagine this will be generalized further with
//							an array of res names and durations (one for RWS, one for
//							the publisher, and one for Postal).
//
//		04/14/97	JMI	Temporarily disabled progress meter for ABC News demo.
//
//		06/03/97	JMI	Now displays the pre-release excuse after the RWS logo.
//
//		06/04/97	JMI	Now you can select a title page relative to the last one
//							by specifying a number < 1.
//
//		06/04/97	JMI	Rearranged title screens and added RC logo.
//
//		06/11/97	JMI	Added check against g_GameSettings.m_szDontShowTitles.
//							Also, DisplayImage() now does a ReleaseAndPurge() instead
//							of an rspReleaseResource() to save what would currently
//							amount to about 1.2M of RAM.
//
//		06/11/97	JMI	Changed RResMgr::ReleaseAndPurge() call to 
//							rspReleaseAndPurge().
//
//		06/12/97	JMI	Now IsInList() uses rspStricmp().
//							
//		06/12/97	JMI	Now only updates the palette of the new image if it is
//							different from the current.
//
//		06/16/97	JMI	Now centers title screens and erases entire screen even
//							no palette change.
//
//		07/09/97	JMI	Now uses its own g_smidTitle for the title noise instead of
//							relying on the menu one.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/23/97	JMI	Added rating.bmp as first title card.
//
//		08/08/97	JMI	Added parameter specifying whether to play musak to 
//							StartTitle().
//
//		08/18/97 BRH	Took out beta screen.
//
//		08/18/97 BRH	Added game ending sequence wiht backgrounds and audio.
//
//		08/18/97	JMI	Changed TitleRFileCallback to call Update() instead of
//							RMix::Do() (Update() calls RMix::Do() ) b/c Update()
//							makes the Mac load faster.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//							Also, put locks around accesses (Blits, Rects, etc.) to the
//							composite buffer.
//							Also, now loops the entire intro and does not abort it when
//							EndTitle() is called (it still stops the loopage, though).
//
//		08/23/97	JMI	Now you can get the sound instance of the title musak from
//							StartTitle().
//
//		08/30/97 BRH	Took out beta test screen.
//
//		09/01/97	BRH	Adjusted the final ending sequence so that the music starts
//							on a black screen, then the door is shown while you hear
//							the door slam sound, then it goes to the first sequence
//							picture of the tortured guy.
//
//		09/03/97	JMI	Many loops in Title_EndingSequence() wer calling RMix::Do()
//							instead of UpdateSystem().  While RMix::Do() will keep the
//							music running it does not give Blue a chance to realize
//							things like focus loss and video mode changes -- we should
//							always call UpdateSystem() in these loops.
//
//		09/04/97	JMI	Now calls UpdateSystem() every iteration instead of every
//							1/20th of a second.
//
//		09/07/97 BRH	You can now quit with Alt-F4 when the final end of
//							game sequence is playing.
//
//		09/07/97	JMI	Removed RipCord logo since the animated version is played
//							elsewhere (main.cpp).
//
//		09/08/97	JMI	Move Rip Cord static logo to front.
//
//		09/17/97	JMI	Removed unused heart beat interval timer, progress
//							meter calculations, and progress image loadage.
//
//		09/24/97 BRH	Added conditional compile based on LOCALE for the title
//							sequence.  The foreign versions have an additional
//							distributer screen after the ripcord logo, and the UK
//							warning screen is different.
//
//		09/25/97	JMI	Added #include of "comileOptions.h" so US, UK, etc. would
//							be defined.
//
//		09/30/97	JMI	Now sets and unsets RFile::ms_criticall in DisplayImage()
//							to guarantee it is being set during any title sequence.
//
//		10/21/97	JMI	A call to Title_DisableRipcordStaticLogo() will now disable
//							the RipCord static logo.
//
//		11/17/97	JMI	DoTitle() could theoretically blow m_adTitlePercent's 
//							bounds.
//
//		06/01/98 BRH	Removed the references to RIPCORD_LOGO_INDEX and the
//							code that disables the static logo to when the movie
//							logo is played etc.  Take2/AIM has requested that
//							the only logo on the game should be Running With Scissors
//							so we don't want Ripcord or Take2 logos at all.
//
//		09/27/99	JMI	Changed to allow deluxe violence disclaimer only in any 
//							locale satisfying the CompilerOptions macro VIOLENT_LOCALE.
//
//		01/20/00	MJR	Added screen for distributor logo.
//
//		03/30/00 MJR	Now uses TITLE_SHOW_DISTRIBUTOR to control whether the
//							distributor screen is shown.
//
////////////////////////////////////////////////////////////////////////////////
#define TITLE_CPP

#include "RSPiX.h"
#include "title.h"
#include "game.h"
#include "update.h"
#include "SampleMaster.h"
#include "CompileOptions.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define TITLE_BG_W			640
#define TITLE_BG_H			400

#define TITLE_BG_X			(g_pimScreenBuf->m_sWidth / 2 - TITLE_BG_W / 2)
#define TITLE_BG_Y			(g_pimScreenBuf->m_sHeight / 2 - TITLE_BG_H / 2)

#define PROGRESS_W			(600-8)

#define TITLE_SOUND_UPDATE_INTERVAL 50

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

// Percentage of progress at which time we switch to the next image.
#define TOGGLE_BG_PERCENT			(100 / NUM_ELEMENTS(ms_apszFiles) + 1)

// Token delimiters.
#define TOKEN_DELIMITERS			"\t ,"

#define MAX_TITLES				10

#define NUM_IMAGES				NUM_ELEMENTS(ms_apszFiles)

#define MUSAK_START_TIME	0	// 11260
#define MUSAK_END_TIME		0

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

long m_lTotalUnits;
long m_lCummUnits;		// I will brace myself for an onslaught of jokes.
double m_adTitlePercent[MAX_TITLES+1];
static short	m_sValid			= FALSE;

static long		ms_lTitleRFileCallbackTime = 0;

// Indicates the currently displayed image.
static short	ms_sImageNum		= 0;

// The instance of the title musak sample.
static SampleMaster::SoundInstance	ms_siMusak;
static SampleMaster::SoundInstance  ms_siEndingAudio;

static bool	ms_bDisableRipcordStaticLogo	= false;

// These are the images (in the order) to display.
static char*	ms_apszFiles[]	=
	{
	// Even the rating disclaimer is too violent for some countries.
	#if VIOLENT_LOCALE
		"Title/rating.bmp",
	#else
		"Title/ratingUK.bmp",
	#endif
	#ifdef TITLE_SHOW_DISTRIBUTOR
		"Title/distrib.bmp",
	#endif
	"Title/Logo2.bmp",
	"Title/Postal.bmp"
	};				 

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// TitleRFileCallback
//
//	Callback that will call the Update() while the next background is being
// loaded.
//
////////////////////////////////////////////////////////////////////////////////

static void TitleRFileCallback(long lBytes)
{
	long lCurrentTime = rspGetMilliseconds();
	if ((lCurrentTime - ms_lTitleRFileCallbackTime) > TITLE_SOUND_UPDATE_INTERVAL)
	{
		UpdateSystem();
		ms_lTitleRFileCallbackTime = rspGetMilliseconds();
	}
}


////////////////////////////////////////////////////////////////////////////////
// Returns true if the specified string is in the comma separated list.
////////////////////////////////////////////////////////////////////////////////
static bool IsInList(	// Returns true if in list.  false otherwise.
	char*	pszSearchFor,	// In:  String to search for in pszSearchIn.
	char*	pszSearchIn)	// In:  Comma delimited list of strings to search in for
								// pszSearchFor.
	{
	bool	bFound	= false;	// Assume not found.

	// Copy to temp so strtok can tokenize leaving full of NULLs.
	char	szTokenize[512];
	strncpy(szTokenize, pszSearchIn, sizeof(szTokenize) - 1);
	szTokenize[sizeof(szTokenize) - 1] = '\0';

	// Tokenize.
	char*	pszToken	= strtok(szTokenize, TOKEN_DELIMITERS);
	while (pszToken != NULL)
		{
		if (rspStricmp(pszToken, pszSearchFor) == 0)
			{
			// Found it.
			bFound	= true;
			break;
			}

		pszToken	= strtok(NULL, TOKEN_DELIMITERS);
		}

	return bFound;
	}

////////////////////////////////////////////////////////////////////////////////
// Loads, displays, and disgards image from specified file.
////////////////////////////////////////////////////////////////////////////////
static short DisplayImage(	// Returns nothing.
	char*	pszImageFile)		// Filename of image (relative path).
	{
	// Store the original callback.
	RFile::CritiCall	criticallRestore	= RFile::ms_criticall;

	// Set the callback
	RFile::ms_criticall = TitleRFileCallback;
	ms_lTitleRFileCallbackTime = rspGetMilliseconds();

	RImage*	pimTitle;
	short sResult = rspGetResource(&g_resmgrShell, pszImageFile, &pimTitle);
	if (sResult == 0)
		{
		// Determine position for new image.
		short	sX	= g_pimScreenBuf->m_sWidth / 2 - pimTitle->m_sWidth / 2;
		short	sY	= g_pimScreenBuf->m_sHeight / 2 - pimTitle->m_sHeight / 2;

		// Set palette
		ASSERT(pimTitle->m_pPalette != NULL);
		ASSERT(pimTitle->m_pPalette->m_type == RPal::PDIB);

		// Get the new palette.
		U8*	pu8NewRed	= pimTitle->m_pPalette->Red(0);
		U8*	pu8NewGreen	= pimTitle->m_pPalette->Green(0);
		U8*	pu8NewBlue	= pimTitle->m_pPalette->Blue(0);

		short	sStartIndex	= pimTitle->m_pPalette->m_sStartIndex;
		short	sNumEntries	= pimTitle->m_pPalette->m_sNumEntries;
		short	sEntrySize	= pimTitle->m_pPalette->m_sPalEntrySize;

		// Get the current palette.
		U8		au8CurRed[256];
		U8		au8CurGreen[256];
		U8		au8CurBlue[256];
		rspGetPaletteEntries(
			sStartIndex,
			sNumEntries,
			au8CurRed,
			au8CurGreen,
			au8CurBlue,
			1);

		// Compare.
		bool	bSetPalette	= false;	// true to set new palette.
		short	i;
		U8*	pu8NewRedEntry		= pu8NewRed;
		U8*	pu8NewGreenEntry	= pu8NewGreen;
		U8*	pu8NewBlueEntry	= pu8NewBlue;
		U8*	pu8CurRedEntry		= au8CurRed;
		U8*	pu8CurGreenEntry	= au8CurGreen;
		U8*	pu8CurBlueEntry	= au8CurBlue;
		for (i = 0; i < sNumEntries; i++)
			{
			if (	*pu8CurRedEntry++		!= *pu8NewRedEntry
				||	*pu8CurGreenEntry++	!= *pu8NewGreenEntry
				||	*pu8CurBlueEntry++	!= *pu8NewBlueEntry)
				{
				bSetPalette	= true;
				break;
				}

			pu8NewRedEntry		+= sEntrySize;
			pu8NewGreenEntry	+= sEntrySize;
			pu8NewBlueEntry	+= sEntrySize;
			}

		// Lock the RSPiX composite buffer so we can access it.
		rspLockBuffer();

		// Blank screen before updating palette.
		rspRect(
			RSP_BLACK_INDEX,
			g_pimScreenBuf,
			0,
			0,
			g_pimScreenBuf->m_sWidth,
			g_pimScreenBuf->m_sHeight);
	
		// If we need to set the palette . . .
		if (bSetPalette == true)
			{
			rspSetPaletteEntries(
				sStartIndex,
				sNumEntries,
				pu8NewRed,
				pu8NewGreen,
				pu8NewBlue,
				sEntrySize);

			// Unlock to update the display.
			rspUnlockBuffer();

			// Make sure screen is black during palette change.
			rspUpdateDisplay();
		
			// Lock the RSPiX composite buffer so we can access it again.
			rspLockBuffer();

			// Update hardware palette.
			rspUpdatePalette();
			}

		// Show title image.
		rspBlit(
			pimTitle, 
			g_pimScreenBuf, 
			0, 0, 
			sX, 
			sY, 
			pimTitle->m_sWidth, 
			pimTitle->m_sHeight);

		// Unlock now that we're done with the composite buffer.
		rspUnlockBuffer();

		// Update display.
		rspUpdateDisplay();

		rspReleaseAndPurgeResource(&g_resmgrShell, &pimTitle);
		}
	else
		{
		TRACE("DisplayImage(): Error loading %s!\n", pszImageFile);
		}

	// Restore the callback.
	RFile::ms_criticall = criticallRestore;

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Loads, displays, and disgards image from file specified via image num.
////////////////////////////////////////////////////////////////////////////////
static short DisplayImageNum(	// Returns nothing.
	short	sImageNum)				// In:  Image Num to show [1..n].
	{
	short	sRes	= 0;	// Assume success.
	
	// Switch to array indexing mode.
	sImageNum--;

	// If not in list of no shows . . .
	while (sImageNum < NUM_ELEMENTS(ms_apszFiles) )
		{
		if (IsInList(ms_apszFiles[sImageNum], g_GameSettings.m_szDontShowTitles) == false)
			{
			break;
			}

		sImageNum++;
		}

	if (sImageNum < NUM_ELEMENTS(ms_apszFiles) )
		{
		sRes	= DisplayImage(ms_apszFiles[sImageNum]);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Start the title sequence, which includes some type of progress meter, which
// may or may not (hopefully not) look like a standard progress meter.  The
// total range of the progress bar is determined by lTotalUnits.  As far as
// this module is concerned, these units are completely abstract.
//
////////////////////////////////////////////////////////////////////////////////
extern short StartTitle(							// Returns 0 if successfull, non-zero otherwise
	short	sStartImage /*= 1*/,						// In:  Image to start with.  Values less
															// than 1 indicate a page relative to the
															// end.
	bool	bPlayMusak /*= false*/,					// In:  true to play title musak.
	SampleMaster::SoundInstance* psi /*= 0*/)	// Out:  Sound instance of musak.
	{
	short sResult = 0;

	// Save total units and reset other stuff
	short i;
	m_lTotalUnits = 0;
	for (i = 0; i < NUM_ELEMENTS(ms_apszFiles); i++)
		m_lTotalUnits += g_GameSettings.m_alTitleDurations[i];

	// Avoid divide by zero and other possible screw-ups.
	if (m_lTotalUnits <= 0)
		{
		m_lTotalUnits	= 1;
		}

	m_adTitlePercent[0] = 0;
	for (i = 0; i < NUM_ELEMENTS(ms_apszFiles); i++)
		m_adTitlePercent[i+1] = ((double) g_GameSettings.m_alTitleDurations[i] / (double) m_lTotalUnits) + m_adTitlePercent[i];

	m_lCummUnits = 0;
	m_sValid = 0;

	// If value less than 1 . . .
	if (sStartImage < 1)
		{
		// It specifies a value relative to the end.
		sStartImage	+= NUM_ELEMENTS(ms_apszFiles);
		}

	// Force this sample to load now
//	CacheSample(g_smidTitle);


	// Display title screen
	sResult = DisplayImageNum(sStartImage);
	if (sResult == 0)
		{
		// If told to play sample . . .
		if (bPlayMusak)
			{
			// If not already playing . . .
			if (IsSamplePlaying(ms_siMusak) == false)
				{
				PlaySample(										// Returns nothing.
																	// Does not fail.
					g_smidTitleMusak,							// In:  Identifier of sample you want played.
					SampleMaster::BackgroundMusic,		// In:  Sound Volume Category for user adjustment
					255,											// In:  Initial Sound Volume (0 - 255)
					&ms_siMusak,								// Out: Handle for adjusting sound volume
					NULL,											// Out: Sample duration in ms, if not NULL.
					MUSAK_START_TIME,							// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
					MUSAK_END_TIME,							// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
					true);										// In:  Call ReleaseAndPurge rather than Release after playing

				// Copy sound instance for user if given a destination for it.
				if (psi)
					{
					*psi	= ms_siMusak;
					}
				}
			}

		m_sValid				= TRUE;

		ms_sImageNum		= sStartImage;
		}
	else
		{
		TRACE("StartTitle(): Error loading %s!\n", ms_apszFiles[sStartImage - 1]);
		}


	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// TitleGetNumTitles - give the number of title screens in use
////////////////////////////////////////////////////////////////////////////////

extern short TitleGetNumTitles(void)
	{
	return NUM_ELEMENTS(ms_apszFiles);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Update the title sequence.  The specified number of units are added to a
// running total.  The ration between the running total and the value passed to
// StartTitle() determines the new position of the progress meter.
//
////////////////////////////////////////////////////////////////////////////////
extern short DoTitle(						// Returns 0 if successfull, non-zero otherwise
	long lUnits)								// In:  Additional progess units
	{
	short sResult = 0;

	// Can't call this if StartTitle() didn't work
	if (m_sValid)
		{
		// Add to cummulative total
		m_lCummUnits += lUnits;

		// Calculate ratio and use it to determine how much progress bar juice
		// we should display.
		double dRatio = (double)m_lCummUnits / (double)m_lTotalUnits;
		
		// While we've passed the duration before displaying the next image.
		// This loop is intended to skip title cards that have 0 for their
		// duration.
		// NOTE that m_adTitlePercent is valid up to and including Num Images + 1.
		while (dRatio > m_adTitlePercent[ms_sImageNum])
			{
			// Advance image.
			if (ms_sImageNum++ < TitleGetNumTitles() )
				{
				// If there's at least a smidgen of time allocated for this image . . .
				if (g_GameSettings.m_alTitleDurations[ms_sImageNum] )
					{
					// Display new image.
					DisplayImageNum(ms_sImageNum);
					}
				}
			else
				{
				break;
				}
			}
		}
	else
		{
		sResult = -1;
		TRACE("DoTitle(): It appears that StartTitle() wasn't called or didn't complete successfully!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// When you are completely done, call EndTitle().  This gives the title sequence
// a chance to fully complete the progess meter (in case it never reached 100%
// due to an overestimated lTotalUnits) and allows all resources to be freed.
//
////////////////////////////////////////////////////////////////////////////////
extern short EndTitle(void)				// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// It's okay to call this even if StartTitle() didn't work
	if (m_sValid)
		{
		// Display this stuff so we can easily tune the total units based on the
		// actual units that were passed to this module.
		TRACE("EndTitle(): lTotalUnits = %ld, lCummUnits = %ld\n", m_lTotalUnits, m_lCummUnits);

		// Always pretend we made it, even if we didn't.
		DoTitle(ABS(m_lTotalUnits - m_lCummUnits) );

		// Lock the RSPiX composite buffer so we can access it.
		rspLockBuffer();

		// Erase progress meter
		RRect	rcClip(0, 440, 640, 40);
		rspRect(
			RSP_BLACK_INDEX,
			g_pimScreenBuf,
			0,
			440,
			640,
			40,
			&rcClip);

		// Unlock now that we're done with the composite buffer.
		rspUnlockBuffer();

		rspUpdateDisplay();

		// Make sure last sample finishes
//		while (IsSamplePlaying(g_smidTitle))
//			Update();

		// Reset.
		m_sValid	= FALSE;
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Title_GameEndSequence
//
// Show the end of game sequence when the player wins.  This will include 4 
// screens and audio clips.
//
////////////////////////////////////////////////////////////////////////////////

void Title_GameEndSequence(void)
{
	long lDisplayTime = 0;
	long lTotalTime = 0;
	long lSectionTime = 0;
	long lCurrentTime;
	long lUpdateTime;
	TRACE("So this is the big end of game sequence eh?\n");

	// Play the sound
	PlaySample(										// Returns nothing.
														// Does not fail.
		g_smidEndingAudio,						// In:  Identifier of sample you want played.
		SampleMaster::Unspecified,				// In:  Sound Volume Category for user adjustment
		255,											// In:  Initial Sound Volume (0 - 255)
		&ms_siEndingAudio,						// Out: Handle for adjusting sound volume
		&lTotalTime,								// Out: Sample duration in ms, if not NULL.
		-1,											// In:  Where to loop back to in milliseconds.
														//	-1 indicates no looping (unless m_sLoop is
														// explicitly set).
		0,												// In:  Where to loop back from in milliseconds.
														// In:  If less than 1, the end + lLoopEndTime is used.
		true);										// In:  Call ReleaseAndPurge rather than Release after playing

	// Set the callback
	RFile::ms_criticall = TitleRFileCallback;
	ms_lTitleRFileCallbackTime = rspGetMilliseconds();

	// Show black while the audio starts and then put up the door when the door slam
	// sound is heard.
	lCurrentTime = rspGetMilliseconds();
	lDisplayTime = lCurrentTime + 2500;
	lTotalTime -= 2500;
	lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
	while (lCurrentTime < lDisplayTime)
	{
		if (lCurrentTime > lUpdateTime)
		{
			lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
		}
		UpdateSystem();
		// If they hit Alt-F4 then quit the loop
		if (rspGetQuitStatus())
			lCurrentTime = lDisplayTime;
		else
			lCurrentTime = rspGetMilliseconds();
	}

	// Show the door for a short time 
	DisplayImage("Title/fdoor.bmp");
	lCurrentTime = rspGetMilliseconds();
	lDisplayTime = lCurrentTime + 3000;
	lTotalTime -= 3000;
	lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
	while (lCurrentTime < lDisplayTime)
	{
		if (lCurrentTime > lUpdateTime)
		{
			lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
		}
		UpdateSystem();

		// If they hit Alt-F4, then quit the loop
		if (rspGetQuitStatus())
			lCurrentTime = lDisplayTime;
		else
			lCurrentTime = rspGetMilliseconds();
	}

	// Divide up the rest of the time.
	lSectionTime = lTotalTime / 4;

	// Display the ending backgrounds.
	DisplayImage("Title/corridor.bmp");
	lDisplayTime = rspGetMilliseconds() + lSectionTime;
	lCurrentTime = rspGetMilliseconds();
	lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
	while (lCurrentTime < lDisplayTime)
	{
		if (lCurrentTime > lUpdateTime)
		{
			lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
		}
		UpdateSystem();
		// If they hit Alt-F4, then quit the loop
		if (rspGetQuitStatus())
			lCurrentTime = lDisplayTime;
		else
			lCurrentTime = rspGetMilliseconds();
	}

	DisplayImage("Title/jacketbg.bmp");
	lDisplayTime = rspGetMilliseconds() + lSectionTime;
	lCurrentTime = rspGetMilliseconds();
	lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
	while (lCurrentTime < lDisplayTime)
	{
		if (lCurrentTime > lUpdateTime)
		{
			lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
		}
		UpdateSystem();
		// If they hit Alt-F4, then quit the loop
		if (rspGetQuitStatus())
			lCurrentTime = lDisplayTime;
		else
			lCurrentTime = rspGetMilliseconds();
	}

	DisplayImage("Title/torture2.bmp");
	lDisplayTime = rspGetMilliseconds() + lSectionTime;
	lCurrentTime = rspGetMilliseconds();
	lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
	while (lCurrentTime < lDisplayTime)
	{
		if (lCurrentTime > lUpdateTime)
		{
			lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
		}
		UpdateSystem();
		// If they hit Alt-F4, then quit the loop
		if (rspGetQuitStatus())
			lCurrentTime = lDisplayTime;
		else
			lCurrentTime = rspGetMilliseconds();
	}

	DisplayImage("Title/fdoor.bmp");
	lDisplayTime = rspGetMilliseconds() + lSectionTime;
	lCurrentTime = rspGetMilliseconds();
	lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
	while (lCurrentTime < lDisplayTime)
	{
		if (lCurrentTime > lUpdateTime)
		{
			lUpdateTime = lCurrentTime + TITLE_SOUND_UPDATE_INTERVAL;
		}
		UpdateSystem();
		// If they hit Alt-F4, then quit the loop
		if (rspGetQuitStatus())
			lCurrentTime = lDisplayTime;
		else
			lCurrentTime = rspGetMilliseconds();
	}

	// Unset the callback
	RFile::ms_criticall = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Disable RipCord static logo.
////////////////////////////////////////////////////////////////////////////////
extern void Title_DisableRipcordStaticLogo(void)
	{
	ms_bDisableRipcordStaticLogo	= true;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
