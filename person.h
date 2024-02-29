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
// person.h
// Project: Postal
//
// History:
//
//		04/28/97 BRH	Started this generic enemy/victim class that will use
//							logic in CDoofus and an information strucutre in
//							Personatorium to determine the abilities and desired
//							logic for this person.
//
//		04/29/97 BRH	Added m_ePersonType value which is the only thing that
//							needs to be loaded and saved.
//
//		05/09/97 BRH	Moved took out m_lNextGroanTime and just used m_lTimer
//							since it is not being used in the Writhing state.
//
//		05/11/97 BRH	Added an override to Logic_Writhing so that I can
//							do a special case for the cop since he crawls along
//							the ground.  
//
//		05/19/97	JMI	Added ms_u16IdLogAI.
//
//		05/19/97	JMI	Added m_szLogicFile, m_sShowState, and Render() override.
//
//		05/19/97 BRH	Added logic table pointer.  Changed the string for the
//							logic file from a char* to an RString so it can be 
//							loaded and saved easily.
//
//		05/20/97 BRH	Added logic table variables as friend classes so their
//							Get functions can access the protected members via the
//							CPerson pointer.
//
//		06/14/97 BRH	Added functions to choose and play various sound effects
//							using the sounds defined in the personatorium for each
//							person type.
//
//		06/18/97	JMI	Changed PlaySoundWrithing() to return the duration of the
//							played sample.
//
//					MJR	Added ResetLogAI() to reset AI logging feature.
//
//		06/27/97	JMI	Now hooks EditRender().
//
//		07/06/97 BRH	Added another friend class for the logic table
//
//		07/17/97	JMI	Changed RSnd*'s to SampleMaster::SoundInstances.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		08/03/97 BRH	Added logic table friend class.
//
//		08/12/97 BRH	Added m_bHitComment flag so that the comment made
//							when shot is only said once, and noises are used
//							for the rest of the shot reactions.
//
//		08/29/97 BRH	Added a static variable for the logic tables to use
//							to set set the state of all of the kids for the
//							ending level.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef PERSON_H
#define PERSON_H

#include "RSPiX.h"
#include "realm.h"
#include "doofus.h"
#include "personatorium.h"
#include "logtab.h"

// CPerson is the class for the enemy guys
class CPerson : public CDoofus
	{
	friend class CLogTabVar_TargetDist;
	friend class CLogTabVar_GetAction;
	friend class CLogTabVar_SetAction;
	friend class CLogTabVar_PopoutAvailable;
	friend class CLogTabVar_RunShootAvailable;
	friend class CLogTabVar_SafetyAvailable;
	friend class CLogTabVar_PylonAvailable;
	friend class CLogTabVar_DudeHealth;
	friend class CLogTabVar_IsTriggered;
	friend class CLogTabVar_UserState1;
	friend class CLogTabVar_RecentlyShot;
	friend class CLogTabVar_IsPanic;
	friend class CLogTabVar_HelpCall;
	friend class CLogTabVar_RecentlyStuck;
	friend class CLogTabVar_UserGlobal;
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	protected:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		RString m_rstrLogicName;

	protected:
		// General position, motion and time variables

		// Store what type of person you are
		Personatorium::Index m_ePersonType;

		// Logic Table for this person's behavior
		CLogTab<CPerson*>* m_pLogicTable;

		// Logic filename.
		RString m_rstrLogicFile;
		
		// If TRUE, the guy's state is shown by his hotspot.
		short	m_sShowState;

		// This is a high level state variable for logic tables.
		short m_sUserState1;

		// This is set once the person has used their 1 shot comment, then
		// it will only use the noises after that.
		bool m_bHitComment;

		// Tracks file counter so we know when to load/save "common" data 
		static short ms_sFileCount;

		// This is the one CPerson that can log its AI table transitions or
		// CIdBank::IdNil.
		static U16	ms_u16IdLogAI;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dLongRange;		// Squared distance (500 pixels away)
		static double ms_dInRangeLow;		// Squared distance to be in range with weapon
		static double ms_dInRangeHigh;	// Squared distance to be in range with weapon
		static double ms_dThrowHorizVel;	// Horizontal throw velocity
		static double ms_dMaxCrawlVel;	// Speed at which cop crawls when writhing
		static long ms_lRandomAvoidTime;	// Time to wander before looking again
		static long ms_lReseekTime;		// Do a 'find' again 
		static long ms_lWatchWaitTime;	// Time to watch shot go
		static long ms_lReselectDudeTime;// Time to go without finding a dude
													// before calling SelectDude() to find
													// possibly a closer one.
		static long ms_lMinCommentTime;	// Min time before making a random comment
		static long ms_lCommentTimeVariance;// Random amount added on to comment timer.

	public:
		static short ms_sLogTabUserGlobal;// Global state set and read by logic tables

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CPerson(CRealm* pRealm)
			: CDoofus(pRealm, CPersonID)
			{
			m_sSuspend = 0;
			m_dRot = 0;
			m_dX = m_dY = m_dZ = m_dVel = m_dAcc = 0;
			m_ePersonType = Personatorium::Grenader;
			m_eWeaponType = CThing::CGrenadeID;
			m_panimCur = m_panimPrev = NULL;
			m_sprite.m_pthing	= this;
			m_rstrLogicFile = "res/logics/default.lgk";
			m_sShowState		= FALSE;
			m_sUserState1 = 0; // Uninitialized
			m_bHitComment = false;
			}

	public:
		// Destructor
		~CPerson()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
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
			*ppNew = new CPerson(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CPerson::Construct(): Couldn't construct CPerson (that's a bad thing)\n");
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

		// Init - set up object before running
		short Init(void);

		// Update object
		void Update(void);

		// Render object.
		void Render(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object.
		void EditRender(void);

		// Function to choose and play the writhing sound effect
		virtual SampleMaster::SoundInstance PlaySoundWrithing(
			long* plDuration);					// Out:  Duration of sample, if not NULL.

		// Function to choose and play the Shot sound effect
		virtual SampleMaster::SoundInstance PlaySoundShot(void);

		// Function to choose and play the Blown up sound effect
		virtual SampleMaster::SoundInstance PlaySoundBlownup(void);

		// Funciton to choose and play the Burning sound effect
		virtual SampleMaster::SoundInstance PlaySoundBurning(void);

		// Function to choose and play the shooting comment
		virtual SampleMaster::SoundInstance PlaySoundShooting(void);

		// Function to choose and play the dying sound.
		virtual SampleMaster::SoundInstance PlaySoundDying(void);

		// Function to choose and play the Random comments
		virtual SampleMaster::SoundInstance PlaySoundRandom(void);

	//---------------------------------------------------------------------------
	// Static functions
	//---------------------------------------------------------------------------
	public:
		static void ResetLogAI(void)
			{
			// Reset AI logging feature.
			ms_u16IdLogAI = CIdBank::IdNil;
			}


	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		short GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Special case logic for the cop writhing since he crawls on the ground.
		virtual void Logic_Writhing(void);
	};


#endif //PERSON_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
