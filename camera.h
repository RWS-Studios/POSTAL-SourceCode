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
// camera.h
// Project: Nostril (aka Postal)
//
// History:
//		01/09/96 MJR	Started.
//
//		02/28/97	JMI	Snap(...) now accepts a CHood*.
//
//		07/31/97	JMI	Added m_bClip to allow us to disable clipping to realm
//							edges.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CAMERA_H
#define CAMERA_H

#include "RSPiX.h"
#include "scene.h"
#include "hood.h"


// A CCamera object is used to take pictures of a CScene and place them onto
// a film (actually, it's an RImage, but "film" fits the concept better).
//
// The area of the scene that is transferred to the film is referred to as the
// camera's "view".  The view is a rectangular area that is "mapped" onto two
// different coordinate systems: that of the scene and that of the film.  The
// view itself represents a third "common" coordinate system.
//
// The variables m_sSceneViewX and m_sSceneViewY link the view to the scene
// coordinate system, and m_sFilmViewX and m_sFilmViewY link it to the film
// coordinate system.  These variables can be used to easily convert between
// the coordinate systems.  In addition, the variables m_sScene2FilmX and
// m_sScene2FilmY are available to convert directly between scene and film
// coordinates.  Note that any coordinate can be converted from or to any
// system with with a single addition or subtraction:
//
// Most (if not all) member variables are "public" to allow for simple and
// efficient access.  If necessary, they can be modified directly, but if
// this is done, Update() MUST BE CALLED IMMEDIATELY AFTERWARDS in order to
// maintain the camera's internal consistancy.
class CCamera
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		short m_sViewW;											// View's width
		short m_sViewH;											// View's height

		CScene* m_pScene;											// Scene (the source)
		short m_sSceneViewX;										// View's upper left x in the scene
		short m_sSceneViewY;										// View's upper left y in the scene

		RImage* m_pimFilm;										// Film (the destination)
		short m_sFilmViewX;										// View's upper left x on the film
		short m_sFilmViewY;										// View's upper left y on the film

		short m_sScene2FilmX;									// Subtract from scene x to convert to film x
		short m_sScene2FilmY;									// Subtract from scene y to convert to film y

		CHood* m_pHood;											// Hood (used to limit camera movement)

		bool	m_bClip;												// Clip camera position to edge of realm, if true
																		// (default); otherwise, no clipping.

	//---------------------------------------------------------------------------
	// Public functions
	//---------------------------------------------------------------------------
	public:
		// Default (and only) constructor
		CCamera();

		// Destructor
		~CCamera();

		// Set camera's scene
		void SetScene(
			CScene* pScene);										// In:  Scene to take picture of

		// Set camera's hood
		void SetHood(
			CHood* pHood);											// In:  Hood to use for camera control

		// Set camera's view of the scene
		void SetView(
			short sSceneViewX,									// In:  View's upper left x (in scene coords)
			short sSceneViewY,									// In:  View's upper left y (in scene coords)
			short sViewW,											// In:  View's width
			short sViewH);											// In:  View's height

		// Set camera's view position (assumes the view's size will be or was set separately)
		void SetViewPos(
			short sSceneViewX,									// In:  View's upper left x (in scene coords)
			short sSceneViewY);									// In:  View's upper left y (in scene coords)

		// Set camera's view size (assumes the view's position will be or was set separately)
		void SetViewSize(
			short sViewW,											// In:  View's width
			short sViewH);											// In:  View's height

		// Set camera's film (the RImage in which to put pictures of the scene).  The view
		// can be put anywhere on the film and is clipped as required to fit the film.
		void SetFilm(
			RImage* pimFilm,										// In:  Film (where the picture ends up)
			short sFilmViewX,										// In:  View's upper left x (in film coords)
			short sFilmViewY);									// In:  View's upper left y (in film coords)

		// Snap a picture with the lens cover on (i.e. - set film's view area to black)
		void SnapWithLensCoverOn(void);

		// Snap a picture using the preset parameters (scene, view, and film)
		void Snap(void);

		// Snap a picture using the specified parameters.  These parameters are
		// temporary -- they do not affect any of the preset parameters!
		void Snap(
			short sViewW,											// In:  View's width
			short sViewH,											// In:  View's height
			CScene* pScene,										// In:  Scene to take picture of
			CHood* phood,											// In:  Hood for this scene.
			short sSceneViewX,									// In:  View's upper left x (in scene coords)
			short sSceneViewY,									// In:  View's upper left y (in scene coords)
			RImage* pimFilm,										// In:  Film (where the picture ends up)
			short sFilmViewX,										// In:  View's upper left x (in film coords)
			short sFilmViewY);									// In:  View's upper left y (in film coords)

		// Update internal state after setting new values
		void Update(void);
	};


#endif //CAMERA_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
