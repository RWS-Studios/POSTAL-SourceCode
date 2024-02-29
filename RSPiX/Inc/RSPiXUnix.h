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
//
//	"RSPiXUnix.h"
// 
// History:
//		06/01/04 RCG Initial implementation.
//
//////////////////////////////////////////////////////////////////////////////
//
// This file #includes all the headers for RSPiX*.lib.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Try to group by color for easy look up.  This may not always be possible,
// but it should be b/c the any green headers that require orange headers
// should already include them.
//
// This is a good place to define platform specific macros and such that don't
// go into system.h (I'm not sure if there could be any, but...).
//
//////////////////////////////////////////////////////////////////////////////
#ifndef RSPiX_H
	#error You must include the platform independent RSPiX.H before RSPiXUnix.h one.
#endif // RSPiX_H

//////////////////////////////////////////////////////////////////////////////
// Blue headers.
//////////////////////////////////////////////////////////////////////////////
#include "BLUE/unix/UnixBlue.h"

//////////////////////////////////////////////////////////////////////////////
// Cyan headers.
//////////////////////////////////////////////////////////////////////////////
#include "CYAN/Unix/UnixCyan.h"

//////////////////////////////////////////////////////////////////////////////
// Green headers.
//////////////////////////////////////////////////////////////////////////////
#include "GREEN/BLiT/BLIT.H"
#include "GREEN/Hot/hot.h"
#include "GREEN/Image/Image.h"
#include "GREEN/Mix/mix.h"
#include "GREEN/Sample/sample.h"
#include "GREEN/Snd/snd.h"
#include "GREEN/SndFx/SndFx.h"
#include "GREEN/Task/task.h"
#include "GREEN/3D/user3d.h"
#include "GREEN/BLiT/alphablit.h"

//////////////////////////////////////////////////////////////////////////////
// Orange headers.
//////////////////////////////////////////////////////////////////////////////
#include "ORANGE/CDT/listbase.h"
#include "ORANGE/CDT/List.h"
#include "ORANGE/CDT/slist.h"
#include "ORANGE/CDT/QUEUE.H"
#include "ORANGE/CDT/fqueue.h"
#include "ORANGE/CDT/stack.h"
#include "ORANGE/CDT/flist.h"
#include "ORANGE/DirtRect/DirtRect.h"
#include "ORANGE/GameLib/ANIMSPRT.H"
#include "ORANGE/GameLib/Region.h"
#include "ORANGE/GameLib/Shapes.h"
#include "ORANGE/GameLib/SPRITE.H"
#include "ORANGE/File/file.h"
#include "ORANGE/IFF/iff.h"
#include "ORANGE/GUI/dlg.h"
#include "ORANGE/GUI/btn.h"
#include "ORANGE/GUI/txt.h"
#include "ORANGE/GUI/edit.h"
#include "ORANGE/GUI/scrollbar.h"
#include "ORANGE/GUI/ListBox.h"
#include "ORANGE/GUI/PushBtn.h"
#include "ORANGE/GUI/MultiBtn.h"
#include "ORANGE/Meter/meter.h"
#include "ORANGE/MsgBox/MsgBox.h"
#include "ORANGE/Attribute/attribute.h"
#include "ORANGE/ImageTools/lasso.h"
#include "ORANGE/Laymage/laymage.h"
#include "ORANGE/MTask/mtask.h"
#include "ORANGE/RString/rstring.h"
#include "ORANGE/Channel/channel.h"
#include "ORANGE/color/colormatch.h"
#include "ORANGE/color/dithermatch.h"
#include "ORANGE/str/str.h"
#include "ORANGE/GUI/ProcessGui.h"

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
