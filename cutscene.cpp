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
// cutscene.cpp
// Project: Postal
//
// This module handles all the cut-scenes.
//
// History:
//		04/18/97 MJR	Started.
//
//		04/20/07 BRH	Problem compiling this file due to RPrint print being
//							defined more than once.  It may have just been cut and
//							pasted code.
//
//		04/21/97 MJR	Created real version of CutScene().
//
//		04/22/97 MJR	Testing and tuning.
//
//		04/26/97 JRD	Added a swirling blur effect to the cutscenes
//
//		04/27/97 JRD	Merged CSwirl::Configure with CutScene to allow greater flxibility
//
//		06/08/97 MJR	Changed interface in order to properly clean everything
//							up without memory leaks.
//
//		06/11/97	JMI	Changed so CutSceneStart() does not require a prefs file
//							and section name but instead opens and creates them using
//							a single realm number that is now passed in.
//
//		06/12/97 MJR	Renamed m_pszRealm to m_pszRealmPrefsFile.
//
//		06/14/97 MJR	Fixed problem regarding default multialpha name and what
//							happens if none gets loaded (i.e. - it crashed).
//
//					MJR	Added support for sepearate network sections in realms
//							prefs file.
//
//		06/17/97	JMI	Added NULL in call to PlaySample() corresponding to new
//							param.
//
//		07/05/97 MJR	Changed to RSP_BLACK_INDEX instead of 0.
//
//		07/14/97 BRH	Added challenge mode parameter to CutSceneStart so that
//							the proper text can be displayed for the Gauntlet.
//
//		07/17/97	JMI	Changed RSnd*'s to SampleMaster::SoundInstances.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97 MJR	Got progress bars working in a ROUGH sort of way.  They
//							still need some tuning to look better and we need to save
//							and load the "total bytes" (aka cumm units) for each
//							realm to the prefs file.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/20/97	JMI	Added NUM_ELEMENTS() macro as alternative to 
//							(sizeof(a)/sizeof(a[0]) ).
//							Changed asWavyY to be of type short (was no specific type
//							and, therefore, int before).
//							Fix parenthesis bug in CutScene_RFileCallback() with index
//							that caused it to hit -1.
//
//		08/18/97 BRH	Changed default cutscene from cut00 to abstract.
//
//		08/18/97	JMI	Now works with the four different combinations of missing 
//							assets I could conceive.
//
//		08/20/97	JMI	Now uses source clipping when copying the progress bar
//							area out of m_pimBGLayer for safety (normally they should
//							be the correct size).
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97 BRH	Added ability to load a different sound file for each
//							cutscene, specified in the realms.ini file.
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//							Also, now set the PlaySample() call to purge the sample
//							when done.
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
//		08/30/97 BRH	Fixed path for loading the realms.ini file.  It now loads
//							from the CD path rather thant the HD path.
//
//		09/08/97 BRH	I just learned about rspGetQuitStatus() and found it to
//							be so much fun, that I just started adding it to every
//							function I could find, including MartiniDo()
//
//		09/11/97	JMI	Added some protection for when the cutscene image is not
//							available.  Only tested for missing cutscenes with CompUSA
//							version though which doesn't let you get to the martini
//							effect so that may still not work without a BMP (although,
//							I did put in the appropriate checking, it may still have
//							side effects of using different numbers than the real 
//							cutscene would (like say a width of 0) ).
//
//		09/24/97	JMI	Now initializes bFemalePain member of m_musicID.  This 
//							member indicates whether the sample is of a female in pain 
//							which some countries (so far just UK) don't want in the 
//							game.
//
//		10/07/97	JMI	Changed bFemalePain to usDescFlags.
//
//		06/03/98 BRH	Had to change the path for the realms.ini file back to
//							loading from the HD for the add-on pack since the
//							new cutscenes are now only stored on the HD while the
//							original PostalCD is in their CD drive (it only has
//							the old cutscene images).
//
//		02/04/00 MJR	Changed so that it now checks the HD first and the VD
//							second when trying to load bg's and multialphas.  This
//							was done to fix problems with the Japan Add On.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "cutscene.h"
#include "game.h"
#include "update.h"
#include "SampleMaster.h"
#include "AlphaAnimType.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define FONT_FILE					"res/fonts/smash.fnt"

#define DEFAULT_TITLE			"Loading..."
#define DEFAULT_BG				"res/cutscene/abstract.bmp"
#define DEFAULT_TEXT				""
#define DEFAULT_MULTIALPHA		"res/cutscene/abstract.mlp"
#define DEFAULT_MUSIC			"psychedelic1.wav"

#define BG_X						(ms_pCut->m_pimDst->m_sWidth / 2  - ms_pCut->m_pimBGLayer->m_sWidth / 2)
#define BG_Y						(ms_pCut->m_pimDst->m_sHeight / 2 - ms_pCut->m_pimBGLayer->m_sHeight / 2)

#define NAME_Y						85
#define TEXT_Y						(280 - 48) 

#define MARGIN						24

#define BLOOD_FORE_R				128 //200
#define BLOOD_FORE_G				0 //10
#define BLOOD_FORE_B				0 //10

#define FONT_FORE_R				255 //180
#define FONT_FORE_G				0 //10
#define FONT_FORE_B				0 //10

#define FONT_SHAD_R				0
#define FONT_SHAD_G				0
#define FONT_SHAD_B				0

// Starting position for progress bar
#define PROGRESS_BAR_START_X		165
#define PROGRESS_BAR_START_Y		460

// Width of progress bar
#define PROGRESS_BAR_WIDTH			310

// Random values for creating spread of pixels (it does +/- these values!!!!)
#define PROGRESS_BAR_RANDOM_X		7
#define PROGRESS_BAR_RANDOM_Y		2

// Number of pixels to plot each time
#define PROGRESS_BAR_DENSITY		10

// How often to update the progress bar (in milliseconds)
#define PROGRESS_BAR_UPDATE_TIME	250

// Area of the screen that needs to be updated (keep 16-byte aligned for speed!!!)
#define PROGRESS_BOX_X			96
#define PROGRESS_BOX_Y			440
#define PROGRESS_BOX_WIDTH		448
#define PROGRESS_BOX_HEIGHT	40

// Get a random number plus/minus the specified value
#define RandomPlusMinus(x)		((MyRandom() % (((x) + 1) * 2)) - (x))

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )


////////////////////////////////////////////////////////////////////////////////
//
// Local random stuff
//
////////////////////////////////////////////////////////////////////////////////
static int ms_lRandom;

inline void MySeedRandom(long seed)
	{
	ms_lRandom = seed;
	}

inline long MyRandom(void)
	{
	return (((ms_lRandom = ms_lRandom * 214013L + 2531011L) >> 16) & 0x7fff);
	}


////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////
class	CCutSceneInfo
	{
	public:
		//-----------------------------------------
		RFont* m_pFont;
		char	m_szTitle[256];
		char	m_szText[4096];
		char	m_szMusic[RSP_MAX_PATH];
		UCHAR	m_ucForeText;
		UCHAR	m_ucShadowText;
		RImage*	m_pimBGLayer;
		RImage*	m_pimTextLayer;
		RImage*	m_pimDst;
		RMultiAlpha*	m_pmaAlpha;
		bool m_bDeleteFont;
		short m_sDelW;
		short m_sDelH;
		long m_lTotalBytes;
		long m_lTimeToUpdate;
		long m_lBytesSoFar;
		U8 m_u8BloodColor;
		short m_sLastDistance;
		SampleMasterID m_musicID;
		//-----------------------------------------

		void	Clear()
			{
			m_pFont = NULL;
			m_szTitle[0] = 0;
			m_szText[0] = 0;
			m_szMusic[0] = 0;
			m_ucForeText = 0;
			m_ucShadowText = 0;
			m_pimBGLayer = NULL;
			m_pimTextLayer = NULL;
			m_pimDst = NULL;
			m_pmaAlpha = NULL;
			m_bDeleteFont = true;
			m_sDelW = 0;
			m_sDelH = 0;
			m_lTotalBytes = 0;
			m_lTimeToUpdate = 0;
			m_lBytesSoFar = 0;
			m_u8BloodColor = 0;
			m_sLastDistance = 0;
			m_musicID = g_smidNil;
			}

		CCutSceneInfo()
			{
			Clear();
			}

		~CCutSceneInfo()
			{
			if (m_bDeleteFont && m_pFont) delete m_pFont;
			if (m_pimBGLayer) delete m_pimBGLayer;
			if (m_pimTextLayer) delete m_pimTextLayer;
			if (m_pmaAlpha) delete m_pmaAlpha;

			Clear();
			}
	};

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
						short	sRadius,				// Your tuning pleasure
						long	lSpinTime,			// in milliseconds
						long	lSwayTime,			// in milliseconds
						RRect*  prCenter,			// if not NULL, use this portion of the image only!
						long	lFadeTime,			// fade to black, in milliseconds.
						SampleMaster::SoundInstance siFade	// to make sound fade out
					)
	{
	// In casew Bill just haphazardly passed me the screen buffer, make a dubber copy:
	RImage imSwirl;

	rspGeneralLock(pimBackground);
	if (prCenter)
		{
		
		imSwirl.CreateImage(prCenter->sW,prCenter->sH,RImage::BMP8);

		rspBlit(pimBackground,&imSwirl,
		prCenter->sX,prCenter->sY,0,0,
		prCenter->sW,prCenter->sH);

		sStartX += prCenter->sX;
		sStartY += prCenter->sY;

		}
		else
		{
		imSwirl.CreateImage(pimBackground->m_sWidth,pimBackground->m_sHeight,RImage::BMP8);


		rspBlit(pimBackground,&imSwirl,0,0,0,0,pimBackground->m_sWidth,pimBackground->m_sHeight);	

		}

	rspGeneralUnlock(pimBackground);

	rspLockBuffer();
	rspRect(RSP_BLACK_INDEX,g_pimScreenBuf,0,0,g_pimScreenBuf->m_sWidth,g_pimScreenBuf->m_sHeight);
	rspUnlockBuffer();	// Dom't show th black!

	//==============================================================================
	// Set up clipping parameters based on radius:
	RRect	rClip(sStartX,sStartY,imSwirl.m_sWidth,imSwirl.m_sHeight);

	// shrink by radius (should work! - this should hide all redrawing!)

	rClip.sX += sRadius;
	rClip.sY += sRadius;
	rClip.sW -= sRadius<<1;
	rClip.sH -= sRadius<<1;
	//==============================================================================
	// Copy the palette:
	UCHAR PaletteCopy[256 * 3] = {0,};
	rspGetPaletteEntries(10,236,PaletteCopy+30,PaletteCopy + 31,PaletteCopy + 32,3);

	long lStartTime = rspGetMilliseconds();
	long	lCurrentTime = -1;

	long	lStartFadeTime = lMilliLen - lFadeTime;  // lCurrentTime is relative to lStartTime

	// Attempt to do a fade out while swirling....
	while (((lCurrentTime = (rspGetMilliseconds() - lStartTime)) < lMilliLen) && !(rspGetQuitStatus()))
		{
		long	lCurrentFadeTime = lCurrentTime - lStartFadeTime;
		//==============================================================================
		// Figure out stage of motion:
		//==============================================================================
		short sDeg = short((360 * lCurrentTime)/lSpinTime);
		sDeg = rspMod360(sDeg);
		short sSwayDeg = short((360 * lCurrentTime)/lSwayTime);
		sSwayDeg = rspMod360(sSwayDeg);
		
		short sR = short(sRadius * rspSin(sSwayDeg));
		short sOffX = short(rspCos(sDeg) * sR);
		short sOffY = short(rspSin(sDeg) * sR);
		//==============================================================================
		// Draw the frame!
		rspLockBuffer();

		// Opaque Blit!
		rspBlit(&imSwirl,g_pimScreenBuf,0,0,sStartX + sOffX,sStartY + sOffY,
						imSwirl.m_sWidth,imSwirl.m_sHeight,&rClip);
		// Alpha Blit!
		rspAlphaBlitT(128,pAlpha,&imSwirl,g_pimScreenBuf,
						sStartX - sOffX,sStartY - sOffY,&rClip);

		rspUnlockBuffer();
		rspUpdateDisplay();
		//---------------------------------------
		// Do a palette fade out:
		if (lCurrentFadeTime > 0)
			{
			// Update the Palette:
			short i=0;

			short sCurPal = 10 * 3;
			short sByteLevel = short((255.0 * lCurrentFadeTime) / lFadeTime);

			for (i=10; i < 246; i++, sCurPal += 3)
				{
				rspSetPaletteEntry( i,
						(PaletteCopy[sCurPal + 0] * (256 - sByteLevel)) / 256,
						(PaletteCopy[sCurPal + 1] * (256 - sByteLevel)) / 256,
						(PaletteCopy[sCurPal + 2] * (256 - sByteLevel)) / 256
						);
				}

			rspUpdatePalette();

			if (siFade)	// adjust sound volume:
				{
				SetInstanceVolume(siFade,255 - sByteLevel);
				}
			}

		//---------------------------------------
		UpdateSystem();
		}
	//==============================================================================
	// Now do a fade out
	//==============================================================================
	rspLockBuffer();
	rspRect(RSP_BLACK_INDEX,g_pimScreenBuf,0,0,g_pimScreenBuf->m_sWidth,g_pimScreenBuf->m_sHeight);
	rspUpdateDisplay();
	rspUnlockBuffer();

	return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////
class	CSwirlMe
	{
	public:
		//------------------------------------------------------------------
		short m_sCenX;		// radius in pixels
		short m_sRadX;
		short m_sCenY;		// radius in pixels
		short m_sRadY;
		double m_dCenA;	// Alpha level (0.0 to 1.0)
		double m_dRadA;
		long	m_lCycleTimeX;	// in milliseconds (Min, Max, Min)
		long	m_lCycleTimeY;	// in milliseconds (Min, Max, Min)
		long m_lCycleTimeA;	//
		long m_lTimeSpin;
		long	m_lBaseTime;
		RRect	m_rClip;			// To offset and limit the effect to a rect:
		SampleMaster::SoundInstance m_siSound;
		CCutSceneInfo* m_pCut;
		//------------------------------------------------------------------

		////////////////////////////////////////////////////////////////////////////
		//		Make next frame of effect
		////////////////////////////////////////////////////////////////////////////
		short	MakeFrame(SampleMasterID* psmid)
			{
			if (!IsSamplePlaying(*psmid))
				{
				PlaySample(										// Returns nothing.
																	// Does not fail.
					*psmid,										// In:  Identifier of sample you want played.
					SampleMaster::Unspecified,				// In:  Sound Volume Category for user adjustment
					255,											// In:  Initial Sound Volume (0 - 255)
					&m_siSound,									// Out: Handle for adjusting sound volume
					NULL,											// Out: Sample duration in ms, if not NULL.
					0,												// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
					0,												// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
					true);										// In:  Call ReleaseAndPurge rather than Release after playing
				}

			if (!m_lBaseTime) m_lBaseTime = rspGetMilliseconds();

			long lDeltaTime = rspGetMilliseconds() - m_lBaseTime;

			short sDegX = (360 * lDeltaTime) / m_lCycleTimeX;
			short sDegY = (360 * lDeltaTime) / m_lCycleTimeY;
			short sDegAlpha = (360 * lDeltaTime) / m_lCycleTimeA;
			short sDegSpin = (360 * lDeltaTime) / m_lTimeSpin;

			sDegX = rspMod360(sDegX);
			sDegY = rspMod360(sDegY);
			sDegAlpha = rspMod360(sDegAlpha);
			sDegSpin = rspMod360(sDegSpin);

			short	sRx = m_sCenX + short(rspSin(sDegX) * m_sRadX);
			short	sRy = m_sCenY + short(rspSin(sDegY) * m_sRadY);

			if (sRx < 0) sRx = 0;
			if (sRy < 0) sRy = 0;

			short sOffX = short(rspCos(sDegSpin) * sRx);
			short sOffY = -short(rspSin(sDegSpin) * sRy);
			sOffX >>= 1;
			sOffY >>= 1;

			double dAlpha = m_dCenA + rspSin(sDegAlpha) * m_dRadA;

			//----------------------- Use global alpha --------------------
			RRect rSafeClip = m_rClip;

			rSafeClip.sX += m_sRadX;
			rSafeClip.sY += m_sRadY;

			rSafeClip.sW -= (m_sRadX<<1);
			rSafeClip.sH -= (m_sRadY<<1);

//******************************************************************************
//  THIS IS A COMPLETE HACK - IT IS A LAST MINUTE ATTEMPT TO ADJUST FOR THE
//  CHANGE TO THE CUT SCENE ASSEST WHICH ADDED BLACK STRIPS TO THE TOP AND
//  BOTTOM OF THE SCREEN....
//******************************************************************************
			rSafeClip.sY += 40; // HARD CODED FOR POSTAL!
			rSafeClip.sH -= 80; // HARD CODED FOR POSTAL!
//******************************************************************************

			// 1) Put original image down upon target:

			if (m_pCut->m_pimBGLayer)
				{
				rspLockBuffer();

				rspBlit(m_pCut->m_pimBGLayer,m_pCut->m_pimDst,0,0,m_rClip.sX - sOffX,m_rClip.sY - sOffY,
						m_rClip.sW,m_rClip.sH,&rSafeClip);

				// 2) Alpha Blit Upon it:
				if (m_pCut->m_pmaAlpha != NULL)
					{
					rspAlphaBlitT(short(255.9*dAlpha),m_pCut->m_pmaAlpha,m_pCut->m_pimBGLayer,m_pCut->m_pimDst,
						m_rClip.sX + sOffX,m_rClip.sY + sOffY,&rSafeClip);
					}

				// 3) GENLOCK the text:
				rspBlit(m_pCut->m_pimTextLayer,m_pCut->m_pimDst,0,0);


				// 4) You're done - let the app update the screen -> he can use
				// the given function or do it himself!

				rspUnlockBuffer();
				}


			return SUCCESS;
			}

		////////////////////////////////////////////////////////////////////////////
		//		Configure effect
		////////////////////////////////////////////////////////////////////////////
		short Configure(//RImage* pimDst,RImage* pimSrc,RMultiAlpha* pX,
					long lTimeSpin,
					short sMinX,short sMaxX,long lTimeX,
					short sMinY,short sMaxY,long lTimeY,
					double dMinA,double dMaxA,long lTimeA,
					short sX,short sY,short sW,short sH)
			{
			ASSERT(m_pCut);
			ASSERT(lTimeX > 0);
			ASSERT(lTimeY > 0);
			ASSERT(lTimeA > 0);
			ASSERT(lTimeSpin > 0);
			ASSERT((dMinA >= 0.0) && (dMinA <= 1.0));
			ASSERT((dMaxA >= 0.0) && (dMaxA <= 1.0));
			ASSERT(sW > 0);
			ASSERT(sH> 0);
			//----------------------------------------------
			m_sRadX = (sMaxX - sMinX)>>1;
			m_sCenX = sMinX + m_sRadX;
			m_sRadY = (sMaxY - sMinY)>>1;
			m_sCenY = sMinY + m_sRadY;
			m_dRadA = (dMaxA - dMinA)/2.0;
			m_dCenA = dMinA + m_dRadA;
			m_lCycleTimeX = lTimeX;
			m_lCycleTimeY = lTimeY;
			m_lCycleTimeA = lTimeA;
			m_lTimeSpin = lTimeSpin;

			// Center the effect!
			if (m_pCut->m_pimBGLayer)
				{
				m_pCut->m_sDelW = m_pCut->m_pimDst->m_sWidth - m_pCut->m_pimBGLayer->m_sWidth;
				m_pCut->m_sDelH = m_pCut->m_pimDst->m_sHeight - m_pCut->m_pimBGLayer->m_sHeight;
				}
			else
				{
				m_pCut->m_sDelW = 0;
				m_pCut->m_sDelH = 0;
				}

			m_rClip.sX = sX + (m_pCut->m_sDelW >> 1);
			m_rClip.sY = sY + (m_pCut->m_sDelH >> 1);
			m_rClip.sW = sW - m_pCut->m_sDelW;
			m_rClip.sH = sH - m_pCut->m_sDelH;

			rspLockBuffer();

			rspRect(RSP_BLACK_INDEX,m_pCut->m_pimDst,0,0,m_pCut->m_pimDst->m_sWidth,m_pCut->m_pimDst->m_sHeight);

			rspUnlockBuffer();

			m_lBaseTime = 0;//rspGetMilliseconds();

			return SUCCESS;
			}

		////////////////////////////////////////////////////////////////////////////
		//		Erase
		////////////////////////////////////////////////////////////////////////////
		void Erase()
			{
			m_sCenX = m_sCenY = m_sRadX = m_sRadY = 
				m_lTimeSpin = m_lCycleTimeX = m_lCycleTimeY = m_lCycleTimeA = 0;
			m_dCenA = m_dRadA = 0.0;
			m_rClip = RRect(0,0,0,0);
			m_siSound = NULL;
			}

		////////////////////////////////////////////////////////////////////////////
		//		CSwirlMe
		////////////////////////////////////////////////////////////////////////////
		CSwirlMe(
			CCutSceneInfo* pCut)
			{
			m_pCut = pCut; // YOU must free the cut scene info - we won't clear it!
			Erase();
			}

		////////////////////////////////////////////////////////////////////////////
		//		~CSwirlMe
		////////////////////////////////////////////////////////////////////////////
		~CSwirlMe()
			{
 			if (m_siSound != 0) AbortSample(m_siSound);
			m_siSound = 0;
			m_pCut = NULL;	// we don't free this - you do!
			}

		////////////////////////////////////////////////////////////////////////////
		//		Update (optional) - from screen buffer only!
		////////////////////////////////////////////////////////////////////////////
		void	Update()
			{
			rspUpdateDisplay(m_rClip.sX,m_rClip.sY,m_rClip.sW,m_rClip.sH);
			}

		////////////////////////////////////////////////////////////////////////////
		//		Clear
		////////////////////////////////////////////////////////////////////////////
		void	Clear()
			{
			//if (m_pimCopy) delete m_pimCopy;
			Erase();
			}
	};


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

static CCutSceneInfo* ms_pCut = NULL;
static CSwirlMe* pSwirl = NULL;


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

static void CutScene_RFileCallback(long lBytes);


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
	short sBorderY)
	{
	// This is used for more than just paths, so it has a larger size!
	char szText[2048];

	// Init stuff
	ms_pCut = new CCutSceneInfo;
	ms_pCut->m_pimDst = g_pimScreenBuf;

	// Clear screen
	rspLockBuffer();

	rspRect(RSP_BLACK_INDEX, ms_pCut->m_pimDst, 0, 0, ms_pCut->m_pimDst->m_sWidth, ms_pCut->m_pimDst->m_sHeight);

	rspUnlockBuffer();

	rspUpdateDisplay();

	// Open the realm prefs file
	RPrefs prefsRealm;
	if (prefsRealm.Open(FullPathHD(g_GameSettings.m_pszRealmPrefsFile), "rt") != 0)
		{
		TRACE("CutSceneStart(): Error opening prefs file.\n");
		return ;
		}

	//------------------------------------------------------------------------------
	// Get BG
	//------------------------------------------------------------------------------

	// In simple mode, use default bg.  Otherwise, get bg from prefs file.
	if (bSimple)
		{
		strcpy(szText, DEFAULT_BG);
		}
	else
		{
		prefsRealm.GetVal(*pstrSection, "Bg", DEFAULT_BG, szText);
		if ((strlen(szText) + 1) >= RSP_MAX_PATH)
			{
			TRACE("CutScene(): Bg file name/path too long: '%s'!\n", szText);
			strcpy(szText, DEFAULT_BG);
			}
		}

	// Load bg, but do NOT display yet!
	ms_pCut->m_pimBGLayer = new RImage;
	if ((ms_pCut->m_pimBGLayer->Load(FullPathHD(szText)) == 0) ||
		 (ms_pCut->m_pimBGLayer->Load(FullPathVD(szText)) == 0))
		{
		// Set palette
		ASSERT(ms_pCut->m_pimBGLayer->m_pPalette != NULL);
		ASSERT(ms_pCut->m_pimBGLayer->m_pPalette->m_type == RPal::PDIB);
		rspSetPaletteEntries(
			0, 256, ms_pCut->m_pimBGLayer->m_pPalette->Red(0), 
			ms_pCut->m_pimBGLayer->m_pPalette->Green(0), 
			ms_pCut->m_pimBGLayer->m_pPalette->Blue(0), 
			ms_pCut->m_pimBGLayer->m_pPalette->m_sPalEntrySize);

		rspUpdatePalette();
		}
	else
		{
		TRACE("CutScene(): Error loading bg image: '%s'\n", FullPathVD(szText));
		delete ms_pCut->m_pimBGLayer;
		ms_pCut->m_pimBGLayer	= NULL;
		}

	//------------------------------------------------------------------------------
	// Get tables used for color matching
	//------------------------------------------------------------------------------

	U8	au8Red[256];
	U8	au8Green[256];
	U8	au8Blue[256];
	rspGetPaletteEntries(0, 256, au8Red, au8Green, au8Blue, sizeof(U8));

	//------------------------------------------------------------------------------
	// Get multialpha
	//------------------------------------------------------------------------------

	ms_pCut->m_pmaAlpha = new RMultiAlpha;

	prefsRealm.GetVal(*pstrSection, "Alpha", DEFAULT_MULTIALPHA, szText);
	if ((ms_pCut->m_pmaAlpha->Load(FullPathHD(szText)) == 0) ||
		 (ms_pCut->m_pmaAlpha->Load(FullPathVD(szText)) == 0))
		{
		}
	else
		{
		TRACE("CutScene(): Error loading multialpha: '%s'\n", FullPathVD(szText));
		delete ms_pCut->m_pmaAlpha;
		ms_pCut->m_pmaAlpha = NULL;
		}

	//------------------------------------------------------------------------------
	// Get font
	//------------------------------------------------------------------------------

	// Try to load font, but if it fails, use the already-loaded game font
	ms_pCut->m_bDeleteFont = true;
	ms_pCut->m_pFont = new RFont;
	if (ms_pCut->m_pFont->Load(FullPathVD(FONT_FILE)) != SUCCESS)
		{
		TRACE("CutScene(): Error loading font: '%s'!\n", FullPathVD(FONT_FILE));
		delete ms_pCut->m_pFont;
		ms_pCut->m_pFont = &g_fontBig; // system font
		ms_pCut->m_bDeleteFont = false; // don't try to free it!
		}

	// Find closest matches for the desired colors
	ms_pCut->m_ucForeText = rspMatchColorRGB(
		FONT_FORE_R, FONT_FORE_G, FONT_FORE_B, 10, 236, au8Red, au8Green, au8Blue, sizeof(U8));
	ms_pCut->m_ucShadowText = rspMatchColorRGB(
		FONT_SHAD_R, FONT_SHAD_G, FONT_SHAD_B, 10, 236, au8Red, au8Green, au8Blue, sizeof(U8));

	// Setup print
	RPrint print;


	//------------------------------------------------------------------------------
	// Get title
	//------------------------------------------------------------------------------

	// In simple mode, use default title.  Otherwise, get title from prefs file.
	if (bSimple)
		{
		strcpy(ms_pCut->m_szTitle, DEFAULT_TITLE);
		}
	else
		{
		prefsRealm.GetVal(*pstrSection, "Title", DEFAULT_TITLE, ms_pCut->m_szTitle);
		if ((strlen(ms_pCut->m_szTitle) + 1) >= sizeof(ms_pCut->m_szTitle))
			{
			TRACE("CutScene(): Title too long: '%s'!\n", ms_pCut->m_szTitle);
			strcpy(ms_pCut->m_szTitle, DEFAULT_TITLE);
			}
		}

 	//------------------------------------------------------------------------------
	// Get music
	//------------------------------------------------------------------------------

	// In simple mode, use the default music, otherwise get the music from prefs file
	if (bSimple)
		{
		strcpy(ms_pCut->m_szMusic, DEFAULT_MUSIC);
		}
	else
		{
		prefsRealm.GetVal(*pstrSection, "Music", DEFAULT_MUSIC, ms_pCut->m_szMusic);
		if ((strlen(ms_pCut->m_szMusic) + 1) >= sizeof(ms_pCut->m_szMusic))
			{
			TRACE("CutScene(): Music filename too long: '%s'!\n", ms_pCut->m_szMusic);
			strcpy(ms_pCut->m_szMusic, DEFAULT_MUSIC);
			}
		}

	// Set the resource name for the sample
	ms_pCut->m_musicID.pszId			= ms_pCut->m_szMusic;
	ms_pCut->m_musicID.usDescFlags	= SMDF_NO_DESCRIPT;

	//------------------------------------------------------------------------------
	//	Create the Text overlay layer (GENLOCK!)
	//------------------------------------------------------------------------------
	
	ms_pCut->m_pimTextLayer = new RImage;
	ms_pCut->m_pimTextLayer->CreateImage(ms_pCut->m_pimDst->m_sWidth,ms_pCut->m_pimDst->m_sHeight,RImage::BMP8);

	// Create text layer
	print.SetDestination(ms_pCut->m_pimTextLayer);
	print.SetFont(48, ms_pCut->m_pFont);
	print.SetColor(ms_pCut->m_ucForeText, 0, ms_pCut->m_ucShadowText);
	print.SetEffectAbs(RPrint::SHADOW_X, 2);
	print.SetEffectAbs(RPrint::SHADOW_Y, 2);
	print.print(((ms_pCut->m_pimTextLayer->m_sWidth - print.GetWidth(ms_pCut->m_szTitle)) / 2),
		NAME_Y, ms_pCut->m_szTitle);

	// Get text to be displayed
	if (bSimple)
		{
		strcpy(ms_pCut->m_szText, DEFAULT_TEXT);
		}
	else
		{
		prefsRealm.GetVal(*pstrSection, "Text", DEFAULT_TEXT, ms_pCut->m_szText);
		if ((strlen(ms_pCut->m_szText) + 1) >= sizeof(ms_pCut->m_szText))
			{
			TRACE("CutScene(): Text too long: '%s'!\n", ms_pCut->m_szText);
			strcpy(ms_pCut->m_szText, DEFAULT_TEXT);
			}
		}

	// Create cropping rectangle
	// Try to compensate by the way the POSTAL cut scene assets were hosed

	RRect	rCrop;
	if (ms_pCut->m_pimBGLayer)
		{
		rCrop = RRect(
			BG_X + sBorderX,
			BG_Y + sBorderY + 40, // there were horizontal strips implanted
			ms_pCut->m_pimBGLayer->m_sWidth - (sBorderX * 2),
			ms_pCut->m_pimBGLayer->m_sHeight - (sBorderY * 2) - 80);
		}
	else
		{
		rCrop.sX	= 0;
		rCrop.sY	= 0;
		rCrop.sW	= g_pimScreenBuf->m_sWidth - (sBorderX * 2);
		rCrop.sH	= g_pimScreenBuf->m_sHeight - (sBorderY * 2) - 80;
		}

	// Create text layer
	print.SetFont(28, ms_pCut->m_pFont);
	print.SetColor(ms_pCut->m_ucForeText, 0, ms_pCut->m_ucShadowText);
	print.SetEffectAbs(RPrint::SHADOW_X, 1);
	print.SetEffectAbs(RPrint::SHADOW_Y, 1);
	print.SetWordWrap(TRUE);
	print.SetColumn(rCrop.sX + MARGIN, rCrop.sY, rCrop.sW - (MARGIN*2), rCrop.sH);
	print.print(0, TEXT_Y, ms_pCut->m_szText);

	ms_pCut->m_pimTextLayer->Convert(RImage::FSPR8);

	//------------------------------------------------------------------------------
	// Update the actual screen now that everything is ready
	//------------------------------------------------------------------------------

	// We must lock the composite buffer before reading/writing it.
	rspLockBuffer();

	// Clear dst to black
	rspRect(
		RSP_BLACK_INDEX,
		ms_pCut->m_pimDst,
		0, 0,
		ms_pCut->m_pimDst->m_sWidth, ms_pCut->m_pimDst->m_sHeight);

	if (ms_pCut->m_pimBGLayer)
		{
		// Copy bg image to dst, cropping as we go
		rspBlit(
			ms_pCut->m_pimBGLayer,
			ms_pCut->m_pimDst,
			0, 0,
			BG_X, BG_Y,
			ms_pCut->m_pimBGLayer->m_sWidth, ms_pCut->m_pimBGLayer->m_sHeight,
			&rCrop);

		RRect	rcBGClipper(
			0,			
			0,	
			ms_pCut->m_pimBGLayer->m_sWidth,
			ms_pCut->m_pimBGLayer->m_sHeight);

		// Copy progress bar area of bg image to dst with SOURCE clipping
		// just in case image is too small.
		rspBlit(
			ms_pCut->m_pimBGLayer,								// Src.
			ms_pCut->m_pimDst,									// Dst.
			PROGRESS_BOX_X, PROGRESS_BOX_Y,					// Src.
			PROGRESS_BOX_X, PROGRESS_BOX_Y,					// Dst.
			PROGRESS_BOX_WIDTH, PROGRESS_BOX_HEIGHT,		// Both.
			NULL,														// Dst.
			&rcBGClipper);											// Src.
		}

	// Copy text image to dst, cropping as we go
	rspBlit(
		ms_pCut->m_pimTextLayer,
		ms_pCut->m_pimDst,
		0, 0,
		&rCrop);

	// Release composite buffer now that we are done.
	rspUnlockBuffer();

	// Update display
	rspUpdateDisplay();

	//------------------------------------------------------------------------------
	// Close prefs file.
	//------------------------------------------------------------------------------
	prefsRealm.Close();

	//------------------------------------------------------------------------------
	// Setup stuff for the progress bar
	//------------------------------------------------------------------------------

	// Hook the RFile callback that tells us how much data is about to be read/written
	RFile::ms_criticall = CutScene_RFileCallback;

	// Reset various stuff
	ms_pCut->m_lBytesSoFar = 0;
	ms_pCut->m_lTotalBytes = 12000000;
	ms_pCut->m_lTimeToUpdate = rspGetMilliseconds();
	ms_pCut->m_sLastDistance = 0;

	// Find good blood color
	ms_pCut->m_u8BloodColor = rspMatchColorRGB(
		BLOOD_FORE_R, BLOOD_FORE_G, BLOOD_FORE_B, 1, 254, au8Red, au8Green, au8Blue, sizeof(U8));
	}


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
	short sX,short sY,short sW,short sH)
	{
	ASSERT(ms_pCut);
	pSwirl = new CSwirlMe(ms_pCut);

	if (ms_pCut->m_pimBGLayer)
		{
		// Erase progress bar region from BG layer and dst and display
		// Note: We use the color index in the upper left corner of the BG layer
		// to erase this area.  We can't use the expected RSP_BLACK_INDEX because
		// on the mac, it creates problems with the multialpha stuff.  We assume
		// the pixel in the upper left corner is black, but it's a DIFFERENT black
		// which is safe to use with the multialpha stuff.
		rspRect(
			*(ms_pCut->m_pimBGLayer->m_pData),
			ms_pCut->m_pimBGLayer,
			PROGRESS_BOX_X, PROGRESS_BOX_Y,
			PROGRESS_BOX_WIDTH, PROGRESS_BOX_HEIGHT);

		RRect	rcBGClipper(
			0,
			0,
			ms_pCut->m_pimBGLayer->m_sWidth,
			ms_pCut->m_pimBGLayer->m_sHeight);

		rspLockBuffer();

		// Copy progress bar area of bg image to dst with SOURCE clipping
		// just in case image is too small.
		rspBlit(
			ms_pCut->m_pimBGLayer,								// Src.
			ms_pCut->m_pimDst,									// Dst.
			PROGRESS_BOX_X, PROGRESS_BOX_Y,					// Src.
			PROGRESS_BOX_X, PROGRESS_BOX_Y,					// Dst.
			PROGRESS_BOX_WIDTH, PROGRESS_BOX_HEIGHT,		// Both.
			NULL,														// Dst.
			&rcBGClipper);											// Src.

		rspUnlockBuffer();

		rspUpdateDisplay(PROGRESS_BOX_X, PROGRESS_BOX_Y, PROGRESS_BOX_WIDTH, PROGRESS_BOX_HEIGHT);
		}

	// Clear RFile hook so nothing gets drawn after this
	RFile::ms_criticall = 0;

	return pSwirl->Configure(
		lTimeSpin,
		sMinX, sMaxX, lTimeX,
		sMinY, sMaxY, lTimeY,
		dMinA, dMaxA, lTimeA,
		sX, sY, sW, sH);
	}


////////////////////////////////////////////////////////////////////////////
//
// Update cutscene effect
//
////////////////////////////////////////////////////////////////////////////
extern void CutSceneUpdate(void)
	{
	ASSERT(ms_pCut);
	ASSERT(pSwirl);

	pSwirl->MakeFrame(&(ms_pCut->m_musicID));
	pSwirl->Update();
	}


////////////////////////////////////////////////////////////////////////////////
//
// Clean up after the cutscene.  It is safe to call this multiple times.
//
////////////////////////////////////////////////////////////////////////////////
extern void CutSceneEnd(void)
	{
	delete ms_pCut;
	ms_pCut = 0;

	delete pSwirl;
	pSwirl = 0;

	// Clear RFile hook
	RFile::ms_criticall = 0;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Our RFile callback
//
////////////////////////////////////////////////////////////////////////////////
static void CutScene_RFileCallback(long lBytes)
	{
	static short asWavyY[] =
		{
		0, 1, 2, 0, -2, -1, 1, 0, -1, 0, 1, 2, 1, 0,
		1, 0, -1, -2, -1, -2, 0, 1, 2, 0, -2, -1, 0, 0
		};

	if (ms_pCut)
		{
		if (ms_pCut->m_pimBGLayer && ms_pCut->m_pmaAlpha)
			{
			if (ms_pCut->m_pimBGLayer->m_type != RImage::NOT_SUPPORTED)
				{
				// Update bytes so far
				ms_pCut->m_lBytesSoFar += lBytes;

				// Check if time for an update
				long lNow = rspGetMilliseconds();
				if ((lNow - ms_pCut->m_lTimeToUpdate) > PROGRESS_BAR_UPDATE_TIME)
					{
					// Get percentage that's been loaded so far (result is from 0 to 1)
					double dPercentage = (double)ms_pCut->m_lBytesSoFar / (double)ms_pCut->m_lTotalBytes;
					if (dPercentage > 1.0)
						dPercentage = 1.0;
					short sDistance = (short)((double)PROGRESS_BAR_WIDTH * dPercentage);

					short sBaseY = asWavyY[(short)(dPercentage * (NUM_ELEMENTS(asWavyY) - 1) )];
						 
					// Draw from previous distance to current distance in case a big jump occurred
					for (short sBaseX = ms_pCut->m_sLastDistance; sBaseX <= sDistance; sBaseX++)
						{
						// Draw a few pixels at a time to get a more solid look
						for (short i = 0; i < PROGRESS_BAR_DENSITY; i++)
							{
							// Choose random location nearby the specified location
							short sX = PROGRESS_BAR_START_X + sBaseX + RandomPlusMinus(PROGRESS_BAR_RANDOM_X);
							short sY = PROGRESS_BAR_START_Y + sBaseY + RandomPlusMinus(PROGRESS_BAR_RANDOM_Y);

							// Draw an alpha'd pixel at this location
							U8 u8dst = *(ms_pCut->m_pimBGLayer->m_pData + (sY * ms_pCut->m_pimBGLayer->m_lPitch) + sX);
							rspPlot(
								rspBlendColor(150, ms_pCut->m_pmaAlpha, ms_pCut->m_u8BloodColor, u8dst),
								ms_pCut->m_pimBGLayer,
								sX,
								sY);
							}
						}

					// Save for next time
					ms_pCut->m_sLastDistance = sDistance;
					
					// Update progress bar area

					RRect	rcBGClipper(
						0,
						0,
						ms_pCut->m_pimBGLayer->m_sWidth,
						ms_pCut->m_pimBGLayer->m_sHeight);

					rspLockBuffer();

					// Copy progress bar area of bg image to dst with SOURCE clipping
					// just in case image is too small.
					rspBlit(
						ms_pCut->m_pimBGLayer,								// Src.
						ms_pCut->m_pimDst,									// Dst.
						PROGRESS_BOX_X, PROGRESS_BOX_Y,					// Src.
						PROGRESS_BOX_X, PROGRESS_BOX_Y,					// Dst.
						PROGRESS_BOX_WIDTH, PROGRESS_BOX_HEIGHT,		// Both.
						NULL,														// Dst.
						&rcBGClipper);											// Src.

					rspUnlockBuffer();

					rspUpdateDisplay(PROGRESS_BOX_X, PROGRESS_BOX_Y, PROGRESS_BOX_WIDTH, PROGRESS_BOX_HEIGHT);
					
					// Do an update
					UpdateSystem();

					// Save new time
					ms_pCut->m_lTimeToUpdate = lNow;
					}
				}
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

