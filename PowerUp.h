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
// PowerUp.H
// Project: Nostril (aka Postal)
// 
// History:
//		05/08/97 JMI	Started.
//
//		05/09/97	JMI	Added m_smash and Init().
//
//		05/14/97	JMI	Added Grab() and Drop().
//							Also, made m_sprite public.
//
//		06/06/97	JMI	Got rid of the whole type thing.  Now uses a CStockPile
//							and can contain any combination of powerups w/i one
//							instance.  Types are still used for loading old powerup
//							files.
//							Also, added GetDescription().
//
//		06/12/97	JMI	Added support for new weapon members of CStockPile.
//
//		06/12/97	JMI	Added sHitPointMax parameter to GetDescription().
//
//		06/14/97	JMI	Changed to a descendant of CItem3d instead of CThing.
//
//		06/14/97	JMI	Added KevlarVest (CStockPile::m_sArmorLayers).
//
//		06/15/97	JMI	Added Backpack (CStockPile::m_sBackpack).
//
//		06/16/97	JMI	Added a user (audible) feedback function, PickUpFeedback().
//
//		07/15/97	JMI	Made GetDescription() and TypeToStockPile() static and 
//							added stockpiles as parms so they can be used more 
//							generically.
//							Also, added RepaginateNow().
//							Also, added IsEmpty().
//
//		07/15/97	JMI	Added some message handling functions.
//							Transferred powerup index enums into CStockPile.
//
//		07/16/97	JMI	Moved IsEmpty() from powerup to stockpile.
//
//		07/23/97	JMI	Added separate launcher for napalm.
//
//		08/17/97	JMI	Got rid of m_szMessages and all message related functions
//							and variables from CDude since we are now using the toolbar 
//							for dude status feedback to the user.  This includes:  
//							MsgTypeInfo, m_lNextStatusUpdateTime, m_lMsgUpdateDoneTime, 
//							m_print, m_bClearedStatus, m_szMessages[], m_sDeadMsgNum, 
//							ms_amtfMessages[], ms_u8FontForeIndex, ms_u8FontBackIndex,
//							ms_u8FontShadowIndex, DrawStatus(), StatusChange(), 
//							MessageChange(), Message(), UpdateFontColors(), 
//							CPowerUp::ms_apszPowerUpTypeNames[], 
//							CPowerUp::GetDescription(), and some strings and a string
//							array in localize.*.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CItem3d-derived class will represent power ups that the player can
// pick up.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef POWERUP_H
#define POWERUP_H

#include "RSPiX.h"
#include "realm.h"
#include "item3d.h"
#include "StockPile.h"

class CPowerUp : public CItem3d
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
														
	protected:
														
	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:
		// Powerup anim names.
		static char*	ms_apszPowerUpResNames[CStockPile::NumStockPileItems + 2];

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CPowerUp(CRealm* pRealm)
			: CItem3d(pRealm, CPowerUpID)
			{
			m_panimCur	= &m_anim;

			m_stockpile.m_sHitPoints	= 0;

			m_sprite.m_pthing	= this;

			m_smash.m_pThing	= this;
			}

	public:
		// Destructor
		~CPowerUp()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);

			// Free resources
			FreeResources();

			// Remove collision thinger.
			m_pRealm->m_smashatorium.Remove(&m_smash);
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
			*ppNew = new CPowerUp(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CPowerUp::Construct(): Couldn't construct CPowerUp (that's a bad thing)\n");
				}
			return sResult;
			}


	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Preload assets needed during the game
		static short Preload(CRealm* prealm);

	//---------------------------------------------------------------------------
	// Required virtual functions (implementing them as inlines doesn't pay!)
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
		void Render(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

	//---------------------------------------------------------------------------
	// Handy external functions
	//---------------------------------------------------------------------------
	public:

		short Setup(												// Returns 0 on success.
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ);												// In: New z coord

		// Call to grab this item.
		short Grab(						// Returns 0 on success.
			CSprite* psprParent);	// In:  Parent's sprite.

		// Call to release this item.
		void Drop(						// Returns nothing.
			short sX,					// In:  Position from which to release.
			short sY,					// In:  Position from which to release.
			short sZ);					// In:  Position from which to release.

		// Determine if this powerup's stockpile is empty.
		bool IsEmpty(void)					// Returns true, if the stockpile is
													// empty.
			{
			return m_stockpile.IsEmpty();
			}

		// Plays a sample corresponding to the type of powerup indicating it
		// was picked up.
		void PickUpFeedback(void);	// Returns nothing.

		// If this powerup has nothing left, destroys itself.  Otherwise, chooses
		// a new resource to represent its current contents.
		// NOTE:  This function can cause this item to be destroyed.  Don't use
		// a ptr to this object after calling this function.
		void RepaginateNow(void);

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

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Initialize object.
		short Init(void);									// Returns 0 on success.

		// Get resource name for this item.
		void GetResName(			// Returns nothing.
			char*	pszResName);	// Out: Resource base name.

	};


#endif // POWERUP_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
