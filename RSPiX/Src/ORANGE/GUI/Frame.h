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
//////////////////////////////////////////////////////////////////////////////
//
// Frame.H
// 
// History:
//		01/13/97 JMI	Started.
//
//		08/22/97	JMI	Now uses rspGeneralLock/Unlock() to make sure the 
//							destination buffer to Draw() is properly locked, if 
//							necessary.
//
//////////////////////////////////////////////////////////////////////////////
//
// This is a very simple class designed to be used as a container.  It does
// not allocate any memory for its image and it does not draw its image.  It
// just draws its children.  It's a way of making many child items relative
// to one particular item that has no 'mass'.
// Very useful and efficient for grouping large areas of object with 
// practically no memory overhead.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef FRAME_H
#define FRAME_H

//////////////////////////////////////////////////////////////////////////////
// Please see the CPP file for an explanation of this API.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////

#include "System.h"
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/guiItem.h"
#else
	#include "GuiItem.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RFrame : public RGuiItem
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RFrame(void)
			{
			// No border.
			m_sBorderThickness	= 0;
			// Don't show focus.
			m_sShowFocus			= FALSE;
			}

		// Destructor.
		~RFrame(void)
			{
			}

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		// Compose item.
		virtual					// If you override this, call this base if possible.
		void Compose(			// Returns nothing.
			RImage* pim = NULL)	// Dest image, uses m_im if NULL.
			{
			// Set hot area.
			SetHotArea();

			// Set event area.
			SetEventArea();
			}

		// Creates a displayable Gui.  Call SetFont and SetText before calling
		// this as it calls Compose.
		virtual					// If you override this, call this base if possible.
		short Create(			// Returns 0 on success.
			short sX,			// X position relative to "parent" item.
			short sY,			// Y position relative to "parent" item.
			short sW,			// Width.
			short sH,			// Height.
			short sDepth)		// Color depth.
			{
			m_sX				= sX;
			m_sY				= sY;
			m_im.m_sWidth	= sW;
			m_im.m_sHeight	= sH;

			Compose();

			// Nothing to allocate.
			return 0;
			}

		// Draw this item and all its subitems into the provided RImage.
		virtual						// If you override this, call this base if possible.
		short Draw(					// Returns 0 on success.
			RImage* pimDst,		// Destination image.
			short sDstX	= 0,		// X position in destination.
			short sDstY	= 0,		// Y position in destination.
			short sSrcX = 0,		// X position in source.
			short sSrcY = 0,		// Y position in source.
			short sW = 0,			// Amount to draw.
			short sH = 0,			// Amount to draw.
			RRect* prc = NULL)	// Clip to.
			{
			short	sRes	= 0;	// Assume success.

			// If visible . . .
			if (m_sVisible != FALSE)
				{
				sDstX	+= m_sX;
				sDstY	+= m_sY;

				// An original clippage should be passed to all these children.
				// Set up default clip rect.
				RRect	rc;
				// Get client.
				GetClient(&rc.sX, &rc.sY, &rc.sW, &rc.sH);
				
				// Affect by draw position.
				rc.sX	+= sDstX;
				rc.sY	+= sDstY;
				
				// Clip to destination.
				RRect	rcDst;

				// If no rect . . .
				if (prc == NULL)
					{
					// Provide one using destination image.
					rcDst.sX	= 0;
					rcDst.sY	= 0;
					rcDst.sW	= pimDst->m_sWidth;
					rcDst.sH	= pimDst->m_sHeight;

					prc	= &rcDst;
					}

				// Clip default clipper to provided or destination.
				rc.ClipTo(prc);

				// Draw back to front!

				// Note that we lock this here and don't unlock it until after we
				// have processed our children.  Although and because Draw()
				// is recursive, this is the most efficient way to do this lock.
				// Since rspGeneralLock() (or, at least, the functions it calls) 
				// do not do anything if the buffer is already locked, there is
				// little efficiency lost in calling this multiple times but plenty
				// gained by not locking/unlocking and locking/unlocking again and
				// again (the exception being the flip page).
				rspGeneralLock(pimDst);

				RGuiItem*	pgui	= m_listguiChildren.GetTail();
				
				while (sRes == 0 && pgui != NULL)
					{
					// Draw subitem and all its subitems.
					sRes	= pgui->Draw(pimDst, sDstX, sDstY, 0, 0, 0, 0, &rc);

					pgui	= m_listguiChildren.GetPrev();
					}

				// If this item has the focus . . .
				if (ms_pguiFocus == this)
					{
					// Draw focus feedback, if this item shows the focus.
					DrawFocus(pimDst, sDstX, sDstY, &rc);
					}

				// Unlock the destination buffer.
				rspGeneralUnlock(pimDst);
				}

			return sRes;
			}


		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

	protected:	// Internal typedefs.

	protected:	// Protected member variables.

	};

#endif // FRAME_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
