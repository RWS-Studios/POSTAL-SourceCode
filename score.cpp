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
// Score.cpp
// Project: Postal
//
//	Description:
//		This module will be used to collect and display the scoring for the
//		game.  Characters that die will call a function to register the kill
//		giving their ID and the ID of the character that killed them.  Play
//		will call a function to update the display for the score and depending
//		on the mode of scoring, this module will draw the score into the given
//		image.  The first scoring mode will be for multiplayer where all 8
//		players will have the number of kills displayed with their names.  
//		Single player score may show something like Number of victims and
//		number of enemies.  A timed challenge level may show the kills and the
//		time remaining.  
// 
// History:
//		
//		06/11/97 BRH	Started this module for scoring of different types.  
//
//		06/12/97 BRH	Added Scoring display for multiplayer and moved the 
//							single player score from Realm to the same display
//							function here in score.
//
//		06/13/97 BRH	Fixed the formatting for multiplayer mode.
//
//		06/15/97 BRH	Took out temporary code that forced it into multiplayer
//							display for testing.  Put back the normal code to 
//							work for either single or multiplayer mode.
//
//		06/16/97 BRH	Added a reset function to reset the scores and 
//							display time.
//
//		06/17/97 BRH	Fixed the display area for the score.
//
//		06/26/97 BRH	Changed the score so that the population number goes
//							down when someone is killed.
//
//		06/29/97 BRH	Separated the score and status lines so that the mission
//							goal can be displayed for a short period of time while
//							the score displays constantly.
//
//		07/04/97 BRH	Added scoring displays for some of the other types
//							of games, timed, goal, capture the flag, etc.
//
//		07/06/97 BRH	Added scoring displays and staus lines for the challenge
//							levels.
//
//		07/07/97	JMI	Now ScoreUpdateDisplay() returns true if it drew into
//							the passed in buffer and false otherwise.
//
//		07/08/97 BRH	Changed the check for scoring mode from the scoreboard
//							to the realm's enumerated type.
//
//		07/14/97 BRH	Fixed clock format to use %2.2d to correctly display
//							2 digits for seconds.
//
//		07/22/97 BRH	Added ScoreDisplayHighScores function which is called
//							from Play.cpp after the goal level has been met.  This
//							will check the player's score against the top 5 stored
//							scores for the level and if they place, it will allow
//							them to enter their name.  The initial version of this
//							will just skip the dialog for now, but will soon
//							do a switch statement for each scoring mode.
//
//		07/26/97 BRH	Added loading and saving of high scores in a prefs type
//							file.  Now the only thing that needs to be changed is
//							to use the function that returns the realm name or
//							descriptive string which will be used as the section
//							name to find the scores particular to this realm.
//
//		07/27/97 BRH	Made use of the new realm variable m_lScoreIntialTime 
//							to calculate the elapsed time for two of the scoring
//							modes.
//
//		07/30/97 BRH	Used the realm's string to identify the current realm
//							being played so that the high scores for this realm
//							can be loaded and saved.
//
//		08/10/97	JRD	Modified ScoreUpdateDisplay to accept a hood pointer so
//							it could create a graphical back drop.
//
//		09/10/97	JRD	Added color matching and shadow drop to the upper score bar.
//
//		08/14/97 BRH	Changed location of gui for high scores, moved from 
//							/game/ to /shell directory so that the textures used
//							for it could be shared easier with the other shell gui's
//
//		08/17/97 MJR	Now uses g_resmgrShell to load gui's.  Also now frees
//							the resources (ooooohhh Bill!)
//
//		08/19/97 BRH	Fixed the problem with checkpoint scoring, probably 
//							some typo that Mike put in by accident.
//
//		08/19/97	JMI	There was a missing '}' ala Mike via Bill.
//
//		08/20/97 BRH	No thanks to Mike or Jon, I had to fix the multiplayer
//							score displays so that new time information would
//							fit on a third line.
//
//		08/25/97	JMI	Moved gsStatus* to toolbar.h.
//
//		08/28/97	JMI	Added GetRes/ReleaseRes callbacks to dialogs, PalTrans,
//							Postal Font, and sound to name edit field.
//
//		08/29/97 BRH	Changed challenge modes to use Population numbers
//							rather than just hostile numbers so that the victims
//							count also.
//
//		08/29/97	JMI	Now ScoreDisplayHighScores() quits immediately for
//							scoring modes that don't track high scores (currently,
//							only standard).
//
//		08/30/97	JMI	Now uses population deaths instead of hostile deaths
//							for timed challenge scoring.
//							Also, sets the palette every time the dialog is shown.
//
//		09/01/97	JMI	Now your high score is entered in the high score dialog
//							itself.  Also, now uses a listbox for high scores so
//							that we could edit merely one high score item and fill
//							the listbox with instances of that same item.
//
//		09/01/97	JMI	Now displays new highscore in different color.
//
//		09/02/97	JMI	Now ScoreDisplayHighScores() makes sure you're not playing
//							a demo before displaying the highscores.
//
//		09/02/97	JMI	Now has separate cases for determing scoring for every
//							mode in ScoreDisplayHighScores().
//
//		09/04/97 BRH	Fixed font size for the MPFrag scoring mode.
//
//		09/07/97	JMI	Added ability to display high scores for multiplayer.
//							Also, added optional high score display timeout.
//
//		09/07/97 BRH	Fixed problem with long multi player names wrapping
//							to the next line.
//
//		09/07/97	JMI	Now colors this player's score in MP mode.  Also, dialog
//							is much smaller so name length is restricted more.  But
//							does allow up to 16 scores now.
//
//		09/08/97 BRH	Fixed Goal and TimedGoal to display the Remaining: as
//							the number remaining in the goal, not the number of
//							people remaining in the realm.
//
//		09/08/97 BRH	Adjusted the posiiton of the MP mission goals since
//							the font size is smaller, they weren't centered properly.
//
//		09/08/97	JMI	Changed "Congratulations! Please enter your name for your 
//							score." to "Please enter your name, Asshole." but the end
//							clips off (just kidding).
//
//		09/08/97	JMI	Now sorts multiplayer names by score.
//
//		06/04/98	JMI	Strings used for storage and sorting of multiplayer names
//							were sized at MAX_PLAYER_NAME_LEN (to fit the high score
//							GUI without overlapping the score) but Postal's net 
//							client (from which the names are querried) allows longer
//							names (specifically Net::MaxPlayerNameSize).  Now only
//							copies first MAX_PLAYER_NAME_LEN chars from the player's
//							name.
//
//////////////////////////////////////////////////////////////////////////////

#define SCORE_CPP

//////////////////////////////////////////////////////////////////////////////
// C Headers
//////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "score.h"
#include "dude.h"
#include "toolbar.h"
#include "update.h"

//////////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////////

// Time, in ms, between status updates.
#define SCORE_UPDATE_INTERVAL			1000
#define STATUS_DISPLAY_TIMEOUT		8000

#define STATUS_PRINT_X					2
#define STATUS_PRINT_Y					0
#define STATUS_PRINT_Y2					14
#define STATUS_PRINT_Y3					28

#define STATUS_FONT_SIZE				19
#define STATUS_DISPLAY_HEIGHT			40

#define MP_FONT_SIZE						11 //15
#define MP_PRINT_X						2
#define MP_PRINT_Y1						0
#define MP_PRINT_Y2						11
#define MP_PRINT_Y3						22

//------------------------ These are color matched in the toolbar module

#define HIGHSCORE_NAMEDIALOG_FILE	"menu/addname.gui"
#define HIGHSCORE_DIALOG_FILE			"menu/hiscore.gui"
#define HIGHSCORE_ITEM_FILE			"menu/HiScoreItem.gui"

#define HIGHSCORE_SCORES_FILE			"res/savegame/high.ini"

#define TEXT_SHADOW_COLOR				220

#define TEXT_HIGHLIGHT_COLOR			9

// This path gets prepended to the resource path before passing the
// GUI res request to the resource manager.  This makes it easy to
// keep the path for the file simple while in the GUI Editor so the default
// handling can take care of it there.  The GUI Editor does not use the resmgr,
// it simply loads it blindly with no regard to its current directory.
#define GUI_RES_DIR						"menu/"

// Maximum name length.
#define MAX_PLAYER_NAME_LEN			17


#define MAX_HIGH_SCORES					16

//////////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////////

CScoreboard g_scoreboard;
RPrint		ms_print;

static long	ms_lScoreMaxTimeOut;		// Optional score timeout (max time spent
												// on score screen).

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Helper to make a time value.
//////////////////////////////////////////////////////////////////////////////
inline char* CreateTimeString(	// Returns time string.  No failures.
	long	lTimeVal)					// In:  Time value in milliseconds.
	{
	static char	szTime[100];

	short sMinutes = lTimeVal / 60000;
	short	sSeconds = (lTimeVal / 1000) % 60;
	sprintf(szTime, "%2.2d:%2.2d", sMinutes, sSeconds);

	return szTime;
	}


//////////////////////////////////////////////////////////////////////////////
// GuiReleaseRes - Release a resource that the requesting GUI wants to 
// discard.
//////////////////////////////////////////////////////////////////////////////

static void GuiReleaseRes(	// Returns nothing.
	RGuiItem* pgui)			// In:  Requesting GUI.
	{
	rspReleaseResource(&g_resmgrShell, &pgui->m_pimBkdRes);
	}

//////////////////////////////////////////////////////////////////////////////
// GuiGetRes - Get a resource that the requesting GUI wants to use for its
// background or state image.
//////////////////////////////////////////////////////////////////////////////

static short GuiGetRes(		// Returns 0 on success; non-zero on failure.
	RGuiItem* pgui)			// In:  Requesting GUI.
	{
	short sResult = 0;

	// Release resources first (just in case)
	GuiReleaseRes(pgui);

	// Allocate and load new resources.  We get the name of the file (which
	// is ASSUMED to have NO PATH!!) from the gui itself, then tack on the
	// path we need and get the resource from the resource manager.
	char szFile[RSP_MAX_PATH * 2];
	sprintf(szFile, "%s%s", GUI_RES_DIR, pgui->m_szBkdResName);

	if (rspGetResource(&g_resmgrShell, szFile, &pgui->m_pimBkdRes) == 0)
		{
		// Set palette via resource.
		ASSERT(pgui->m_pimBkdRes->m_pPalette != NULL);
		ASSERT(pgui->m_pimBkdRes->m_pPalette->m_type == RPal::PDIB);

		rspSetPaletteEntries(
			0,
			230,
			pgui->m_pimBkdRes->m_pPalette->Red(0),
			pgui->m_pimBkdRes->m_pPalette->Green(0),
			pgui->m_pimBkdRes->m_pPalette->Blue(0),
			pgui->m_pimBkdRes->m_pPalette->m_sPalEntrySize);

		// Update hardware palette.
		rspUpdatePalette();
		}
	else
		{
		sResult = -1;
		TRACE("GuiGetRes(): Failed to open file '%s'\n", szFile);
		}

	return sResult;
	}

//////////////////////////////////////////////////////////////////////////////
// EditInputUserFeedback -- Whines when the user causes an input 
// disgruntlement.
//////////////////////////////////////////////////////////////////////////////

static void EditInputUserFeedback(	// Called when a user input notification
												// should occur.
	REdit*	pedit)						// In:  Edit field.
	{
	PlaySample(g_smidEmptyWeapon, SampleMaster::UserFeedBack);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from RProcessGui for system update.
//
////////////////////////////////////////////////////////////////////////////////
static long SysUpdate(			// Returns a non-zero ID to abort or zero
										// to continue.                          
	RInputEvent*	pie)			// Out: Next input event to process.     
	{
	long	lIdRes	= 0;	// Assume no GUI ID pressed (i.e., continue).

	UpdateSystem();
	rspGetNextInputEvent(pie);

	// If timeout has expired . . .
	if (rspGetMilliseconds() > ms_lScoreMaxTimeOut)
		{
		// Auto push OK.
		lIdRes	= 1;
		}

	return lIdRes;
	}

//////////////////////////////////////////////////////////////////////////////
// ScoreInit - Set up the RPrint for the score
//////////////////////////////////////////////////////////////////////////////

void ScoreInit(void)
{
	// Setup print.
	ms_print.SetFont(STATUS_FONT_SIZE, &g_fontBig);
	ms_print.SetColor(
		gsStatusFontForeIndex, 
		gsStatusFontBackIndex, 
		gsStatusFontShadowIndex);

	// Make sure shadow is one off:
	ms_print.SetEffectAbs(RPrint::SHADOW_X,+1);
	ms_print.SetEffectAbs(RPrint::SHADOW_Y,+1);

	// Warning!  This may need to be set back again for gui items - if so, use 
	// my OWN print!

}

//////////////////////////////////////////////////////////////////////////////
// ScoreReset - Reset the scores and the display time
//////////////////////////////////////////////////////////////////////////////

void ScoreReset(void)
{
	g_scoreboard.Reset();
}

//////////////////////////////////////////////////////////////////////////////
// ScoreResetDisplay - Reset the timer for the display before the start 
//							  of each realm
//////////////////////////////////////////////////////////////////////////////

void ScoreResetDisplay(void)
{
	g_scoreboard.m_lLastStatusDrawTime = 0;
	g_scoreboard.m_lLastScoreDrawTime = 0;
}

//////////////////////////////////////////////////////////////////////////////
// ScoreRegisterKill - Characters should call this when they die
//////////////////////////////////////////////////////////////////////////////

void ScoreRegisterKill(CRealm* pRealm, U16 u16DeadGuy, U16 u16Killer)
{
	CThing* pShooter = NULL;
	pRealm->m_idbank.GetThingByID(&pShooter, u16Killer);
	if (pShooter && pShooter->GetClassID() == CThing::CDudeID)
	{
		if (u16DeadGuy == u16Killer)
			g_scoreboard.SubtractOne(((CDude*) pShooter)->m_sDudeNum);
		else
			g_scoreboard.AddOne(((CDude*) pShooter)->m_sDudeNum);
	}
}

//////////////////////////////////////////////////////////////////////////////
// ScoreUpdateDisplay - Display the score for the current mode
// Returns true, if pImage was updated; false otherwise.
//////////////////////////////////////////////////////////////////////////////

bool ScoreUpdateDisplay(RImage* pim, RRect* prc, CRealm* pRealm, CNetClient* pclient,
								short sDstX,short sDstY,CHood* pHood)
{
	RRect rcBox;
	RRect	rcDst;
	rcDst.sX	= prc->sX + STATUS_PRINT_X;
	rcDst.sY = prc->sY + STATUS_PRINT_Y;
	rcDst.sW = prc->sW - 2 * STATUS_PRINT_X;
	rcDst.sH	= prc->sH - STATUS_PRINT_Y;
	short sMinutes = pRealm->m_lScoreTimeDisplay / 60000;
	short sSeconds = (pRealm->m_lScoreTimeDisplay / 1000) % 60;
	bool	bDrew	= false;	// Assume we do not draw.

	long	lCurTime	= pRealm->m_time.GetGameTime();
	if (lCurTime > g_scoreboard.m_lLastScoreDrawTime + SCORE_UPDATE_INTERVAL)
	{
		ms_print.SetColor(	// set current color
		gsStatusFontForeIndex, 
		gsStatusFontBackIndex, 
		gsStatusFontShadowIndex);

		// Set print/clip to area.
		// instead of clearing, drop the special backdrop.
		//rspRect(RSP_BLACK_INDEX, pim, rcDst.sX, rcDst.sY, rcDst.sW, rcDst.sH);

		if (pHood) rspBlit(pHood->m_pimTopBar,pim,0,0,sDstX,sDstY,
			pHood->m_pimTopBar->m_sWidth,pHood->m_pimTopBar->m_sHeight);
		
		short sNumDudes = pRealm->m_asClassNumThings[CThing::CDudeID];	
		short i;

		switch (pRealm->m_ScoringMode)
		{
			case CRealm::MPFrag:
				ms_print.SetFont(MP_FONT_SIZE, &g_fontBig);
				rcBox.sY = prc->sY + MP_PRINT_Y1;
				rcBox.sH = MP_FONT_SIZE + 1;
				rcBox.sW = rcDst.sW / sNumDudes;

				ms_print.SetJustifyLeft();
				ms_print.SetWordWrap(FALSE);
				for (i = 0; i < sNumDudes; i++)
				{
					rcBox.sX = rcDst.sX + (i * rcBox.sW);
					rcBox.sY = prc->sY + STATUS_PRINT_Y;
					ms_print.SetDestination(pim, &rcBox);
					ms_print.print(
						rcBox.sX,
						rcBox.sY,
						"%s",
						pclient->GetPlayerName(i)
						);

					rcBox.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcBox);
					ms_print.print(
						rcBox.sX,
						rcBox.sY,
						"%d",
						g_scoreboard.m_asScores[i]
						);
				}
				break;

			// These have the same display, but different ending conditions
			case CRealm::MPTimed:
			case CRealm::MPTimedFrag:
				ms_print.SetFont(MP_FONT_SIZE, &g_fontBig);
				rcBox.sY = prc->sY + MP_PRINT_Y1;
				rcBox.sH = MP_FONT_SIZE + 1;
				rcBox.sW = rcDst.sW / sNumDudes;

				ms_print.SetJustifyLeft();
				ms_print.SetWordWrap(FALSE);
				for (i = 0; i < sNumDudes; i++)
				{
					rcBox.sX = rcDst.sX + (i * rcBox.sW);
					rcBox.sY = prc->sY + MP_PRINT_Y1;
					ms_print.SetDestination(pim, &rcBox);
					ms_print.print(
						rcBox.sX,
						rcBox.sY,
						"%s",
						pclient->GetPlayerName(i)
						);

					rcBox.sY = prc->sY + MP_PRINT_Y2;
					ms_print.SetDestination(pim, &rcBox);
					ms_print.print(
						rcBox.sX,
						rcBox.sY,
						"%d",
						g_scoreboard.m_asScores[i]
						);

				}
				rcBox.sY = prc->sY + MP_PRINT_Y3;
				rcBox.sX = prc->sX;
				ms_print.SetDestination(pim, &rcBox);
				ms_print.print(
					rcDst.sX,
					rcDst.sY,
//					"Time Remaining %d:%2.2d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],					
					sMinutes,
					sSeconds);
				break;

			case CRealm::MPLastMan:
				rcBox.sY = prc->sY + STATUS_PRINT_Y;
				rcBox.sH = STATUS_FONT_SIZE + 1;
				rcBox.sW = rcDst.sW / sNumDudes;

				ms_print.SetJustifyLeft();
				ms_print.SetWordWrap(FALSE);
				for (i = 0; i < sNumDudes; i++)
				{
					// If this player is dead, set the color to dead color
					/*
						ms_print.SetColor(	// set current color
						gsStatusFontForeDeadIndex, 
						gsStatusFontBackIndex, 
						gsStatusFontShadowIndex);

							else

						ms_print.SetColor(	// set current color
						gsStatusFontForeIndex, 
						gsStatusFontBackIndex, 
						gsStatusFontShadowIndex);
					*/

					rcBox.sX = rcDst.sX + (i * rcBox.sW);
					rcBox.sY = prc->sY + STATUS_PRINT_Y;
					ms_print.SetDestination(pim, &rcBox);
					ms_print.print(
						rcBox.sX,
						rcBox.sY,
						"%s",
						pclient->GetPlayerName(i)
						);

					rcBox.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcBox);
					ms_print.print(
						rcBox.sX,
						rcBox.sY,
						"%d",
						g_scoreboard.m_asScores[i]
						);
				}
				break;

			case CRealm::MPLastManTimed:
				break;

			case CRealm::MPLastManFrag:
				break;

			case CRealm::MPLastManTimedFrag:
				break;

			case CRealm::Standard:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim, 
					rcDst.sX, 
					rcDst.sY, 
//					"      Population %d                        Hostiles %d   Killed %d (%d%% / %d%%)",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],
					pRealm->m_sPopulation,
					pRealm->m_sHostiles,
					pRealm->m_sHostileKills,
					pRealm->m_sHostileKills * 100 / ((pRealm->m_sHostileBirths != 0) ? pRealm->m_sHostileBirths : 1),
					(short) pRealm->m_dKillsPercentGoal
					);
				break;

			case CRealm::Timed:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim,
					rcDst.sX,
					rcDst.sY,
//					" Time Remaining %d:%2.2d                                Kills %d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],
					sMinutes,
					sSeconds,
					pRealm->m_sPopulationDeaths
					);
				break;

			case CRealm::TimedGoal:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim,
					rcDst.sX,
					rcDst.sY,
//					" Time Remaining %d:%2.2d            Kills %d               Remaining %d / %d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],
					sMinutes,
					sSeconds,
					pRealm->m_sPopulationDeaths,
					pRealm->m_sKillsGoal - pRealm->m_sPopulationDeaths, pRealm->m_sPopulation
					);
				break;

			case CRealm::TimedFlag:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim,
					rcDst.sX,
					rcDst.sY,
//					" Time Remaining %d:%2.2d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],
					sMinutes,
					sSeconds
					);
					break;

			case CRealm::Goal:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim,
					rcDst.sX,
					rcDst.sY,
//					" Kills %d                     Remaining %d            Time Elapsed %d:%2.2d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],
					pRealm->m_sPopulationDeaths,
					pRealm->m_sKillsGoal - pRealm->m_sPopulationDeaths /*pRealm->m_sPopulation*/,
					sMinutes,
					sSeconds
					);
				break;

			case CRealm::CaptureFlag:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim,
					rcDst.sX,
					rcDst.sY,
//					" Time Elapsed %d:%2.2d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode], 
					sMinutes,
					sSeconds
					);
				break;

			case CRealm::Checkpoint:
				ms_print.SetDestination(pim, &rcDst);
				ms_print.print(
					pim,
					rcDst.sX,
					rcDst.sY,
//					" Clock %d:%2.2d    You have %d flags    Flags Remaining %d",
					g_apszScoreDisplayText[pRealm->m_ScoringMode],
					sMinutes,
					sSeconds,
					pRealm->m_sFlagsCaptured,
					pRealm->m_asClassNumThings[CThing::CFlagID] - pRealm->m_sFlagsCaptured
					);
				break;

			default:
				break;

		}
		g_scoreboard.m_lLastScoreDrawTime	= lCurTime;

		// Display the status or mission statement line
		if (lCurTime < g_scoreboard.m_lLastStatusDrawTime + STATUS_DISPLAY_TIMEOUT)
		{
			switch (pRealm->m_ScoringMode)
			{
				case CRealm::MPFrag:
					rcDst.sY = prc->sY + MP_PRINT_Y3;
					if (pRealm->m_sKillsGoal < 1)
					{
						rcDst.sX = prc->sX + 0;
						ms_print.SetDestination(pim, &rcDst);
						ms_print.print(
							rcDst.sX,
							rcDst.sY,
//							" There are no time or kill limits on this game - play as long as you like"
							g_apszScoreGoalText[CRealm::MPLastManTimedFrag] // Cheating since this is
																							// Really none of the scoring modes

						);
					}
					else
					{
						rcDst.sX = prc->sX + 220;
						ms_print.SetDestination(pim, &rcDst);
						ms_print.print(
							rcDst.sX,
							rcDst.sY,
//							" The first player to get %d kills wins",
							g_apszScoreGoalText[pRealm->m_ScoringMode],
							pRealm->m_sKillsGoal
						);
					}

					break;

				case CRealm::MPTimed:
					rcDst.sY = prc->sY + MP_PRINT_Y3;
					rcDst.sX = prc->sX + 180;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" The player with the most kills when time expires is the winner"
						g_apszScoreGoalText[pRealm->m_ScoringMode]
						);
					break;

				case CRealm::MPTimedFrag:
					rcDst.sY = prc->sY + MP_PRINT_Y3;
					rcDst.sX = prc->sX + 220;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Try to reach %d kills before time expires",
						g_apszScoreGoalText[pRealm->m_ScoringMode],
						pRealm->m_sKillsGoal
						);
					break;

				case CRealm::Standard:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;

					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						"      You must kill %d%% of the hostiles.",
						g_apszScoreGoalText[pRealm->m_ScoringMode],
						(short) pRealm->m_dKillsPercentGoal
						);
					break;

				case CRealm::Timed:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Score as many kills as possible in the time remaining."
						g_apszScoreGoalText[pRealm->m_ScoringMode]
						);
					break;

				case CRealm::TimedGoal:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Kill everyone before time runs out."
						g_apszScoreGoalText[pRealm->m_ScoringMode]
						);
					break;

				case CRealm::TimedFlag:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Capture the flag before time runs out."
						g_apszScoreGoalText[pRealm->m_ScoringMode]
						);
						break;

				case CRealm::Goal:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Kill %d People in as little time as possible.",
						g_apszScoreGoalText[pRealm->m_ScoringMode],
						pRealm->m_sKillsGoal
						);
					break;

				case CRealm::CaptureFlag:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Capture the flag in as little time as possible."
						g_apszScoreGoalText[pRealm->m_ScoringMode]
						);
					break;

				case CRealm::Checkpoint:
					rcDst.sY = prc->sY + STATUS_PRINT_Y2;
					ms_print.SetDestination(pim, &rcDst);
					ms_print.print(
						rcDst.sX,
						rcDst.sY,
//						" Grab as many flags as possible before time runs out."
						g_apszScoreGoalText[pRealm->m_ScoringMode]
						);
					break;

				default:
					break;
			}
		}


		// Note that we drew.
		bDrew	= true;
	}

	return bDrew;
}

//////////////////////////////////////////////////////////////////////////////
// ScoreDisplayStatus
//
//		Sets the status display timer so it will show for a few seconds
//////////////////////////////////////////////////////////////////////////////

void ScoreDisplayStatus(CRealm* pRealm)
{
	g_scoreboard.m_lLastStatusDrawTime = pRealm->m_time.GetGameTime();
}

//////////////////////////////////////////////////////////////////////////////
// ScoreSetMode - Set mode of scoring
//////////////////////////////////////////////////////////////////////////////

void ScoreSetMode(CScoreboard::ScoringMode Mode)
{
	g_scoreboard.SetScoringMode(Mode);
}


//////////////////////////////////////////////////////////////////////////////
// ScoreDisplayHighScores
//////////////////////////////////////////////////////////////////////////////

void ScoreDisplayHighScores(	// Returns nothing.
	CRealm* pRealm,				// In:  Realm won.
	CNetClient* pclient,			// In:  Client ptr for MP mode, or NULL in SP mode.
	long lMaxTimeOut)				// In:  Max time on score screen (quits after this
										// duration, if not -1).
{
	RGuiItem* pguiRoot;
	RGuiItem::ms_print.SetFont(15, &g_fontPostal);
	RProcessGui guiDialog;
	short sResult;
	char szScoringExplanation[512]	= "";
	long alScores[MAX_HIGH_SCORES];
	char astrNames[MAX_HIGH_SCORES][MAX_PLAYER_NAME_LEN + 1];
	char szKeyName[256];
	RPrefs scores;

	typedef enum
		{
		Value,
		Time
		} ValType;

	ValType	vtScoringUnit	= Value;


	// Let's just not do any of this for modes that have no scoring . . .
	if (pRealm->m_ScoringMode >= CRealm::Timed && pRealm->m_ScoringMode <= CRealm::MPLastManTimedFrag && GetInputMode() != INPUT_MODE_PLAYBACK)
	{
		// Determine player's score and note how we determined it in a string
		// for the user.
		long	lPlayerScore	= 0;
		switch (pRealm->m_ScoringMode)
			{
			case CRealm::Standard:
				// No high scores for this mode
				break;

			case CRealm::Timed:
				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], CreateTimeString(pRealm->m_lScoreInitialTime) );
				// Number of deaths.
				lPlayerScore	= pRealm->m_sPopulationDeaths;
				vtScoringUnit	= Value;
				break;

			case CRealm::TimedGoal:
				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], pRealm->m_sKillsGoal);
				// Elapsed time, if goal met.
				if (pRealm->m_sPopulationDeaths >= pRealm->m_sKillsGoal)
					{
					lPlayerScore	= pRealm->m_lScoreInitialTime - pRealm->m_lScoreTimeDisplay;
					}
				else
					{
					// Really bad elapsed time.
					lPlayerScore	= LONG_MAX;
					}

				vtScoringUnit	= Time;
				break;

			case CRealm::TimedFlag:
				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], pRealm->m_sKillsGoal);
				// Elapsed time, if goal met.
				if (pRealm->m_sFlagbaseCaptured >= pRealm->m_sFlagsGoal)
					{
					lPlayerScore	= pRealm->m_lScoreInitialTime - pRealm->m_lScoreTimeDisplay;
					}
				else
					{
					// Really bad elapsed time.
					lPlayerScore	= LONG_MAX;
					}

				vtScoringUnit	= Time;
				break;

			case CRealm::CaptureFlag:
				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], pRealm->m_sKillsGoal);
				// Time left, if goal met.
				if (pRealm->m_sFlagbaseCaptured >= pRealm->m_sFlagsGoal)
					{
					lPlayerScore	= pRealm->m_lScoreTimeDisplay;
					}
				else
					{
					// Really bad time.
					lPlayerScore	= LONG_MIN;
					}

				vtScoringUnit	= Time;
				break;

			case CRealm::Goal:
				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], pRealm->m_sKillsGoal);
				// Time left, if goal met.
				if (pRealm->m_sPopulationDeaths >= pRealm->m_sKillsGoal)
					{
					lPlayerScore	= pRealm->m_lScoreTimeDisplay;
					}
				else
					{
					// Really bad time.
					lPlayerScore	= LONG_MIN;
					}

				vtScoringUnit	= Time;
				break;

			case CRealm::Checkpoint:
				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], 0);
				// Number of flags captured.
				lPlayerScore	= pRealm->m_sFlagsCaptured;
				vtScoringUnit	= Value;
				break;

			case CRealm::MPTimed:
			case CRealm::MPFrag:
			case CRealm::MPLastMan:
			case CRealm::MPCaptureFlag:
			case CRealm::MPTimedFlag:
			case CRealm::MPTimedFrag:
			case CRealm::MPLastManFrag:
			case CRealm::MPLastManTimed:
			case CRealm::MPLastManTimedFrag:

				sprintf(szScoringExplanation, g_apszScoreExplanations[pRealm->m_ScoringMode], MAX_HIGH_SCORES);
				vtScoringUnit	= Value;
				break;
			}

		// Get the name or description string for this realm file and use that as
		// the prefs section name from which to get the high scores.  Scores are
		// stored as longs whether they be times in seconds or counts and the names
		// are stored as strings.

		// Temporarily I will base the section name on the scoring method
		// until the realm description function is available.

		short	sPlayersScorePosition	= -1;	// Valid score index once/if we find a slot
														// for this player's score.

		// If not a multiplayer scoring . . .
		if (pRealm->m_flags.bMultiplayer == false)
			{
			// Try to open the file, but if it doesn't exist, then we simply won't be
			// getting any values from it.
			short sOpenRes = scores.Open(FullPathHD(HIGHSCORE_SCORES_FILE), "r");
			
			// Read in the scores file
			short	sSrcIndex;
			short	sDstIndex;
			for (sSrcIndex = 0, sDstIndex = 0; sDstIndex < MAX_HIGH_SCORES; sDstIndex++)
				{
				sprintf(szKeyName, "Player%d", sSrcIndex);
				if (sOpenRes == 0)
					scores.GetVal((char*) pRealm->m_rsRealmString, szKeyName, "<Empty>", astrNames[sDstIndex]);
				else
					strcpy(astrNames[sDstIndex], "<Empty>");
					
				// Determine if our score beat this score.
				bool	bPlayerBeatThisScore	= false;
				sprintf(szKeyName, "Score%d", sSrcIndex);
				// Some scoring modes need to default to zero scores, but the timed levels
				// need to default to some high, easy to beat time if there is no saved score
				if (vtScoringUnit == Value)
					{
					if (sOpenRes == 0)
						scores.GetVal((char*) pRealm->m_rsRealmString, szKeyName, (long) 0, &(alScores[sDstIndex]));
					else
						alScores[sDstIndex] = 0;
						
					// Did we get a higher value than in score?
					if (lPlayerScore > alScores[sDstIndex])
						{
						bPlayerBeatThisScore	= true;
						}
					}
				else
					{
					if (sOpenRes == 0)
						scores.GetVal((char*) pRealm->m_rsRealmString, szKeyName, (long) 3600000, &(alScores[sDstIndex]));
					else
						alScores[sDstIndex] = (long) 3600000;
			
					// Did we get a better time than read in score?
					if (lPlayerScore < alScores[sDstIndex] )
						{
						bPlayerBeatThisScore	= true;
						}
					}

				// If we beat this score and haven't yet found a score position . . .
				if (bPlayerBeatThisScore == true && sPlayersScorePosition < 0)
					{
					// Remember player's score.
					alScores[sDstIndex]		= lPlayerScore;
					// Clear player's name.
					astrNames[sDstIndex][0]	= '\0';
					// Remember player's score position.
					sPlayersScorePosition	= sDstIndex;
					// Don't increment the source index.
					}
				else
					{
					// Move to the next source score val and name from the INI.
					sSrcIndex++;
					}
				}

			scores.Close();
			}
		else
			{
			ASSERT(pclient);

			long	alTempScores[MAX_HIGH_SCORES];
			char	astrTempNames[MAX_HIGH_SCORES][MAX_PLAYER_NAME_LEN + 1];

			short	sIndex;
			for (sIndex = 0; sIndex < MAX_HIGH_SCORES ; sIndex++)
				{
				if (sIndex < pRealm->m_asClassNumThings[CThing::CDudeID])
					{
					strncpy(astrTempNames[sIndex], pclient->GetPlayerName(sIndex), MAX_PLAYER_NAME_LEN);
					// Strncpy does not NULL terminate if the 'n' is less than or equal to the length
					// of the src string.
					astrTempNames[sIndex][MAX_PLAYER_NAME_LEN]	= '\0';

					alTempScores[sIndex]	= g_scoreboard.m_asScores[sIndex];
					}
				else
					{
					astrTempNames[sIndex][0]	= '\0';
					alTempScores[sIndex]			= LONG_MIN + 1;
					}
				}

			// Find the largest score (most frags in all modes) and put it at the
			// next position.
			short	sDstIndex;
			short	sSrcIndex;
			short	sHighestScoreIndex;
			long	lHighestScore;
			
			// This declaration relies on false being zero!!
			ASSERT(false == 0);
			bool	abAlreadyCopied[MAX_HIGH_SCORES]	=	{ false, };

			for (sDstIndex = 0;	sDstIndex < MAX_HIGH_SCORES; sDstIndex++)
				{
				sHighestScoreIndex	= -1;
				lHighestScore			= LONG_MIN;

				// Find the highest score of the ones not yet copied.
				for (sSrcIndex = 0; sSrcIndex < MAX_HIGH_SCORES; sSrcIndex++)
					{
					// If not yet copied . . .
					if (abAlreadyCopied[sSrcIndex] == false)
						{
						// If this score is higher . . .
						if (alTempScores[sSrcIndex] > lHighestScore)
							{
							sHighestScoreIndex	= sSrcIndex;
							lHighestScore			= alTempScores[sSrcIndex];
							}
						}
					}

				// Use the highest score.
				ASSERT(sHighestScoreIndex != -1);
				alScores[sDstIndex]	= alTempScores[sHighestScoreIndex];
				// This copy is safe b/c astrTempNames[] and astrNames[] are the
				// same length.
				
				// Check for safety (future changes).
				ASSERT(sizeof(astrNames[0]) >= sizeof(astrTempNames[0]) );

				strcpy(astrNames[sDstIndex], astrTempNames[sHighestScoreIndex] );
				// Note that this score is already placed.
				abAlreadyCopied[sHighestScoreIndex]	= true;

				// If this is us . . .
				if (sHighestScoreIndex == pclient->GetID() )
					{
					// Remember our position (placement) so we can highlight it.
					sPlayersScorePosition	= sDstIndex;
					}
				}

			// Note that Player's score position stays -1 since there's no name to enter.
			}


		short i;

		if (rspGetResource(&g_resmgrShell, HIGHSCORE_DIALOG_FILE, (RDlg**)&pguiRoot) == 0)
		{
			RGuiItem* pguiOk			= pguiRoot->GetItemFromId(1);
			RGuiItem* pguiCancel		= pguiRoot->GetItemFromId(2);

			RTxt* ptextExplain1		= (RTxt*) pguiRoot->GetItemFromId(50);
			RTxt* ptextExplain2		= (RTxt*) pguiRoot->GetItemFromId(51);
			RListBox*	plbScores	= (RListBox*) pguiRoot->GetItemFromId(1000);

			// Set to the input field if the player gets a high score.
			RGuiItem*	pguiPlayersName	= NULL;

			// Create and add all score items.
			short	sScoreIndex;
			bool	bGotAllScoreItems	= true;

			if (plbScores)
				{
				ASSERT(plbScores->m_type == RGuiItem::ListBox);

				for (sScoreIndex = 0; sScoreIndex < MAX_HIGH_SCORES && bGotAllScoreItems; sScoreIndex++)
					{
					// If there's an associated name or this is the one we're adding . . .
					if (astrNames[sScoreIndex][0] != '\0' || sPlayersScorePosition == sScoreIndex)
						{
						RGuiItem*	pguiItem;
						if (rspGetResourceInstance(&g_resmgrShell, HIGHSCORE_ITEM_FILE, &pguiItem) == 0)
							{
							// Get the two settable items.
							RGuiItem*	pguiName		= pguiItem->GetItemFromId(100);
							RGuiItem*	pguiScore	= pguiItem->GetItemFromId(101);
							RGuiItem*	pguiPlace	= pguiItem->GetItemFromId(102);
							if (pguiName && pguiScore)
								{
								// Add shadow attributes.
								pguiName->m_sTextEffects			|= RGuiItem::Shadow;
								pguiName->m_u32TextShadowColor	=	TEXT_SHADOW_COLOR;
								pguiScore->m_sTextEffects			|= RGuiItem::Shadow;
								pguiScore->m_u32TextShadowColor	=	TEXT_SHADOW_COLOR;

								// If this is the place for the new name . . .
								if (sPlayersScorePosition == sScoreIndex)
									{
									// Set the focus to this item.
									pguiName->SetFocus();
									// Limit input text to the space in our storage area.
									// Must be edit field for this op.
									ASSERT(pguiName->m_type == RGuiItem::Edit);
									((REdit*)pguiName)->m_sMaxText	= sizeof(astrNames[0]) - 1;
									// Highlight this entry.
									pguiName->m_u32TextColor	= TEXT_HIGHLIGHT_COLOR;
									// Remember which one so we can get the name later.
									pguiPlayersName	= pguiName;
									}
								else
									{
									// Deactivate all others.
									pguiName->m_sActive	= FALSE;
									}

								// Set placement.
								pguiPlace->SetText("%d)", sScoreIndex + 1);
								pguiPlace->Compose();

								// Set name.
								pguiName->SetText("%s", astrNames[sScoreIndex]);
								pguiName->Compose();

								// Set score.
								// There are two types of scores.
								if (vtScoringUnit == Value)
									{
									// Value.
									pguiScore->SetText("%ld %s", alScores[sScoreIndex], g_apszScoreUnits[pRealm->m_ScoringMode] );
									}
								else
									{
									// Time.
									pguiScore->SetText("%s %s", CreateTimeString(alScores[sScoreIndex] ), g_apszScoreUnits[pRealm->m_ScoringMode] );
									}

								pguiScore->Compose();

								// Mark item as an encapsulator.  When an item is marked this way the listbox
								// knows it's okay to move it around and stuff.
								RListBox::MakeEncapsulator(pguiItem);
								
								// Add to the list box . . .
								if (plbScores->AddItem(pguiItem) )
									{
									// Success.
									}
								else
									{
									TRACE("ScoreDisplayHighScores(): Unable to add item to listbox.\n");
									bGotAllScoreItems	= false;
									}
								}
							else
								{
								TRACE("ScoreDisplayHighScores(): Missing items in this instance of \"%d\".\n",
									HIGHSCORE_ITEM_FILE);
								bGotAllScoreItems	= false;
								}
							}
						else
							{
							TRACE("ScoreDisplayHighScores(): Failed to get instance of \"%d\".\n",
								HIGHSCORE_ITEM_FILE);
							bGotAllScoreItems	= false;
							}
						}
					}

				// Repaginate now.
				plbScores->AdjustContents();

				// If we have an entry . . .
				if (pguiPlayersName)
					{
					// Make sure it's visible . . .
					plbScores->EnsureVisible(pguiPlayersName, RListBox::Bottom);
					}
				}

			if (ptextExplain1 != NULL &&
				 ptextExplain2 != NULL &&
				 plbScores		!= NULL &&
				 bGotAllScoreItems == true)
			{
				// Get some colors free.
				PalTranOn();

				// Set the callbacks for the resource load and discard (note that it 
				// already tried to load it during the above rspGetResource() and failed 
				// b/c the default implementation has no clue about the resmgr).
				pguiRoot->m_fnGetRes			= GuiGetRes;
				pguiRoot->m_fnReleaseRes	= GuiReleaseRes;

				pguiRoot->ReleaseRes();
				// Recompose the root item (does not recompose children).  This time
				// it should successfully find the resources via the Get callback.
				pguiRoot->Compose();
					
				// Let us handle updates.
				guiDialog.m_fnUpdate		= SysUpdate;

				// Center the GUI root.
				pguiRoot->Move(
					g_pimScreenBuf->m_sWidth / 2 - pguiRoot->m_im.m_sWidth / 2,
					g_pimScreenBuf->m_sHeight / 2 - pguiRoot->m_im.m_sHeight / 2);

				// Make the explanation texts shadowed.
				ptextExplain1->m_sTextEffects			|= RGuiItem::Shadow;
				ptextExplain1->m_u32TextShadowColor	=	TEXT_SHADOW_COLOR;
				
				ptextExplain2->m_sTextEffects			|= RGuiItem::Shadow;
				ptextExplain2->m_u32TextShadowColor	=	TEXT_SHADOW_COLOR;
				ptextExplain2->Compose();

				// Store current mouse show level so we can restore it.
				short	sOrigShowLevel	= rspGetMouseCursorShowLevel();
				// Make sure it's visible.
// Let's not do this and instead try to insinuate keyboard use.
//				rspSetMouseCursorShowLevel(1);

				// Make sure there's no timeout on while player is adding their name.
				ms_lScoreMaxTimeOut	= LONG_MAX;

				// If we want a high score from this player . . .
				if (sPlayersScorePosition >= 0 && pRealm->m_flags.bMultiplayer == false)
					{
					// Ask the player for their name.
					ptextExplain1->SetText("Please enter your name.\n");
					ptextExplain1->Compose();
					// Don't clean the screen when done so we can have a smooth transition
					// to the next DoModal().
					guiDialog.m_sFlags	= RProcessGui::NoCleanScreen;
					// Do the dialog once to get the name.
					guiDialog.DoModal(pguiRoot, pguiOk, pguiCancel);
					// Clear the focus.
					RGuiItem::SetFocus(NULL);

					ASSERT(pguiPlayersName);
					// Get the user's name for saving.
					pguiPlayersName->GetText(astrNames[sPlayersScorePosition], sizeof(astrNames[sPlayersScorePosition]) );
					}

				// Set the explanation text.
				ptextExplain1->SetText("%s", szScoringExplanation);
				ptextExplain1->Compose();

				// Don't set time out time until after player has entered name for
				// reduced frustration.
				// If timeout specified . . . 
				if (lMaxTimeOut >= 0)
					{
					ms_lScoreMaxTimeOut	= rspGetMilliseconds() + lMaxTimeOut;
					}
				else
					{
					ms_lScoreMaxTimeOut	= LONG_MAX;
					}

				// Set the focus to the listbox's vertical scrollbar so that the arrows will work.
				plbScores->m_sbVert.SetFocus();

				// This time we want the screen cleared.
				guiDialog.m_sFlags	= 0;
				// Display the high scores.
				guiDialog.DoModal(pguiRoot, pguiOk, pguiCancel);

				// Restore mouse cursor show level.
				rspSetMouseCursorShowLevel(sOrigShowLevel);

				// If we got a high score . . .
				if (sPlayersScorePosition >= 0 && pRealm->m_flags.bMultiplayer == false)
					{
					RPrefs prefsScores;
					// Save the scores to the file.  First we open it in read+ mode, which is
					// safe if the file already exists.  If that fails, we assume the file does
					// NOT exist and we try to open it in write+ mode, which will clobber the
					// contents of the file if it does exist.
					sResult = prefsScores.Open(FullPathHD(HIGHSCORE_SCORES_FILE), "r+");
					if (sResult != SUCCESS)
						sResult = prefsScores.Open(FullPathHD(HIGHSCORE_SCORES_FILE), "w+");
					if (sResult == SUCCESS)
						{
						for (i = 0; i < MAX_HIGH_SCORES; i++)
							{
							sprintf(szKeyName, "Player%d", i);
							prefsScores.SetVal((char*) pRealm->m_rsRealmString, szKeyName, astrNames[i]);
							sprintf(szKeyName, "Score%d", i);
							prefsScores.SetVal((char*) pRealm->m_rsRealmString, szKeyName, alScores[i]);
							}
						}
					prefsScores.Close();
					}

				// Put the colors back.
				PalTranOff();

			}

			if (plbScores)
				{
#if 0
				// Get rid of all score item instances.
				RGuiItem*	pguiScoreItem	= plbScores->GetFirst();
				while (pguiScoreItem)
					{
					// Remove the listbox's encapsulator property.
					pguiScoreItem->RemoveProp(ENCAPSULATOR_PROP_KEY);

					// Release the current one.
					rspReleaseResourceInstance(&g_resmgrShell, &pguiScoreItem);

					// Set the next one as the current.
					pguiScoreItem	= plbScores->GetNext();
					}
#else
				plbScores->RemoveAll();
#endif
				}
		
			rspReleaseResource(&g_resmgrShell, &pguiRoot);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// ScoreHighestKills
//
// Return the highest number of kills among all of the players.  This will
// be called to determine if a frag limit level is over.
//
//////////////////////////////////////////////////////////////////////////////

short ScoreHighestKills(CRealm* pRealm)
{
	short sHighest = 0;
	short sNumDudes = pRealm->m_asClassNumThings[CThing::CDudeID];	
	short i;

	for (i = 0; i < sNumDudes; i++)
	{
		if (g_scoreboard.m_asScores[i] > sHighest)
			sHighest = g_scoreboard.m_asScores[i];
	}
	
	return sHighest;	
}


//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////

