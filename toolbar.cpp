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
// toolbar.cpp
// Project: Nostril (aka Postal)
//
// This module augments the CDude class, which used to be soley responsible for 
// updating player and weapon status.  It currently graphically manages the
// toolbars and sound reactions.  (Still need to add sounds)
////////////////////////////////////////////////////////////////////////////////
//
// History:
//
//		08/06/97 JRD	Started.  Set bars to load in the realm
//
//		08/08/97 JRD	Changed methodology to a proprietary data format and abandoning
//							attempts of a one to one interface with postal.  Tollbar will 
//							update an internal map of the weapon states based on events
//							passed.
//
//		08/08/97	JRD	Completely revamped and finalized the class structure.
//							Added the first hook to draw the entire bar.
//							Added aliased functions to reduce header dependencies.
//		08/09/97	JRD	Added all functionality except a hook to refresh on event.
//
//		08/10/97	JRD	Added two deferred update modes.  One limits updates to a 
//							given time interval, and the other only updates when a 
//							change of value or state has occurred.  All this occurs
//							without any interaction with the app.
//
//		08/10/97	JRD	Added hook so powerup values resetting due to time would
//							be a valid draw event.
//
//		08/10/97	JRD	Based on Steve Wik's opinion, I set health and vest to 
//							show well over 100%.  Since people liked it, it was actually
//							my opinion and I shall keep it.
//
//		08/10/97	JRD	Added a dangerous cross file help to poor old score in order
//							to easily color map the top toolbar.  Poor score.  Set to Steve
//							colors.  If you think their cool, then I too agreed with Steve. 
//							If not, then Steve forced me to use them.
//
//		08/12/97	JRD	Changed the class o the Mac can deal with it.
//
//		08/29/97	JRD	Patched Render to not display amount of bullets.
//
//		09/27/99	JMI	Eliminated boolean performance warnings.
//
////////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"
#include "dude.h"
#include "StockPile.h"
#include "hood.h"
#include "toolbar.h"
#include "ORANGE/color/colormatch.h"
#include "ORANGE/color/dithermatch.h"
////////////////////////////////////////////////////////////////////////////////
// TOOLBAR Graphic parameters:
////////////////////////////////////////////////////////////////////////////////

//=================== toolbar templates (loaded each level) ====================

//=====================  General Toolbar Parameters  ===========================
#define	TB_BAR_X	0				// where to draw the entire bar
#define	TB_BAR_Y	440

#define	TB_TWEAK_FONT_Y		-5
#define	TB_TWEAK_FONT_X		-3

#define  TB_SMALL_FONT			11		// pixel height for ammo font
#define	TB_LARGE_FONT			15		// for health and armour

#define	TB_AMMOW_LOW	3			// ammo changes color as warning
#define	TB_MILLI_TO_LITE	3000	// after you get a powerup.

//#ifdef MOBILE
#if 1
#define TB_MILLI_INTERVAL_TIME	100 //500ms lag is pretty annoying!
#else
#define	TB_MILLI_INTERVAL_TIME	500	// maximum update rate, unless forced
#endif

extern int wideScreenWidth;


extern RFont g_fontBig;	// I hope this one is OK....

typedef	struct { UCHAR r; UCHAR g; UCHAR b; } MatchColor;

MatchColor	gmcSmallFont = { 255,255,0 };	// yellow
MatchColor	gmcLargeFont = { 255,255,0 };	// yellow
MatchColor	gmcWarningFont = { 255,0,0 };	// red
MatchColor	gmcAmmoGoneFont = { 96,0,0 };	// dark red
MatchColor	gmcAttentionFont = { 255,255,128 };	// saturated yellow

//------------------ Top Bar:
MatchColor	gmcSolidScore = { 40,34,13 };	// saturated yellow
MatchColor	gmcShadowScore = { 138,123,65 };	// saturated yellow
//MatchColor	gmcShadowScore = { 119,102,60 };	// saturated yellow
//MatchColor	gmcSolidScore = { 0,0,255 };	// saturated yellow
//MatchColor	gmcShadowScore = { 255,0,0 };	// saturated yellow

//------------------ So Score can access these:
short gsStatusFontForeIndex	=	251;
short gsStatusFontBackIndex	=	0;
short gsStatusFontShadowIndex	=	0;
short gsStatusFontForeDeadIndex = 252;

//=====================================================================
//===========  INTERNAL VISUAL STOCKPILE CONTROLS  ====================
//=====================================================================

// I am abandoning the previous idea of linking my own definition
// of weapons and ammo to postal's, as they are too incompatible
// for a direct interface.  I am therefor switching to a proprietary one.

// Local definition of a "WEAPON":  Does NOT have an associated number
// Local definition of an "AMMO":	Has a number amount.

// Proprietary formats:
typedef	enum
	{
	NotWeapon = 0,
	MachineGun,
	ShotGun,
	SprayCannon,
	MissileLauncher,
	NapalmLauncher,
	FlameThrower,
	NumberOfWeapons
	}	ToolWeaponType;


typedef	enum
	{
	NotAmmo = 0,
	Health,
	KevlarVest,
	Bullets,
	Shells,
	Spray,		// proprietary = Bullets
	Grenades,
	Rockets,
	HeatSeekers,// proprietary = Heatseekers
	Cocktails,
	Napalm,
	Fuel,
	ProximityMine, // all three alias to the same amount of mines!
	TimedMine, 
	BouncingBettyMine,
	NumberOfAmmos
	}	ToolAmmoType;

// NOTE: The concept is that, in general, you select an AMMO, and the weapon
// highlights dependently.
//
class	CToolItem
	{
public:
	//----------------------------------------------------------------------
	typedef enum { Uninitialized = 0, Weapon , Ammo } Type;
	typedef enum { Bar = 0, BarSel = 1, Full = 2, FullSel = 3 } State;
	typedef enum { Small = 0, Large } Size;
	//----------------------------------------------------------------------
	Type	m_eType;
	bool	m_bExists;		// bar or full?
	bool	m_bSelected;	// selected or not?
	bool	m_bTreasure;	// found in a power up
	long	m_lMilli;		// relative time in milliseconds for timing stuff
	State	m_eState;		// short cut for applying state
	State	m_ePrevState;	// Stored for event triggering
	double	m_dValue;	// if ammo
	double	m_dPrevValue;	// if ammo
	double	m_dLow;		// value change
	Size	m_eFontType;	// for color and display
	RRect	m_rImage;		// location of icon, if applicable
	RRect m_rText;			// location of text, if applicable
	//----------------------------------------------------------------------
	CToolItem*	m_pWeapon;	// Links to associated weapon
	CToolItem*	m_pAmmo1;	// Links to associated ammo for drawing order
	CToolItem*	m_pAmmo2;	// Links to associated ammo for drawing order
	//----------------------------------------------------------------------
	ToolWeaponType			m_eWeaponType;
	ToolAmmoType			m_eAmmoType;
	CDude::WeaponType		m_eStockPile;	// Postal app equivalent from Dude.h
	//----------------------------------------------------------------------
	static CToolItem* ms_aWeapons; // [NumberOfWeapons];
	static CToolItem* ms_aAmmo; //[NumberOfAmmos];
	static	RFont*	ms_pfntTool;		// General font and print
	static	RPrint	ms_pntTool;
	static	short		ms_sSmallFontColor;	// color index
	static	short		ms_sLargeFontColor;
	static	short		ms_sWarningColor;
	static	short		ms_sAmmoGoneColor;
	static	short		ms_sAttentionColor;
	static	RImage*	ms_pimCompositeBuffer;
	static	RImage*	ms_pimCompositeBufferScaled;
	static	long		ms_lLastTime;
	//----------------------------------------------------------------------
	CToolItem()
		{
		m_eType = Uninitialized;
		m_bExists = false;
		m_bSelected = false;
		m_bTreasure = false;
		m_lMilli = 0;
		m_eState = Bar;
		m_dValue = 0.0;
		m_dPrevValue = 0.0;
		m_dLow = 0;
		m_eFontType = Small;
		m_pWeapon = m_pAmmo1 = m_pAmmo2 = NULL;
		m_eWeaponType = NotWeapon;
		m_eAmmoType = NotAmmo;
		m_eStockPile = CDude::NoWeapon;
		}

	~CToolItem()	
		{ 
		}

	// This sets most members in a generic way for a weapon
	// It can add members if more differentiation is needed
	void	ArrangeWeapon(
		ToolWeaponType eType,
		CDude::WeaponType eStock,
		const RRect &prImage,
		CToolItem*	pAmmo1,
		CToolItem*	pAmmo2 = NULL,
		CToolItem*	pAmmo3 = NULL)
		{
		m_eWeaponType = eType;
		m_eStockPile = eStock;
		m_rImage = prImage;
		m_pWeapon = pAmmo1;
		m_pAmmo1 = pAmmo2;
		m_pAmmo2 = pAmmo3;
		//-------------------- Weapon specific:
		m_eType = Weapon;
		}

	void	ArrangeAmmo(
		ToolAmmoType eType,
		CDude::WeaponType eStock,
		const RRect &prImage,
		short sTextX,
		short sTextY,
		CToolItem*	pWeapon,
		Size		eSize = Small)
		{
		m_eAmmoType = eType;
		m_eStockPile = eStock;
		m_rImage = prImage;
		m_pWeapon = pWeapon;
		m_rText.sX = sTextX;
		m_rText.sY = sTextY;
		m_eFontType = eSize;
		//-------------------- Weapon specific:
		m_eType = Ammo;
		m_dLow = 3.0;	// General state for alert point
		}

	//=======================================================================
	static short	Init(CHood* pHood)	// do color matching & asset loading
		{
		// Use the current hood palette to color match the text indicies
		ms_sSmallFontColor = rspMatchColorRGB(
		gmcSmallFont.r,gmcSmallFont.g,gmcSmallFont.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

		ms_sLargeFontColor = rspMatchColorRGB(
		gmcLargeFont.r,gmcLargeFont.g,gmcLargeFont.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

		ms_sWarningColor = rspMatchColorRGB(
		gmcWarningFont.r,gmcWarningFont.g,gmcWarningFont.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

		ms_sAmmoGoneColor = rspMatchColorRGB(
		gmcAmmoGoneFont.r,gmcAmmoGoneFont.g,gmcAmmoGoneFont.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

		ms_sAttentionColor = rspMatchColorRGB(
		gmcAttentionFont.r,gmcAttentionFont.g,gmcAttentionFont.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

		//----------------------------- Match Top bar stuff:
		gsStatusFontForeIndex = rspMatchColorRGB(
		gmcSolidScore.r,gmcSolidScore.g,gmcSolidScore.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

		gsStatusFontShadowIndex = rspMatchColorRGB(
		gmcShadowScore.r,gmcShadowScore.g,gmcShadowScore.b,10,236,
		pHood->m_pimBackground->m_pPalette->Red(),
		pHood->m_pimBackground->m_pPalette->Green(),
		pHood->m_pimBackground->m_pPalette->Blue(),4);

#if 0	// checking for palette errors:
		rspSetWindowColors();

		U8 r[256],g[256],b[256];	// for palette checking:
		rspGetPaletteEntries(0,256,r,g,b,1);
		short i;

		for (i=0;i < 256;i++)
			{
			if ( (*pHood->m_pimBackground->m_pPalette->Red(i) != r[i]) ||
				 (*pHood->m_pimBackground->m_pPalette->Green(i) != g[i]) ||
				  (*pHood->m_pimBackground->m_pPalette->Blue(i) != b[i]) )
				{
				TRACE("INDEX %d: POST(%d,%d,%d),WIN(%d,%d,%d)\n",i,
					(short)*pHood->m_pimBackground->m_pPalette->Red(i),
					(short)*pHood->m_pimBackground->m_pPalette->Green(i),
					(short)*pHood->m_pimBackground->m_pPalette->Blue(i),
					(short)r[i],(short)g[i],(short)b[i]
					);
				}
			}

#endif

		ms_pfntTool = &g_fontBig;

		if (!ms_pimCompositeBuffer)
			{
			ms_pimCompositeBuffer = new RImage;
			ms_pimCompositeBuffer->CreateImage(
				pHood->m_pimEmptyBar->m_sWidth,
				pHood->m_pimEmptyBar->m_sHeight,
				RImage::BMP8);


			ms_pimCompositeBufferScaled = new RImage;
			ms_pimCompositeBufferScaled->CreateImage(
				wideScreenWidth, //HACK HACK
				pHood->m_pimEmptyBar->m_sHeight,
				RImage::BMP8);
			}

		ms_pntTool.SetFont(TB_SMALL_FONT,ms_pfntTool);
		ms_pntTool.SetDestination(ms_pimCompositeBuffer);

		// Set up the bar to a neutral background:
		rspBlit(pHood->m_pimEmptyBar,ms_pimCompositeBuffer,0,0,0,0,
					ms_pimCompositeBuffer->m_sWidth,
					ms_pimCompositeBuffer->m_sHeight);


		return SUCCESS;
		}

	// Assume the entire bar needs to be redrawn.
	static void	RenderBar(CHood* pHood,RImage* pimDst,short sDstX,short sDstY)
		{
		// First Draw all the weapons...
		short i;
		RImage*	pimPlane = NULL;

		// Set up the bar to a neutral background:
		rspBlit(pHood->m_pimEmptyBar,ms_pimCompositeBuffer,0,0,0,0,
					ms_pimCompositeBuffer->m_sWidth,
					ms_pimCompositeBuffer->m_sHeight);

		for (i = NotWeapon + 1; i < NumberOfWeapons; i++)
			{
			switch	(ms_aWeapons[i].m_eState)
				{
				case Bar: pimPlane = pHood->m_pimEmptyBar; break;
				case BarSel: pimPlane = pHood->m_pimEmptyBarSelected; break;
				case Full: pimPlane = pHood->m_pimFullBar; break;
				case FullSel: pimPlane = pHood->m_pimFullBarSelected; break;
				}

			// copy the graphic:
			rspBlit(pimPlane,ms_pimCompositeBuffer,
				ms_aWeapons[i].m_rImage.sX,
				ms_aWeapons[i].m_rImage.sY,
				ms_aWeapons[i].m_rImage.sX,
				ms_aWeapons[i].m_rImage.sY,
				ms_aWeapons[i].m_rImage.sW,
				ms_aWeapons[i].m_rImage.sH);
			}

		// Then, draw all the ammo...
		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			switch	(ms_aAmmo[i].m_eState)
				{
				case Bar: pimPlane = pHood->m_pimEmptyBar; break;
				case BarSel: pimPlane = pHood->m_pimEmptyBarSelected; break;
				case Full: pimPlane = pHood->m_pimFullBar; break;
				case FullSel: pimPlane = pHood->m_pimFullBarSelected; break;
				}

			if (ms_aAmmo[i].m_rImage.sW && ms_aAmmo[i].m_rImage.sH)
			rspBlit(pimPlane,ms_pimCompositeBuffer,
				ms_aAmmo[i].m_rImage.sX,
				ms_aAmmo[i].m_rImage.sY,
				ms_aAmmo[i].m_rImage.sX,
				ms_aAmmo[i].m_rImage.sY,
				ms_aAmmo[i].m_rImage.sW,
				ms_aAmmo[i].m_rImage.sH);
			}


		// Finally, print all the values...
		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			short sFontSize;
			short	sFontColor;

			// choose the correct font.
			if (ms_aAmmo[i].m_eFontType == Small)
				{
				sFontSize = TB_SMALL_FONT;
				sFontColor = ms_sSmallFontColor;
				}
			else
				{
				sFontSize = TB_LARGE_FONT;
				sFontColor = ms_sLargeFontColor;
				}

			// Update the attention color time:
		   if (ms_aAmmo[i].m_bTreasure)
				{
				if ( (rspGetMilliseconds() - ms_aAmmo[i].m_lMilli) >
					TB_MILLI_TO_LITE)
					{
					// no longer highlighted:
					ms_aAmmo[i].m_bTreasure = false;
					}
				}

			// check for special color conditions:
			if (ms_aAmmo[i].m_dValue <= 0.1) sFontColor = ms_sAmmoGoneColor;
			else if (ms_aAmmo[i].m_bTreasure) sFontColor = ms_sAttentionColor;
			else if (ms_aAmmo[i].m_dValue <= ms_aAmmo[i].m_dLow)
					sFontColor = ms_sWarningColor;

			// select the correct font and draw the amount:
			ms_pntTool.SetFont(sFontSize,ms_pfntTool);
			ms_pntTool.SetColor(sFontColor);

			if (i != Bullets)	// nolonger print the amount of bullets...
				{
				ms_pntTool.print(ms_aAmmo[i].m_rText.sX,ms_aAmmo[i].m_rText.sY,
					"%3ld",long(ms_aAmmo[i].m_dValue));
				}
			}


		//Really nasty, scale the image incase its widescreen
		if (ms_pimCompositeBuffer->m_sWidth != ms_pimCompositeBufferScaled->m_sWidth)
		{
			float widthScale = (float)ms_pimCompositeBuffer->m_sWidth / (float)ms_pimCompositeBufferScaled->m_sWidth;
			for (int n=0;n<ms_pimCompositeBuffer->m_sHeight;n++)
			{
				for (int x=0;x<ms_pimCompositeBufferScaled->m_sWidth;x++)
				{
					UCHAR* dest = ms_pimCompositeBufferScaled->m_pData + n * ms_pimCompositeBufferScaled->m_lPitch + x;
					UCHAR* src = ms_pimCompositeBuffer->m_pData + n * ms_pimCompositeBuffer->m_lPitch + (int)((float)x * widthScale);
					*dest = *src;
				}
				//memcpy(ms_pimCompositeBufferScaled->m_pData + n * ms_pimCompositeBufferScaled->m_lPitch,
				//		ms_pimCompositeBuffer->m_pData + n * ms_pimCompositeBuffer->m_lPitch,ms_pimCompositeBuffer->m_sWidth);
			}
			rspBlit(ms_pimCompositeBufferScaled,pimDst,0,0,sDstX,sDstY,
					ms_pimCompositeBufferScaled->m_sWidth,
					ms_pimCompositeBufferScaled->m_sHeight);
		}
		else //No scaling needed
		{
			// put into background
			rspBlit(ms_pimCompositeBuffer,pimDst,0,0,sDstX,sDstY,
					ms_pimCompositeBuffer->m_sWidth,
					ms_pimCompositeBuffer->m_sHeight);
		}

		}

	// Handle dude events, including graphical updates
	// Wil return true if a change of state has occurred
	static bool	UpdateStatus(CDude* pDude)
		{
		// Fist, try to get all the status possible from the CDude:
		// Use the stockpile for most:
		short i;

		// Get the current values from the stockpile:
		// Because GetWeaponInfo goes by weapon to recieve ammo,
		// we need to check ammo directly:

		//==========================================================================
		// Save the old ammo amounts and ammo state:
		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			// do this to catch powerups or ammot changes:
			ms_aAmmo[i].m_dPrevValue = ms_aAmmo[i].m_dValue;
			ms_aAmmo[i].m_ePrevState = ms_aAmmo[i].m_eState;
			}

		// Save the states of all the weapons:
		for (i = NotWeapon + 1; i < NumberOfWeapons; i++)
			{
			// do this to catch powerups or ammot changes:
			ms_aWeapons[i].m_ePrevState = ms_aWeapons[i].m_eState;
			}
		//==========================================================================

		// Hit points must be translated into a percentage of the whole:
		ms_aAmmo[Health].m_dValue = double(pDude->m_stockpile.m_sHitPoints)*100/
			pDude->m_sOrigHitPoints;

		if ( (ms_aAmmo[Health].m_dValue > 0.0) && (ms_aAmmo[Health].m_dValue < 1.0) )
			{
			ms_aAmmo[Health].m_dValue = 1.0;
			}

		if (pDude->IsDead()) ms_aAmmo[Health].m_dValue = 0.0;

		ms_aAmmo[KevlarVest].m_dValue = double(pDude->m_stockpile.m_sKevlarLayers)/5.0;
		//if (ms_aAmmo[KevlarVest].m_dValue > 100.0) ms_aAmmo[KevlarVest].m_dValue = 100.0;

		ms_aAmmo[Bullets].m_dValue = pDude->m_stockpile.m_sNumBullets;
		ms_aAmmo[Shells].m_dValue = pDude->m_stockpile.m_sNumShells;
		ms_aAmmo[Spray].m_dValue = pDude->m_stockpile.m_sNumShells;
		ms_aAmmo[Grenades].m_dValue = pDude->m_stockpile.m_sNumGrenades;
		ms_aAmmo[Rockets].m_dValue = pDude->m_stockpile.m_sNumMissiles;
		ms_aAmmo[HeatSeekers].m_dValue = pDude->m_stockpile.m_sNumHeatseekers;
		ms_aAmmo[Cocktails].m_dValue = pDude->m_stockpile.m_sNumFireBombs;
		ms_aAmmo[Napalm].m_dValue = pDude->m_stockpile.m_sNumNapalms;
		ms_aAmmo[Fuel].m_dValue = double(pDude->m_stockpile.m_sNumFuel)/2.50;
		ms_aAmmo[ProximityMine].m_dValue = pDude->m_stockpile.m_sNumMines;
		ms_aAmmo[TimedMine].m_dValue = pDude->m_stockpile.m_sNumMines;
		ms_aAmmo[BouncingBettyMine].m_dValue = pDude->m_stockpile.m_sNumMines;

		// Which weapons do I have?
		ms_aWeapons[MachineGun].m_bExists = (pDude->m_stockpile.m_sMachineGun) ? true : false;
		ms_aWeapons[ShotGun].m_bExists = (pDude->m_stockpile.m_sShotGun) ? true : false;
		ms_aWeapons[SprayCannon].m_bExists = (pDude->m_stockpile.m_sSprayCannon) ? true : false;
		ms_aWeapons[MissileLauncher].m_bExists = (pDude->m_stockpile.m_sMissileLauncher) ? true : false;
		ms_aWeapons[NapalmLauncher].m_bExists = (pDude->m_stockpile.m_sNapalmLauncher) ? true : false;
		ms_aWeapons[FlameThrower].m_bExists = (pDude->m_stockpile.m_sFlameThrower) ? true : false;

		// Mark those ammos that have gone up
		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			// do this to catch powerups or ammot changes:
			if (ms_aAmmo[i].m_dPrevValue < ms_aAmmo[i].m_dValue)
				{
				ms_aAmmo[i].m_bTreasure = true;
				ms_aAmmo[i].m_lMilli = rspGetMilliseconds();
				}
			}

		// Note that throwing ammo exists if ammo is not zero

		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			ms_aAmmo[i].m_bExists = (ms_aAmmo[i].m_dValue > 0.01);
			ms_aAmmo[i].m_bSelected = FALSE;	// initializing
			}

		// Clear out all selections:
		for (i = NotWeapon + 1; i < NumberOfWeapons; i++)
			{
			ms_aWeapons[i].m_bSelected = FALSE;
			}

		// Which weapon is currently selected?
		// (I guess this is a scheme)
		CDude::WeaponType CurWeapon = pDude->GetCurrentWeapon();

		switch (CurWeapon)
			{
			case CDude::SemiAutomatic:
				ms_aWeapons[MachineGun].m_bSelected = true;
				ms_aAmmo[Bullets].m_bSelected = true;
				break;
			case CDude::ShotGun:
				ms_aWeapons[ShotGun].m_bSelected = true;
				ms_aAmmo[Shells].m_bSelected = true;
				break;
			case CDude::SprayCannon:
				ms_aWeapons[SprayCannon].m_bSelected = true;
				ms_aAmmo[Spray].m_bSelected = true;
				break;
			case CDude::Grenade:
				ms_aAmmo[Grenades].m_bSelected = true;
				break;
			case CDude::Rocket:
				ms_aWeapons[MissileLauncher].m_bSelected = true;
				ms_aAmmo[Rockets].m_bSelected = true;
				break;
			case CDude::Heatseeker:
				ms_aWeapons[MissileLauncher].m_bSelected = true;
				ms_aAmmo[HeatSeekers].m_bSelected = true;
				break;
			case CDude::FireBomb:
				ms_aAmmo[Cocktails].m_bSelected = true;
				break;
			case CDude::Napalm:
				ms_aWeapons[NapalmLauncher].m_bSelected = true;
				ms_aAmmo[Napalm].m_bSelected = true;
				break;
			case CDude::FlameThrower:
				ms_aWeapons[FlameThrower].m_bSelected = true;
				ms_aAmmo[Fuel].m_bSelected = true;
				break;
			case CDude::ProximityMine:
				ms_aAmmo[ProximityMine].m_bSelected = true;
				break;
			case CDude::TimedMine:
				ms_aAmmo[TimedMine].m_bSelected = true;
				break;
			case CDude::BouncingBettyMine:
				ms_aAmmo[BouncingBettyMine].m_bSelected = true;
				break;
			case CDude::DeathWad:
				ms_aWeapons[MachineGun].m_bSelected = true;
				ms_aWeapons[ShotGun].m_bSelected = true;
				ms_aWeapons[SprayCannon].m_bSelected = true;
				ms_aWeapons[MissileLauncher].m_bSelected = true;
				ms_aWeapons[NapalmLauncher].m_bSelected = true;
				ms_aWeapons[FlameThrower].m_bSelected = true;
				ms_aAmmo[Bullets].m_bSelected = true;
				ms_aAmmo[Shells].m_bSelected = true;
				ms_aAmmo[Spray].m_bSelected = true;
				ms_aAmmo[Grenades].m_bSelected = true;
				ms_aAmmo[Rockets].m_bSelected = true;
				ms_aAmmo[HeatSeekers].m_bSelected = true;
				ms_aAmmo[Cocktails].m_bSelected = true;
				ms_aAmmo[Napalm].m_bSelected = true;
				ms_aAmmo[Fuel].m_bSelected = true;
				break;
			case CDude::DoubleBarrel:
				ms_aWeapons[ShotGun].m_bSelected = true;
				ms_aAmmo[Shells].m_bSelected = true;
				break;
			}


		// Light everything that would be selected by this "scheme"
		// Now set all the situation codes for drawing!
		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			if (!ms_aAmmo[i].m_bExists && !ms_aAmmo[i].m_bSelected) 
				ms_aAmmo[i].m_eState = Bar;
			else if (!ms_aAmmo[i].m_bExists && ms_aAmmo[i].m_bSelected) 
				ms_aAmmo[i].m_eState = BarSel;
			else if (ms_aAmmo[i].m_bExists && !ms_aAmmo[i].m_bSelected) 
				ms_aAmmo[i].m_eState = Full;
			else if (ms_aAmmo[i].m_bExists && ms_aAmmo[i].m_bSelected) 
				ms_aAmmo[i].m_eState = FullSel;
			}

		for (i = NotWeapon + 1; i < NumberOfWeapons; i++)
			{
			if (!ms_aWeapons[i].m_bExists && !ms_aWeapons[i].m_bSelected) 
				ms_aWeapons[i].m_eState = Bar;
			else if (!ms_aWeapons[i].m_bExists && ms_aWeapons[i].m_bSelected) 
				ms_aWeapons[i].m_eState = BarSel;
			else if (ms_aWeapons[i].m_bExists && !ms_aWeapons[i].m_bSelected) 
				ms_aWeapons[i].m_eState = Full;
			else if (ms_aWeapons[i].m_bExists && ms_aWeapons[i].m_bSelected) 
				ms_aWeapons[i].m_eState = FullSel;
			}

		//======================== CHECK FOR CHANGE IN AMMO OR STATUS:
		bool bChange = false;

		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			if (ms_aAmmo[i].m_eState != ms_aAmmo[i].m_ePrevState)
				{
				bChange = true;
				break;
				}

			if (ms_aAmmo[i].m_dValue != ms_aAmmo[i].m_dPrevValue)
				{
				bChange = true;
				break;
				}
	
			// This hook is when the tellow numbers become white.
			if (ms_aAmmo[i].m_bTreasure)
				{
				if ( (rspGetMilliseconds() - ms_aAmmo[i].m_lMilli) >
					TB_MILLI_TO_LITE)
					{
					// no longer highlighted:
					bChange = true;
					}
				}

			}

		if (bChange == false)
			{
			for (i = NotWeapon + 1; i < NumberOfWeapons; i++)
				{
				if (ms_aWeapons[i].m_eState != ms_aWeapons[i].m_ePrevState)
					{
					bChange = true;
					break;
					}
				}
			}

		return bChange;
		}

	};
//----------------------------------------------------------------------

// I am hoping that before any instance of a class
// exists, that the staic members must also exist.
CToolItem* CToolItem::ms_aWeapons = NULL;
CToolItem* CToolItem::ms_aAmmo = NULL;
RFont*	CToolItem::ms_pfntTool = NULL;		// General font and print
RPrint	CToolItem::ms_pntTool;
short		CToolItem::ms_sSmallFontColor	=	255;	// color index
short		CToolItem::ms_sLargeFontColor =	255;
short		CToolItem::ms_sWarningColor	=	255;
short		CToolItem::ms_sAmmoGoneColor	=	255;
short		CToolItem::ms_sAttentionColor	=	255;
RImage*	    CToolItem::ms_pimCompositeBuffer  = NULL;
RImage*	    CToolItem::ms_pimCompositeBufferScaled  = NULL;
long		CToolItem::ms_lLastTime = 0;


// Time to arrange the basic bar relationhips:
class	CInitToolBar
	{
public:
	CInitToolBar()
		{
		// arrange the patterns of weapons and ammo:
		CToolItem::ms_aWeapons = new CToolItem[NumberOfWeapons];
		CToolItem::ms_aAmmo = new CToolItem[NumberOfAmmos];

		//************* WEAPONS **************
		CToolItem::ms_aWeapons[MachineGun].ArrangeWeapon(
			MachineGun,CDude::SemiAutomatic,
			RRect(123,443,48,23),
			&CToolItem::ms_aAmmo[Bullets]);

		CToolItem::ms_aWeapons[ShotGun].ArrangeWeapon(
			ShotGun,CDude::ShotGun,
			RRect(176,444,42,19),
			&CToolItem::ms_aAmmo[Shells]);

		CToolItem::ms_aWeapons[SprayCannon].ArrangeWeapon(
			SprayCannon,CDude::SprayCannon,
			RRect(221,442,44,22),
			&CToolItem::ms_aAmmo[Spray]);

		CToolItem::ms_aWeapons[MissileLauncher].ArrangeWeapon(
			MissileLauncher,CDude::Rocket,
			RRect(306,442,54,21),
			&CToolItem::ms_aAmmo[Rockets],
			&CToolItem::ms_aAmmo[HeatSeekers]); // double whammy

		CToolItem::ms_aWeapons[NapalmLauncher].ArrangeWeapon(
			NapalmLauncher,CDude::Napalm,
			RRect(413,441,49,23),
			&CToolItem::ms_aAmmo[Napalm]);

		CToolItem::ms_aWeapons[FlameThrower].ArrangeWeapon(
			FlameThrower,CDude::FlameThrower,
			RRect(464,442,52,22),
			&CToolItem::ms_aAmmo[Fuel]);

		//************** AMMO ****************
		CToolItem::ms_aAmmo[Health].ArrangeAmmo(
			Health,CDude::NoWeapon,
			RRect(0,440,58,40), 
			42,457,
			NULL,
			CToolItem::Large);

		CToolItem::ms_aAmmo[KevlarVest].ArrangeAmmo(
			KevlarVest,CDude::NoWeapon,
			RRect(61,445,35,34),
			97,457,
			NULL,
			CToolItem::Large);
		//-------------------------------------
		CToolItem::ms_aAmmo[Bullets].ArrangeAmmo(
			Bullets,CDude::SemiAutomatic,
			RRect(129,461,16,16),
			149,467,
			&CToolItem::ms_aWeapons[MachineGun],
			CToolItem::Small);

		CToolItem::ms_aAmmo[Shells].ArrangeAmmo(
			Shells,CDude::ShotGun,
			RRect(182,463,16,15),
			202,467,
			&CToolItem::ms_aWeapons[ShotGun],
			CToolItem::Small);

		CToolItem::ms_aAmmo[Spray].ArrangeAmmo(
			Spray,CDude::SprayCannon,
			RRect(228,463,16,15),
			247,467,
			&CToolItem::ms_aWeapons[SprayCannon],
			CToolItem::Small);

		CToolItem::ms_aAmmo[Grenades].ArrangeAmmo(
			Grenades,CDude::Grenade,
			RRect(272,443,19,23),
			276,467,
			NULL,
			CToolItem::Small);

		CToolItem::ms_aAmmo[Rockets].ArrangeAmmo(
			Rockets,CDude::Rocket,
			RRect(295,462,24,17),
			321,467,
			&CToolItem::ms_aWeapons[MissileLauncher],
			CToolItem::Small);

		CToolItem::ms_aAmmo[HeatSeekers].ArrangeAmmo(
			HeatSeekers,CDude::Heatseeker,
			RRect(337,464,26,13),
			364,467,
			&CToolItem::ms_aWeapons[MissileLauncher],
			CToolItem::Small);

		CToolItem::ms_aAmmo[Cocktails].ArrangeAmmo(
			Cocktails,CDude::FireBomb,
			RRect(390,442,20,24),
			394,467,
			NULL,
			CToolItem::Small);

		CToolItem::ms_aAmmo[Napalm].ArrangeAmmo(
			Napalm,CDude::Napalm,
			RRect(414,464,28,13),
			447,466,
			&CToolItem::ms_aWeapons[NapalmLauncher],
			CToolItem::Small);

		CToolItem::ms_aAmmo[Fuel].ArrangeAmmo(
			Fuel,CDude::FlameThrower,
			RRect(476,465,14,13),
			494,466,
			&CToolItem::ms_aWeapons[FlameThrower],
			CToolItem::Small);
		//----------------------------------------
		CToolItem::ms_aAmmo[ProximityMine].ArrangeAmmo(
			ProximityMine,CDude::ProximityMine,
			RRect(567,443,30,23),
			575,467, // redundantly repeated
			NULL,
			CToolItem::Small);

		CToolItem::ms_aAmmo[TimedMine].ArrangeAmmo(
			TimedMine,CDude::TimedMine,
			RRect(599,443,30,26),
			575,467, // redundantly repeated
			NULL,
			CToolItem::Small);

		CToolItem::ms_aAmmo[BouncingBettyMine].ArrangeAmmo(
			BouncingBettyMine,CDude::BouncingBettyMine,
			RRect(538,444,28,28),
			575,467, // redundantly repeated
			NULL,
			CToolItem::Small);
		
		// Now, factor out the bar location from all the coordinates:
		short i;

		for (i = NotWeapon + 1; i < NumberOfWeapons; i++)
			{
			CToolItem::ms_aWeapons[i].m_rImage.sY -= TB_BAR_Y;
			CToolItem::ms_aWeapons[i].m_rText.sY -= TB_BAR_Y;
			CToolItem::ms_aWeapons[i].m_rText.sY += TB_TWEAK_FONT_Y;
			CToolItem::ms_aWeapons[i].m_rText.sX += TB_TWEAK_FONT_X;
			}

		for (i = NotAmmo + 1; i < NumberOfAmmos; i++)
			{
			CToolItem::ms_aAmmo[i].m_rImage.sY -= TB_BAR_Y;
			CToolItem::ms_aAmmo[i].m_rText.sY -= TB_BAR_Y;
			CToolItem::ms_aAmmo[i].m_rText.sY += TB_TWEAK_FONT_Y;
			CToolItem::ms_aAmmo[i].m_rText.sX += TB_TWEAK_FONT_X;
			}

		// Set differect "low" values for each item:
		CToolItem::ms_aAmmo[Health].m_dLow = 20.0;
		CToolItem::ms_aAmmo[KevlarVest].m_dLow = 9.0;
		CToolItem::ms_aAmmo[Shells].m_dLow = 3.0;
		CToolItem::ms_aAmmo[Spray].m_dLow = 15.0;
		CToolItem::ms_aAmmo[Grenades].m_dLow = 2.0;
		CToolItem::ms_aAmmo[Rockets].m_dLow = 2.0;
		CToolItem::ms_aAmmo[HeatSeekers].m_dLow = 2.0;
		CToolItem::ms_aAmmo[Cocktails].m_dLow = 2.0;
		CToolItem::ms_aAmmo[Napalm].m_dLow = 2.0;
		CToolItem::ms_aAmmo[Fuel].m_dLow = 10.0;
		CToolItem::ms_aAmmo[ProximityMine].m_dLow = 2.0;
		CToolItem::ms_aAmmo[BouncingBettyMine].m_dLow = 2.0;
		CToolItem::ms_aAmmo[TimedMine].m_dLow = 1.0;
		}

	~CInitToolBar() 
		{		
		delete [] CToolItem::ms_aWeapons;
		delete [] CToolItem::ms_aAmmo;
		delete CToolItem::ms_pimCompositeBuffer; 
		}

	} DoInitToolBar;

/////////////////////////////////////////
// aliased functions "for her pleasure."
/////////////////////////////////////////
short	ToolBarInit(CHood* pHood)
	{
	return CToolItem::Init(pHood);
	}

bool	ToolBarRender(CHood* pHood,RImage* pimDst,short sDstX,short sDstY,
						  CDude* pDude,bool bForceRender)
	{
	bool bRender = true;

	if (pDude == NULL) 
		{
		TRACE("ToolBarRender: Dude doesn't exist!\n"); 
		return false;
		}

	// First check for minimum interval time:
	if (!bForceRender)	// unless forced to render
		{
		if ( (rspGetMilliseconds() - CToolItem::ms_lLastTime) 
			> TB_MILLI_INTERVAL_TIME)
			{
			CToolItem::ms_lLastTime = rspGetMilliseconds();

			// see if anything has truly changed - if not, don't update:
			bool bRender = false;
			bRender = CToolItem::UpdateStatus(pDude);
			if (bRender) CToolItem::RenderBar(pHood,pimDst,sDstX,sDstY);

			return bRender;
			}
		}
	else
		{
		// force the render
		CToolItem::UpdateStatus(pDude);
		CToolItem::RenderBar(pHood,pimDst,sDstX,sDstY); // don't care about update
		}

	return bRender;
	}

////////////////////////////////////////////////
// Use events which change the state of the bar
////////////////////////////////////////////////

