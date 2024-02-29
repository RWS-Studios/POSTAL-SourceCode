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
// scene.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CScene class
//
// History:
//		01/05/97 MJR	Started.
//
//		01/31/97	JMI	Added cheezy alpha to Render().
//
//		02/06/97	JMI	Added 3D transform and render to Render() for CSprite3's.
//							For now, the world transform is set up on each 3D render,
//							but I will fix that next to be affected once.
//
//		02/07/97	JMI	Now uses m_transWorld member as the world transform.
//
//		02/07/97	JMI	Added the alpha blit for alphable sprite.
//							May not work yet, but Jeff wants it anyways.
//
//		02/07/97	JMI	Now it works, but there's some fudge in presuming the 3D
//							guy's diameter for the collision detection necessary for
//							the alpha X Ray.
//
//		02/07/97	JMI	Now the scene has its own pipeline and CSprite3 does not.
//							B/c of this we no longer needed the m_transWorld, b/c we
//							could incorporate it right into the view.
//							Also, removed m_ppipe from CSprite3 and added m_sRadius
//							so the X Ray logic could do correct collision between
//							the X Rayee and X Rayables. 
//							Also, added a function to initialize/create the pipeline.
//
//		02/10/97	JMI	Changed the number of "Randy Units" represented by our
//							SCREEN_DIAMETER_FOR_3D to 25.5 (was 30.0).
//
//		02/10/97	JMI	Added support for child objects of 3D sprites (CSprite3's).
//
//		02/11/97	JMI	Changed 3D support to recursive inline so indefinite child
//							depths can be supported.
//
//		02/11/97	JMI	Fixed bug in SetupPipeline() that was incorrectly trans-
//							lating the view transform along Z a distance of 
//							dModelRadius.  Now it transforms 0 along Z.
//							Also, changed MODEL_DIAMETER to the actual units used in
//							SoftImage and, consequently, set SCREEN_DIAMETER_FOR_3D
//							to the actual pixel size we want to use.  I imagine we'll
//							have to make both of these larger eventually to fit all
//							the animations.
//
//		02/13/97	JMI	Render() now uses the hood passed to it to get the lighting
//							effects.  Render() now takes a CHood* as a parm.
//
//		02/13/97	JMI	Render3D() now uses textures and fog.
//
//		02/14/97	JMI	Render3D() now passes the CSprite3.m_psphere values to Jeff
//							who returns them and sizes an image.  Currently, for 
//							clipping purposes, the dude is rendered in the image and
//							then rspBlitT'd to the screen (so, if his textures all map
//							to 0, he won't show up b/c 0 is the transparent color pass-
//							ed to rspBlitT).
//							Also, CSprite3 now contains an m_psphere ptr which must be    
//							set to the bounding sphere data for the CSprite3 frame (x, y,  
//							z, w).                                                     
//							Also, added TransformPts() function which will map an array
//							of pts through the optionally provided transform and then  
//							through the scenes transforms.                             
//
//		02/15/97	JMI	Now there are 3 ways a 3D object is dealt with in Render3D:
//							1) Not transformed or rendered -- it is entirely off the
//							destination region.
//							2) Rendered into an intermediate buffer Jeff allocates and
//							then rspBlitT'ed to the destination buffer -- it is 
//							partially off the destination region and, therefore, 
//							requires clippage.
//							3) Rendered directly into the destination region -- it is
//							entirely within the region.
//
//		02/14/97	JMI	Added m_transNoZ which is a transform that is equivalent
//							to the scene's pipeline (m_pipeline), but does not scale
//							Z to the maximum value.  It instead scales Z the same
//							as X and Y.  This is now used by TransformPts().
//		02/16/97	JMI	Now the Render3D should clip to the bounding cube of the
//							points to efficientize the clippage further.
//							Need to clean up Render3D(). I feel like I'm doing a little
//							too much stuff.
//		02/17/97	JMI	Also, increased model diameter (MODEL_DAIMETER) to 30.0
//							since the dude was 10.0 off the origin (very close to
//							exceeding the Z buf height.
//							Also, Render3D() now uses brighter lighting for objects
//							that have the InHighIntensity flag set and also uses
//							the m_sBrightness flag as the center point for the fog.
//							Also, added TransformPtsToRealm() which translate points
//							through the specified transform, the scene's pipeline's 
//							transforms, and finally maps them to Realm 3D coordinates.
//
//		02/17/97	JMI	Now the alpha X Ray effect auto centers on 3D dudes.
//
//		02/19/97	JMI	Now Alpha2d can handle sprites with no alpha.
//
//		02/19/97	JMI	Now does not affect the passed in brightness in Render3D.
//
//		02/21/97	JMI	Increased MODEL_DIAMETER to 50.0 to avoid problem with
//							children rigids exceeding the Z and clip buffers.
//
//		02/23/97	JMI	Had forgotten to Make1() the NoZ* transforms in 
//							SetupPipeline().
//
//		02/24/97	JMI	Had forgotten to specify dest clipping rect for FSPR8 blit
//							of Alpha2d.
//							
//		02/24/97	JMI	Collapsed CAlphaSprite2 into CSprite2 and added 
//							m_sAlphaLevel so a constant alpha level can be specified.
//							Using a constant alpha level is faster than using a mask.
//							Also, made it so the m_type field of the various sprites
//							are set automatically on construction so YOU SHOULD NO
//							LONGER SET THE CSprite*::m_type!
//
//		02/24/97	JMI	Added homogeneous alpha blit but it is untested as far as
//							its use in this module.
//
//		02/25/97	JMI	PATCHED FOR DEMO:  Instead of ASSERTing in Render3D() when
//							a CSprite3 is hosed, it will try to detect and skip it
//							for now.
//
//		02/26/97	JMI	Took out above patch.
//
//		03/05/97	JMI	Added Render() function to render a 3D sprite into a
//							specified background.
//
//		03/21/97	JMI	Added InDeleteOnRender flag and added support for new
//							CSpriteLine2d CSprite derived class.
//
//		03/27/97	JMI	Added SCALE3D parameter to allow quick scaling.
//
//		03/28/97	JMI	Put in VERY TEMP clippage to keep until the rspLine can
//							clip to a specified area (now clips to the destination).
//
//		04/02/97	JMI	Now use slightly faster method to determine the blit 
//							position for the sprite rendered into the clip buffer.
//							Changed MODEL_DIAMETER to 28.444444445 (from 50.0) so
//							we can divide it by 2 many times before getting errors
//							due to using integer division.  The idea of that 
//							particular number is to get a pixel size of the 3d world
//							of 128 which, of course, is a power of 2.
//
//		04/02/97	JMI	Put MODEL_DIAMETER back to 50.0 b/c rocket man's rocket
//							is too far away from him when the animation starts.
//
//		04/02/97	JMI	Put MODEL_DIAMETER back to 30.0.  Now Render3D() checks
//							to make sure the item fits into the Z buffer and, instead
//							of ASSERTing like it used to, it just TRACEs and does not
//							do the render.
//
//		04/02/97	JMI	In last fix, I used ||'s to separate newly combined ASSERTs
//							into an if ().  It should've been &&'s.  Fixed.
//
//		04/02/97	JMI	Removed old, commented out ASSERT in Render3D().  Also,
//							made Render3D() aware of InHidden flag.  Although the
//							loop in Render() takes care of the top-level objects'
//							InHidden flag, Render3D() was not, so child items that
//							were hidden would have been drawn, I think.  Now they
//							definitely won't.
//
//		04/14/97	JMI	Added use of TEMP flag m_bDontBlit to allow us to speed up
//							the Snap() call by not blitting.
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
//		05/02/97	JMI	If an alpha was 100 or over for a general alpha blit, it
//							was using the regular blit (opaque) instead of an rspBlitT
//							(transparent).
//
//		05/02/97	JMI	Added CSpriteCylinder3d, a cylinder primitive.
//
//		05/13/97	JMI	Now Render3D() tries to maximize the size of an object it
//							can render by centering top-level 3D objects in the 3D
//							screen based on their bounding sphere.
//							This change required additional fields in CSprite3.
//
//		05/13/97	JMI	Casted instances of warning C4018 (signed/unsigned mismatch)
//							to make MSVC 4.1(Alpha) happy (these seem to fall under
//							Warning Level 3 in 4.1(Alpha) but not in 4.2(Intel)).
//
//		05/15/97	JMI	I had thought that the max alpha (i.e., opaque) level was 
//							100 and was, therefore, checking to see if the level was
//							less than 100 before calling the alpha blit.  As it turns
//							out, it is 255.  Fixed.
//
//		05/15/97	JMI	Now obeys g_GameSettings.m_sAlphaBlend and m_sXRayEffect.
//
//		05/16/97	JMI	Code in Render3D() is set up so that no more than 1 level
//							deep of child will currently work for CSprite3.  The 
//							solution would be to add an m_transCumm to CSprite3 that
//							could accumulate the combined transforms as CSprite3's are
//							Render3D()'d.
//
//		05/19/97	JMI	Now, by pointing mysprite.m_pszText at some text, you can
//							display text at the sprites m_sX2, m_sY2.
//
//		05/20/97	JMI	Now DrawLine2D() uses rspLine() with a clip rect.
//
//		05/20/97	JMI	Text (CSprite::m_pszText) drawn via ms_print is now clipped
//							to a column that is guaranteed to be within the clip rect.
//
//		05/22/97	JMI	Now obeys g_GameSettings.m_s3dFog.
//
//		05/22/97	JMI	Now the collision detection for the alpha affect is done
//							using the width and height of the xray mask instead of the
//							dude's xrayee's width and height.
//
//		06/05/97	JMI	m_pipeline.m_pZB->Clear() was being called for top-level
//							3D items even if they were OFFSCREEN.  WHoopsee.  Fixed.
//
//		06/05/97	JMI	In Render3D(), the indirect render, when copying from the
//							temp (indirect) buf to the composite buffer, did not take
//							into account the fact that the center of the render may
//							not have been the center of the current renderee's bounding
//							sphere (which happens in the case of a child object (they
//							are blit relative to the center of the top-level parent's
//							bounding sphere) ).
//
//		06/10/97 JRD	Returned lighting to non-adjusting because the spheres weren't
//							accurate enough.  Luckily, the fix in the 3d export made the
//							non-adaptive lighting MUCH better.
//
//		06/10/97 JRD	Updated to centered based adaptive lighting - a good compromise!
//
//		06/14/97 MJR	Added g_bSceneDontBlit to replace the flag in gamesettings
//							that was being misused for that purpose.
//
//		06/16/97 JRD	Shrunk Z-buffer from 30 to 16 Randy Units
//
//		06/25/97	JMI	Added SetXRayAll() and m_bXRayAll which enable or disable 
//							xraying of all 'alpha' _and_ 'opaque' layers.
//							Also, changed InXrayable to InAlpha and added InOpaque to
//							differentiate between these two types which are now, at 
//							times, both xrayable.
//
//		06/26/97	JMI	Now uses phood->Map3Dto2D() (was using the global 
//							version defined in reality.h which is no more).
//
//		06/29/97 MJR	Converted vector of layers into simple array of layers
//							(motivated by trying to get it to work on the mac).
//
//		07/01/97	JMI	Added m_transScene2Realm which is used in 
//							TransformPtsToRealm() to convert from the scene to the 
//							realm.  This value is set via the SetupPipeline() call.
//							Currently, the code exists but is commented out b/c it
//							does not seem to bridge the gap between scene and realm
//							coords correctly.
//
//		07/10/97	JMI	Now Render2D() can uses an alpha mask with an alpha level
//							via the new rspGeneralAlphaBlit() which accepts a level.
//
//		07/17/97	JRD	Moved the pipeline tranform so it would only be called 
//							if a 3d object was on screen.  (watch for side effects)
//
//		07/20/97	JMI	Added some ASSERTs on m_sAlphaLevel in Render2D().
//
//		07/03/97 JRD	Altered the load to update the pipeline based on the 
//							3d scale for that level.
//
//		08/04/97	JMI	Now Render3D() verifies that a sprite has a valid m_pthing
//							before using it to display info on a sprite that could not
//							be drawn.
//
//		08/07/97	JMI	Moved global single instance of gdSCALE3D defined in 
//							scene.cpp into scene as an instantiable member, m_dScale3d.
//							Also, now SetupPipeline() does what UpdatePipeline() used
//							to (got rid of UpdatePipeline()).
//
//		08/18/97	JMI	Added ASSERTs to make sure inserted sprite's layer number
//							is greater than or equal to 0.
//
//		08/18/97	JMI	If the InDeleteOnRender flag was specified in a sprite,
//							it was removed from the scene list, deleted, and then its
//							flags were checked for the XRay bit which it usually had
//							because it was memset'ed to 0xdd on deletion.  This caused
//							it to be the XRayee.  Fixed.
//							Also, made the Render() call used for blitting things into
//							the background call the basic Render().
//							Also, increased MODEL_DIAMETER to 18.0 (was 16.0).
//
//		08/18/97	JMI	Changed Render() used by dead things to update themselves
//							into the background to be called DeadRender3D() to better
//							describe what it does and differentiate it from the other
//							more generic Render()ers.  Also, added two parameters so
//							it could call itself with 3D children.
//
//		08/23/97	JMI	Now offsets child light offset by the parent's.
//
//		10/03/99	JMI	Changed Render3D() to take a light scheme instead of a hood
//							to make it more general.
//
////////////////////////////////////////////////////////////////////////////////
#define SCENE_CPP

#include "RSPiX.h"
#include "scene.h"
#include "game.h"
#include "alphablitforpostal.h"
#include "reality.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SCREEN2MODEL_RATIO			(4.5 * m_dScale3d)

// This represents the number of "Randy units" represented by 
// SCREEN_DIAMETER_FOR_3D.
#define MODEL_DIAMETER				20.0	// In "Randy Units".
#define MODEL_ZSPAN					30.0  // must NEVER be exceeded or lighting is hosed

// Diameter in pixels for screen for 3D objects.
// Should be largest to hold largest 3D object.
#define SCREEN_DIAMETER_FOR_3D	(MODEL_DIAMETER * SCREEN2MODEL_RATIO)	

short	gsGlobalBrightnessPerLightAttribute = 5;  
/* short gsGlobalLightingAdjustment = 128; /* neutral center */
// NOTE: This max value completely depends on the actual lighting effect curve:
// it should be set near the bright spot of the curve.
short gsGlobalLightingAdjustment = 128; // NEUTRAL BABY!
//160 /* Max White */ - 6 * gsGlobalBrightnessPerLightAttribute;

// Used to control whether or not scene will actually blit.  When set to true,
// scene will execute quite a bit faster, except you won't see anything. :)
bool	g_bSceneDontBlit;

// Font stuff.
#define FONT_CELL_HEIGHT			15
#define FONT_FORE_COLOR				250
#define FONT_BACK_COLOR				0
#define FONT_SHADOW_COLOR			0

const double	c_dMaxScale		= 10.0;
const double	c_dMinScale		= 0.2;

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////
// Used to draw within the scene.
RPrint	CScene::ms_print;

////////////////////////////////////////////////////////////////////////////////
// Default (and only) constructor
////////////////////////////////////////////////////////////////////////////////
CScene::CScene()
	{
	// No array of layers yet
	m_pLayers = 0;
	m_sNumLayers = 0;
	
	// This can fail, so I had wanted to put it in SetupPipeline(), but there's
	// not much difference really since that function does not return an error
	// either.  Since this only needs to be done once, I decided to put it
	// here....let me know if you think it sucks.

	// Note that this function initializes m_dScale3d and other members.
	SetupPipeline();

	// If the static print has no font . . .
	if (ms_print.GetFont() == NULL)
		{
		ms_print.SetFont(FONT_CELL_HEIGHT, &g_fontBig);
		ms_print.SetColor(FONT_FORE_COLOR, FONT_BACK_COLOR, FONT_SHADOW_COLOR);
		ms_print.SetWordWrap(FALSE);
		}

	// Default to something (anything)
	m_bXRayAll = false;
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CScene::~CScene()
	{
	Clear();
	}


////////////////////////////////////////////////////////////////////////////////
// Clear the scene (clears out all layers and sprites)
////////////////////////////////////////////////////////////////////////////////
void CScene::Clear(void)
	{
	// Clear all sprites from all layers
	RemoveAllSprites();

	// Delete the layers
 	delete []m_pLayers;
 	m_pLayers = 0;
 	m_sNumLayers = 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Remove all sprites (from all layers)
///////////////////////////////////////////////////////////////////////////////
void CScene::RemoveAllSprites(void)
	{
	// Clear sprites from each layer
	for (short s = 0; s < m_sNumLayers; s++)
		RemoveSprites(s);
	}


////////////////////////////////////////////////////////////////////////////////
// Remove sprites from specified layer
////////////////////////////////////////////////////////////////////////////////
void CScene::RemoveSprites(
	short sLayer)											// In:  Layer number (0 to n-1)
	{
	// Get pointer to layer
	Layer* pLayer = &(m_pLayers[sLayer]);

	// Go through layer's collection of sprites
	for (msetSprites::iterator i = pLayer->m_sprites.begin(); i != pLayer->m_sprites.end(); i++)
		{
		// Get pointer to sprite
		CSprite* pSprite = *i;

		// Clear inserted flag
		pSprite->m_sPrivFlags &= ~CSprite::PrivInserted;

		// Delete sprite if delete-on-clear flag is set
		if (pSprite->m_sInFlags & CSprite::InDeleteOnClear)
			delete pSprite;
		}

	// Clear the sprite container
#if _MSC_VER >= 1020
	pLayer->m_sprites.clear();
#else
	pLayer->m_sprites.erase(pLayer->m_sprites.begin(), pLayer->m_sprites.end());
#endif
	}


////////////////////////////////////////////////////////////////////////////////
// Set number of layers (destroys any existing layers!)
////////////////////////////////////////////////////////////////////////////////
void CScene::SetLayers(
	short sNumLayers)										// In:  Number of layers
	{
	// Validate parameters
	ASSERT(sNumLayers > 0);

	// Clear all layers (and sprites)
	Clear();

	// Create array of layers
	m_pLayers = new Layer[sNumLayers];
	m_sNumLayers = sNumLayers;
	}


////////////////////////////////////////////////////////////////////////////////
// Update existing sprite or add new sprite
////////////////////////////////////////////////////////////////////////////////
void CScene::UpdateSprite(
	CSprite* pSprite)										// In:  Sprite to add
	{
	ASSERT(pSprite != NULL);

	// Check if it's already in the scene
	if (pSprite->m_sPrivFlags & CSprite::PrivInserted)
		{
		// Check if layer has changed
		if (pSprite->m_sLayer != pSprite->m_sSavedLayer)
			{
			// Erase from old layer
			m_pLayers[pSprite->m_sSavedLayer].m_sprites.erase(pSprite->m_iter);

			// Add to specified layer and save iterator for fast access later on
			ASSERT(pSprite->m_sLayer < m_sNumLayers);
			ASSERT(pSprite->m_sLayer >= 0);
			pSprite->m_iter = m_pLayers[pSprite->m_sLayer].m_sprites.insert(pSprite);
			pSprite->m_sSavedLayer = pSprite->m_sLayer;
			}

		// Check if priority has changed
		if (pSprite->m_sPriority != pSprite->m_sSavedPriority)
			{
			// For now, just erase it and then re-insert it.  There is definitely
			// a faster way to do this, but since we'll likely be revamping the
			// entire draw-order logic, I'll leave it like this.
			m_pLayers[pSprite->m_sSavedLayer].m_sprites.erase(pSprite->m_iter);
			pSprite->m_iter = m_pLayers[pSprite->m_sLayer].m_sprites.insert(pSprite);
			pSprite->m_sSavedPriority = pSprite->m_sPriority;
			}
		}
	else
		{
		// Add to specified layer and save iterator for fast access later on
		ASSERT(pSprite->m_sLayer < m_sNumLayers);
		ASSERT(pSprite->m_sLayer >= 0);
		pSprite->m_iter = m_pLayers[pSprite->m_sLayer].m_sprites.insert(pSprite);
 
		// Save layer and priority so we can detect changes to them
		pSprite->m_sSavedLayer = pSprite->m_sLayer;
		pSprite->m_sSavedPriority = pSprite->m_sPriority;

		// Set inserted flag
		pSprite->m_sPrivFlags |= CSprite::PrivInserted;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Remove sprite (safe to call even if sprite isn't in scene).
////////////////////////////////////////////////////////////////////////////////
void CScene::RemoveSprite(
	CSprite* pSprite)										// In:  Sprite to remove
	{
	ASSERT(pSprite != NULL);

	// Make sure sprite is currently inserted -- if not, skip it.  This is
	// not treated as an error because there may be situations where the
	// caller doesn't know the sprite has already been removed from the layer,
	// such as when someone else has called Clear() or some similar function.
	if(pSprite->m_sPrivFlags & CSprite::PrivInserted)
		{
		// Erase sprite from layer.  Knowing the iterator makes this very fast.
		ASSERT(pSprite->m_sSavedLayer < m_sNumLayers);
		m_pLayers[pSprite->m_sSavedLayer].m_sprites.erase(pSprite->m_iter);

		// Clear inserted flag
		pSprite->m_sPrivFlags &= ~CSprite::PrivInserted;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render a 3D sprite.
//
// See the Render3D(..., RAlpha*, ...) for details.
//
////////////////////////////////////////////////////////////////////////////////
void									// Returns nothing.
CScene::Render3D(		
	RImage*		pimDst,			// Destination image.
	short			sDstX,			// Destination 2D x coord.
	short			sDstY,			// Destination 2D y coord.
	CSprite3*	ps3Cur,			// 3D sprite to render.
	CHood*		phood,			// Da hood, homey.
	RRect*		prcDstClip)		// Dst clip rect.
	{
	RAlpha* palphaLight;
	// If high intensity indicated . . .
	if (ps3Cur->m_sInFlags & CSprite::InHighIntensity)
		{
		// Use spot lighting.
		palphaLight	= phood->m_pltSpot;
		}
	else
		{
		// Use ambient lighting.
		palphaLight	= phood->m_pltAmbient;
		}

	Render3D(					// Returns happy thoughts.
		pimDst,					// Destination image.
		sDstX,					// Destination 2D x coord.
		sDstY,					// Destination 2D y coord.
		ps3Cur,					// Tree of 3D sprites to render.
		palphaLight,			// Da hood, homey.
		prcDstClip);			// Dst clip rect.
	}

////////////////////////////////////////////////////////////////////////////////
// Render a 3D sprite.
//
// This function will, for a top level 3D sprite:
// 0) Clear the Z buffer.
//	1) Transform the bounding sphere to screen coordinates.
// 2) Ignore the sprite if the bounding sphere indicates the sprite is entirely
// offscreen.
// 3) Center the Render() on the bounding sphere to get maximum Z buffer room
// on X/Z plane.
// 4) Render into a clip image if the bounding sphere indicates the sprite is
// partially offscreen.  The clip image then gets rspBlitT'd to the composite
// buffer with clipping.
//
// For a non-top level 3D sprite:
// 1) Combine all parent transforms with this sprite's transform.
// 2) Transform the bounding sphere to screen coordinates.
// 3) Center the Render() on the top-level parent's bounding sphere so it matches 
// the parents' position(s).
// 4) Render into a clip image if the bounding sphere indicates the sprite is
// partially offscreen.  The clip image then gets rspBlitT'd to the composite
// buffer with clipping.
//
////////////////////////////////////////////////////////////////////////////////
void									// Returns nothing.
CScene::Render3D(		
	RImage*		pimDst,			// Destination image.
	short			sDstX,			// Destination 2D x coord.
	short			sDstY,			// Destination 2D y coord.
	CSprite3*	ps3Cur,			// 3D sprite to render.
	RAlpha*		plight,			// Light to render with.
	RRect*		prcDstClip)		// Dst clip rect.
	{
	// This transform is the product of the parent and the
	// child's transform used to determine the position of
	// a child object absolutely.
	RTransform	transChildAbs;
	RTransform*	ptransRender;	// The transform used.

	short		sCurX;
	short		sCurY;
	RP3d		pt3dSrcCenter, pt3dSrcRadius;		// Center and point on outside of bounding
															// sphere in "Randy" coords.
	short		sClipLeft;			// Amount clipped off left edge of dest region.
	short		sClipTop;			// Amount clipped off top edge of dest region.
	short		sClipRight;			// Amount clipped off right edge of dest region.
	short		sClipBottom;		// Amount clipped off bottom edge of dest region.
	short		sRadius;				// Radius of bounding sphere in pixels.
	short		sDiameter;			// Diameter of bounding sphere in pixels.
	short		sCenterX;			// Center of bounding sphere as screen coord.
	short		sCenterY;			// Center of bounding sphere as screen coord.
	RImage*	pimRender;			// Image to render into.
	bool		bIndirectRender;	// Rendering to intermediate buffer.
	short		sIndirectRenderX;	// Render coord when rendering to clip buffer.
	short		sIndirectRenderY;	// Render coord when rendering to clip buffer.
	short		sDirectRenderZ;	// Render Coord in either case
	short		sDirectRenderX;	// Render coord when rendering to pimDst buffer.
	short		sDirectRenderY;	// Render coord when rendering to pimDst buffer.
	short		sRenderX;			// Render coord passed to Render().
	short		sRenderY;			// Render coord passed to Render().
	short		sBlitX;				// Blit pos for 3D model.
	short		sBlitY;				// Biit pos for 3D model.
	short		sRenderOffX;		// Offset to Render().
	short		sRenderOffY;		// Offset to Render().

	ASSERT(ps3Cur->m_psop != NULL);
	ASSERT(ps3Cur->m_ptex != NULL);
	ASSERT(ps3Cur->m_pmesh != NULL);
	ASSERT(ps3Cur->m_ptrans != NULL);
	ASSERT(ps3Cur->m_psphere != NULL);

	// If there's a parent . . .
	if (ps3Cur->m_psprParent != NULL)
		{
		// NOTE: This does NOT work for more than 1 level of child depth.
		// To make that work, we must put a transform in the CSprite3.
		// Apply child and parent to transChildAbs.
		transChildAbs.Mul( ((CSprite3*)ps3Cur->m_psprParent)->m_ptrans->T, ps3Cur->m_ptrans->T);

		// Use transChildAbs.
		ptransRender	= &transChildAbs;
		}
	else
		{
		// Use current top-level matrix.
		ptransRender	= ps3Cur->m_ptrans;
		}

	// Here's the deal with m_pipeline. :
	// X == Origin.
	// @ == Center of sphere of points.
	//	 ________________m_pimClipBuf________________
	//	|			(m_sX, m_sY)	(m_sX + m_sW, m_sY)	|
	//	|					 ___________						|
	//	|					|    ooo    |						|
	//	|					|   o\o/o   |						|
	//	|					|    o*o    |						|
	//	|					|     o     |						|
	//	|					|ooooo@ooooo|						|
	//	|					|     o     |						|
	//	|					|    o o    |						|
	//	|					|   o   o   |						|
	//	|					|_oo__X__oo_|						|
	//	|	(m_sX, m_sY + m_sH)	(m_sX + m_sW, m_sY + m_sH)
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|															|
	//	|____________________________________________|

	// Get the bounding info.
	// Setup src pts.

	// Get current sphere.
	pt3dSrcCenter	= *(ps3Cur->m_psphere);
	pt3dSrcRadius.x	= pt3dSrcCenter.x + pt3dSrcCenter.w;
	pt3dSrcRadius.y	= pt3dSrcCenter.y + pt3dSrcCenter.w;
	pt3dSrcRadius.z	= pt3dSrcCenter.z + pt3dSrcCenter.w;
	pt3dSrcRadius.w	= 1;
	pt3dSrcCenter.w	= 1;

	// Let the pipeline know of the bounding sphere.
	m_pipeline.BoundingSphereToScreen(pt3dSrcCenter, pt3dSrcRadius, *ptransRender);

	// Check screen location:
	// Get radius of sphere of points (SphOP).
	sDiameter	= MAX(m_pipeline.m_sW, m_pipeline.m_sH);
	sRadius		= sDiameter / 2;

// ****TEMP****
if (g_bSceneDontBlit == false)
{
// ****END TEMP****

	// Determine destination of hotspot on screen relative to parent, if any.
	sCurX		= ps3Cur->m_sX2 + sDstX;
	sCurY		= ps3Cur->m_sY2 + sDstY;

	// Determine center of sphere of points relative to origin.
	short	sOrgRelCenX	= (m_pipeline.m_sCenX - (short)(SCREEN_DIAMETER_FOR_3D / 2));
	short	sOrgRelCenY	= (m_pipeline.m_sCenY - (short)(SCREEN_DIAMETER_FOR_3D / 2));
	// Determine center of sphere of points on screen.
	sCenterX		= sCurX + sOrgRelCenX;
	sCenterY		= sCurY + sOrgRelCenY;
	// Determine amount that is off of the clip rect.
	sClipLeft	= prcDstClip->sX - (sCenterX - sRadius);
	sClipTop		= prcDstClip->sY - (sCenterY - sRadius);
	sClipRight	= (sCenterX + sRadius) - (prcDstClip->sX + prcDstClip->sW);
	sClipBottom	= (sCenterY + sRadius) - (prcDstClip->sY + prcDstClip->sH);
	// If on screen at all . . .
	if (sClipLeft < sDiameter && sClipTop < sDiameter && sClipRight < sDiameter && sClipBottom < sDiameter)
		{
		// Transform pts through *ptransRender, view, and finally screen transforms.
		m_pipeline.Transform(
			ps3Cur->m_psop,		// Sea of 3D points to form
										// mesh around.
			*ptransRender);		// The transformation.

		// If there's no parent . . .
		if (ps3Cur->m_psprParent == NULL)
			{
			// Clear Z buffer for new 3D tree.
			m_pipeline.m_pZB->Clear();
			}

		// Determine destination for sprite on screen.
		// Get upper left hand corner by subtracting the radius from the screen center of points.
		sBlitX	= sCenterX - sRadius;
		sBlitY	= sCenterY - sRadius;

		//////////////////////////////////////////////////////////////////////////////
		// GLOBAL LIGHTING FACTOR: (ADAPTIVE)
		//////////////////////////////////////////////////////////////////////////////
		// Here we do dynamic light adjustment based on the current sphere.
		// Stage two aligns front of sphere:
		// *** RESTORING DO NON-ADJUSTMENT! ***
		// re-attempting center adjustment level
		short sLightOffset = 0;	// Must be initialized here (the other assignments are +=).
		//	- (m_pipeline.m_sCenZ + m_pipeline.m_sZ); 

		// If no parent . . .
		if (ps3Cur->m_psprParent == NULL)
			{
			// For indirect Render:  
			// Render so center of sphere is at center of clip image.
			sIndirectRenderX	= 0;
			sIndirectRenderY	= 0;
			// For direct Render:
			// Determine destination for render on screen.
			// The position you supply to Render() is one 3D screen radius
			// up and to the left.
			sDirectRenderX		= sCenterX - (short)(SCREEN_DIAMETER_FOR_3D / 2);
			sDirectRenderY		= sCenterY - (short)(SCREEN_DIAMETER_FOR_3D / 2);
			sDirectRenderZ		= m_pipeline.m_sCenZ; // don't need to know
			// Offset by the center relative to the origin.
			sRenderOffX			= -sOrgRelCenX;
			sRenderOffY			= -sOrgRelCenY;
			}
		else
			{
			// MUST BE 3D PARENT!
			ASSERT(ps3Cur->m_psprParent->m_type == CSprite::Standard3d);
			CSprite3*	ps3Parent	= (CSprite3*)(ps3Cur->m_psprParent);

			// Use the position the parent rendered at.
			sDirectRenderX		= ps3Parent->m_sDirectRenderX;
			sDirectRenderY		= ps3Parent->m_sDirectRenderY;
			sDirectRenderZ		= ps3Parent->m_sDirectRenderZ;

			sIndirectRenderX	= ps3Parent->m_sIndirectRenderX;
			sIndirectRenderY	= ps3Parent->m_sIndirectRenderY;

			// Offset by the amount the parent was offset.
			sRenderOffX			= ps3Parent->m_sRenderOffX;
			sRenderOffY			= ps3Parent->m_sRenderOffY;

			// Offset lighting by parent's offset.
			sLightOffset		+= ps3Parent->m_sBrightness;
			}

		// If this sprite has children . . .
		if (ps3Cur->m_psprHeadChild != NULL)
			{
			// Store Render() positions for children to use.
			ps3Cur->m_sDirectRenderX	= sDirectRenderX;
			ps3Cur->m_sDirectRenderY	= sDirectRenderY;
			ps3Cur->m_sDirectRenderZ	= sDirectRenderZ;
			ps3Cur->m_sIndirectRenderX	= sIndirectRenderX;
			ps3Cur->m_sIndirectRenderY	= sIndirectRenderY;
			// Store Render() offsets for children to use.
			ps3Cur->m_sRenderOffX		= sRenderOffX;
			ps3Cur->m_sRenderOffY		= sRenderOffY;
			}

		// If only partially on screen . . .
		if (sClipLeft > 0 || sClipTop > 0 || sClipRight > 0 || sClipBottom > 0)
			{
			ASSERT(m_pipeline.m_pimClipBuf != NULL);
			// Use clip image.
			pimRender			= m_pipeline.m_pimClipBuf;
			// Remember to blit to composite.
			bIndirectRender	= true;
			// Use appropriate position.
			sRenderX				= sIndirectRenderX;
			sRenderY				= sIndirectRenderY;
			}
		else
			{
			// Use composite buffer.
			pimRender			= pimDst;
			// Remember we don't need to blit to composite.
			bIndirectRender	= false;
			// Use appropriate position.
			sRenderX				= sDirectRenderX;
			sRenderY				= sDirectRenderY;
			}

		// use either the current z center or parent to adjust:
		// Note that we've already offset this by the parent and that is why
		// we do a += (see above (search for sLightOffset) ).
		sLightOffset += ps3Cur->m_sBrightness + gsGlobalLightingAdjustment - sDirectRenderZ;

		// Make sure we don't overrun the Z buffer . . .
		// Note that m_pipeline.m_sCen? are in image coords (i.e., (0, 0) is
		// the upper, left hand corner and (SCREEN_DIAMETER_FOR_3D, 
		// SCREEN_DIAMETER_FOR_3D) is the lower, right hand corner).
		if (	m_pipeline.m_sCenX - sRadius >= -sRenderOffX
			&&	m_pipeline.m_sCenX + sRadius < SCREEN_DIAMETER_FOR_3D - sRenderOffX
			&&	m_pipeline.m_sCenY - sRadius >= -sRenderOffY
			&&	m_pipeline.m_sCenY + sRadius < SCREEN_DIAMETER_FOR_3D - sRenderOffY)
			{
			// If fog enabled . . .
			if (g_GameSettings.m_s3dFog != FALSE)
				{
				// Render with textures and fog.
				m_pipeline.Render(
					pimRender,						// Dst image.
					sRenderX,						// 2D Dst coord.
					sRenderY,						// 2D Dst coord.
					ps3Cur->m_pmesh,				// Src mesh.
					m_pipeline.m_pZB,				// Z buffer (use its own for now).
					ps3Cur->m_ptex,				// Textures.
					sLightOffset,					// Fog offset.  Fogool?
					plight,							// Ambient lighting schtuff.
					sRenderOffX,					// Offset render/z-buffer to center of sphere of points.
					sRenderOffY);					// Offset render/z-buffer to center of sphere of points.
				}
			else
				{
				// Render with textures, no fog.
				m_pipeline.Render(
					pimRender,						// Dst image.
					sRenderX,						// 2D Dst coord.
					sRenderY,						// 2D Dst coord.
					ps3Cur->m_pmesh,				// Src mesh.
					m_pipeline.m_pZB,				// Z buffer (use its own for now).
					ps3Cur->m_ptex,				// Textures.
					sRenderOffX,					// Offset render/z-buffer to center of sphere of points.
					sRenderOffY);					// Offset render/z-buffer to center of sphere of points.
				}

			// If we rendered into an intermediate buffer b/c of clipping . . .
			if (bIndirectRender == true)
				{
				// Get it into destination.
				rspBlitT(
					0,																// Transparent index.
					pimRender,													// Src.
					pimDst,														// Dst.
					m_pipeline.m_sCenX - sRadius + sRenderOffX,	// Src.
					m_pipeline.m_sCenY - sRadius + sRenderOffY,	// Src.
					sBlitX,														// Dst.
					sBlitY,														// Dst.
					m_pipeline.m_sW,											// Both.
					m_pipeline.m_sH,											// Both.
					prcDstClip,													// Dst.
					NULL);														// Src.

				m_pipeline.ClearClipBuffer();
				}
			else
				{
				// Already in pimDst.
				}
#if 0	// Set to 1 to see origin target, center of points X, 
		// rect for bounding cube, and rect for clip image.
			// Draw bounds.
			rspRect(
				1, 
				RSP_WHITE_INDEX,
				pimDst,
				sCenterX - sRadius,
				sCenterY - sRadius,
				sDiameter, 
				sDiameter,
				prcDstClip);

			// Draw target at hotspot on screen.
			#define HOTSPOT_TARGET_LENGTH		5
			rspLine(
				RSP_WHITE_INDEX, 
				pimDst, 
				sCurX - HOTSPOT_TARGET_LENGTH / 2, 
				sCurY, 
				sCurX + HOTSPOT_TARGET_LENGTH / 2 + 1, 
				sCurY);
//				prcDstClip);
			rspLine(RSP_WHITE_INDEX, 
				pimDst, 
				sCurX, 
				sCurY - HOTSPOT_TARGET_LENGTH / 2, 
				sCurX, 
				sCurY + HOTSPOT_TARGET_LENGTH / 2 + 1);
//				prcDstClip);

			// Draw origin with X.
			#define ORIGIN_TARGET_LENGTH	6
			rspLine(
				251/*Yellow*/, 
				pimDst, 
				sCenterX - ORIGIN_TARGET_LENGTH / 2, 
				sCenterY - ORIGIN_TARGET_LENGTH / 2,
				sCenterX + ORIGIN_TARGET_LENGTH / 2 + 1, 
				sCenterY + ORIGIN_TARGET_LENGTH / 2 + 1);
//				prcDstClip);
			rspLine(
				251/*Yellow*/, 
				pimDst, 
				sCenterX + ORIGIN_TARGET_LENGTH / 2, 
				sCenterY - ORIGIN_TARGET_LENGTH / 2,
				sCenterX - ORIGIN_TARGET_LENGTH / 2 - 1, 
				sCenterY + ORIGIN_TARGET_LENGTH / 2 + 1);
//				prcDstClip);

			// Draw image.
			rspRect(1, RSP_WHITE_INDEX, pimDst, 
				sCenterX - (short)(SCREEN_DIAMETER_FOR_3D / 2), 
				sCenterY - (short)(SCREEN_DIAMETER_FOR_3D / 2), 
				m_pipeline.m_pimClipBuf->m_sWidth, 
				m_pipeline.m_pimClipBuf->m_sHeight,
				prcDstClip);

			// Note whether clipping.
			if (bIndirectRender == true)
				{
				rspLine(251/*Yellow*/, pimDst, sCenterX, sCenterY, 1, 1);
				}
#endif
			}
		else
			{
			char	szMsg[1024];
			if (ps3Cur->m_pthing)
				{
				sprintf(szMsg, "Render3D(): %s with ID %d",
					CThing::ms_aClassInfo[ps3Cur->m_pthing->GetClassID()].pszClassName,
					ps3Cur->m_pthing->GetInstanceID());
				}
			else
				{
				strcpy(szMsg, "Render3D(): Unknown class with unknown ID");
				}
			
			// If there's a parent . . .
			CSprite*	psprParent	= ps3Cur->m_psprParent;
			if (psprParent != NULL)
				{
				if (psprParent->m_pthing)
					{
					sprintf(szMsg, "%s, child of %s with ID %d,",
						szMsg,
						CThing::ms_aClassInfo[psprParent->m_pthing->GetClassID()].pszClassName,
						psprParent->m_pthing->GetInstanceID());
					}
				}

			strcat(szMsg, " would exceed the Z buffer, if rendered.  Not gonna do it.\n");
			TRACE(szMsg);
			}
		}

// ****TEMP****
}
// ****END TEMP****

	// Store this location (it is used by Render() to do collision circle for XRay).
	ps3Cur->m_sCenX	= ps3Cur->m_sX2 + m_pipeline.m_sCenX - (short)(SCREEN_DIAMETER_FOR_3D / 2);
	ps3Cur->m_sCenY	= ps3Cur->m_sY2 + m_pipeline.m_sCenY - (short)(SCREEN_DIAMETER_FOR_3D / 2);
	ps3Cur->m_sRadius	= sRadius;
	}

////////////////////////////////////////////////////////////////////////////////
// Line function until we have one that can clip to other than the dest image.
////////////////////////////////////////////////////////////////////////////////
inline
void DrawLine2d(			// Returns nothing.
	U8			u8Color,		// Color to draw line with.
	RImage*	pimDst,		// Destination image.
	short		sDstX1,		// Start pt.
	short		sDstY1,		// Start pt.
	short		sDstX2,		// End pt.
	short		sDstY2,		// End pt.
	RRect*	prcDstClip)	// Dst clip rect.
	{
	#if 0
		// If entirely on view . . .
		// *****VERY CHEESY, INCORRECT CLIP FOR LINE TOTALLY ON SCREEN****
		RRect	rcClipPt1(sDstX1, sDstY1, 1, 1);
		RRect	rcClipPt2(sDstX2, sDstY2, 1, 1);
		if (	rcClipPt1.ClipTo(prcDstClip) != -1
			&&	rcClipPt2.ClipTo(prcDstClip) != -1)
			{
			// Draw.
			rspLine(
				u8Color, 
				pimDst, 
				sDstX1,
				sDstY1,
				sDstX2,
				sDstY2);
			}
	#else
		// Draw.
		rspLine(
			u8Color, 
			pimDst, 
			sDstX1,
			sDstY1,
			sDstX2,
			sDstY2,
			prcDstClip);
	#endif
	}

////////////////////////////////////////////////////////////////////////////////
// Line function for 3D line.
////////////////////////////////////////////////////////////////////////////////
inline
void DrawLine3d(			// Returns nothing.
	U8			u8Color,		// Color to draw line with.
	RImage*	pimDst,		// Destination image.
	short		sDstX1,		// Start pt.
	short		sDstY1,		// Start pt.
	short		sDstZ1,		// Start pt.
	short		sDstX2,		// End pt.
	short		sDstY2,		// End pt.
	short		sDstZ2,		// End pt.
	CHood*	phood,		// Da hood, homey.
	RRect*	prcDstClip)	// Dst clip rect.
	{
	short	s2dX1, s2dY1;
	phood->Map3Dto2D(sDstX1, sDstY1, sDstZ1, &s2dX1, &s2dY1);
	short s2dX2, s2dY2;
	phood->Map3Dto2D(sDstX2, sDstY2, sDstZ2, &s2dX2, &s2dY2);

	DrawLine2d(u8Color, pimDst, s2dX1, s2dY1, s2dX2, s2dY2, prcDstClip);
	}

////////////////////////////////////////////////////////////////////////////////
// Draw a 2D line.
////////////////////////////////////////////////////////////////////////////////
void CScene::Line2D(					// Returns nothing.
	RImage*			pimDst,			// Destination image.
	short				sDstX,			// Destination 2D x coord.
	short				sDstY,			// Destination 2D y coord.
	CSpriteLine2d*	psl2,				// Tree of sprites to render.
	RRect*			prcDstClip)		// Dst clip rect.
	{
	DrawLine2d(
		psl2->m_u8Color,
		pimDst,
		psl2->m_sX2 + sDstX,   
		psl2->m_sY2 + sDstY,
		psl2->m_sX2End + sDstX,
		psl2->m_sY2End + sDstY,
		prcDstClip);
	}

////////////////////////////////////////////////////////////////////////////////
// Draw a 3D (as if there were another kind) cylinder.
// Actually, it draws a box from a cylinder description.
////////////////////////////////////////////////////////////////////////////////
void CScene::RenderCylinder3D(			// Returns nothing.
	RImage*					pimDst,			// Destination image.
	short						sDstX,			// Destination 2D x coord.
	short						sDstY,			// Destination 2D y coord.
	CSpriteCylinder3d*	psc3,				// Cylinder sprite.
	CHood*					phood,			// Da hood, homey.
	RRect*					prcDstClip)		// Dst clip rect.
	{
	short sX	= sDstX + psc3->m_sX2;
	short	sZ	= sDstY + psc3->m_sY2;

	// Determine points.
	short	sX1	= sX - psc3->m_sRadius;
	short	sX2	= sX + psc3->m_sRadius;
	short	sY1	= 0;
	short	sY2	= psc3->m_sHeight;
	short	sZ1	= sZ - psc3->m_sRadius;
	short	sZ2	= sZ + psc3->m_sRadius;

	// Base box.
	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX,
		sY1,
		sZ1,
		sX2,
		sY1,
		sZ,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX2,
		sY1,
		sZ,
		sX,
		sY1,
		sZ2,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX,
		sY1,
		sZ2,
		sX1,
		sY1,
		sZ,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX1,
		sY1,
		sZ,
		sX,
		sY1,
		sZ1,
		phood,
		prcDstClip);

	// Top box.
	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX,
		sY2,
		sZ1,
		sX2,
		sY2,
		sZ,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX2,
		sY2,
		sZ,
		sX,
		sY2,
		sZ2,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX,
		sY2,
		sZ2,
		sX1,
		sY2,
		sZ,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX1,
		sY2,
		sZ,
		sX,
		sY2,
		sZ1,
		phood,
		prcDstClip);

	// Height edges.
	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX,
		sY1,
		sZ1,
		sX,
		sY2,
		sZ1,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX2,
		sY1,
		sZ,
		sX2,
		sY2,
		sZ,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX,
		sY1,
		sZ2,
		sX,
		sY2,
		sZ2,
		phood,
		prcDstClip);

	DrawLine3d(
		psc3->m_u8Color,
		pimDst,
		sX1,
		sY1,
		sZ,
		sX1,
		sY2,
		sZ,
		phood,
		prcDstClip);
	}

////////////////////////////////////////////////////////////////////////////////
// Render a 2D sprite.
////////////////////////////////////////////////////////////////////////////////
void CScene::Render2D(		// Returns nothing.
	RImage*		pimDst,		// Destination image.
	short			sDstX,		// Destination 2D x coord.
	short			sDstY,		// Destination 2D y coord.
	CSprite2*	ps2Cur,		// Tree of sprites to render.
	CHood*		phood,		// Da hood, homey.
	RRect*		prcDstClip,	// Dst clip rect.
	CSprite*		psprXRayee)	// XRayee, if not NULL.
	{
	// Make sure the stuff we need is there.
	ASSERT(ps2Cur->m_pImage != NULL);

	ASSERT(ps2Cur->m_sAlphaLevel >= 0);
	ASSERT(ps2Cur->m_sAlphaLevel <= 255);

	if (ps2Cur->m_pimAlpha != NULL && g_GameSettings.m_sAlphaBlend != FALSE)
		{
		// If the alpha level is not opaque . . .
		if (ps2Cur->m_sAlphaLevel < 255)
			{
			// Do the Alpha Mask with adaptable alpha level blit.
			rspGeneralAlphaBlit(
				ps2Cur->m_sAlphaLevel,		// Adaptable alpha level.
				phood->m_pmaTransparency,	// Levels multialpha table.
				ps2Cur->m_pimAlpha,			// Alpha mask.
				ps2Cur->m_pImage,				// Src.
				pimDst,							// Dst.
				ps2Cur->m_sX2 + sDstX,		// 2D Dst coord.
				ps2Cur->m_sY2 + sDstY,		// 2D Dst coord.
				*prcDstClip);					// Dst clip.
			}
		else
			{
			// Do the Alpha Mask blit.
			rspGeneralAlphaBlit(
				phood->m_pmaTransparency,	// Levels multialpha table.
				ps2Cur->m_pimAlpha,			// Alpha mask.
				ps2Cur->m_pImage,				// Src.
				pimDst,							// Dst.
				ps2Cur->m_sX2 + sDstX,		// 2D Dst coord.
				ps2Cur->m_sY2 + sDstY,		// 2D Dst coord.
				*prcDstClip);					// Dst clip.
			}
		}
	else
		{
		// If the alpha level is not opaque . . .
		if (ps2Cur->m_sAlphaLevel < 255 && g_GameSettings.m_sAlphaBlend != FALSE)
			{
			// Do Homogeneous Alpha blit.
			rspAlphaBlitT(
				ps2Cur->m_sAlphaLevel,		// Homogeneous alpha level.
				phood->m_pmaTransparency,	// Levels multialpha table.
				ps2Cur->m_pImage,				// Src.
				pimDst,							// Dst.
				ps2Cur->m_sX2 + sDstX,		// 2D Dst coord.
				ps2Cur->m_sY2 + sDstY,		// 2D Dst coord.
				prcDstClip);					// Dst clip.
			}
		else
			{
			// Determine which BLiT to use based on image type
			if (ps2Cur->m_pImage->m_type == RImage::FSPR8)
				{
				// If this item is alpha OR this item is an opaque and we are
				// xraying opaques AND there is an xrayee AND the xray effect is enabled, 
				// we need to see if the xray should be used via collision detection.
				short sNormalBlit = 1;
#if 0			// The math equiv which appears to be slower than the confusing but
				// similar compares.  Also, the math allows none of the short circuiting 
				// that the C++ compares do.
				short	sUseXRay		
					=	(	(ps2Cur->m_sInFlags & CSprite::InAlpha) 
						+	(	(ps2Cur->m_sInFlags & CSprite::InOpaque)
							*	m_bXRayAll
							)
						)
					*	(long)psprXRayee
					*	(	g_GameSettings.m_sXRayEffect
						+	m_bXRayAll
						)
					;
						

				if (sUseXRay)
#else
				if (	(		(ps2Cur->m_sInFlags & CSprite::InAlpha) 
							|| ( (ps2Cur->m_sInFlags & CSprite::InOpaque) 
								&& m_bXRayAll == true
								)
						)
						&& psprXRayee 
						&& 
						(		g_GameSettings.m_sXRayEffect != FALSE
							||	m_bXRayAll == true)
					)
#endif
					{
					// Check if this sprite collides with the xrayee sprite.  If so, 
					// we need to do draw this sprite with the xray effect so the 
					// xrayee(s) can be seen through it.
					short	sXRayeeW	= phood->m_pimXRayMask->m_sWidth;
					short	sXRayeeH	= phood->m_pimXRayMask->m_sHeight;
					short	sXRayeeX	= psprXRayee->m_sX2;	// Default to 2D blit location.
					short	sXRayeeY	= psprXRayee->m_sY2;	// Default to 2D blit location.
					switch (psprXRayee->m_type)
						{
						case CSprite::Standard2d:
							// Use default sXRayeeX, sXRayeeY.
							break;
						case CSprite::Standard3d:
							{
							// For ease of access.
							CSprite3*	ps3XRayee	= (CSprite3*)psprXRayee;
							sXRayeeX	= ps3XRayee->m_sCenX - sXRayeeW / 2;
							sXRayeeY	= ps3XRayee->m_sCenY - sXRayeeH / 2;
							break;
							}
						}
					
					// Do an exteremely cheesy collision detecting by clipping the two rects!!!!
					RRect rect1(sXRayeeX, sXRayeeY, sXRayeeW, sXRayeeH);
					RRect rect2(ps2Cur->m_sX2, ps2Cur->m_sY2, ps2Cur->m_pImage->m_sWidth, ps2Cur->m_pImage->m_sHeight);
					if (rect2.ClipTo(&rect1) != -1)
						{
						// Clear flag so he doesn't get drawn by normal blit
						sNormalBlit = 0;

						// Get coordinate in BUILDING coordinates (i.e., coords inside/realtive-to the building)
						// for the alpha effect.
						short	sAlphaX	= (sXRayeeX + sXRayeeW / 2 - phood->m_pimXRayMask->m_sWidth / 2) - ps2Cur->m_sX2;
						short	sAlphaY	= (sXRayeeY + sXRayeeH / 2 - phood->m_pimXRayMask->m_sHeight / 2) - ps2Cur->m_sY2;

						// Set position of alpha effect.
						g_alphaBlit(
								ps2Cur->m_pImage,				// Source image.
								pimDst,							// Destination image.
								phood->m_pimXRayMask,		// Mask of alphable area.
								phood->m_pmaTransparency,	// Table of alphas or something.
								sAlphaX,							// Source coordinate in pimSrc to start blit.
								sAlphaY,							// Source coordinate in pimSrc to start blit.
								ps2Cur->m_sX2 + sDstX,		// Destination coordinate in pimDst for pimSrc(0,0).
								ps2Cur->m_sY2 + sDstY,		// Destination coordinate in pimDst for pimSrc(0,0).
								*prcDstClip);					// Rectangle to clip Dst to.
						}
					}
				if (sNormalBlit)
					{
					// Transparent BLiT
					rspBlit(
						ps2Cur->m_pImage,					// src
						pimDst,								// dst
						ps2Cur->m_sX2 + sDstX,			// map x to dst image
						ps2Cur->m_sY2 + sDstY,			// map y to dst image
						prcDstClip);						// dst clip
					}
				}
			else
				{
				// Non-alpha BLiT
				rspBlitT(
					0,										// Src.
					ps2Cur->m_pImage,					// src
					pimDst,								// dst
					0,										// src x
					0,										// src y
					ps2Cur->m_sX2 + sDstX,			// dst x
					ps2Cur->m_sY2 + sDstY,			// dst y
					ps2Cur->m_pImage->m_sWidth,	// width
					ps2Cur->m_pImage->m_sHeight,	// height
					prcDstClip,							// dst clip
					NULL);								// Src clip.
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render a sprite tree.
////////////////////////////////////////////////////////////////////////////////
void CScene::Render(			// Returns nothing.
	RImage*		pimDst,		// Destination image.
	short			sDstX,		// Destination 2D x coord.
	short			sDstY,		// Destination 2D y coord.
	CSprite*		pSprite,		// Tree of sprites to render.
	CHood*		phood,		// Da hood, homey.
	RRect*		prcDstClip,	// Dst clip rect.
	CSprite*		psprXRayee)	// XRayee, if not NULL.
	{
	while (pSprite != NULL)
		{
		// Make sure sprite isn't hidden (if it is, skip it)
		if (!(pSprite->m_sInFlags & CSprite::InHidden))
			{
			// Set flag to indicate sprite was rendered.  As of right now, the sprite WILL
			// be rendered if we get this far, but if the logic changes, the setting of
			// this flag might need to be moved to somewhere else.
			pSprite->m_sOutFlags |= CSprite::OutRendered;

			// Determine whether this is a 2d or 3d sprite
			switch (pSprite->m_type)
				{
				case CSprite::Standard2d:
					{
		// ****TEMP****
		if (g_bSceneDontBlit == false)
		{
		// ****END TEMP****
					Render2D(					// Returns nothing.
						pimDst,					// Destination image.
						sDstX,					// Destination 2D x coord.
						sDstY,					// Destination 2D y coord.
						(CSprite2*)pSprite,	// Tree of sprites to render.
						phood,					// Da hood, homey.
						prcDstClip,				// Dst clip rect.
						psprXRayee);			// XRayee, if not NULL.
		// ****TEMP****
		}
		// ****END TEMP****

					break;	// CSprite::Standard2d.
					}

				case CSprite::Standard3d:	// *** g_bSceneDontBlit is checked inside Render3D() 10/03/99 ***
					Render3D(					// Returns happy thoughts.
						pimDst,					// Destination image.
						sDstX,					// Destination 2D x coord.
						sDstY,					// Destination 2D y coord.
						(CSprite3*)pSprite,	// Tree of 3D sprites to render.
						phood,					// Da hood, homey.
						prcDstClip);			// Dst clip rect.

					break;	// CSprite::Standard3d.

				case CSprite::Line2d:
					{
		// ****TEMP****
		if (g_bSceneDontBlit == false)
		{
		// ****END TEMP****
					Line2D(								// Returns nothing.
						pimDst,							// Destination image.
						sDstX,							// Destination 2D x coord.
						sDstY,							// Destination 2D y coord.
						(CSpriteLine2d*)pSprite,	// Tree of sprites to render.
						prcDstClip);					// Dst clip rect.

		// ****TEMP****
		}
		// ****END TEMP****

					break;
					}

				case CSprite::Cylinder3d:
					{
		// ****TEMP****
		if (g_bSceneDontBlit == false)
		{
		// ****END TEMP****
					RenderCylinder3D(						// Returns nothing.
						pimDst,								// Destination image.
						sDstX,								// Destination 2D x coord.
						sDstY,								// Destination 2D y coord.
						(CSpriteCylinder3d*)pSprite,	// Cylinder sprite.
						phood,								// Da hood, homey.
						prcDstClip);						// Dst clip rect.

		// ****TEMP****
		}
		// ****END TEMP****

					break;
					}

				}

			// ****TEMP****
			if (g_bSceneDontBlit == false)
			{
			// ****END TEMP****
			// If there's some text . . .
			if (pSprite->m_pszText != NULL)
				{


				// Bind it.
				short	sX	= pSprite->m_sX2 + sDstX;
				short	sY	= pSprite->m_sY2 + sDstY;
				short	sW	= prcDstClip->sX + prcDstClip->sW - sX;
				short	sH	= prcDstClip->sY + prcDstClip->sH - sY;

				if (sW > 0 && sH > 0)
					{
					ms_print.SetColumn(
						sX, 
						sY,
						sW,
						sH);

					// Print it.
					ms_print.print(
						sX, sY,
						"%s",
						pSprite->m_pszText);
					}
				}
			// ****TEMP****
			}
			// ****END TEMP****

			// If this sprite has any children . . .
			if (pSprite->m_psprHeadChild != NULL)
				{
				Render(								// Returns nothing.
					pimDst,							// Destination image.
					sDstX + pSprite->m_sX2,		// Destination 2D x coord.
					sDstY + pSprite->m_sY2,		// Destination 2D y coord.
					pSprite->m_psprHeadChild,	// Tree of sprites to render.
					phood,							// Da hood, homey.
					prcDstClip,						// Dst clip rect.
					psprXRayee);					// XRayee, if not NULL.
				}
			}

		// Get sibling.
		pSprite	= pSprite->m_psprNext;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render specified area of scene into specified image
///////////////////////////////////////////////////////////////////////////////
void CScene::Render(
	short sSrcX,											// In:  Source (scene) x coord
	short sSrcY,											// In:  Source (scene) y coord
	short sW,												// In:  Width
	short sH,												// In:  Height
	RImage* pimDst,										// In:  Destination image
	short sDstX,											// In:  Destination (image) x coord
	short sDstY,											// In:  Destination (image) y coord
	CHood* phood)											// In:  The hood involved.
	{
	// Init dst clipping rect
	RRect rDstClip(sDstX, sDstY, sW, sH);

	// Clip the dst clipping rect to the image size (in case image is smaller)
	RRect rImageSize(0, 0, pimDst->m_sWidth, pimDst->m_sHeight);
	rDstClip.ClipTo(&rImageSize);

	ms_print.SetDestination(pimDst, &rDstClip);

	// Calculate mapping from scene to image coords
	short sMapX = sSrcX - sDstX;
	short sMapY = sSrcY - sDstY;

	CSprite*	psprXRayee	= NULL;	// XRayee when not NULL.

	// Go through all the layers, back to front
	for (short sLayer = 0; sLayer < m_sNumLayers; sLayer++)
		{
		// Get pointer to layer (more readable)
		Layer* pLayer = &m_pLayers[sLayer];

		// Make sure layer isn't hidden (if it is, skip it)
		if (!(pLayer->m_bHidden))
			{

			// Go through all the sprites stored in this layer
			for (msetSprites::iterator iSprite = pLayer->m_sprites.begin(); iSprite != pLayer->m_sprites.end(); )
				{
				// Get pointer to sprite (more readable than iterator dereference and may optimize better)
				CSprite* pSprite = *iSprite;

				Render(				// Returns nothing.
					pimDst,			// Destination image.
					-sMapX,			// Destination 2D x coord.
					-sMapY,			// Destination 2D y coord.
					pSprite,			// Tree of sprites to render.
					phood,			// Da hood, homey.
					&rDstClip,		// Dst clip rect.
					psprXRayee);	// XRayee, if not NULL.

				// Move to next item before deleting this one.
				iSprite++;

				// If this sprite wanted to be deleted after use . . .
				if (pSprite->m_sInFlags & CSprite::InDeleteOnRender)
					{
					RemoveSprite(pSprite);
					// Be gone, vile weed.
					delete pSprite;
					}
				else
					{
					// If this sprite is an xrayee, add it to the container
					if (pSprite->m_sInFlags & CSprite::InXrayee)
						{
						psprXRayee	= pSprite;
						}
					}
				}
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Setup render pipeline.  Use this function to setup or alter the pipeline.
// This function DOES a Make1() and then multiplies by the supplied transform,
// if any.  Any transforms that need to be applied after this setup can be
// done following a call to this function.
// The only sux is you cannot insert yourself into the middle of this function.
// If that's what you need, you should probably just do the entire setup for
// the pipe yourself or add a similar function to this one.
////////////////////////////////////////////////////////////////////////////////
void CScene::SetupPipeline(						// Returns nothing.
	RTransform* ptrans /*= NULL*/,				// Transform to apply before doing defaults.
	RTransform* ptransScene2Realm /*= NULL*/,	// Transform to convert from scene to realm.
	double		dScale3d /*= 0.0*/)				// In:  New scaling to apply to pipeline (see
															// m_dScale3d declaration).
	{
	/////////////////////////////////////////////////////////////////////////////
	// This portion was previously done in UpdatePipeline().
	/////////////////////////////////////////////////////////////////////////////
	// Adjust the current pipeline to account for the case when we've
	// loaded a new hood which may have a different 3d scaling for
	// the 3d characters.  This effects both the pipeline and the
	// smashatorium!
	/////////////////////////////////////////////////////////////////////////////
	// Some extra safety
	if (dScale3d <= c_dMinScale) dScale3d = c_dMinScale;
	if (dScale3d > c_dMaxScale) dScale3d = c_dMaxScale;

	ASSERT( (dScale3d >= c_dMinScale) && (dScale3d <= c_dMaxScale) );

	// Strap onto the old methods:
	m_dScale3d = dScale3d;

	// Use the built in adjustment features of the pipeline:
	if (m_pipeline.Create(1000, SCREEN_DIAMETER_FOR_3D) != 0)
		TRACE("SetupPipeline(): FONGOOL!  m_pipeline.Create() failed!  No 3D for you!\n");

	/////////////////////////////////////////////////////////////////////////////
	// End previously done in UpdatePipeline().
	/////////////////////////////////////////////////////////////////////////////

	// Re-init (Renit) the pipeline.
	m_pipeline.m_tView.Make1();	// Identity.
	m_pipeline.m_tScreen.Make1();	// Identity.
	m_transNoZView.Make1();			// Identity.
	m_transNoZScreen.Make1();		// Identity.

	// If there is a user transform . . .
	if (ptrans != NULL)
		{
		// Apply now. Ok. Close.
		m_pipeline.m_tView.PreMulBy(ptrans->T);
		}

	// If there is a conversion transform . . .
	if (ptransScene2Realm != NULL)
		{
		m_transScene2Realm	= *ptransScene2Realm;
		}
	else
		{
		m_transScene2Realm.Make1();
		}

	////////////////////////////////////////////////////////
	// Stolen from the famous 3D e-letters by Jeff Diamond.
	// We use this to get from SoftImage to Postal coord
	// system.
	////////////////////////////////////////////////////////

	// OK, this is a bit more professional way to do it:
	short sScreenSize		= SCREEN_DIAMETER_FOR_3D; // diameter in pixels;
//	short	sScreenRadius	= sScreenSize>>1;

	double dModelSize		= MODEL_DIAMETER; // diameter in Randy's coordinates
	double dModelRadius	= 0.5 * dModelSize;

	// Flipping the image in the view was a bad hack, bacause it
	// changes the direction of rotation about the axis:
	//	m_pipe.m_tView.Scale(1.0,-1.0,1.0); // DON'T DO!

	// Just put the Randy origin in the center of the cube:
	m_pipeline.m_tView.Trans(dModelRadius, dModelRadius, 0.0);
	// Mimic this in transform that contains no Z scaling.
	m_transNoZView.Trans(dModelRadius, dModelRadius, 0.0);

	// Then, to project to the screen, need to FIRST scale it,
	// THEN flip it about y=0, and FINALLY slide it back down sp
	// we can see it.  (Luckily, when you're down it's still a single
	// matrix, so it's no slower than anything else you might do.

	// Also, stretch out the z-coordinate to take full advantage of the 
	// 16-bit z-buffer
	//
	m_pipeline.m_tScreen.Scale(
		double(sScreenSize) / dModelSize,
		double(sScreenSize) / dModelSize,
		REAL(65535.0 / MODEL_ZSPAN));	 // Use MODEL_ZSPAN for consistent fog thickness
	// Mimic this in transform that contains no Z scaling.
	m_transNoZScreen.Scale(
		double(sScreenSize) / dModelSize,
		double(sScreenSize) / dModelSize,
		double(sScreenSize) / dModelSize);

	// Now offset the image and flip the y-coordinate:
	m_pipeline.m_tScreen.Scale(1.0,-1.0,1.0); // mirror vertically about (y=0)
	// Mimic this in transform that contains no Z scaling.
	m_transNoZScreen.Scale(1.0, -1.0, 1.0);

	m_pipeline.m_tScreen.Trans(0.0,sScreenSize,0.0); // slide down the screen
	// Mimic this in transform that contains no Z scaling.
	m_transNoZScreen.Trans(0.0,sScreenSize,0.0);

	////////////////////////////////////////////////////////
	// More transforms can be applied to either of the 
	// pipeline's transforms after this function exits, if
	// so desired.
	////////////////////////////////////////////////////////
	}

////////////////////////////////////////////////////////////////////////////////
// Transform the given points through the CScene's pipeline with the
// supplied transform.
////////////////////////////////////////////////////////////////////////////////
void CScene::TransformPts(	// Returns nothing.
	RTransform*	ptrans,		// In:  Transformation to apply to CScene's before
									// transforming pts.
	RP3d*		p3dPtsSrc,		// In:  Ptr to group of pts to transform from.
	RP3d*		p3dPtsDst,		// Out: Ptr to group of pts to transform into.
	short		sNum)				// In:  The number of pts in p3dPtsSrc to transform.
	{
	// Let's try to be nice and let the user call us all the time.
	// That is, check if there's anything to do before wasting any time . . .
	if (sNum > 0)
		{
		RTransform	transApplied;	// Auto-identitied.

		// If there is a user transformation . . .
		if (ptrans != NULL)
			{
			// Create scene-user transform:
			// Apply user to view.
			transApplied.Mul(m_transNoZView.T, ptrans->T);
			// Apply user-view to screen (store in user-view).
			transApplied.PreMulBy(m_transNoZScreen.T);
			}
		else
			{
			// Create plain scene transform:
			// Apply view to screen (store in scene transform).
			transApplied.Mul(m_transNoZScreen.T, m_transNoZView.T);
			}

		// Translate points
		short	sCur;
		// Note that this could save a fraction of a miniscule(sp?) moment by being
		// a do { } while since we know sNum > 0, but fongooey.
		for (sCur = 0; sCur < sNum; sCur++)
			{
			transApplied.TransformInto(*p3dPtsSrc++, *p3dPtsDst++);
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Transform the given points through the CScene's pipeline with the
// supplied transform and then map them to Realm 3D coordinates.
////////////////////////////////////////////////////////////////////////////////
void CScene::TransformPtsToRealm(	// Returns nothing.
	RTransform*	ptrans,		// In:  Transformation to apply to CScene's before
									// transforming pts.
	RP3d*		p3dPtsSrc,		// In:  Ptr to group of pts to transform from.
	RP3d*		p3dPtsDst,		// Out: Ptr to group of pts to transform into.
	short		sNum)				// In:  The number of pts in p3dPtsSrc to transform.
	{
#if 0	// This doesn't work.
	RTransform	trans;
	// If there is a user transform . . .
	if (ptrans != NULL)
		{
		// Combine the caller's with the conversion.
		trans.Mul(m_transScene2Realm.T, ptrans->T);
		// Use new transform.
		ptrans	= &trans;
		}
	else
		{
		// Use the conversion transform.
		ptrans	= &m_transScene2Realm;
		}
#endif

	// Map to scene.
	TransformPts(ptrans, p3dPtsSrc, p3dPtsDst, sNum);

	// Translate to Postal Realm coords.
	short	i;
	for (i = 0; i < sNum; i++)
		{
		p3dPtsDst->x		-= SCREEN_DIAMETER_FOR_3D / 2;
		p3dPtsDst->y		= -(p3dPtsDst->y - SCREEN_DIAMETER_FOR_3D / 2);
		// No Z mapping.
//		p3dPtsDst->z		= p3dPtsDst->z;

		p3dPtsDst++;
		}

	}

////////////////////////////////////////////////////////////////////////////////
// Render a 3D sprite tree into the specified image.
// Ignores non 3D sprites.
////////////////////////////////////////////////////////////////////////////////
void CScene::DeadRender3D(					// Returns nothing.
	RImage*		pimDst,						// Destination image.
	CSprite3*	ps3,							// Tree of 3D sprites to render.
	CHood*		phood,						// Da hood, homey.
	short			sDstX /*= 0*/,				// Destination 2D x coord.
	short			sDstY /*= 0*/,				// Destination 2D y coord.
	RRect*		prcDstClip /*= NULL*/)	// Dst clip rect.
	{
	// Make sure sprite isn't hidden (if it is, skip it)
	if (!(ps3->m_sInFlags & CSprite::InHidden))
		{
		ASSERT(pimDst	!= NULL);
		ASSERT(ps3		!= NULL);
		ASSERT(phood	!= NULL);

		RRect	rcClip;
		// If no clipping rect specified . . .
		if (prcDstClip == NULL)
			{
			prcDstClip	= &rcClip;

			rcClip.sX	= 0;
			rcClip.sY	= 0;
			rcClip.sW	= pimDst->m_sWidth;
			rcClip.sH	= pimDst->m_sHeight;
			}

#if 1
		// Render.
		Render3D(
			pimDst,			// Destination image.
			sDstX,			// Destination 2D x coord.
			sDstY,			// Destination 2D y coord.
			ps3,				// Tree of 3D sprites to render.
			phood,			// Da hood, homey.
			prcDstClip);	// Dst clip rect.

		// Call for all children . . .
		CSprite*	psprite	= ps3->m_psprHeadChild;
		while (psprite)
			{
			// If 3D child and visible . . .
			if (psprite->m_type == CSprite::Standard3d)
				{
				// Render 'em.
				DeadRender3D(
					pimDst, 
					(CSprite3*)psprite, 
					phood, 
					sDstX + ps3->m_sX2,
					sDstY + ps3->m_sY2,
					prcDstClip);
				}

			// Next, please.
			psprite	= psprite->m_psprNext;
			}
#else
		Render(				// Returns nothing.
			pimDst,			// Destination image.
			0,					// Destination 2D x coord.
			0,					// Destination 2D y coord.
			ps3,				// Tree of sprites to render.
			phood,			// Da hood, homey.
			prcDstClip,		// Dst clip rect.
			NULL);			// XRayee, if not NULL.
#endif
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Set all 'alpha' _and_ 'opaque' layers to xray.
////////////////////////////////////////////////////////////////////////////////
void CScene::SetXRayAll(	// You see a door to the north.  Returns nothing.       
	bool bXRayAll)				// In:  true to X Ray all 'alpha' _and_ 'opaque' layers.
	{
	m_bXRayAll	= bXRayAll;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
