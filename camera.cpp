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
// camera.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CCamara class
//
// History:
//		01/09/96 MJR	Started.
//
//		02/13/97	JMI	Now passes m_pHood to CScene::Render().
//
//		02/28/97	JMI	The Snap that takes many parameters was not using the
//							passed in CScene*, now it is.
//							Snap() now accepts a CHood* as well.
//
//		07/05/97 MJR	Changed to RSP_BLACK_INDEX instead of 0.
//
//		07/31/97	JMI	Added m_bClip to allow us to disable clipping to realm
//							edges.
//
////////////////////////////////////////////////////////////////////////////////
#define CAMERA_CPP

#include "RSPiX.h"
#include "camera.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Default constructor
////////////////////////////////////////////////////////////////////////////////
CCamera::CCamera()
	{
	// Clear these for testing/debugging
	m_pScene = 0;
	m_pimFilm = 0;
	m_pHood = 0;
	// Default to clipping to realm edges.
	m_bClip = true;
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CCamera::~CCamera()
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Set camera's scene
////////////////////////////////////////////////////////////////////////////////
void CCamera::SetScene(
	CScene* pScene)										// In:  Scene to take picture of
	{
	// Save specified values
	m_pScene = pScene;
	}


////////////////////////////////////////////////////////////////////////////////
// Set camera's hood
////////////////////////////////////////////////////////////////////////////////
void CCamera::SetHood(
	CHood* pHood)											// In:  Hood to use for camera control
	{
	// Save specified values
	m_pHood = pHood;
	}


////////////////////////////////////////////////////////////////////////////////
// Set camera's view of the scene
////////////////////////////////////////////////////////////////////////////////
void CCamera::SetView(
	short sSceneViewX,									// In:  View's upper left x (in scene coords)
	short sSceneViewY,									// In:  View's upper left y (in scene coords)
	short sViewW,											// In:  View's width
	short sViewH)											// In:  View's height
	{
	// Save specified values
	m_sSceneViewX = sSceneViewX;
	m_sSceneViewY = sSceneViewY;
	m_sViewW = sViewW;
	m_sViewH = sViewH;

	// Update state
	Update();
	}


////////////////////////////////////////////////////////////////////////////////
// Set camera's view position (assumes the view's size will be or was set separately)
////////////////////////////////////////////////////////////////////////////////
void CCamera::SetViewPos(
	short sSceneViewX,									// In:  View's upper left x (in scene coords)
	short sSceneViewY)									// In:  View's upper left y (in scene coords)
	{
	// Save specified values
	m_sSceneViewX = sSceneViewX;
	m_sSceneViewY = sSceneViewY;

	// Update state
	Update();
	}


////////////////////////////////////////////////////////////////////////////////
// Set camera's view size (assumes the view's position will be or was set separately)
////////////////////////////////////////////////////////////////////////////////
void CCamera::SetViewSize(
	short sViewW,											// In:  View's width
	short sViewH)											// In:  View's height
	{
	// Save specified values
	m_sViewW = sViewW;
	m_sViewH = sViewH;

	// Update state
	Update();
	}


////////////////////////////////////////////////////////////////////////////////
// Set camera's film (the RImage in which to put pictures of the scene).  The view
// can be put anywhere on the film and is clipped as required to fit the film.
////////////////////////////////////////////////////////////////////////////////
void CCamera::SetFilm(
	RImage* pimFilm,										// In:  Film (where the picture ends up)
	short sFilmViewX,										// In:  View's upper left x (in film coords)
	short sFilmViewY)										// In:  View's upper left y (in film coords)
	{
	// Save specified values
	m_pimFilm = pimFilm;
	m_sFilmViewX = sFilmViewX;
	m_sFilmViewY = sFilmViewY;

	// Update state
	Update();
	}


////////////////////////////////////////////////////////////////////////////////
// Snap a picture with the lens cover on (i.e. - set film's view area to black)
////////////////////////////////////////////////////////////////////////////////
void CCamera::SnapWithLensCoverOn(void)
	{
	ASSERT(m_pimFilm != NULL);

	// Draw black rectangle (automatically clips to image size)
	rspRect(RSP_BLACK_INDEX, m_pimFilm, m_sFilmViewX, m_sFilmViewY, m_sViewW, m_sViewH);
	}


////////////////////////////////////////////////////////////////////////////////
// Snap a picture using the preset parameters (scene, view, and film)
////////////////////////////////////////////////////////////////////////////////
void CCamera::Snap(void)
	{
	ASSERT(m_pScene != NULL);
	ASSERT(m_pimFilm != NULL);

/*
	// Init film clipping rect to view's location on the film
	RRect rFilmClip(m_sFilmViewX, m_sFilmViewY, m_sViewW, m_sViewH);

	// Clip the clipping rect to the film size
	RRect rFilmSize(0, 0, m_pimFilm->m_sWidth, m_pimFilm->m_sHeight);
	rFilmClip.ClipTo(&rFilmSize);

	// Tell scene to render itself onto film
	m_pScene->Render(m_sScene2FilmX, m_sScene2FilmY, m_pimFilm, &rFilmClip);
*/
	// Tell scene to render itself onto film (scene handles clipping)
	m_pScene->Render(m_sSceneViewX, m_sSceneViewY, m_sViewW, m_sViewH, m_pimFilm, m_sFilmViewX, m_sFilmViewY, m_pHood);
	}


////////////////////////////////////////////////////////////////////////////////
// Snap a picture using the specified parameters.  These parameters are
// temporary -- they do not affect any of the preset parameters!
////////////////////////////////////////////////////////////////////////////////
void CCamera::Snap(
	short sViewW,											// In:  View's width
	short sViewH,											// In:  View's height
	CScene* pScene,										// In:  Scene to take picture of
	CHood* phood,											// In:  Hood for this scene.
	short sSceneViewX,									// In:  View's upper left x (in scene coords)
	short sSceneViewY,									// In:  View's upper left y (in scene coords)
	RImage* pimFilm,										// In:  Film (where the picture ends up)
	short sFilmViewX,										// In:  View's upper left x (in film coords)
	short sFilmViewY)										// In:  View's upper left y (in film coords)
	{
/*
	// Init film clipping rect to view's location on the film
	RRect rFilmClip(sFilmViewX, sFilmViewY, sViewW, sViewH);

	// Clip the clipping rect to the film size
	RRect rFilmSize(0, 0, pimFilm->m_sWidth, pimFilm->m_sHeight);
	rFilmClip.ClipTo(&rFilmSize);

	// Calculate mapping from scene to film coords
	short sScene2FilmX = sSceneViewX - sFilmViewX;
	short sScene2FilmY = sSceneViewY - sFilmViewY;

	// Tell scene to render itself onto film
	pScene->Render(sScene2FilmX, sScene2FilmY, pimFilm, &rFilmClip);
*/
	// Tell scene to render itself onto film (scene handles clipping)
	pScene->Render(sSceneViewX, sSceneViewY, sViewW, sViewH, pimFilm, sFilmViewX, sFilmViewY, phood);
	}


////////////////////////////////////////////////////////////////////////////////
// Update internal state after setting new values
////////////////////////////////////////////////////////////////////////////////
void CCamera::Update(void)
	{
	// If clipping is on . . .
	if (m_bClip == true)
		{
		// Limit to left edge of scene
		if (m_sSceneViewX < 0)
			m_sSceneViewX = 0;

		// Limit to top edge of scene
		if (m_sSceneViewY < 0)
			m_sSceneViewY = 0;

		// Limit to right edge of scene
		if (m_pHood != 0)
			{
			short sClipX = (m_sSceneViewX + m_sViewW) - m_pHood->GetWidth();
			if (sClipX > 0)
				m_sSceneViewX -= sClipX;

			// Limit to bottom edge of scene
			short sClipY = (m_sSceneViewY + m_sViewH) - m_pHood->GetHeight();
			if (sClipY > 0)
				m_sSceneViewY -= sClipY;
			}
		}

	// Calculate mapping from scene to film coords
	m_sScene2FilmX = m_sSceneViewX - m_sFilmViewX;
	m_sScene2FilmY = m_sSceneViewY - m_sFilmViewY;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
