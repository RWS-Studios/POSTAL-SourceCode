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
// This is a quick include macro to get the user all 
// headers he needs to work with 3d stuff:

// 3d stuff:
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/QuickMath/VectorMath.h"
	#include "GREEN/3D/types3d.h"
	#include "GREEN/3D/zbuffer.h"
	#include "GREEN/3D/render.h"
	#include "GREEN/3D/pipeline.h"
#else
	#include "vectormath.h"
	#include "types3d.h"
	#include "zbuffer.h"
	#include "render.h"
	#include "pipeline.h"
#endif
