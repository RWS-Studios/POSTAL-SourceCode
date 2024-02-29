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
// person.cpp
// Project: Postal
//
// This module implements the CPerson class which is mostly a test object
//	the the most basic functionality of the enemy guys derrived from CDoofus.
//
// History:
//
//		04/28/97 BRH	Started this person to be the generic Enemy/Victim
//							that will use the structures set up in 
//							Personatorium to determine the abilities and
//							desired logic.
//
//		04/30/97 BRH	Fixed up the EditModify dialog box to select any type
//							of personality and any type of weapon.  Modified the
//							Update to shoot any weapon.
//
//		05/11/97 BRH	Imported the Update function from CDoofus.  CDoofus will
//							most likely make its Update funciton pure virtual so that
//							it is up to the derived classes to move from state to
//							state.  
//
//		05/12/97 BRH	Added the crawling special case for the Cop and Gunner in
//							Logic_Writhing.  Added the logic for the circle fight
//							and retreat in CDoofus.
//
//		05/19/97	JMI	Added ms_u16IdLogAI and EditModify() code to check and
//							change it.
//
//		05/19/97	JMI	Added m_szLogicFile, m_sShowState, and Render() override.
//							Can change m_szLogicFile and m_sShowState in EditModify().
//
//		05/19/97 BRH	Saved the m_sShowState variable so that it works during
//							play mode.  Changed the name of the logic file to an
//							RString and added the RString load and save to the
//							CPerson load and save functions.
//
//		05/20/97 BRH	Making the high level actions more concrete by adding
//							Actions as a higher level concept than the states.  Each
//							action may consist of several pre-programmed state
//							sequences.  At good evaluation points in each action, 
//							it will check the suggested logic action and change states
//							to perform that action.  
//
//		05/21/97 BRH	Added Resource management for ShootRun animation.
//
//		05/22/97	JMI	FreeResources() was releasing m_pLogicTable in g_resmgrGame
//							instead of g_resmgrRes (which is where it was 
//							rspGetResource'ed from).
//
//		05/23/97	JMI	Added NULLs for new pszEventName parameter to 
//							CAnim3D::Get()s indicating to load no events.
//
//		05/25/97 BRH	Added Release calls for the new animations.
//
//		05/25/97	JMI	Changed "Personality.GUI" to "Person.GUI".
//
//		05/26/97 BRH	Calls CDoofus::OnDead rather than the CCharacter version.
//
//		06/02/97 BRH	Added case for State_HuntHold.
//
//		06/04/97 BRH	Fixed Writing crawl to use the m_AnimRot so that it faces
//							the correct direction.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/10/97 BRH	Sends Death certificate to CDemon.
//
//		06/10/97 BRH	Added Search and Crouch animations to Get and Free Resources
//
//		06/13/97	JMI	Now GetResources() loads events for animations that need
//							them.
//
//		06/15/97 BRH	Adjusted the time and comment count so people don't 
//							speak as often.
//
//		06/16/97 BRH	Tried Aborting any shot sounds if it wasn't done playing.
//							If it doesn sound good, then we will just verify that
//							the previous sample is done before playing the next one.
//							We were thinking that it may sound cool if he interrupted
//							"My leg" before saying "Ug".  
//
//		06/16/97 BRH	Switched to IsSamplePlaying rather than aborting, because
//							the phrases were rarely finished before aborting so you
//							never got to hear them if you kept shooting.
//
//		06/17/97 BRH	Took out IsSamplePlaying because it screws up the network
//							mode because the Sample can play for different intervals
//							on different systems.
//
//		06/18/97 BRH	Enabled the Shooting sounds.  
//
//		06/18/97	JMI	Changed PlaySoundWrithing() to return the duration of the
//							played sample.
//
//		06/18/97 BRH	Changed to using GetRandom();
//
//		06/18/97	JMI	Now uses the sample duration returned from PlaySample()
//							instead of getting it from the RSnd.
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/27/97	JMI	Now shows weapon type when in edit mode via EditRender().
//
//		06/30/97 BRH	Changed the random type sounds to play and then purge to 
//							save memory.  Cached the other sound effects during the
//							load for each different person type loaded so that their
//							sounds won't pause the game when they are first shot, 
//							set on fire, etc.
//
//		07/03/97	JMI	Converted calls to rspOpen/SaveBox() to new parm 
//							conventions.
//
//		07/04/97 BRH	Changed run and shoot prefix from run45rt to 45rt to
//							match the exported names.  Also added a substitution of
//							the run animation if there is no onfire animation since
//							the victims don't yet have an onfire animaiton.
//
//		07/06/97 BRH	Added new victim states to the Update function.
//
//		07/08/97 BRH	Changed run backwards animation name to runbackwr since
//							some of the filenames were too long for the delicate
//							MacOS.
//
//		07/17/97 BRH	Added delay shoot case in update.
//
//		07/17/97	JMI	Changed RSnd*'s to SampleMaster::SoundInstances.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/21/97	JMI	Now Update() calls CCharacter version (note that this is
//							skipping CDoofus::Update() ).
//
//		07/23/97 BRH	Checks for a live dude before making a random comment so
//							that they don't say stupid things after the CDude is dead.
//
//		07/26/97 BRH	Changed the initial setting of hit points from the CThing3D
//							default value to using the new value in the personatorium.
//
//		08/01/97 BRH	Enemies now upgrade their weapons if the game is set
//							on difficulty 11.  Guns are upgraded to Assault Weapons
//							and rockets are upgraded to heatseekers.  Also added
//							case for State_AvoidFire and State_DangerNear.
//
//		08/06/97	JMI	Now loads execution target points for writhing mode.
//							Also, now uses CDoofus's PositionSmash() for positioning 
//							the main smash.
//
//		08/07/97	JMI	Now loads new resname gotten from Personatorium for the
//							linkpoint/rigid-body for the person's hand.
//							Now calls CDoofus' GetResources()/ReleaseResources() to
//							get and release resources for weapons.
//
//		08/08/97	JMI	Now does not display the mines in the EditModify().
//							
//		08/08/97	JMI	Upgraded to use all the new weapons.
//							Now does the weapon upgrade for difficulty 11 in only
//							when editing flag is set to false in the realm so it does 
//							not occur in the editor.
//
//		08/09/97 BRH	Added TRACE message for invalid state type - so if it
//							it happens sgain where a state is not accounted for in
//							the Update switch statement, it will print the number
//							and them attempt to print the description of the state
//							for debugging purposes.
//
//		08/10/97 BRH	Fixed problem with executions when people yelled
//							things like ah my leg when they were being executed.  
//							Changed this to a generic grunt that should work
//							for all people.
//
//		08/11/97	JMI	Now sets backup weapon type based on personatorium.
//							Also, added case for State_WalkNext to Update()'s logic
//							switch.
//
//		08/12/97	JMI	Now loads main event for all animations.
//
//		08/12/97 BRH	Added missing case statement for Hide and HideBegin.
//							Also upped the number of random comments for the victims
//							(the ones in panic mode so that they will scream and 
//							yell more often when in panic mode).  Also added 
//							special case for shot sound effects where the first
//							shot sound will be the comment, and the other 3 just
//							noises.  Once the comment is played (randomly chosen)
//							the m_bHitComment will be set so it doesn't get played
//							again.
//
//		08/14/97	JMI	Switched references to g_GameSettings.m_sDifficulty to
//							m_pRealm->m_flags.sDifficulty.
//
//		08/17/97 BRH	Slowed writhing crawl velocity from 5 to 2.5
//
//		08/20/97 BRH	Changed the sound categories from voice to voice and
//							the new pain and suffering categories.
//
//		08/24/97	JMI	Also, now the gunner type of motion (PushBack) also dies 
//							when it hits something.
//							Now checks a point that should be the person's furthest
//							extremity when he's writhing and if that point goes into
//							anything he acts the same as if his main attrib check area
//							hit it except he ignores the height (that is, he treats it
//							all as no walk)
//
//		08/24/97 BRH	The PlaySound functions now set the new doofus m_siPlaying
//							sound instance so that it can be aborted when the guy is 
//							killed so he doesn't keep making noises, especially after
//							being executed.
//
//		08/25/97	JMI	When I added the last change on checking their heads'
//							while writhing, I inadvertently took out the check to kill
//							them if they're feet hit something.  Since there moving
//							in the direction of their head, this shouldn't've been a
//							problem, but it's worth fixing.  Fixed.
//
//		08/26/97 BRH	Added special case for the people who have a description
//							starting with "Kid".  The Kids smash bits are set to zero
//							so that they cannot be hit by weapons for the final
//							scene.
//
//		08/28/97 BRH	Fixed the push back code in Logic_Writhing
//
//		08/29/97 BRH	Added static variable for the logic table to use to set
//							group motions.
//
//		09/03/97	JMI	Civilians now use Civilian Smash bit instead of Bad.
//
//		10/24/97	JMI	Added Mac specific code to make sure browsed-for logic
//							files are relative paths.
//
//		01/14/98	JMI	Added more descriptive alert (than the ASSERT(0) ) for
//							unexpected states in the main switch() based on the TRACEs
//							Bill had.
//
//		10/03/99	JMI	Now launches TexEdit when Edit Textures... chosen.
//
//		10/06/99	JMI	Now only adds the sTextureScheme if it is non-negative.
//							Now passes the hood lights to the texture editor.
//
////////////////////////////////////////////////////////////////////////////////
#define PERSON_CPP

#include "RSPiX.h"				 
#include <math.h>

#include "person.h"
#include "TexEdit.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define NEAR_DEATH_HITPOINTS	20
#define MS_BETWEEN_SAMPLES		100
#define PERSONALITY_ITEM_ID_BASE 200

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

double CPerson::ms_dLongRange = 500*500;		// Squared distance (500 pixels away)
double CPerson::ms_dInRangeLow = 190*190;		// Squared distance to be in range with weapon
double CPerson::ms_dInRangeHigh = 230*230;	// Squared distance to be in range with weapon
double CPerson::ms_dThrowHorizVel = 200;		// Throw out at this velocity
double CPerson::ms_dMaxCrawlVel = 2.5;			// Speed at which cop crawls
long CPerson::ms_lMinCommentTime = 10000;		// Amount of time before making a comment
long CPerson::ms_lCommentTimeVariance = 35000;// Random amount added on.
long CPerson::ms_lRandomAvoidTime = 200;		// Time to wander before looking again
long CPerson::ms_lReseekTime = 1000;			// Do a 'find' again 
long CPerson::ms_lWatchWaitTime = 5000;		// Time to watch shot go
long CPerson::ms_lReselectDudeTime	= 3000;	// Time to go without finding a dude
															// before calling SelectDude() to find
															// possibly a closer one.
short CPerson::ms_sLogTabUserGlobal = 0;		// Logic table variable for group effects

// Let this auto-init to 0
short CPerson::ms_sFileCount;

// This is the one CPerson that can log its AI table transitions or
// CIdBank::IdNil.
U16	CPerson::ms_u16IdLogAI	= CIdBank::IdNil;

// The max amount a guy and step up while writhing.
#define WRITHING_VERTICAL_TOLERANCE		(MaxStepUpThreshold / 2)

#define ID_GUI_EDIT_TEXTURES				900

//#ifdef MOBILE
extern bool demoCompat; 
//#endif

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CPerson::Load(				// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	short sFileCount,					// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)				// In:  Version of file format to load.
{
	short sResult = 0;
	sResult = CDoofus::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	
	if (sResult == 0)
	{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
		{
			ms_sFileCount = sFileCount;

			// Load static data
			switch (ulFileVersion)
			{
				default:
				case 1:
					pFile->Read(&ms_dLongRange);
					pFile->Read(&ms_dInRangeLow);
					pFile->Read(&ms_dInRangeHigh);
					pFile->Read(&ms_lRandomAvoidTime);
					pFile->Read(&ms_lReseekTime);
					pFile->Read(&ms_lWatchWaitTime);
					pFile->Read(&ms_lReselectDudeTime);
					break;
			}
		}

		UCHAR uc;

		// Load data specific to CPerson (if any)
		switch (ulFileVersion)
		{
			default:
			case 14:
				pFile->Read(&uc);
				m_ePersonType = (Personatorium::Index) uc;
				pFile->Read(&m_sShowState);
				m_rstrLogicFile.Load(pFile);
				break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			{
				pFile->Read(&uc);
				m_ePersonType = (Personatorium::Index) uc;
				break;
			}
		}

		// Cache the samples that this type of person uses
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidSuffering1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidSuffering2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidSuffering3));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidSuffering4));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShot1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShot2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShot3));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShot4));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidBlownup1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidBlownup2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidBurning1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidBurning2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidDying1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidDying2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShooting1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShooting2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShooting3));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidShooting4));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidRandom1));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidRandom2));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidRandom3));
		CacheSample(*(g_apersons[m_ePersonType].Sample.psmidRandom4));

		m_bCivilian = (g_apersons[m_ePersonType].eLifestyle == Personatorium::Civilian);
		
		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = -1;
			TRACE("CPerson::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CPerson::Load():  CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CPerson::Save(				// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to save to
	short sFileCount)					// In:  File count (unique per file, never 0)
{
	short sResult = SUCCESS;
	// Call the base class save to save the u16InstanceID
	CDoofus::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dLongRange);
		pFile->Write(&ms_dInRangeLow);
		pFile->Write(&ms_dInRangeHigh);
		pFile->Write(&ms_lRandomAvoidTime);
		pFile->Write(&ms_lReseekTime);
		pFile->Write(&ms_lWatchWaitTime);
		pFile->Write(&ms_lReselectDudeTime);
		}

	// Save imbecile specific data if any
	pFile->Write((UCHAR*) &m_ePersonType);
	pFile->Write(&m_sShowState);
	m_rstrLogicFile.Save(pFile);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CPerson::Save() - Error writing to file\n");
		sResult = -1;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CPerson::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	// Set the current height, previous time, and Nav Net
	CDoofus::Startup();

	// Init other stuff
	Init();

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

short CPerson::Init(void)
{
	short sResult = 0;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Clear the hit comment
	m_bHitComment = false;

	// Init other stuff
	m_dVel = 0.0;						 
	m_dRot = 0.0;
	// Set to different starting state based on the design of the animation, but
	// for now, ok.  Then also set his current animation.
	m_state = CCharacter::State_Guard;
	m_eCurrentAction = m_eSuggestedAction = CDoofus::Action_Guard;
	m_dAcc = ms_dAccUser;
	m_panimCur = &m_animStand;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 500;

	// Stock up.
	m_stockpile.Copy(&CStockPile::ms_stockpileMax);

	m_stockpile.m_sHitPoints = g_apersons[m_ePersonType].sInitialHitPoints;

	// Get back up weapon type.
	m_eFallbackWeaponType	= g_apersons[m_ePersonType].Weapon.idWeapon1;
																		 
	// If not civilian . . .
	if (g_apersons[m_ePersonType].eLifestyle != Personatorium::Civilian)
		{
		// Bad guy.
		m_smash.m_bits = CSmash::Bad | CSmash::Character;
		}
	else
		{
		// Civilian.
		m_smash.m_bits = CSmash::Civilian | CSmash::Character;
		}

	m_smash.m_pThing = this;

	// Special case for the kids who cannot be hit.
	if (strncmp(g_apersons[m_ePersonType].pszDescription, "Kid", 3) == 0)
		m_smash.m_bits = 0;

	m_sBrightness = 0;	// Default Brightness level

	m_lCommentTimer = m_pRealm->m_time.GetGameTime() + ms_lMinCommentTime + GetRandom() % ms_lCommentTimeVariance;

	// Override the Doofus set timeout defaults by replacing the values with the
	// personatorium values.
	m_lGuardTimeout = g_apersons[m_ePersonType].lGuardTimeout;
	m_lRunShootInterval = g_apersons[m_ePersonType].lRunShootInterval;
	m_lShotReactionTimeout = g_apersons[m_ePersonType].lShotTimeout;

	// If the difficulty is set up to its highest level, then upgrade
	// guns to spray cannons and missiles to heatseekers.
	// Note we only do this if not in edit mode.
	if (m_pRealm->m_flags.sDifficulty == 11 && m_pRealm->m_flags.bEditing == false)
	{
		switch (m_eWeaponType)
		{
			case CUziID:
			case CAutoRifleID:
			case CSmallPistolID:
			case CPistolID:
			case CMachineGunID:
			case CShotGunID:
				m_eWeaponType = CAssaultWeaponID;
				break;

			case CRocketID:
				m_eWeaponType = CHeatseekerID;
				break;
		}
	}

	// Set initial group state to zero.  If you use dispensers then initalizing
	// this value here would be bad, but this is a special case for the last 
	// demo level with the kids and there aren't any dispensers on that level.
	ms_sLogTabUserGlobal = 0;

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CPerson::Update(void)
{
	CThing* pDemon = NULL;

	if (!m_sSuspend)
	{
		long lThisTime = m_pRealm->m_time.GetGameTime();

		// See if its time to reevaluate the states
		if (lThisTime > m_lEvalTimer)
		{
			m_lEvalTimer = lThisTime + 500 + (GetRandom() % 500);
			m_pLogicTable->Evaluate(this, ms_u16IdLogAI == GetInstanceID());
		}

		// Check for new messages that may change the state
		ProcessMessages();

		// See if its time to make a random comment yet
		if (lThisTime > m_lCommentTimer)
		{
			m_lCommentTimer = lThisTime + ms_lMinCommentTime + GetRandom() % ms_lCommentTimeVariance;
			PlaySoundRandom();
		}

		// See if there are any pylons nearby that he wants to use.
//		if (m_state == State_Idle || 
//		    m_state == State_Wait ||
//			 m_state == State_Hunt)
//			Logic_PylonDetect();
	 
		switch (m_state)
		{
			case State_Guard:
				Logic_Guard();
				break;

			case State_PopBegin:
				Logic_PopBegin();
				break;

			case State_PopWait:
				Logic_PopWait();
				break;

			case State_Popout:
				Logic_Popout();
				break;

			case State_RunShootBegin:
				Logic_RunShootBegin();
				break;

			case State_RunShoot:
				Logic_RunShoot();
				break;

			case State_RunShootWait:
				Logic_RunShootWait();
				break;

			case State_Shoot:
				Logic_Shoot();
				break;

			case State_ShootRun:
				Logic_ShootRun();
				break;

			case State_Hunt:
				Logic_Hunt();
				break;

			case State_HuntNext:
			case State_MoveNext:
			case State_MarchNext:
			case State_WalkNext:
				Logic_MoveNext();
				break;

			case State_HuntHold:
				Logic_HuntHold();
				break;

			case State_Shot:
				Logic_Shot();
				break;

			case State_BlownUp:
				Logic_BlownUp();
				break;

			case State_Burning:
				Logic_Burning();
				break;

			case State_Die:
				Logic_Die();
				break;

			case State_Writhing:
				Logic_Writhing();
				break;

			case State_Engage:
				Logic_Engage();
				break;

			case State_PositionSet:
				Logic_PositionSet();
				break;

			case State_PositionMove:
				Logic_PositionMove();
				break;

			case State_Retreat:
				Logic_Retreat();
				break;

			case State_Dead:
				if ((m_ePersonType == Personatorium::Nude1) || (m_ePersonType == Personatorium::Nude2))
					UnlockAchievement(ACHIEVEMENT_KILL_A_NAKED_PERSON);

				GameMessage msg;
				msg.msg_Death.eType = typeDeath;
				msg.msg_Death.sPriority = 0;
				pDemon = m_pRealm->m_aclassHeads[CThing::CDemonID].GetNext();
				if (pDemon)
					SendThingMessage(&msg, pDemon);				
				CDoofus::OnDead();
				delete this;  
				return;
				break;

			case State_PanicBegin:
				Logic_PanicBegin();
				break;

			case State_PanicContinue:
				Logic_PanicContinue();
				// Call this a few times to make it happen more often, since
				// it normally happens only every 10th time.
				PlaySoundRandom();
				PlaySoundRandom();
				PlaySoundRandom();
				PlaySoundRandom();
				break;

			case State_WalkBegin:
				Logic_WalkBegin();
				break;

			case State_WalkContinue:
				Logic_WalkContinue();
				break;

			case State_DelayShoot:
				Logic_DelayShoot();
				break;

			case State_AvoidFire:
				Logic_AvoidFire();
				break;

			case State_Helping:
				Logic_Helping();
				break;

			case State_March:
				Logic_MarchBegin();
				break;

			case State_HideBegin:
				Logic_HideBegin();
				break;

			case State_Hide:
				Logic_Hide();
				break;

			default:
				// If it ever gets to this case, then it has entered an unknown
				// state which may perhaps be out of range.  Please note the
				// value of the state to check to see if it is in the valid range
				// of states, and also note the m_ePreviousState to see if we can 
				// detect where the error occurred.
#if 1
				TRACE("Current state is %d - description is on next line if possible\n", m_state);
				if (m_state < NumThing3dStates)
					TRACE("Current state is %s\n", ms_apszStateNames[m_state]);
				ASSERT(0);
				ASSERT(m_state > 0);
				if (m_state == m_ePreviousState)
					TRACE("STATE-ALERT Probably an uninitialized previous state\n");
#else
				if (rspMsgBox(
						RSP_MB_ICN_EXCLAIM | RSP_MB_BUT_YESNO,
						g_pszAppName,
						"Person.cpp encountered an unexpected state.\n"
						"Displaying the state's description may cause a crash.\n"
						"Would you like to see the state's description?\n"
						"%s",
						(m_state < 0) ? "The state is negative!!" : "") == RSP_MB_RET_YES)
					{
					rspMsgBox(
						RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
						g_pszAppName,
						"The state is \"%s\" (%ld).\n"
						"%s",
						ms_apszStateNames[m_state],
						m_state,
						(m_state == m_ePreviousState) ? "STATE-ALERT Probably an uninitialized previous state!" : "");
					}
#endif
				break;

		}	

		// Update the avoidance smash
		m_smashAvoid.m_sphere.sphere.X = m_dX + (rspCos(m_dRot) * ms_lAvoidRadius);
		m_smashAvoid.m_sphere.sphere.Y = m_dY;
		m_smashAvoid.m_sphere.sphere.Z = m_dZ - (rspSin(m_dRot) * ms_lAvoidRadius);

		// Determine appropriate position for main smash.
		PositionSmash();

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);
		
		m_lPrevTime = lThisTime;

		// Call base class //////////////////////////////////////////////////////

		CCharacter::Update();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Render object.
////////////////////////////////////////////////////////////////////////////////
void CPerson::Render(void)
{
	if (m_sShowState != FALSE)
	{
		m_rstrLogicName = ms_apszStateNames[m_state];
		m_rstrLogicName += "/";
		m_rstrLogicName += ms_apszActionNames[m_eCurrentAction];
		m_sprite.m_pszText = (char*) m_rstrLogicName;
//		m_sprite.m_pszText = ms_apszStateNames[m_state];
	}
	else
		m_sprite.m_pszText		= NULL;

	CDoofus::Render();
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object.
////////////////////////////////////////////////////////////////////////////////
void CPerson::EditRender(void)
	{
	CDoofus::EditRender();

	if (m_sShowState != FALSE)
		{
		m_rstrLogicName	+= "/";
		WeaponDetails*	pwd	= GetWeaponDetails(m_eWeaponType);
		if (pwd)
			{
			m_rstrLogicName	+= pwd->pszName;
			}
		else
			{
			m_rstrLogicName	+= "Invalid weapon";
			}

		m_sprite.m_pszText = (char*) m_rstrLogicName;
		}
	else
		{
		WeaponDetails*	pwd	= GetWeaponDetails(m_eWeaponType);
		if (pwd)
			{
			m_sprite.m_pszText	= pwd->pszName;
			}
		else
			{
			m_sprite.m_pszText	= NULL;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Logic_Writhing - special case for the cop since he has to crawl
////////////////////////////////////////////////////////////////////////////////

void CPerson::Logic_Writhing(void)
{
//	m_dAnimRot = m_dRot;

	CDoofus::Logic_Writhing();

//	if (m_ePersonType == Personatorium::Cop || m_ePersonType == Personatorium::Gunner)
	if (g_apersons[m_ePersonType].eWrithingMotion != Personatorium::Still)
	{
		double dX = m_dX;
		double dZ = m_dZ;
		double dNewX;
		double dNewY;
		double dNewZ;
		double dSeconds = (m_pRealm->m_time.GetGameTime() - m_lPrevTime) / 1000.0;

		m_dAcc = ms_dAccUser;
		UpdateVelocities(dSeconds, ms_dMaxCrawlVel, ms_dMaxCrawlVel);
		GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);
		if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, WRITHING_VERTICAL_TOLERANCE) == true)
		{
		// Update Values /////////////////////////////////////////////////////////

			m_dX	= dNewX;
			m_dY	= dNewY;
			m_dZ	= dNewZ;

			UpdateFirePosition();
		}
		else
		{
		// Restore Values ////////////////////////////////////////////////////////

			m_dVel			-= m_dDeltaVel;
		}

#if 1
		// Get radius.
		short	sRadius			= m_sprite.m_sRadius;
		// Determine pseudo-head point.
		short	sRot				= rspMod360(m_dAnimRot);
		short	sPseudoHeadX	= m_dX + COSQ[sRot] * sRadius;
		short	sPseudoHeadY	= m_dZ - SINQ[sRot] * sRadius;
		// Check pseudo-head point.
		U16	u16Attrib	= 0;	// Safety.
		short	sHeight		= 0;	// Safety.
		GetFloorAttributes(sPseudoHeadX, sPseudoHeadY, &u16Attrib, &sHeight);
		if ( (u16Attrib & REALM_ATTR_NOT_WALKABLE) || sHeight > m_dY + WRITHING_VERTICAL_TOLERANCE
			|| (dX == m_dX && dZ == m_dZ) )
		{
			// Die real soon.
			m_stockpile.m_sHitPoints	= 0;
		}
		else
		{
			if (g_apersons[m_ePersonType].eWrithingMotion == Personatorium::PushBack)
			{
				// If they are supposed to push themselves on their back, then the
				// animation angle and the direction angle need to be 180 degrees
				// apart.
				if (m_dRot != rspMod360(m_dAnimRot + 180))
					m_dRot = rspMod360(m_dAnimRot + 180);
			}	
		}
#else
//		if (m_ePersonType == Personatorium::Gunner)
		if (g_apersons[m_ePersonType].eWrithingMotion == Personatorium::PushBack)
		{
	#if 1
			// If he hits something, just make him die since his hotspot
			// is not in the correct place, he rotates around his feet
			// and it looks weird.
			m_stockpile.m_sHitPoints = 0;
	
			if (m_dRot != rspMod360(m_dAnimRot + 180))
				m_dRot = rspMod360(m_dAnimRot + 180);
	#else
			if (dX == m_dX && dZ == m_dZ)
			{
				// This is a sort of randomness that will make some turn left and
				// others turn right depending on if their rotation is even or odd.
				if (((short) m_dRot) & 0x01)
					m_dRot = rspMod360(m_dRot - 20);
				else
					m_dRot = rspMod360(m_dRot + 20);
				m_dAnimRot = rspMod360(m_dRot + 180);
			}
	#endif
		}
		else
		{
			// If he didn't move at all, then turn him so he will
			// avoid the wall
			if (dX == m_dX && dZ == m_dZ)
			{
	#if 1
				// If he hits something, just make him die since his hotspot
				// is not in the correct place, he rotates around his feet
				// and it looks weird.
				m_stockpile.m_sHitPoints = 0;
	#else
				// This is a sort of randomness that will make some turn left and
				// others turn right depending on if their rotation is even or odd.
				if (((short) m_dRot) & 0x01)
					m_dAnimRot = m_dShootAngle = m_dRot = rspMod360(m_dRot - 20);
				else
					m_dAnimRot = m_dShootAngle = m_dRot = rspMod360(m_dRot + 20);
	#endif
			}
		}
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CPerson::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CPerson::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;

	sResult = CDoofus::EditNew(sX, sY, sZ);

	if (sResult == SUCCESS)
	{
		// Load resources
		sResult = GetResources();
		if (sResult == SUCCESS)
		{
			sResult	= Init();
		}
	}
	else
	{
		sResult = -1;
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Call back for the user-browse-for-logic-file button.
////////////////////////////////////////////////////////////////////////////////
static void LogicUserBrowse(	// Returns nothing
	RGuiItem* pgui)				// In: GUI pressed.
{
	RGuiItem* pguiLogicFileName	= (RGuiItem*)pgui->m_ulUserInstance;
	ASSERT(pguiLogicFileName != NULL);

	// Get the logic file name . . .
	char	szLogicFile[RSP_MAX_PATH];
	strcpy(szLogicFile, FullPathHD(pguiLogicFileName->m_szText));

	if (rspOpenBox(
		"Choose logic file",
		szLogicFile,
		szLogicFile,
		sizeof(szLogicFile),
		"lgk.") == 0)
	{
		char	szHDPath[RSP_MAX_PATH];
		strcpy(szHDPath, FullPathHD(""));
		// Attempt to remove HD path . . .
		if (rspStrnicmp(szLogicFile, szHDPath, strlen(szHDPath) ) == 0)
			{
			// Determine amount of path to ignore.
			long	lSubPathBegin	= strlen(szHDPath);

			// Update the GUI that shows the filename.
			pguiLogicFileName->SetText("%s", rspPathFromSystem(szLogicFile + lSubPathBegin) );
			pguiLogicFileName->Compose();
			}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CPerson::EditModify(void)
{
	short sResult = 0;
	Personatorium::Index eCurrentType = m_ePersonType;
	RGuiItem* pGuiItem = NULL;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/person.gui"));
	if (pGui)
	{
		RListBox* pPersonalityList = (RListBox*) pGui->GetItemFromId(3);
		RListBox* pWeaponList = (RListBox*) pGui->GetItemFromId(4);
		RMultiBtn* pmbtnLogAI = (RMultiBtn*) pGui->GetItemFromId(102);
		RMultiBtn* pmbtnShowState = (RMultiBtn*) pGui->GetItemFromId(103);
		REdit* peditLogicFile = (REdit*) pGui->GetItemFromId(100);
		REdit* peditStartBouy = (REdit*) pGui->GetItemFromId(300);
		REdit* peditEndBouy   = (REdit*) pGui->GetItemFromId(301);
		RBtn* pbtnLogicUserBrowse = (RBtn*) pGui->GetItemFromId(101);
		if (	pWeaponList && pPersonalityList && pmbtnLogAI 
			&&	pmbtnShowState && peditLogicFile && pbtnLogicUserBrowse &&
			peditStartBouy && peditEndBouy)
		{
			// Verify these are the type we think they are before accessing type specific
			// members.
			ASSERT(pPersonalityList->m_type == RGuiItem::ListBox);
			ASSERT(pWeaponList->m_type == RGuiItem::ListBox);
			ASSERT(pmbtnLogAI->m_type == RGuiItem::MultiBtn);
			ASSERT(pmbtnShowState->m_type == RGuiItem::MultiBtn);
			ASSERT(peditLogicFile->m_type == RGuiItem::Edit);
			ASSERT(peditStartBouy->m_type == RGuiItem::Edit);
			ASSERT(peditEndBouy->m_type == RGuiItem::Edit);
			ASSERT(pbtnLogicUserBrowse->m_type == RGuiItem::Btn);

			// Fill in list box with current available personalities
			short i;
			for (i = 0; i < Personatorium::NumPersons; i++)
			{
				pGuiItem = pPersonalityList->AddString(g_apersons[i].pszDescription);
				if (pGuiItem != NULL)
				{
					pGuiItem->m_lId = PERSONALITY_ITEM_ID_BASE + i;
					pGuiItem->m_ulUserData = (ULONG) i;
				}
			}

			pPersonalityList->AdjustContents();

			// Show currently selected personality type
			pGuiItem	= pGui->GetItemFromId(PERSONALITY_ITEM_ID_BASE + m_ePersonType);
			if (pGuiItem)
				{
				pPersonalityList->SetSel(pGuiItem);
				pPersonalityList->EnsureVisible(pGuiItem);
				}

			// Empty list box.  We don't want to modify the .GUI resource just yet
			// so we don't screw up people using the current .EXE on the server.
			pWeaponList->RemoveAll();

			// Fill in the list box with current available weapons.
			for (i = 0; i < NumWeaponTypes; i++)
				{
				if (	(i != DeathWad || CStockPile::ms_sEnableDeathWad) &&
						(i != DoubleBarrel || CStockPile::ms_sEnableDoubleBarrel) &&
						(i != ProximityMine) &&
						(i != TimedMine) &&
						(i != RemoteControlMine) &&
						(i != BouncingBettyMine) )
					{
					pGuiItem	= pWeaponList->AddString(ms_awdWeapons[i].pszName);
					if (pGuiItem != NULL)
						{
						// Store class ID so we can determine user selection
						pGuiItem->m_lId	= ms_awdWeapons[i].id;
						}
					}
				}

			pWeaponList->AdjustContents();

			// Show which weapon is currently selected
			pGuiItem	= pGui->GetItemFromId(m_eWeaponType);
			if (pGuiItem)
				{
				pWeaponList->SetSel(pGuiItem);
				pWeaponList->EnsureVisible(pGuiItem);
				}

			// Set current state for AI log check box, if this is the guy being logged.
			pmbtnLogAI->m_sState	= (ms_u16IdLogAI == GetInstanceID()) ? 2 : 1;
			// Reflect changes.
			pmbtnLogAI->Compose();

			// Set current state for show state check box.
			pmbtnShowState->m_sState = (m_sShowState == FALSE) ? 1 : 2;
			// Reflect changes.
			pmbtnShowState->Compose();

			// Set current logic file.
			peditLogicFile->SetText("%s", (char*) m_rstrLogicFile);
			// Reflect changes.
			peditLogicFile->Compose();

			// Set current start bouy
			peditStartBouy->SetText("%d", m_ucSpecialBouy0ID);
			peditStartBouy->Compose();

			// Set current end bouy
			peditEndBouy->SetText("%d", m_ucSpecialBouy1ID);
			peditEndBouy->Compose();

			// Set callback for logic browser button.
			pbtnLogicUserBrowse->m_bcUser	= LogicUserBrowse;
			// Set instance data to GUI to query/update.
			pbtnLogicUserBrowse->m_ulUserInstance	= (ULONG)peditLogicFile;

			SetGuiToNotify(pGui->GetItemFromId(ID_GUI_EDIT_TEXTURES) );

			sResult = DoGui(pGui);
			switch (sResult)
			{
				case ID_GUI_EDIT_TEXTURES:	// Edit textures
				case 1:	// OK
				{
					RGuiItem* pSelection = pWeaponList->GetSel();
					if (pSelection)
					{
						m_eWeaponType	= pSelection->m_lId;
					}
					pSelection = pPersonalityList->GetSel();
					if (pSelection)
					{
						m_ePersonType = (Personatorium::Index) pSelection->m_ulUserData;
						if (m_ePersonType != eCurrentType)
						{
							// switch to new animation resources.
							FreeResources();
							GetResources();
							m_bCivilian = (g_apersons[m_ePersonType].eLifestyle == Personatorium::Civilian);
						}
					}

					// If the AI log checkbox is checked . . .
					if (pmbtnLogAI->m_sState == 2)
					{
						// This is the guy who gets his AI logged.
						ms_u16IdLogAI	= GetInstanceID();
					}
					else
					{
						// If this was the guy who got his AI logged . . .
						if (ms_u16IdLogAI == GetInstanceID() )
						{
							// No one is logging.
							ms_u16IdLogAI	= CIdBank::IdNil;
						}
					}

					// Determine whether or not to display state on screen.
					m_sShowState	= (pmbtnShowState->m_sState	== 2) ? TRUE : FALSE;

					// Copy logic file to use.
					m_rstrLogicFile.Grow(256);
					peditLogicFile->GetText((char*) m_rstrLogicFile, 255);
					m_rstrLogicFile.Update();

					// Get the bouy settings
					m_ucSpecialBouy0ID = peditStartBouy->GetVal();
					m_ucSpecialBouy1ID = peditEndBouy->GetVal();

					if (sResult == ID_GUI_EDIT_TEXTURES)
						{
						// Form save name.
						RString	strFile;
						strFile	= g_resmgrGame.GetBasePath();
						strFile	+= g_apersons[m_ePersonType].Anim.pszBaseName;
						if (g_apersons[m_ePersonType].Anim.sTextureScheme >= 0)
							strFile	+= g_apersons[m_ePersonType].Anim.sTextureScheme;
						strFile	+= ".tex";

						CTexEdit	te;
						te.DoModal(&m_animStand, m_pRealm->m_phood->m_pltAmbient, m_pRealm->m_phood->m_pltSpot, strFile);
						}
					break;
				}
			}
		}
	}
	delete pGui;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CPerson::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CPerson::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;
	short sLoadResult = 0;

	
	// Load Stand animation
	sResult = m_animStand.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									 g_apersons[m_ePersonType].Anim.sTextureScheme,
									 "stand",
									 g_apersons[m_ePersonType].Anim.pszObjectName,
									 g_apersons[m_ePersonType].Anim.pszEventName,
									 g_apersons[m_ePersonType].Anim.pszHandName,
									 RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Load Run animation, or load stand in its place if that fails
	sLoadResult = m_animRun.Get(g_apersons[m_ePersonType].Anim.pszBaseName, 
									 g_apersons[m_ePersonType].Anim.sTextureScheme,
	                        "run",
					   			 g_apersons[m_ePersonType].Anim.pszObjectName,
										g_apersons[m_ePersonType].Anim.pszEventName,
								    g_apersons[m_ePersonType].Anim.pszHandName,
									 RChannel_LoopAtStart | RChannel_LoopAtEnd);
	if (sLoadResult != SUCCESS)
		sResult |= m_animRun.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
										 g_apersons[m_ePersonType].Anim.sTextureScheme,
										 "stand",
										 g_apersons[m_ePersonType].Anim.pszObjectName,
		  								 g_apersons[m_ePersonType].Anim.pszEventName,
									    g_apersons[m_ePersonType].Anim.pszHandName,
										 RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Load Shoot animation or load stand in its place if that fails
	sLoadResult = m_animShoot.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									   g_apersons[m_ePersonType].Anim.sTextureScheme,
										"shoot",
										g_apersons[m_ePersonType].Anim.pszObjectName,
										g_apersons[m_ePersonType].Anim.pszEventName,
									   g_apersons[m_ePersonType].Anim.pszHandName,
										0);
	if (sLoadResult != SUCCESS)
		sResult |= m_animShoot.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									    g_apersons[m_ePersonType].Anim.sTextureScheme,
										 "stand",
										 g_apersons[m_ePersonType].Anim.pszObjectName,
										 g_apersons[m_ePersonType].Anim.pszEventName,
									    g_apersons[m_ePersonType].Anim.pszHandName,
										 RChannel_LoopAtStart | RChannel_LoopAtEnd);


	// Load Shot animation or load stand in its place if that fails
	sLoadResult = m_animShot.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									     g_apersons[m_ePersonType].Anim.sTextureScheme,
									     "shot",
									     g_apersons[m_ePersonType].Anim.pszObjectName,
									     g_apersons[m_ePersonType].Anim.pszEventName,
									     g_apersons[m_ePersonType].Anim.pszHandName,
									     0);
	if (sLoadResult != SUCCESS)
		sResult |= m_animShot.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									     g_apersons[m_ePersonType].Anim.sTextureScheme,
										  "stand",
										  g_apersons[m_ePersonType].Anim.pszObjectName,
										  g_apersons[m_ePersonType].Anim.pszEventName,
									     g_apersons[m_ePersonType].Anim.pszHandName,
										  RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Load Die animation or load stand in its place if that fails										
	sLoadResult = m_animDie.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									    g_apersons[m_ePersonType].Anim.sTextureScheme,
									    "die",
									    g_apersons[m_ePersonType].Anim.pszObjectName,
										 g_apersons[m_ePersonType].Anim.pszEventName,
									    g_apersons[m_ePersonType].Anim.pszHandName,
									    0);
	if (sLoadResult != SUCCESS)
		sResult |= m_animDie.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									    g_apersons[m_ePersonType].Anim.sTextureScheme,
										 "stand",
										 g_apersons[m_ePersonType].Anim.pszObjectName,
										 g_apersons[m_ePersonType].Anim.pszEventName,
									    g_apersons[m_ePersonType].Anim.pszHandName,
										 RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Load Writhing animation or load stand in its place if that fails
	sLoadResult = m_animWrithing.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									         g_apersons[m_ePersonType].Anim.sTextureScheme,
									         "writhing",
									         g_apersons[m_ePersonType].Anim.pszObjectName,
											   g_apersons[m_ePersonType].Anim.pszEventName,
											   g_apersons[m_ePersonType].Anim.pszHandName,
									         RChannel_LoopAtEnd);
	if (sLoadResult != SUCCESS)
		sResult |= m_animWrithing.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
									         g_apersons[m_ePersonType].Anim.sTextureScheme,
										      "stand",
										      g_apersons[m_ePersonType].Anim.pszObjectName,
										      g_apersons[m_ePersonType].Anim.pszEventName,
										      g_apersons[m_ePersonType].Anim.pszHandName,
										      RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Load Executed animation or load stand in its place if that fails
	sLoadResult = m_animExecuted.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												g_apersons[m_ePersonType].Anim.sTextureScheme,
												"executed",
											   g_apersons[m_ePersonType].Anim.pszObjectName,
												g_apersons[m_ePersonType].Anim.pszEventName,
												g_apersons[m_ePersonType].Anim.pszHandName,
												0);
	if (sLoadResult != SUCCESS)
		sResult |= m_animExecuted.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												g_apersons[m_ePersonType].Anim.sTextureScheme,
												"stand",
												g_apersons[m_ePersonType].Anim.pszObjectName,
												g_apersons[m_ePersonType].Anim.pszEventName,
												g_apersons[m_ePersonType].Anim.pszHandName,
												RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// Load Onfire animation animation or load stand in its place if that fails
	sLoadResult = m_animOnfire.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
											 g_apersons[m_ePersonType].Anim.sTextureScheme,
											 "onfire",
											 g_apersons[m_ePersonType].Anim.pszObjectName,
											 g_apersons[m_ePersonType].Anim.pszEventName,
											 g_apersons[m_ePersonType].Anim.pszHandName,
											 RChannel_LoopAtEnd);
	if (sLoadResult != SUCCESS)
	{
		sResult |= m_animOnfire.Get(g_apersons[m_ePersonType].Anim.pszBaseName, 
											 g_apersons[m_ePersonType].Anim.sTextureScheme,
											 "run",
											 g_apersons[m_ePersonType].Anim.pszObjectName,
											 g_apersons[m_ePersonType].Anim.pszEventName,
											 g_apersons[m_ePersonType].Anim.pszHandName,
											 RChannel_LoopAtEnd);


		if (sLoadResult != SUCCESS)
			sResult |= m_animOnfire.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												 g_apersons[m_ePersonType].Anim.sTextureScheme,
												 "stand",
												 g_apersons[m_ePersonType].Anim.pszObjectName,
												 g_apersons[m_ePersonType].Anim.pszEventName,
												 g_apersons[m_ePersonType].Anim.pszHandName,
												 RChannel_LoopAtStart | RChannel_LoopAtEnd);
	}

	// Load Walk animation animation or load stand in its place if that fails
	sLoadResult = m_animWalk.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
										  g_apersons[m_ePersonType].Anim.sTextureScheme,
										  "walk",
										  g_apersons[m_ePersonType].Anim.pszObjectName,
										  g_apersons[m_ePersonType].Anim.pszEventName,
										  g_apersons[m_ePersonType].Anim.pszHandName,
										  RChannel_LoopAtEnd);
	if (sLoadResult != SUCCESS)
		sResult |= m_animWalk.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
										  g_apersons[m_ePersonType].Anim.sTextureScheme,
										  "stand",
										  g_apersons[m_ePersonType].Anim.pszObjectName,
										  g_apersons[m_ePersonType].Anim.pszEventName,
										  g_apersons[m_ePersonType].Anim.pszHandName,
										  RChannel_LoopAtStart | RChannel_LoopAtEnd);
	if (sResult != SUCCESS)
		TRACE("CPerson::GetResources - Error loading animation resources\n");

	// Load ShootRun animation, or try Shoot if there isn't one, or stand as last option
	sLoadResult = m_animShootRun.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												g_apersons[m_ePersonType].Anim.sTextureScheme,
												"runshoot",
												g_apersons[m_ePersonType].Anim.pszObjectName,
												g_apersons[m_ePersonType].Anim.pszEventName,
												g_apersons[m_ePersonType].Anim.pszHandName,
												0);
	if (sLoadResult != SUCCESS)
	{
		sLoadResult = m_animShootRun.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													g_apersons[m_ePersonType].Anim.sTextureScheme,
													"shoot",
													g_apersons[m_ePersonType].Anim.pszObjectName,
													g_apersons[m_ePersonType].Anim.pszEventName,
													g_apersons[m_ePersonType].Anim.pszHandName,
													0);
		if (sLoadResult != SUCCESS)
			sResult |= m_animShootRun.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													g_apersons[m_ePersonType].Anim.sTextureScheme,
													"stand",
													g_apersons[m_ePersonType].Anim.pszObjectName,
													g_apersons[m_ePersonType].Anim.pszEventName,
													g_apersons[m_ePersonType].Anim.pszHandName,
													0);
	}

	// Load ShootRunR0 animation, or try Shoot if there isn't one, or stand as last option
	sLoadResult = m_animShootRunR0.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												  g_apersons[m_ePersonType].Anim.sTextureScheme,
												  "45rt",
													g_apersons[m_ePersonType].Anim.pszObjectName,
													g_apersons[m_ePersonType].Anim.pszEventName,
													g_apersons[m_ePersonType].Anim.pszHandName,
													0);
	if (sLoadResult != SUCCESS)
	{
		sLoadResult = m_animShootRunR0.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "shoot",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
												  	  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
		if (sLoadResult != SUCCESS)
			sResult |= m_animShootRunR0.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "stand",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
													  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
	}
	
	// Load ShootRunR1 animation, or try Shoot if there isn't one, or stand as last option
	sLoadResult = m_animShootRunR1.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												  g_apersons[m_ePersonType].Anim.sTextureScheme,
												  "90rt",
												  g_apersons[m_ePersonType].Anim.pszObjectName,
												  g_apersons[m_ePersonType].Anim.pszEventName,
												  g_apersons[m_ePersonType].Anim.pszHandName,
												  0);
	if (sLoadResult != SUCCESS)
	{
		sLoadResult = m_animShootRunR1.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "shoot",
												  	  g_apersons[m_ePersonType].Anim.pszObjectName,
												 	  g_apersons[m_ePersonType].Anim.pszEventName,
												 	  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
		if (sLoadResult != SUCCESS)
			sResult |= m_animShootRunR1.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "stand",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
													  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
	}

	// Load ShootRunL0 animation, or try Shoot if there isn't one, or stand as last option
	sLoadResult = m_animShootRunL0.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												  g_apersons[m_ePersonType].Anim.sTextureScheme,
												  "45lft",
												  g_apersons[m_ePersonType].Anim.pszObjectName,
												  g_apersons[m_ePersonType].Anim.pszEventName,
												  g_apersons[m_ePersonType].Anim.pszHandName,
												  0);
	if (sLoadResult != SUCCESS)
	{
		sLoadResult = m_animShootRunL0.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "shoot",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
													  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
												     0);
		if (sLoadResult != SUCCESS)
			sResult |= m_animShootRunL0.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "stand",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
													  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
	}

	// Load ShootRunL1 animation, or try Shoot if there isn't one, or stand as last option
	sLoadResult = m_animShootRunL1.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
												  g_apersons[m_ePersonType].Anim.sTextureScheme,
												  "90lft",
												  g_apersons[m_ePersonType].Anim.pszObjectName,
												  g_apersons[m_ePersonType].Anim.pszEventName,
											     g_apersons[m_ePersonType].Anim.pszHandName,
												  0);
	if (sLoadResult != SUCCESS)
	{
		sLoadResult = m_animShootRunL1.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "shoot",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
													  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
		if (sLoadResult != SUCCESS)
			sResult |= m_animShootRunL1.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													  g_apersons[m_ePersonType].Anim.sTextureScheme,
													  "stand",
													  g_apersons[m_ePersonType].Anim.pszObjectName,
													  g_apersons[m_ePersonType].Anim.pszEventName,
													  g_apersons[m_ePersonType].Anim.pszHandName,
													  0);
	}

	// Load ShootRunBack animation, or try Shoot if there isn't one, or stand as last option
	sLoadResult = m_animShootRunBack.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													 g_apersons[m_ePersonType].Anim.sTextureScheme,
													 "runbackwr",
													 g_apersons[m_ePersonType].Anim.pszObjectName,
													 g_apersons[m_ePersonType].Anim.pszEventName,
													 g_apersons[m_ePersonType].Anim.pszHandName,
													 0);
	if (sLoadResult != SUCCESS)
	{
		sLoadResult = m_animShootRunBack.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
													    g_apersons[m_ePersonType].Anim.sTextureScheme,
														 "shoot",
														 g_apersons[m_ePersonType].Anim.pszObjectName,
														 g_apersons[m_ePersonType].Anim.pszEventName,
														 g_apersons[m_ePersonType].Anim.pszHandName,
														 0);
		if (sLoadResult != SUCCESS)
			sResult |= m_animShootRunBack.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
														 g_apersons[m_ePersonType].Anim.sTextureScheme,
													    "stand",
														 g_apersons[m_ePersonType].Anim.pszObjectName,
														 g_apersons[m_ePersonType].Anim.pszEventName,
														 g_apersons[m_ePersonType].Anim.pszHandName,
														 0);
	}

	// Load Crouch animation, or load stand in its place if that fails
	sLoadResult = m_animCrouch.Get(g_apersons[m_ePersonType].Anim.pszBaseName, 
											 g_apersons[m_ePersonType].Anim.sTextureScheme,
											 "crouch",
					   					 g_apersons[m_ePersonType].Anim.pszObjectName,
											 g_apersons[m_ePersonType].Anim.pszEventName,
											 g_apersons[m_ePersonType].Anim.pszHandName,
											 0);
	if (sLoadResult != SUCCESS)
		sResult |= m_animCrouch.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
											 g_apersons[m_ePersonType].Anim.sTextureScheme,
											 "stand",
											 g_apersons[m_ePersonType].Anim.pszObjectName,
											 g_apersons[m_ePersonType].Anim.pszEventName,
											 g_apersons[m_ePersonType].Anim.pszHandName,
											 0);
									
	// Load Search animation, or load stand in its place if that fails
	sLoadResult = m_animSearch.Get(g_apersons[m_ePersonType].Anim.pszBaseName, 
											 g_apersons[m_ePersonType].Anim.sTextureScheme,
											 "search",
					   					 g_apersons[m_ePersonType].Anim.pszObjectName,
											 g_apersons[m_ePersonType].Anim.pszEventName,
											 g_apersons[m_ePersonType].Anim.pszHandName,
											 RChannel_LoopAtStart | RChannel_LoopAtEnd);
	if (sLoadResult != SUCCESS)
		sResult |= m_animSearch.Get(g_apersons[m_ePersonType].Anim.pszBaseName,
										    g_apersons[m_ePersonType].Anim.sTextureScheme,
											 "stand",
											 g_apersons[m_ePersonType].Anim.pszObjectName,
											 g_apersons[m_ePersonType].Anim.pszEventName,
											 g_apersons[m_ePersonType].Anim.pszHandName,
											 RChannel_LoopAtStart | RChannel_LoopAtEnd);


	// Get execution target points -- NOT essential.
	char	szExeTargetResName[RSP_MAX_PATH];
	sprintf(szExeTargetResName, "%s_writhing_exe.trans", g_apersons[m_ePersonType].Anim.pszBaseName);
	sLoadResult	= rspGetResource(&g_resmgrGame, szExeTargetResName, &m_ptransExecutionTarget);
	if (sLoadResult == 0)
		{
		m_ptransExecutionTarget->SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);
		}

	// Get Logic Table
	sResult |= rspGetResource(&g_resmgrRes, (char*) m_rstrLogicFile, &m_pLogicTable);

	// Get base class resources.
	sResult |= CDoofus::GetResources();


	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CPerson::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animRun.Release();
	m_animStand.Release();
	m_animShoot.Release();
	m_animShot.Release();
	m_animDie.Release();
	m_animWrithing.Release();
	m_animExecuted.Release();
	m_animOnfire.Release();
	m_animWalk.Release();
	m_animShootRun.Release();
	m_animShootRunL0.Release();
	m_animShootRunL1.Release();
	m_animShootRunR0.Release();
	m_animShootRunR1.Release();
	m_animShootRunBack.Release();
	m_animCrouch.Release();
	m_animSearch.Release();
	rspReleaseResource(&g_resmgrRes, &m_pLogicTable);

	// If we have the execution target points . . .
	if (m_ptransExecutionTarget)
		rspReleaseResource(&g_resmgrGame, &m_ptransExecutionTarget);

	// Release base class resources.
	CDoofus::ReleaseResources();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundWrithing - choose among the writhing sound effects
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundWrithing(
			long* plDuration)					// Out:  Duration of sample, if not NULL.
{
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;

//#ifdef MOBILE //Reduce annoying comments when dying
	if ((demoCompat) || (++m_usCommentCounter % 5 == 0) )
	{
//#endif
	switch (GetRandom() % 4)
	{
		case 0:
			psmid	= g_apersons[m_ePersonType].Sample.psmidSuffering1;
			break;

		case 1:
			psmid	= g_apersons[m_ePersonType].Sample.psmidSuffering2;
			break;

		case 2:
			psmid	= g_apersons[m_ePersonType].Sample.psmidSuffering3;
			break;

		case 3:
			psmid	= g_apersons[m_ePersonType].Sample.psmidSuffering4;
			break;
	}
	
	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmid,										// In:  Identifier of sample you want played.
		SampleMaster::Suffering,				// In:  Sound Volume Category for user adjustment
		-1,											// In:  Initial Sound Volume (0 - 255)
														// Negative indicates to use the distance to the ear.
		&m_siPlaying,								// Out: Handle for adjusting sound volume
		plDuration);								// Out: Sample duration in ms, if not NULL.
//#ifdef MOBILE
	}
//#endif
	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundShot - choose among the shot sound effects
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundShot(void)
{
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;
	long lThisTime = m_pRealm->m_time.GetGameTime();
	long	lSampleDuration	= 0;	// Safety.

	if (lThisTime > m_lSampleTimeIsPlaying)
	{
		if (m_state == State_Writhing)
		{
			// Use the generic grunt when shot from writhing, so that they
			// don't accidently say something stupid like Oh my leg when
			// you execute them by shooting them in the head.
			psmid = &g_smidRubinAh;
		}
		else
		{
			switch(GetRandom() % 4)
			{
				case 0:
					if (m_bHitComment)
					{
						psmid = g_apersons[m_ePersonType].Sample.psmidShot2;
					}
					else
					{	
						psmid	= g_apersons[m_ePersonType].Sample.psmidShot1;
						m_bHitComment = true;
					}
					break;

				case 1:
					psmid	= g_apersons[m_ePersonType].Sample.psmidShot2;
					break;

				case 2:
					psmid	= g_apersons[m_ePersonType].Sample.psmidShot3;
					break;

				case 3:
					psmid	= g_apersons[m_ePersonType].Sample.psmidShot4;
					break;
			}
		}

		// We must get the sample duration from the PlaySample() function and NOT
		// from the RSnd that it returns.  The reason is that if we fail to play
		// the sample (perhaps all the sound channels are in use), we still need
		// the time reflected for the duration to be the same so the game will
		// run consistently no matter the speed of sound playback.  It could even
		// be the case that there is no sound card in which case the only way to
		// get it to run the same from machine to machine is to use the sample
		// duration whether we succeed or fail to play the sample.
		m_lSampleTimeIsPlaying = lThisTime + lSampleDuration;
	}

	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmid,										// In:  Identifier of sample you want played.
		SampleMaster::Pain,						// In:  Sound Volume Category for user adjustment
		-1,											// In:  Initial Sound Volume (0 - 255)
														// Negative indicates to use the distance to the ear.
		&m_siPlaying,								// Out: Handle for adjusting sound volume
		&lSampleDuration);						// Out: Sample duration in ms, if not NULL.

	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundBlownup - choose among the blown up sound effects
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundBlownup(void)
{	
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;

	switch (GetRandom() % 2)
	{
		case 0:
			psmid	= g_apersons[m_ePersonType].Sample.psmidBlownup1;
			break;

		case 1:
			psmid	= g_apersons[m_ePersonType].Sample.psmidBlownup2;
			break;
	}

	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmid,										// In:  Identifier of sample you want played.
		SampleMaster::Pain,						// In:  Sound Volume Category for user adjustment
		-1,											// In:  Initial Sound Volume (0 - 255)
														// Negative indicates to use the distance to the ear.
		&m_siPlaying);								// Out: Handle for adjusting sound volume

	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundBurning - choose among burning yells
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundBurning(void)
{
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;

	switch (GetRandom() % 2)
	{
		case 0:
			psmid	= g_apersons[m_ePersonType].Sample.psmidBurning1;
			break;

		case 1:
			psmid	= g_apersons[m_ePersonType].Sample.psmidBurning2;
			break;
	}

	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmid,										// In:  Identifier of sample you want played.
		SampleMaster::Pain,						// In:  Sound Volume Category for user adjustment
		-1,											// In:  Initial Sound Volume (0 - 255)
														// Negative indicates to use the distance to the ear.
		&m_siPlaying);											// Out: Handle for adjusting sound volume

	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundShooting - choose among comments to say before shooting
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundShooting(void)
{
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;

	if (++m_usCommentCounter % 10 == 0 && m_idDude != CIdBank::IdNil)
	{
		switch (GetRandom() % 4)
		{
			case 0:
				psmid	= g_apersons[m_ePersonType].Sample.psmidShooting1;
				break;
				
			case 1:
				psmid	= g_apersons[m_ePersonType].Sample.psmidShooting2;
				break;

			case 2:
				psmid	= g_apersons[m_ePersonType].Sample.psmidShooting3;
				break;

			case 3:
				psmid	= g_apersons[m_ePersonType].Sample.psmidShooting4;
				break;
		}		
	}

	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmid,										// In:  Identifier of sample you want played.
		SampleMaster::Voices,					// In:  Sound Volume Category for user adjustment
		-1,											// In:  Initial Sound Volume (0 - 255)
														// Negative indicates to use the distance to the ear.
		&m_siPlaying);								// Out: Handle for adjusting sound volume

	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundDying - choose among dying sounds and comments
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundDying(void)
{
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;

	if (++m_usCommentCounter % 4 == 0)
	{
		switch (GetRandom() % 2)
		{
			case 0:
				psmid	= g_apersons[m_ePersonType].Sample.psmidDying1;
				break;

			case 1:
				psmid	= g_apersons[m_ePersonType].Sample.psmidDying2;
				break;
		}
	}

	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmid,										// In:  Identifier of sample you want played.
		SampleMaster::Suffering,				// In:  Sound Volume Category for user adjustment
		-1,											// In:  Initial Sound Volume (0 - 255)
														// Negative indicates to use the distance to the ear.
		&m_siPlaying);								// Out: Handle for adjusting sound volume

	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// PlaySoundRandom - choose among several random comments
////////////////////////////////////////////////////////////////////////////////

SampleMaster::SoundInstance CPerson::PlaySoundRandom(void)
{
	m_siPlaying = 0;
	SampleMasterID*	psmid	= &g_smidNil;

//#ifdef MOBILE //reduce NPC random comments
	int n;
	if (demoCompat)
		n = 10;
	else
		n = 20;
	if (++m_usCommentCounter % n == 0 && m_idDude != CIdBank::IdNil)
//#else
	//if (++m_usCommentCounter % 10 == 0 && m_idDude != CIdBank::IdNil)
//#endif
	{
		// Make sure the dude you are tracking is not dead before making any
		// stupid comments about him, like "where did he go?", "Get that guy"
		if (m_idDude != CIdBank::IdNil)
		{
			CDude* pdude;
			if (m_pRealm->m_idbank.GetThingByID((CThing**) &pdude, m_idDude) == 0)
			{
				if (pdude && pdude->m_state != State_Dead)
				{
					switch (GetRandom() % 4)
					{
						case 0:
							psmid	= g_apersons[m_ePersonType].Sample.psmidRandom1;
							break;

						case 1:
							psmid	= g_apersons[m_ePersonType].Sample.psmidRandom2;
							break;

						case 2:
							psmid	= g_apersons[m_ePersonType].Sample.psmidRandom3;
							break;

						case 3:
							psmid	= g_apersons[m_ePersonType].Sample.psmidRandom4;
							break;
					}

					PlaySample(										// Returns nothing.
																		// Does not fail.
						*psmid,										// In:  Identifier of sample you want played.
						SampleMaster::Voices,					// In:  Sound Volume Category for user adjustment
						-1,											// In:  Initial Sound Volume (0 - 255)
																		// Negative indicates to use the distance to the ear.
						&m_siPlaying);											// Out: Handle for adjusting sound volume
				}
			}
		}
	}

	return m_siPlaying;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
