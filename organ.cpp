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
// Organ.cpp
// 
// History:
//		08/05/97 JRD	Started.  Linked in with SampleMaster.h
//
//		08/05/97	JMI	Now calls PurgeSamples() when done playing with the organ.
//							Converted to rspGetKeyStatusArray().
//
//		08/17/97	JMI	The second time you entered the organ, it would exit 
//							right away.
//							Also, now, when you choose 'Back', it goes back.  It
//							used to merely check for the escape key in order to 
//							exit.  Now it actually pumps the menu input so any normal
//							means of exiting a menu will work for this menu as well.
//							This provides a little better user feedback.
//							Also, took out aborting of sounds on exit.  Seemed to
//							abrupt or something.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem().
//
//////////////////////////////////////////////////////////////////////////////
//
// Allows a player to play with his organ
//
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/ResourceManager/resmgr.h"
#else
	#include "resmgr.h"
#endif
#include "SampleMaster.h"
#include "game.h"
#include "update.h"
#include "organ.h"

#define OR_BANK_KEY		RSP_SK_TAB

// Set by callback or within PlayWithMyOrgan() to quit.
static short	ms_sContinue;

void	PlayWithMyOrgan()
	{
	// only allow certain keys to be active:
	char	acValidKeys[] = "1234567890QWERTYUIOPASDFGHJKLZXCVBNM";
	short	sNumValid = strlen(acValidKeys);
	short sNumSounds = CSoundCatalogue::NumSounds();
	short sNumBanks = (sNumSounds + sNumValid - 1) / sNumValid;

	short sCurBank = 0;
	ms_sContinue = 1;

	U8*	pau8KeyStatus	= rspGetKeyStatusArray();
	RInputEvent	ie;

	// Clear status array.
	memset(pau8KeyStatus, 0, 128);

	// Let's play some noise!
	while (ms_sContinue)
		{
		short i;

		UpdateSystem();

		for (i=0;i<sNumValid;i++)
			{
			if ( pau8KeyStatus[acValidKeys[i]] )
				{
				// he just pressed this key - play the note:
				// Play sample CSoundCatalogue::ms_ppcNameList[
				// i + sNumValid * sCurBank if this isn't bigger
				// than sNumSounds
				short sCurSample = (sNumValid * sCurBank + i) % sNumSounds;
				if (sCurSample < sNumSounds)
					{
					// Play the sample
					PlaySample(
						*CSoundCatalogue::ms_ppsmNameList[sCurSample],
						SampleMaster::Unspecified,
						255);	// initial volume
					}

				// Clear key.
				pau8KeyStatus[acValidKeys[i]]	= 0;
				}
			}

		if (pau8KeyStatus[OR_BANK_KEY]) // switch banks:
			{
			sCurBank++;
			if (sCurBank > sNumBanks)
				{
				sCurBank = 0;
				}

			// Clear key.
			pau8KeyStatus[OR_BANK_KEY]	= 0;
			}

		
		// Get the next input event, if any.
		ie.type	= RInputEvent::None;
		rspGetNextInputEvent(&ie);

		// Process menu input (cancel or choice keys).
		DoMenuInput(&ie, 1);
		// Do menu output (probably none most of the time in this case).
		// ....maybe not necessary since there's
//		DoMenuOutput(g_pimScreenBuf);

		// Draw newest state of menu....maybe not necessary since there's
		// only one option.
//		rspUpdateDisplay();

		if (rspGetQuitStatus() )
			{
			ms_sContinue = 0;
			}
		}

	// End playing samples.
//	AbortAllSamples();

	// Loop until done (if not they won't purge).
//	while (IsSamplePlaying() == true)
		{
//		Update();
		}

	// Purge all samples so we increase memory overhead.
	PurgeSamples();

	rspClearAllInputEvents();
	}

//////////////////////////////////////////////////////////////////////////////
// Choice callback from menu.
//////////////////////////////////////////////////////////////////////////////
extern bool Organ_MenuChoice(	// Returns true to accept choice, false to deny.
	Menu*	/*pmenuCurrent*/,		// Current menu.
	short	sMenuItem)				// Item chosen.
	{
	if (sMenuItem >= 0)
		{
		ms_sContinue	= FALSE;
		}

	// Audible Feedback.
	if (sMenuItem == -1)
		PlaySample(g_smidMenuItemChange, SampleMaster::UserFeedBack);
	else
		PlaySample(g_smidMenuItemSelect, SampleMaster::UserFeedBack);

	// I'm sure that choice'll be fine.
	return true;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
