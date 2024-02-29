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
// barrel.h
// Project: Nostril (aka Postal)
//
// History:
//		03/14/97 BRH	Started this exploding barrel object from the napalm
//							object.  
//
//		03/17/97 BRH	Restarted this object based on the new CThing3D base
//							class which will make it easier to do the motion for
//							explosions and other message processing.
//
//		04/02/97	JMI	Removed initialization of m_pFire.
//
//		04/04/97	JMI	Added barrel spin animation which has the origin at the
//							barrel's center of gravity for better spinning.
//
//		06/11/97 BRH	Added m_u16ShooterID so it can pass along the information
//							for scoring purposes.
//
//		06/25/97 BRH	Added shadow sprite and alpha animation for the shadow
//							along the ground to see how it looks.  If it looks good, 
//							then it will probably be moved further up into CThing3D.
//
//		08/15/97 BRH	Added a special barrel flag so that this type of 
//							barrel can only be destroyed by the CDude.  Added an
//							EditModify dialog to set the option and added it to
//							the load and save.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BARREL_H
#define BARREL_H

#include "RSPiX.h"
#include "Thing3d.h"
#include "realm.h"
#include "grenade.h"
#include "firebomb.h"
#include "doofus.h"
#include "AlphaAnimType.h"

// CBarrel is an exploding barrel that can react to explosions, fire and shots
class CBarrel : public CThing3d
{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	typedef RChannel<CAlphaAnim> ChannelAA;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		U16		m_u16ShooterID;				// Variable for storing the shooter ID
														// to pass along in the messages.

	protected:

		CCharacter::State m_ePreviousState;		// State variable to remember where he was

		CAnim3D	m_animStill;		// Barrel still animation
		CAnim3D	m_animSpin;			// Barrel still but rotatable around center of gravity.

		CAnim3D*	m_pPreviousAnim;	// Previous state's animation

		short					m_sSphereRadius;	// Radius of the grenader's current frame
		short					m_sSphereX;			// Location of the grenader's sphere center
		short					m_sSphereY;			// Location of the grenader's sphere center

		short					m_sScreenRadius;	// Object's radius
		bool					m_bSpecial;			// Special dude-destroy-only barrel;

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static long ms_lExplosionWait;		// Amount of time to wait before staring fire
		static long ms_lExplosionDelay;		// Time before explosion triggers another.
		static short ms_sNumFires;				// Number of fires to create after explosion

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CBarrel(CRealm* pRealm)
			: CThing3d(pRealm, CBarrelID)
			{
			m_sSuspend = 0;
			m_dRot = 0;
			m_dX = m_dY = m_dZ = m_dVel = m_dAcc = 0;
			m_sScreenRadius = 20;
			m_panimCur = m_pPreviousAnim = NULL;
			m_sprite.m_pthing	= this;
			m_bSpecial = false;
			}

	public:
		// Destructor
		~CBarrel()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			m_pRealm->m_scene.RemoveSprite(&m_spriteShadow);
			m_pRealm->m_smashatorium.Remove(&m_smash);

			// Free resources
			FreeResources();
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
			*ppNew = new CBarrel(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CBarrel::Construct(): Couldn't construct CBarrel (that's a bad thing)\n");
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

		// Startup object
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Find angle to a CDude
		short FindDirection(void);

		// Find the squared distance to the CDude (to avoid sqrt)
		double SQDistanceToDude(void);

		// Set a pointer to the guy you are tracking for other guy related
		// functions like FindDirection and SQDistanceToGuy
		short SelectDude(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Initialize states, positions etc.
		short Init(void);

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

};


#endif //GRENADER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
