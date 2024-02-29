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
// title.h
// Project: Postal
//
// History:
//		12/04/96 MJR	Started.
//
//		04/14/97	JMI	You can select the starting bitmap on the call to 
//							StartTitle().
//
//		06/04/97	JMI	Now you can select a title page relative to the last one
//							by specifying a number < 1.
//
//		07/23/97 BRH	Added a function to return the number of titles in use.
//
//		08/08/97	JMI	Added parameter specifying whether to play musak to 
//							StartTitle().
//
//		08/18/97 BRH	Added the end of game sequence function.
//
//		08/23/97	JMI	Now you can get the sound instance of the title musak from
//							StartTitle().
//
//		10/21/97	JMI	A call to Title_DisableRipcordStaticLogo() will now disable
//							the RipCord static logo.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TITLE_H
#define TITLE_H

#include "SampleMaster.h"

// Start the title sequence, which includes some type of progress meter, which
// may or may not (hopefully not) look like a standard progress meter.  The
// total range of the progress bar is determined by lTotalUnits.  As far as
// this module is concerned, these units are completely abstract.
extern short StartTitle(						// Returns 0 if successfull, non-zero otherwise
	short	sStartImage = 1,						// In:  Image to start with.  Values less
														// than 1 indicate a page relative to the
														// end.
	bool	bPlayMusak = false,					// In:  true to play title musak.
	SampleMaster::SoundInstance* psi = 0);	// Out:  Sound instance of musak.
														

// Update the title sequence.  The specified number of units are added to a
// running total.  The ration between the running total and the value passed to
// StartTitle() determines the new position of the progress meter.
extern short DoTitle(						// Returns 0 if successfull, non-zero otherwise
	long lUnits);								// In:  Additional progess units

// When you are completely done, call EndTitle().  This gives the title sequence
// a chance to fully complete the progess meter (in case it never reached 100%
// due to an overestimated lTotalUnits) and allows all resources to be freed.
extern short EndTitle(void);				// Returns 0 if successfull, non-zero otherwise

// Return number of title screens in use
extern short TitleGetNumTitles(void);

// Show end of game sequence when the player wins
extern void Title_GameEndSequence(void);

// Disable RipCord static logo.
extern void Title_DisableRipcordStaticLogo(void);

#endif //TITLE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
