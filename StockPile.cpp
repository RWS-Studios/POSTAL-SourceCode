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
// Stockpile.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		06/03/97 JMI	Started.
//
//		06/06/97	JMI	Added Add(), Union(), Intersect(), and Copy() functions.
//
//		06/11/97	JMI	Added Zero().
//
//		06/12/97	JMI	Added weapon values.
//
//		06/14/97	JMI	Added m_sKevlarLayers.
//
//		06/15/97	JMI	Added m_sBackpack and maximums.
//
//		06/15/97	JMI	Added m_sBackpack, maximums, and Truncate().
//
//		07/10/97	JMI	Increased maximums on fuel.
//
//		07/14/97	JMI	Can now edit fuel via dialog.
//
//		07/15/97	JMI	Added Sub().
//
//		07/15/97	JMI	Transferred powerup index enums into CStockPile.
//							Also, added GetItem().  See proto for details.
//
//		07/16/97	JMI	Moved IsEmpty() from powerup to stockpile.
//
//		07/19/97	JMI	Added optional child GUI param to UserEdit().
//
//		07/23/97	JMI	Added separate launcher for napalm.
//							Also, made all functions like Add(), Sub(), etc. use
//							iterative technique for accessing members so I don't
//							have to keep updating every one to have new members.
//
//		07/30/97	JMI	Added DeathWadLauncher.
//							Made pstockpile inputs to functions const.  This caused
//							some silly stupidity in GetItem() calls but I just casted
//							it away b/c the stockpile was not being modified.
//
//		08/02/97	JMI	Upgraded kevlar maximums from 50 to 32767.
//
//		08/07/97	JMI	Added DoubleBarrel.
//
//		08/13/97	JMI	Decreased maxes on m_sKevlarLayers to 100 (was 32767).
//
//		08/21/97	JMI	Set maximums for hitpoints to 1000.
//
//		08/24/97	JMI	Decreased the max carry with backpack on grenades,
//							firebombs, missiles, heatseekers, napalms, and mines.
//
//		12/08/97	JMI	Added GetWeapon() that takes a CDude::WeaponType enum
//							to index into the stockpile.
//
//////////////////////////////////////////////////////////////////////////////
//
// This represents a character's stockpile of weapons and health.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////////////
// Postal Headers.
//////////////////////////////////////////////////////////////////////////////

#include "StockPile.h"
#include "game.h"
#include "thing.h"
#include "dude.h"	// For GetWeapon().

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

#define GUI_FILE_NAME				"res/editor/EditPile.gui"

// IDs for GUIs.
#define HITPOINTS_GUI_ID			3
											
#define GRENADES_GUI_ID				4
#define FIREBOMB_GUI_ID				5
#define MISSILE_GUI_ID				6
#define NAPALM_GUI_ID				7
#define BULLETS_GUI_ID				8
#define SHELLS_GUI_ID				9
#define MINES_GUI_ID					10
#define HEATSEEKERS_GUI_ID			11
#define FUEL_GUI_ID					12

#define MACHINEGUN_GUI_ID			20
#define MISSILELAUNCHER_GUI_ID	21
#define SHOTGUN_GUI_ID				22
#define SPRAYCANNON_GUI_ID			23
#define FLAMETHROWER_GUI_ID		24
#define NAPALMLAUNCHER_GUI_ID		25
#define DEATHWADLAUNCHER_GUI_ID	26
#define DOUBLEBARREL_GUI_ID		27

#define ARMOR_GUI_ID					30

#define BACKPACK_GUI_ID				40

#define DEATHWADLAUNCHER_TXT_ID	100
#define DOUBLEBARREL_TXT_ID		101

// Space above, below, to the left, and right of optional
// user's GUI on stockpile's GUI.
#define GUI_SPACING					5

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Exported (extern) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables / Instantiate class statics.
//////////////////////////////////////////////////////////////////////////////

CStockPile	CStockPile::ms_stockpileMax	=			// Maximum one can carry 
																	// w/o a backpack.
	{
	1000,			// m_sHitPoints
					                     
	10,			// m_sNumGrenades
	10,			// m_sNumFireBombs
	5,				// m_sNumMissiles
	5,				// m_sNumNapalms
	100,			// m_sNumBullets
	50,			// m_sNumShells
	250,			// m_sNumFuel
	5,				// m_sNumMines
	5,				// m_sNumHeatseekers
					                     
	1,				// m_sMachineGun
	1,				// m_sMissileLauncher
	1,				// m_sShotGun
	1,				// m_sSprayCannon
	1,				// m_sFlameThrower
	1,				// m_sNapalmLauncher
	1,				// m_sDeathWadLauncher
	1,				// m_sDoubleBarrel
					                     
	100,			// m_sKevlarLayers
					                     
	1,				// m_sBackpack
	};

CStockPile	CStockPile::ms_stockpileBackPackMax	=	// Maximum one can carry
																	// with a backpack.
	{
	1000,			// m_sHitPoints
					                     
	20,			// m_sNumGrenades
	20,			// m_sNumFireBombs
	15,			// m_sNumMissiles
	15,			// m_sNumNapalms
	100,			// m_sNumBullets
	100,			// m_sNumShells
	500,			// m_sNumFuel
	10,			// m_sNumMines
	15,			// m_sNumHeatseekers
					                     
	1,				// m_sMachineGun
	1,				// m_sMissileLauncher
	1,				// m_sShotGun
	1,				// m_sSprayCannon
	1,				// m_sFlameThrower
	1,				// m_sNapalmLauncher
	1,				// m_sDeathWadLauncher
	1,				// m_sDoubleBarrel
					                     
	100,			// m_sKevlarLayers
					                     
	1,				// m_sBackpack
	};

short		CStockPile::ms_sEnableDeathWad		= FALSE;	// Enable the death wad
																		// check box.

short		CStockPile::ms_sEnableDoubleBarrel	= FALSE;	// Enable the double barrel
																		// check box.

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a GUI, set its text to the value, and recompose it.
////////////////////////////////////////////////////////////////////////////////
inline
void SetText(					// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	long			lId,			// In:  ID of GUI to set text.
	long			lVal)			// In:  Value to set text to.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui != NULL)
		{
		pgui->SetText("%ld", lVal);
		pgui->Compose(); 
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a RMultiBtn GUI, and set its state.
////////////////////////////////////////////////////////////////////////////////
inline
void CheckMultiBtn(			// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	long			lId,			// In:  ID of GUI to set text.
	short			sChecked)	// In:  1 to check, 0 to uncheck.
	{
	short	sRes	= 0;	// Assume nothing;

	RMultiBtn*	pmb	= (RMultiBtn*)pguiRoot->GetItemFromId(lId);
	if (pmb != NULL)
		{
		ASSERT(pmb->m_type == RGuiItem::MultiBtn);

		pmb->m_sState	 = sChecked + 1;

		pmb->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a RMultiBtn GUI, and return its state.
////////////////////////////////////////////////////////////////////////////////
inline
short IsMultiBtnChecked(	// Returns multibtn's state.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	long			lId)			// In:  ID of GUI to set text.
	{
	short	sRes	= 0;	// Assume nothing;

	RMultiBtn*	pmb	= (RMultiBtn*)pguiRoot->GetItemFromId(lId);
	if (pmb != NULL)
		{
		ASSERT(pmb->m_type == RGuiItem::MultiBtn);

		sRes	= (pmb->m_sState == 1) ? 0 : 1;
		}

	return sRes;
	}

//////////////////////////////////////////////////////////////////////////////
// Allow user to edit members.
//////////////////////////////////////////////////////////////////////////////
short CStockPile::UserEdit(				// Returns 0 on success.
	RGuiItem*	pguiChild /*= NULL*/)	// In: Optional child GUI to be placed at 
													// botom of Stockpile GUI.
	{
	short	sResult	= 0;

	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(GUI_FILE_NAME));
	if (pgui)
		{
		// If there is a user specified GUI . . .
		if (pguiChild)
			{
			short	sChildPosX;
			short	sChildPosY;
			short	sOkHeight;
			// Get the position of the OK button.
			RGuiItem*	pguiOk		= pgui->GetItemFromId(1);
			RGuiItem*	pguiCancel	= pgui->GetItemFromId(2);
			if (pguiOk)
				{
				sChildPosY	= pguiOk->m_sY;
				sOkHeight	= pguiOk->m_im.m_sHeight;
				}
			else
				{
				sChildPosY	= pgui->m_im.m_sHeight;
				sOkHeight	= 0;
				}

			sChildPosX	= GUI_SPACING;

			// Expand stockpile's GUI as necessary allowing space between
			// stockpile's stuff and user's stuff and the bottom.
			short	sOldHeight	= pgui->m_im.m_sHeight;
			short	sNewWidth	= MAX(short(sChildPosX + pguiChild->m_im.m_sWidth + GUI_SPACING), pgui->m_im.m_sWidth);
			short	sNewHeight	= sChildPosY + pguiChild->m_im.m_sHeight + GUI_SPACING + sOkHeight;
			if (pgui->Create(		// Returns 0 on success.
				pgui->m_sX,
				pgui->m_sY,
				sNewWidth,
				sNewHeight,
				pgui->m_im.m_sDepth) == 0)
				{
				// Parent and move the child GUI.
				pguiChild->Move(
					pgui->m_im.m_sWidth / 2 - pguiChild->m_im.m_sWidth / 2, 
					sChildPosY);
				pguiChild->SetParent(pgui);

				// Reposition OK and Cancel.
				RSP_SAFE_GUI_REF_VOID(pguiOk, Move(pguiOk->m_sX, pguiOk->m_sY + (pgui->m_im.m_sHeight - sOldHeight) ) );
				RSP_SAFE_GUI_REF_VOID(pguiCancel, Move(pguiCancel->m_sX, pguiCancel->m_sY + (pgui->m_im.m_sHeight - sOldHeight) ) );
				}
			else
				{
				TRACE("UserEdit(): Failed to resize Stockpile GUI to accommodate "
					"user's GUI.\n");
				}
			}

		SetText(pgui, HITPOINTS_GUI_ID,			m_sHitPoints		);
															
		SetText(pgui, BULLETS_GUI_ID,				m_sNumBullets		);
		SetText(pgui, GRENADES_GUI_ID,			m_sNumGrenades		);
		SetText(pgui, FIREBOMB_GUI_ID,			m_sNumFireBombs	);
		SetText(pgui, MISSILE_GUI_ID,				m_sNumMissiles		);
		SetText(pgui, NAPALM_GUI_ID,				m_sNumNapalms		);
		SetText(pgui, SHELLS_GUI_ID,				m_sNumShells		);
		SetText(pgui, MINES_GUI_ID,				m_sNumMines			);
		SetText(pgui, HEATSEEKERS_GUI_ID,		m_sNumHeatseekers	);
		SetText(pgui, FUEL_GUI_ID,					m_sNumFuel			);
		
		SetText(pgui, ARMOR_GUI_ID,				m_sKevlarLayers	);

		CheckMultiBtn(pgui, MACHINEGUN_GUI_ID,			m_sMachineGun			);
		CheckMultiBtn(pgui, MISSILELAUNCHER_GUI_ID,	m_sMissileLauncher	);
		CheckMultiBtn(pgui, SHOTGUN_GUI_ID,				m_sShotGun				);
		CheckMultiBtn(pgui, SPRAYCANNON_GUI_ID,		m_sSprayCannon			);
		CheckMultiBtn(pgui, FLAMETHROWER_GUI_ID,		m_sFlameThrower		);
		CheckMultiBtn(pgui, NAPALMLAUNCHER_GUI_ID,	m_sNapalmLauncher		);
		CheckMultiBtn(pgui, DEATHWADLAUNCHER_GUI_ID,	m_sDeathWadLauncher	);
		CheckMultiBtn(pgui, DOUBLEBARREL_GUI_ID,		m_sDoubleBarrel		);

		// If available . . .
		RSP_SAFE_GUI_REF_VOID(pgui->GetItemFromId(DEATHWADLAUNCHER_GUI_ID), SetVisible(ms_sEnableDeathWad) );
		RSP_SAFE_GUI_REF_VOID(pgui->GetItemFromId(DEATHWADLAUNCHER_TXT_ID), SetVisible(ms_sEnableDeathWad) );

		// If available . . .
		RSP_SAFE_GUI_REF_VOID(pgui->GetItemFromId(DOUBLEBARREL_GUI_ID), SetVisible(ms_sEnableDoubleBarrel) );
		RSP_SAFE_GUI_REF_VOID(pgui->GetItemFromId(DOUBLEBARREL_TXT_ID), SetVisible(ms_sEnableDoubleBarrel) );

		CheckMultiBtn(pgui, BACKPACK_GUI_ID,			m_sBackpack			);

		sResult = CThing::DoGui(pgui);
		if (sResult == 1)
			{
			// Get new values.
			m_sHitPoints			= pgui->GetVal(HITPOINTS_GUI_ID			);
																							
			m_sNumBullets			= pgui->GetVal(BULLETS_GUI_ID				);
			m_sNumGrenades			= pgui->GetVal(GRENADES_GUI_ID			);
			m_sNumFireBombs		= pgui->GetVal(FIREBOMB_GUI_ID			);
			m_sNumMissiles			= pgui->GetVal(MISSILE_GUI_ID				);
			m_sNumNapalms			= pgui->GetVal(NAPALM_GUI_ID				);
			m_sNumShells			= pgui->GetVal(SHELLS_GUI_ID				);
			m_sNumMines				= pgui->GetVal(MINES_GUI_ID				);
			m_sNumHeatseekers		= pgui->GetVal(HEATSEEKERS_GUI_ID		);
			m_sNumFuel				= pgui->GetVal(FUEL_GUI_ID					);

			m_sKevlarLayers		= pgui->GetVal(ARMOR_GUI_ID				);

			m_sMachineGun			= IsMultiBtnChecked(pgui, MACHINEGUN_GUI_ID			);
			m_sMissileLauncher	= IsMultiBtnChecked(pgui, MISSILELAUNCHER_GUI_ID	);
			m_sShotGun				= IsMultiBtnChecked(pgui, SHOTGUN_GUI_ID				);
			m_sSprayCannon			= IsMultiBtnChecked(pgui, SPRAYCANNON_GUI_ID			);
			m_sFlameThrower		= IsMultiBtnChecked(pgui, FLAMETHROWER_GUI_ID		);
			m_sNapalmLauncher		= IsMultiBtnChecked(pgui, NAPALMLAUNCHER_GUI_ID		);
			m_sDeathWadLauncher	= IsMultiBtnChecked(pgui, DEATHWADLAUNCHER_GUI_ID	);
			m_sDoubleBarrel		= IsMultiBtnChecked(pgui, DOUBLEBARREL_GUI_ID		);

			m_sBackpack				= IsMultiBtnChecked(pgui, BACKPACK_GUI_ID				);

			// Success.
			sResult	= 0;
			}

		// If there is a user specified GUI . . .
		if (pguiChild)
			{
			// Get it outta there before we delete the tree.
			pguiChild->SetParent(NULL);
			}

		delete pgui;
		}

	return sResult;
	}

///////////////////////////////////////////////////////////////////////////////
// Save stockpile to the specified file.
///////////////////////////////////////////////////////////////////////////////
short CStockPile::Save(		// Returns 0 on success.
	RFile*	pfile)			// In:  File to save to.
	{
	pfile->Write(m_sDoubleBarrel		);

	pfile->Write(m_sDeathWadLauncher	);

	pfile->Write(m_sNapalmLauncher	);

	pfile->Write(m_sBackpack			);

	pfile->Write(m_sKevlarLayers		);

	pfile->Write(m_sMachineGun			);
	pfile->Write(m_sMissileLauncher	);
	pfile->Write(m_sShotGun				);
	pfile->Write(m_sSprayCannon		);
	pfile->Write(m_sFlameThrower		);

	pfile->Write(m_sHitPoints			);
	pfile->Write(m_sNumGrenades		);
	pfile->Write(m_sNumFireBombs		);
	pfile->Write(m_sNumMissiles		);
	pfile->Write(m_sNumNapalms			);
	pfile->Write(m_sNumBullets			);
	pfile->Write(m_sNumShells			);
	pfile->Write(m_sNumFuel				);
	pfile->Write(m_sNumMines			);
	pfile->Write(m_sNumHeatseekers	);

	return pfile->Error();
	}

///////////////////////////////////////////////////////////////////////////////
// Load stockpile from the specified file.
///////////////////////////////////////////////////////////////////////////////
short CStockPile::Load(		// Returns 0 on success.
	RFile*	pfile,			// In:  File to load from.
	ULONG		ulVersion)		// In:  File version to load. 
	{
	// Zero() out first in case file version doesn't support a value.
	Zero();

	switch (ulVersion)
		{
		default:
		case 41:
			pfile->Read(&m_sDoubleBarrel		);
		case 40:
		case 39:
		case 38:
		case 37:
		case 36:
			pfile->Read(&m_sDeathWadLauncher	);
		case 35:
		case 34:
		case 33:
			pfile->Read(&m_sNapalmLauncher	);
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
			pfile->Read(&m_sBackpack			);
		case 21:
			pfile->Read(&m_sKevlarLayers		);
		case 20:
		case 19:
			pfile->Read(&m_sMachineGun			);
			pfile->Read(&m_sMissileLauncher	);
			pfile->Read(&m_sShotGun				);
			pfile->Read(&m_sSprayCannon		);
			pfile->Read(&m_sFlameThrower		);
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
			pfile->Read(&m_sHitPoints			);
			pfile->Read(&m_sNumGrenades		);
			pfile->Read(&m_sNumFireBombs		);
			pfile->Read(&m_sNumMissiles		);
			pfile->Read(&m_sNumNapalms			);
			pfile->Read(&m_sNumBullets			);
			pfile->Read(&m_sNumShells			);
			pfile->Read(&m_sNumFuel				);
			pfile->Read(&m_sNumMines			);
			pfile->Read(&m_sNumHeatseekers	);
			break;
		}

	return pfile->Error();
	}

///////////////////////////////////////////////////////////////////////////////
// Add another stockpile to this one.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Add(				// Returns nothing.
	const CStockPile*	pstockpile)	// In:  Stockpile to add to this one.
	{
	short	sTypeIndex;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		GetItem(sTypeIndex)	+= ((CStockPile*)pstockpile)->GetItem(sTypeIndex);
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Subtract another stockpile from this one.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Sub(				// Returns nothing.
	const CStockPile*	pstockpile)	// In:  Stockpile to sub from this one.
	{
	short	sTypeIndex;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		GetItem(sTypeIndex)	-= ((CStockPile*)pstockpile)->GetItem(sTypeIndex);
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Combine specified stockpile and this stockpile into this one
// using maximum extents from either.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Union(					// Returns nothing.
	const CStockPile*	pstockpile)		// In:  Stockpile to combine into this one.
	{
	short	sTypeIndex;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		GetItem(sTypeIndex)	= MAX(GetItem(sTypeIndex), ((CStockPile*)pstockpile)->GetItem(sTypeIndex) );
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Combine specified stockpile and this stockpile into this one
// using minimum extents from either.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Intersect(			// Returns nothing.
	const CStockPile*	pstockpile)		// In:  Stockpile to combine into this one.
	{
	short	sTypeIndex;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		GetItem(sTypeIndex)	= MIN(GetItem(sTypeIndex), ((CStockPile*)pstockpile)->GetItem(sTypeIndex) );
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Copy the specified stockpile over this one.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Copy(					// Returns nothing.
	const CStockPile*	pstockpile)		// In:  Stockpile to copy from.
	{
	short	sTypeIndex;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		GetItem(sTypeIndex)	= ((CStockPile*)pstockpile)->GetItem(sTypeIndex);
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Zero the stockpile.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Zero(void)			// Returns nothing.
	{
	short	sTypeIndex;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		GetItem(sTypeIndex)	= 0;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Truncate based on maximums that can be carried.
// Note that this varies based on m_sBackpack.
///////////////////////////////////////////////////////////////////////////////
void CStockPile::Truncate(void)				// Returns nothing.
	{
	// If we have a backpack . . .
	if (m_sBackpack != 0)
		{
		Intersect(&ms_stockpileBackPackMax);
		}
	else
		{
		Intersect(&ms_stockpileMax);
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Index by the StockPileItem you are interested in.
///////////////////////////////////////////////////////////////////////////////
short& CStockPile::GetItem(	// Returns a reference to the indexed item.
	short				 sIndex)		// In:  The item index.
	{
	// KEEP THIS SWITCH STATEMENT IN ORDER OF SMALLEST TO LARGEST LEAVING OUT
	// NO INTERMEDIATE VALUES.
	// This allows the compiler, when optimizing for speed, to make a jump table.
	switch (sIndex)
		{
		case Bullets:
			return m_sNumBullets;
		case Grenades:
			return m_sNumGrenades;
		case Cocktails:
			return m_sNumFireBombs;
		case Rockets:
			return m_sNumMissiles;
		case Napalm:
			return m_sNumNapalms;
		case Shells:
			return m_sNumShells;
		case Fuel:
			return m_sNumFuel;
		case Mines:
			return m_sNumMines;
		case Health:
			return m_sHitPoints;
		case Heatseekers:
			return m_sNumHeatseekers;
		case MachineGun:
			return m_sMachineGun;
		case MissileLauncher:
			return m_sMissileLauncher;
		case ShotGun:
			return m_sShotGun;
		case SprayCannon:
			return m_sSprayCannon;
		case FlameThrower:
			return m_sFlameThrower;
		case NapalmLauncher:
			return m_sNapalmLauncher;
		case DeathWadLauncher:
			return m_sDeathWadLauncher;
		case DoubleBarrel:
			return m_sDoubleBarrel;
		case KevlarVest:
			return m_sKevlarLayers;
		case Backpack:
			return m_sBackpack;
		default:
			{
			TRACE("GetItem(): Unknown type passed.\n");
			ASSERT(0);

			// This should never happen.
			// Try to save the day in release mode.
			static short sUnknown;
			sUnknown	= 0;
			return sUnknown;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Index by the CDude::WeaponType you are interested in.
///////////////////////////////////////////////////////////////////////////////
short& CStockPile::GetWeapon(		// Returns a reference to the indexed item.
	short	sIndex)						// In:  The item index.
	{
	static short sNoWeapon;
	// KEEP THIS SWITCH STATEMENT IN ORDER OF SMALLEST TO LARGEST LEAVING OUT
	// NO INTERMEDIATE VALUES.
	// This allows the compiler, when optimizing for speed, to make a jump table.
	switch (sIndex)
		{
		case CDude::SemiAutomatic:
			return m_sMachineGun;
		case CDude::ShotGun:
			return m_sShotGun;
		case CDude::SprayCannon:
			return m_sSprayCannon;
		case CDude::Grenade:
			sNoWeapon	= 0;
			return sNoWeapon;
		case CDude::Rocket:
			return m_sMissileLauncher;
		case CDude::Heatseeker:
			return m_sMissileLauncher;
		case CDude::FireBomb:
			sNoWeapon	= 0;
			return sNoWeapon;
		case CDude::Napalm:
			return m_sNapalmLauncher;
		case CDude::FlameThrower:
			return m_sFlameThrower;
		case CDude::ProximityMine:
		case CDude::TimedMine:
		case CDude::RemoteMine:
		case CDude::BouncingBettyMine:
			sNoWeapon	= 0;
			return sNoWeapon;
		case CDude::DeathWad:
			return m_sDeathWadLauncher;
		case CDude::DoubleBarrel:
			return m_sDoubleBarrel;
		default:
			{
			TRACE("GetWeapon(): Unknown type passed.\n");
			ASSERT(0);

			// This should never happen.
			// Try to save the day in release mode.
			sNoWeapon	= 0;
			return sNoWeapon;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Determine if this stockpile is empty.
////////////////////////////////////////////////////////////////////////////////
bool CStockPile::IsEmpty(void)		// Returns true, if the stockpile is
												// empty.
	{
	// Inventory.
	short	sTypeIndex;
	bool	bEmpty	= true;
	for (sTypeIndex = 0; sTypeIndex < NumStockPileItems; sTypeIndex++)
		{
		if (GetItem(sTypeIndex) > 0)
			{
			bEmpty	= false;
			break;
			}
		}

	return bEmpty;
	}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
