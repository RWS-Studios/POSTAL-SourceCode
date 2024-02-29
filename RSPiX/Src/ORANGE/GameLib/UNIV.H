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
//*****************************************************************************
//
// UNIV.H
//	
//*****************************************************************************

#ifndef _UNIV_H
#define _UNIV_H

#include "ANIMDATA.H"
#include "PAL.H"	  
#include "System.h"
#include "BG.H"
#include "ZONE.H"
#include "event.h"
#include "attrib.h"

#define	VERTICAL		0
#define 	HORIZONTAL	1

#define UNIVERSE_CANT_OPEN_FILE 	1  
#define UNIVERSE_INVALID_WORLD	2
#define UNIVERSE_INVALID_STAGE	3
#define UNIVERSE_INVALID_SECTION	4
#define UNIVERSE_OFFSET_ERROR		5
#define UNIVERSE_INVALID_LEVEL	6
#define UNIVERSE_WRONG_FILETYPE	7

#define UNIVERSE_MAX_FILENAME		32	
#define UNIVERSE_MAX_PATHNAME		512

#define UNIVERSE_MAXSTAGES 5
#define UNIVERSE_MAXWORLDS 5
#define UNIVERSE_MAXSECTIONS 10

#define UNIVERSE_MAXLINE 80

typedef short ORIENT;

// Section Structure.  This structure contains the actual data that is
// needed to load animations and backgrounds for a particular level.  All
// of the other levels are indirection only and are used mainly for logical
// grouping of the levels.  This structure contains pointers to the other
// pieces of data including palettes, animations, backgrounds, and events
// along with some size information.

// Here is a sample of the section data in the file
//stage0sect0
//	da	sprite_pal	;sprite palette
//	da	monday_anim_set_bld	;sprite build data
//	da	monday_pal	;background palette
//	da	meal_bg_set_bld	;background build data
//	dw	0	;bg1 orientation
//	dw	1792	;bg1 width in pixels
//	dw	256	;bg1 height in pixels
//	dw	256	;bg1 horizontal motion ratio*256
//	dw	256	;bg1 vertical motion ratio*256
//	dw	2	;bg1 unit number
//	da	bg_unit0_attribute	;bg1 unit attribute table
//	da	brk1_evt	;bg1 event data

typedef struct tag_SECTION
{
	PALETTE*	pSpritePal;				// sprite palette
	ANIM*		pAnimBuildSet;			// animation data
	PALETTE*	pBackgroundPal;		// background palette
	BG*		pBackgroundBuildSet;	// background data
	ORIENT	orBGOrientation;		// vertical or horizontal world flag
	short		sBGWidth;				// background width in pixels
	short		sBGHeight;				// background height in pixels
	short 	sBGHorzMotionRatio;	//	background horizontal motion ratio*256
	short		sBGVertMotionRatio;	// background vertical motion ratio*256
	ATTRIB*	pAttribute;				// background attribute data
	EVENT*	pEvent;					// background event data
} SECTION;


// Stage Structure.  The first word in the struct gives the number of
// sections in the stage.  File pointer offsets to each section follow which
// will be resolved to pointers when the data is loaded

// Here is a sample from the file
//stage0
//	dw	1
//	da	stage0sect0

typedef struct tag_STAGE
{
	short sNumSections;								// Number of sections in this stage
	SECTION* apSections[UNIVERSE_MAXSECTIONS];// SECTION's (max set by linker)
} STAGE;


// World Structure.  The first word in the struct gives the number of stages
// in this world.  File pointer offsets to each stage follow which will
// be resolved to pointers when the data is loaded.

// Here is a sample from the file
//World0
//	dw	3
//	da	stage0
//	da	stage1
//	da	stage2

typedef struct tag_WORLD
{
 	short sNumStages;							// Number of stages in this world
	STAGE* apStages[UNIVERSE_MAXSTAGES];// STAGE's (max set by linker)
} WORLD;


// Universe Structure.  The first word in the struct gives the number of
// worlds in the game, file pointer offsets to each world follow which
// will be resolved to pointers when the data is loaded in.

// Here is a sample from the file
//Universe
//	dw 2
//	da	world0
//	da world1

typedef struct tag_UNIVERSE
{
 	short sNumWorlds;							// Number of worlds in the game
	WORLD* apWorlds[UNIVERSE_MAXWORLDS];// WORLD's (max set by linker)
} UNIVERSE;
	

class CUniverse
{
	private:
		char	m_strFileGame[UNIVERSE_MAX_FILENAME];	// Game Info data file
		char	m_strFileAnim[UNIVERSE_MAX_FILENAME];	// Animation build data
		char	m_strFileImage[UNIVERSE_MAX_FILENAME]; // Animation Images
		char	m_strFileZone[UNIVERSE_MAX_FILENAME];	// Zoneset data
		char	m_strFileBack[UNIVERSE_MAX_FILENAME];	// Background build data
		char	m_strFileAPal[UNIVERSE_MAX_FILENAME];	// Animation Palette
		char	m_strFileBPal[UNIVERSE_MAX_FILENAME];	// Background Palette
		char	m_strFileAttr[UNIVERSE_MAX_FILENAME];	// Background Attribute data
		char	m_strFileEvent[UNIVERSE_MAX_FILENAME];	// Background Event data
		char  m_strDataPath[UNIVERSE_MAX_PATHNAME];	// Base path for all data files

		short LoadAnimData();		// private function to load animation file
		short LoadBackgroundData();// private function to load background file
		short LoadAttributeData();	// private function to load attribute file
		short LoadEventData();		// private function to load event file
		PALETTE* LoadPalette(LPSTR);//private function to load anim/bg palette
		void SetAnimOffsets(ANIM*);// resolve Anim offset to pointers
		void SetZonesetOffsets(ZONESET*); // resolve Zoneset offsets to pointers
		
 	protected:
		WORLD*	m_pWorld;			// ??
		STAGE* 	m_pStage;			// ??
		SECTION* m_pSection;			// ??Pointer to the current section structure

		ANIMSET*	m_pAnimData;		// Pointer to Animation build data
		BG*		m_pBackData;		// Pointer to Background build data
		PALETTE*	m_pSpritePal;		// Pointer to sprite palette
		PALETTE* m_pBackgroundPal;	// Pointer to background palette
		WORD*		m_pAttrMap;			// Pointer to background attribute map
		short		m_sMapWidth;		// Width of attribute map
		CONTOUR*	m_pAttrCont;		// Pointer to arrays of attribute contours
		EVENT*	m_pEventData;		// Pointer to background event data
		IMAGE*	m_pImageData;		// Pointer to animation images


	public:
		CUniverse();
		~CUniverse();

		// This function loads in a SECTION structure from the game info file.
		// Once this function has been called, other modules such as Anim can
		// call the other member access function to get pointers to the data.
		short SetSection(			// Load a section structure from file to memory
			short sWorld,			// World number
			short sStage,			// Stage number
			short sSection			// Section number
			);

		// These access functions return pointers to various parts of the
		// SECTION structure.

		ANIMSET*	GetAnim() {return m_pAnimData;};
		BG*		GetBG() {return m_pBackData;};				 // Background data
		PALETTE* GetAnimPal() {return m_pSpritePal;};		 // Animation palette
		PALETTE* GetBGPal() {return m_pBackgroundPal;};		 // Background palette
		EVENT*	GetEvents() {return m_pEventData;};			 // Event data
		void  	GetAttributes(WORD* pwMap, 		 			 // Attribute data
		                       CONTOUR* pContour,
									  short* psWidth)
		 			{
		 				pwMap = m_pAttrMap;
						pContour = (CONTOUR*) m_pAttrCont;
						*psWidth = m_sMapWidth;
					};


};

#endif //_UNIV_H

//*****************************************************************************
// EOF
//*****************************************************************************




