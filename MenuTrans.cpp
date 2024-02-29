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
// MenuTrans.cpp
// Project: Nostril (aka Postal)
//
// This module sets up the system and RSPiX.
//
// History:
//		11/19/96 MJR	Started.
//
//		08/04/97	JMI	Although the Remap() function was calling 
//							rspLockVideoBuffer(), it was effectively ignoring it.  That
//							is, it passed 'dummy' vars which it did not use to access
//							the screen.  These values MUST be used or there is very
//							little point in locking the video buffer in the first place.
//							This probably occurs all over the place.  I'm pretty sure
//							I've done it this way in spots.
//
//		08/21/97	JMI	Changed occurrences of rspUpdateDisplay() to 
//							UpdateDisplay().
//
//		08/22/97	JMI	Changed to use rspLockBuffer() (the BLiT library version
//							of locking the composite buffer).
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//							Also, put locks around accesses (Blits, Rects, etc.) to the
//							composite buffer.
//
////////////////////////////////////////////////////////////////////////////////
//
// Things to change:
//
// I don't currently care much for the fade to red -- it looks pretty damn
// plain.
//
// This currently requires a huge amount of memory since it needs to save the
// screen data so that the effect can be run backwards.  The problem with this
// is that it means we're always wasting alot more memory while running the
// game than we would otherwise need to, because we have to be ready to pop up
// the menu (and hence to use this effect) at any time.
//
// One possible way to use less memory is to save the screen data to the
// hard drive and load it back when we need it.  I'm not sure how slow this
// will be, and I'm also not sure what's involved in actually doing this since
// the buffer is really just a stub, which may not support saving properly.
//
// Not sure what to do about EndMenuTrans().  Original idea was to be able to
// call it at any time, and you could either tell it finish the effect as
// quickly as possible or to abort, which might leave the screen in an ugly
// state.  The idea was that if the user hit a key or something that meant
// "skip this stupid effect", that we could quickly do so.  I'm not sure how
// actual usage will turn out, so for now, I'm ignoring the flag being passed
// to this function.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
#else
#endif

#include "game.h"
#include "MenuTrans.h"
#include "update.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Define number of shades along with mask that will create that many shades
//#define NUM_SHADES				32
//#define SHADE_MASK				0xf8
#define NUM_SHADES				16
#define SHADE_MASK				0xf0

// Define range of palette indices to be affected (inclusive).
// THIS RANGE MUST BE AT LEAST TWICE AS LARGE AS NUM_SHADES!!!
#define EFFECT_BEG			10
#define EFFECT_END			245
#define EFFECT_LEN			((EFFECT_END - EFFECT_BEG) + 1)

#define SHADE_BEG				((EFFECT_END + 1) - NUM_SHADES)
#define SHADE_END				(EFFECT_END)
#define SHADE_LEN				((SHADE_END - SHADE_BEG) + 1)

#define NONSHADE_BEG			EFFECT_BEG
#define NONSHADE_END			(SHADE_BEG - 1)
#define NONSHADE_LEN			((NONSHADE_END - NONSHADE_BEG) + 1)

// Simple struct for working with palettes
typedef struct
	{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char x;
	} rgb;


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

static rgb* m_pOrig;
static rgb* m_pWork;
static rgb* m_pSaveStep4;
static unsigned char* m_pUnmapStep4;
static unsigned char* m_pUnmapStep5;
static RImage* m_pim;

static short m_sStep;
static bool m_bFinishASAP;
static long m_lTotalTime;
static long m_lBaseTime;

static double m_dReduce = 1.0;


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

static void Remap(
	unsigned char* aucMap);


////////////////////////////////////////////////////////////////////////////////
// Call this to start the menu transition effect
////////////////////////////////////////////////////////////////////////////////
extern void StartMenuTrans(
	long lTotalTime)										// In:  Effect time (in ms) must be >= 0
	{
	// Default to step 0 (nothing) in case something goes wrong
	m_sStep = 0;

	// Save total time
	if (lTotalTime < 0)
		{
		TRACE("StartMenuTransIn(): Moronic time specified: %ld -- changed to 0!\n", lTotalTime);
		lTotalTime = 0;
		}
	m_lTotalTime = lTotalTime;

	// Clear finish flag
	m_bFinishASAP = false;

	// Allocate lots of stuff
	m_pOrig = new rgb[256];
	m_pWork = new rgb[256];
	m_pSaveStep4 = new rgb[256];
	m_pUnmapStep4 = new unsigned char[256];
	m_pUnmapStep5 = new unsigned char[256];
	m_pim = new RImage;
	if (m_pOrig && m_pWork && m_pSaveStep4 && m_pUnmapStep4 && m_pUnmapStep5 && m_pim)
		{

		// Setup image to match screen buffer
		if (m_pim->CreateImage(g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight, RImage::BMP8) == 0)
			{

			// Everything's cool, so set for first step
			m_sStep = 1;

			}
		else
			TRACE("StartMenuTrans(): Error returned by RImage::CreateImage()!\n");
		}
	else
		TRACE("StartMenuTrans(): Couldn't allocate memory!\n");
	}


////////////////////////////////////////////////////////////////////////////////
//
// The starting time of the effect is considered to be the first time this
// function is called (NOT when InitMenuTransIn() is called!)
//
// The return value is true when the effect has completed, false otherwise.
////////////////////////////////////////////////////////////////////////////////
extern bool DoPreMenuTrans(void)
	{
	//---------------------------------------------------------------------------
	// Step 1: Start
	//
	// Get the current palette and the current time and go on to the next step.
	// This could have been done in the init function, but I didn't want the
	// timing to start then, but rather on the first call to this function.
	//---------------------------------------------------------------------------
	if (m_sStep == 1)
		{
		// Get the "original" palette
		rspGetPaletteEntries(0, 256, &(m_pOrig[0].r), &(m_pOrig[0].g), &(m_pOrig[0].b), sizeof(rgb));

		// Lock the buffer before reading from it.
		rspLockBuffer();

		// Copy the screen into our image
		rspBlitA(g_pimScreenBuf, m_pim, 0, 0, m_pim->m_sWidth, m_pim->m_sHeight);

		// Unlock now that we're done with the composite buffer.
		rspUnlockBuffer();

		// Calculate goal for each color's red component.  We use the otherwise
		// unused member of the struct to store the goal.
		for (short i = EFFECT_BEG; i <= EFFECT_END; i++)
//			m_pOrig[i].x = (unsigned char)((double)(255 - m_pOrig[i].r) * m_dReduce) & SHADE_MASK;
			m_pOrig[i].x = (unsigned char)((double)(m_pOrig[i].r) * m_dReduce) & SHADE_MASK;

		// Get base time for next step
		m_lBaseTime = rspGetMilliseconds();
		
		// Go to next step
		m_sStep = 2;
		}
	//---------------------------------------------------------------------------
	// Step 2: Fade colors
	//
	// Over the specified period of time, we transform the original palette
	// entries into MAX_SHADES shades of red.
	//---------------------------------------------------------------------------
	else if (m_sStep == 2)
		{
		// Calculate how far into the effect we are based on the elapsed time
		// since we started.  If the total time specified by the user was 0, or
		// if we're being asked to finish the effect ASAP, we go right to 100%.
		double dPercent;
		if ((m_lTotalTime == 0) || m_bFinishASAP)
			{
			dPercent = 1.0;
			}
		else
			{
			dPercent = (double)(rspGetMilliseconds() - m_lBaseTime) / (double)m_lTotalTime;
			if (dPercent > 1.0)
				dPercent = 1.0;
			}

		// Update all colors to where they should be based on given percentage
		for (short i = EFFECT_BEG; i <= EFFECT_END; i++)
			{
			double dRedDiff = m_pOrig[i].r - m_pOrig[i].x;
			m_pWork[i].r = m_pOrig[i].x + (unsigned char)(dRedDiff * (1.0 - dPercent));
			m_pWork[i].g = (unsigned char)((double)m_pOrig[i].g * (1.0 - dPercent));
			m_pWork[i].b = (unsigned char)((double)m_pOrig[i].b * (1.0 - dPercent));
			}

		// Set new palette
		rspSetPaletteEntries(EFFECT_BEG, EFFECT_LEN, &(m_pWork[EFFECT_BEG].r), &(m_pWork[EFFECT_BEG].g), &(m_pWork[EFFECT_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// If we're done with the fade, go to next step
		if (dPercent == 1.0)
			m_sStep = 3;
		}
	//---------------------------------------------------------------------------
	// Step 3: Consolidate
	//
	// At this point, the palette entries are all set to any of several shades of
	// red.  Since the range of entries we're dealing with is at least twice the
	// maximum number of shades of red, there will definitely be some duplicate
	// entries.  We remap the pixels to get rid of the duplicates, at the same
	// time creating some unused entries.  We set flags showing which entries are
	// used and which are free in preperation for the next step.
	//---------------------------------------------------------------------------
	else if (m_sStep == 3)
		{
		// Start mapping table out as an "identity map" (pixels map to themselves)
		unsigned char aucMap[256];
		for (short m = 0; m < 256; m++)
			aucMap[m] = m;

		// Scan through the palette mapping each entry onto the first entry with the
		// same color.  Only checks red since blue and green are always 0.
		for (short i = EFFECT_BEG; i <= EFFECT_END; i++)
			{
			// Loop ends on first match - worst case is that entry matches itself
			short j;
			for (j = EFFECT_BEG; m_pWork[i].r != m_pWork[j].r; j++) ;
			aucMap[i] = j;

			// Set flag to 0 if this entry will be free after remapping, 1 otherwise.
			// The next step relies on this!
			m_pWork[i].x = (j < i) ? 0 : 1;
			}

		// Remap screen pixels
		Remap(aucMap);
		rspUpdateDisplay();

		// Go to next step
		m_sStep = 4;
		}
	//---------------------------------------------------------------------------
	// Step 4: Clear Official Area
	//
	// The image is now only using a small number of entries, and there are at
	// least that many unused entries.  Unfortunately, the used and unused
	// entries are scattered around in no particular order, so some of the used
	// entries may be where we eventually want the "official" set of shades to
	// reside.  We remap the pixels to move any such entries into unused entries
	// outside of that "official" area.
	//---------------------------------------------------------------------------
	else if (m_sStep == 4)
		{
		// Start mapping table out as an "identity map" (pixels map to themselves)
		unsigned char aucMap[256];
		for (short m = 0; m < 256; m++)
			{
			aucMap[m] = m;
			m_pUnmapStep4[m] = m;
			}

		// Go through area reserved for "official" shades looking for used entries
		// and, if found, remap them to unused entries outside of that range.
		for (short s = SHADE_BEG; s <= SHADE_END; s++)
			{
			if (m_pWork[s].x)
				{
				short i;
				for (i = NONSHADE_BEG; i <= NONSHADE_END; i++)
					{
					if (m_pWork[i].x == 0)
						{
						aucMap[s] = i;
						m_pUnmapStep4[i] = s;
						m_pWork[i].r = m_pWork[s].r;
						m_pWork[i].x = 1;
						m_pWork[s].x = 0;	// not required but makes table easier to "read" in debugger
						break;
						}
					}
				// Make sure we found an unused entry to map onto
				ASSERT(i <= NONSHADE_END);
				}
			}

		// Set new palette (some entries may have been changed by the above remapping)
		rspSetPaletteEntries(NONSHADE_BEG, NONSHADE_LEN, &(m_pWork[NONSHADE_BEG].r), &(m_pWork[NONSHADE_BEG].g), &(m_pWork[NONSHADE_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// Remap screen pixels
		Remap(aucMap);
		rspUpdateDisplay();

		// Save current palette so we can run the effect backwards
		for (short p = 0; p < 256; p++)
			m_pSaveStep4[p] = m_pWork[p];

		// Go to next step
		m_sStep = 5;
		}
	//---------------------------------------------------------------------------
	// Step 5: Pack Into Official Area
	//
	// We now put the full set of shades at the specified section of the palette
	// and remap the image so that only those "official" entries are used.
	//---------------------------------------------------------------------------
	else if (m_sStep == 5)
		{
		// Put full set of shades at proper position in palette
		for (short s = 0; s < SHADE_LEN; s++)
			m_pWork[SHADE_BEG + s].r = s * (~SHADE_MASK + 1);
		
		// Set new palette (do this before remapping so the shades will be there before they're needed)
		rspSetPaletteEntries(SHADE_BEG, SHADE_LEN, &(m_pWork[SHADE_BEG].r), &(m_pWork[SHADE_BEG].g), &(m_pWork[SHADE_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// Start mapping table out as an "identity map" (pixels map to themselves)
		unsigned char aucMap[256];
		for (short m = 0; m < 256; m++)
			{
			aucMap[m] = m;
			m_pUnmapStep5[m] = m;
			}

		// Scan through the palette mapping each used entry onto the first
		// "official" shade entry with the same color.
		for (short i = NONSHADE_BEG; i <= NONSHADE_END; i++)
			{
			// Only do this for used entries (the mapping would work without
			// this check, since it wouldn't hurt to map unused entries, but
			// the unmapping table would be screwed up -- it's really bizarre
			// to think about, but eventually it makes sense)
			if (m_pWork[i].x)
				{
				// Loop ends on first match (and there always will be a match)
				short j;
				for (j = SHADE_BEG; m_pWork[i].r != m_pWork[j].r; j++) ;
				aucMap[i] = j;
				m_pUnmapStep5[j] = i;
				}
			}

		// Remap screen pixels
		Remap(aucMap);
		rspUpdateDisplay();

		// Go to next step
		m_sStep = 6;
		}

	// Condition return value based on whether or not we're done
	if (m_sStep == 6)
		return true;
	return false;
	}


////////////////////////////////////////////////////////////////////////////////
//
// The starting time of the effect is considered to be the first time this
// function is called (NOT when InitMenuTransIn() is called!)
//
// The return value is true when the effect has completed, false otherwise.
////////////////////////////////////////////////////////////////////////////////
extern bool DoPostMenuTrans(void)
	{
	//---------------------------------------------------------------------------
	// Step 6: Undo Step 5
	//---------------------------------------------------------------------------
	if (m_sStep == 6)
		{
		// Restore palette to where it was prior to step 5
		for (short p = 0; p < 256; p++)
			m_pWork[p] = m_pSaveStep4[p];

		// Set only the non-shade portion of the palette until we remap the pixels
		rspSetPaletteEntries(NONSHADE_BEG, NONSHADE_LEN, &(m_pWork[NONSHADE_BEG].r), &(m_pWork[NONSHADE_BEG].g), &(m_pWork[NONSHADE_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// Remap screen pixels
		Remap(m_pUnmapStep5);
		rspUpdateDisplay();

		// Go to next step
		m_sStep = 7;
		}
	//---------------------------------------------------------------------------
	// Step 7: Undo Step 4
	//---------------------------------------------------------------------------
	else if (m_sStep == 7)
		{
		// Set the remainder of the palette now that the pixels are no longer using that part
		rspSetPaletteEntries(SHADE_BEG, SHADE_LEN, &(m_pWork[SHADE_BEG].r), &(m_pWork[SHADE_BEG].g), &(m_pWork[SHADE_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// Remap screen pixels
		Remap(m_pUnmapStep4);
		rspUpdateDisplay();

		// Go to next step
		m_sStep = 8;
		}
	//---------------------------------------------------------------------------
	// Step 8: 
	//---------------------------------------------------------------------------
	else if (m_sStep == 8)
		{
		// Use the original palette to figure out what the colors should be like
		// as we start the reverse fade effect.  If everything went right, we should
		// come up with the same values that already exist for those entries that
		// are actually being used by the pixels, and for those entries that aren't
		// being used, we're getting them ready for when we go back to the original
		// image.
		for (short i = EFFECT_BEG; i <= EFFECT_END; i++)
			{
			m_pWork[i].r = m_pOrig[i].x;
			m_pWork[i].g = (unsigned char)0;
			m_pWork[i].b = (unsigned char)0;
			}
		rspSetPaletteEntries(EFFECT_BEG, EFFECT_LEN, &(m_pWork[EFFECT_BEG].r), &(m_pWork[EFFECT_BEG].g), &(m_pWork[EFFECT_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// Lock the buffer before writing to it.
		rspLockBuffer();

		// Restore the original image
		rspBlitA(m_pim, g_pimScreenBuf, 0, 0, m_pim->m_sWidth, m_pim->m_sHeight);

		// Unlock now that we're done with the composite buffer.
		rspUnlockBuffer();

		rspUpdateDisplay();

		// Get base time for next step
		m_lBaseTime = rspGetMilliseconds();
		
		// Go to next step
		m_sStep = 9;
		}
	//---------------------------------------------------------------------------
	// Step 9: Do a reverse-fade of the colors
	//---------------------------------------------------------------------------
	else if (m_sStep == 9)
		{
		// Calculate how far into the effect we are based on the elapsed time
		// since we started.  If the total time specified by the user was 0, or
		// if we're being asked to finish the effect ASAP, we go right to 100%.
		double dPercent;
		if ((m_lTotalTime == 0) || m_bFinishASAP)
			{
			dPercent = 1.0;
			}
		else
			{
			dPercent = (double)(rspGetMilliseconds() - m_lBaseTime) / (double)m_lTotalTime;
			if (dPercent > 1.0)
				dPercent = 1.0;
			}

		// Update all colors to where they should be based on given percentage
		for (short i = EFFECT_BEG; i <= EFFECT_END; i++)
			{
			double dRedDiff = m_pOrig[i].r - m_pOrig[i].x;
			m_pWork[i].r = m_pOrig[i].x + (unsigned char)(dRedDiff * dPercent);
			m_pWork[i].g = (unsigned char)((double)m_pOrig[i].g * dPercent);
			m_pWork[i].b = (unsigned char)((double)m_pOrig[i].b * dPercent);
			}

		// Set new palette
		rspSetPaletteEntries(EFFECT_BEG, EFFECT_LEN, &(m_pWork[EFFECT_BEG].r), &(m_pWork[EFFECT_BEG].g), &(m_pWork[EFFECT_BEG].b), sizeof(rgb));
		rspUpdatePalette();

		// If we're done with the fade, go to next step
		if (dPercent == 1.0)
			m_sStep = 10;
		}

	// Condition return value based on whether or not we're done
	if (m_sStep == 10)
		return true;
	return false;
	}


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
extern void EndMenuTrans(
	bool bFinish)											// In: true to finish effect, false to abort it
	{
	// If step isn't over 0 then there's nothing to do
	if (m_sStep > 0)
		{
		// Check if caller wants to finish effect
		if (bFinish)
			{
			// Finish the effect
			m_bFinishASAP = bFinish;
//			while (!DoMenuTrans())
//				;
			}

		// Free lots of stuff
		delete []m_pOrig;
		delete []m_pWork;
		delete []m_pSaveStep4;
		delete []m_pUnmapStep4;
		delete []m_pUnmapStep5;
		delete m_pim;

		// Reset step to "nothing"
		m_sStep = 0;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Remap the pixels using the specified map
////////////////////////////////////////////////////////////////////////////////
static void Remap(
	unsigned char* aucMap)
	{
	// Jon brought up a potential problem with calling rspLockBuffer(), which
	// is BLiT's version of this.  In debug mode, it apparently doesn't do
	// anything because it assumes you will be calling a BLiT function, which
	// would actually do the locking.  Since we don't call a BLiT function
	// here, the end result would be no locking.  This is only a problem in
	// debug mode.  We'll have to check into a better solution, but for now
	// I'm just calling the "real" buffer lock.
	U8* pu8VideoBuf;
	long	lPitch;
// Note that we only need to do this in the case that the buffer is not already
// locked.  Since we keep it locked while the game is running now, we don't need
// it (note also regarding the lock comment above that currently rspLockBuffer() 
// does the lock even in DEBUG mode)
// IF you comment this back in, remember to comment in the unlock as well!
#if 0
	if (rspLockVideoBuffer((void**)&pu8VideoBuf, &lPitch) ) == 0)
		{
#else
	if (rspLockBuffer() == 0)
		{
		pu8VideoBuf	= g_pimScreenBuf->m_pData;
		lPitch		= g_pimScreenBuf->m_lPitch;
#endif

		short sHeight = g_pimScreenBuf->m_sHeight;
		short sWidth = g_pimScreenBuf->m_sWidth;
		short sWidth2;
		long lNextRow = lPitch - (long)sWidth;
		unsigned char* pBuf = pu8VideoBuf;
		if ((sHeight > 0) && (sWidth > 0))
			{
			do {
				sWidth2 = sWidth;
				do	{
					*pBuf = *(aucMap + (long)*pBuf);	// may be faster than aucMap[*pBuf]
					pBuf++;
					} while (--sWidth2);
				pBuf += lNextRow;
				} while (--sHeight);
			}

#if 0
		rspUnlockVideoBuffer();
#else
		rspUnlockBuffer();
#endif
		}
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
