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
////////////////////////////////////////////////////////////////////////////////

#ifndef UNIXBLUEKEYS_H
#define UNIXBLUEKEYS_H

// The SK_ macros are used to index into the array returned by rspScanKeys().
// Note that only "basic" key values are represented here, where "basic" means
// each phyiscal key on the keyboard being pressed by itself.

// WARNING: These macros are dependant on a "standard" keyboard layout!  They
// may not work with all keyboards, especially foreign ones.  For instance, on
// a US keyboard, the key to the left of the '1' key is the 'left quote" key,
// but on European keyboards, it is the '@' key.  Note further that there is no
// SK macro for the '@' key because it is not a "basic" key on the US keybaord
// (you have to press SHIFT with the '2' key to get an '@').

//#define								0
#define RSP_SK_END					1
#define RSP_SK_HOME					2
#define RSP_SK_LEFT					3
#define RSP_SK_UP						4
#define RSP_SK_RIGHT					5
#define RSP_SK_DOWN					6
//#define								7
#define RSP_SK_BACKSPACE			8
#define RSP_SK_TAB					9
#define RSP_SK_INSERT				10
#define RSP_SK_DELETE				11
//#define								12
#define RSP_SK_ENTER					13
#define RSP_SK_LSHIFT				14
#define RSP_SK_SHIFT					15
#define RSP_SK_RSHIFT				16
#define RSP_SK_LCONTROL				17
#define RSP_SK_CONTROL				18
#define RSP_SK_RCONTROL				19
#define RSP_SK_LALT					20
#define RSP_SK_ALT					21
#define RSP_SK_RALT					22
//#define								23
//#define								24
#define RSP_SK_PAGEUP				25
#define RSP_SK_PAGEDOWN				26
#define RSP_SK_ESCAPE				27
#define RSP_SK_PAUSE					28
#define RSP_SK_CAPSLOCK				29
#define RSP_SK_NUMLOCK				30
#define RSP_SK_SCROLL				31
#define RSP_SK_SPACE					32
#define RSP_SK_PRINTSCREEN			33		//???
//#define								34
//#define								35
//#define								36
//#define								37
//#define								38
#define RSP_SK_RQUOTE				'\''	//39
//#define								40
//#define								41
//#define								42
//#define								43
#define RSP_SK_COMMA					','	//44
#define RSP_SK_MINUS					'-'	//45
#define RSP_SK_PERIOD				'.'	//46
#define RSP_SK_SLASH					'/'	//47
#define RSP_SK_0						'0'	//48
#define RSP_SK_1						'1'	
#define RSP_SK_2						'2'	
#define RSP_SK_3						'3'	
#define RSP_SK_4						'4'	
#define RSP_SK_5						'5'	
#define RSP_SK_6						'6'	
#define RSP_SK_7						'7'	
#define RSP_SK_8						'8'	
#define RSP_SK_9						'9'	//57
//#define								58
#define RSP_SK_SEMICOLON			';'	//59
//#define								60
#define RSP_SK_EQUALS				'='	//61
//#define								62
//#define								63
//#define								64
#define RSP_SK_A						'A'	//65
#define RSP_SK_B						'B'	
#define RSP_SK_C						'C'	
#define RSP_SK_D						'D'	
#define RSP_SK_E						'E'	
#define RSP_SK_F						'F'	
#define RSP_SK_G						'G'	
#define RSP_SK_H						'H'	
#define RSP_SK_I						'I'	
#define RSP_SK_J						'J'	
#define RSP_SK_K						'K'	
#define RSP_SK_L						'L'	
#define RSP_SK_M						'M'	
#define RSP_SK_N						'N'	
#define RSP_SK_O						'O'	
#define RSP_SK_P						'P'	
#define RSP_SK_Q						'Q'	
#define RSP_SK_R						'R'	
#define RSP_SK_S						'S'	
#define RSP_SK_T						'T'	
#define RSP_SK_U						'U'	
#define RSP_SK_V						'V'	
#define RSP_SK_W						'W'	
#define RSP_SK_X						'X'	
#define RSP_SK_Y						'Y'	
#define RSP_SK_Z						'Z'	//90
#define RSP_SK_LBRACKET				'['	//91
#define RSP_SK_BACKSLASH			'\\'	//92
#define RSP_SK_RBRACKET				']'	//93
//#define								94
#define RSP_SK_NUMPAD_EQUAL		95				// Mac only!
#define RSP_SK_LQUOTE				'`'	//96	
#define RSP_SK_NUMPAD_0				97	
#define RSP_SK_NUMPAD_1				98	
#define RSP_SK_NUMPAD_2				99	
#define RSP_SK_NUMPAD_3				100	
#define RSP_SK_NUMPAD_4				101	
#define RSP_SK_NUMPAD_5				102	
#define RSP_SK_NUMPAD_6				103	
#define RSP_SK_NUMPAD_7				104	
#define RSP_SK_NUMPAD_8				105	
#define RSP_SK_NUMPAD_9				106	
#define RSP_SK_NUMPAD_ASTERISK	107	
#define RSP_SK_NUMPAD_PLUS			108	
#define RSP_SK_NUMPAD_MINUS		109	
#define RSP_SK_NUMPAD_DECIMAL		110	
#define RSP_SK_NUMPAD_DIVIDE		111	
#define RSP_SK_NUMPAD_ENTER		112			// Mac only!
#define RSP_SK_F1						113	
#define RSP_SK_F2						114	
#define RSP_SK_F3						115	
#define RSP_SK_F4						116	
#define RSP_SK_F5						117	
#define RSP_SK_F6						118	
#define RSP_SK_F7						119	
#define RSP_SK_F8						120	
#define RSP_SK_F9						121	
#define RSP_SK_F10					122
#define RSP_SK_F11					123
#define RSP_SK_F12					124
#define RSP_SK_LSYSTEM				125
#define RSP_SK_SYSTEM				126	
#define RSP_SK_RSYSTEM				127	

// The GKF_ macros are bit masks for the modifier keys returned by rspGetKey().
#define RSP_GKF_SHIFT				0x00010000
#define RSP_GKF_CONTROL				0x00020000
#define RSP_GKF_ALT					0x00040000
#define RSP_GKF_SYSTEM				0x00080000	// Win95 or Mac
#define RSP_GKF_CAPSLOCK			0x00100000


// The RSP_GK_ macros define the extended keys returned by rspGetKey().  They
// are designed to be compared to the lower 16-bits of the returned value.
#define RSP_GK_END					(RSP_SK_END					<< 8)	// Extended
#define RSP_GK_HOME					(RSP_SK_HOME				<< 8)	// Extended
#define RSP_GK_LEFT					(RSP_SK_LEFT				<< 8)	// Extended
#define RSP_GK_UP						(RSP_SK_UP					<< 8)	// Extended
#define RSP_GK_RIGHT					(RSP_SK_RIGHT				<< 8)	// Extended
#define RSP_GK_DOWN					(RSP_SK_DOWN				<< 8)	// Extended

#define RSP_GK_INSERT				(RSP_SK_INSERT				<< 8)	// Extended
#define RSP_GK_DELETE				(RSP_SK_DELETE				<< 8)	// Extended
																
#define RSP_GK_SHIFT					(RSP_SK_SHIFT				<< 8)	// Extended
#define RSP_GK_CONTROL				(RSP_SK_CONTROL			<< 8)	// Extended
#define RSP_GK_ALT					(RSP_SK_ALT					<< 8)	// Extended

#define RSP_GK_PAGEUP				(RSP_SK_PAGEUP				<< 8)	// Extended
#define RSP_GK_PAGEDOWN				(RSP_SK_PAGEDOWN			<< 8)	// Extended

#define RSP_GK_PAUSE					(RSP_SK_PAUSE				<< 8) // Extended
#define RSP_GK_CAPSLOCK				(RSP_SK_CAPSLOCK			<< 8)	// Extended
#define RSP_GK_NUMLOCK				(RSP_SK_NUMLOCK			<< 8)	// Extended
#define RSP_GK_SCROLL				(RSP_SK_SCROLL				<< 8)	// Extended

#define RSP_GK_PRINTSCREEN			(RSP_SK_PRINTSCREEN		<< 8) // Extended	

#define RSP_GK_NUMPAD_0				(RSP_SK_NUMPAD_0			<< 8)	// Extended
#define RSP_GK_NUMPAD_1				(RSP_SK_NUMPAD_1			<< 8)	// Extended
#define RSP_GK_NUMPAD_2				(RSP_SK_NUMPAD_2			<< 8)	// Extended
#define RSP_GK_NUMPAD_3				(RSP_SK_NUMPAD_3			<< 8)	// Extended
#define RSP_GK_NUMPAD_4				(RSP_SK_NUMPAD_4			<< 8)	// Extended
#define RSP_GK_NUMPAD_5				(RSP_SK_NUMPAD_5			<< 8)	// Extended
#define RSP_GK_NUMPAD_6				(RSP_SK_NUMPAD_6			<< 8)	// Extended
#define RSP_GK_NUMPAD_7				(RSP_SK_NUMPAD_7			<< 8)	// Extended
#define RSP_GK_NUMPAD_8				(RSP_SK_NUMPAD_8			<< 8)	// Extended
#define RSP_GK_NUMPAD_9				(RSP_SK_NUMPAD_9			<< 8)	// Extended
#define RSP_GK_NUMPAD_ASTERISK	(RSP_SK_NUMPAD_ASTERISK	<< 8)	// Extended
#define RSP_GK_NUMPAD_PLUS			(RSP_SK_NUMPAD_PLUS		<< 8)	// Extended
#define RSP_GK_NUMPAD_MINUS		(RSP_SK_NUMPAD_MINUS		<< 8)	// Extended
#define RSP_GK_NUMPAD_DECIMAL		(RSP_SK_NUMPAD_DECIMAL	<< 8)	// Extended
#define RSP_GK_NUMPAD_DIVIDE		(RSP_SK_NUMPAD_DIVIDE	<< 8)	// Extended

#define RSP_GK_F1						(RSP_SK_F1					<< 8)	// Extended
#define RSP_GK_F2						(RSP_SK_F2					<< 8)	// Extended
#define RSP_GK_F3						(RSP_SK_F3					<< 8)	// Extended
#define RSP_GK_F4						(RSP_SK_F4					<< 8)	// Extended
#define RSP_GK_F5						(RSP_SK_F5					<< 8)	// Extended
#define RSP_GK_F6						(RSP_SK_F6					<< 8)	// Extended
#define RSP_GK_F7						(RSP_SK_F7					<< 8)	// Extended
#define RSP_GK_F8						(RSP_SK_F8					<< 8)	// Extended
#define RSP_GK_F9						(RSP_SK_F9					<< 8)	// Extended
#define RSP_GK_F10					(RSP_SK_F10					<< 8)	// Extended 
#define RSP_GK_F11					(RSP_SK_F11					<< 8)	// Extended 
#define RSP_GK_F12					(RSP_SK_F12					<< 8)	// Extended 

#define RSP_GK_SYSTEM				(RSP_SK_SYSTEM				<< 8) // Extended


#endif // UNIXBLUEKEYS_H

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
