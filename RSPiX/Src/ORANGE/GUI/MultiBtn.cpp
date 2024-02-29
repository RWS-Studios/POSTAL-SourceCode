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
// MultiBtn.cpp
// 
// History:
//		04/10/97 JMI	Started this using RPushBtn as a template.
//
//		04/17/97	JMI	Added Load and Save components.
//
//		04/22/97	JMI	Added NextState().
//							CursorEvent() now uses NextState().
//							Also, DrawBackgroundRes() now chooses the image indexed
//							by m_sState instead of m_sState + 1.
//
//		09/25/97	JMI	ReadMembers() was not clearing states that had no 
//							corresponding images which, since SetNumStates() 
//							preserves existing state images, could result in old 
//							images persisting through loads that contained no image
//							for that state.
//							Also, now, in file version 7, reads and writes the 
//							current state.
//
//////////////////////////////////////////////////////////////////////////////
//
// This a GUI item that is based on RBtn. 
// This overrides CursorEvent() to get information about where a click in its
// RHot occurred.
// This overrides Compose() to add text.
//
// Enhancements/Uses:
// To change the look of a button when pressed, you may want to override the
// Compose() or DrawBorder() in a derived class.
// To get a callback on a click/release pair in the button, set m_bcUser.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/MultiBtn.h"
#else
	#include "multibtn.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)	((val == -1) ? def : val)

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RMultiBtn::RMultiBtn()
	{
	// Override RGuiItem's/RBtn's defaults.
	m_type				= MultiBtn;	// Indicates type of GUI item.

	// Initialize RMultiBtn members.
	m_sState				= 0;		// The button's current state, 0..m_sNumStates - 1.
	m_sNumStates		= 0;		// Number of button states.                        
	m_papimStates		= NULL;	// Ptr to array of m_sNumStates ptrs to button     
										// state images.                                   
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RMultiBtn::~RMultiBtn()
	{
	DestroyStates();
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Cursor event notification.
// Events in event area.
// (virtual).
//
////////////////////////////////////////////////////////////////////////
void RMultiBtn::CursorEvent(	// Returns nothing.
	RInputEvent* pie)				// In:  Most recent user input event.             
										// Out: pie->sUsed = TRUE, if used.
	{
	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_RELEASED:
			// If we were clicked in . . .
			if (m_sPressed != FALSE)
				{
				// Do change of state right away so user callback gets the new
				// value.
				// If within event area . . .
				if (		pie->sPosX >= m_sEventAreaX && pie->sPosX < m_sEventAreaX + m_sEventAreaW
						&&	pie->sPosY >= m_sEventAreaY && pie->sPosY < m_sEventAreaY + m_sEventAreaH)
					{
					// Change state.
					NextState();
					}
				}

			break;
		}

	// Call base.
	RGuiItem::CursorEvent(pie);

	switch (pie->sEvent)
		{
		case RSP_MB0_DOUBLECLICK:
		case RSP_MB0_PRESSED:
			// Always recompose on press, since there's so many possibilities
			// with this button.
			Compose();

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;

		case RSP_MB0_RELEASED:
			// Always recompose on release, since there's so many possibilities
			// with this button.
			Compose();

			// Note that we used it.
			pie->sUsed	= TRUE;

			break;
		}
	}

////////////////////////////////////////////////////////////////////////
// Draw background resource, if one is specified.
// Utilizes base class version to place and BLiT the resource.
// (virtual).
////////////////////////////////////////////////////////////////////////
void RMultiBtn::DrawBackgroundRes(	// Returns nothing.
	RImage* pim /*= NULL*/)				// Dest image, uses m_im, if NULL.
	{
	// Store old bkd res.
	RImage*	pimBkdRes	= m_pimBkdRes;

	// If we have any states . . .
	if (m_papimStates != NULL)
		{
		// Choose proper image.
		if (m_sPressed == FALSE)
			{
			// If the state is available . . .
			if (m_sState <= m_sNumStates)
				{
				// Get the state.
				m_pimBkdRes	= m_papimStates[m_sState];
				}
			}
		else
			{
			// Get the pressed feedback.
			m_pimBkdRes	= m_papimStates[0];
			}
		}

	// Call base.
	RBtn::DrawBackgroundRes(pim);

	// Restore bkd res.
	m_pimBkdRes	= pimBkdRes;
	}

////////////////////////////////////////////////////////////////////////
// Set number of states.
// This will clear all existing state images.
////////////////////////////////////////////////////////////////////////
short RMultiBtn::SetNumStates(	// Returns 0 on success.
	short sNumStates)					// In:  New number of states.
	{
	short	sRes	= 0;	// Assume success.

	// Allocate an array of image ptrs and clear them all . . .
	RImage** papimNewStates	= new RImage*[sNumStates + 1];
	if (papimNewStates != NULL)
		{
		// Clear all the ptrs to NULL.
		memset(papimNewStates, 0, sizeof(RImage*) * (sNumStates + 1));

		// If there was an old array . . .
		if (m_papimStates != NULL)
			{
			// Copy any currently valid ptrs within new range.
			short	i;
			for (i = 0; i <= sNumStates && i <= m_sNumStates; i++)
				{
				// Copy entry.
				papimNewStates[i]	= m_papimStates[i];
				// Clear entry so it is not deleted.
				m_papimStates[i]	= NULL;
				}

			// Destroy any current states plus array.
			DestroyStates();
			}

		// Store the new number of states.
		m_sNumStates	= sNumStates;
		// Store new arrray.
		m_papimStates	= papimNewStates;
		}
	else
		{
		TRACE("SetNumStates(): Failed to allocate new array of Image ptrs.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Set button state or feedback state image.
////////////////////////////////////////////////////////////////////////
short RMultiBtn::SetState(	// Returns 0 on success.
	RImage*	pim,				// In:  Image for state sState.
	short		sState)			// In:  State to update (0 == feedback state,
									// 1..n == state number).
	{
	short	sRes	= 0;	// Assume success.

	if (m_papimStates == NULL || sState >= m_sNumStates)
		{
		sRes = SetNumStates(sState);
		}

	// If successful so far . . .
	if (sRes == 0)
		{
		// Clear current value.
		delete m_papimStates[sState];

		// Allocate new one . . .
		m_papimStates[sState]	= new RImage;
		if (m_papimStates[sState] != NULL)
			{
			// Copy specified image.
			*(m_papimStates[sState])	= *pim;
			}
		else
			{
			TRACE("SetState():  Failed to allocate new RImage.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Set button state or feedback state image.
// The feedback state image is always the last image (m_sNumStates).
//////////////////////////////////////////////////////////////////////////////
short RMultiBtn::SetState(	// Returns 0 on success.
	char*	pszImageName,		// In:  File name of image for state sState.
	short		sState)			// In:  State to update (0 == feedback state,
									// 1..n == state number).
	{
	short	sRes	= 0;	// Assume success.

	RImage	im;
	if (RFileEZLoad(&im, pszImageName, "rb", RFile::LittleEndian) == 0)
		{
		sRes	= SetState(&im, sState);
		}
	else
		{
		TRACE("SetState():  RFileEZLoad() failed for \"%s\".\n", pszImageName);
		sRes	= -1;
		}
	
	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Clear button state or feedback state image.
// The feedback state image is always the first image.
//////////////////////////////////////////////////////////////////////////////
void RMultiBtn::ClearState(	// Returns nothing.
	short	sState)					// In:  State to clear (0 == feedback state,
										// 1..n == state number).
	{
	if (sState >= 0 && sState <= m_sNumStates)
		{
		if (m_papimStates != NULL)
			{
			// State be gone.  Safe for already deallocated states as long as
			// they're NULL.
			delete m_papimStates[sState];
			m_papimStates[sState]	= NULL;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// Go to the next logical state.
//////////////////////////////////////////////////////////////////////////////
short RMultiBtn::NextState(void)	// Returns new state.
	{
	if (m_sNumStates > 0)
		{
		m_sState	= (m_sState % m_sNumStates) + 1;
		}
	else
		{
		m_sState	= 0;
		}

	return m_sState;
	}

//////////////////////////////////////////////////////////////////////////////
// Querries.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Get the current image for the specified state.
//////////////////////////////////////////////////////////////////////////////
RImage* RMultiBtn::GetState(	// Returns image, if available; NULL, otherwise.
	short	sState)					// In:  State to get (0 == feedback state,
										// 1..n == state number).
	{
	RImage*	pimRes	= NULL;	// Assume not available.

	if (sState >= 0 && sState <= m_sNumStates)
		{
		if (m_papimStates != NULL)
			{
			pimRes	= m_papimStates[sState];
			}
		}

	return pimRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Internal functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Destroys current state bitmaps.
//////////////////////////////////////////////////////////////////////////////
void RMultiBtn::DestroyStates(void)	// Returns nothing.
	{
	if (m_papimStates != NULL)
		{
		short i;
		for (i = 0; i <= m_sNumStates; i++)
			{
			delete m_papimStates[i];
			}

		delete []m_papimStates;

		m_sNumStates	= 0;
		}
	}

////////////////////////////////////////////////////////////////////////
// Read item's members from file.
// (virtual/protected (overriden here)).
////////////////////////////////////////////////////////////////////////
short RMultiBtn::ReadMembers(		// Returns 0 on success.
	RFile*	pfile,					// File to read from.
	U32		u32Version)				// File format version to use.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RBtn::ReadMembers(pfile, u32Version);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		// Switch on version.
		switch (u32Version)
			{
			default:
			// Insert additional version numbers here!
			case 7:
				pfile->Read(&m_sState);
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:

			case 1:
				{
				short	sNumStates	= 0;	// Safety.
				// Read this class's members.
				pfile->Read(&sNumStates);
				// Set number of states.
				if (SetNumStates(sNumStates) == 0)
					{
					// Read all the images.
					short	sCurState;
					short	sImageForState;
					for (sCurState = 0; sCurState <= m_sNumStates && sRes == 0; sCurState++)
						{
						pfile->Read(&sImageForState);
						if (sImageForState != FALSE)
							{
							// There is an image.  Load it.
							RImage	imState;
							if (imState.Load(pfile) == 0)
								{
								// Set that state.
								if (SetState(&imState, sCurState) == 0)
									{
									// Successfully loaded and set state image.
									}
								else
									{
									TRACE("ReadMembers9): SetState() failed for state #%d.\n", sCurState);
									sRes	= -3;
									}
								}
							else
								{
								TRACE("ReadMembers(): GetState() failed for state #%d.\n", sCurState);
								sRes	= -2;
								}
							}
						else
							{
							// Make sure the state is clear.
							ClearState(sCurState);
							}
						}
					}
				else
					{
					TRACE("ReadMembers(): SetNumStates() failed.\n");
					sRes	= -1;
					}
				}


			case 0:	// In version 0, only base class RGuiItem members were stored.
				// If successful . . .
				if (pfile->Error() == FALSE)
					{
					// Success.
					}
				else
					{
					TRACE("ReadMembers(): Error reading RMultiBtn members.\n");
					sRes	= -1;
					}
				break;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Write item's members to file.
// (virtual/protected (overriden here)).
////////////////////////////////////////////////////////////////////////
short RMultiBtn::WriteMembers(	// Returns 0 on success.
	RFile*	pfile)					// File to write to.
	{
	short	sRes	= 0;	// Assume success.

	// Invoke base class to read base members.
	sRes	= RBtn::WriteMembers(pfile);

	// If okay so far . . .
	if (sRes == 0)
		{
		ASSERT(pfile != NULL);
		ASSERT(pfile->IsOpen() != FALSE);
		
		// Write this class's members.
		pfile->Write(m_sState);
		pfile->Write(m_sNumStates);
		// Write all the images.
		short	sCurState;
		for (sCurState = 0; sCurState <= m_sNumStates && sRes == 0; sCurState++)
			{
			// If there is a bitmap for this state . . .
			RImage*	pimState	= GetState(sCurState);
			if (pimState != NULL)
				{
				// There is an image.  Write flag indicating such.
				pfile->Write((short)TRUE);
				// Write image.
				sRes	= pimState->Save(pfile);
				}
			else
				{
				// No image.  Write flag indicating such.
				pfile->Write((short)FALSE);
				}
			}

		// If successful . . .
		if (pfile->Error() == FALSE)
			{
			// Success.
			}
		else
			{
			TRACE("WriteMembers(): Error writing RMultiBtn members.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
