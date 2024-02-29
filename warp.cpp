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
// Warp.cpp
// Project: Postal
// 
// History:
//		06/02/97 JMI	Started.
//
//		06/06/97	JMI	Priority was being set incorrectly (was adding the height
//							of this object to our Z, but since our hotspot is at our
//							bottom, our priority is simply our Z).
//
//		06/07/97	JMI	Added WarpIn() and WarpInAnywhere() function bodies.
//							Also, added CreateWarpFromDude().
//
//		06/15/97	JMI	Moved initialization of ms_stockpile into warp.cpp since
//							the stockpile is non aggregatable.
//
//		06/16/97	JMI	Moved initialization of dude's original hitpoints such
//							that both warp types (ones where he is preallocated and
//							ones where he is allocated by the WarpIn called) set it.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/29/97	JMI	Converted EditRect(), EditRender(), and/or Render() to
//							use Map3Dto2D().
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/19/97	JMI	Added m_sRotY, the dude's initial rotation around the Y
//							axis.  Also, now passes a GUI to CStockPile::UserEdit()
//							with some settings for us.
//
//		08/03/97 BRH	Increased the starting hit points for the Dude if the
//							game difficulty is set to 11.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/08/97	JMI	Upped GUI_ID_ROT_Y from 101 to 1001 so it wouldn't 
//							conflict with the stockpile dialog.
//
//		08/10/97	JMI	Now the warp places the dude via a call to 
//							CDude::SetPosition().
//
//		08/14/97	JMI	Switched references to g_GameSettings.m_sDifficulty to
//							m_pRealm->m_flags.sDifficulty.
//
//		09/30/97	JMI	Filled in empty fields in ms_stockpile.  Although it is
//							not necessary b/c of the implicit filescope zero init,
//							it just seems like a good idea.  But they are still zero
//							(just using enums instead of 0s) so there should be no 
//							synch problems with old versions.
//
//		12/01/97 BRH	For multiplayer deathmatch, the warp sets the hit
//							points to WARP_DEATHMATCH_DEFAULT_HP which will be
//							set to an amount we determine to be more fun.  Right
//							now, most levels are set to 500 hit points, but for
//							deathmatch, its too hard to kill each other.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent places and settings for dudes to
// 'warp' in at/with.
//
//////////////////////////////////////////////////////////////////////////////
#define WARP_CPP

#include "RSPiX.h"

#include "warp.h"
#include "game.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// This is for edit mode only.
#define WARP_IMAGE_FILENAME		"warp.bmp"

#define GUI_FILENAME					"res/editor/warp.gui"

#define GUI_ID_ROT_Y					1001

#define WARP_DEATHMATCH_DEFAULT_HP 100

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// The stockpile of ammo and health used by all CWarps.
CStockPile	CWarp::ms_stockpile	=
	{
	// Use CDude defaults for stockpile.
	CDude::DefHitPoints,

	CDude::DefNumGrenades,
	CDude::DefNumFireBombs,
	CDude::DefNumMissiles,
	CDude::DefNumNapalms,
	CDude::DefNumBullets,
	CDude::DefNumShells,
	CDude::DefNumFuel,
	CDude::DefNumMines,
	CDude::DefNumHeatseekers,

	CDude::DefHasMachineGun,
	CDude::DefHasLauncher,
	CDude::DefHasShotgun,
	CDude::DefHasSprayCannon,
	CDude::DefHasFlamer,
	CDude::DefHasNapalmLauncher,	
	CDude::DefHasDeathWadLauncher,	
	CDude::DefHasDoubleBarrel,		

	CDude::DefKevlarLayers,
			                   
	CDude::DefHasBackpack,
	};

// Tracks file counter so we know when to load/save "common" data 
short	CWarp::ms_sFileCount	= 0;

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CWarp::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,										// In:  File to load from
	bool bEditMode,									// In:  True for edit mode, false otherwise
	short sFileCount,									// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)								// In:  Version of file format to load.
	{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// If statics have not yet been loaded . . .
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			ms_stockpile.Load(pFile, ulFileVersion);
			}

		switch (ulFileVersion)
			{
			default:
			case 32:
				pFile->Read(&m_sRotY);
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
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				break;
			}

		// If the file version is earlier than the change to real 3D coords . . .
		if (ulFileVersion < 24)
			{
			// Convert to 3D.
			m_pRealm->MapY2DtoZ3D(
				m_dZ,
				&m_dZ);
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// If in edit mode . . .
			if (bEditMode == true)
				{
				// Get resources and initialize.
				sResult = Init();
				}
			}
		else
			{
			sResult = -1;
			TRACE("CWarp::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CWarp::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			ms_stockpile.Save(pFile);
			}

		pFile->Write(&m_sRotY);
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
short CWarp::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CWarp::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CWarp::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CWarp::Resume(void)
	{
	m_sSuspend--;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CWarp::Update(void)
	{
	// Do schtuff.
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CWarp::Render(void)
	{
	// Doesn't normally draw anything (see EditRender() for render during
	// edit mode).
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CWarp::EditNew(								// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	sResult	= Init();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CWarp::EditModify(void)
	{
	short	sResult	= 0;
	// Load our GUI.
	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(GUI_FILENAME));
	if (pgui)
		{
		// Set current Y axis rotation.
		RGuiItem*	pguiRotY	= pgui->GetItemFromId(GUI_ID_ROT_Y);
		if (pguiRotY)
			{
			pguiRotY->SetText("%d", m_sRotY);
			pguiRotY->Compose();
			}

		// Display stockpile's GUI with ours at the bottom.
		sResult	= ms_stockpile.UserEdit(pgui);

		// If not cancelled . . .
		if (sResult == 0)
			{
			if (pguiRotY)
				{
				m_sRotY	= pguiRotY->GetVal();
				}
			}

		// Destroy our GUI.
		delete pgui;
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CWarp::EditMove(								// Returns 0 if successfull, non-zero otherwise
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
void CWarp::EditRect(	// Returns nothiing.
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

	if (m_sprite.m_pImage != NULL)
		{
		prc->sW	= m_sprite.m_pImage->m_sWidth;
		prc->sH	= m_sprite.m_pImage->m_sHeight;
		}

	prc->sX	-= prc->sW / 2;
	prc->sY	-= prc->sH;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CWarp::EditHotSpot(	// Returns nothiing.
	short*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	short*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
	{
	*psX	= 0;	// Safety.
	*psY	= 0;	// Safety.

	if (m_sprite.m_pImage != NULL)
		{
		*psX	= m_sprite.m_pImage->m_sWidth / 2;
		*psY	= m_sprite.m_pImage->m_sHeight;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CWarp::EditUpdate(void)
	{
	// The editor schtuff.
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CWarp::EditRender(void)
	{
	// Map from 3d to 2d coords
	Map3Dto2D(
		(short) m_dX, 
		(short) m_dY, 
		(short) m_dZ, 
		&m_sprite.m_sX2, 
		&m_sprite.m_sY2);

	// Priority is based on bottom edge of sprite on viewing plane.
	m_sprite.m_sPriority = m_dZ;
		
	// Center on image.
	m_sprite.m_sX2	-= m_sprite.m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_sprite.m_pImage->m_sHeight;

	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}


////////////////////////////////////////////////////////////////////////////////
// Initialize object.
////////////////////////////////////////////////////////////////////////////////
short CWarp::Init(void)	// Returns 0 on success.
	{
	short	sRes	= GetResources();

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CWarp::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Safe to call even if no resource.
	FreeResources();

	sResult	= rspGetResource(
		&g_resmgrGame, 
		m_pRealm->Make2dResPath(WARP_IMAGE_FILENAME),
		&m_sprite.m_pImage);

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CWarp::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	if (m_sprite.m_pImage != NULL)
		{
		rspReleaseResource(&g_resmgrGame, &m_sprite.m_pImage);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Stocks, rejuvenates, and places a CDude.  The dude can be passed to this
// function or allocated by this function.
////////////////////////////////////////////////////////////////////////////////
short CWarp::WarpIn(	// Returns 0 on success.
	CDude**	ppdude,	// In:  CDude to 'warp in', *ppdude = NULL to create one.
							// Out: Newly created CDude, if no CDude passed in.
	short	sOptions)	// In:  Options for 'warp in'.
	{
	short	sRes	= 0;	// Assume success.

	// If we are on difficulty 11, multiply the dude's hit points by 10 so that
	// the player can have some chance of playing.
	if (m_pRealm->m_flags.sDifficulty == 11)
	{
		ms_stockpile.m_sHitPoints *= 10;
		if (ms_stockpile.m_sHitPoints < 0)
			ms_stockpile.m_sHitPoints = 30000;
	}

	// If we are playing multiplayer deathmatch, then set the hitpoints to the
	// default deathmatch amount so that it is easier to kill each other 
	// and thus more fun.
	if (m_pRealm->m_flags.bMultiplayer == true && m_pRealm->m_flags.bCoopMode == false)
	{
		ms_stockpile.m_sHitPoints = WARP_DEATHMATCH_DEFAULT_HP;
	}

	// If no dude passed . . .
	if (*ppdude == NULL)
		{
		sRes	= ConstructWithID(CDudeID, m_pRealm, (CThing**)ppdude);
		if (sRes == 0)
			{
			// Copy stockpile to new CDude.
			(*ppdude)->m_stockpile.Copy(&ms_stockpile);
			// Initialize.
			sRes	= (*ppdude)->Init();
			if (sRes == 0)
				{
				// Start up.
				sRes	= (*ppdude)->Startup();
				if (sRes == 0)
					{
					// Successfully created and setup CDude.
					}
				else
					{
					TRACE("WarpIn(): (*ppdude)->Startup() failed.\n");
					}
				}
			else
				{
				TRACE("WarpIn(): (*ppdude)->Init() failed.\n");
				}
			}
		else
			{
			TRACE("WarpIn(): Construct() failed for CDude.\n");
			}
		}
	else
		{
		switch (sOptions & StockPileMask)
			{
			case CopyStockPile:
				// Copy stockpile to CDude.
				(*ppdude)->m_stockpile.Copy(&ms_stockpile);
				break;
			case UnionStockPile:
				// Union stockpile with CDude.
				(*ppdude)->m_stockpile.Union(&ms_stockpile);
				break;
			case AddStockPile:
				// Add stockpile to CDude.
				(*ppdude)->m_stockpile.Add(&ms_stockpile);
				break;
			}
		}

	// If successful so far . . .
	if (sRes == 0)
		{
		// Truncate to amount he can carry.
		(*ppdude)->m_stockpile.Truncate();

		if ((*ppdude)->m_stockpile.m_sHitPoints > 0)
			{
			// Base original hitpoints upon new settings.
			(*ppdude)->m_sOrigHitPoints	= (*ppdude)->m_stockpile.m_sHitPoints;
			}

		// Place.
		(*ppdude)->SetPosition(m_dX, m_dY, m_dZ);
		// Orient.
		(*ppdude)->m_dRot	= m_sRotY;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Stocks, rejuvenates, and places a CDude at a random warp.  The dude can 
// be passed to this function or allocated by this function.
// (static)
////////////////////////////////////////////////////////////////////////////////
short CWarp::WarpInAnywhere(	// Returns 0 on success.
	CRealm*	prealm,				// In:  Realm in which to choose CWarp.
	CDude**	ppdude,				// In:  CDude to 'warp in', *ppdude = NULL to create one.
										// Out: Newly created CDude, if no CDude passed in.
	short	sOptions)				// In:  Options for 'warp in'.
	{
	short	sRes	= 0;	// Assume success.

	// Find a warp:
	
	short	sNumWarps	= prealm->m_asClassNumThings[CWarpID];
	if (sNumWarps > 0)
		{
		// Pick a random warp number.
		short	sWarpNum	= GetRand() % sNumWarps;
		// Find that warp.
		short	i;
		CListNode<CThing>*	pln		= prealm->m_aclassHeads[CWarpID].m_pnNext;
		CListNode<CThing>*	plnTail	= &(prealm->m_aclassTails[CWarpID]);
		for (i = 0; i < sWarpNum && pln != plnTail; i++, pln = pln->m_pnNext)
			;
	
		// If we found one . . .
		if (pln != plnTail)
			{
			ASSERT(pln->m_powner != NULL);

			// Do it.
			sRes	= ((CWarp*)(pln->m_powner) )->WarpIn(ppdude, sOptions);
			}
		else
			{
			TRACE("WarpInAnywhere(): Failed to find chosen CWarp.\n");
			sRes	= -2;
			}
		}
	else
		{
		TRACE("WarpInAnywhere(): Specified realm contains no CWarps.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Creates a warp based on a dude's settings.
// (static)
////////////////////////////////////////////////////////////////////////////////
short CWarp::CreateWarpFromDude(	// Returns 0 on success.
	CRealm*	prealm,					// In:  Realm in which to choose CWarp.
	CDude*	pdude,					// In:  Dude to create warp from.
	CWarp**	ppwarp,					// Out: New warp on success.
	bool		bCopyStockPile)		// In:  true to copy stockpile, false otherwise.
	{
	short	sRes	= 0;	// Assume success.

	// Create warp . . .
	if (ConstructWithID(CWarpID, prealm, (CThing**)ppwarp) == 0)
		{
		// Copy dude's position and orientation.
		(*ppwarp)->m_dX		= pdude->m_dX;
		(*ppwarp)->m_dY		= pdude->m_dY;
		(*ppwarp)->m_dZ		= pdude->m_dZ;
		(*ppwarp)->m_sRotY	= pdude->m_dRot;

		if (bCopyStockPile == true)
			{
			// Copy dude's stockpile.
			// Note that this is static member of warp so this only needs to be for
			// one, but the question is which one.
			(*ppwarp)->ms_stockpile.Copy( &(pdude->m_stockpile) );
			}
		}
	else
		{
		TRACE("CreateWarpFromDude(): ConstructWithID() failed.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
