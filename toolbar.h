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
// toolbar.h
// Project: Nostril (aka Postal)
//
// This module augments the CDude class, which used to be soley responsible for 
// updating player and weapon status.  It currently graphically manages the
// toolbars and sound reactions.  (Still need to add sounds)
////////////////////////////////////////////////////////////////////////////////
//
// History:
//
//		08/06/97 JRD	Started.  Set bars to load in the realm
//
//		08/08/97 JRD	Changed methodology to a proprietary data format and abandoning
//							attempts of a one to one interface with postal.  Tollbar will 
//							update an internal map of the weapon states based on events
//							passed.
//
//		08/08/97	JRD	Completely revamped and finalized the class structure.
//							Added the first hook to draw the entire bar.
//							Added aliased functions to reduce header dependencies.
//		08/09/97	JRD	Added all functionality except a hook to refresh on event.
//
//		08/25/97	JMI	Moved gsStatus* here from score.cpp.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include "RSPiX.h"
#include "hood.h"

// Because I do NOT yet want to be forced to put the entire clas in the header, I 
// am ALIASING the hooks to the toolbar so recompiles will be minimal:

// This should be done everytime the hood's palette changes.
extern	short	ToolBarInit(CHood* pHood);

// This does a FULL regeneration of the toolbar and draws it in yout area.
// It returns false if it isn't time to render.
extern	bool	ToolBarRender(
						CHood* pHood,
						RImage* pimDst,
						short sDstX,
						short sDstY,
						CDude* pDude,
						bool bForceRender = false
						);

extern	short gsStatusFontForeIndex;
extern	short gsStatusFontBackIndex;
extern	short gsStatusFontShadowIndex;

extern	short gsStatusFontForeDeadIndex;

#endif
