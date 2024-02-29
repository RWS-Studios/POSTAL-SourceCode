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
// credits.h
// Project: Nostril (aka Postal)
//
// History:
//		12/04/96 MJR	Started.
//
//		08/11/97	JRD	Transforming this module into a device for scolling text
//							which is intended to be used both by credits and by story.
//							It wil operate similar to cutscene in that all of it's
//							assets and memory usage is assumed temporary.  It will still
//							use the shell sak for assets.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CREDITS_H
#define CREDITS_H
#include "SampleMaster.h"


// Display the credits, returns SUCCESS or FAILURE
extern short Credits(SampleMasterID* pMusic = NULL,
							char*	pszBackground = NULL,
							char* pszCredits = NULL);

// For general usage
extern short	ScrollPage(char* pszBackground,char* pszScrollScript,
								  double dScrollRate = 0.0,RRect *prWindow = NULL);




#endif //CREDITS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
