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
// TexEdit.h
// 
// History:
//		10/03/99 JMI	Started.
//
//		10/06/99	JMI	Now DoModal() accepts two lights to use.
//
//		10/07/99	JMI	Replaced m_fAlt and m_fAzi with a transform so rotations
//							can always be relative to the current orientation.
//
//////////////////////////////////////////////////////////////////////////////
//
// Implements a cheezy texture editor.
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(CTEXEDIT_H)
#define CTEXEDIT_H

//------------------------------------------------------------------------------
// Includes.
//------------------------------------------------------------------------------

#include "scene.h"

//------------------------------------------------------------------------------
// Macros.
//------------------------------------------------------------------------------

#define DECL_BTN_CALL(Name)											\
	void Name(RGuiItem* pgui);											\
	static void Name##_Static(RGuiItem* pgui)						\
		{																		\
		ASSERT(pgui->m_ulUserInstance);								\
		CTexEdit* pte	= (CTexEdit*)pgui->m_ulUserInstance;	\
		pte->Name(pgui);													\
		}

#define DECL_EVENT_CALL(Name)												\
	void Name(RGuiItem* pgui, RInputEvent* pie);						\
	static void Name##_Static(RGuiItem* pgui, RInputEvent* pie)	\
		{																			\
		ASSERT(pgui->m_ulUserInstance);									\
		CTexEdit* pte	= (CTexEdit*)pgui->m_ulUserInstance;		\
		pte->Name(pgui, pie);												\
		}

#define DECL_SB_CALL(Name)													\
	void Name(RScrollBar* psb);											\
	static void Name##_Static(RScrollBar* psb)						\
		{																			\
		ASSERT(psb->m_ulUserInstance);									\
		CTexEdit* pte	= (CTexEdit*)psb->m_ulUserInstance;			\
		pte->Name(psb);														\
		}



//------------------------------------------------------------------------------
// Typedefs.
//------------------------------------------------------------------------------

// References ////////////////////////////////////////////////////////////////
class CAnim3D;

// Classes ///////////////////////////////////////////////////////////////////
class CTexEdit 
	{
	//------------------------------------------------------------------------------
	// Typedefs.
	//------------------------------------------------------------------------------
	public:

		typedef enum
			{
			Trans,
			Scale,
			Rot,
			Paint
			} Manip;

		typedef enum
			{
			MaxCmds	= 10
			} Macros;


		typedef struct
			{
			long	lTriIndex;		// Triangle affected.
			U8		u8Color;			// Color applied.
			U8		u8PrevColor;	// Prev color.
			} Cmd;

	//------------------------------------------------------------------------------
	// Data.
	//------------------------------------------------------------------------------
	public:

		RGuiItem*	m_pguiRoot;			// Main dialog frame.
		RGuiItem*	m_pguiAnim;			// Anim frame.
		RGuiItem*	m_pguiCurColor;	// Current color frame.
		RGuiItem*	m_pguiPal;			// Color palette frame.
		CScene		m_scene;				// Scene for rendering.
		Manip			m_manip;				// Current manipulation type.
		bool			m_bDragging;		// true if in drag manipulation.
		short			m_sCursorResetX;	// Used by ProcessManip() to process drags.
		short			m_sCursorResetY;	// Used by ProcessManip() to process drags.
		float			m_fScale;			// Scaling.
		RTransform	m_transRot;			// Rotation.
		float			m_fX;					// Translation.
		float			m_fY;					// Translation.
		bool			m_bQuit;				// true when done.
		U8				m_u8Color;			// Current color.
		long			m_lTriIndex;		// Index of tri selected or < 0, if none.
		RTexture		m_texWork;			// Work texture.
		RTexture*	m_ptexSrc;			// The real thing.
		ChanTexture*	m_ptexchanSrc;	// Animation being edited.
		bool			m_bModified;		// true if texture has been modified since last
												// synch (revert or apply).
		RString		m_strFileName;		// Filename to save textures as.
		bool			m_bSpotLight;		// true for spotlight; false for ambient.
		short			m_sBrightness;		// Brightness for sprite.

	//------------------------------------------------------------------------------
	// Construction.
	//------------------------------------------------------------------------------
	public:

		//////////////////////////////////////////////////////////////////////////////
		// Default constructor.
		//////////////////////////////////////////////////////////////////////////////
		CTexEdit(void)
			;

		//////////////////////////////////////////////////////////////////////////////
		// Destructor.
		//////////////////////////////////////////////////////////////////////////////
		~CTexEdit(void)
			;

	//------------------------------------------------------------------------------
	// Methods.
	//------------------------------------------------------------------------------
	public:

		//////////////////////////////////////////////////////////////////////////////
		// Edit the texture of the specified 3D animation in a modal dialog.
		//////////////////////////////////////////////////////////////////////////////
		void
		DoModal(
			CAnim3D* panim,			// In:  3D animation to paint on.
			RAlpha* pltAmbient,		// In:  Ambient light.
			RAlpha* pltSpot,			// In:  Spot light.
			const RString& strFile)	// In:  Filename to save modified texture as.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Render the 3D animation at the specified time.
		//////////////////////////////////////////////////////////////////////////////
		void
		DoOutput(
			CSprite3* psprite,	// In:  3D data to render.
			RSop* psopView,		// In:  View space SOP.
			RTransform& trans,	// In:  Transformation.
			RAlpha* palphaLight,	// In:  Light.
			RImage* pimDst,		// In:  Destination for result.
			short sOffsetX,		// In:  X offset.
			short sOffsetY,		// In:  Y offset.
			RRect& rcClip)			// In:  Dst clip rect.
			;

		////////////////////////////////////////////////////////////////////////////////
		// Processes drags and keys into transform stuff.
		////////////////////////////////////////////////////////////////////////////////
		void
		ProcessManip(
			bool bButtonDown,		// In:  true if mouse button down.
			RInputEvent* pie,		// In:  Current input event.
			CSprite3* psprite,	// In:  3D data to process.
			RSop* psopView)		// In:  View space SOP.
			;

	//------------------------------------------------------------------------------
	// Querries.
	//------------------------------------------------------------------------------
	public:

		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------
	// Internal functions.
	//------------------------------------------------------------------------------
	protected:	

		//////////////////////////////////////////////////////////////////////////////
		// Apply work colors to real thing.
		//////////////////////////////////////////////////////////////////////////////
		void
		Apply(void)
			;

		//////////////////////////////////////////////////////////////////////////////
		// Revert work colors to real thing.
		//////////////////////////////////////////////////////////////////////////////
		void
		Revert(void)
			;

		//////////////////////////////////////////////////////////////////////////////
		// Save work colors to real thing.
		//////////////////////////////////////////////////////////////////////////////
		void
		Save(void)
			;

		//////////////////////////////////////////////////////////////////////////////
		// Set the current manipulation type.
		//////////////////////////////////////////////////////////////////////////////
		void
		SetManip(
			Manip manip)	// In:  New manipulation type.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Set the current palette color.
		//////////////////////////////////////////////////////////////////////////////
		void
		SetColor(
			U8	u8Color)		// In:  New color index.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Set the specified button to notify.
		//////////////////////////////////////////////////////////////////////////////
		void
		SetToNotify(
			long lBtnId,					// In:  ID of btn whose callback will be set.
			RGuiItem::BtnUpCall pfn)	// In:  Function to notify.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Set the specified gui to notify.
		//////////////////////////////////////////////////////////////////////////////
		void
		SetToNotify(
			long lId,							// In:  ID of gui whose callback will be set.
			RGuiItem::InputEventCall pfn)	// In:  Function to notify.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Set the specified scrollbar to notify.
		//////////////////////////////////////////////////////////////////////////////
		void
		SetToNotify(
			long lId,								// In:  ID of scrollbar whose callback will be set.
			RScrollBar::UpdatePosCall pfn)	// In:  Function to notify.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Reset transformation members.
		//////////////////////////////////////////////////////////////////////////////
		void
		ResetTransformation(void)
			{
			m_fScale = 0.5f;
			m_transRot.Make1();
			m_fX		= 0;	
			m_fY		= 0;	
			}

		//////////////////////////////////////////////////////////////////////////////
		// Set the status text field to that specified.
		//////////////////////////////////////////////////////////////////////////////
		void
		SetStatusText(
			const char* pszFrmt,		// In:  Format specifier ala sprintf.
			...)							// In:  Arguments as specified by format.
			;

		//////////////////////////////////////////////////////////////////////////////
		// Compose tarnsformations in the specified transform.  Init transform before
		// calling as necessary.
		//////////////////////////////////////////////////////////////////////////////
		void
		ComposeTransform(
			RTransform& trans)		// In:  Transform to compose in.
			;

	//------------------------------------------------------------------------------
	// Callbacks.
	//------------------------------------------------------------------------------
	protected:	

		DECL_BTN_CALL(QuitCall)
		DECL_BTN_CALL(ManipCall)
		DECL_EVENT_CALL(ColorCall)
		DECL_BTN_CALL(ApplyCall)
		DECL_BTN_CALL(SaveCall)
		DECL_BTN_CALL(RevertCall)
		DECL_BTN_CALL(SpotCall)
		DECL_SB_CALL(BrightnessCall)
		DECL_BTN_CALL(AdjustCall)
		DECL_EVENT_CALL(AnimCall)

	};

#endif // CTEXEDIT_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
