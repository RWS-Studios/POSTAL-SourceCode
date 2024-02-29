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
// weapon.h
// Project: Postal
//
//	History:
//
//		02/28/97 BRH	Started this base class for weapons.  Will change several
//							of the current weapons to derive from this base class.
//
//		03/03/97 BRH	Added a few states from the Rocket and Napalm.
//
//		03/03/97 BRH	Added 3d sprite to the weapon to make it more universal.
//							A weapon that is 2d or uses no sprite will be a special
//							case where it will add a 2d sprite to its derived class, 
//							or ignore the sprite entirely.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Added virtual functions for processing messages and
//							virtual OnMessage handler functions so that it follows
//							the model of the CThing3d base class object.  
//
//		04/29/97	JMI	Removed the m_sprite (was a CSprite3) so we could instead
//							make this data part of the descended class.  Since C++ does
//							not support virtual data, we had to replace the base class
//							m_sprite with a pure virtual GetSprite() that returns the
//							appropriate sprite for the class.
//
//		05/09/97 BRH	Added SetRangeToTarget function to vary the velocity
//							of the weapon before it is shot in order to hit
//							your target. 
//
//		06/25/97 BRH	Added m_spriteShadow and PrepareShadow() for shadow
//							support for weapons.   
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		07/31/97 BRH	Added remote control state.
//
//		08/14/97 BRH	Added a virtual function SetCollideBits so that weapons
//							that use collision bits can overload it to set their
//							bit fields.  Others that don't use it won't overload it
//							and the call will be ignored.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//							Also, changed MineSndHalfLife to 150 (was 500).
//
//		08/24/97 BRH	Added SetDetectionBits function like the SetCollideBits
//							which the heatseeker will override to set its bits.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef WEAPON_H
#define WEAPON_H

#include "RSPiX.h"
#include "realm.h"

// CWeapon is the base class for the weapons
class CWeapon : public CThing
{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	typedef unsigned char CWeaponState;

	typedef enum
		{
		LaunchSndHalfLife			= 1000,
		ExplosionSndHalfLife		= 1000,
		SideEffectSndHalfLife	= 1000,	// Grenade bounce or similar.
		FireBombSndHalfLife		= 1000,
		NapalmSndHalfLife			= 1000,
		MineSndHalfLife			= 80
		} Macros;

	typedef enum
	{
		State_Idle,
		State_Fire,
		State_Go,
		State_Roll,
		State_Explode,
		State_Slide,
		State_Hide,
		State_Chase,
		State_Find,
		State_Deleted,
		State_Armed,
		State_RemoteControl
	};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		double m_dX;							// x coord
		double m_dY;							// y coord
		double m_dZ;							// z coord
		double m_dVertVel;					// Vertical Velocity
		double m_dHorizVel;					// Horizontal Velocity								
		double m_dRot;							// Rotation (in degrees, 0 to 359.999999)

		CWeaponState m_eState;				// State variable for run routine

		U16		m_idParent;					// Anyone can be this item's parent.
													// It'd probably be a good idea to make
													// sure this is NULL before setting it,
													// though.

		U16		m_u16ShooterID;			// Instance ID of the shooter (so that credit
													// for a kill can be determined)
		CSprite2	m_spriteShadow;			// 2D sprite for shadow on the ground

	protected:
		long m_lTimer;							// Timer for explosion
		long m_lPrevTime;						// Previous update time

		short m_sSuspend;						// Suspend flag
		short	m_sCurRadius;					// Radius of the dude's current frame.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

	public:
		// "Constant" values that we want to be able to tune using the editor

 	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CWeapon(CRealm* pRealm, ClassIDType id)
			: CThing(pRealm, id)
			{
			m_sSuspend = 0;
			m_dX = m_dY = m_dZ = m_dRot = m_dVertVel = m_dHorizVel = 0.0;
			m_eState = State_Idle;
			m_idParent = CIdBank::IdNil;
			m_spriteShadow.m_sInFlags = CSprite::InHidden;
			m_spriteShadow.m_pImage = NULL;
			m_spriteShadow.m_pthing = this;
			m_lPrevTime = 0;  // valgrind fix.  --ryan.
			}


	public:
		// Destructor
		~CWeapon()
			{
				// Remove sprite from scene (this is safe even if it was already removed!)
				m_pRealm->m_scene.RemoveSprite(&m_spriteShadow);
				// Release the shadow resource
				if (m_spriteShadow.m_pImage)
					rspReleaseResource(&g_resmgrGame, &(m_spriteShadow.m_pImage));
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
			return 0;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		virtual short Load(										// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		virtual short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Setup object after creating it
		virtual short Setup(
			short sX,												// In: Starting X position
			short sY,												// In: Starting Y position
			short sZ);												// In: Starting Z position

		// Startup object
		virtual short Startup(void);							// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		virtual short Shutdown(void);							// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		virtual void Suspend(void);

		// Resume object
		virtual void Resume(void);

		// Update object
		virtual void Update(void);

		// Render object
		virtual void Render(void);

		// Called by editor to init new object at specified position
		virtual short EditNew(									// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		virtual short EditModify(void);						// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		virtual short EditMove(									// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to update object
		virtual void EditUpdate(void);

		// Called by editor to render object
		virtual void EditRender(void);

		// Give Edit a rectangle around this object
		virtual void EditRect(RRect* pRect)
		{
		}

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

		virtual short GetResources(void);

		virtual short FreeResources(void);

		virtual double BounceAngle(double dAngle);

		// Function to modify the velocity for a requested range
		// Weapons that can modify their velocity to affect the
		// range can do the calculation and modify their horizontal
		// velocity.  This default one does nothing
		virtual short SetRangeToTarget(short sRequestedRange)
		{
			return sRequestedRange;
		}

		// Get the descended class's sprite.  Note that the type will vary.
		// This is a pure virtual function, making this an abstract
		// (i.e., uninstantiable) class.
		virtual			// A descended class must define this virtual
							// in order to be instantiable.
		CSprite* GetSprite(void) = 0;	// Returns this weapon's sprite.

		// Process all messages currently in the message queue through 
		// ProcessMessage().
		virtual			// Override to implement additional functionality.
		void ProcessMessages(void);

		// Process the specified message.  For most messages, this function
		// will call the equivalent On* function.
		virtual			// Override to implement additional functionality.
		void ProcessMessage(		// Returns nothing.
			GameMessage* pmsg);	// Message to process.

		// This function provides a way to set the collision bits for
		// a specific weapon, if that weapon uses collision bits.  Rocket
		//	and Heatseeker will override this to set their collision bits, 
		// but others like the cocktail don't use collision bits so they
		// will just rely on this version of the function to throw out
		// the request to set the bits.
		virtual
		void SetCollideBits(		// Returns nothing
			U32 u32BitsInclude,  // Bits included in a collision
			U32 u32BitsDontCare, // Bits that are ignored for collision
			U32 u32BitsExclude)	// Bits that invalidate collision
		{
			// The base class does nothing - override if you want to use it, but
			// you don't have to if your weapon doesn't have collide bits.
		}

		// This function provides a way to set the detection bits for
		// a specific weapon.  If that weapon uses detection bits, it
		// can override this function, otherwise the bits will be ignored
		virtual
		void SetDetectionBits(		// Returns nothing
			U32 u32BitsInclude,		// Bits included in a collision
			U32 u32BitsDontcare,		// Bits that are ignored for collision
			U32 u32BitsExclude)		// Bits that invalidate collision
		{
			// The base class does nothing - override if you want to use it, but
			// you don't have to if your weapon doesn't have detect bits
		}

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

		// Handles Trigger_Message
		virtual			// Overrride to implement additional functionality
							// Call base class to get default functionality.
		void OnTriggerMsg(							// Returns nothing.
			Trigger_Message* ptriggermsg);		// In: Message to handle

		// Load the default shadow resource unless one has already been loaded
		// and set the shadow to visible
		virtual			// Override to implement additional functionality
							// Call base class to get default functionality
		short PrepareShadow(void);	// Returns SUCCESS if set successfully

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
};

#endif //WEAPON_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
