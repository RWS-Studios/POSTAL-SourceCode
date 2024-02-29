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
// NetDlg.H
// Project: Nostril (aka Postal)
// 
// History:
//		04/08/97 JMI	Started.
//
//		05/25/97	JMI	Integrated newest TCP/IP CNetServer/Client interface.
//							Still need to detect when the game is starting.
//							GUI could still use a little cleaning up.
//
//		05/26/97	JMI	Removed psNumPlayers and psMyPlayerNum parameters.
//
//		06/13/97 MJR	Changed function prototypes.
//
//		06/16/97 MJR	Added use of watchdog timer for network blocking callbacks.
//
//		08/02/97	JMI	Added an icon to watchdog timer for network blocking
//							callbacks and made it generally callable.
//
//		08/10/97 MJR	Added browse parameter to DoNetGameDialog().
//
//		08/27/97	JMI	Changed NetProbIcons functions to NetProbGui functions.
//							Also, instead of a DrawNetProbGui() there's a 
//							GetNetProbGui() so you can draw it, move it, change the
//							text, etc.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NETDLG_H
#define NETDLG_H

#include "RSPiX.h"
#include "netclient.h"
#include "netserver.h"


// More than one module needs to know the "standard" abort key for the "NetProb"
// gui that this module defines.  This gives both types of rspix codes for the
// key along with the descriptive text for that key.
#define NET_PROB_GUI_ABORT_GK_KEY		RSP_GK_F9
#define NET_PROB_GUI_ABORT_SK_KEY		RSP_SK_F9
#define NET_PROB_GUI_ABORT_KEY_TEXT		"F9"

// This is a general message for the "NetProb" gui that works for general
// cases of "network not responding".
extern char* g_pszNetProb_General;


////////////////////////////////////////////////////////////////////////////////
//
// Do network game dialog
//
////////////////////////////////////////////////////////////////////////////////
extern short DoNetGameDialog(							// Returns 0 if successfull, non-zero otherwise.
	CNetClient*	pclient,									// I/O: Client interface
	bool bBrowse,											// In:  Whether to browse (true) or connect (false)
	CNetServer*	pserver,									// I/O: Server interface or NULL if not server
	NetMsg* pmsgOut);										// Out: NetMsg::NOTHING or NetMsg::START_GAME


//////////////////////////////////////////////////////////////////////////////
//
// Get text associated with the specified error message
//
//////////////////////////////////////////////////////////////////////////////
extern const char* NetErrorText(						// Returns pointer to text
	NetMsg* pmsg);											// In:  Error message


//////////////////////////////////////////////////////////////////////////////
//
// Check if a blocked network operation was aborted.
//
//////////////////////////////////////////////////////////////////////////////
extern bool NetBlockingWasAborted(void);


////////////////////////////////////////////////////////////////////////////////
//
// Net blocking watchdog.  Call this periodically to let the watchdog know
// that the program hasn't locked up.
//
////////////////////////////////////////////////////////////////////////////////
extern void NetBlockingWatchdog(void);


//////////////////////////////////////////////////////////////////////////////
//
// Initialize the net problems GUI.
// Note that this is nearly instantaneous (no file access) if it has already
// been called.  That is, calling this twice in a row is fine.  Just make
// sure you call KillNetProbGUI() when done (although, even that is not
// essential).
//
//////////////////////////////////////////////////////////////////////////////
extern short InitNetProbGUI(void);

//////////////////////////////////////////////////////////////////////////////
//
// Kill the net problems GUI.
//
//////////////////////////////////////////////////////////////////////////////
extern void KillNetProbGUI(void);

//////////////////////////////////////////////////////////////////////////////
//
// Get the net prob GUI which is used to notify the user of network problems.
// You can move it, draw it, change its text, or whatever.  It's just up to 
// you to avoid having a problem with other things updating its settings while
// you are.
// Note that this is automagically drawn to the screen via the blocking 
// callback.
//
//////////////////////////////////////////////////////////////////////////////
extern RTxt* GetNetProbGUI(void);	// Returns the net prob GUI.

//////////////////////////////////////////////////////////////////////////////
//
// Determine whether there is a net problem.  Set via blocking callback.
// Cleared by you when you call ClearNetProb().
//
//////////////////////////////////////////////////////////////////////////////
extern bool IsNetProb(void);	// Returns true, if net problem; false otherwise.

//////////////////////////////////////////////////////////////////////////////
//
// Clear a net problem.  After a call to this function, IsNetProb() will 
// return false until the next net blocking callback or other asynch net 
// error.
//
//////////////////////////////////////////////////////////////////////////////
extern void ClearNetProb(void);	// Returns nothing.

#endif	// NETDLG_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
