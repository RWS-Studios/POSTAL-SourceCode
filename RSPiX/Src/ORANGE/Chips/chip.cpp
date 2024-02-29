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
//
//	CHIP.CPP
//   
//	Created on 1/31/96	JMI 
//
//////////////////////////////////////////////////////////////////////
//
// Container class for a chip's sprite (x, y, z, etc.).
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Blue Includes.
//////////////////////////////////////////////////////////////////////
//#include "common/system.h"

//////////////////////////////////////////////////////////////////////
// Green Includes.
//////////////////////////////////////////////////////////////////////
//#include "BLiT/blit2d/blimage.h"
#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////
// Orange Includes.
//////////////////////////////////////////////////////////////////////
//#include "cdt/queue.h"
#include "chip.h"

//////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////
#define MAX_CHIPS	4096

#define BASE		(m_pimChip->lHeight)
#define THICKNESS	0.50F

//////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Module specific (static) vars.
//////////////////////////////////////////////////////////////////////
RSList<CChip, float>	CChip::ms_slistChips;	// List of all 
															// chips sorted by
															// each chip's Z
															// position.
RImage*	CChip::ms_pimView		= NULL;	// Background for all chips.
RImage*	CChip::ms_pimStack		= NULL;	// Stack for all chips.

//////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Con/Destruction.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Default constructor.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////
CChip::CChip()
	{
	Init();

	if (ms_slistChips.Insert(this, &m_fZ) == 0)
		{
		}
	else
		{
		TRACE("CChip(): Unable to Insert this into list.\n");
		}
	}

//////////////////////////////////////////////////////////////////////
//
// Destructor.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////
CChip::~CChip()
	{
	Reset();

	if (ms_slistChips.Remove(this) == 0)
		{
		}
	else
		{
		TRACE("CChip(): Unable to Remove this from list.\n");
		}
	}

//////////////////////////////////////////////////////////////////////
// Methods.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Move this chip to the new location, erasing the old.
// Returns nothing.
//
//////////////////////////////////////////////////////////////////////
void CChip::SetPosition(long lX, long lY, long lZ)
	{
	float	fOldZ	= m_fZ;

	m_fX	= (float)lX;
	m_fY	= (float)lY;
	m_fZ	= (float)lZ;

	// If the z position changed . . .
	if (fOldZ != m_fZ)
		{
		// We must reposition this item.
		ms_slistChips.Reposition(this);
		}
	}

//////////////////////////////////////////////////////////////////////
//
// Slide this chip on update to the new location.
// lRate is the magnitude of the vector directing the
// chip.
// You may call this when the chip is already sliding to redirect it.
// Returns 0 if successfully started.
//
//////////////////////////////////////////////////////////////////////
short CChip::Slide(long lX, long lY, long lZ, long lRate)
	{
	short	sRes	= 0;	// Assume success.

	// Store destination position.
	m_sDestX	= (short)lX;
	m_sDestY	= (short)lY;
	m_sDestZ	= (short)lZ;

	m_sRate	= (short)lRate;

	// Success.
	m_sSliding	= TRUE;

	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// Reset this chip.  Stop sliding, if doing so.
// Initialize/Release all members.
//
//////////////////////////////////////////////////////////////////////
void CChip::Reset(void)
	{
	Init();
	}

//////////////////////////////////////////////////////////////////////
//
// Update movement.
// Returns 0 normally, 1 if chip destroyed.
//
//////////////////////////////////////////////////////////////////////
short CChip::Update(void)
	{
	short	sDeleted	= 0;	// Assume normal ops.

	if (m_sSliding == TRUE)
		{
		m_fX	+= ((float)m_sDestX - m_fX) / (float)m_sRate;
		m_fY	+= ((float)m_sDestY - m_fY) / (float)m_sRate;
		m_fZ	+= ((float)m_sDestZ - m_fZ) / (float)m_sRate;

		m_sRate--;
		ASSERT(m_sRate >= 0);

		if (	(short)m_fX == m_sDestX
			&&	(short)m_fY	== m_sDestY
			&& (short)m_fZ	== m_sDestZ)
			{
			// Done.
			m_sSliding	= FALSE;
			// Stack if necessary.
			sDeleted	= Stack();
			}
		}

	return sDeleted;
	}


//////////////////////////////////////////////////////////////////////
//
// Called to draw this chip.
//
//////////////////////////////////////////////////////////////////////
void CChip::Draw(void)
	{
	ASSERT(ms_pimView != NULL);
	ASSERT(m_pimChip != NULL);
	
	// If this is merely a chip . . .
	if (m_sNumChips == 1)
		{
		// Put'er there!  NOTE: ItoBKD is a real time macro containing 
		// merely 2 dereferences, worst case.
		rspBlit(m_pimChip, ms_pimView, m_fX, m_fY);
//		BLT_FItoBKD(m_pimChip, ms_pimView, (short)m_fX, (short)m_fY);
		}
	else
		{
		// Otherwise, display a stack.
		short	sHeight	= (short)(THICKNESS * (float)GetSize());
		
		// The smallest stack should look like a stack.
		if (sHeight < 1)
			{
			sHeight = 1;
			}

		for (short	s = 0; s < sHeight; s++)
			{
//			BLT_FItoBKD(m_pimChip, ms_pimView, (short)m_fX, (short)m_fY - s);
			rspBlit(m_pimChip, ms_pimView, m_fX, m_fY - s);
			}
		}
	}

//////////////////////////////////////////////////////////////////////
//
// Called to update all sliding chips.
// Technically, we should go through the list twice.  Once to move the
// chips and rearrange them in the list and once to blt them.
// We cannot rearrange them as we go through the list or we could end
// up processing a chip twice or not at all.  This indicates going
// through the list three times:
// 1) Move chips and set a flag to rearrange.
// 2) Rearrange.
// 3) Blt.
// OR
// we could go through the list once adding all the ones to be re-
// arranged to a queue.  Reprocess only those, rearranging as 
// necessary and then traverse the list and blt:
// 1) Move chips and add those with new Z positions to a queue.
// 2) Empty queue rearranging queued chips only.
// 3) Blt chips.
//
//////////////////////////////////////////////////////////////////////
void CChip::Critical(void)
	{
	RQueue<CChip, MAX_CHIPS>	qChips;

	// Step 1: Traverse the Z sorted list updating all chips and 
	// adding those that change their Z position to a queue of chips
	// to be repositioned.
	CChip*	pchip	= ms_slistChips.GetHead();
	float		fOldZ;
	while (pchip != NULL)
		{
		fOldZ	= pchip->m_fZ;

		// If not deleted during update . . .
		if (pchip->Update() == 0)
			{
			// If this chip changed its Z position . . .
			if (fOldZ != pchip->m_fZ)
				{
				// Add it to the queue of chips to be resorted.
				qChips.EnQ(pchip);
				}
			}

		// Get the next chip to move.
		pchip	= ms_slistChips.GetNext();
		}

	// Step 2: Empty the queue resorting each chip.
	pchip	= qChips.DeQ();
	while (pchip != NULL)
		{
		// Reposition this chip in the sorted list since its Z position
		// changed.
		ms_slistChips.Reposition(pchip);

		pchip = qChips.DeQ();
		}

	// Step 3: Blt the chips in order.
	pchip	= ms_slistChips.GetHead();
	while (pchip != NULL)
		{
		pchip->Draw();

		pchip = ms_slistChips.GetNext();
		}

	}

//////////////////////////////////////////////////////////////////////
//
// Destroy all current chips.
//
//////////////////////////////////////////////////////////////////////
void CChip::DeleteAll(void)
	{
	CChip*	pchip	= ms_slistChips.GetHead();

	while (pchip != NULL)
		{
		// Someday a chip may have data to allocate.
		pchip->Reset();

		// Destructor pulls off of list.
		delete pchip;

		pchip	= ms_slistChips.GetNext();
		}
	}

//////////////////////////////////////////////////////////////////////
//
// Stack chip if necessary. 
// Returns 0 normally, 1 if chip destroyed.
//
//////////////////////////////////////////////////////////////////////
short CChip::Stack(void)
	{
	short	sDeleted	= 0;	// Assume normal ops.

	if (m_sStackable != FALSE)
		{
		// If we are near a chip that's not sliding . . .
		CChip* pchip	= IsColliding(FALSE);
		if (pchip != NULL)
			{
			// Add this chip/stack to that stack.
			pchip->Add(this);

			sDeleted	= TRUE;

			// It's important that we delete this chip and
			// add to the other as, if we are in one of
			// the Critical loops, Remove()ing another chip
			// from the list of chips will make that chip
			// the current and screw up the loop's next call
			// to GetNext().
			// Destroy this chip (it is reflected in the new
			// size of pchip's stack).
			delete this;
			}
		}

	return sDeleted;
	}

//////////////////////////////////////////////////////////////////////
//
// Add to this stack.
// Returns 0 on success.
//
//////////////////////////////////////////////////////////////////////
short CChip::Add(CChip*	pchip)
	{
	short	sRes	= 0;	// Assume success.

	// Take stack's size and add to this one's.
	m_sNumChips += pchip->GetSize();

	return sRes;
	}

//////////////////////////////////////////////////////////////////////
//
// Remove sNum chips from this stack.
// Returns chip/stack on success; NULL on error.
//
//////////////////////////////////////////////////////////////////////
CChip* CChip::Sub(short sNum)
	{
	CChip*	pchip	= NULL;	// Assume failure.

	// If we have sNum to give up . . .
	if (sNum < GetSize())
		{
		// Create a new chip/stack for these.
		pchip	= new CChip;
		// Make it empty for now.
		pchip->SetSize(0);
		}
	else
		{
		if (sNum == GetSize())
			{
			// Use this chip.
			pchip	= this;
			}
		}

	// If successful . . . 
	if (pchip != NULL)
		{
		// Add to new stack.
		pchip->SetSize(pchip->GetSize() + sNum);
		// Remove from this.
		SetSize(GetSize() - sNum);

		// Some attributes that should be copied.
		pchip->SetPosition((long)m_fX, (long)m_fY, (long)m_fZ);
		pchip->SetChipImage(GetChipImage());
		}

	return pchip;
	}

//////////////////////////////////////////////////////////////////////
// Querries.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Returns the first chip found to be colliding
// with this chip.  If sTop is TRUE, the highest,
// the one with the greatest Y, is found.
//
//////////////////////////////////////////////////////////////////////
CChip* CChip::IsColliding(short sTop/*	= FALSE*/)
	{
	CChip*	pchip	= NULL;				// Assume not found.
	CChip*	pchipHighest	= NULL;	// If sTop is TRUE, this is used
												// to store the chip with the
												// lowest Y.

	for (short i = 0; i < 2 && (pchip == NULL || sTop == TRUE); i++)
		{
		// Search back from this one . . .
		pchip	= (i == 0	? ms_slistChips.GetPrev(this) 
								: ms_slistChips.GetNext(this));

		while (pchip != NULL && pchip != this)
			{
			// Is it outside the Z range?
			// NOTE: If this check stops the search, it can screw up in the
			// event that another chip has a different size.
			if (	pchip->m_fZ + (float)pchip->m_pimChip->m_sHeight < m_fZ
				||	pchip->m_fZ > m_fZ + (float)m_pimChip->m_sHeight)
				{
				// Outside.
				pchip	= NULL;
				}
			else
				{
				// Is it outside the X range?
				// NOTE: this check has the same limitation as above.
				if (	pchip->m_fX + pchip->m_pimChip->m_sWidth < m_fX
					||	pchip->m_fX > m_fX + (float)m_pimChip->m_sWidth)
					{
					// Outside.
					}
				else
					{
					if (pchip->m_sStackable != FALSE)
						{
						// If not looking for the highest . . .
						if (sTop == FALSE)
							{
							// We're done.
							break;
							}
						else
							{
							if (pchipHighest == NULL)
								{
								pchipHighest	= pchip;
								}
							else
								{
								if (pchip->m_fY < pchipHighest->m_fY)
									{
									pchipHighest	= pchip;
									}
								}
							}
						}
					else
						{
						// Can't use a stackable chip.
						}
					}

				pchip = (i == 0	? ms_slistChips.GetPrev()
										: ms_slistChips.GetNext() );
				}
			}
		}


	// If we're searching for the highest and we found one . . .
	if (sTop == TRUE && pchipHighest != NULL)
		{
		pchip = pchipHighest;
		}

	return pchip;
	}

//////////////////////////////////////////////////////////////////////
//
// Returns the first chip/stack in the given rectangle.
// Returns NULL if none found.
// (static)
//
//////////////////////////////////////////////////////////////////////
CChip* CChip::GetChipIn(long lX, long lY, long lW, long lH)
	{
	CChip*	pchip	= ms_slistChips.GetHead();
	while (pchip != NULL)
		{
		if (	(long)pchip->m_fX < lX 
			||	(long)pchip->m_fX > lX + lW
			|| (long)pchip->m_fY < lY
			||	(long)pchip->m_fY > lY + lH)
			{
			// No match.
			pchip = ms_slistChips.GetNext();
			}
		else
			{
			// Found one.
			break;
			}
		}

	return pchip;
	}

//////////////////////////////////////////////////////////////////////
// Internal.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Initialize all members regardless of their current values.
//
//////////////////////////////////////////////////////////////////////
void CChip::Init(void)
	{
	m_fX		= 0.0F;
	m_fY		= 0.0F;
	m_fZ		= 0.0F;

	m_sDestX	= 0;
	m_sDestY	= 0;
	m_sDestZ	= 0;

	m_sRate	= 0;

	m_pimChip	= NULL;

	m_sSliding	= FALSE;

	m_sStackable	= TRUE;

	m_sNumChips		= 1;
	}

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
