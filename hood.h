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
// hood.h
// Project: Nostril (aka Postal)
//
//	History:
//		01/22/97	JMI	Converted many m_spryAlphas/Opaques to arrays and added
//							a macro enum MaxLayers.
//		
//		01/23/97	JMI	Added two very non-major comments and removed a #if 0
//							block.
//		
//		01/26/97	JMI	Made m_imBackground public.  It was the only way I could
//							figure to get any idea of the realm dimensions that I
//							needed for the editor.
//		
//		01/26/97	JMI	Added override of EditRect() that sets the rect to the
//							dimensions of m_pimBackground->
//		
//		01/31/97 MJR	Added GetWidth() and GetHeight().
//		
//		02/03/97	JMI	Updated default relative path used to get hood resources.
//		
//		02/04/97	JMI	Changed all resources to pointers so we can fully utilize
//							the RResMgr.
//		
//		02/07/97	JMI	Added m_sWorldXRot, world transformation x rotation.
//
//		02/13/97	JMI	Changed paths for hoods to be in hoods/ instead of bg/
//							and also now just store the import part (like "city" instead
//							of "hoods/city").
//							Also, now gets resources for lighting effects.
//							Also, now sets the realm's m_phood ptr to this instance.
//
//		02/13/97	JMI	Had to make m_pimBackground public so 3D guys could get in-
//							to its palette.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/09/97 BRH	Added RMultiGrid to the hoods for the new type of attribute
//							maps.  These will eventuall replace the RAttribMap but for
//							now they are both here so that all parts of the game
//							will continue to work during testing of the new attribute
//							map.
//							
//		05/29/97	JMI	Changed occurences of m_pHeightMap to m_pTerrainMap.
//							Changed occurences of m_pAttrMap to m_pLayerMap.
//							Also, removed occurences of m_pattribMap.
//
//		06/16/97	JMI	Added SetPalette() which can be used to set the hood's
//							palette at the convenience of the Realm runner.
//
//		06/26/97	JMI	Added read-only access to m_sWorldXRot, GetWorldRotX().
//
//		06/28/97	JMI	Changed m_sWorldXRot to m_sRealmRotX and GetWorldRotX() and
//							GetRealmRotX(), and added m_sSceneRotX & GetSceneRotX().
//
//		07/01/97	JMI	Added m_sNumInits.  Some things in Init() should only be
//							done once (like getting the resources, adding them to
//							the scene, and allocating the smashatorium).
//
//		07/01/97	JMI	Added m_sScaleAttribHeights indicating whether to scale
//							height values gotten via the terrain map.
//
//		07/09/97	JMI	Added setting for which side view assets to use, 
//							m_s2dResPathIndex.  For now, only 0 or 1 are valid, but
//							eventually, it may be more of an index than a bool, and,
//							hence the name.
//
//		07/09/97	JMI	Moved m_s2dResPathIndex from CHood to CRealm b/c of order
//							issues when loading.
//
//
//		08/03/97	JRD	Added ability to save 3d scale with hood
//
//
//		08/13/97	JRD	Began adding Randy's numbers to the toolbar...
//
//		08/17/97	JMI	Added handy macro for updating hood's settings to scene's
//							pipeline.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef HOOD_H
#define HOOD_H

#include "RSPiX.h"
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/Spry/spry.h"
	#include "ORANGE/MultiGrid/MultiGrid.h"
#else
	#include "spry.h"
	#include "multigrid.h"
#endif
#include "thing.h"


// Let's just know that the Realm exists.
class CRealm;

// This is the hood object
class CHood : public CThing
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:
		enum	// Macros.
			{
			MaxLayers			= 8	// Maximum number of alpha & opaque layers.
			};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		
		RMultiGrid*		m_pTerrainMap;							// Height layer for attribute map
		RMultiGrid*		m_pLayerMap;							// Attribute bits layer for attribue map.

		RImage*			m_pimXRayMask;							// Mask for XRayable area.
		RMultiAlpha*	m_pmaTransparency;					// Alpha tables for XRay and fire FX.

		RAlpha*			m_pltAmbient;							// Ambient lighting effect.
		RAlpha*			m_pltSpot;								// Spot lighting effect.

		RImage*			m_pimBackground;						// Main background image

		RImage*			m_pimEmptyBar;							// Bar with nothing
		RImage*			m_pimEmptyBarSelected;				// Empty with highlights
		RImage*			m_pimFullBar;							// Weap & Ammo present
		RImage*			m_pimFullBarSelected;				// Full with highlights
		RImage*			m_pimTopBar;							// Top bar

		RImage*			m_pimNum;								// Randy raster numbers (0-9)
		RImage*			m_pimNumLite;							// PowerUp
		RImage*			m_pimNumLow;							// Ammo Low
		RImage*			m_pimNumGone;							// Ammo Gone


		short				m_sScaleAttribHeights;				// TRUE, to enable scaling of attribute
																		// map heights via the realm view angle.
																		// FALSE, to use naked values.
		double			m_dScale3d;								// defaults to 1.0.  smash is affected
		short				m_sShadowAngle;						// 0-259
		double			m_dShadowLength;						// if 0, no shadows
		short				m_sShadowIntensity;					// if 0, no shadows


	protected:

		RSpry*	m_apspryAlphas[MaxLayers];					// "Alpha" layers of sprites.
		RSpry*	m_apspryOpaques[MaxLayers];				// "Opaque" layeres of sprites.

		bool m_bResourcesExist;									// Flags whether resources exist

		char m_acBaseName[RSP_MAX_PATH];						// Base name for all resources.

		short	m_sRealmRotX;										// Realm X rotation.
		short	m_sSceneRotX;										// Scene transform X rotation.

		short	m_sNumInits;										// Number of times Init() has
																		// been called (some things
																		// are only done on the first
																		// call).

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CHood(CRealm* pRealm);

	public:
		// Destructor
		~CHood();

	//---------------------------------------------------------------------------
	// Required static functions
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
			CThing** ppNew)										// Out: Pointer to new object
			{
			short sResult = 0;
			*ppNew = new CHood(pRealm);
			if (*ppNew == 0)
				{
				sResult = -1;
				TRACE("CHood::Construct(): Couldn't construct CHood!\n");
				}
			return sResult;
			}

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Called by editor to get the clickable pos/area of an object.
		virtual	// If you override this, do NOT call this base class.
		void EditRect(				// Returns nothiing.
			RRect*	prc)			// Out: Clickable pos/area of object.
			{
			// Default implementation makes the object unclickable.
			prc->sX	= 0;
			prc->sY	= 0;
			prc->sW	= m_pimBackground->m_sWidth;
			prc->sH	= m_pimBackground->m_sHeight;
			}

	//---------------------------------------------------------------------------
	// Hood-specific functions
	//---------------------------------------------------------------------------
	public:
		// Get width
		short GetWidth(void)
			{ return m_pimBackground->m_sWidth; }

		// Get height
		short GetHeight(void)
			{ return m_pimBackground->m_sHeight; }

		// Get realm X rotation.
		short GetRealmRotX(void)
			{ return m_sRealmRotX; }

		// Set the hood's palette.
		void SetPalette(void);	// Returns nothing.

		// Setup pipeline to hood's specifications.
		void SetupPipeline(void);	// Returns nothing.

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Init the hood
		short Init(void);											// Returns 0 if successfull, non-zero otherwise

		// Kill the hood
		short Kill(void);											// Returns 0 if successfull, non-zero otherwise

		// Load object
		short LoadThis(											// Returns 0 if successfull, non-zero otherwise
			RFile* pFile);											// In:  File to load from

		// Get all required resources
		short GetResources(void);								// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		short FreeResources(void);								// Returns 0 if successfull, non-zero otherwise
	};


#endif //HOOD_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
