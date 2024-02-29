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
// hood.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CHood class, which embodies the background,
// the "building" sprites, the "opaque" sprites, and the attribute map.
//
// History:
//		12/25/96 MJR	Started.
//
//		01/22/97	JMI	Now uses one string m_acBaseName to create the filenames
//							for the Background Image, the Attribute Map, and the 
//							Alpha & Opaque layers.
//							Also, added GUI to get this string.
//							
//		01/27/97	JMI	Now fills the edit field with m_acBaseName before
//							presenting the dialog to the user in EditNew().
//
//		01/27/97	JMI	Now loads files as %s%02dAlpha/Opaque instead of
//							%sAlpha/Opaque%02d.	Removed #if block around some hack
//							I'm not sure I understood to begin with.
//
//		01/27/97	JMI	I must've introduced a copy/paste type error or something
//							when I changed the Init().  But, anyways, now opaque layers
//							don't x-ray.
//
//		01/30/97	JMI	Now uses FullPath() to construct paths and uses smaller
//							conventions for layer files to help fit into 8.3 names
//							for ISO 9660 (now uses a for alpha and o for opaque).
//							Also, positions caret at end of line of text in EditNew()'s
//							dialog's edit field.
//
//		02/03/97	JMI	Now loads realm specific assets through the resource 
//							manager.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/04/97	JMI	Changed all resources to pointers so we can fully utilize
//							the RResMgr.
//
//		02/07/97	JMI	Now allows you to setup the world transform 
//							(m_pRealm->m_scene.m_transWorld) via the EditNew/Modify
//							dialog.
//
//		02/07/97	JMI	Now calls m_scene.SetupPipeline() to update the world
//							transform (which is now the view member of the scene's
//							pipeline).
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/13/97	JMI	Changed paths for hoods to be in hoods/ instead of bg/
//							and also now just store the import part (like "city" instead
//							of "hoods/city").
//							Also, now gets resources for lighting effects.
//
//		02/13/97	JMI	In previous update, forgot to release alpha related res's.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/26/97	JMI	No longer skips map load if no layers loaded.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/25/97	JMI	GetResources() now opens the hood sak with the same name as
//							the hood.
//
//		04/09/97 BRH	Added RMultiGrid for the multi layer attribute maps, but
//							haven't taken out the RAttributeMap yet so that the
//							game will continue to work until we completely switch over.
//
//		04/18/97	JMI	Now Startup() calls rspSetPalette*() and rspUpdatePalette()
//							instead of Init().
//
//		04/22/97	JMI	Now ReleaseResources() purges m_pRealm->m_resmgr after
//							releasing the hood's resources so we get them out of memory
//							right away.
//
//		05/09/97	JMI	EditNew() was not checking the result of Init() before
//							calling Startup().  Fixed.
//
//		05/13/97	JMI	Casted instances of warning C4018 (signed/unsigned mismatch)
//							to make MSVC 4.1(Alpha) happy (these seem to fall under
//							Warning Level 3 in 4.1(Alpha) but not in 4.2(Intel)).
//
//		05/15/97	JMI	As per Bill, I changed the default world X axis rotation to
//							30.
//
//		05/26/97	JMI	Now Save()s, Load()s, and EditModify()s 
//							m_pRealm->m_dKillsPercentGoal.
//							
//		05/29/97	JMI	Changed occurences of m_pHeightMap to m_pTerrainMap.
//							Changed occurences of m_pAttrMap to m_pLayerMap.
//							Also, removed occurences of m_pattribMap.
//
//		05/30/97	JMI	The distance between building layers was hard coded to 3.  
//							Now that the distance is 4, I used a more dynamic value
//							(subtract two of the same type of building layers to get
//							this value now).
//
//		06/16/97	JMI	Now CSprite::InBlitOpaque is one of the flags set for the
//							background image when it is added to the CScene.
//
//		06/16/97	JMI	Added SetPalette() which can be used to set the hood's
//							palette at the convenience of the Realm runner.
//
//		06/25/97	JMI	Got rid of use of the obsolete CSprite::InXrayable and now
//							use CSprite::InAlpha in its place.  Also, now the opaques
//							use the CSprite::InOpaque flag.
//
//		06/28/97	JMI	Changed m_sWorldXRot to m_sRealmRotX & GetWorldRotX() and
//							GetRealmRotX(), and added m_sSceneRotX & GetSceneRotX().
//							Also, now loads and saves m_sRealmRotX.
//
//		06/29/97 MJR	Modified to use new RSpry interface (replaced STL).
//
//		06/30/97 MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//		07/01/97	JMI	Added m_sNumInits.  Some things in Init() should only be
//							done once (like getting the resources, adding them to
//							the scene, and allocating the smashatorium).
//
//		07/01/97	JMI	Added m_sScaleAttribHeights indicating whether to scale
//							height values gotten via the terrain map.
//
//		07/09/97	JMI	Added setting for which side view assets to use.
//
//		07/09/97	JMI	Moved m_s2dResPathIndex from CHood to CRealm b/c of order
//							issues when loading.
//
//		07/13/97	JMI	Now only attempts to allocate the smashatorium if the
//							preceding stuff in Init() succeeded.
//
//		08/03/97	JRD	Added ability to save 3d scale with hood
//
//		08/07/97	JMI	Removed call to obsoleted CScene::UpdatePipeline()
//							(CRealm::SetupPipeline() now does it all).
//
//		08/10/97	JRD	Added shadow parameters to the hood & dialogue
//
//		08/13/97 MJR	Released toolbar resources.
//
//		08/13/97	JRD	Began adding Randy's numbers to the toolbar...
//
//		08/18/97 MJR	Changed sprintf() from %lg to %g for ANSI-correctness
//							(and so it would work on the ANSI-correct mac).
//
//		08/17/97	JMI	Added handy macro for updating hood's settings to scene's
//							pipeline, SetupPipeline().
//							Also, some spots were checking if m_dScale3d was less than
//							or equal to 0 and setting it to 0.2 if it was.  This 
//							allowed the value to be 0.000000000001...0.1999999999999
//							which resulted in some wierd behavior including divide-by-
//							zeroes.
//
//		08/20/97	JMI	Now can use a listbox in the EditModify() for 2D res
//							paths as well as the original multibtn.
//
//		11/25/97	JMI	Now checks for Hood SAK on HD first and then on Hoods path.
//							Also, added Browse For Hood button and logic.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "hood.h"
#include "game.h"
#include "realm.h"

#include <string.h>


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define GUI_FILE_NAME	"res/editor/EditHood.gui"

// IDs from loaded GUI.
#define GUI_ID_OK							1
#define GUI_ID_CANCEL					2
#define GUI_ID_BASENAME					3
#define GUI_ID_REALMXROTATE			4
#define GUI_ID_REALMYROTATE			5
#define GUI_ID_REALMZROTATE			6
#define GUI_ID_KILLSPERCENTGOAL		7
#define GUI_ID_SCENEXROTATE			8
#define GUI_ID_SCALEATTRIBHEIGHTS	9
#define GUI_ID_2DRESPATHS				10
#define GUI_ID_3DSCALE					16
#define GUI_ID_SHADOW_ANGLE			888
#define GUI_ID_SHADOW_LENGTH			889
#define GUI_ID_SHADOW_INTENSITY		890
#define GUI_ID_BROWSE					1000

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Constructor
// (protected).
////////////////////////////////////////////////////////////////////////////////
CHood::CHood(CRealm* pRealm)
	: CThing(pRealm, CHoodID)
	{
	// Initialize ptrs to resources.
	m_pimBackground	= NULL;
	m_pTerrainMap		= NULL;
	m_pLayerMap			= NULL;

	short i;
	for (i = 0; i < MaxLayers; i++)
		{
		m_apspryAlphas[i]		= NULL;
		m_apspryOpaques[i]	= NULL;
		}

	m_pimXRayMask		= NULL;
	m_pmaTransparency	= NULL;
	m_pltAmbient		= NULL;
	m_pltSpot			= NULL;

	m_pimEmptyBar		= NULL;			
	m_pimEmptyBarSelected= NULL;	
	m_pimFullBar		= NULL;			
	m_pimFullBarSelected	= NULL;	
	m_pimTopBar = NULL;

	m_pimNum			= NULL;		
	m_pimNumLite	= NULL;
	m_pimNumLow		= NULL;	
	m_pimNumGone	= NULL;		
	
	// Must flag resources as not yet existing
	m_bResourcesExist = false;

	// Initialize.
	strcpy(m_acBaseName, "");

	m_sSceneRotX		= 30;
	m_sRealmRotX		= 45;
	m_dScale3d			= 1.0;

	m_sShadowAngle = 0;	
	m_dShadowLength = 1.0;	
	m_sShadowIntensity = 64;	
	
	m_sScaleAttribHeights	= TRUE;

	// Let realm have quick access to us.
	pRealm->m_phood	= this;

	// We want a Startup() call.
	m_sCallStartup		= 1;

	// No Init() calls yet.
	m_sNumInits			= 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Destructor
// (public).
////////////////////////////////////////////////////////////////////////////////
CHood::~CHood()
	{
	Kill();
	// Clear Realm's ptr.
	m_pRealm->m_phood	= NULL;
	}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CHood::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,									// In:  File to load from
	bool bEditMode,								// In:  True for edit mode, false otherwise
	short sFileCount,								// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)							// In:  Version of file format to load.
	{
	short sResult = 0;

	// In most cases, the base class Load() should be called.
	sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// Load object data
		switch (ulFileVersion)
			{
			default:

			case 44:
				pFile->Read(&m_sScaleAttribHeights);
				pFile->Read(&m_sRealmRotX);
				pFile->Read(&(m_pRealm->m_dKillsPercentGoal) );
				pFile->Read(m_acBaseName);
				pFile->Read(&m_sSceneRotX);
				pFile->Read(&m_dScale3d);

				// Some extra safety (to avoid crashing)
				if (m_dScale3d < 0.2) m_dScale3d = 0.2;
				if (m_dScale3d > 5.0) m_dScale3d = 5.0;

				pFile->Read(&m_sShadowAngle);
				pFile->Read(&m_dShadowLength);
				pFile->Read(&m_sShadowIntensity);

				// Some extra safety (to avoid crashing)
				if (m_dShadowLength < 0.0) m_dShadowLength = 0.0;
				if (m_dShadowLength > 1.5) m_dShadowLength = 1.5;

				break;
			case 43:
			case 42:
			case 41:
			case 40:
			case 39:
				pFile->Read(&m_sScaleAttribHeights);
				pFile->Read(&m_sRealmRotX);
				pFile->Read(&(m_pRealm->m_dKillsPercentGoal) );
				pFile->Read(m_acBaseName);
				pFile->Read(&m_sSceneRotX);
				pFile->Read(&m_dScale3d);

				// Some extra safety
				if (m_dScale3d < 0.2) m_dScale3d = 0.2;
				if (m_dScale3d > 5.0) m_dScale3d = 5.0;

				break;
			case 38:
			case 37:
			case 36:
			case 35:
			case 34:
			case 33:
			case 32:
			case 31:
			case 30:
				pFile->Read(&m_sScaleAttribHeights);
				pFile->Read(&m_sRealmRotX);
				pFile->Read(&(m_pRealm->m_dKillsPercentGoal) );
				pFile->Read(m_acBaseName);
				pFile->Read(&m_sSceneRotX);
				break;

			case 29:
			case 28:
				short	sDummy2dResPathIndex;	// This is no longer stored here.
				pFile->Read(&sDummy2dResPathIndex);

			case 27:
			case 26:
				pFile->Read(&m_sScaleAttribHeights);

			case 25:
			case 24:
			case 23:
				pFile->Read(&m_sRealmRotX);

			case 22:
			case 21:
			case 20:
			case 19:
			case 18:
			case 17:
			case 16:
			case 15:
				pFile->Read(&(m_pRealm->m_dKillsPercentGoal) );

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
				pFile->Read(m_acBaseName);
				pFile->Read(&m_sSceneRotX);
				break;
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CHood::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CHood::Load(): CThing::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CHood::Save(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,									// In:  File to save to
	short sFileCount)								// In:  File count (unique per file, never 0)
	{
	short	sResult	= 0;

	// In most cases, the base class Save() should be called.
	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save object data
		pFile->Write(m_sScaleAttribHeights);
		pFile->Write(m_sRealmRotX);
		pFile->Write(m_pRealm->m_dKillsPercentGoal);
		pFile->Write(m_acBaseName);
		pFile->Write(m_sSceneRotX);
		pFile->Write(m_dScale3d);
		pFile->Write(m_sShadowAngle);
		pFile->Write(m_dShadowLength);
		pFile->Write(m_sShadowIntensity);

		sResult	= pFile->Error();
		if (sResult == 0)
			{
			}
		else
			{
			TRACE("CHood::Save(): Error writing to file.\n");
			}
		}
	else
		{
		TRACE("CHood::Save(): CThing::Save() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CHood::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CHood::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CHood::Suspend(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CHood::Resume(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CHood::Update(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CHood::Render(void)
	{
	}

////////////////////////////////////////////////////////////////////////////////
// User callback on btn released within button.
////////////////////////////////////////////////////////////////////////////////
static void BrowseBtnUp(	// Returns nothing.  Called on button released in
									// m_hot when active.
	RGuiItem* pgui)			// this.
	{
	RGuiItem*	pguiBaseName	= (RGuiItem*)(pgui->m_ulUserInstance);
	if (pguiBaseName)
		{
		// Create full system path from existing RSPiX subpath.
		char	szSystemPath[RSP_MAX_PATH];
		if (pguiBaseName->m_szText[0] == '\0')
			{
			pguiBaseName->SetText(".");
			}

		strcpy(szSystemPath, FullPathHoods("res/hoods/") );
		strcat(szSystemPath, pguiBaseName->m_szText);

		short	sResult;
		do {
			sResult	= SubPathOpenBox(			// Returns 0 on success, negative on error, 1 if 
														// not subpathable (i.e., returned path is full path).
				FullPathHoods("res/hoods"),	// In:  Full path to be relative to (system format). 
				"Browse for Hood",				// In:  Title of box.                                
				szSystemPath,						// In:  Default filename (system format).            
				szSystemPath,						// Out: User's choice (system format).               
				sizeof(szSystemPath),			// In:  Amount of memory pointed to by pszChosenFileName.
				"sak");								// In:  If not NULL, '.' delimited extension based filename
														//	filter specification.  Ex: ".cpp.h.exe.lib" or "cpp.h.exe.lib"
														// Note: Cannot use '.' in filter.  Preceding '.' ignored.

			if (sResult > 0)
				{
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					g_pszAppName,
					g_pszGenericMustBeRelativePath_s,
					FullPathHoods("res/hoods"));
				}

			} while (sResult > 0);

		// If successful in getting a relative path . . .
		if (sResult == 0)
			{
			// Copy back from system format to RSPiX -- we're actually only looking for one word
			// anyway (not a path).
			strcpy(szSystemPath, rspPathFromSystem(szSystemPath) );

			// Get rid of extension.
			short	sIndex;
			for (sIndex = 0; szSystemPath[sIndex]; sIndex++)
				{
				// If this is a dot representing the beginning of an extension . . .
				if (szSystemPath[sIndex] == '.' && szSystemPath[sIndex + 1] != '.' && szSystemPath[sIndex + 1] != '/')
					{
					szSystemPath[sIndex]	= '\0';
					break;
					}
				}

			// Udpate GUI.
			pguiBaseName->SetText("%s", szSystemPath );
			pguiBaseName->Compose();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CHood::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	char	szScale3d[256];
	char	szShadowLength[256];

	// double values must be handled indirectly through strings
	sprintf(szScale3d,"%g",m_dScale3d);
	sprintf(szShadowLength,"%g",m_dShadowLength);

	// Load the GUI for editting.
	RGuiItem*	pguiEdit	= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_HD, GUI_FILE_NAME) );
	if (pguiEdit != NULL)
		{
		// Get ptr to edit field . . .
		RGuiItem*	pguiBaseName	= pguiEdit->GetItemFromId(GUI_ID_BASENAME);
		RGuiItem*	pguiSceneRotX	= pguiEdit->GetItemFromId(GUI_ID_SCENEXROTATE);
		RGuiItem*	pguiRealmRotX	= pguiEdit->GetItemFromId(GUI_ID_REALMXROTATE);
		RGuiItem*	pguiScale3d		= pguiEdit->GetItemFromId(GUI_ID_3DSCALE);
		RGuiItem*	pguiShadowAngle		= pguiEdit->GetItemFromId(GUI_ID_SHADOW_ANGLE);
		RGuiItem*	pguiShadowLength		= pguiEdit->GetItemFromId(GUI_ID_SHADOW_LENGTH);
		RGuiItem*	pguiShadowIntensity	= pguiEdit->GetItemFromId(GUI_ID_SHADOW_INTENSITY);
		RGuiItem*	pguiKillsPercentGoal	= pguiEdit->GetItemFromId(GUI_ID_KILLSPERCENTGOAL);
		RGuiItem*	pguiScaleHeights		= pguiEdit->GetItemFromId(GUI_ID_SCALEATTRIBHEIGHTS);
		RGuiItem*	pgui2dResPaths			= pguiEdit->GetItemFromId(GUI_ID_2DRESPATHS);
		RGuiItem*	pguiBrowse				= pguiEdit->GetItemFromId(GUI_ID_BROWSE);
	
		if (pguiBaseName != NULL)
			{
			// Set text to start with current base name.
			pguiBaseName->SetText("%s", m_acBaseName);

			// Position caret at end of line.
			((REdit*)pguiBaseName)->m_sCaretPos	= strlen(pguiBaseName->m_szText);
			// Recompose with new text.
			pguiBaseName->Compose();

			RSP_SAFE_GUI_REF_VOID(pguiSceneRotX, SetText("%d", m_sSceneRotX) );
			RSP_SAFE_GUI_REF((REdit*)pguiSceneRotX, m_sCaretPos = strlen(pguiSceneRotX->m_szText) );
			RSP_SAFE_GUI_REF_VOID(pguiSceneRotX, Compose() );

			RSP_SAFE_GUI_REF_VOID(pguiRealmRotX, SetText("%d", m_sRealmRotX) );
			RSP_SAFE_GUI_REF((REdit*)pguiRealmRotX, m_sCaretPos = strlen(pguiRealmRotX->m_szText) );
			RSP_SAFE_GUI_REF_VOID(pguiRealmRotX, Compose() );

			RSP_SAFE_GUI_REF_VOID(pguiScale3d, SetText("%s", szScale3d) );
			RSP_SAFE_GUI_REF((REdit*)pguiScale3d, m_sCaretPos = strlen(pguiScale3d->m_szText) );
			RSP_SAFE_GUI_REF_VOID(pguiScale3d, Compose() );

			RSP_SAFE_GUI_REF_VOID(pguiShadowAngle, SetText("%d", m_sShadowAngle) );
			RSP_SAFE_GUI_REF((REdit*)pguiShadowAngle, m_sCaretPos = strlen(pguiShadowAngle->m_szText) );
			RSP_SAFE_GUI_REF_VOID(pguiShadowAngle, Compose() );

			RSP_SAFE_GUI_REF_VOID(pguiShadowLength, SetText("%s", szShadowLength) );
			RSP_SAFE_GUI_REF((REdit*)pguiShadowLength, m_sCaretPos = strlen(pguiShadowLength->m_szText) );
			RSP_SAFE_GUI_REF_VOID(pguiShadowLength, Compose() );

			RSP_SAFE_GUI_REF_VOID(pguiShadowIntensity, SetText("%d", m_sShadowIntensity) );
			RSP_SAFE_GUI_REF((REdit*)pguiShadowIntensity, m_sCaretPos = strlen(pguiShadowIntensity->m_szText) );
			RSP_SAFE_GUI_REF_VOID(pguiShadowIntensity, Compose() );

			RSP_SAFE_GUI_REF_VOID(pguiKillsPercentGoal, SetText("%g", m_pRealm->m_dKillsPercentGoal) );
			RSP_SAFE_GUI_REF_VOID(pguiKillsPercentGoal, Compose() );

			ASSERT(pguiScaleHeights->m_type == RGuiItem::MultiBtn);
			RSP_SAFE_GUI_REF((RMultiBtn*)pguiScaleHeights,	m_sState = (m_sScaleAttribHeights == FALSE) ? 1 : 2);
			RSP_SAFE_GUI_REF_VOID(pguiScaleHeights, Compose() );

			RSP_SAFE_GUI_REF(pguiBrowse, m_bcUser = BrowseBtnUp);
			RSP_SAFE_GUI_REF(pguiBrowse, m_ulUserInstance = (ULONG)pguiBaseName);

			switch (pgui2dResPaths->m_type)
				{
				case RGuiItem::MultiBtn:
					RSP_SAFE_GUI_REF( (RMultiBtn*)pgui2dResPaths, m_sState = m_pRealm->m_s2dResPathIndex + 1);
					RSP_SAFE_GUI_REF_VOID(pgui2dResPaths, Compose() );
					break;
				case RGuiItem::ListBox:
					{
					RListBox*	plb	=  (RListBox*)pgui2dResPaths;

					short	sIndex;
					for (sIndex = 0; sIndex < CRealm::Num2dPaths; sIndex++)
						{
						RGuiItem*	pgui	= plb->AddString(CRealm::ms_apsz2dResPaths[sIndex]);
						if (pgui)
							{
							// Set ID to identify proper index.
							pgui->m_ulUserData	= sIndex;
							// If this is the current one . . .
							if (sIndex == m_pRealm->m_s2dResPathIndex)
								{
								// Select it.
								plb->SetSel(pgui);
								}
							}
						}

					// Repaginate now.
					plb->AdjustContents();

					break;
					}
				default:
					TRACE("EditNew():  GUI %d missing!!\n", GUI_ID_2DRESPATHS);
					break;
				}

			if (DoGui(pguiEdit) == GUI_ID_OK)
				{
				pguiBaseName->GetText(m_acBaseName, sizeof(m_acBaseName) );

				m_sSceneRotX	= RSP_SAFE_GUI_REF(pguiSceneRotX, GetVal() );

				m_sRealmRotX	= RSP_SAFE_GUI_REF(pguiRealmRotX, GetVal() );

				m_sShadowAngle	= RSP_SAFE_GUI_REF(pguiShadowAngle, GetVal() );

				m_sShadowIntensity = RSP_SAFE_GUI_REF(pguiShadowIntensity, GetVal() );

				pguiScale3d->GetText(szScale3d, sizeof(szScale3d));

				m_dScale3d	= atof(szScale3d);
				if (m_dScale3d < 0.2) m_dScale3d = 0.2;	// some safety
				if (m_dScale3d > 5.0) m_dScale3d = 5.0; // some safety

				pguiShadowLength->GetText(szShadowLength, sizeof(szShadowLength));

				m_dShadowLength	= atof(szShadowLength);
				if (m_dShadowLength < 0.0) m_dShadowLength = 0.2;	// some safety
				if (m_dShadowLength > 1.5) m_dShadowLength = 1.5; // some safety

				if (RSP_SAFE_GUI_REF((RMultiBtn*)pguiScaleHeights, m_sState) == 1)
					{
					m_sScaleAttribHeights	= FALSE;
					}
				else
					{
					m_sScaleAttribHeights	= TRUE;
					}

				switch (pgui2dResPaths->m_type)
					{
					case RGuiItem::MultiBtn:
						m_pRealm->m_s2dResPathIndex	= RSP_SAFE_GUI_REF((RMultiBtn*)pgui2dResPaths, m_sState) - 1;
						break;
					case RGuiItem::ListBox:
						{
						RGuiItem*	pguiSel	= ( (RListBox*)pgui2dResPaths)->GetSel();
						if (pguiSel)
							{
							m_pRealm->m_s2dResPathIndex	= pguiSel->m_ulUserData;
							}
						break;
						}
					}

				if (pguiKillsPercentGoal != NULL)
					{
					m_pRealm->m_dKillsPercentGoal	= strtod(pguiKillsPercentGoal->m_szText, NULL);
					}

				// Init the hood
				sResult = Init();
				if (sResult == 0)
					{
					// Start it.
					Startup();
					}
				}
			else
				{
				// User abort.
				sResult	= 1;
				}
			}
		else
			{
			TRACE("EditNew(): No GUI with ID %ld.\n", GUI_ID_BASENAME);
			sResult	= -2;
			}

		// Delete GUI.
		delete pguiEdit;
		}
	else
		{
		TRACE("EditNew(): Failed to load GUI \"%s\".\n", GUI_FILE_NAME);
		sResult	= -1;
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CHood::EditModify(void)
	{
	return EditNew(0, 0, 0);
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CHood::EditMove(									// Returns 0 if successfull, non-zero otherwise
	short /*sX*/,											// In:  New x coord
	short /*sY*/,											// In:  New y coord
	short /*sZ*/)											// In:  New z coord
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CHood::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CHood::EditRender(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Init the hood
////////////////////////////////////////////////////////////////////////////////
short CHood::Init(void)									// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// If first call . . .
	if (m_sNumInits++ == 0)
		{
		// Get resources
		sResult = GetResources();
		if (sResult == 0)
			{
			// Set the realm's pointer to point at our attribute map.  This is a
			// shortcut to make it easier and faster for objects to access the map.
			m_pRealm->m_pTerrainMap = m_pTerrainMap;
			m_pRealm->m_pLayerMap = m_pLayerMap;

			// Background is only thing on rear-most layer
			CSprite2* pSprite2 = new CSprite2;
			pSprite2->m_sX2 = 0;
			pSprite2->m_sY2 = 0;
			pSprite2->m_sPriority = 0;
			pSprite2->m_pImage = m_pimBackground;
			pSprite2->m_sLayer = CRealm::LayerBg;
			pSprite2->m_sInFlags = CSprite::InDeleteOnClear | CSprite::InBlitOpaque;
			m_pRealm->m_scene.UpdateSprite(pSprite2);

			// Attempt to load all layers . . .
			long	lIndex;
			// Put the contents of the various spry's onto the other layers
			for (lIndex	= 0; lIndex < MaxLayers; lIndex++)
				{
				// If this layer exists . . .
				if (m_apspryAlphas[lIndex] != NULL)
					{
					RSpry::ListOfSprites::Pointer p = m_apspryAlphas[lIndex]->m_listSprites.GetHead();
					while (p)
						{
						RSprite* pSprite = m_apspryAlphas[lIndex]->m_listSprites.GetData(p);

						CSprite2* pSprite2 = new CSprite2;
						pSprite2->m_sX2 = pSprite->m_sX;
						pSprite2->m_sY2 = pSprite->m_sY;
						pSprite2->m_sPriority = pSprite->m_sZ;
						pSprite2->m_pImage = pSprite->m_pImage;
						pSprite2->m_sLayer = CRealm::LayerAlpha1 + (short)lIndex * (CRealm::LayerAlpha2 - CRealm::LayerAlpha1);
						pSprite2->m_sInFlags = CSprite::InDeleteOnClear | CSprite::InAlpha;
						m_pRealm->m_scene.UpdateSprite(pSprite2);

						p = m_apspryAlphas[lIndex]->m_listSprites.GetNext(p);
						}
					}

				// If this layer exists . . .
				if (m_apspryOpaques[lIndex] != NULL)
					{
					RSpry::ListOfSprites::Pointer p = m_apspryOpaques[lIndex]->m_listSprites.GetHead();
					while (p)
						{
						RSprite* pSprite = m_apspryOpaques[lIndex]->m_listSprites.GetData(p);

						CSprite2* pSprite2 = new CSprite2;
						pSprite2->m_sX2 = pSprite->m_sX;
						pSprite2->m_sY2 = pSprite->m_sY;
						pSprite2->m_sPriority = pSprite->m_sZ;
						pSprite2->m_pImage = pSprite->m_pImage;
						pSprite2->m_sLayer = CRealm::LayerOpaque1 + (short)lIndex * (CRealm::LayerOpaque2 - CRealm::LayerOpaque1);
						pSprite2->m_sInFlags = CSprite::InDeleteOnClear | CSprite::InOpaque;
						m_pRealm->m_scene.UpdateSprite(pSprite2);
						
						p = m_apspryOpaques[lIndex]->m_listSprites.GetNext(p);
						}
					}
				}
			}

		// Allocate the Smashatorium:
		#ifdef NEW_SMASH

		// This requires success in the previous ops . . .
		if (sResult == 0)
			{
			// Jon has PROMISED me that there is an m_pRealm which is ACCURATE.
			#define MAX_SMASHEE_W 72 // 40	once we deal with fat objects
			#define MAX_SMASHEE_H 72 // 40	once we deal with fat objects
			
			// Allocate the Smashatorium:  Pick tile size greater than any normal object radius...
			m_pRealm->m_smashatorium.Destroy();
			m_pRealm->m_smashatorium.Alloc(m_pRealm->GetRealmWidth(),m_pRealm->GetRealmHeight(),
				short(MAX_SMASHEE_W * m_dScale3d),  // I'm assuming this scales with size
				short(MAX_SMASHEE_H * m_dScale3d) );
			}

		#endif
		}
 
	// If successful so far . . .
	if (sResult == 0)
		{
		SetupPipeline();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Setup pipeline to hood's specifications.
////////////////////////////////////////////////////////////////////////////////
void CHood::SetupPipeline(void)	// Returns nothing.
	{
	// Some extra safety (to avoid crashing)
	if (m_dScale3d < 0.2) m_dScale3d = 0.2;
	if (m_dScale3d > 5.0) m_dScale3d = 5.0;

	// Set up world transformation matrix with CHood's little tweak on view
	// angle:

	// Create scene transform with our tweak.
	RTransform	transScene;	// Identity.
	transScene.Rx(m_sSceneRotX);
	// Create transform to convert from scene to realm.
	RTransform	transScene2Realm;	// Identity.
	transScene2Realm.Rx(m_sRealmRotX - m_sSceneRotX);
	// Re-setup with tweakage and scaling.
	m_pRealm->m_scene.SetupPipeline(&transScene, &transScene2Realm, m_dScale3d);
	}

////////////////////////////////////////////////////////////////////////////////
// Kill the hood
////////////////////////////////////////////////////////////////////////////////
short CHood::Kill(void)									// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;
	
	// Free resources
	sResult = FreeResources();

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Helper to load and convert SPRYs.
////////////////////////////////////////////////////////////////////////////////
inline
short SpryLoadConv(			// Returns 0 on success.
	RResMgr*	presmgr,			// In:  ResMgr to load from.
	RSpry**	ppspry,			// Out: Ptr to SPRY resource.
	char*		pszFileName,	// In:  File/Res name of .SAY file.
	RImage::Type	type)		// In:  Destination type.
	{
	short	sRes	= 0;	// Assume success.

	if (rspGetResource(
		presmgr,
		pszFileName,
		ppspry) == 0)
		{
		// If conversion specified . . .
		if (type != RImage::NOT_SUPPORTED)
			{
			// Convert . . .
			if ((*ppspry)->Convert(type) == 0)
				{
				// Success.
				}
			else
				{
				TRACE("SpryLoadConv(): (*ppspry)->Convert(type) failed.\n");
				sRes	= -3;
				}
			}
		}
	else
		{
//		TRACE("SpryLoadConv(): Failed to load SPRY \"%s\".\n", pszFileName);
		sRes	= -1;
		}

	return sRes;
	}

extern int wideScreenWidth;


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CHood::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// If all resources were already successfully loaded, then don't do this again.
	// Note that if only some of them loaded and then an error occurred, we'll
	// end up reloading all of them, even those that loaded okay.
	if (!m_bResourcesExist)
		{
		// Free any resources that might have been loaded by a previous (unsuccessfull) attempt
		FreeResources();

		char	szFileName[RSP_MAX_PATH];			// Temp storage to create filenames.
		char	szBasePath[RSP_MAX_PATH];			// Temp storage of file path.
		
		// Create the real base file path.
		sprintf(szBasePath, "hoods/%s/%s", m_acBaseName, m_acBaseName);

		// Add .ext for SAKs.
		sprintf(szFileName, "res/hoods/%s.sak", m_acBaseName);

		// Open SAK, if available . . .
		if (m_pRealm->m_resmgr.OpenSak(FullPathHD(szFileName)) == 0)
			{
			// Using SAK from HD.
			}
		else
			{
			if (m_pRealm->m_resmgr.OpenSak(FullPathHoods(szFileName)) == 0)
				{
				// Using SAK from hoods path.
				}
			else
				{
				// Using Disk, set path.
				m_pRealm->m_resmgr.SetBasePath(g_GameSettings.m_szNoSakDir);
				}
			}

		// Background filename.
		sprintf(szFileName, "%s.bmp", szBasePath);

		// Load background
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimBackground) != 0)
			{
			sResult = -1;
			TRACE("CHood::GetResources(): Couldn't load background: %s\n", szFileName);
			goto Error;
			}

		// Attempt to load all layers . . .
		long	lIndex;
		short	sNumAlphaLayersLoaded	= 0;
		short	sNumOpaqueLayersLoaded	= 0;
		for (lIndex	= 0; lIndex < MaxLayers; lIndex++)
			{
			// Make alpha layer name.
			sprintf(szFileName, "%s%02da.say", szBasePath, lIndex);
			// Load & convert . . .
			if (SpryLoadConv(&(m_pRealm->m_resmgr), m_apspryAlphas + lIndex, szFileName, RImage::FSPR8) == 0)
				{
				sNumAlphaLayersLoaded++;
				}

			// Make opaque layer name.
			sprintf(szFileName, "%s%02do.say", szBasePath, lIndex);
			// Load & convert . . .
			if (SpryLoadConv(&(m_pRealm->m_resmgr), m_apspryOpaques + lIndex, szFileName, RImage::FSPR8) == 0)
				{
				sNumOpaqueLayersLoaded++;
				}
			}

		// If no layers loaded . . .
		if (sNumOpaqueLayersLoaded == 0 && sNumAlphaLayersLoaded == 0)
			{
			TRACE("GetResources(): No alpha or opaque layers successfully loaded.\n");
			}
		else
			{
			TRACE("GetResources(): Successfully loaded %hd Alpha layer%s and %hd Opaque layer%s.\n",
				sNumAlphaLayersLoaded, (sNumAlphaLayersLoaded == 1 ? "" : "s"),
				sNumOpaqueLayersLoaded, (sNumOpaqueLayersLoaded == 1 ? "" : "s"));
			}

		sprintf(szFileName, "%s.mp1", szBasePath);

		// Load new multi layer attribute maps
		if (rspGetResource(
			&(m_pRealm->m_resmgr), 
			szFileName,
			&m_pTerrainMap) != 0)
			{
			sResult = -1;
			TRACE("CHood::GetResources(): Couldn't load attribute map %s\n", szFileName);
			goto Error;
			}

		sprintf(szFileName, "%s.mp2", szBasePath);

		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pLayerMap) != 0)
			{
			sResult = -1;
			TRACE("CHood::GetResources(): Couldn't load attribute map %s\n", szFileName);
			goto Error;
			}
			

		// Make XRay mask name.
		sprintf(szFileName, "%s.XRayMask.bmp", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimXRayMask) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}

		sprintf(szFileName, "%s.Transparency.MultiAlpha", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pmaTransparency) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}

		sprintf(szFileName, "%s.Ambient.alpha", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pltAmbient) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}

		sprintf(szFileName, "%s.Spot.alpha", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pltSpot) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}

		// Load all the assets for the toolbar
		sprintf(szFileName, "%s.emptybar.bmp", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimEmptyBar) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}
		sprintf(szFileName, "%s.emptybarselected.bmp", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimEmptyBarSelected) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}
		sprintf(szFileName, "%s.fullbar.bmp", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimFullBar) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}
		sprintf(szFileName, "%s.fullbarselected.bmp", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimFullBarSelected) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}
		sprintf(szFileName, "%s.topbar.bmp", szBasePath);
		if (rspGetResource(
			&(m_pRealm->m_resmgr),
			szFileName,
			&m_pimTopBar) != 0)
			{
			sResult	= -1;
			TRACE("GetResources(): Failed to load: %s.\n", szFileName);
			}


		RImage * stretched = new RImage();
		stretched->CreateImage(wideScreenWidth,m_pimTopBar->GetHeight(),m_pimTopBar->GetType(),0);

		//Here we create a new streched image for widescreen
		float widthScale = (float)m_pimTopBar->m_sWidth / (float)stretched->m_sWidth;
		for (int n=0;n<m_pimTopBar->m_sHeight;n++)
		{
			for (int x=0;x<stretched->m_sWidth;x++)
			{
				UCHAR* dest = stretched->m_pData + n * stretched->m_lPitch + x;
				UCHAR* src = m_pimTopBar->m_pData + n * m_pimTopBar->m_lPitch + (int)((float)x * widthScale);
				*dest = *src;
			}
			//memcpy(ms_pimCompositeBufferScaled->m_pData + n * ms_pimCompositeBufferScaled->m_lPitch,
			//		ms_pimCompositeBuffer->m_pData + n * ms_pimCompositeBuffer->m_lPitch,ms_pimCompositeBuffer->m_sWidth);
		}
		//TODO free m_pimTopBar
		m_pimTopBar = stretched;


		// Resources now exist
		m_bResourcesExist = true;
		}

Error:

	// Close SAK, if open.
	m_pRealm->m_resmgr.CloseSak();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CHood::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Don't check whether resources exist!  Even if they were only partially
	// loaded (due to an error during the load process) we still want to clear
	// those resources that were loaded.  The stuff being done here is safe
	// regardless of whether or not the resource was actually loaded.
	
	if (m_pimBackground != NULL)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimBackground);

	long	lIndex;
	for (lIndex	= 0; lIndex < MaxLayers; lIndex++)
		{
		if (m_apspryAlphas[lIndex] != NULL)
			rspReleaseResource(&(m_pRealm->m_resmgr), &(m_apspryAlphas[lIndex]));

		if (m_apspryOpaques[lIndex] != NULL)
			rspReleaseResource(&(m_pRealm->m_resmgr), &(m_apspryOpaques[lIndex]));
		}

	if (m_pTerrainMap != NULL)
		{
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pTerrainMap);
		// Clear Realm's map ptr
		m_pRealm->m_pTerrainMap = NULL;
		}

	if (m_pLayerMap != NULL)
		{
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pLayerMap);
		// Clear Realm's map ptr
		m_pRealm->m_pLayerMap = NULL;
		}

	if (m_pimXRayMask != NULL)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimXRayMask);

	if (m_pmaTransparency != NULL)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pmaTransparency);

	if (m_pltAmbient != NULL)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pltAmbient);

	if (m_pltSpot != NULL)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pltSpot);

	if (m_pimEmptyBar)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimEmptyBar);
	if (m_pimEmptyBarSelected)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimEmptyBarSelected);
	if (m_pimFullBar)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimFullBar);
	if (m_pimFullBarSelected)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimFullBarSelected);
	if (m_pimTopBar)
		rspReleaseResource(&(m_pRealm->m_resmgr), &m_pimTopBar);

	// Resources no longer exist for sure.
	m_bResourcesExist = false;

	// Let's get these out of memory right away.
	m_pRealm->m_resmgr.Purge();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Set the hood's palette.
////////////////////////////////////////////////////////////////////////////////
void CHood::SetPalette(void)	// Returns nothing.
	{
	ASSERT(m_pimBackground->m_pPalette != NULL);
	ASSERT(m_pimBackground->m_pPalette->m_type == RPal::PDIB);
	
	rspSetPaletteEntries(
		0, //10,
		256, // 236,
		m_pimBackground->m_pPalette->Red(0), //10
		m_pimBackground->m_pPalette->Green(0), //10
		m_pimBackground->m_pPalette->Blue(0), //10
		m_pimBackground->m_pPalette->m_sPalEntrySize);

	// This part could be left up to the caller...?  should it be?
	rspUpdatePalette();
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
