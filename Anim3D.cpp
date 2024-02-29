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
// Anim3D.CPP
// Project: Postal
// 
// History:
//		05/23/97 JMI	Started.  Moved here from thing.h.
//							Also, added events.
//
//		08/01/97 BRH	Took out loading/releasing .hot channels since
//							we aren't using them.
//
//		08/07/97	JMI	Added m_ptransWeapon rigid body transforms channel for
//							weapons.
//							Also, added SetLooping().
//							Also, removed all (including commented out) references to
//							hots and floors.
//
//		08/11/97	JMI	Now has TRACEs to complain when an animation fails to 
//							load.
//							Also, on the first component to fail, no more are loaded.
//							Also, if any fail to load, all that were loaded are
//							released.
//
//		08/12/97 BRH	Added yet another overloaded version of Get which
//							takes a number of a texture file to be loaded.
//
//		12/17/97	JMI	Now if the sTextureScheme values is less than zero, no 
//							number is used (i.e., the default texture scheme is used).
//
//		10/07/99	JMI	Added conditional compile options for some error messages.
//
//////////////////////////////////////////////////////////////////////////////
//
// 3D animation class that is a collection of channels that make up a 3D
// animation.
//
//////////////////////////////////////////////////////////////////////////////
#define ANIM3D_CPP

#include "RSPiX.h"

#include "Anim3D.h"
#include "game.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Default constructor.
////////////////////////////////////////////////////////////////////////////////
CAnim3D::CAnim3D()
	{
	m_psops			= NULL;
	m_pmeshes		= NULL;
	m_ptextures		= NULL;
	m_pbounds		= NULL;
	m_ptransRigid	= NULL;
	m_pevent			= NULL;
	m_ptransWeapon	= NULL;
	}

////////////////////////////////////////////////////////////////////////////////
// Get the various components of this animation from the resource names
// specified in the provided array of pointers to strings.
// (virtual)
////////////////////////////////////////////////////////////////////////////////
short CAnim3D::Get(				// Returns 0 on success.
	char**	ppszFileNames)		// Pointer to array of pointers to filenames.
										// These filenames should be in the order
										// the members are listed in this class's
										// definition.
	{
	short	sRes;
	short	sComplainIndex;	// If sRes is non-zero, this is the index of the
									// resname that could not load.

	// NOTE:  If you add any new channel loads here, please make sure
	// you maintain the order described above.
	sComplainIndex	= 0;
	sRes	=  rspGetResource(&g_resmgrGame, ppszFileNames[sComplainIndex], &m_psops);
	if (sRes == 0)
		{
		sComplainIndex	= 1;
		sRes	= rspGetResource(&g_resmgrGame, ppszFileNames[sComplainIndex], &m_pmeshes);
		if (sRes == 0)
			{
			sComplainIndex	= 2;
			sRes	= rspGetResource(&g_resmgrGame, ppszFileNames[sComplainIndex], &m_ptextures);
			if (sRes == 0)
				{
				sComplainIndex	= 4;
				sRes	= rspGetResource(&g_resmgrGame, ppszFileNames[sComplainIndex], &m_pbounds);
				if (sRes == 0)
					{
					sComplainIndex	= 6;
					if (ppszFileNames[sComplainIndex] != NULL)
						{
						sRes	= rspGetResource(&g_resmgrGame, ppszFileNames[sComplainIndex], &m_ptransRigid);
						}
					}
				}
			}
		}

	// No current support for events since that might break existing
	// arrays of size 6 elements.

	// No current support for weapon transforms either.

	// If an error occurred . . .
	if (sRes != 0)
		{
#if defined(VERBOSE)
		// Complain.
		TRACE("Get(): Unable to load \"%s\".\n", ppszFileNames[sComplainIndex] );
#endif
		// Clean.
		Release();
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Get the various components of this animation from the resource names
// specified in the provided array of pointers to strings.
// (virtual)
////////////////////////////////////////////////////////////////////////////////
short CAnim3D::Get(				// Returns 0 on success.
	char**	ppszFileNames,		// Pointer to array of pointers to filenames.
										// These filenames should be in the order
										// the members are listed in this class's
										// definition.
	short		sLoopFlags)			// Looping flags to apply to all channels in this anim
	{
	short	sRes	= Get(ppszFileNames);
	// If successful . . .
	if (sRes == 0)
		{
		SetLooping(sLoopFlags);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Get the various components of this animation from the resource names
// specified by base name, optionally, with a rigid name.
// (virtual)
////////////////////////////////////////////////////////////////////////////////
short CAnim3D::Get(					// Returns 0 on success.
	char*		pszBaseFileName,		// In:  Base string for resource filenames.
	char*		pszRigidName,			// In:  String to add for rigid transform channel,
											// "", or NULL for none.
	char*		pszEventName,			// In:  String to add for event states channel,
											// "", or NULL for none.
	char*		pszWeaponTransName,	// In:  String to add for weapon transforms channel,
											// "", or NULL for none.
	short		sLoopFlags)				// In:  Looping flags to apply to all channels
											// in this anim.
	{
	short	sRes;
	char	szResName[RSP_MAX_PATH];
	sprintf(szResName, "%s.sop", pszBaseFileName);
	sRes	=  rspGetResource(&g_resmgrGame, szResName, &m_psops);
	if (sRes == 0)
		{
		sprintf(szResName, "%s.mesh", pszBaseFileName);
		sRes	= rspGetResource(&g_resmgrGame, szResName, &m_pmeshes);
		if (sRes == 0)
			{
			sprintf(szResName, "%s.tex", pszBaseFileName);
			sRes	= rspGetResource(&g_resmgrGame, szResName, &m_ptextures);
			if (sRes == 0)
				{
				sprintf(szResName, "%s.bounds", pszBaseFileName);
				sRes	= rspGetResource(&g_resmgrGame, szResName, &m_pbounds);
				if (sRes == 0)
					{
					if (pszRigidName != NULL)
						{
						if (*pszRigidName != '\0')
							{
							sprintf(szResName, "%s_%s.trans", pszBaseFileName, pszRigidName);
							sRes	= rspGetResource(&g_resmgrGame, szResName, &m_ptransRigid);
							}
						}

					if (sRes == 0)
						{
						if (pszEventName != NULL)
							{
							if (*pszEventName != '\0')
								{
								sprintf(szResName, "%s_%s.event", pszBaseFileName, pszEventName);
								sRes	= rspGetResource(&g_resmgrGame, szResName, &m_pevent);
								}
							}

						if (sRes == 0)
							{
							if (pszWeaponTransName != NULL)
								{
								if (*pszWeaponTransName != '\0')
									{
									sprintf(szResName, "%s_%s.trans", pszBaseFileName, pszWeaponTransName);
									sRes	= rspGetResource(&g_resmgrGame, szResName, &m_ptransWeapon);
									}
								}
							}
						}
					}
				}
			}
		}

	// If successful . . .
	if (sRes == 0)
		{
		SetLooping(sLoopFlags);
		}
	else
		{
#if defined(VERBOSE)
		TRACE("Get(): Unable to load \"%s\".\n", szResName);
#endif
		// Clean.
		Release();
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Get the various components of this animation from the resource names
// specified by base name, optionally, with a rigid name.
// (virtual)
////////////////////////////////////////////////////////////////////////////////
short CAnim3D::Get(					// Returns 0 on success.
	char*		pszBaseFileName,		// In:  Base string for resource filenames.
	char*		pszVerb,					// In:  Action name to be appended to the base
	char*		pszRigidName,			// In:  String to add for rigid transform channel,
											// "", or NULL for none.
	char*		pszEventName,			// In:  String to add for event states channel,
											// "", or NULL for none.
	char*		pszWeaponTransName,	// In:  String to add for weapon transforms channel,
											// "", or NULL for none.
	short		sLoopFlags)				// In:  Looping flags to apply to all channels
											// in this anim.
	{
	char	szVerbedBaseName[RSP_MAX_PATH];
	sprintf(szVerbedBaseName, "%s_%s", pszBaseFileName, pszVerb);
	
	return Get(szVerbedBaseName, pszRigidName, pszEventName, pszWeaponTransName, sLoopFlags);
	}

////////////////////////////////////////////////////////////////////////////////
// Get the various components of the animation using the given resource names,
// but load only 1 .tex file based on the color scheme number passed in.
////////////////////////////////////////////////////////////////////////////////
short CAnim3D::Get(					// Returns 0 on success.
	char*		pszBaseFileName,		// In:  Base string for resource filenames.
	short		sTextureScheme,		// In:  Number to append after name for texture file
	char*		pszVerb,					// In:  Action name to be appended to the base
	char*		pszRigidName,			// In:  String to add for rigid transform channel,
											// "", or NULL for none.
	char*		pszEventName,			// In:  String to add for event states channel,
											// "", or NULL for none.
	char*		pszWeaponTransName,	// In:  String to add for weapon transforms channel,
											// "", or NULL for none.
	short		sLoopFlags)				// In:  Looping flags to apply to all channels
											// in this anim.
{
	char	szVerbedBaseName[RSP_MAX_PATH];
	sprintf(szVerbedBaseName, "%s_%s", pszBaseFileName, pszVerb);

	short	sRes;
	char	szResName[RSP_MAX_PATH];
	sprintf(szResName, "%s.sop", szVerbedBaseName);
	sRes	=  rspGetResource(&g_resmgrGame, szResName, &m_psops);
	if (sRes == 0)
		{
		sprintf(szResName, "%s.mesh", szVerbedBaseName);
		sRes	= rspGetResource(&g_resmgrGame, szResName, &m_pmeshes);
		if (sRes == 0)
			{
			// If there's an associated texture scheme . . .
			if (sTextureScheme >= 0)
				{
				sprintf(szResName, "%s%d.tex", pszBaseFileName, sTextureScheme);
				}
			else
				{
				sprintf(szResName, "%s.tex", szVerbedBaseName);
				}

			sRes	= rspGetResource(&g_resmgrGame, szResName, &m_ptextures);
			if (sRes == 0)
				{
				sprintf(szResName, "%s.bounds", szVerbedBaseName);
				sRes	= rspGetResource(&g_resmgrGame, szResName, &m_pbounds);
				if (sRes == 0)
					{
					if (pszRigidName != NULL)
						{
						if (*pszRigidName != '\0')
							{
							sprintf(szResName, "%s_%s.trans", szVerbedBaseName, pszRigidName);
							sRes	= rspGetResource(&g_resmgrGame, szResName, &m_ptransRigid);
							}
						}

					if (sRes == 0)
						{
						if (pszEventName != NULL)
							{
							if (*pszEventName != '\0')
								{
								sprintf(szResName, "%s_%s.event", szVerbedBaseName, pszEventName);
								sRes	= rspGetResource(&g_resmgrGame, szResName, &m_pevent);
								}
							}

						if (sRes == 0)
							{
							if (pszWeaponTransName != NULL)
								{
								if (*pszWeaponTransName != '\0')
									{
									sprintf(szResName, "%s_%s.trans", szVerbedBaseName, pszWeaponTransName);
									sRes	= rspGetResource(&g_resmgrGame, szResName, &m_ptransWeapon);
									}
								}
							}
						}
					}
				}
			}
		}

	// If successful . . .
	if (sRes == 0)
		{
		SetLooping(sLoopFlags);
		}
	else
		{
#if defined(VERBOSE)
		TRACE("Get(): Unable to load \"%s\".\n", szResName);
#endif
		// Clean.
		Release();
		}

	return sRes;
}


////////////////////////////////////////////////////////////////////////////////
// Release all resources.
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CAnim3D::Release(void)	// Returns nothing.
	{
	if (m_psops != NULL)
		rspReleaseResource(&g_resmgrGame, &m_psops);

	if (m_pmeshes != NULL)
		rspReleaseResource(&g_resmgrGame, &m_pmeshes);

	if (m_ptextures != NULL)
		rspReleaseResource(&g_resmgrGame, &m_ptextures);

	if (m_pbounds != NULL)
		rspReleaseResource(&g_resmgrGame, &m_pbounds);

	if (m_ptransRigid != NULL)
		rspReleaseResource(&g_resmgrGame, &m_ptransRigid);

	if (m_pevent != NULL)
		rspReleaseResource(&g_resmgrGame, &m_pevent);

	if (m_ptransWeapon)
		rspReleaseResource(&g_resmgrGame, &m_ptransWeapon);
	}

////////////////////////////////////////////////////////////////////////////////
// Set looping flags for this channel.
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CAnim3D::SetLooping(		// Returns nothing.
	short		sLoopFlags)			// In:  Looping flags to apply to all channels
										// in this anim.
	{
	m_psops->SetLooping(sLoopFlags);
	m_pmeshes->SetLooping(sLoopFlags);
	m_ptextures->SetLooping(sLoopFlags);
	m_pbounds->SetLooping(sLoopFlags);
	if (m_ptransRigid != NULL)
		m_ptransRigid->SetLooping(sLoopFlags);
	if (m_pevent != NULL)
		m_pevent->SetLooping(sLoopFlags);
	if (m_ptransWeapon)
		m_ptransWeapon->SetLooping(sLoopFlags);
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
