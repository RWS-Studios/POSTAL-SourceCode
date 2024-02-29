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
#ifndef PLUGIN_H
#define PLUGIN_H

///////////////////////////////////////////////////////////////////////////////
// Includes.
///////////////////////////////////////////////////////////////////////////////
#include "Image/Image.h"
#include "MFC_CNTL/RSPXBlue.h"

///////////////////////////////////////////////////////////////////////////////
// Macros.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Typedefs.
///////////////////////////////////////////////////////////////////////////////
class CPlugIn
	{
	public:
		// Init reserved space to zero so older versions will continue to work
		// with newer versions of CPlugIn.
		CPlugIn()																
			{
			memset(m_aucReserved, 0, sizeof(m_aucReserved)); 
			m_lMagicSize	= sizeof(CPlugIn);
			strcpy(m_szMagicTime, __TIMESTAMP__);
			}

      /////////////////////////////////////////////////////////////////////////
		// The following pure virtual functions must be defined by the plug in 
		// DLL.
      /////////////////////////////////////////////////////////////////////////

		// Override this to return the description of your class.
		// You MUST define this function.
		// Returns a pointer to text (that will not be freed) that describes
		// your class.
		// NOTE: Pure virtual.
		DLL2EXE virtual char* GetDescription(void)	= 0;

		// Override this to activate your plug-in.
		// This gets called when the user "selects" your plug-in.
		// You should define this function if you want to react to 
		// the user selecting your plug in DLL.
		// Returns 0 on success.
		DLL2EXE virtual short Activate(void)	{ return 0; }

		// Override this to deactivate your plug-in.
		// This gets called when the DLL must be unloaded.
		// You should define this function if you need to perform tasks before
		// your DLL is unloaded.
		// Returns 0 on success.
		DLL2EXE virtual short Deactivate(void)	{ return 0; }

		// Override this to initialize your plug-in.
		// This gets called when the plug-in chooser dialog loads
		// your plug-in.
		// You should define this function if you want to do things
		// as soon as your DLL is mapped into the exe.
		// Returns 0 on success.
		DLL2EXE virtual short Init(void)	{ return 0; }

		// Override this function to trap menu choices.
		// This gets called when the user chooses a menu item in the
		// main dialog's menu.
		// You should define this function if you want to do things
		// based on a certain menu choice.
		// Returns 0 on success.
		DLL2EXE virtual short OnMenuItem(UINT uiIdChoice)	{ return 0; }

		/////////////////////////////////////////////////////////////////////////
		// The following functions are automatically defined by the plug in DLL.
		/////////////////////////////////////////////////////////////////////////

		// These are automagically defined in the DLL.  They return the timestamp of 
		// this file and the size of the CPlugIn structure.  The idea is that the EXE 
		// can then check the values returned by this function against the timestamp of 
		// the file and size of CPlugIn at the time of its compile and determine if 
		// there's a potential synchronization issue with the communication structure.
		DLL2EXE virtual char*	GetDllMagicTime(void);

		DLL2EXE virtual long	GetDllMagicSize(void);

		/////////////////////////////////////////////////////////////////////////
		// The following functions are defined by the EXE.
		/////////////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////////////
		// Instantiable variables:
      /////////////////////////////////////////////////////////////////////////
		HINSTANCE		m_hInstance;			// The instance handle of this DLL.
		long				m_lMagicSize;			// The size of this structure.
		char				m_szMagicTime[256];	// The timestamp of
																				// this header.
		

		// Storage of this object is oversized to accomodate new variables even
		// in older versions.  ALWAYS add new variables before this.
		UCHAR			m_aucReserved[1024];	// NEVER use this space.

      /////////////////////////////////////////////////////////////////////////
		// Static members instantiated in the executable for the DLLs' 
		// convenience.
      /////////////////////////////////////////////////////////////////////////
		static EXE2DLL CImage*			ms_pimBuf;	// The current buffer.
		static EXE2DLL CDialog*			ms_pdlg;		// The main dialog/window.
		static EXE2DLL	CRSPiXBlue*		ms_prspix;	// The RSPiX Blue class.
	};

///////////////////////////////////////////////////////////////////////////////
// Prototypes.
///////////////////////////////////////////////////////////////////////////////

// You MUST delcare this function.
// Returns an instance of your communication object.
DLL2EXE CPlugIn* GetPlugInObject(void);

// These typedefs is used by the Plugger dialog to refer to the function pointer.
typedef CPlugIn*	(GETPLUGINOBJECTFUNC)(void);
typedef long		(GETDLLMAGICSIZE)(CPlugIn*);
typedef char*		(GETDLLMAGICTIME)(CPlugIn*);

// These return values from the EXE for the EXE.  They are not necessarily used
// in the DLL.
inline char* GetMagicTime(void) { return __TIMESTAMP__; }
inline long	GetMagicSize(void) { return sizeof(CPlugIn); }

#endif PLUGIN_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
