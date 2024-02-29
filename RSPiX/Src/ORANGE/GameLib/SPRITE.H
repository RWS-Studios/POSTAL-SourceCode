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
///////////////////////////////////////////////////////////////////////////////
//
// SPRITE.H
//
//
// History:
//
//		10/31/96 BRH	Changed CSprite to RSprite to match new RSPiX
//							naming convention.  Also changed references to
//							other RSPiX classes to use the R prefix.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SPRITE_H
#define SPRITE_H

#include "System.h"

// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	// Green include files
	#include "GREEN/Image/Image.h"
	// Orange include files
	#include "ORANGE/GameLib/Region.h"
	#include "ORANGE/CDT/List.h"
	#include "ORANGE/CDT/slist.h"
#else
	// Green include files
	#include "Image.h"
	// Orange include files
	#include "region.h"
	#include "list.h"
	#include "slist.h"
#endif // PATHS_IN_INCLUDES


// These values are used to identify the sprite file
#define SPRITE_COOKIE				0x54525053	// Looks like "SPRT" in file
#define SPRITE_CURRENT_VERSION	3				// Change this whenever you must
															// change the file format

// These masks are used with the sprite flags
#define SPRITE_FLAG_VISIBLE		0x0001	// 1=visible, 0=hidden 
#define SPRITE_FLAG_MODIFIED		0x0100	// 1=modified, 0=not
#define SPRITE_FLAG_RENDERED		0x0200	// 1=rendered, 0=not yet
// These masks are used with the collision flags.
// (THESE ARE SAMPLES -- GET REAL ONES FROM SNES/GENESIS LIBRARY!)
#define SPRITE_CX_TOP				0x0001
#define SPRITE_CX_BOTTOM			0x0002
#define SPRITE_CX_LEFT				0x0004
#define SPRITE_CX_RIGHT				0x0008

#define SPRITE_LOWPRI				80			// Lowest priority between sprites


// Define the pre-display callback function.  This user-definable function
// is called from DisplayAll() before the sprites are actually displayed.
// The callback pointer defaults to NULL, and is automatically called if
// it is any value except NULL.
class RSprite;	// Forward declaration of class
typedef void (*SPRITE_PREDISP)(RSprite* psprite);


class RSprite
{
	public:
		short		m_sX;					// Sprite X
 		short 	m_sY;					// Sprite Y
		short		m_sZ;					// Sprite Z
		short		m_sHotSpotX;		// The offset from m_sX to the hotspot.
											// This value would normally be subtracted from
											// m_sX to get the destination (e.g., screen)
											// position.
		short		m_sHotSpotY;		// The offset from m_sY to the hotspot.
											// This value would normally be subtracted from
											// m_sY to get the destination (e.g., screen)
											// position.
		short		m_sHotSpotZ;		// The offset from m_sZ to the hotspot.
											// This value would normally be subtracted from
											// m_sZ to get the destination (e.g., screen)
											// position.
		short		m_sAngle;			// Angle of rotation
		long		m_lWidth;			// Sprite's Desired Display Width (for scale blit)
		long		m_lHeight;			// Sprite's Desired Display Height (for scale blit)
		double	m_dX;					// Sprite X as a double (for motion)
		double	m_dY;					// Sprite Y as a double (for motion)
		double	m_dZ;					// Sprite Z as a double (for motion)
		double	m_dXacc;				// X Acceleration
		double	m_dYacc;				// Y Acceleration
		double	m_dZacc;				// Z Acceleration
		double	m_dXvel;				// X Velocity
		double	m_dYvel;				// Y Velocity
		double	m_dZvel;				// Z Velocity
		ULONG		m_ulFlags;			// Attribute and status flags
		RImage*	m_pImage;			// Image
		RList<RRegion> m_clRegions;// List of regions
		SPRITE_PREDISP	m_cbPreDisp;// Pre-display callback function
		short		m_sOwnImage;		// TRUE if image was allocated by us; FALSE, otherwise.
											// Basically, if this flag is TRUE, this will delete
											// m_pImage when ~RSprite() is called.
		
		static RSList<RSprite, short> SpriteList;

	public:
		// Default constructor
		RSprite();

		// Constructor for image and flags
		RSprite(RImage* pImage, ULONG ulFlags = 0);

		// Constructor for complete initialize
		RSprite(short sX, short sY, short sZ, short sAngle, long lWidth, long lHeight,
		        ULONG ulFlags, RImage* pImage);

		// Does basic default initialization for RSprite.
		// Should not be called after CreateImage() unless DestroyImage() is called
		// or special care is taken with the *m_pImage data.
		void Init(void);

		// Allocates an image pointed to by m_pImage.  Use DestroyImage() to
		// destroy.
		short CreateImage(void);	// Returns 0 on success.

		// Destroys an image previously allocated by CreateImage().  If m_pImage
		// is NULL or m_sOwnImage is FALSE, this function does nothing.
		void DestroyImage(void);	// Returns nothing.

		// Save sprite information and its image.  This version of the save
		// takes a filename and creates a new file.  Note:  If the image
		// format cannot be saved directly, the image will be converted
		// to one of the standard types and the Destination type will be
		// set to reconvert it to its original format upon load.
		short Save(char* pszFilename);

		// Save sprite information and its image.  This version of the save
		// takes an open RFile poniter and writes to the file.  This can be
		// used to include a sprite within another file.
		short Save(RFile* pcf);

		// Load sprite information and image.  This version of the load takes
		// a filename and opens that file to read the information.  
		short Load(char* pszFilename);

		// Load sprite information and image.  This version of the load takes
		// an open RFile ponter and reads the data from the current position
		// in the file.
		short Load(RFile* pcf);

		// Destructor
		virtual ~RSprite();

		// Mark sprite as modified
		virtual void Modified() {m_ulFlags |= SPRITE_FLAG_MODIFIED;};

		// Check for zone collisions
 		virtual short DoRegionsCollide(// Returns 1 if collision, 0 otherwise
			short sThisZoneType,			// type of zone for this sprite
			short sOtherZoneType,		// zone type to check
			RSprite* pSprite);			// other sprite

		// Get details about collision.  Use the predefined COLLIDE_??? masks
		// with the return value to determine details about the collision.
		virtual ULONG GetCxFlags() {return m_ulFlags;};	// Returns detailed collision flags

		// Add a region to this sprite's region list
		virtual short AddRegion(RRegion*);

		// Remove a region from this sprite's region list
		virtual short RemoveRegion(RRegion*);

		// Set the X, Y, Z position of this sprite
		void SetPosition(short sX, short sY, short sZ)
			{m_sX = sX; m_sY = sY; m_sZ = sZ;};

		// Set the angle of rotation for this sprite
		void SetAngle(short sAngle)
			{m_sAngle = sAngle;};

		// Set the desired size of the sprite
		void SetSize(long lWidth, long lHeight)
			{m_lWidth = lWidth; m_lHeight = lHeight;};

		// Set acceleration for 3D movement
		void SetAcceleration(double dX, double dY, double dZ)
			{m_dXacc = dX; m_dYacc = dY; m_dZacc = dZ;};

		// Set acceleration for 2D movement
		void SetAcceleration(double dX, double dY)
			{m_dXacc = dX; m_dYacc = dY;};

		// Set velocity for 3D movement
		void SetVelocity(double dX, double dY, double dZ)
			{m_dXvel = dX; m_dYvel = dY; m_dZvel = dZ;};

		// Set velocity for 2D movement
		void SetVelocity(double dX, double dY)
			{m_dXvel = dX; m_dYvel = dY;};

		// Update this sprite and/or descendent.  This is purely a hook.
		// No default processing is provided by this level.  When descending
		// a class from this object, overriding this is a good way to grab 
		// control from a RSprite.
		// Provides callability from this object to higher layers.
		virtual short Update(void)	// Returns user defined value or 0.
			{ return 0; }

	private:
		// Add the sprite to the static list of sprites
		short AddSpriteToList();
};

#endif //SPRITE_H

//*****************************************************************************
// EOF
//*****************************************************************************
