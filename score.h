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
// Score.h
// Project: Postal
// 
// History:
//
//		06/11/97 BRH	Started this module
//
//		06/16/97 BRH	Added a scoreboard reset function to reset the scores
//							and display time.
//
//		06/17/97 BRH	Increased max number of players to make sure there are
//							enough entries for 16 or more players.
//
//		07/04/97 BRH	Added two more scoring modes.
//
//		07/07/97	JMI	Now ScoreUpdateDisplay() returns true if it drew into
//							the passed in buffer and false otherwise.
//
//		07/22/97 BRH	Added ScoreDisplayHighScores.
//
//		07/26/97 BRH	Added ScoreGetName to get the name of the player when 
//							they better one of the high scores.
//
//		08/10/97	JRD	Added a hood to the score status to allow graphical
//							backdrops behind the scoring status.
//
//		09/07/97	JMI	Now requires a pclient for MP scores.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SCORE_H
#define SCORE_H

#include "RSPiX.h"
#include "game.h"
#include "realm.h"
#include "net.h"
#include "netclient.h"


class CScoreboard
{
	//---------------------------------------------------------------------------
	// typedefs
	//---------------------------------------------------------------------------
	public:
		
		typedef enum
		{
			SinglePlayer,
			MultiPlayer,
			Timed,
			Goal,
			CaptureFlag,
			TimeBonus
		} ScoringMode;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		
		short	m_asScores[Net::MaxNumIDs+1];			// Score for each player
//		U16	m_au16PlayerIDs[Net::MaxNumIDs+1];	// ID of each player
		long	m_lLastScoreDrawTime;						// Time since last update
		long	m_lLastStatusDrawTime;						// Time since last update

	protected:
		ScoringMode m_ScoringMode;						// Mode of scoring

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:

		CScoreboard()
		{
			short i;
			for (i = 0; i <= Net::MaxNumIDs; i++)
			{
				m_asScores[i] = 0;
//				m_au16PlayerIDs[i] = 0;
			}
		}

		~CScoreboard()
		{

		}

	//---------------------------------------------------------------------------
	// Functions
	//---------------------------------------------------------------------------
	public:

		// Return the indes of the player or -1 if not found
//		short GetPlayerIndex(U16 uInstanceID)
//		{
//			short sPlayerIndex = Net::MaxNumIDs;
 //
//			CThing* pShooter = GetItemById(uInstanceID)
//			if (pShooter && pShooter->m_id == CThing::CDudeID)
//			{
//				sPlayerIndex = MIN(((CDude*) pShooter)->m_sDudeNum, Net::MaxNumIDs);
//			}	
//		}

		void Reset(void)
		{
			short i;
			for (i = 0; i < Net::MaxNumIDs; i++)
				m_asScores[i] = 0;

			m_lLastStatusDrawTime = 0;
			m_lLastScoreDrawTime = 0;
		}
		
		// Subtract one from the score of the indicated guy
		void SubtractOne(short sPlayerIndex)
		{
			// If it is beyond the number of players, set it to the
			// overflow bin.
			if (sPlayerIndex > Net::MaxNumIDs)
				sPlayerIndex = Net::MaxNumIDs;

			m_asScores[sPlayerIndex]--;
		}		


		// Add one to the score of the indicated guy
		void AddOne(short sPlayerIndex)
		{
			// If it is beyond the number of players, set it to the
			// overflow bin.
			if (sPlayerIndex > Net::MaxNumIDs)
				sPlayerIndex = Net::MaxNumIDs;

			m_asScores[sPlayerIndex]++;
		}		

		void SetScoringMode(ScoringMode Mode)
			{m_ScoringMode = Mode;};

		short GetScoringMode(void)
			{return m_ScoringMode;};
	
};

// Set up the RPrint for the score
void ScoreInit(void);

// Reset the display time and the multiplayer scores
void ScoreReset(void);

// Reset the display time - required before each realm
void ScoreResetDisplay(void);

// Function called by Characters when they die
void ScoreRegisterKill(CRealm* pRealm, U16 u16DeadGuy, U16 u16Killer);

// Function called by play to update the score display
// Returns true, if pImage was updated; false otherwise.
bool ScoreUpdateDisplay(RImage* pImage, RRect* pRect, CRealm* pRealm, 
								CNetClient* pclient, short sDstX, short sDstY, CHood* pHood);

// Function to set the scoring mode (ie multiplayer, single, timed etc)
void ScoreSetMode(CScoreboard::ScoringMode Mode);

// Call this function to show the status line or mission goal for a few
// seconds.  This can be called when the "show mission" key is pressed.
void ScoreDisplayStatus(CRealm* pRealm);

// Call this function at the end of the level and if it is a challenge level,
// it will display the high scores for that level and give the user a chance
//	to enter their name if they beat one of the top scores.
void ScoreDisplayHighScores(			// Returns nothing.
	CRealm* pRealm,						// In:  Realm won.
	CNetClient* pclient	= NULL,		// In:  Client ptr for MP mode, or NULL in SP mode.
	long lMaxTimeOut	= -1);			// In:  Max time on score screen (quits after this
												// duration, if not -1).

// Get the name for a new high score
short ScoreGetName(char* pszName);

// Returns the highest multiplayer score.
short ScoreHighestKills(CRealm* pRealm);

#endif //SCORE_H

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

