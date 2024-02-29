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
// PowerUp.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		05/08/97 JMI	Started.
//
//		05/09/97	JMI	Added m_smash and Init().
//
//		05/14/97	JMI	Added Grab() and Drop().
//							Also, Update() will not update collision sphere unless
//							there's no parent.
//							Also, increased COLLISION_RADIUS from 5 to 10.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/06/97	JMI	Now hotspot is bottom center of image (was center of 
//							image).
//							Increased COLLISION_RADIUS to 20 (was 10).
//							Got rid of the whole type thing.  Now uses a CStockPile
//							and can contain any combination of powerups w/i one    
//							instance.  Types are still used for loading old powerup
//							files.
//							Now EditModify() simply invokes m_stockpile.UserEdit().
//
//		06/09/97	JMI	Changed RES_NAME from "2d/box.bmp" to "2d/box.img".
//
//		06/10/97	JMI	Changed "First Aid Kit" to "Health".
//
//		06/12/97	JMI	Added support for new weapon members of CStockPile.
//
//		06/12/97	JMI	Added sHitPointMax parameter to GetDescription().
//
//		06/12/97	JMI	Drop was not checking that we had a parent before trying
//							to call RemoveChild() in the parent.
//
//		06/13/97	JMI	Moved priority spot to center of image vertically.
//
//		06/14/97	JMI	Decreased COLLISION_RADIUS to 10 (was 20).
//
//		06/14/97	JMI	Changed to a descendant of CItem3d instead of CThing.
//							Removed 2D bullshit.
//
//		06/14/97	JMI	Changed SCALE from 3.0 to 2.0.  Note that once Randy 
//							makes larger powerup specific animations, this should be
//							changed to 1.0.
//							Seems that the BoundingSphereToScreen() function does not
//							take scaling into account.
//
//		06/14/97	JMI	Added KevlarVest (CStockPile::m_sArmorLayers).
//
//		06/15/97	JMI	Added Backpack (CStockPile::m_sBackpack).
//							Now returns to normal spinning after blown up state is 
//							complete.
//
//		06/15/97	JMI	Reduced SCALE to 1.0 (was 2.0) and added some of the
//							real powerup filenames.
//
//		06/16/97	JMI	Added more real powerup names.
//
//					JMI	Added a user (audible) feedback function, PickUpFeedback().
//
//		06/17/97	JMI	Changed the 'no items' and 'multiple items' anim to
//							the mine crate.
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/30/97 BRH	Caches the sample it uses in the Load function so that
//							the sound effect will be ready in any level that the
//							powerups are in.
//
//		07/15/97	JMI	Made GetDescription() and TypeToStockPile() static and 
//							added stockpiles as parms so they can be used more 
//							generically.
//							Also, added RepaginateNow().
//							Also, added IsEmpty().
//
//		07/15/97	JMI	Added some message handling functions.
//							Transferred powerup index enums into CStockPile.
//							Now boxes of multiple powerups explode into many when
//							blown up.
//
//		07/16/97	JMI	Now Setup() automatically calls Startup().
//							Moved IsEmpty() from powerup to stockpile.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/23/97	JMI	Added separate launcher for napalm and icons for napalm
//							launcher, flame thrower, and fuel.
//
//		07/28/97	JMI	Changed "nplmlauncher" to "napalmer" for res name.
//
//		07/30/97	JMI	Added DeathWadLauncher to array of powerup names and
//							res names.
//
//		08/07/97	JMI	Added DoubleBarrel to array of powerup names and res 
//							names.
//
//		08/07/97	JMI	Added additional parameter to CAnim3D::Get() call.
//
//		08/07/97	JMI	Changed backpack resname from "3d/backpack" to
//							"3d/backpackicon".
//
//		08/17/97	JMI	Got rid of m_szMessages and all message related functions
//							and variables from CDude since we are now using the toolbar 
//							for dude status feedback to the user.  This includes:  
//							MsgTypeInfo, m_lNextStatusUpdateTime, m_lMsgUpdateDoneTime, 
//							m_print, m_bClearedStatus, m_szMessages[], m_sDeadMsgNum, 
//							ms_amtfMessages[], ms_u8FontForeIndex, ms_u8FontBackIndex,
//							ms_u8FontShadowIndex, DrawStatus(), StatusChange(), 
//							MessageChange(), Message(), UpdateFontColors(), 
//							CPowerUp::ms_apszPowerUpTypeNames[], 
//							CPowerUp::GetDescription(), and some strings and a string
//							array in localize.*.
//
//		08/28/97 BRH	Added a Preload function for the powerups since they
//							may pop up during gameplay, we want to have their assets
//							ready.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CItem3d-derived class will represent power ups that the player can
// pick up.
//
//////////////////////////////////////////////////////////////////////////////
#define POWERUP_CPP

#include "RSPiX.h"
#include <math.h>

#include "PowerUp.h"
#include "dude.h"
#include "game.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define GUI_FILE_NAME		"res/editor/PowerUp.gui"

#define GUI_ID_STOCKPILE	3

#define Y_AXIS_ROTATION_RATE	240.0

#define SCALE						1.0

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Animations.
char*	CPowerUp::ms_apszPowerUpResNames[CStockPile::NumStockPileItems + 2]	=
	{
	"3d/ammo",							// Bullets.
	"3d/grenadeicon",					// Grenades.
	"3d/cocktail",						// Cocktails.
	"3d/missileicon",					// Rockets.
	"3d/napalmicon",					// Napalm.
	"3d/shotshells",					// Shells.
	"3d/fuel",							// Fuel.
	"3d/minecrate",					// Mines.
	"3d/health",						// Health.
	"3d/gmissileicon",				// Heatseekers.

	"3d/machinegun",					//	MachineGun.
	"3d/launcher",						// MissileLauncher.
	"3d/shotgun",						// ShotGun.
	"3d/spraygun",						// SprayCannon.
	"3d/flmthrower",					// FlameThrower.
	"3d/napalmer",						// NapalmLauncher.
	"3d/napalmer",						// DeathWadLauncher.
	"3d/shotgun",						// DoubleBarrel.

	"3d/kevlarvest",					// KevlarVest.

	"3d/backpackicon",				// Backpack.

	// Insert new items above this item.
	"3d/minecrate",					// Multiple items.
	"3d/minecrate",					// No items.
	};

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,										// In:  File to load from
	bool bEditMode,									// In:  True for edit mode, false otherwise
	short sFileCount,									// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)								// In:  Version of file format to load.
	{
	short sResult = 0;
	if (ulFileVersion < 20)
		{
		sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
		}
	else
		{
		// Note that we bypass CItem3d::Load() cuz I think that would be wierd.
		sResult	= CThing3d::Load(pFile, bEditMode, sFileCount, ulFileVersion);
		}

	if (sResult == 0)
		{
		CacheSample(g_smidPickedUpWeapon);
		switch (ulFileVersion)
			{
			default:
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
				// Base class gets it all.
				break;
			case 19:
			case 18:
			case 17:
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				sResult	= m_stockpile.Load(pFile, ulFileVersion);
				break;
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
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				U8	u8Type	= (U8)CStockPile::Bullets; 
				pFile->Read(&u8Type);
				long	lPowerVal;
				pFile->Read(&lPowerVal);
				switch (u8Type)
					{
					case 0://Bullets:
						m_stockpile.m_sNumBullets	= lPowerVal;
						break;
					case 1://Grenades:
						m_stockpile.m_sNumGrenades	= lPowerVal;
						break;
					case 2://Cocktails:
						m_stockpile.m_sNumFireBombs	= lPowerVal;
						break;
					case 3://Rockets:
						m_stockpile.m_sNumMissiles	= lPowerVal;
						break;
					case 4://Napalm:
						m_stockpile.m_sNumNapalms	= lPowerVal;
						break;
					case 5://Shells:
						m_stockpile.m_sNumShells	= lPowerVal;
						break;
					case 6://Fuel:
						m_stockpile.m_sNumFuel	= lPowerVal;
						break;
					case 7://ProximityMine:
					case 8://TimedMine:
					case 9://RemoteMine:
					case 10://BouncingBettyMine:
						m_stockpile.m_sNumMines	= lPowerVal;
						break;
					case 11://Health:
						m_stockpile.m_sHitPoints	= lPowerVal;
						break;
					case 12://Heatseekers:
						m_stockpile.m_sNumHeatseekers	= lPowerVal;
						break;
					}
				break;
				}
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Get resources and initialize.
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CPowerUp::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	// Note that we bypass CItem3d::Save() cuz I think that would be wierd.
	short	sResult	= CThing3d::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Base class does it all.
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::Update(void)
	{
	if (!m_sSuspend)
		{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime();

		// Advance the animation timer.
		long	lDifTime		= lThisTime - m_lAnimPrevUpdateTime;
		m_lAnimTime			+= lDifTime;

		// Update prev time.
		m_lAnimPrevUpdateTime	= lThisTime;

		// Service message queue.
		ProcessMessages();

		// Switch on state.
		switch (m_state)
			{
			case State_BlownUp:
				// Handle state.  If done . . .
				if (WhileBlownUp() == false)
					{
					m_state	= State_Idle;
					// Restore powerup cheese spin.
					m_dExtRotVelY	= Y_AXIS_ROTATION_RATE;
					m_dExtRotVelZ	= 0.0;
					m_dRotZ			= 0.0;
					}
				break;
			default:
				if (m_u16IdParent == CIdBank::IdNil)
					{
					// Get time from last call in seconds.
					double	dSeconds	= double(lThisTime - m_lPrevTime) / 1000.0;
			
					DeluxeUpdatePosVel(dSeconds);
					}

				break;
			}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y			= m_dY;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sprite.m_sRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Save time for next time
		m_lPrevTime = lThisTime;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::Render(void)
	{
	CItem3d::Render();

	// Make larger.
	m_trans.Scale(SCALE, SCALE, SCALE);
	}

////////////////////////////////////////////////////////////////////////////////
// Setup object.
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	// Load resources and initialize.
	sResult = Init();

	if (sResult == 0)
		{
		sResult	= Startup();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::EditNew(								// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	sResult	= EditModify();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::EditModify(void)
	{
	short	sResult	= m_stockpile.UserEdit();

	// If successful so far . . .
	if (sResult == 0)
		{
		// Load resources and initialize.
		sResult = Init();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Initialize object.
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::Init(void)	// Returns 0 on success.
	{
	short	sRes	= GetResources();

	// Prepare shadow (get resources and setup sprite).
	sRes	|= PrepareShadow();

	m_dExtRotVelY	= Y_AXIS_ROTATION_RATE;

	m_lAnimTime			= 0;

	// Set up collision object
	m_smash.m_bits		= CSmash::PowerUp | CSmash::Misc;

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Get resource name for this item.
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::GetResName(	// Returns nothing.
	char*	pszResName)			// Out: Resource base name.
	{
	short	sTypeIndex;
	short	sTypeInPowerUp	= CStockPile::NumStockPileItems;
	for (sTypeIndex = 0; sTypeIndex < CStockPile::NumStockPileItems; sTypeIndex++)
		{
		if (m_stockpile.GetItem(sTypeIndex) > 0)
			{
			// If no type yet . . .
			if (sTypeInPowerUp == CStockPile::NumStockPileItems)
				{
				sTypeInPowerUp	= sTypeIndex;
				}
			else
				{
				sTypeInPowerUp	= CStockPile::NumStockPileItems + 1;
				break;
				}
			}
		}

	strcpy(pszResName, ms_apszPowerUpResNames[sTypeInPowerUp]);
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Safe to call even if no resource.
	FreeResources();

	char	szResName[RSP_MAX_PATH];
	GetResName(szResName);

	sResult	= m_anim.Get(
		szResName, 
		NULL,
		NULL,
		NULL,
		RChannel_LoopAtStart | RChannel_LoopAtEnd);

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	if (m_anim.m_psops != NULL)
		{
		m_anim.Release();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Preload - Preload assets needed so they can be cached and don't have to load
//			    during the game
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::Preload(CRealm* /*prealm*/)
{
	short sResult = 0;

	short i;
	CAnim3D anim;

	for (i = 0; i < CStockPile::NumStockPileItems + 2; i++)
	{
		sResult |= anim.Get(ms_apszPowerUpResNames[i], NULL, NULL, NULL, 0);
		anim.Release();			
	}	

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Call to grab this item.
////////////////////////////////////////////////////////////////////////////////
short CPowerUp::Grab(		// Returns 0 on success..
	CSprite* psprParent)		// In:  Parent's sprite.
	{
	short	sRes	= 0;	// Assume success.

	// If we are not already grabbed . . .
	if (m_sprite.m_psprParent == NULL)
		{
		m_dX	= 0.0;
		m_dY	= 0.0;
		m_dZ	= 0.0;

		psprParent->AddChild(&m_sprite);

		// Don't use this for now b/c the next line that removes the
		// smash from the smashatorium will be ineffective b/c the
		// base class will put it right back.
		ASSERT(0);

		// Remove collision thinger.
		m_pRealm->m_smashatorium.Remove(&m_smash);
		}
	else
		{
		// Already have a parent.
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Call to release this item.
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::Drop(			// Returns nothing.
	short sX,					// In:  Position from which to release.
	short sY,					// In:  Position from which to release.
	short sZ)					// In:  Position from which to release.
	{
	m_dX	= sX;
	m_dY	= sY;
	m_dZ	= sZ;

	// If we have a parent . . .
	if (m_sprite.m_psprParent != NULL)
		{
		// Remove from parent.
		m_sprite.m_psprParent->RemoveChild(&m_sprite);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Plays a sample corresponding to the type of powerup indicating it
// was picked up.
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::PickUpFeedback(void)	// Returns nothing.
	{
	PlaySample(g_smidPickedUpWeapon, SampleMaster::UserFeedBack);
	}

////////////////////////////////////////////////////////////////////////////////
// If this powerup has nothing left, destroys itself.  Otherwise, chooses
// a new resource to represent its current contents.
// NOTE:  This function can cause this item to be destroyed.  Don't use
// a ptr to this object after calling this function.
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::RepaginateNow(void)
	{
	// If not empty . . .
	if (IsEmpty() == false)
		{
		// If this fails, we'd better go away . . .
		if (GetResources() != 0)
			{
			delete this;
			}
		}
	else
		{
		// Done then.
		delete this;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Message Handlers.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Handles a Shot_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::OnShotMsg(		// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
	{
	// Eventually, it'd be cool to have the ammos respond to bullets.

	// Invoke base class implementation.
	CItem3d::OnShotMsg(pshotmsg);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::OnExplosionMsg(			// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
	{
	// Break into smaller pieces and pass the message on . . .
	// Inventory.
	short	sTypeIndex;
	bool	bFirst	= true;
	short	sNumGenerated	= 0;
	for (sTypeIndex = 0; sTypeIndex < CStockPile::NumStockPileItems; sTypeIndex++)
		{
		if (m_stockpile.GetItem(sTypeIndex) > 0)
			{
			if (bFirst == false)
				{
				CPowerUp*	ppowerup;
				if (ConstructWithID(CPowerUpID, m_pRealm, (CThing**)&ppowerup) == 0)
					{
					// Transfer item.
					ppowerup->m_stockpile.GetItem(sTypeIndex)	= m_stockpile.GetItem(sTypeIndex);
					// Clear local item.
					m_stockpile.GetItem(sTypeIndex)				= 0;
					// Prepare for the cold, cruel, fire-infested realm.
					ppowerup->Setup(m_dX, m_dY, m_dZ);
					// Slap it some (bypass this function, though..that'd be a waste to 
					// do again).
					ppowerup->CItem3d::OnExplosionMsg(pexplosionmsg);
					// Note that another was created.
					sNumGenerated++;
					// Choose random trajectory.
					ppowerup->m_dExtHorzRot	= GetRandom() % 360;
					}
				else
					{
					TRACE("OnExplosionMsg(): Failed to create powerup.\n");
					}
				}
			else
				{
				bFirst	= false;
				}
			}
		}

	// If we created any new powerups . . .
	if (sNumGenerated)
		{
		// This one was altered.
		if (GetResources() != 0)
			{
			// Doh!
			delete this;
			return;
			}
		}

	// Invoke base class implementation.
	CItem3d::OnExplosionMsg(pexplosionmsg);
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CPowerUp::OnBurnMsg(		// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
	{
	// Eventually, it'd be cool to have the ammos respond to fire.

	// Invoke base class implementation.
	CItem3d::OnBurnMsg(pburnmsg);
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
