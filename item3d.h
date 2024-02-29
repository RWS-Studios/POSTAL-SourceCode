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
// item3d.h
// Project: Nostril (aka Postal)
//
//	History:
//		03/06/97	JMI	Started this 3D item class.
//
//		03/06/97	JMI	Overrided EditModify().
//
//		03/06/97	JMI	Added m_u16IdParent member and override for Render().
//
//		03/06/97	JMI	Added Trumpet, Horn, and Sax and descriptions for known
//							types.
//
//		03/07/97	JMI	Added handy everything-to-get-started Setup().
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/17/97	JMI	Now based on CThing3d instead of CCharacter.
//
//		03/18/97	JMI	Made ItemType and instantiable members public.
//							Also, moved m_u16IdParent into base class.
//							Also, removed Render() proto.
//
//		06/14/97	JMI	Added a constructor that will allow derivations of this
//							class.
//
//		08/07/97	JMI	Added ability to optionally load a rigid body and child
//							anim for this item3d.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef ITEM3D_H
#define ITEM3D_H

#include "RSPiX.h"

#include "character.h"

class CItem3d : public CThing3d
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum		// Items.
			{
			None,				// No current type.  Must be 0.
			Custom,			// A type whose name is stored in m_szAnimBaseName[].
			Trumpet,
			Horn,
			Sax,

			// Add new item enums above this line.
			NumTypes
			} ItemType;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

		CAnim3D		m_anim;									// One animation.
		char			m_szAnimBaseName[RSP_MAX_PATH];	// Name of animation.
		ItemType		m_type;									// Item type if known.

		char			m_szAnimRigidName[RSP_MAX_PATH];	// Rigid body transform anim name.
		CAnim3D		m_animChild;									// Optional child anim.
		char			m_szChildAnimBaseName[RSP_MAX_PATH];	// Name of child anim.

		CSprite3		m_spriteChild;							// Child sprite.  Never 
																	// explicitly added to scene
																	// (acts just as a child to
																	// our main sprite).

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:
		// "Constant" values that we want to be able to tune using the editor

		// Array of known animation base names.
		static char*	ms_apszKnownAnimBaseNames[NumTypes];
		static char*	ms_apszKnownAnimDescriptions[NumTypes];

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CItem3d(CRealm* pRealm)
			: CThing3d(pRealm, CItem3dID)
			{
			Reset();
			}

		// Constructor
		CItem3d(CRealm* pRealm, ClassIDType id)
			: CThing3d(pRealm, id)
			{
			Reset();
			}

		// Reset this object.
		void Reset(void)	// Returns nothing.
			{
			m_szAnimBaseName[0]	= '\0';
			m_type					= None;

			m_szAnimRigidName[0]			= '\0';
			m_szChildAnimBaseName[0]	= '\0';

			m_spriteChild.m_pthing		= this;
			}

	public:
		// Destructor
		~CItem3d()
			{
			// Kill item3d
			Kill();
			}

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			short sResult = 0;
			*ppNew = new CItem3d(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CItem3d::Construct(): Couldn't construct CItem3d!\n");
				}

			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Update object
		void Update(void);

		// Render object
		void Render(void);										// Returns nothing.

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object.
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

	//---------------------------------------------------------------------------
	// Other functions
	//---------------------------------------------------------------------------
	public:
		// Setup object after creating it
		virtual				// Override to implement additional functionality.
								// Call base class to get default functionality.
		short Setup(						// Returns 0 on success.
			short sX,						// In:  Starting X position
			short sY,						// In:  Starting Y position
			short sZ,						// In:  Starting Z position
			ItemType type,					// In:  Known item type or Custom.
			char*	pszCustomBaseName = NULL,	// In:  Required if type == Custom.
														// Base name for custom type resources.
			U16	u16IdParentInstance = CIdBank::IdNil);	// In:  Parent instance ID.

		// Message handling functions ////////////////////////////////////////////

		// Handles a Shot_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnShotMsg(					// Returns nothing.
			Shot_Message* pshotmsg);	// In:  Message to handle.

		// Handles an Explosion_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnExplosionMsg(							// Returns nothing.
			Explosion_Message* pexplosionmsg);	// In:  Message to handle.

		// Handles a Burn_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnBurnMsg(					// Returns nothing.
			Burn_Message* pburnmsg);	// In:  Message to handle.

		// Handles an ObjectDelete_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnDeleteMsg(								// Returns nothing.
			ObjectDelete_Message* pdeletemsg);	// In:  Message to handle.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Init item3d
		short Init(void);											// Returns 0 if successfull, non-zero otherwise
		
		// Kill item3d
		void Kill(void);

		// Get all required resources
		short GetResources(void);								// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		void FreeResources(void);

	};


#endif //ITEM3D_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
