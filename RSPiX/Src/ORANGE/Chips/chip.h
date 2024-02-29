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
//////////////////////////////////////////////////////////////////////
// Blue Includes.
//////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////
// Green Includes.
//////////////////////////////////////////////////////////////////////
//#include "BLiT/blit2d/blimage.h"
//#include "cdt/slist.h"

//////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////

class CChip
	{
	public:	// Con/Destruction.
		// Default constructor.
		CChip();
		// Destructor.
		~CChip();

	public:	// Methods.
		// Set an image for this chip.
		void SetChipImage(RImage* pim)
			{ m_pimChip = pim; }

		// Set background BKD for all chips.
		static void SetBackground(RImage*	pimbkd)
			{ ms_pimView	= pimbkd; }

		// Set stack image for all chips.
		static void SetStackImage(RImage* pim)
			{ ms_pimStack	= pim; }

		// Move this chip to the new location.
		void SetPosition(long lX, long lY, long lZ);

		// Slide this chip on update to the new location.
		// lRate is the magnitude of the vector directing the
		// chip.
		// Returns 0 if successfully started.
		short Slide(long lX, long lY, long lZ, long lRate);

		// Reset this chip.  Stop sliding, if doing so.
		// Initialize/Release all members.
		void Reset(void);

		// Called to motivate this chip.
		// Returns 0 normally, 1 if chip destroyed.
		short Update(void);

		// Called to draw this chip.
		void Draw(void);

		// Called to motivate all chips.
		static void Critical(void);

		// Destroy all current chips.
		static void DeleteAll(void);

		// Stack chip if necessary.  
		// Returns 0 normally, 1 if chip destroyed.
		short Stack(void);

		// Mark this chip as not stackable if sStack is FALSE.
		void SetStackable(short sStackable)
			{ m_sStackable	= sStackable; }

		// Add to this stack.
		// Returns 0 on success.
		short Add(CChip*	pchip);

		// Remove sNum chips from this stack.
		// Returns chip/stack on success; NULL on error.
		CChip* Sub(short sNum);

		void SetSize(short sNum)
			{	m_sNumChips	= sNum; }

	public:	// Querries.
		// Get the sprite for this chip.
		RImage*	GetChipImage(void)
			{ return m_pimChip; }

		// Returns pointer to background BKD for all chips.
		static RImage* GetBackground(void)
			{ return ms_pimView; }

		// Returns pointer to stack image for all chips.
		static RImage* GetStackImage(void)
			{ return ms_pimStack; }

		// TRUE if sliding, FALSE otherwise.
		short IsSliding(void)
			{ return m_sSliding; }

		// Returns the first chip found to be colliding
		// with this chip.  If sTop is TRUE, the lowest,
		// the one with the greatest Y, is found.
		CChip* IsColliding(short sTop	= FALSE);

		// Returns size of this stack.
		short GetSize(void)
			{ return m_sNumChips; }

		// Returns the first chip/stack in the given rectangle.
		// Returns NULL if none found.
		static CChip* GetChipIn(long lX, long lY, long lW, long lH);

	protected:	// Internal.
		void Init(void);

	protected:
		RImage*	m_pimChip;		// Pointer to this image's sprite.
		float		m_fX;					// Current x coordinate.
		float		m_fY;					// Current y coordinate.
		float		m_fZ;					// Current z coordinate.
		short		m_sDestX;			// Destination x coordinate.
		short		m_sDestY;			// Destination y coordinate.
		short		m_sDestZ;			// Destination z coordinate.
		short		m_sRate;				// Rate to destination.
		short		m_sSliding;			// Sliding if TRUE.
		short		m_sStackable;		// Stackable if not FALSE.
		short		m_sNumChips;		// If greater than 1, this is
											// a stack.

		static RSList<CChip, float>	ms_slistChips;	// List of all 
																	// chips sorted by
																	// each chip's Z
																	// position.

		static RImage*	ms_pimView;		// Background for all chips.

		static RImage*	ms_pimStack;	// Stack for all chips.
	};

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
