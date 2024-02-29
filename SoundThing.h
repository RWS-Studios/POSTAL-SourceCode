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
// SoundThing.H
// Project: Nostril (aka Postal)
// 
// History:
//		02/24/97 MJR	Stole infrastructure from Jon's AnimThing.
//
//		03/07/97	JMI	Added m_psndChannel member and ProcessMessages() and
//							m_state (with enums).
//
//		03/13/97	JMI	Load now takes a version number.
//
//		07/17/97	JMI	Changed RSnd*'s to SampleMaster::SoundInstances.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//							Also, added user edittable member m_lVolumeHalfLife.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		08/01/97	JMI	Added looping parameters.
//
//		08/04/97	JMI	Now defaults to enabled.
//
//		08/04/97	JMI	Added m_sAmbient indicating whether or not this sound
//							is ambient (i.e., non-essential).
//							Also, implemented a special random number generator
//							strictly for sound things so they can be merry and random
//							and not de-synchronize.
//
//		08/11/97	JMI	Added RelayVolume() and m_lCollectiveVolume so 
//							CSoundRelays can update their CSoundThing parents.
//
//		09/24/97	JMI	Now initializes bFemalePain member of m_id.  This member
//							indicates whether the sample is of a female in pain which
//							some countries (so far just UK) don't want in the game.
//
//		10/07/97	JMI	Changed bFemalePain to usDescFlags.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will play sounds with various options.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SOUNDTHING_H
#define SOUNDTHING_H

#include "RSPiX.h"
#include "realm.h"
#include "SampleMaster.h"

// This class has its own GetRandom() to keep it from de-synching the game.
#ifdef GetRandom
	#undef GetRandom
#endif

#ifdef GetRand
	#undef GetRand
#endif


class CSoundThing : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum
			{
			State_Happy,		// La, la, la.
			State_Delete		// Delete self next chance.
			} State;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		bool m_bInitiallyEnabled;
		bool m_bInitiallyRepeats;
		long m_lMinTime[2];
		long m_lRndTime[2];
		char	m_szResName[RSP_MAX_PATH];		// Resource name

		SampleMasterID m_id;

		RImage* m_pImage;							// Pointer to only image (for editor only)
		CSprite2 m_sprite;						// Sprite (for editor only)
		double m_dX;								// x coord (for editor only)
		double m_dY;								// y coord (for editor only)
		double m_dZ;								// z coord (for editor only)

		SampleMaster::SoundInstance	m_siChannel;

		long m_lLastStartTime;
		long m_lNextStartTime;
		short m_sWhichTime;
		bool m_bEnabled;
		bool m_bRepeats;

		short	m_sUseLooping;						// TRUE, to use looping parameters.
		long	m_lStopLoopingTime;				// Time that we stop looping the sample.
		long	m_lNumLoopBacks;					// Number of times to play loop area of sample.
		long	m_lLoopBackTo;						// Where to loop back to.
		long	m_lLoopBackFrom;					// Where to loop back from.

		short m_sSuspend;							// Suspend flag

		State	m_state;								// Current state.

		long	m_lVolumeHalfLife;				// Half life of the current sound.

		short	m_sPurgeSampleWhenDone;			// TRUE, to purge sample when done.

		short	m_sAmbient;							// TRUE, if ambient (i.e., non-essential) sound.

		long	m_lCollectiveVolume;				// Collective volume from this object and
														// its child satellites.
														
	protected:

		static short	ms_sFileCount;			// File count.
		static long		ms_lGetRandomSeed;	// Seed for get random.
														
	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CSoundThing(CRealm* pRealm)
			: CThing(pRealm, CSoundThingID)
			{
			m_bInitiallyEnabled = true;
			m_bInitiallyRepeats = false;
			m_lMinTime[0] = 0;
			m_lMinTime[1] = 0;
			m_lRndTime[0] = 0;
			m_lRndTime[1] = 0;
			m_szResName[0] = 0;

			m_id.pszId = m_szResName;
			m_id.usDescFlags	= SMDF_NO_DESCRIPT;

			m_pImage = 0;

			m_lLastStartTime = 0;
			m_lNextStartTime = 0;
			m_sWhichTime = -1;
			m_bEnabled = m_bInitiallyEnabled;
			m_bRepeats = m_bInitiallyRepeats;

			m_sSuspend = 0;

			m_siChannel	= 0;

			m_state	= State_Happy;

			m_lVolumeHalfLife	= 1000;

			m_sUseLooping			= FALSE;
			m_lStopLoopingTime	= 0;
			m_lNumLoopBacks		= 0;	
			m_lLoopBackTo			= 0;		
			m_lLoopBackFrom		= 0;	

			m_sPurgeSampleWhenDone	= FALSE;

			m_sAmbient				= TRUE;

			m_lCollectiveVolume	= 0;
			}

	public:
		// Destructor
		~CSoundThing()
			{
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
			*ppNew = new CSoundThing(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CExplode::Construct(): Couldn't construct CSoundThing (that's a bad thing)\n");
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

		// Render object
		void Render(void);

		short Setup(												// Returns 0 on success.
			short sX,												// In: New x coord
			short sY,												// In: New y coord
			short sZ);												// In: New z coord

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to get the clickable pos/area of an object in 2D.
		virtual	// Overridden here.
		void EditRect(				// Returns nothiing.
			RRect*	prc);			// Out: Clickable pos/area of object.

		// Called by editor to get the hotspot of an object in 2D.
		virtual	// Overridden here.
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

	//---------------------------------------------------------------------------
	// External functions
	//---------------------------------------------------------------------------
	public:
		// Relay the volume to add to this CSoundThing's collective volume.
		void RelayVolume(	// Returns nothing.
			long lVolume);	// In:  Volume to relay.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Init object
		short Init(void);											// Returns 0 if successfull, non-zero otherwise
		
		// Kill object
		short Kill(void);											// Returns 0 if successfull, non-zero otherwise

		// Process our message queue.
		void ProcessMessages(void);

		// Don't call this from outside of CSoundThing.  It should affect only
		// CSoundThing stuff.
		static long GetRandom(void);
		static long GetRand(void)
			{
			return GetRandom();
			}

	};


#endif // SOUNDTHING_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
