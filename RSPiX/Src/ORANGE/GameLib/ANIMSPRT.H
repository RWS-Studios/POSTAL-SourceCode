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
//	ANIMSPRT.H
//   
//	Created on 		3/15/95 BRH 
// Implemented on 3/21/95 BRH
//
// The RAnimSprite class is derived from the Sprite class.  It adds
//	animation capabilities to the basic sprite.
//
//	History:
//
//		10/31/96 BRH	Changed names from CAnimSprite to RAnimSprite for
//							the new RSPiX naming convention.
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CImage			RImage
//
//////////////////////////////////////////////////////////////////////
#ifndef animsprt_h
#define animsprt_h

#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	// Orange includes
	#include "ORANGE/GameLib/SPRITE.H"
	#include "ORANGE/File/file.h"
#else
	// Orange includes
	#include "sprite.h"
	#include "file.h"
#endif // PATHS_IN_INCLUDES

#ifdef _MSC_VER
	#pragma warning (disable:4200) // disable warning about zero-length arrays
#endif

// These masks are used with the animation flags
#define ANIMSPRITE_FLAG_LOOP		0x0001	// 1=loop anim, 0=stop at end
#define ANIMSPRITE_FLAG_NOSKIP	0x0002	// 1=can't skip frames, 0=can
#define ANIMSPRITE_FLAG_OFFSET	0x0004	// 1=add anim offsets to sprite
#define ANIMSPRITE_FLAG_ROTATION	0x0008	// 1=modify rotation of sprite
#define ANIMSPRITE_FLAG_SCALE		0x0010	// 1=modify sprite's scale
#define ANIMSPRITE_FLAG_END		0x0100	// 1=ending frame, 0=other
#define ANIMSPRITE_FLAG_KEY		0x0200	// 1=key frame, 0=other

#define ANIMSPRITE_LAST_FRAME		-1
#define ANIMSPRITE_FIRST_FRAME	-2
#define ANIMSPRITE_WAITING			-3

#define ANIMSPRITE_CURRENT_VERSION	0
#define ANIMSPRITE_COOKIE				0x4d494e41	//"ANIM" in the file

// Error codes
#define ANIM_INVALID_FRAME			1			// Current anim does not contain 

// A FRAME contains offsets that describe how the hotspot should be moved
// in order to get to the position at which the specified IMAGE should be
// displayed.  The "hold" value tells how many milliseconds this FRAME should
// be held for before moving on to the next frame.  The sPicIndex is stored
// in the animation file to associate this frame with one of the animation's
// pictures.  Once the pictures are loaded, the pImage pointer is set to the
// image for this frame.  This way multiple frames can use the same picture.
typedef struct tag_FRAME
{
	short		sOffsetX;	// amount to add to sprite's X position for this frame
	short		sOffsetY;	// amount to add to sprite's Y position for this frame
	short		sOffsetZ;	// amount to add to sprite's Z position for this frame
	short		sRotDeg;		// Rotation in Degrees
	long		lScaleWidth;// Scaled width for this frame
	long		lScaleHeight;//Scaled height for this frame
	ULONG		ulFlags;		// Flags for this frame
	short		sHold;		// number of milliseconds to hold this frame
	short		sPicIndex;	// Animation's picture index for this frame (used to load)
	RImage*	pImage;		// pointer to the image for this frame
} FRAME;
										

class RAnimSprite : public RSprite
{
	private:
		short		m_sVersion;			// Animation version number (to mark .ani files)
		short		m_sAllocatedPics;	 // Keep track of number allocated (user can't change)
	public:							// so Load can check the header format
		short		m_sNumFrames;		// Number of frames in this animation
		short		m_sNumPictures;	// Number of RImages in this animation
		short		m_sLoopToFrame;	// -1 if no loop, or frame number to start loop
		ULONG		m_ulAnimFlags;		// Animation flags and status
		long		m_lTimer;			// timer used for animating
		short		m_sCurrFrame;		// current frame in the animation
		FRAME*	m_aFrames;			// Array of frame structures
		RImage**	m_apPictures;		// Array of RImage*

	public:
		RAnimSprite();
		~RAnimSprite();

		// Load an animation from a .ani file with this filename
		short Load(char* pszFilename);

		// Load an animation from an open RFile
		short Load(RFile* pcf);

		// Save this animation to a .ani file and give it this filename
		short Save(char* pszFilename);

		// Save this animation to an open RFile
		short Save(RFile* pcf);

		// Go to the next frame of animation if it is time
		short Animate(void);

		// Set to specified frame in current animation.
		// Updates m_sAnimFrameNum, m_sAnimFrameNum and m_usAnimFlags.
 		// NOTE: If frame is too high then ending frame is used.
		// NOTE: A sequential search for specified frame is required!
		// NOTE: If ANIMSPRITE_FLAGS_NOSKIP flag is set, nearest key-frame
		// is used (nearest before, nearest after, or just nearest?)
		short SetFrame(short sFrameNum);	

		// Go on to next frame of current anim.  If anim was on ending frame
		// then this will wrap back around to frame 1 (not frame 0!)
		// Updates m_sAnimFrameNum, m_sAnimFrameNum and m_usAnimFlags.
		short NextFrame(void);

		// Go on to next key frame of current anim.  If anim was on ending frame
		// then this will wrap back around to frame 1 (not frame 0!)
		// Updates m_sAnimFrameNum, m_sAnimFrameNum and m_usAnimFlags.
		short NextKeyFrame(void);

		// Go on to previous frame of current anim.  If anim was on first frame
		// then this will wrap around to the end frame
		short PrevFrame(void);

		// Function to allocate space for given number of frames
		short AllocateFrames(short sNumFrames);

		// Function to allocate space for given number of RImage pointers
		short AllocatePictures(short sNumPictures);

		// Function to free frames if you allocated them
		short FreeFrames(void);

		// Function to free pictues if you allocated them
		short FreePictures(void);

		// Return a pointer to the current frame's RImage
		RImage* GetFrameImage(void)
		{
			if (m_sCurrFrame >= 0 && m_sCurrFrame < m_sNumFrames)
				return m_aFrames[m_sCurrFrame].pImage;
			else
				return NULL;
		};
		
		// Return a pointer to a specified frame's RImage
		RImage* GetFrameImage(short sFrameNumber)
		{
			if (sFrameNumber >= 0 && sFrameNumber < m_sNumFrames)
				return m_aFrames[sFrameNumber].pImage;
			else
				return NULL;
		}
		
		// Return a pointer to the current frame
		FRAME* GetFrame(void)
		{
			return &(m_aFrames[m_sCurrFrame]);
		};

		// Get the current frame's hold time
		short GetFrameTime(void)
		{
			return m_aFrames[m_sCurrFrame].sHold;
		};

	private:
		// Function used by Save to loop through and save all of the images
		short WritePictures(RFile* pcf);

		// Function used by Save to loop through and save all of the frames
		short WriteFrames(RFile* pcf);

		// Function used by Load to loop through and load all of the images
		short ReadPictures(RFile* pcf);

		// Function used by Load to loop through and load all of the frames
		short ReadFrames(RFile* pcf);
};

#endif //animsprt_h

//*****************************************************************************
// EOF
//*****************************************************************************
