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
// message.h
// Project: Postal
//
//	History
//
//		02/15/97 BRH	Started this file to include a few messages that we will
//							start out with.  All of the messages have the type first
//							so that if they are cast as a Generic Message, you can 
//							look at the type first and do a switch statement to 
//							cast it to the correct type of message struct.  
//
//		02/16/97 BRH	We talked about doing a union of the structs but I'm
//							not sure how that makes it easier to use rather than
//							doing the casting like DirectPlay.  Also I was wondering
//							if these should have constructors that default their type
//							variable to their specific type rather than making the
//							person who creates a struct fill in the type.
//
//		02/17/97 BRH	Created another message structure with a union of the
//							other message types and included the type enum in
//							that structure.  Changed the Thing to use a MessageQueue
//							of this new structure rather than passing the generic
//							message structure pointer and casting it to the specific
//							type of message.
//
//		02/18/97	JMI	Moved MessageType enum to global scope.
//
//		02/19/97 BRH	Added ObjectDelete message which tells an object to
//							kill itself immediately (ie. delete this)
//
//		02/19/97 BRH	Changed the bogus extra data in the messages like
//							burn heat, explosion force, bullet caliber to sDamage 
//							so that all of these can give some kind of game relative
//							damage amount.
//
//		02/24/97	JMI	Added generic notification message with generic parameters.
//							Useful for simple messages within an object and stuff like
//							that where it seems like defining a whole new message
//							would be silly.
//
//		03/04/97	JMI	Well, I guess it wasn't that silly after all.  Added a
//							DrawBlood_Message which I should've done to begin with
//							instead of adding that silly notification message.
//
//		03/05/97 BRH	Added center and velocity of explosion to the explosion
//							message.  Also added panic message which is sent by 
//							victims to other victims to warn them.
//
//		03/05/97	JMI	Added Suicide_Message.
//
//		03/06/97	JMI	Added msg_Suicide to GameMessage union.
//
//		03/19/97 BRH	Added Trigger message so that the CDude can trigger
//							the remote controlled mine or other weapons that we
//							may decide require an external trigger.
//
//		04/20/97 BRH	Added bouy strategy suggestion messages.  Bouys can
//							be assigned a suggestion message that an enemy can
//							ask the bouy to send in order to get information from
//							the bouy to perform certain local specific logic.
//
//		04/20/97 BRH	Added load and save functions to messages so that the
//							bouys could use them to save and load the messages.
//
//		05/02/97 BRH	Added a message for the dude trigger which informs
//							a pylon that a CDude stepped on the trigger area.
//							Also changed the Popout and ShootCycle messages to 
//							contain more information.
//
//		06/09/97 BRH	Added Messages for CDude to send to the CDemon to inform
//							of his actions like selecting a new weapon and firing
//							a weapon.
//
//		06/10/97 BRH	Added Death and Writhing messages so CPerson can send
//							CDemon messsages for comments.
//
//		06/11/97 BRH	Added u16ShooterID to the weapon messages so that we can
//							identify the shooter of the weapon so that credit can
//							be given to whoever gets a kill using a weapon.
//
//		08/01/97 BRH	Added a cheater message to send to the Demon when someone
//							enters a cheat code.
//
//		08/02/97 BRH	Added a call for help message that enemies send to each other
//							when they are getting shot.
//
//		08/28/97 BRH	Added PutMeDown message that the flag sends to the dude 
//							when it finds the flagbase.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MESSAGE_H
#define MESSAGE_H

#include "RSPiX.h"
//#include "dude.h"

////////////////////////////////////////////////////////////////////////////////
// Message Types
////////////////////////////////////////////////////////////////////////////////

typedef unsigned char MessageType;

enum
{
	typeGeneric,
	typeShot,
	typeExplosion,
	typeBurn,
	typeKill,
	typeObjectDelete,
	typeNotification,		// Generic notification with generic parameters for
								// reuse.  Delete this silly message!
	typeDrawBlood,			// Message indicating blood should be drawn on the
								// background at the specified location.
	typePanic,
	typeSuicide,			// Receiver should commit suicide.
	typeTrigger,			// Used to trigger remote control mines
	typePopout,				// Used by bouy to give direction and distance for popout logic
	typeShootCycle,		// Used by bouy to give next bouy ID for cycling between
	typeSafeSpot,			// Used by bouy to identify itself as a "behind cover" bouy
	typeDudeTrigger,		// Used by attribute trigger to tell pylons that a Dude is near
	typeWeaponSelect,		// Used by CDude to tell CDemon when a new weapon has been selected
	typeWeaponFire,		// Used by CDude to tell CDemon when a weapon has been fired.
	typeWrithing,			// Used by CPerson to tell CDemon when a person is writhing
	typeDeath,				// Used by CPerson to tell CDemon when a person has died
	typeCheater,			// Used by CDude to tell CDemon when a person entered a cheat code
								// so that it can mock them.
	typeHelp,				// Used by CPerson to tell others that it it getting shot
	typePutMeDown,			// Used by flag to tell CDude to put it down in the base.

	typeEndOfMessages
};


////////////////////////////////////////////////////////////////////////////////
// Message structures
////////////////////////////////////////////////////////////////////////////////

typedef struct tag_MESSAGE_GENERIC
{
	MessageType eType; // = typeGeneric
	short			sPriority;
} Generic_Message, *pGeneric_Message;

typedef struct tag_MESSAGE_SHOT
{
	MessageType eType;// = typeShot;
	short			sPriority;
	short			sDamage;
	short			sAngle;
	U16			u16ShooterID; // Instance ID of the shooter (for scoring)
} Shot_Message, *pShot_Message;

typedef struct tag_MESSAGE_EXPLOSION
{
	MessageType eType; // = typeExplosion;
	short			sPriority;
	short			sDamage;
	short			sX;			// Center of explosion
	short			sY;			// Center of explosion
	short			sZ;			// Center of explosion
	short			sVelocity;	// Relative size of explosion 
	U16			u16ShooterID;// Instance ID of the shooter
} Explosion_Message, *pExplosion_Message;

typedef struct tag_MESSAGE_BURN
{
	MessageType eType; // = typeBurn;
	short			sPriority;
	short			sDamage;
	U16			u16ShooterID;	// Instance ID of the shooter
} Burn_Message, *pBurn_Message;

typedef struct tag_MESSAGE_OBJECTDELETE
{
	MessageType eType; // = typeObjectDelete
	short			sPriority;
} ObjectDelete_Message, *pObjectDelte_Message;

typedef struct tag_MESSAGE_POPOUT
{
	MessageType eType; // = typePopout
	short			sPriority;
	UCHAR			ucIDNext;			// Next Pylon to run to
	U16			u16UniqueDudeID;	// Dude to target
	U16			u16UniquePylonID;	// Easier way to get to pylon data
	short			sNextPylonX;		// Next pylon X position
	short			sNextPylonZ;		// Next pylon Z position
} Popout_Message, *pPopout_Message;

typedef struct tag_MESSAGE_SHOOTCYCLE
{
	MessageType eType; // = typeShootCycle
	short			sPriority;
	UCHAR			ucIDNext;			// ID of next bouy in cycle
	U16			u16UniqueDudeID;	// Dude to target
	U16			u16UniquePylonID;	// Easier way to get to pylon data
	short			sNextPylonX;		// Next pylon X position
	short			sNextPylonZ;		// Next pylon Z position
} ShootCycle_Message, *pShootCycle_Message;

typedef struct tag_MESSAGE_SAFESPOT
{
	MessageType eType; // = typeSafeSpot
	short			sPriority;
} SafeSpot_Message, *pSafeSpot_Message;

typedef struct tag_MESSAGE_PANIC
{
	MessageType eType;	//typePanic
	short			sPriority;
	short			sX;		// Center of panic
	short			sY;		// Center of panic
	short			sZ;		// Center of panic
} Panic_Message, *pPanic_Message;

typedef struct
{
	MessageType	eType;	// = typeDrawBlood
	short			sPriority;
	short			s2dX;		// 2D X position on background to draw blood.
	short			s2dY;		// 2D Y position on background to draw blood.
} DrawBlood_Message;

typedef struct
{
	MessageType	eType;	// = typeSuicide
	short			sPriority;
} Suicide_Message;

typedef struct tag_MESSAGE_TRIGGER
{
	MessageType eType;	// = typeTrigger
	short			sPriority;
} Trigger_Message;

typedef struct tag_MESSAGE_DUDETRIGGER
{
	MessageType eType; // = typeDudeTrigger
	short		   sPriority;
	U16			u16DudeUniqueID;
	double		dX;
	double		dZ;
} DudeTrigger_Message;

typedef struct tag_MESSAGE_WEAPONSELECT
{
	MessageType	eType; // = typeWeaponSelect
	short			sPriority;
	short			sWeapon; //CDude::WeaponType eWeapon;
} WeaponSelect_Message;

typedef struct tag_MESSAGE_WEAPONFIRE
{
	MessageType	eType; // = typeWeaponFire
	short			sPriority;
	short			sWeapon; //CDude::WeaponType	eWeapon;
} WeaponFire_Message;

typedef struct tag_MESSAGE_WRITHING
{
	MessageType	eType; // = typeWrithing
	short			sPriority;	
} Writhing_Message;

typedef struct tag_MESSAGE_DEATH
{
	MessageType	eType; // = typeDeath
	short			sPriority;
} Death_Message;

typedef struct tag_MESSAGE_CHEATER
{
	MessageType eType;// = typeCheater
	short			sPriority;
} Cheater_Message;

typedef struct tag_MESSAGE_HELP
{
	MessageType eType; // = typeHelp
	short			sPriority;
} Help_Message;

typedef struct tag_MESSAGE_PUTMEDOWN
{
	MessageType eType;// = typePutMeDown
	short			sPriority;
	U16			u16FlagInstanceID;
} PutMeDown_Message;

typedef struct tag_GameMessage
{
union
	{
		Generic_Message		msg_Generic;
		Shot_Message			msg_Shot;
		Explosion_Message		msg_Explosion;
		Burn_Message			msg_Burn;
		ObjectDelete_Message	msg_ObjectDelete;
		DrawBlood_Message		msg_DrawBlood;
		Panic_Message			msg_Panic;
		Suicide_Message		msg_Suicide;
		Trigger_Message		msg_Trigger;
		Popout_Message			msg_Popout;
		ShootCycle_Message	msg_ShootCycle;
		SafeSpot_Message		msg_SafeSpot;
		DudeTrigger_Message	msg_DudeTrigger;
		WeaponSelect_Message msg_WeaponSelect;
		WeaponFire_Message	msg_WeaponFire;
		Writhing_Message		msg_Writhing;
		Death_Message			msg_Death;
		Cheater_Message		msg_Cheater;
		Help_Message			msg_Help;
		PutMeDown_Message		msg_PutMeDown;

	};

	// Function to save whatever type of message this is.
	short Save(RFile* pFile)
	{
		short sResult = 0;

		if (pFile && pFile->IsOpen())
		{
			pFile->Write(&msg_Generic.eType);
			pFile->Write(&msg_Generic.sPriority);

			switch (msg_Generic.eType)
			{
				case typeGeneric:
				case typeObjectDelete:
				case typeSafeSpot:
				case typeSuicide:
				case typeTrigger:
				case typeWeaponSelect:
				case typeWeaponFire:
				case typeWrithing:
				case typeDeath:
				case typeCheater:
				case typeHelp:
					break;

				case typeShot:
					pFile->Write(&msg_Shot.sDamage);
					pFile->Write(&msg_Shot.sAngle);
					break;

				case typeExplosion:
					pFile->Write(&msg_Explosion.sDamage);
					pFile->Write(&msg_Explosion.sX);
					pFile->Write(&msg_Explosion.sY);
					pFile->Write(&msg_Explosion.sZ);
					pFile->Write(&msg_Explosion.sVelocity);
					break;

				case typeBurn:
					pFile->Write(&msg_Burn.sDamage);
					break;

				case typePopout:
				case typeShootCycle:
					pFile->Write(&msg_Popout.ucIDNext);
					pFile->Write(&msg_Popout.u16UniqueDudeID);
					pFile->Write(&msg_Popout.u16UniquePylonID);
					pFile->Write(&msg_Popout.sNextPylonX);
					pFile->Write(&msg_Popout.sNextPylonZ);
					break;

				case typePanic:
					pFile->Write(&msg_Panic.sX);
					pFile->Write(&msg_Panic.sY);
					pFile->Write(&msg_Panic.sZ);
					break;

				case typeDrawBlood:
					pFile->Write(&msg_DrawBlood.s2dX);
					pFile->Write(&msg_DrawBlood.s2dY);
					break;

				case typeDudeTrigger:
					pFile->Write(&msg_DudeTrigger.u16DudeUniqueID);
					pFile->Write(&msg_DudeTrigger.dX);
					pFile->Write(&msg_DudeTrigger.dZ);
					break;

				case typePutMeDown:
					pFile->Write(&msg_PutMeDown.u16FlagInstanceID);
					break;
			}		
		}
		else
		{
			sResult = -1;
		}

		return sResult;
	}

	// Function to load whatever type of message was saved
	short Load(RFile* pFile)
	{
		short sResult = 0;

		if (pFile && pFile->IsOpen())
		{
			pFile->Read(&msg_Generic.eType);
			pFile->Read(&msg_Generic.sPriority);

			switch (msg_Generic.eType)
			{
				case typeGeneric:
				case typeObjectDelete:
				case typeSafeSpot:
				case typeSuicide:
				case typeTrigger:
					break;
					
				case typeShot:
					pFile->Read(&msg_Shot.sDamage);
					pFile->Read(&msg_Shot.sAngle);
					break;

				case typeExplosion:
					pFile->Read(&msg_Explosion.sDamage);
					pFile->Read(&msg_Explosion.sX);
					pFile->Read(&msg_Explosion.sY);
					pFile->Read(&msg_Explosion.sZ);
					pFile->Read(&msg_Explosion.sVelocity);
					break;

				case typeBurn:
					pFile->Read(&msg_Burn.sDamage);
					break;

				case typePopout:
				case typeShootCycle:
					pFile->Read(&msg_Popout.ucIDNext);
					pFile->Read(&msg_Popout.u16UniqueDudeID);
					pFile->Read(&msg_Popout.u16UniquePylonID);
					pFile->Read(&msg_Popout.sNextPylonX);
					pFile->Read(&msg_Popout.sNextPylonZ);
					break;

				case typePanic:
					pFile->Read(&msg_Panic.sX);
					pFile->Read(&msg_Panic.sY);
					pFile->Read(&msg_Panic.sZ);
					break;

				case typeDrawBlood:
					pFile->Read(&msg_DrawBlood.s2dX);
					pFile->Read(&msg_DrawBlood.s2dY);
					break;

				case typeDudeTrigger:
					pFile->Read(&msg_DudeTrigger.u16DudeUniqueID);
					pFile->Read(&msg_DudeTrigger.dX);
					pFile->Read(&msg_DudeTrigger.dZ);
					break;

				case typePutMeDown:
					pFile->Read(&msg_PutMeDown.u16FlagInstanceID);
					break;
			}
		}
		else
		{
			sResult = FAILURE;
		}

		return sResult;
	}
} GameMessage, *pGameMessage;


////////////////////////////////////////////////////////////////////////////////
// Message union
////////////////////////////////////////////////////////////////////////////////


#endif //MESSAGE_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
