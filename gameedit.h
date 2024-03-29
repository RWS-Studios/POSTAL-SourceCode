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
// GameEdit.h
// Project: Nostril (aka Postal)
//
// History:
//		12/18/96 MJR	Started.
//
//		02/07/97	JMI	Added CGameEditThing to store editor settings and, perhaps,
//							later it could do more things that only a CThing derived
//							class can do for the Editor.
//
//		02/07/97	JMI	Needed include of idBank.h.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/18/97	JMI	Added Edit_Menu_Continue() and Edit_Menu_ExitEditor().
//
////////////////////////////////////////////////////////////////////////////////
#ifndef GAMEEDIT_H
#define GAMEEDIT_H

////////////////////////////////////////////////////////////////////////////////
// Includes.
////////////////////////////////////////////////////////////////////////////////
#include "thing.h"
#include "camera.h"
#include "IdBank.h"

extern void NavNetListPressedCall(RGuiItem* pgui);

////////////////////////////////////////////////////////////////////////////////
// Typedefs.
////////////////////////////////////////////////////////////////////////////////

// One of these classes is created by the editor for each Realm.  This class can
// then store and retrieve the editor settings for a Realm on Load and Save.
class CGameEditThing : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

		// Settings //////////////////////////////////////////////////////////////
		
		U16	m_u16CameraTrackId;	// ID of object for grip to track.
		int16_t	m_sViewPosX;			// View position.
		int16_t	m_sViewPosY;			// View position.
		RListBox* m_plbNavNetList; // Pointer to Nav Net List Box

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CGameEditThing(CRealm* pRealm)
			: CThing(pRealm, CGameEditThingID)
			{
			// Set defaults.
			m_u16CameraTrackId	= CIdBank::IdNil;
			m_sViewPosX				= 0;
			m_sViewPosY				= 0;
			}

	public:
		// Destructor
		~CGameEditThing()
			{
			}

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static int16_t Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			int16_t sResult = 0;
			*ppNew = new CGameEditThing(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CGameEditThing::Construct(): Couldn't construct CGameEditThing!\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implementing them as inlines doesn't pay!
	// (unless you really, really don't want them in your CPP!)).
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		int16_t Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			int16_t sFileCount,										// In:  File count (unique per file, never 0)
			uint32_t	ulFileVersion)									// In:  Version of file format to load.
			{
			// Call base class.
			int16_t	sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
			if (sResult == 0)
				{
				// Read settings.
				switch (ulFileVersion)
					{
					default:
					case 1:
						pFile->Read(&m_u16CameraTrackId);
						pFile->Read(&m_sViewPosX);
						pFile->Read(&m_sViewPosY);
						break;
					}

				// Make sure there were no format errors . . .
				if (sResult == 0)
					{
					// File status indicates our success, or lack thereof.
					sResult	= pFile->Error();
					}
				}
			else
				{
				TRACE("CGameEditThing::Load(): Base class Load() failed.\n");
				}

			return sResult;
			}

		// Save object (should call base class version!)
		int16_t Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			int16_t sFileCount)										// In:  File count (unique per file, never 0)
			{
			// Call base class.
			int16_t	sResult	= CThing::Save(pFile, sFileCount);
			if (sResult == 0)
				{
				// Write settings.
				pFile->Write(m_u16CameraTrackId);
				pFile->Write(m_sViewPosX);
				pFile->Write(m_sViewPosY);

				// File status indicates our success, or lack thereof.
				sResult	= pFile->Error();
				}
			else
				{
				TRACE("CGameEditThing::Save(): Base class Save() failed.\n");
				}

			return sResult;
			}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
	};

////////////////////////////////////////////////////////////////////////////////
// Protos.
////////////////////////////////////////////////////////////////////////////////

extern void GameEdit(
	void);

// Called by the menu callback when it wants to tell the editor to continue 
// editting (end the menu).
extern void Edit_Menu_Continue(void);
// Called by the menu callback when it wants to tell the editor to quit the
// editor.
extern void Edit_Menu_ExitEditor(void);

//Utilities needed elsewhere


// Map a screen coordinate to a realm coordinate.
// Note that this function's *psRealmY output is always
// the height specified by the realm's attribute map
// at the resulting *psRealmX, *psRealmZ.
extern void MapScreen2Realm(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	int16_t sScreenX,				// In:  Screen x coord.
	int16_t sScreenY,				// In:  Screen y coord.
	int16_t* psRealmX,				// Out: Realm x coord.
	int16_t* psRealmY,				// Out: Realm y coord (always via realm's height map).
	int16_t* psRealmZ);				// Out: Realm z coord.


////////////////////////////////////////////////////////////////////////////////
// Map a screen coordinate to a realm coordinate.
// Note that this function's *psRealmY output is always
// the height specified by the realm's attribute map
// at the resulting *psRealmX, *psRealmZ.
////////////////////////////////////////////////////////////////////////////////
extern void MapScreen2Realm(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	double sScreenX,				// In:  Screen x coord.
	double sScreenY,				// In:  Screen y coord.
	double* psRealmX,				// Out: Realm x coord.
	double* psRealmY,				// Out: Realm y coord (always via realm's height map).
	double* psRealmZ);				// Out: Realm z coord.

// Map a realm coordinate to a screen coordinate.
extern void Maprealm2Screen(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	int16_t		sRealmX,				// In:  Realm x coord.
	int16_t		sRealmY,				// In:  Realm y coord.
	int16_t		sRealmZ,				// In:  Realm z coord.
	int16_t*	psScreenX,			// Out: Screen x coord.
	int16_t*	psScreenY);			// Out: Screen y coord.

////////////////////////////////////////////////////////////////////////////////
// Map a realm coordinate to a screen coordinate.
////////////////////////////////////////////////////////////////////////////////
extern void Maprealm2Screen(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	double		sRealmX,				// In:  Realm x coord.
	double		sRealmY,				// In:  Realm y coord.
	double		sRealmZ,				// In:  Realm z coord.
	double*	psScreenX,			// Out: Screen x coord.
	double*	psScreenY);			// Out: Screen y coord.

#endif //GAMEEDIT_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
