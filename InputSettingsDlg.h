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
// InputSettingsDlg.h
// Project: Nostril (aka Postal)
// 
// History:
//		07/03/97 JMI	Started.
//
//		07/05/97	JMI	Added protos for InputSettingsDlg_InitMenu() and
//							InputSettingsDlg_KillMenu().
//							Removed protos for EditKeySettings() and EditMouseSetings().
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INPUTSETTINGSDLG_H
#define INPUTSETTINGSDLG_H

#include "RSPiX.h"


//////////////////////////////////////////////////////////////////////////////
// Called to setup an input settings menu.
//////////////////////////////////////////////////////////////////////////////
extern short InputSettingsDlg_InitMenu(	// Returns 0 on success.
	Menu* pmenu);									// In:  Menu to setup.

//////////////////////////////////////////////////////////////////////////////
// Called to clean up an input settings menu.
//////////////////////////////////////////////////////////////////////////////
extern short InputSettingsDlg_KillMenu(	// Returns 0 on success.
	Menu* pmenu);									// In:  Menu to clean up.  

//////////////////////////////////////////////////////////////////////////////
// Called when a choice is made or a selection is changed on an input
// setttings menu.
//////////////////////////////////////////////////////////////////////////////
void InputSettingsDlg_Choice(	// Returns nothing.
	Menu*	pmenuCurrent,			// In:  Current menu.
	short sMenuItem);				// In:  Menu item chosen or -1 if selection 
										// change.

//////////////////////////////////////////////////////////////////////////////
// Edit the input settings via menu.
//////////////////////////////////////////////////////////////////////////////
extern short EditInputSettings(void);	// Returns nothing.

#endif	// INPUTSETTINGSDLG_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
