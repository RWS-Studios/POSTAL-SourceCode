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
// scene.h
// Project: Nostril (aka Postal)
//
//	History:
//		01/28/97	JMI	Made the m_layers member public.
//
//		02/06/97	JMI	Filled out the CSprite3 class.
//
//		02/07/97	JMI	Added m_transWorld transform to affect all 3D objects
//							rendered via the CScene.
//
//		02/07/97	JMI	Added alphable sprite.
//
//		02/07/97	JMI	Moved m_sX2 and m_sY2 that all three types of sprites
//							contained, to the base class.
//
//		02/07/97	JMI	Now the scene has its own pipeline and CSprite3 does not.
//							B/c of this we no longer needed the m_transWorld, b/c we
//							could incorporate it right into the view.
//							Also, removed m_ppipe from CSprite3 and added m_sRadius
//							so the X Ray logic could do correct collision between
//							the X Rayee and X Rayables.
//							Also, added a function to initialize/create the pipeline.
//
//		02/10/97	JMI	Added support for child objects of 3D sprites (CSprite3's).
//
//		02/13/97	JMI	Changing RForm3d to RSop.
//
//		02/13/97	JMI	Render() now uses the hood passed to it to get the lighting
//							effects.  Render() now takes a CHood* as a parm.
//
//		02/14/97	JMI	CSprite3 now contains an m_psphere ptr which must be set
//							to the bounding sphere data for the CSprite3 frame (x, y, 
//							z, w).
//							Also, added TransformPts() function which will map an array
//							of pts through the optionally provided transform and then
//							through the scenes transforms.
//
//		02/14/97	JMI	Added m_transNoZ which is a transform that is equivalent
//							to the scene's pipeline (m_pipeline), but does not scale
//							Z to the maximum value.  It instead scales Z the same
//							as X and Y.
//							Also, added InFlag InHighIntensity indicating that the
//							object should be rendered/BLiT'ed with higher intensity
//							lighting.
//							Also, added m_sBrightness to CSprite3 indicating the
//							center point for the fog effect on the 3D object when
//							rendered.
//							Also, added TransformPtsToRealm() which translate points
//							through the specified transform, the scene's pipeline's 
//							transforms, and finally maps them to Realm 3D coordinates.
//							
//		02/17/97	JMI	Added CSprite3::m_sCenX and CSprite3::m_sCenY indicating 
//							center of circle that's radius was indicated by 
//							CSprite3::m_sRadius.
//
//		02/20/97 JRD	Added global lighting control and parameters to scene.
//
//		02/23/97	JMI	Now CSprite3 intializes m_psphere to NULL in its 
//							constructor and m_sBrightness was being set to 128
//							which used to be the middle range of light but now,
//							as an offset, the middle is 0 (negative is decreased
//							brightness and positive is increased brightness).
//							
//		02/24/97	JMI	Collapsed CAlphaSprite2 into CSprite2 and added 
//							m_sAlphaLevel so a constant alpha level can be specified.
//							Using a constant alpha level is faster than using a mask.
//							Also, made it so the m_type field of the various sprites
//							are set automatically on construction so YOU SHOULD NO
//							LONGER SET THE CSprite*::m_type!
//
//		02/26/97	JMI	Added m_pthing to CSprite class for debugging purposes.
//
//		03/05/97	JMI	Added Render() function to render a 3D sprite into a
//							specified background.
//
//		03/21/97	JMI	Added InDeleteOnRender flag and made CSprite base class
//							destructor virtual so, when objects are deleted as general
//							CSprites, all their destructors will be called (even though
//							none do anything yet, they may someday).
//							Also, added a CSpriteLine2d.
//
//		04/29/97	JMI	Made the child concept part of the base class sprite (was
//							part of the 3D sprite (CSprite3)) and added a parent 
//							pointer.
//							Now that we have the parent sprite pointer, I was able to
//							make sure the sprites remove themselves from their parents
//							and remove their children when they are destroyed.
//							Also, added member functions to CScene to handle rendering
//							2D sprites, 3D sprites, and 2D lines.
//
//		04/29/97	JMI	Added GetType() to CSprite to allow read-only access to
//							its m_type member.
//
//		05/02/97	JMI	Added CSpriteCylinder3d, a cylinder primitive.
//
//		05/08/97	JMI	If VC < 4.2, we were including <multiset>.  Changed <multiset> to 
//							<multiset.h>
//
//		05/12/97	JMI	Added m_sRenderX, m_sRenderY, m_sRenderOffX, m_sRenderOffY 
//							which is used by Render3D() to store at what offset the 3D 
//							item was rendered.
//
//		05/15/97	JMI	I had thought that the max alpha (i.e., opaque) level was 
//							100 and was, therefore, checking to see if the level was
//							less than 100 before calling the alpha blit.  As it turns
//							out, it is 255.  Fixed.
//
//		05/17/97	JMI	Moved class CLayer out of CScene.
//
//		05/19/97	JMI	Added m_pszText to basic CSprite.
//							Also, added ms_print to CScene to print m_pszText.
//
//		06/09/97	JMI	RemoveChild() was not moving the pointer to the previous
//							child's next.
//
//		06/14/97 MJR	Added g_bSceneDontBlit to replace the flag in gamesettings
//							that was being misused for that purpose.
//
//		06/16/97	JMI	Added InBlitOpaque flag for opaque 2D sprites.
//
//		06/25/97	JMI	Added SetXRayAll() and m_bXRayAll which enable or disable 
//							xraying of all 'alpha' _and_ 'opaque' layers.
//							Also, changed InXrayable to InAlpha and added InOpaque to
//							differentiate between these two types which are now, at 
//							times, both xrayable.
//
//		06/26/97	JMI	Added phood parm to RenderCylinder3D().
//
//		06/27/97	JMI	Moved CSprite and derived sprite classes from here to 
//							sprites.h
//
//		06/29/97 MJR	Converted vector of layers into simple array of layers
//							(motivated by trying to get it to work on the mac).
//
//		07/01/97	JMI	Added m_transScene2Realm which is used in 
//							TransformPtsToRealm() to convert from the scene to the 
//							realm.  This value is set via the SetupPipeline() call.
//
//		08/07/97	JMI	Moved global single instance of gdSCALE3D defined in 
//							scene.cpp into scene as an instantiable member, m_dScale3d.
//
//		08/18/97	JMI	Changed Render() used by dead things to update themselves
//							into the background to be called DeadRender3D() to better
//							describe what it does and differentiate it from the other
//							more generic Render()ers.  Also, added two parameters so
//							it could call itself with 3D children.
//
//		10/03/99	JMI	Changed Render3D() to take a light scheme instead of a hood
//							to make it more general.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SCENE_H
#define SCENE_H

#if _MSC_VER >= 1020 || __MWERKS__ >= 0x1100 || __GNUC__
	#include <set>
#else
	#include <multiset.h>
#endif

#include "RSPiX.h"
#include "hood.h"
#include "sprites.h"


////////////////////////////////////////////////////////////////////////////////
// HERE ARE GLOBAL LIGHTING MACROS FOR YOUR ENJOYMENT!
// They are nolonger constant, as the user may adjust them.
//
// NOTE: ONLY THE MENU MAY ADJUST THESE VALUES
// 3d objects may need to use gsGlobalBrightnessPerLightAttribute for calculation
//
extern	short	gsGlobalBrightnessPerLightAttribute; 
extern	short gsGlobalLightingAdjustment;
//
////////////////////////////////////////////////////////////////////////////////

// Used to control whether or not scene will actually blit.  When set to true,
// scene will execute quite a bit faster, except you won't see anything. :)
extern	bool	g_bSceneDontBlit;

// Define a layer, which is a sorted collection of sprites (this must be
// a class so that the member object's constructor gets called!)
class Layer
	{
	public:
		msetSprites m_sprites;							// Sprites in this layer
		bool m_bHidden;									// Whether this layer is hidden

	Layer()
		{
		m_bHidden = false;
		}

	~Layer()
		{
		}
	};

// A CScene object consists of any number of layers, each of which contains any
// number of CSprites.  It handles both 2d and 3d sprites, and does everything
// necessary to render a view of the scene.
class CScene
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		// STATIC! Used to draw within the scene.
		static RPrint	ms_print;

		// Array of layers
		Layer*		m_pLayers;
		short			m_sNumLayers;
		
		// The world pipeline.  All 3D objects are rendered via this pipeline.
		// We might eventually have more than one.
		RPipeLine	m_pipeline;

		//	This transform is equivalent to the scene's pipeline (m_pipeline), but
		// does not scale Z to the maximum value.  It instead scales Z the same
		//	as X and Y.
		RTransform	m_transNoZView;
		RTransform	m_transNoZScreen;

		// When true, all 'opaque' layers will be alpha'ed in the way 'alpha' layers
		// are.  But not when false; duh!
		bool			m_bXRayAll;

		// The transform for converting from the scene to the realm.  
		// Used in TransformPtsToRealm().
		RTransform	m_transScene2Realm;

		// The global scaling factor.  This is used to tune our model ratio so we
		// can scale 3D objects differently on a per realm basis.
		double		m_dScale3d;

	//---------------------------------------------------------------------------
	// Functions
	//---------------------------------------------------------------------------
	public:
		// Default (and only) constructor
		CScene();

		// Destructor
		~CScene();

		// Clear the scene (clears out all layers and sprites)
		void Clear(void);

		// Remove all sprites (from all layers)
		void RemoveAllSprites(void);

		// Remove sprites from specified layer
		void RemoveSprites(
			short sLayer);											// In:  Layer number (0 to n-1)

		// Set number of layers (destroys any existing layers!)
		void SetLayers(											// Returns 0 if successfull, non-zero otherwise
			short sNumLayers);									// In:  Number of layers

		// Update existing sprite or add new sprite
		void UpdateSprite(
			CSprite* pSprite);									// In:  Sprite to add

		// Remove sprite (safe to call even if sprite isn't in scene).
		void RemoveSprite(
			CSprite* pSprite);									// In:  Sprite to remove

		// Render specified area of scene into specified image
		void Render(
			short sSrcX,											// In:  Source (scene) x coord
			short sSrcY,											// In:  Source (scene) y coord
			short sW,												// In:  Width
			short sH,												// In:  Height
			RImage* pimDst,										// In:  Destination image
			short sDstX,											// In:  Destination (image) x coord
			short sDstY,											// In:  Destination (image) y coord
			CHood* phood);											// In:  The hood involved.

		// Render a single sprite tree.
		void Render(						// Returns nothing.
			RImage*		pimDst,			// Destination image.
			short			sDstX,			// Destination 2D x coord.
			short			sDstY,			// Destination 2D y coord.
			CSprite*		pSprite,			// Tree of sprites to render.
			CHood*		phood,			// Da hood, homey.
			RRect*		prcDstClip,		// Dst clip rect.
			CSprite*		psXRayee);		// XRayee, if not NULL.

		// Render a 2D sprite.
		void Render2D(					// Returns nothing.
			RImage*		pimDst,		// Destination image.
			short			sDstX,		// Destination 2D x coord.
			short			sDstY,		// Destination 2D y coord.
			CSprite2*	ps2Cur,		// Tree of sprites to render.
			CHood*		phood,		// Da hood, homey.
			RRect*		prcDstClip,	// Dst clip rect.
			CSprite*		psXRayee);	// XRayee, if not NULL.

		// Draw a 2D line.
		void Line2D(							// Returns nothing.
			RImage*			pimDst,			// Destination image.
			short				sDstX,			// Destination 2D x coord.
			short				sDstY,			// Destination 2D y coord.
			CSpriteLine2d*	psl2,				// Tree of sprites to render.
			RRect*			prcDstClip);	// Dst clip rect.

		// Draw a 3D (as if there were another kind) cylinder.
		void RenderCylinder3D(						// Returns nothing.
			RImage*					pimDst,			// Destination image.
			short						sDstX,			// Destination 2D x coord.
			short						sDstY,			// Destination 2D y coord.
			CSpriteCylinder3d*	psc3,				// Cylinder sprite.
			CHood*					phood,			// Da hood, homey.
			RRect*					prcDstClip);	// Dst clip rect.

		void									// Returns nothing.
		Render3D(
			RImage*		pimDst,			// Destination image.
			short			sDstX,			// Destination 2D x coord.
			short			sDstY,			// Destination 2D y coord.
			CSprite3*	ps3Cur,			// 3D sprite to render.
			RAlpha*		plight,			// Light to render with.
			RRect*		prcDstClip);	// Dst clip rect.

		inline
		void									// Returns nothing.
		Render3D(
			RImage*		pimDst,			// Destination image.
			short			sDstX,			// Destination 2D x coord.
			short			sDstY,			// Destination 2D y coord.
			CSprite3*	ps3Cur,			// 3D sprite to render.
			CHood*		phood,			// Da hood, homey.
			RRect*		prcDstClip);	// Dst clip rect.

		// Setup render pipeline.  Use this function to setup or alter the pipeline.
		// This function DOES a Make1() and then multiplies by the supplied transform,
		// if any.  Any transforms that need to be applied after this setup can be
		// done following a call to this function.
		// The only sux is you cannot insert yourself into the middle of this function.
		// If that's what you need, you should probably just do the entire setup for
		// the pipe yourself or add a similar function to this one.
		void SetupPipeline(								// Returns nothing.
			RTransform* ptransScene = NULL,			// In:  Transform to apply before doing defaults.
			RTransform* ptransScene2Realm = NULL,	// In:  Transform to convert from scene to realm.
			double		dScale3d = 0.0)				// In:  New scaling to apply to pipeline (see
																// m_dScale3d declaration).
			;

		// Transform the given points through the CScene's pipeline with the
		// supplied transform.
		void TransformPts(		// Returns nothing.
			RTransform*	ptrans,	// In:  Transformation to apply to CScene's before
										// transforming pts.
			RP3d*		p3dPtsSrc,	// In:  Ptr to group of pts to transform from.
			RP3d*		p3dPtsDst,	// Out: Ptr to group of pts to transform into.
			short		sNum);		// In:  The number of pts in p3dPtsSrc to transform.

		// Transform the given points through the CScene's pipeline with the
		// supplied transform and then map them to Realm 3D coordinates.
		void TransformPtsToRealm(	// Returns nothing.
			RTransform*	ptrans,		// In:  Transformation to apply to CScene's before
											// transforming pts.
			RP3d*		p3dPtsSrc,		// In:  Ptr to group of pts to transform from.
			RP3d*		p3dPtsDst,		// Out: Ptr to group of pts to transform into.
			short		sNum);			// In:  The number of pts in p3dPtsSrc to transform.

		// Render a 3D sprite tree into the specified image.
		// Ignores non 3D sprites.
		void DeadRender3D(						// Returns nothing.
			RImage*		pimDst,					// Destination image.
			CSprite3*	ps3Cur,					// Tree of 3D sprites to render.
			CHood*		phood,					// Da hood, homey.
			short			sDstX = 0,				// Destination 2D x coord.
			short			sDstY = 0,				// Destination 2D y coord.
			RRect*		prcDstClip = NULL);	// Dst clip rect.

		// Set all 'alpha' _and_ 'opaque' layers to xray.
		void SetXRayAll(		// You see a door to the north.  Returns nothing.
			bool bXRayAll);	// In:  true to X Ray all 'alpha' _and_ 'opaque' layers. 
	};


#endif //SCENE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
