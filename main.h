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
// main.h
// Project: Nostril (aka Postal)
//
// History:
//		11/19/96 MJR	Started.
//
//		01/31/97 BRH	Changed MAIN_AUDIO_BUFTIME from 1000 to 100 to decrease
//							the sound effect latency.  I tried 75 but the sound started
//							to break up.
//
//		02/21/97	JMI	Changed MAIN_AUDIO_BUFTIME from 100 to 300 to decrease the
//							likelihood of audio drop-outs.
//
//		02/21/97	JMI	Changed MAIN_AUDIO_BUFTIME back to 100 now that we have new
//							SB driver for NT.
//
//		02/21/97	JMI	Changed MAIN_AUDIO_RATE to 11025 and MAIN_AUDIO_BITS to 16.
//
//		07/13/97	JMI	Added MAIN_VANILLA_AUDIO_* macro overrides to MAIN_AUDIO_*
//							defaults.  These should be used when the INI or default
//							audio mode fails.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MAIN_H
#define MAIN_H

extern int wideScreenWidth;

#define MAIN_SCREEN_DEPTH			8
#define MAIN_SCREEN_MIN_WIDTH		640
#define MAIN_SCREEN_MIN_HEIGHT	480
#define MAIN_SCREEN_PAGES			1
#define MAIN_SCREEN_SCALING		0

#define MAIN_WINDOW_WIDTH			wideScreenWidth
#define MAIN_WINDOW_HEIGHT			480

// This is our default mode.
#define MAIN_AUDIO_RATE				11025
#define MAIN_AUDIO_BITS				16
#define MAIN_AUDIO_CHANNELS		1
#define MAIN_AUDIO_BUFTIME			100
#define MAIN_AUDIO_MAXBUFTIME		2000

// This is our vanilla mode which contains just a couple
// overrides to the default mode.  These should be used when the INI
// or default audio specs fail.
#define MAIN_VANILLA_AUDIO_RATE		22050
#define MAIN_VANILLA_AUDIO_BITS		8

#endif // MAIN_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
