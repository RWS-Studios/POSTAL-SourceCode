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
// Demon.cpp
// Project: Postal
// 
// History:
//		06/09/97 BRH	Started this from from SoundThing.cpp
//
//		06/10/97 BRH	Added comments for deaths and added a comment counter
//							so it doesn't say something all of the time.
//
//		06/11/97 BRH	Changed the demon icon.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/29/97	JMI	Converted EditRect(), EditRender(), and/or Render() to
//							use Map3Dto2D().
//
//		06/30/97 BRH	Took out the cache samples in the Preload function for
//							now so that the sounds won't be preloaded.  If we 
//							decide to have a memory based setting for this as an
//							option, then it can check it here and decide to load
//							the samples or not.  Also changed the PlaySample
//							calls to PlaySampleThenPurge to keep the memory usage
//							down.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
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
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/20/97 BRH	Changed weapon case statements to use the CDude's
//							enumerated type so that if the keys are switched
//							around again, the comments will still be correctly
//							associated with their weapon type.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/26/97 BRH	Got rid of a few phrases and added a few others in their
//							place.  Now there is only one Going Postal instead of 4,
//							and a few other phrases that weren't being used are now
//							in.
//
//		12/02/97	JMI	Added new m_sSoundBank member (edittable via EditModify()
//							that allows one to specify a sound bank index for
//							additional sounds).
//							Also, removed unused vars:
//							m_lNextStartTime, m_lLastStartTime, m_sWhichTime, 
//							m_bEnabled, m_bRepeats, m_bInitiallyEnabled, 
//							m_bInitiallyRepeats, m_lMinTime[], m_lRndTime[], 
//							m_szResName, and m_id.
//							Also, now loads the GUI off the HD b/c the Add On packs
//							new assets (such as Demon.GUI) will have to be on the
//							HD so we can require the user have the original Postal
//							CD in the drive while playing.
//							Also, now saves position and defaults to position on the
//							the screen (that way older levels that didn't save the
//							position will have the demon on the screen).
//
//		01/07/98 BRH	Added level specific new Demon sounds for the Add On Pack
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will play sounds with various options.
//
//////////////////////////////////////////////////////////////////////////////
#define DEMON_CPP

#include "RSPiX.h"
#include "demon.h"
#include "game.h"
#include "dude.h"

// To help identify the m_sSoundBank number for each new Add on level
#define DEMON_SHANTY_LEVEL			1
#define DEMON_RESORT_LEVEL			2
#define DEMON_WALMART_LEVEL		3
#define DEMON_EARTHQUAKE_LEVEL	4

////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////

long CDemon::ms_lMinIdleTime = 2000;		// Time before saying next thing
long CDemon::ms_lBonusKillTime = 5000;		// Kill an amount in this time, get bonus

// Sound banks of explosion comments indexed by m_sSoundBank.
SampleMasterID* CDemon::ms_apsmidExplosion[NumSoundBanks][NumExplosionComments]	=
	{
		{	// 0 == Normal.
		&g_smidDemonSlam,
		&g_smidDemonYes1,
		&g_smidDemonGoodOne,
		&g_smidDemonHa,
		&g_smidDemonButtSauce,
		&g_smidDemonHesOut,
		&g_smidDemonInHell,
		&g_smidDemonBuckwheat4,
		},

		{ // 1 = Shanty Town
		&g_smidDemonKeepTheChange,
		&g_smidDemonPropertyValues,
		&g_smidDemonCantHaveAnyNice,
		&g_smidDemonWelfareReform,
		&g_smidDemonGrenadeWorksGreat,
		&g_smidDemonYouBlewItUp,
		&g_smidDemonKeepTheChange,
		&g_smidDemonAltLifestyles,
		},

		{ // 2 = Resort
		&g_smidDemonProsecutionRests,
		&g_smidDemonCaseDismissed,
		&g_smidDemonCheckOutEarly,
		&g_smidDemonHoleInOne,
		&g_smidDemonRippedBday,
		&g_smidDemonShakeItUpBaby,
		&g_smidDemonWatchItWiggle,
		&g_smidDemonNoDecency,
		},

		{ // 3 = Wal Mart
		&g_smidDemonSatisfactGnty,
		&g_smidDemonCustomerRight,
		&g_smidDemonCleanupAisle5,
		&g_smidDemonBrownBagBody,
		&g_smidDemonBastardsWCoupons,
		&g_smidDemonNoRefunds,
		&g_smidDemonWhatLaneClosed,
		&g_smidDemonTenItemsOrLess,
		},

		{ // 4 = Earthquake
		&g_smidDemonYouBlewItUp,
		&g_smidDemonKillingGoodSoal,
		&g_smidDemonAngelOfDeath,
		&g_smidDemonAwwBoBo,
		&g_smidDemonYouBlewItUp,
		&g_smidDemonDeathMyFriend,
		&g_smidDemonDieWeakling,
		&g_smidDemonIsThereDoctor,
		}
	};

// Sound banks of burn comments indexed by m_sSoundBank.
SampleMasterID* CDemon::ms_apsmidBurn[NumSoundBanks][NumBurnComments]	=
	{
		{	// 0 == Normal.
		&g_smidDemonSmellChicken,
		&g_smidDemonFeelHeat,
		&g_smidDemonBurn,
		&g_smidDemonBurnBaby,
		},

		{  // 1 = Shanty Town
		&g_smidDemonSmellsSourMilk,
		&g_smidDemonBegForThis,
		&g_smidDemonFreezingWarm,
		&g_smidDemonBurningGovtCheese,
		},

		{ // 2 = Resort
		&g_smidDemonIsThereDoctor,
		&g_smidDemonOutHotTowels,
		&g_smidDemonFreezingWarm,
		&g_smidDemonILoveGoodBBQ,
		},

		{ // 3 = Wal Mart
		&g_smidDemonIsItHotOrJustMe,
		&g_smidDemonBlueLightSpecial,
		&g_smidDemonSmellBurning,
		&g_smidDemonBastardsWCoupons,
		},

		{ // 4 = Earthquake
		&g_smidDemonSeeYouInHellHa,
		&g_smidDemonILoveGoodBBQ,
		&g_smidDemonWhusy,
		&g_smidDemonShakeItUpBaby,
		}
	};

// Sound banks of suicide comments indexed by m_sSoundBank.
SampleMasterID* CDemon::ms_apsmidSuicide[NumSoundBanks][NumSuicideComments]	=
	{
		{	// 0 == Normal.
		&g_smidDemonNoRegrets,
		},

		{ // 1 = Shanty Town
		&g_smidDemonTodayGoodToDie,
		},

		{ // 2 = Resort
		&g_smidDemonShowNoMercy,
		},

		{ // 3 = Wal Mart
		&g_smidDemonDontSellPostal1,
		},

		{ // 4 = Earthquake
		&g_smidDemonDeathMyMaster,
		}
	};

// Sound banks of writhing comments indexed by m_sSoundBank.
SampleMasterID* CDemon::ms_apsmidWrithing[NumSoundBanks][NumWrithingComments]	=
	{
		{	// 0 == Normal.
		&g_smidDemonSissy1,
		&g_smidDemonSissy2,
		&g_smidDemonThatHurt1,
		&g_smidDemonThatHurt2,
		&g_smidDemonBleed,
		},

		{ // 1 = Shanty Town
		&g_smidDemonBegForThis,
		&g_smidDemonDoItQuietly,
		&g_smidDemonDieLikeDogYouAre,
		&g_smidDemonIsThereDoctor,
		&g_smidDemonWhusy,
		},

		{ // 2 = Resort
		&g_smidDemonWontAffectTip,
		&g_smidDemonYouNeedMasage,
		&g_smidDemonDizkneeland,
		&g_smidDemonHeCheckedOut,
		&g_smidDemonWatchItWiggle,
		},

		{ // 3 = Wal Mart
		&g_smidDemonDontSellPostal2,
		&g_smidDemonNoRefunds,
		&g_smidWalMartCleanupAisle17,
		&g_smidWalMartCleanupAisle6b,
		&g_smidWWalMartCleanupAisle6,
		},

		{ // 4 = Earthquake
		&g_smidDemonRemainStillInjured,
		&g_smidDemonPinnedDown,
		&g_smidDemonAwwBoBo,
		&g_smidDemonFeelWrathDog,
		&g_smidDemonDieWeakling,
		}
	};

// Sound banks of kill series comments indexed by m_sSoundBank.
SampleMasterID* CDemon::ms_apsmidKillSeries[NumSoundBanks][NumKillSeriesComments]	=
	{
		{	// 0 == Normal.
		&g_smidDemonTheMan,
		&g_smidDemonKickAss,
		&g_smidDemonOJ,
		&g_smidDemonPostal,
		&g_smidDemonBuckwheat4,
		&g_smidDemonLikeYou,
		&g_smidDemonGoPostal4,
		},

		{ // 1 = Shanty Town
		&g_smidDemonDamnImGood,
		&g_smidDemonJudgeJuryExe,
		&g_smidDemonExterminatorsBack,
		&g_smidDemonAngelOfDeath,
		&g_smidDemonPropertyValues,
		&g_smidDemonJudgeJuryExe,
		&g_smidDemonKickAss,
		},

		{ // 2 = Resort
		&g_smidDemonRichBastards,
		&g_smidDemonBuckwheat4,
		&g_smidDemonLikeYou,
		&g_smidDemonKillForMasage,
		&g_smidDemonWhoPeedInPool,
		&g_smidDemonRichBastards,
		&g_smidDemonCaseDismissed,
		},

		{ // 3 = Wal Mart
		&g_smidDemonShowNoMercy,
		&g_smidDemonBuckwheat4,
		&g_smidDemonMadeInUSABaby,
		&g_smidDemonCleanupAisle5,
		&g_smidDemonLowPriceQnty,
		&g_smidDemonSeeYouInHellHa,
		&g_smidDemonNoRefunds,
		},

		{ // 4 = Earthquake
		&g_smidDemonBuckwheat4,
		&g_smidDemonAngelOfDeath,
		&g_smidDemonShowNoMercy,
		&g_smidDemonSeeYouInHellHa,
		&g_smidDemonShakeItUpBaby,
		&g_smidDemonKillingGoodSoal,
		&g_smidDemonDamnImGood,
		}
	};

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE		"demon.bmp"

#define GUI_FILE_NAME	"res/editor/Demon.gui"

#define GUI_ID_BANK		1000


////////////////////////////////////////////////////////////////////////////////
// Preload - cache the sounds that may be used.
////////////////////////////////////////////////////////////////////////////////
short CDemon::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
#if 0
	// Tell samplemaster to cache (preload) this sample
	CacheSample(g_smidDemonBleed);
	CacheSample(g_smidDemonBlowup);
	CacheSample(g_smidDemonBurn);
	CacheSample(g_smidDemonBurnBaby);
	CacheSample(g_smidDemonButtSauce);
	CacheSample(g_smidDemonSmellChicken);
	CacheSample(g_smidDemonDie1);
	CacheSample(g_smidDemonDie2);
	CacheSample(g_smidDemonSissy1);
	CacheSample(g_smidDemonSissy2);
	CacheSample(g_smidDemonEvil);
	CacheSample(g_smidDemonFeelHeat);
	CacheSample(g_smidDemonGetEm1);
	CacheSample(g_smidDemonGetEm2);
	CacheSample(g_smidDemonGone1);
	CacheSample(g_smidDemonGone2);
	CacheSample(g_smidDemonGoodOne);
	CacheSample(g_smidDemonGoPostal4);
	CacheSample(g_smidDemonHa);
	CacheSample(g_smidDemonHesOut);
	CacheSample(g_smidDemonInHell);
	CacheSample(g_smidDemonLikeItHot);
	CacheSample(g_smidDemonKickAss);
	CacheSample(g_smidDemonLaugh1);
	CacheSample(g_smidDemonLaugh2);
	CacheSample(g_smidDemonLikeYou);
	CacheSample(g_smidDemonNoRegrets);
	CacheSample(g_smidDemonOhBaby);
	CacheSample(g_smidDemonOJ);
	CacheSample(g_smidDemonOnlyWeapons);
	CacheSample(g_smidDemonPostal);
	CacheSample(g_smidDemonSlam);
	CacheSample(g_smidDemonThatHurt1);
	CacheSample(g_smidDemonThatHurt2);
	CacheSample(g_smidDemonTheGipper);
	CacheSample(g_smidDemonTheGun);
	CacheSample(g_smidDemonWeapon);
	CacheSample(g_smidDemonYes1);
	CacheSample(g_smidDemonYes2);
	CacheSample(g_smidDemonBuckwheat4);
	CacheSample(g_smidDemonTheMan);

#endif
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDemon::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
	{
		switch (ulFileVersion)
		{
			default:
			case 47:
				pFile->Read(&m_sSoundBank);
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				break;

			case 46:
			case 45:
			case 44:
			case 43:
			case 42:
			case 41:
			case 40:
			case 39:
			case 38:
			case 37:
			case 36:
			case 35:
			case 34:
			case 33:
			case 32:
			case 31:
			case 30:
			case 29:
			case 28:
			case 27:
			case 26:
			case 25:
			case 24:
			case 23:
			case 22:
			case 21:
			case 20:
			case 19:
			case 18:
			case 17:
			case 16:
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
			case 7:
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				{
				// For backwards compatability.
				long	alDummy[2];
				char	szResNameDummy[RSP_MAX_PATH];
				pFile->Read(&alDummy[0]/*(long*)&m_bInitiallyEnabled*/);
				pFile->Read(&alDummy[0]/*(long*)&m_bInitiallyRepeats*/);
				pFile->Read(alDummy/*m_lMinTime*/, 2);
				pFile->Read(alDummy/*m_lRndTime*/, 2);
				pFile->Read(szResNameDummy/*m_szResName*/);
				break;
				}
		}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
		{
			sResult = Init();
		}
		else
		{
			sResult = -1;
			TRACE("CDemon::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDemon::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
	{
		pFile->Write(m_sSoundBank);
		pFile->Write(&m_dX);
		pFile->Write(&m_dY);
		pFile->Write(&m_dZ);

		// Make sure there were no file errors
		sResult	= pFile->Error();
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CDemon::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CDemon::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CDemon::Suspend(void)
{
	m_sSuspend++;
}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CDemon::Resume(void)
{
	m_sSuspend--;

	// If we're actually going to start updating again....
	if (m_sSuspend == 0)
	{
	}
}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CDemon::Update(void)
{
	if (!m_sSuspend)
	{

		// Process messages.
		ProcessMessages();

		if (m_state == State_Delete)
		{
			delete this;
			return;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CDemon::Render(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CDemon::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	sResult = Init();

//	sResult	= EditModify();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CDemon::EditModify(void)
{
	short	sResult	= 0;

	// Load gui dialog
	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPath(GAME_PATH_HD, GUI_FILE_NAME));
	if (pgui != NULL)
		{
		// Init "bank" field.
		RGuiItem* pguiBankName = pgui->GetItemFromId(GUI_ID_BANK);
		ASSERT(pguiBankName != NULL);
		pguiBankName->SetText("%hd", m_sSoundBank);
		pguiBankName->Compose();

		// Run the dialog using this super-duper helper funciton
		if (DoGui(pgui) == 1)
			{
			// Get new values from dialog.
			m_sSoundBank	= (short)pguiBankName->GetVal();
			// Keep it in range.
			if (m_sSoundBank < 0)
				{
				m_sSoundBank	= 0;
				}
			else if (m_sSoundBank >= NumSoundBanks)
				{
				m_sSoundBank	= NumSoundBanks - 1;
				}
				
			}
		else
			{
			sResult	= 1;
			}
		
		// Done with GUI.
		delete pgui;
		}
	else
		{
		sResult	= -1;
		}

#if 0	// No settins via dialog currently require re-Init()age.
	// If everything's okay, init using new values
	if (sResult == 0)
		sResult = Init();
#endif

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CDemon::EditMove(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the clickable pos/area of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CDemon::EditRect(	// Returns nothiing.
	RRect*	prc)				// Out: Clickable pos/area of object.
{
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(prc->sX),
		&(prc->sY) );

	prc->sW	= 10;	// Safety.
	prc->sH	= 10;	// Safety.

	if (m_pImage)
	{
		prc->sW	= m_pImage->m_sWidth;
		prc->sH	= m_pImage->m_sHeight;
	}

	prc->sX	-= prc->sW / 2;
	prc->sY	-= prc->sH;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CDemon::EditHotSpot(	// Returns nothiing.
	short*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	short*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
{
	*psX	= 5;	// Safety.
	*psY	= 5;	// Safety.

	if (m_pImage)
	{
		*psX	= m_pImage->m_sWidth / 2;
		*psY	= m_pImage->m_sHeight;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CDemon::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CDemon::EditRender(void)
{
	// Setup simple, non-animating sprite
	m_sprite.m_sInFlags = 0;

	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(m_sprite.m_sX2),
		&(m_sprite.m_sY2) );

	// Priority is based on bottom edge of sprite
	m_sprite.m_sPriority = m_dZ;

	// Center on image.
	m_sprite.m_sX2	-= m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_pImage->m_sHeight;

	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	m_sprite.m_pImage = m_pImage;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
}


////////////////////////////////////////////////////////////////////////////////
// Init object
////////////////////////////////////////////////////////////////////////////////
short CDemon::Init(void)							// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	Kill();

	if (m_pImage == 0)
	{
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(IMAGE_FILE), &m_pImage);
		if (sResult == 0)
		{
			// This is a questionable action on a resource managed item, but it's
			// okay if EVERYONE wants it to be an FSPR8.
			if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
			{
				sResult = -1;
				TRACE("CDemon::GetResource() - Couldn't convert to FSPR8\n");
			}
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Kill object
////////////////////////////////////////////////////////////////////////////////
short CDemon::Kill(void)							// Returns 0 if successfull, non-zero otherwise
{
	if (m_pImage != 0)
		rspReleaseResource(&g_resmgrGame, &m_pImage);

	m_pRealm->m_scene.RemoveSprite(&m_sprite);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Process our message queue.
////////////////////////////////////////////////////////////////////////////////
void CDemon::ProcessMessages(void)
{
	SampleMasterID* psmid = &g_smidNil;
	bool bFoundSample = false;
	long lThisTime = m_pRealm->m_time.GetGameTime();

	// Check queue of messages.
	GameMessage	msg;
	while (m_MessageQueue.DeQ(&msg) == true)
	{
		if (msg.msg_Generic.eType == typeCheater)
		{
			psmid = &g_smidDemonSissy2;
			if (lThisTime > m_lIdleTime + ms_lMinIdleTime)
			{
				PlaySample(										// Returns nothing.
																	// Does not fail.
					*psmid,										// In:  Identifier of sample you want played.
					SampleMaster::Demon);					// In:  Sound Volume Category for user adjustment
				m_lIdleTime = lThisTime;
			}
			bFoundSample = true;
		}

		// If we haven't selected a sound yet...
		if (!bFoundSample && lThisTime > m_lIdleTime + ms_lMinIdleTime)
		{
			switch(msg.msg_Generic.eType)
			{
				// Delete Demon
				case typeObjectDelete:
					m_state	= State_Delete;
					break;

				// Weapon Selector
				case typeWeaponSelect:
					switch (msg.msg_WeaponSelect.sWeapon)
					{
						case CDude::SemiAutomatic:	// Machine Gun
							if (GetRand() % 2)
							{
								if (m_sSoundBank == DEMON_RESORT_LEVEL)
									psmid = &g_smidDemonNudityOffensive;
								else
									psmid = &g_smidDemonTheGun;
							}
							else
							{
								if (m_sSoundBank == DEMON_RESORT_LEVEL)
									psmid = &g_smidDemonWhoPeedInPool;
								else
									psmid = &g_smidDemonWeapon;
							}
							break;

						case CDude::Grenade:	// Grenade
							psmid = &g_smidDemonShakeItUpBaby;
							break;

						case CDude::FireBomb:	// Cocktail
							if (GetRand() % 2)
								psmid = &g_smidDemonLikeItHot;
							else
								psmid = &g_smidDemonAllMustDie;
							break;

						case CDude::Rocket:	// Rocket
							psmid = &g_smidDemonBlowup;
							break;

						case CDude::Napalm:	// Napalm
							switch (GetRand() % 4)
							{
								case 0:
									psmid = &g_smidDemonExterminatorsBack;
									break;
								case 1:
									psmid = &g_smidDemonLikeItHot;
									break;
								case 2:
									psmid = &g_smidDemonOhBaby;
									break;
								case 3:
									psmid = &g_smidDemonYes2;
									break;
							}
							break;

						case CDude::ShotGun:	// Shot Gun
							switch (GetRand() % 4)
							{
								case 0:
									psmid = &g_smidDemonYes2;
									break;
								case 1:
									psmid = &g_smidDemonMadeInUSABaby;
									break;
								case 2:
									psmid = &g_smidDemonMadeInUSABaby;
									break;
								case 3:
									psmid = &g_smidDemonLaugh1;
									break;
							}
							break;

						case CDude::TimedMine:	// Mines
						case CDude::ProximityMine:
						case CDude::RemoteMine:
						case CDude::BouncingBettyMine:
							if (GetRand() % 2)
								psmid = &g_smidDemonTheGipper;
							else
								psmid = &g_smidDemonEvil;
							break;

						case CDude::Heatseeker:	// Heatseeker
							switch (GetRand() % 4)
							{
								case 0:
									psmid = &g_smidDemonSatisfactGnty;
									break;
								case 1:
									psmid = &g_smidDemonMadeInUSABaby;
									break;
								case 2:
									if (m_sSoundBank == DEMON_RESORT_LEVEL)
										psmid = &g_smidDemonTennisBalls;
									else
										psmid = &g_smidDemonTheGipper;
									break;
								case 3:
									psmid = &g_smidDemonLaugh2;
									break;
							}
							break;

						case CDude::SprayCannon:	// Spray Cannon
							psmid = &g_smidDemonLaugh1;
							break;

						default:
							break;
					}
					bFoundSample = true;
					break;

				case typeWeaponFire:
					switch(msg.msg_WeaponFire.sWeapon)
					{
						case CDude::SemiAutomatic:	// Machine Gun
						case CDude::ShotGun:	// Shot Gun
							switch (GetRand() % 3)
							{	
								case 0:
									psmid = &g_smidDemonEatLeadSucker;
									break;
								case 1:
									if (m_sSoundBank == DEMON_SHANTY_LEVEL)
										psmid = &g_smidDemonBegForThis;
									else if (m_sSoundBank == DEMON_WALMART_LEVEL)
										psmid = &g_smidDemonBastardsWCoupons;
									else
										psmid = &g_smidDemonDie2;
									break;
								case 2:
									if (m_sSoundBank == DEMON_SHANTY_LEVEL)
										psmid = &g_smidDemonWelfareReform;
									else if (m_sSoundBank == DEMON_WALMART_LEVEL)
										psmid = &g_smidDemonLikeFreeSample;
									else
										psmid = &g_smidDemonBleed;
									break;
							}
							bFoundSample = true;
							break;

						case CDude::Grenade:	// Grenade
							psmid = &g_smidDemonGrenadeWorksGreat;
							break;

						case CDude::FireBomb:	// Cocktail
						case CDude::Napalm:	// Napalm
							switch (GetRand() % 3)
							{
								case 0:
									if (m_sSoundBank == DEMON_WALMART_LEVEL)
										psmid = &g_smidDemonWhatLaneClosed;
									else
										psmid = &g_smidDemonFeelWrathDog;
									break;
								case 1:
									if (m_sSoundBank == DEMON_WALMART_LEVEL)
										psmid = &g_smidDemonCustomerRight;
									else
										psmid = &g_smidDemonBurnBaby;
									break;
								case 2:
									psmid = &g_smidDemonFreezingWarm;
									break;
							}
							bFoundSample = true;
							break;

						case CDude::Rocket:	// Rocket
							switch (m_sSoundBank)
							{
								case DEMON_SHANTY_LEVEL:
									psmid = &g_smidDemonBegForThis;
									break;

								case DEMON_WALMART_LEVEL:
									if (GetRand() % 2)
										psmid = &g_smidDemonTenItemsOrLess;
									else
										psmid = &g_smidDemonWhatLaneClosed;
									break;
							}
							break;

						case CDude::Heatseeker:	// Heatseeker
							switch (GetRand() % 4)
							{
								case 0:
									if (m_sSoundBank == DEMON_WALMART_LEVEL)
										psmid = &g_smidDemonWhatLaneClosed;
									else
										psmid = &g_smidDemonGone1;
									break;
								case 1:
									if (m_sSoundBank == DEMON_RESORT_LEVEL)
										psmid = &g_smidDemonHoleInOne;
									else
										psmid = &g_smidDemonGone2;
									break;
								case 2:
									if (m_sSoundBank == DEMON_RESORT_LEVEL)
										psmid = &g_smidDemonTennisBalls;
									else
										psmid = &g_smidDemonGetEm1;
									break;
								case 3:
									psmid = &g_smidDemonGetEm2;
									break;
							}
							bFoundSample = true;
							break;

						case CDude::SprayCannon:	// Spray Cannon
							psmid = &g_smidDemonShowNoMercy;
							break;

						default:
							break;
					}
					break;

				case typeExplosion:
#if 1
					ASSERT(m_sSoundBank < NumSoundBanks);

					psmid	= ms_apsmidExplosion[m_sSoundBank][GetRand() % NumExplosionComments];
#else
					switch (GetRand() % 8)
					{
						case 0:
							psmid = &g_smidDemonSlam;
							break;
						case 1:
							psmid = &g_smidDemonYes1;
							break;
						case 2:
							psmid = &g_smidDemonGoodOne;
							break;
						case 3:
							psmid = &g_smidDemonHa;
							break;
						case 4:
							psmid = &g_smidDemonButtSauce;
							break;
						case 5:
							psmid = &g_smidDemonHesOut;
							break;
						case 6:
							psmid = &g_smidDemonInHell;
							break;
						case 7:
							psmid = &g_smidDemonBuckwheat4;
							break;
					}
#endif
					bFoundSample = true;
					break;

				case typeBurn:
#if 1
					ASSERT(m_sSoundBank < NumSoundBanks);

					psmid	= ms_apsmidBurn[m_sSoundBank][GetRand() % NumBurnComments];
#else
					switch (GetRand() % 4)
					{
						case 0:
							psmid = &g_smidDemonSmellChicken;
							break;
						case 1:
							psmid = &g_smidDemonFeelHeat;
							break;
						case 2:
							psmid = &g_smidDemonBurn;
							break;
						case 3:
							psmid = &g_smidDemonBurnBaby;
							break;
					}
#endif
					bFoundSample = true;
					break;

				case typeSuicide:
#if 1
					ASSERT(m_sSoundBank < NumSoundBanks);

					psmid	= ms_apsmidSuicide[m_sSoundBank][GetRand() % NumSuicideComments];
#else
					psmid = &g_smidDemonNoRegrets;
#endif
					if (lThisTime > m_lIdleTime + ms_lMinIdleTime)
					{
						PlaySample(										// Returns nothing.
																			// Does not fail.
							*psmid,										// In:  Identifier of sample you want played.
							SampleMaster::Demon);					// In:  Sound Volume Category for user adjustment

						m_lIdleTime = lThisTime;
					}
					break;

				case typeWrithing:
#if 1
					ASSERT(m_sSoundBank < NumSoundBanks);

					psmid	= ms_apsmidWrithing[m_sSoundBank][GetRand() % NumWrithingComments];
#else
					switch (GetRand() % 5)
					{
						case 0:
							psmid = &g_smidDemonSissy1;
							break;
						case 1:
							psmid = &g_smidDemonSissy2;
							break;
						case 2:
							psmid = &g_smidDemonThatHurt1;
							break;
						case 3:
							psmid = &g_smidDemonThatHurt2;
							break;
						case 4:
							psmid = &g_smidDemonBleed;
							break;
					}
#endif
					bFoundSample = true;
					break;

				case typeDeath:
					m_sRecentKills++;
					break;
			}	
		}
	}		

	if (lThisTime > m_lIdleTime + ms_lMinIdleTime)
	{
		// Check to see if a series of kills have been made and use this as the
		// higher priority saying.
		if (m_sRecentKills > 3 && lThisTime < m_lKillTimer)
		{
#if 1
					ASSERT(m_sSoundBank < NumSoundBanks);

					psmid	= ms_apsmidKillSeries[m_sSoundBank][GetRand() % NumKillSeriesComments];
#else
			switch (GetRand() % 7)
			{
				case 0:
					psmid = &g_smidDemonTheMan;
					break;
				case 1:
					psmid = &g_smidDemonKickAss;
					break;
				case 2:
					psmid = &g_smidDemonOJ;
					break;
				case 3:
					psmid = &g_smidDemonPostal;
					break;
				case 4:
					psmid = &g_smidDemonBuckwheat4;
					break;
				case 5:
					psmid = &g_smidDemonLikeYou;
					break;
				case 6:
					psmid = &g_smidDemonGoPostal4;
					break;
			}
#endif

			if (lThisTime > m_lIdleTime + ms_lMinIdleTime)
			{
				PlaySample(										// Returns nothing.
																	// Does not fail.
					*psmid,										// In:  Identifier of sample you want played.
					SampleMaster::Demon);					// In:  Sound Volume Category for user adjustment
				m_lIdleTime = lThisTime;
				m_sCommentCount++;
			}
		}
		else
		{
			// See if we got a new sound
			if (bFoundSample)
			{
				m_sCommentCount++;
				if (m_sCommentCount > 4 && psmid)
				{
					PlaySample(										// Returns nothing.
																		// Does not fail.
						*psmid,										// In:  Identifier of sample you want played.
						SampleMaster::Demon);					// In:  Sound Volume Category for user adjustment

					m_lIdleTime = lThisTime;
					m_sCommentCount = 0;
				}
			}
		}
	}

	// Adjust Kill timer
	if (lThisTime > m_lKillTimer)
	{
		m_sRecentKills = 0;
		m_lKillTimer = lThisTime + ms_lBonusKillTime;
	}
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
