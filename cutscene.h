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
// cutscene.h
// Project: Nostril (aka Postal)
//
// History:
//		04/18/97 MJR	Started.
//
//		04/22/97 MJR	Testing and tuning.
//
//		04/26/97 JRD	Added CSwirlMe
//
//		06/08/97 MJR	Changed interface in order to properly clean everything
//							up without memory leaks.
//
//		06/11/97	JMI	Changed so CutSceneStart() does not require a prefs file
//							and section name but instead opens and creates them using
//							a single realm number that is now passed in.
//
//					MJR	Added support for sepearate network sections in realms
//							prefs file.
//
//		07/14/97 BRH	Added challenge mode parameter to CutSceneStart so that
//							the proper text can be displayed for the Gauntlet.
//
//		08/27/97	JRD	Adding a compact stand along Martini effect for us with the 
//							end of the game called MartiniDo
//
//		08/27/97	JRD	Made MartiniDo WAY to easy for Bill to use, by putting up 
//							with him justpassing the whole screen buffer, me doing a 
//							general lock on it, creating a temporary bmp, and copying
//							it in.  Happy Bill?
//
//		08/28/97 JRD	Refined martini effect and added fade out option.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CUTSCENE_H
#define CUTSCENE_H

#include "RSPiX.h"
#include "WishPiX/Prefs/prefs.h"
#include "SampleMaster.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//		MartiniDo - this is a greatly simplified version of the cutscene Martini
//
//		It allows the user a few parameters and then does the effect for a set
//		amount of time, regardless of user input.  There are no text overlays.
//		The program will blacken the screen when done.  (g_pimScreenBuf)
//
////////////////////////////////////////////////////////////////////////////////
short	MartiniDo(	RImage*	pimBackground,	// actually, this is the ONLY graphic
						short	sStartX,				// logical start position of image
						short	sStartY,				// NOTE: it will be clipped so won't actually hit this point!
						RMultiAlpha*	pAlpha,	// only need 50% - see cut scenes
						long	lMilliLen,			// how long to do the effect
						short	sRadius = 24,		// Your tuning pleasure
						long	lSpinTime = 3600,	// in milliseconds
						long	lSwayTime = 4000,	// in milliseconds
						RRect*  prCenter = NULL,// if not NULL, use this portion of the image only!
						long	lFadeTime = 0,		// fade to black, in milliseconds. (INCL in total time!)
						SampleMaster::SoundInstance siFade=0// to make sound fade out
					);

////////////////////////////////////////////////////////////////////////////////
//
// Start cutscene.
//
// If 'simple' mode is true, a simple background is displayed with the name
// of the realm file printed on it.
//
// Otherwise, the variables in the specified section of the specified prefs file
// determine what text is printed on top of the background.
//
// If a special effect is desired, CutSceneConfig() must be called, followed
// by repeated calls to CutSceneUpdate().
//
// CutSceneEnd() must be called when the cutscene is no longer needed.
//
////////////////////////////////////////////////////////////////////////////////
extern void CutSceneStart(
	bool bSimple,											// In:  Set to 'true' for simple mode
	const RString* pstrSection,						// In:  Section to use for this realm
	const RString* pstrEntry,							// In:  Entry to use for this realm
	short sBorderX,
	short sBorderY);


////////////////////////////////////////////////////////////////////////////
//
// Configure cutscene effect
//
////////////////////////////////////////////////////////////////////////////
extern short CutSceneConfig(
	long lTimeSpin,
	short sMinX,short sMaxX,long lTimeX,
	short sMinY,short sMaxY,long lTimeY,
	double dMinA,double dMaxA,long lTimeA,
	short sX,short sY,short sW,short sH);


////////////////////////////////////////////////////////////////////////////
//
// Update cutscene effect
//
////////////////////////////////////////////////////////////////////////////
extern void CutSceneUpdate(void);


////////////////////////////////////////////////////////////////////////////////
//
// Clean up after the cutscene.  It is safe to call this multiple times.
//
////////////////////////////////////////////////////////////////////////////////
extern void CutSceneEnd(void);


#endif //CUTSCENE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
