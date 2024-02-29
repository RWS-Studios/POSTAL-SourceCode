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
// StockPile.H
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
//		06/15/97	JMI	Added m_sBackpack, maximums, and Truncate().
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
//
//		07/30/97	JMI	Added DeathWadLauncher.
//							Made pstockpile inputs to functions const.  This caused
//							some silly stupidity in GetItem() calls but I just casted
//							it away b/c the stockpile was not being modified.
//
//		08/07/97	JMI	Added DoubleBarrel.
//
//		12/08/97	JMI	Added GetWeapon() that takes a CDude::WeaponType enum
//							to index into the stockpile.
//
//////////////////////////////////////////////////////////////////////////////
//
// This represents a character's stockpile of weapons and health.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef STOCKPILE_H
#define STOCKPILE_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
class CStockPile
	{
	///////////////////////////////////////////////////////////////////////////
	// Macros.
	///////////////////////////////////////////////////////////////////////////
	public:

		// Use this to enumerate the stockpile through GetItem().
		typedef enum
			{
			Bullets,
			Grenades,
			Cocktails,
			Rockets,
			Napalm,
			Shells,
			Fuel,
#if 0
			ProximityMine, 
			TimedMine, 
			RemoteMine, 
			BouncingBettyMine,
#else
			Mines,
#endif
			Health,
			Heatseekers,

			MachineGun,
			MissileLauncher,
			ShotGun,
			SprayCannon,
			FlameThrower,
			NapalmLauncher,
			DeathWadLauncher,
			DoubleBarrel,

			KevlarVest,

			Backpack,
			
			NumStockPileItems
			};

	///////////////////////////////////////////////////////////////////////////
	// Con/Destruction.
	///////////////////////////////////////////////////////////////////////////
	public:
#if 0	// By not having a constructor, virtual funcs, and a base class we 
		// become aggregate and can, therefore, be initialized with an initializer
		// list.  Don't forget to Zero() this since the constructor won't!
		CStockPile()
			{
			Zero();
			}

		~CStockPile() 
			{
			}
#endif

	///////////////////////////////////////////////////////////////////////////
	// Methods.
	///////////////////////////////////////////////////////////////////////////
	public:

		// Allow user to edit the stockpile.
		short UserEdit(					// Returns 0 on success.
			RGuiItem*	pgui = NULL);	// In: Optional child GUI to be placed at 
												// botom of Stockpile GUI.

		// Save stockpile to the specified file.
		short Save(						// Returns 0 on success.
			RFile*	pfile);			// In:  File to save to.

		// Load stockpile from the specified file.
		short Load(						// Returns 0 on success.
			RFile*	pfile,			// In:  File to load from.
			ULONG		ulVersion);		// In:  File version to load. 

		// Add another stockpile to this one.
		void Add(									// Returns nothing.
			const CStockPile*	pstockpile);	// In:  Stockpile to add to this one.

		// Subtract another stockpile from this one.
		void Sub(									// Returns nothing.
			const CStockPile*	pstockpile);	// In:  Stockpile to sub from this one.

		// Combine specified stockpile and this stockpile into this one
		// using maximum extents from either.
		void Union(									// Returns nothing.
			const CStockPile*	pstockpile);	// In:  Stockpile to combine into this one.

		// Combine specified stockpile and this stockpile into this one
		// using minimum extents from either.
		void Intersect(							// Returns nothing.
			const CStockPile*	pstockpile);	// In:  Stockpile to combine into this one.

		// Copy the specified stockpile over this one.
		void Copy(									// Returns nothing.
			const CStockPile*	pstockpile);	// In:  Stockpile to copy from.

		// Zero the stockpile.
		void Zero(void);					// Returns nothing.

		// Truncate based on maximums that can be carried.
		// Note that this varies based on m_sBackpack.
		void Truncate(void);				// Returns nothing.

		// Index by the StockPileItem you are interested in.
		short& GetItem(	// Returns a reference to the indexed item.
			short sIndex);	// In:  The item index.

		// Index by the CDude::WeaponType you are interested in.
		short& GetWeapon(		// Returns a reference to the indexed item.
			short	sIndex);		// In:  The item index.

		// Determine if this stockpile is empty.
		bool	IsEmpty(void);				// Returns true, if the stockpile is
												// empty.


	///////////////////////////////////////////////////////////////////////////
	// Querries.
	///////////////////////////////////////////////////////////////////////////
	public:


	///////////////////////////////////////////////////////////////////////////
	// Instantiable data.
	///////////////////////////////////////////////////////////////////////////
	public:

		// *********************************************************************
		// *********************************************************************
		// PLEASE, PLEASE, PLEASE, use short type only for members of this
		// class!!!!  Otherwise, GetItem() won't work for that item.
		// *********************************************************************
		// *********************************************************************
		short	m_sHitPoints;					// Health status.

		short	m_sNumGrenades;				// How many grenades dude has.
		short	m_sNumFireBombs;				// How many firebombs dude has.
		short	m_sNumMissiles;				// How many missiles dude has.
		short	m_sNumNapalms;					// How many napalms dude has.
		short	m_sNumBullets;					// How many bullets dude has.
		short	m_sNumShells;					// How many shotgun shells dude has.
		short	m_sNumFuel;						// How much fuel dude has.
		short	m_sNumMines;					// How many mines dude has.
		short	m_sNumHeatseekers;			// How many heatseekers dude has.

		short	m_sMachineGun;					// 0 if no machine gun, nonzero otherwise.
		short	m_sMissileLauncher;			// 0 if no missile launcher, nonzero otherwise.
		short	m_sShotGun;						// 0 if no shotgun, nonzero otherwise.
		short	m_sSprayCannon;				// 0 if no spray cannon, nonzero otherwise.
		short	m_sFlameThrower;				// 0 if no flame thrower, nonzero otherwise.
		short	m_sNapalmLauncher;			// 0 if no napalm launcher, nonzero otherwise.
		short	m_sDeathWadLauncher;			// 0 if no deathwad launcher, nonzero otherwise.
		short	m_sDoubleBarrel;				// 0 if no double barrel, nonzero otherwise.

		short	m_sKevlarLayers;				// 0 .. n layers of 'armor'.

		short	m_sBackpack;					// 0 if no backpack, nonzero otherwise.


	///////////////////////////////////////////////////////////////////////////
	// Static data.
	///////////////////////////////////////////////////////////////////////////
	public:

		static CStockPile	ms_stockpileMax;				// Maximum one can carry 
																	// w/o a backpack.
		static CStockPile	ms_stockpileBackPackMax;	// Maximum one can carry
																	// with a backpack.
		static short		ms_sEnableDeathWad;			// Enable the death wad
																	// check box.
		static short		ms_sEnableDoubleBarrel;		// Enable the double barrel
																	// check box.
	};

#endif	// STOCKPILE_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
