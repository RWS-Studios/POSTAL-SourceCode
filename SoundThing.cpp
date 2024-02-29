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
// SoundThing.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		02/24/97 MJR	Stole infrastructure from Jon's AnimThing.
//
//		03/07/97	JMI	Now remembers last channel played with and will attempt
//							to abort the sample, if this object is killed.
//							This obejct now responds to the delete message.
//
//		03/07/97	JMI	Was only checking messages when the sound looped.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/25/97	JMI	Changed EDIT_GUI_FILE to 8.3 compliant name.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/22/97	JMI	When I added the remembering of the last channel played,
//							I forgot to actually remember the last channel played.
//							Duh!  Fixed.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/29/97	JMI	Converted EditRect(), EditRender(), and/or Render() to
//							use Map3Dto2D().
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/17/97	JMI	Changed RSnd*'s to SampleMaster::SoundInstances.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//							Also, added user edittable member m_lVolumeHalfLife.
//
//		07/25/97 MJR	Added the loading/saving of x,y,z.
//
//		07/28/97	JMI	Now varies volume as sound plays (based on distance to
//							ear).
//							Also, makes sure the next sound does not start until at
//							least after the sound sample should be done.
//
//		08/01/97	JMI	Added looping parameters.
//							Also, fixed bug where &m_b* were casted to long* (but
//							they are not 32 bits).
//
//		08/04/97	JMI	Made hiding and showing of id 499, in UseLoopingGuiCall,
//							_slightly_ more efficient.
//
//		08/04/97	JMI	Added m_sAmbient indicating whether or not this sound
//							is ambient (i.e., non-essential).
//							Also, implemented a special random number generator
//							strictly for sound things so they can be merry and random
//							and not de-synchronize.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/09/97	JMI	Added better UI for 'loop back from end' usage.
//							Now offers a checkbox option to loop infinitely.
//
//		08/11/97	JMI	Added RelayVolume() so CSoundRelays can update their
//							CSoundThing parents.
//
//		08/12/97	JMI	Now can browse for files with paths relative to the
//							samples SAK.
//
//		09/27/99	JMI	Eliminated boolean performance warnings.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will play sounds with various options.
//
//////////////////////////////////////////////////////////////////////////////
#define SOUNDTHING_CPP

#include "RSPiX.h"
#include "SoundThing.h"
#include "game.h"

// This class has its own GetRandom() to keep it from de-synching the game.
#ifdef GetRandom
	#undef GetRandom
#endif

#ifdef GetRand
	#undef GetRand
#endif

#ifdef MOBILE //Arm RAND_MAX is a full int, code expecting a short!!
#define RAND_MAX 0x7fff
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"soundthing.bmp"

#define GUI_FILE_NAME		"res/editor/Sound.gui"

#define END_OF_TIME			0x7fffffff	// Unfortunately, we won't be dead (or,
													// considering it's always just 24 days 
													// off, fortunately).  The program, will
													// break, however.

////////////////////////////////////////////////////////////////////////////////
// Class statics.
////////////////////////////////////////////////////////////////////////////////

short	CSoundThing::ms_sFileCount			= 0;	// File count.         
long	CSoundThing::ms_lGetRandomSeed	= 0;	// Seed for get random.

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// If new file . . . 
		if (sFileCount != ms_sFileCount)
			{
			ms_sFileCount	= sFileCount;
			
			// Do one time stuff.

			// Reseed local random.
			ms_lGetRandomSeed	= 0;
			}

		switch (ulFileVersion)
			{
			default:
			case 40:
				pFile->Read(&m_sAmbient);
			case 39:
			case 38:
				pFile->Read(&m_sPurgeSampleWhenDone	);
				pFile->Read(&m_sUseLooping				);
				pFile->Read(&m_lStopLoopingTime		);
				pFile->Read(&m_lNumLoopBacks			);
				pFile->Read(&m_lLoopBackTo				);
				pFile->Read(&m_lLoopBackFrom			);
																
			case 37:
			case 36:
			case 35:
			case 34:
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
			case 33:
			case 32:
			case 31:
				pFile->Read(&m_lVolumeHalfLife);
			case 30:
			case 29:
			case 28:
			case 27:
			case 26:
			case 25:
			case 24:
			case 23:
			case 22:
			case 21:
			case 20:
			case 19:
			case 18:
			case 17:
			case 16:
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
			case 7:
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				long	lBool;
				pFile->Read(&lBool);
				m_bInitiallyEnabled	= lBool ? true : false;
				pFile->Read(&lBool);
				m_bInitiallyRepeats	= lBool ? true : false;
				pFile->Read(m_lMinTime, 2);
				pFile->Read(m_lRndTime, 2);
				pFile->Read(m_szResName);
				break;
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CSoundThing::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		pFile->Write(m_sAmbient);
		pFile->Write(m_sPurgeSampleWhenDone	);
		pFile->Write(m_sUseLooping				);
		pFile->Write(m_lStopLoopingTime		);
		pFile->Write(m_lNumLoopBacks			);
		pFile->Write(m_lLoopBackTo				);
		pFile->Write(m_lLoopBackFrom			);
		pFile->Write(m_dX);
		pFile->Write(m_dY);
		pFile->Write(m_dZ);
		pFile->Write(m_lVolumeHalfLife);
		pFile->Write((long)m_bInitiallyEnabled);
		pFile->Write((long)m_bInitiallyRepeats);
		pFile->Write(m_lMinTime, 2);
		pFile->Write(m_lRndTime, 2);
		pFile->Write(m_szResName);

		// Make sure there were no file errors
		sResult	= pFile->Error();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{
	// Set the collective volume to zero to start.
	m_lCollectiveVolume	= 0;

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::Resume(void)
	{
	m_sSuspend--;

	// If we're actually going to start updating again....
	if (m_sSuspend == 0)
		{
		// This is kind of cheesy, but it will work to some degree
		m_lLastStartTime = m_pRealm->m_time.GetGameTime();
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::Update(void)
	{
	if (!m_sSuspend)
		{
		// Get current time
		long lCurTime = m_pRealm->m_time.GetGameTime();

		// If enabled and (non-ambient or ambients allowed) . . .
		if (m_bEnabled == true && (m_sAmbient == FALSE || g_GameSettings.m_sPlayAmbientSounds) )
			{
			// If current time hits next starting time (or if we're in the init state)
			if ((lCurTime >= m_lNextStartTime) || (m_sWhichTime < 0))
				{
				long	lSampleDuration	= 0;
				long	lLoopStartTime;
				long	lLoopEndTime	= m_lLoopBackFrom;
				if (m_sUseLooping)
					{
					lLoopStartTime	= m_lLoopBackTo;
					}
				else
					{
					// Indicate no looping.
					lLoopStartTime	= -1;
					}

				// If first time (not init) OR nth time with repeat enabled . . .
				// Play the sound (unless it's the init state)
				if (m_sWhichTime == 0 || (m_sWhichTime > 0 && m_bRepeats == true) )
					{
					// If using loop parameters . . .
					if (m_sUseLooping)
						{
						// Stop looping existing sound.
						StopLoopingSample(m_siChannel);
						}

					bool	bPurge;
					if (m_sPurgeSampleWhenDone)
						{
						bPurge	= true;
						}
					else
						{
						bPurge	= false;
						}

					PlaySample(																	// Returns nothing.
																									// Does not fail.
						m_id,																		// In:  Identifier of sample you want played.
						SampleMaster::Ambient,												// In:  Sound Volume Category for user adjustment
						DistanceToVolume(m_dX, m_dY, m_dZ, m_lVolumeHalfLife),	// In:  Initial Sound Volume (0 - 255) 
						&m_siChannel,															// Out: Handle for adjusting sound volume
						&lSampleDuration,														// Out: Sample duration in ms, if not NULL.
						lLoopStartTime,														// In:  Where to loop back to in milliseconds.                
																									//	-1 indicates no looping (unless m_sLoop is                 
																									// explicitly set).                                           
						lLoopEndTime,															// In:  Where to loop back from in milliseconds.              
																									// If less than 1, the end + lLoopEndTime is used.       
						bPurge);																	// In:  Call ReleaseAndPurge rather than Release after playing

					///////////////////////////////////////////////////////////////////////////////////////////////////
					// NOTE: We have to use the exact logic RSnd does to determine the loop points so we can correctly
					// determine the amount of time until we stop looping.
					///////////////////////////////////////////////////////////////////////////////////////////////////

					// If using the end . . .
					if (lLoopEndTime < 1)
						{
						// Use the duration plus the specified negative time.
						lLoopEndTime		+= lSampleDuration;
						}

					// Fix these values in case we're in release mode.
					if (lLoopStartTime > lSampleDuration)
						{
						lLoopStartTime = lSampleDuration;
						}

					if (lLoopEndTime > lSampleDuration)
						{
						lLoopEndTime = lSampleDuration;
						}

					if (lLoopStartTime < 0 && m_sUseLooping)
						{
						lLoopStartTime = 0;
						}

					if (lLoopStartTime > lLoopEndTime)
						{
						lLoopStartTime = lLoopEndTime;
						}

					// If using loop parameters and loop backs are not infinite . . .
					if (m_sUseLooping && m_lNumLoopBacks >= 0)
						{
						// Calculate time until we stop looping . . .
						m_lStopLoopingTime	= lCurTime + (m_lNumLoopBacks + 1) * (lLoopEndTime - lLoopStartTime) + lLoopStartTime;
						}
					else
						{
						// Set time way into the future so we don't bother for a while.
						m_lStopLoopingTime	= END_OF_TIME;
						}
					}

				// Save current start time (use lCurTime in case we're in the init state)
				m_lLastStartTime = lCurTime;
				
				// Inc which time to use (don't go past 1)
				if (m_sWhichTime < 1)
					m_sWhichTime++;


				// Pick random time between 0 and specified random time
				long	lRnd = (long)(((float)GetRand() / (float)RAND_MAX) * (float)m_lRndTime[m_sWhichTime]);
				// Make sure this at least the length of the sample.
				long	lWaitDuration	= MAX(long(m_lMinTime[m_sWhichTime] + lRnd), lSampleDuration);
				// Calculate next starting time
				m_lNextStartTime = m_lLastStartTime + lWaitDuration;
				}
			
			// Relay our volume (we act as just another satellite).
			RelayVolume(DistanceToVolume(m_dX, m_dY, m_dZ, m_lVolumeHalfLife) ); 
			}

		// If time to stop looping . . .
		if (lCurTime >= m_lStopLoopingTime)
			{
			if (m_sUseLooping)
				{
				// Stop looping existing sound.
				StopLoopingSample(m_siChannel);
				}

			// Set time way into the future so we don't bother for a while.
			m_lStopLoopingTime	= END_OF_TIME;
			}

		// Process messages.
		ProcessMessages();

		switch (m_state)
			{
			case State_Happy:
				break;
			case State_Delete:
				// Banzai!
				delete this;
				return ;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::Render(void)
	{
	// Render our sound.  What a cheezy way of trying to justify putting this 
	//	here.

	// Adjust volume of last play instance.  Clip just in case.
	SetInstanceVolume(m_siChannel, MIN(255L, m_lCollectiveVolume) );

	// Reset volume.
	m_lCollectiveVolume	= 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	sResult	= EditModify();

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Helper function/macro for changing a GUIs text value.
////////////////////////////////////////////////////////////////////////////////
inline void SetGuiItemVal(	// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  GUI Root.
	long			lId,			// In:  ID of item whose text we'll change.
	long			lVal)			// In:  New value.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui)
		{
		pgui->SetText("%ld", lVal);
		pgui->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Callback from multibtn checkbox.
////////////////////////////////////////////////////////////////////////////////
static void CheckEnableGuiCall(	// Returns nothing.
	RGuiItem*	pgui_pmb)			// In:  GUI pointer to the multi button that was 
											// pressed.
	{
	ASSERT(pgui_pmb->m_type == RGuiItem::MultiBtn);

	RMultiBtn*	pmb	= (RMultiBtn*)pgui_pmb;

	// Show based on value stored in GUI.
	short	sVisible	= pmb->m_ulUserData;
	// If unchecked . . .
	if (pmb->m_sState == 1)
		{
		// Opposite show/hide state.
		sVisible	= !sVisible;
		}

	RGuiItem*	pguiLoopSettingsContainer	= pmb->GetParent()->GetItemFromId(pmb->m_ulUserInstance);
	if (pguiLoopSettingsContainer)
		{
		pguiLoopSettingsContainer->SetVisible(sVisible);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Callback from GUI to browse and fill another GUI with result, if a relative
// path could be made.
////////////////////////////////////////////////////////////////////////////////
static void BrowseCall(		// Returns nothing.
	RGuiItem*	pgui)			// In:  GUI pointer that was pressed.
	{
	ASSERT(pgui->GetParent() );

	RGuiItem*	pguiName	= pgui->GetParent()->GetItemFromId(pgui->m_ulUserInstance);
	if (pguiName)
		{
		char	szTitle[256];
		sprintf(szTitle, g_pszGenericBrowseFor_s_Title, "sound file");

		// Create full system path from existing RSPiX subpath.
		char	szSystemPath[RSP_MAX_PATH];
		char*	pszSakpath	= g_resmgrSamples.GetBasePath();
		if (pguiName->m_szText[0] == '\0')
			{
			pguiName->SetText(".");
			}

		strcpy(szSystemPath, FullPathCustom(pszSakpath, pguiName->m_szText) );

		short	sResult;
		do {
			sResult	= SubPathOpenBox(			// Returns 0 on success, negative on error, 1 if 
														// not subpathable (i.e., returned path is full path).
				pszSakpath,							// In:  Full path to be relative to (system format). 
				szTitle,								// In:  Title of box.                                
				szSystemPath,						// In:  Default filename (system format).            
				szSystemPath,						// Out: User's choice (system format).               
				sizeof(szSystemPath),			// In:  Amount of memory pointed to by pszChosenFileName.
				"wav");								// In:  If not NULL, '.' delimited extension based filename
														//	filter specification.  Ex: ".cpp.h.exe.lib" or "cpp.h.exe.lib"
														// Note: Cannot use '.' in filter.  Preceding '.' ignored.

			if (sResult > 0)
				{
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					g_pszAppName,
					g_pszGenericMustBeRelativePath_s,
					pszSakpath);
				}

			} while (sResult > 0);

		// If successful in getting a relative path . . .
		if (sResult == 0)
			{
			// Udpate GUI.
			pguiName->SetText("%s", rspPathFromSystem(szSystemPath) );
			pguiName->Compose();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::EditModify(void)
	{
	short	sResult	= 0;

	// Load gui dialog
	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(GUI_FILE_NAME));
	if (pgui != NULL)
		{
		// Init "name" edit
		REdit* pResName = (REdit*)pgui->GetItemFromId(3);
		ASSERT(pResName != NULL);
		ASSERT(pResName->m_type == RGuiItem::Edit);
		pResName->SetText("%s", m_szResName);
		pResName->Compose();

		// Setup name browse button.
		RGuiItem*	pguiNameBrowse	= pgui->GetItemFromId(4);
		ASSERT(pguiNameBrowse);
		// Set callback for browsage.
		pguiNameBrowse->m_bcUser			= BrowseCall;
		// Tell it what ID to fill with result.
		pguiNameBrowse->m_ulUserInstance	= pResName->m_lId;

		// Init "first time" edit
		REdit* pFirstTime = (REdit*)pgui->GetItemFromId(100);
		ASSERT(pFirstTime != NULL);
		ASSERT(pFirstTime->m_type == RGuiItem::Edit);
		pFirstTime->SetText("%ld", m_lMinTime[0]);
		pFirstTime->Compose();

		// Init "first random time" edit
		REdit* pFirstRndTime = (REdit*)pgui->GetItemFromId(101);
		ASSERT(pFirstRndTime != NULL);
		ASSERT(pFirstRndTime->m_type == RGuiItem::Edit);
		pFirstRndTime->SetText("%ld", m_lRndTime[0]);
		pFirstRndTime->Compose();

		// Init "repeat time" edit
		REdit* pRepeatTime = (REdit*)pgui->GetItemFromId(102);
		ASSERT(pRepeatTime != NULL);
		ASSERT(pRepeatTime->m_type == RGuiItem::Edit);
		pRepeatTime->SetText("%ld", m_lMinTime[1]);
		pRepeatTime->Compose();

		// Init "repeat random time" edit
		REdit* pRepeatRndTime = (REdit*)pgui->GetItemFromId(103);
		ASSERT(pRepeatRndTime != NULL);
		ASSERT(pRepeatRndTime->m_type == RGuiItem::Edit);
		pRepeatRndTime->SetText("%ld", m_lRndTime[1]);
		pRepeatRndTime->Compose();

		// Init "enable" push button
		RPushBtn* pEnable = (RPushBtn*)pgui->GetItemFromId(200);
		ASSERT(pEnable != NULL);
		pEnable->m_state = m_bInitiallyEnabled ? RPushBtn::On : RPushBtn::Off;
		pEnable->Compose();

		// Init "repeat" push button
		RPushBtn* pRepeat = (RPushBtn*)pgui->GetItemFromId(201);
		ASSERT(pRepeat != NULL);
		pRepeat->m_state = m_bInitiallyRepeats ? RPushBtn::On : RPushBtn::Off;
		pRepeat->Compose();

		// Init "volume half life" edit.
		REdit* peditHalfLife = (REdit*)pgui->GetItemFromId(301);
		ASSERT(peditHalfLife != NULL);
		ASSERT(peditHalfLife->m_type == RGuiItem::Edit);
		peditHalfLife->SetText("%ld", m_lVolumeHalfLife);
		peditHalfLife->Compose();

		// Init "Purge sample when done" checkbox.
		RMultiBtn*	pmbPurge	= (RMultiBtn*)pgui->GetItemFromId(500);
		ASSERT(pmbPurge);
		ASSERT(pmbPurge->m_type == RGuiItem::MultiBtn);
		pmbPurge->m_sState	= (m_sPurgeSampleWhenDone == FALSE) ? 1 : 2;
		pmbPurge->Compose();

		// Init "Use looping" checkbox.
		RMultiBtn*	pmbUseLooping	= (RMultiBtn*)pgui->GetItemFromId(400);
		ASSERT(pmbUseLooping);
		ASSERT(pmbUseLooping->m_type == RGuiItem::MultiBtn);
		pmbUseLooping->m_sState	= (m_sUseLooping == FALSE) ? 1 : 2;
		pmbUseLooping->Compose();
		// Set the callback so we can tell when state changes.
		pmbUseLooping->m_bcUser	= CheckEnableGuiCall;
		// Set GUI to enable/disable.
		pmbUseLooping->m_ulUserInstance	= 499;
		// Set show state to use when checked.
		pmbUseLooping->m_ulUserData		= TRUE;
		// Setup intially.
		CheckEnableGuiCall(pmbUseLooping);

		SetGuiItemVal(pgui, 401, m_lLoopBackTo);
		SetGuiItemVal(pgui, 402, (m_lLoopBackFrom == 0) ? 1 : m_lLoopBackFrom);
		SetGuiItemVal(pgui, 403, (m_lNumLoopBacks < 0) ? 0 : m_lNumLoopBacks);

		// Init "End" checkbox.
		RMultiBtn*	pmbLoopFromEnd	= (RMultiBtn*)pgui->GetItemFromId(404);
		ASSERT(pmbLoopFromEnd);
		ASSERT(pmbLoopFromEnd->m_type == RGuiItem::MultiBtn);
		pmbLoopFromEnd->m_sState	= (m_lLoopBackFrom == 0) ? 2 : 1;
		pmbLoopFromEnd->Compose();
		// Set the callback so we can tell when state changes.
		pmbLoopFromEnd->m_bcUser	= CheckEnableGuiCall;
		// Set GUI to enable/disable.
		pmbLoopFromEnd->m_ulUserInstance	= 402;
		// Set show state to use when checked.
		pmbLoopFromEnd->m_ulUserData		= FALSE;
		// Setup intially.
		CheckEnableGuiCall(pmbLoopFromEnd);

		// Init "Infinite" checkbox.
		RMultiBtn*	pmbLoopInfinitely	= (RMultiBtn*)pgui->GetItemFromId(405);
		ASSERT(pmbLoopInfinitely);
		ASSERT(pmbLoopInfinitely->m_type == RGuiItem::MultiBtn);
		pmbLoopInfinitely->m_sState	= (m_lNumLoopBacks < 0) ? 2 : 1;
		pmbLoopInfinitely->Compose();
		// Set the callback so we can tell when state changes.
		pmbLoopInfinitely->m_bcUser	= CheckEnableGuiCall;
		// Set GUI to enable/disable.
		pmbLoopInfinitely->m_ulUserInstance	= 403;
		// Set show state to use when checked.
		pmbLoopInfinitely->m_ulUserData		= FALSE;
		// Setup intially.
		CheckEnableGuiCall(pmbLoopInfinitely);

		// Init "Ambient sound" checkbox.
		RMultiBtn*	pmbAmbient	= (RMultiBtn*)pgui->GetItemFromId(600);
		ASSERT(pmbAmbient);
		ASSERT(pmbAmbient->m_type == RGuiItem::MultiBtn);
		pmbAmbient->m_sState	= (m_sAmbient == FALSE) ? 1 : 2;
		pmbAmbient->Compose();

		// Run the dialog using this super-duper helper funciton
		if (DoGui(pgui) == 1)
			{
			// Get new values from dialog
			pResName->GetText(m_szResName, sizeof(m_szResName) );
			m_lMinTime[0] = pFirstTime->GetVal();
			m_lRndTime[0] = pFirstRndTime->GetVal();
			m_lMinTime[1] = pRepeatTime->GetVal();
			m_lRndTime[1] = pRepeatRndTime->GetVal();
			m_bInitiallyEnabled = (pEnable->m_state == RPushBtn::On) ? true : false;
			m_bInitiallyRepeats = (pRepeat->m_state == RPushBtn::On) ? true : false;
			m_lVolumeHalfLife	= peditHalfLife->GetVal();
			m_sPurgeSampleWhenDone		= (pmbPurge->m_sState == 2) ? TRUE : FALSE;
			m_sUseLooping		= (pmbUseLooping->m_sState == 2) ? TRUE : FALSE;
			m_lLoopBackTo		= pgui->GetVal(401);
			m_lLoopBackFrom	= (pmbLoopFromEnd->m_sState == 2) ? 0 : pgui->GetVal(402);
			m_lNumLoopBacks	= (pmbLoopInfinitely->m_sState == 2) ? -1 : pgui->GetVal(403);
			m_sAmbient			= (pmbAmbient->m_sState == 2) ? TRUE : FALSE;
			}
		else
			{
			sResult	= 1;
			}
		
		// Done with GUI.
		delete pgui;
		}
	else
		{
		sResult	= -1;
		}

	// If everything's okay, init using new values
	if (sResult == 0)
		sResult = Init();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::EditMove(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the clickable pos/area of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::EditRect(	// Returns nothiing.
	RRect*	prc)				// Out: Clickable pos/area of object.
	{
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(prc->sX),
		&(prc->sY) );

	prc->sW	= 10;	// Safety.
	prc->sH	= 10;	// Safety.

	if (m_pImage)
		{
		prc->sW	= m_pImage->m_sWidth;
		prc->sH	= m_pImage->m_sHeight;
		}

	prc->sX	-= prc->sW / 2;
	prc->sY	-= prc->sH;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::EditHotSpot(	// Returns nothiing.
	short*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	short*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
	{
	*psX	= 5;	// Safety.
	*psY	= 5;	// Safety.

	if (m_pImage)
		{
		*psX	= m_pImage->m_sWidth / 2;
		*psY	= m_pImage->m_sHeight;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::EditRender(void)
	{
	// Setup simple, non-animating sprite
	m_sprite.m_sInFlags = 0;

	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(m_sprite.m_sX2),
		&(m_sprite.m_sY2) );

	// Priority is based on bottom edge of sprite
	m_sprite.m_sPriority = m_dZ;

	// Center on image.
	m_sprite.m_sX2	-= m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_pImage->m_sHeight;

	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	m_sprite.m_pImage = m_pImage;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}


////////////////////////////////////////////////////////////////////////////////
// Init object
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::Init(void)							// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	Kill();

	m_lLastStartTime = 0;
	m_lNextStartTime = 0;
	m_sWhichTime = -1;
	m_bEnabled = m_bInitiallyEnabled;
	m_bRepeats = m_bInitiallyRepeats;

	// Tell samplemaster to cache (preload) this sample
	CacheSample(m_id);

	if (m_pImage == 0)
		{
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(IMAGE_FILE), &m_pImage);
		if (sResult == 0)
			{
			// This is a questionable action on a resource managed item, but it's
			// okay if EVERYONE wants it to be an FSPR8.
			if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
				{
				sResult = -1;
				TRACE("CSoundThing::GetResource() - Couldn't convert to FSPR8\n");
				}
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Kill object
////////////////////////////////////////////////////////////////////////////////
short CSoundThing::Kill(void)							// Returns 0 if successfull, non-zero otherwise
	{
	if (m_pImage != 0)
		rspReleaseResource(&g_resmgrGame, &m_pImage);

	m_pRealm->m_scene.RemoveSprite(&m_sprite);

	// If we have a play instance identifier . . .
	if (m_siChannel != 0)
		{
		// Abort it.
		AbortSample(m_siChannel);
		// Done with ID.
		m_siChannel	= 0;
		}

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Process our message queue.
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::ProcessMessages(void)
	{
	// Check queue of messages.
	GameMessage	msg;
	while (m_MessageQueue.DeQ(&msg) == true)
		{
		switch(msg.msg_Generic.eType)
			{
			case typeObjectDelete:
				m_state	= State_Delete;
				break;
			}
		
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Don't call this from outside of CSoundThing.  It should affect only
// CSoundThing stuff.
////////////////////////////////////////////////////////////////////////////////
long CSoundThing::GetRandom(void)
	{
	return (((ms_lGetRandomSeed = ms_lGetRandomSeed * 214013L + 2531011L) >> 16) & 0x7fff);
	}

////////////////////////////////////////////////////////////////////////////////
// Relay the volume to add to this CSoundThing's collective volume.
////////////////////////////////////////////////////////////////////////////////
void CSoundThing::RelayVolume(	// Returns nothing.
	long lVolume)						// In:  Volume to relay.
	{
	m_lCollectiveVolume	+= lVolume;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
