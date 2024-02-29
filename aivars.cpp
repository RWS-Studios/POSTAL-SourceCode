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
// aivars.cpp
// Project: Postal
//
// History:
//		05/19/97 BRH	Started.
//
//		05/19/97 BRH	Added the first few logic variables.
//
//		05/20/97 BRH	Changed real states to suggested Actions.  Also added 
//							Pylon availability variables.
//
//		05/26/97 BRH	Added IsTriggered variable so that you can distinguish
//							between a Dude nearby and a Dude nearby who is also
//							standing on the trigger area for your pylon based logic.
//
//		05/27/97 BRH	Added Recently Shot variable.  Tuned the distances.
//
//		05/31/97	JMI	Changed usage from CDoofus::m_pDude to CDoofus::m_idDude
//							(see doofus.cpp comment with same date for info).
//							PLEASE NOTE:  I could not help but notice that 
//							CLogTabVar_DudeHealth::GetVal() calls pPerson->SelectDude()
//							without first checking if that person was already
//							tracking a dude.  I just though this was bad as it could
//							cause inconsistent behavior between the same states with
//							different logic tables and/or values for logic tables.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		07/06/97 BRH	Added panic variable so the logic table can decide if 
//							a person should panic based on other people choosing to
//							panic.
//
//		08/09/97 BRH	Made a minor change to the IsPanic variable.
//
//		08/29/97 BRH	Added a Statis user variable for the people so that
//							a logic table for one person can set this variable
//							that others can then use.  This will be used for the
//							kids where one kid will trigger the others into doing
//							their group attack logic.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "logtab.h"
#include "person.h"
#include "Thing3d.h"
#include "doofus.h"

// Action input variable
class CLogTabVar_GetAction : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_GetAction()
			{
			m_pszName = "GET_ACTION";
			m_sMaxVal = CDoofus::NumActions;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			m_sNumStrings = CDoofus::NumActions;
			m_papszStrings = CDoofus::ms_apszActionNames;
			}

		short GetVal(CPerson* pPerson)
			{
			return pPerson->m_eCurrentAction;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_getaction;

// Action output variable
class CLogTabVar_SetAction : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_SetAction()
			{
			m_pszName = "SET_ACTION";
			m_sMaxVal = CDoofus::NumActions;
			m_bSettable = true;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			m_sNumStrings = CDoofus::NumActions;
			m_papszStrings = CDoofus::ms_apszActionNames;
			}

		short GetVal(CPerson* pPerson)
			{
			return pPerson->m_eSuggestedAction;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			pPerson->m_eSuggestedAction = (CDoofus::Action) sVal;
			}
	} givename_setaction;

// Distance to target input variable
class CLogTabVar_TargetDist : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_TargetDist()
			{
			m_pszName = "TARGET_DIST";
			m_sMaxVal = 2;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			// IMPORTANT:  these enums all represent distance RANGES!
			// Each one means "between myself and the next highest value"
			//
			static char* ms_sz[] = {"VeryClose", "Close", "Medium", "Far","OffScreen"};
			m_sNumStrings = 5;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			// Define local arguements which may become inputs:
			// (They are the squared distances...)

			// NOTE THAT THESE ARE RANGES BETWEEN MYSELF AND THE NEXT HIGHEST!
			// THE FINAL ENUM HAS NO UPPER BOUND!
			// You cannot evaluate < lowest or > highest!

			// LOWER BOUNDS OF RANGE!
			#define	DIST_VERY_CLOSE_2 (0 * 0)   // the lowest measure
			#define	DIST_CLOSE_2 (80 * 80)
			#define	DIST_MEDIUM_2 (180 * 180)
			#define	DIST_FAR_2 (400 * 400) 
			#define	DIST_OFF_SCREEN_2 (800 * 800)

			double dSqDist = pPerson->SQDistanceToDude();
			if (dSqDist > DIST_OFF_SCREEN_2)
				return 4;
			if (dSqDist > DIST_FAR_2)
				return 3;
			if (dSqDist > DIST_MEDIUM_2)
				return 2;
			if (dSqDist > DIST_CLOSE_2)
				return 1;
			else	//  DIST_VERY_CLOSE_2
				return 0; 
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_targetdist;

// Availability of Popout pylon
class CLogTabVar_PopoutAvailable : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_PopoutAvailable()
			{
			m_pszName = "POPOUT_AVAIL";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			pPerson->Logic_PylonDetect();
			return pPerson->m_bPylonPopoutAvailable;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_popoutavailable;

// Availability of Run and Shoot pylon
class CLogTabVar_RunShootAvailable : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_RunShootAvailable()
			{
			m_pszName = "RUNSHOOT_AVAIL";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			pPerson->Logic_PylonDetect();
			return pPerson->m_bPylonRunShootAvailable;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_runshootavailable;


// Availability of Safety pylon
class CLogTabVar_SafetyAvailable : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_SafetyAvailable()
			{
			m_pszName = "SAFETY_AVAIL";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			pPerson->Logic_PylonDetect();
			return pPerson->m_bPylonSafeAvailable;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_safetyavailable;


// Availability of Pylon
class CLogTabVar_PylonAvailable : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_PylonAvailable()
			{
			m_pszName = "PYLON_AVAIL";
			m_sMaxVal = 3;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"none", "popout", "runshoot", "safety"};
			m_sNumStrings = 4;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			short sResult = 0;
			pPerson->Logic_PylonDetect();
			if (pPerson->m_bPylonPopoutAvailable)
			{
				sResult = 1;
			}
			else
			{
				if (pPerson->m_bPylonRunShootAvailable)
				{
					sResult = 2;
				}
				else
				{
					if (pPerson->m_bPylonSafeAvailable)
						sResult = 3;
				}
			}
					
			return sResult;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_pylonavailable;

// The this character's health
class CLogTabVar_MyHealth : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_MyHealth()
			{
			m_pszName = "MY_HEALTH";
			m_sMaxVal = 2;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"low", "med", "high"};
			m_sNumStrings = 3;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return pPerson->m_stockpile.m_sHitPoints / 34;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_myhealth;


// The target CDude's health
class CLogTabVar_DudeHealth : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_DudeHealth()
			{
			m_pszName = "DUDE_HEALTH";
			m_sMaxVal = 3;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"dead", "low", "med", "high"};
			m_sNumStrings = 4;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			short sResult = 0;

			if (pPerson->SelectDude() == SUCCESS)
			{
				CDude*	pdude;
				if (pPerson->m_pRealm->m_idbank.GetThingByID((CThing**)&pdude, pPerson->m_idDude) == 0)
				{
					short sHitPoints = pdude->GetHealth();
					if (sHitPoints > 80)
					{
						sResult = 3;
					}
					else
					{
						if (sHitPoints > 40)
							sResult = 2;
						else
							sResult = 1;
					}
				}
			}

			return sResult;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_dudehealth;


// Is the pylon area being triggered right now?
class CLogTabVar_IsTriggered : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_IsTriggered()
			{
			m_pszName = "IS_TRIGGERED";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return (pPerson->m_pPylonStart != NULL && pPerson->m_pPylonStart->Triggered());
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_istriggered;

// This is the first high level state variable available to the logic table
class CLogTabVar_UserState1 : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_UserState1()
			{
			m_pszName = "USER1";
			m_sMaxVal = 32767;
			m_bSettable = true;
			m_sOutputWidth = 5;

			// Feel free to add as many states as you need, but remember that the 
			// WHOLE table assumes only ONE of these varables right now
			static char* ms_sz[] = {"default","active"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return pPerson->m_sUserState1;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			pPerson->m_sUserState1 = sVal;
			}
	} givename_userstate1;

// Did the enemy just get shot?
class CLogTabVar_RecentlyShot : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_RecentlyShot()
			{
			m_pszName = "RECENTLY_SHOT";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 13;

			// This var is number-based, so turn off string stuff
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return (pPerson->m_pRealm->m_time.GetGameTime() < pPerson->m_lShotTimeout);
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_recentlyshot;

// Did the anamy get stuck on terrain?
class CLogTabVar_RecentlyStuck : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_RecentlyStuck()
			{
			 m_pszName = "RECENTLY_STUCK";
			 m_sMaxVal = 1;
			 m_bSettable = true;
			 m_sOutputWidth = 14;

			 // This var is a bool
			 static char* ms_sz[] = {"false", "true"};
			 m_sNumStrings = 2;
			 m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return (pPerson->m_pRealm->m_time.GetGameTime() < pPerson->m_lStuckTimeout);
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			// Set to false
			if (sVal == 0)
				pPerson->m_lStuckTimeout = pPerson->m_pRealm->m_time.GetGameTime();
			// Set to true
			else
				pPerson->m_lStuckTimeout = pPerson->m_pRealm->m_time.GetGameTime() + pPerson->ms_lStuckRecoveryTime;
			}		

	} givename_recentlystuck;

// Check to see if panic has begun
class CLogTabVar_IsPanic : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_IsPanic()
			{
			m_pszName = "IS_PANIC";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 8;

			// This var is true/false
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return pPerson->m_bPanic;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_ispanic;

// Check to see if someone yelled for help in your area
class CLogTabVar_HelpCall : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_HelpCall()
			{
			m_pszName = "HELPCALL";
			m_sMaxVal = 1;
			m_bSettable = false;
			m_sOutputWidth = 8;

			// This var is true/false
			static char* ms_sz[] = {"false", "true"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			if (pPerson->m_pRealm->m_time.GetGameTime() < pPerson->m_lLastHelpCallTime + pPerson->ms_lHelpTimeout && pPerson->m_lLastHelpCallTime > 0)
				return 1;
			else
				return 0;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			}
	} givename_helpcall;


// This is the first high level state variable available to the logic table
class CLogTabVar_UserGlobal : CLogTabVar<CPerson*>
	{
	public:
		CLogTabVar_UserGlobal()
			{
			m_pszName = "USER_GLOBAL";
			m_sMaxVal = 32767;
			m_bSettable = true;
			m_sOutputWidth = 5;

			// Feel free to add as many states as you need, but remember that the 
			// WHOLE table assumes only ONE of these varables right now
			static char* ms_sz[] = {"normal","attack"};
			m_sNumStrings = 2;
			m_papszStrings = ms_sz;
			}

		short GetVal(CPerson* pPerson)
			{
			return pPerson->ms_sLogTabUserGlobal;
			}

		void SetVal(CPerson* pPerson, short sVal)
			{
			pPerson->ms_sLogTabUserGlobal = sVal;
			}
	} givename_userglobal;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
