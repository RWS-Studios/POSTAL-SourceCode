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
///////////////////////////////////////////////////////////////////////////////
// PLUGIN.CPP
///////////////////////////////////////////////////////////////////////////////
// NOTE:  The following is temporary until a custom AppWizard is created and
// the process is refined to include Release versions of the lib and executable
// for Plugeth.
// NOTE: RSPXBlue.h is in ORANGE/MFC_CNTL.
///////////////////////////////////////////////////////////////////////////////
// Plug-ins:
//	1) To get started making your plug-in DLL, you must first create an 
// AppWizard DLL project.  Choose an MFC Extension DLL as the way in which your
// project uses MFC (on the first step dialog in the AppWizard).
//
// 2) Next, you need to add PlugIn.cpp (this file) and 
// \\narnia\apps\plugeth\plugethD.lib to your DLL project (although these files 
// are in orange you need to add them to your DLL project (not RSPiX Orange)).
//
// 3) For debugging, set the executable for debug session (under the Debug tab)
// to \\narnia\apps\plugeth\plugethD.exe.  You also may want to add a custom
// build step to copy your DLL to wherever you are storing your plugins.
//
// 4) Add GetPlugInObject to your .def file under EXPORTS (if your project name
// was "hosenbank", your .def file is hosenbank.def.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Includes.
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "PlugIn/PlugIn.h"


///////////////////////////////////////////////////////////////////////////////
// Instantiate static members.
///////////////////////////////////////////////////////////////////////////////
#ifndef _WINDLL
/*EXE2DLL*/ CImage*			CPlugIn::ms_pimBuf	= NULL;	// The current buffer.
/*EXE2DLL*/ CDialog*			CPlugIn::ms_pdlg		= NULL;	// The Plugger dialog.
/*EXE2DLL*/ CRSPiXBlue*		CPlugIn::ms_prspix	= NULL;	// The RSPiX window.
#endif

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Other members.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// These are automagically defined in the DLL.  They return the timestamp of 
// this file and the size of the CPlugIn structure.  The idea is that the EXE 
// can then check the values returned by this function against the timestamp of 
// the file and size of CPlugIn at the time of its compile and determine if 
// there's a potential synchronization issue with the communication structure.
//
///////////////////////////////////////////////////////////////////////////////
#ifdef _WINDLL
DLL2EXE char*	CPlugIn::GetDllMagicTime(void)
	{
	return m_szMagicTime; 
	}
#endif // _WINDLL

#ifdef _WINDLL
DLL2EXE long	CPlugIn::GetDllMagicSize(void)
	{
	return m_lMagicSize; 
	}
#endif // _WINDLL

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
