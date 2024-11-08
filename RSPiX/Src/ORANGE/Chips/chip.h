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
		void SetPosition(int32_t lX, int32_t lY, int32_t lZ);

		// Slide this chip on update to the new location.
		// lRate is the magnitude of the vector directing the
		// chip.
		// Returns 0 if successfully started.
		int16_t Slide(int32_t lX, int32_t lY, int32_t lZ, int32_t lRate);

		// Reset this chip.  Stop sliding, if doing so.
		// Initialize/Release all members.
		void Reset(void);

		// Called to motivate this chip.
		// Returns 0 normally, 1 if chip destroyed.
		int16_t Update(void);

		// Called to draw this chip.
		void Draw(void);

		// Called to motivate all chips.
		static void Critical(void);

		// Destroy all current chips.
		static void DeleteAll(void);

		// Stack chip if necessary.  
		// Returns 0 normally, 1 if chip destroyed.
		int16_t Stack(void);

		// Mark this chip as not stackable if sStack is FALSE.
		void SetStackable(int16_t sStackable)
			{ m_sStackable	= sStackable; }

		// Add to this stack.
		// Returns 0 on success.
		int16_t Add(CChip*	pchip);

		// Remove sNum chips from this stack.
		// Returns chip/stack on success; NULL on error.
		CChip* Sub(int16_t sNum);

		void SetSize(int16_t sNum)
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
		int16_t IsSliding(void)
			{ return m_sSliding; }

		// Returns the first chip found to be colliding
		// with this chip.  If sTop is TRUE, the lowest,
		// the one with the greatest Y, is found.
		CChip* IsColliding(int16_t sTop	= FALSE);

		// Returns size of this stack.
		int16_t GetSize(void)
			{ return m_sNumChips; }

		// Returns the first chip/stack in the given rectangle.
		// Returns NULL if none found.
		static CChip* GetChipIn(int32_t lX, int32_t lY, int32_t lW, int32_t lH);

	protected:	// Internal.
		void Init(void);

	protected:
		RImage*	m_pimChip;		// Pointer to this image's sprite.
		float		m_fX;					// Current x coordinate.
		float		m_fY;					// Current y coordinate.
		float		m_fZ;					// Current z coordinate.
		int16_t		m_sDestX;			// Destination x coordinate.
		int16_t		m_sDestY;			// Destination y coordinate.
		int16_t		m_sDestZ;			// Destination z coordinate.
		int16_t		m_sRate;				// Rate to destination.
		int16_t		m_sSliding;			// Sliding if TRUE.
		int16_t		m_sStackable;		// Stackable if not FALSE.
		int16_t		m_sNumChips;		// If greater than 1, this is
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
