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
// band.h
// Project: Postal
//
// History:
//
//		03/04/97 BRH	Started the marching band guy based on CDoofus so that it
//							can use the NavNet and which is now derived from CCharacter.
//							This will be the first guy written based on CCharacter
//							that will use many of the CCharacter functions for updating.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/18/97	JMI	Added m_idChildItem to handle the storage of child items.
//							Also, uncommented Render() proto.
//
//		03/18/97	JMI	Added proto for OnDead() override.
//
//		06/04/97	JMI	Now aborts ms_pBandSongSound, if not NULL, in destructor.
//							Also, added ms_bDonePlaying so marchers know when to not
//							restart ms_pBandSongSound.
//
//		06/08/97	JMI	Added override for WhileDying() and WhileShot().
//
//		06/24/97	JMI	Removed m_sRotateDir member of CBand (it is already defined
//							in CDoofus base class).
//
//		07/17/97	JMI	Changed ms_pBandSongSound to ms_siBandSongInstance.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/31/97 BRH	Set the default value of the destination bouy to 1 in 
//							the constructor.
//
//		08/12/97	JMI	Now one band member maintains the volume for the band 
//							sample.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BAND_H
#define BAND_H

#include "RSPiX.h"
#include "doofus.h"

// CBand is a class of marching band members for the parade
class CBand : public CDoofus
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	protected:
		CCharacter::State m_ePreviousState;	// State variable to remember what he was
														// Doing before he was shot, etc.
		CAnim3D*	m_pPreviousAnim;				// Previous state's animation

		CAnim3D m_animStand;						// Stand animation
		CAnim3D m_animMarch;						// Marching animation
		CAnim3D m_animRun;						// Running away animation
		CAnim3D m_animShot;						// Shot dead animation
		CAnim3D m_animBlownup;					// Blown up by explosion
		CAnim3D m_animOnFire;					// Running while on fire

		U16					m_idChildItem;		// ID of child item or CIdBank::IdNil.

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dCloseToBouy;		// How close to be to a bouy to consider yourself 'there'
		static double ms_dMingleBouyDist;	// How close to be to a bouy when mingling around
		static double ms_dExplosionVelocity;// How high he will get blown up.
		static double ms_dMaxMarchVel;		// How fast to march
		static double ms_dMaxRunVel;			// Hos fast to run
		static long ms_lMingleTime;			// How long to mingle before moving
		static short ms_sStartingHitPoints;	// How many hit points to start with
		static SampleMaster::SoundInstance ms_siBandSongInstance;		// sound played during band march.
		static U16	ms_idBandLeader;			// The person who adjusts the band sound
														// volume or IdNil.


		// This value indicates whether the marchers have stopped playing in this level.
		static bool	ms_bDonePlaying;
	
	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CBand(CRealm* pRealm)
			: CDoofus(pRealm, CBandID)
			{
			m_ucNextBouyID = 1;
			m_ucDestBouyID = 1;
			m_idChildItem	= CIdBank::IdNil;
			m_bCivilian = true;
			}

	public:
		// Destructor
		~CBand()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			m_pRealm->m_smashatorium.Remove(&m_smash);

			// Free resources
			FreeResources();

			// If sample playing . . .
			if (ms_siBandSongInstance != 0)
				{
				AbortSample(ms_siBandSongInstance);
				ms_siBandSongInstance	= 0;
				}
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
			*ppNew = new CBand(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CBand::Construct(): Couldn't construct CBand (that's a bad thing)\n");
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

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by editor when a new object is created
		short EditNew(short sX, short sY, short sZ);

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise
		// Called by editor to render object
//		void EditRender(void);

	//---------------------------------------------------------------------------
	// Message handlers that are called by CCharacter ProcessMessage().  These
	// have code to set the correct animation, state, etc for these messages
	//---------------------------------------------------------------------------
	public:

		void OnShotMsg(Shot_Message* pMessage);

		void OnBurnMsg(Burn_Message* pMessage);

		void OnExplosionMsg(Explosion_Message* pMessage);

		void OnPanicMsg(Panic_Message* pMessage);

	//---------------------------------------------------------------------------
	// Useful generic character state-specific functionality.
	//---------------------------------------------------------------------------
	public:

		// Implements basic one-time functionality for each time State_Dead is
		// entered.
		void OnDead(void);

		// Implements basic functionality while dying and returns true
		// until the state is completed.
		virtual						// Overriden here.
		bool WhileDying(void);	// Returns true until state is complete.

		// Implements basic functionality while being shot and returns true
		// until the state is completed.
		virtual						// Overriden here.
		bool WhileShot(void);	// Returns true until state is complete.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Initalize the object - this should be called after the resources are loaded
		short Init(void);

		// Go through the message queue and change the state if necessary
		void ProcessMessages(void);

		// Send panic message to other band members
		void AlertBand(void);

		// Drop item and apply appropriate forces.
		void DropItem(void);	// Returns nothing.

	};


#endif //BAND_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
