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
// Demon.H
// Project: Postal
// 
// History:
//		06/09/97 BRH	Started this file from SoundThing.h
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/17/97	JMI	Changed m_psndChannel to m_siLastPlayInstance.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//							Then, removed m_psndChannel b/c this class really didn't
//							use it.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		08/01/97	JMI	Demon would set his position (m_dX, Y, Z) when first 
//							created but, since he never Save()d or Load()ed it, he
//							was in an unitialized position when loaded into a level.
//
//		09/24/97	JMI	Now initializes bFemalePain member of m_id.  This member
//							indicates whether the sample is of a female in pain which
//							some countries (so far just UK) don't want in the game.
//
//		10/07/97	JMI	Changed bFemalePain to usDescFlags.
//
//		12/02/97	JMI	Added new m_sSoundBank member to index new static arrays
//							of sound banks for groups of sounds (ms_apsmidExplosion,
//							ms_apsmidBurn, ms_apsmidSuicide, ms_apsmidWrithing,
//							ms_apsmidKillSeries).
//							Also, removed unused vars:
//							m_lNextStartTime, m_lLastStartTime, m_sWhichTime, 
//							m_bEnabled, m_bRepeats, m_bInitiallyEnabled, 
//							m_bInitiallyRepeats, m_lMinTime[], m_lRndTime[], 
//							m_szResName, and m_id.
//							Also, now saves position and defaults to position on the
//							the screen (that way older levels that didn't save the
//							position will have the demon on the screen).
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will get messages from other things in the game
//	and decide what comment to make about it.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef DEMON_H
#define DEMON_H

#include "RSPiX.h"
#include "realm.h"
#include "SampleMaster.h"


class CDemon : public CThing
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

		typedef enum
			{
			NumSoundBanks = 5,			// Number of sound banks.
			NumExplosionComments = 8,	// Number of explosion comments per bank.
			NumBurnComments = 4,			// Number of burn comments per bank.
			NumSuicideComments = 1,		// Number of suicide comments per bank.
			NumWrithingComments = 5,	// Number of writhing comments per bank.
			NumKillSeriesComments = 7	// Number of kill series comments per bank.
			} Macros;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		long m_lIdleTime;							// Time without saying something
		long m_lKillTimer;						// Bonus kill timer
		short m_sRecentKills;					// Number of recent kills
		short m_sCommentCount;					// Number of comments that could have 
														// been made, but were withheld.

		RImage* m_pImage;							// Pointer to only image (for editor only)
		CSprite2 m_sprite;						// Sprite (for editor only)
		double m_dX;								// x coord (for editor only)
		double m_dY;								// y coord (for editor only)
		double m_dZ;								// z coord (for editor only)

		short m_sSuspend;							// Suspend flag

		State	m_state;								// Current state.

		short	m_sSoundBank;						// Sound bank index.
														
	protected:

		static long ms_lMinIdleTime;			// Min time before playing next sample
		static long ms_lBonusKillTime;		// Kill an amount within this time and get a bonus comment
		// Sound banks of explosion comments indexed by m_sSoundBank.
		static SampleMasterID* ms_apsmidExplosion[NumSoundBanks][NumExplosionComments];
		// Sound banks of burn comments indexed by m_sSoundBank.
		static SampleMasterID* ms_apsmidBurn[NumSoundBanks][NumBurnComments];			
		// Sound banks of suicide comments indexed by m_sSoundBank.
		static SampleMasterID* ms_apsmidSuicide[NumSoundBanks][NumSuicideComments];	
		// Sound banks of writhing comments indexed by m_sSoundBank.
		static SampleMasterID* ms_apsmidWrithing[NumSoundBanks][NumWrithingComments];
		// Sound banks of kill series comments indexed by m_sSoundBank.
		static SampleMasterID* ms_apsmidKillSeries[NumSoundBanks][NumKillSeriesComments];
														
	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
		// Constructor
		CDemon(CRealm* pRealm)
			: CThing(pRealm, CDemonID)
			{
			m_lIdleTime = 0;
			m_lKillTimer = 0;
			m_sRecentKills = 0;
			m_sCommentCount = 0;

			m_pImage = 0;

			m_sSuspend = 0;

			m_state	= State_Happy;

			// Default to position on the screen for old .rlms that did not
			// save their demon's position.
			m_dX		= 100.0;
			m_dY		= 0.0;
			m_dZ		= 50.0;

			m_sSoundBank	= 0;
			}

	public:
		// Destructor
		~CDemon()
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
			*ppNew = new CDemon(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CExplode::Construct(): Couldn't construct CDemon (that's a bad thing)\n");
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
	// Optional Static  functions
	//---------------------------------------------------------------------------

		// Preload the sound samples that might be used.
		static short Preload(
			CRealm* prealm);				// In:  Calling realm.

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
	};


#endif // DEMON_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
