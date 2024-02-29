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
// Thing3d.h
// Project: Postal
//
// This module implements the CThing3d class which is the class of generic
// thing3d functionality for game thing3ds.
//
// History:
//
//		03/03/97	BRH,JMI	Started this generic thing3d object to reduce the
//							amount of redundant code.
//
//		03/17/97	JMI	Turned most of the CCharacter class into this CThing3d 
//							class.
//
//		03/18/97	JMI	Added DetachChild() and m_u16IdParent.
//
//		03/18/97	JMI	Made On and While functions virtual.
//
//		03/19/97	JMI	Added m_dExtRotVelY, m_dExtRotVelZ, and m_dRotZ.
//							Also, DetachChild() now returns a pointer to the detached
//							child.
//
//		03/21/97	JMI	Added State_Launch, State_LaunchRelease, State_LaunchFinish, 
//							and State_LaunchDone.
//
//		03/27/97	JMI	Added State_GetUp, State_Duck, State_Rise.
//
//		04/02/97	JMI	Removed m_pFire.
//
//		04/07/97	JMI	Added State_Jump, State_JumpForward, State_Fall, 
//							State_Land, and State_LandForward.
//							
//		04/10/97 BRH	Changed GetAttributes to two functions for the new
//							multi layred attribute maps.  Now there is a 
//							GetFloorAttributes and GetEffectAttributes that do
//							lookups on the two different attribute maps.
//
//		04/21/97	JMI	Made MakeValidPosition() virtual.
//
//		04/23/97 BRH	Added a few states for new enemy logic.
//
//		04/25/97	JMI	Added State_Writhing for pre-execute dying dudes.
//
//		04/25/97	JMI	Added a state for executing dudes, State_Execute.
//
//		04/29/97	JMI	Added State_PutOutFire, State_PickUp, and State_PutDown.
//
//		05/06/97 BRH	Added intermediate states for Popout and Run & Shoot
//
//		05/09/97 BRH	Added walk and hide states for ostriches and others.
//
//		05/12/97 BRH	Added a few more states for enemy logic.
//
//		05/14/97	JMI	Added generic form of DetachChild(...).
//							Also, added generic PositionChild(...).
//
//		05/19/97 BRH	Added StateNames array so that the names of the states
//							can be shown for thought balloons and refernced for
//							the logic tables.
//
//		05/20/97 BRH	Added a few more states for victim logic.
//
//		05/21/97 BRH	Added ShootRun state for an alternative to the shoot
//							animation.  They will use this state when shooting while
//							running.
//
//		05/26/97 BRH	Doubled the Default Hit Points to make the enemy guys
//							stronger.
//
//		05/29/97	JMI	Added GetLayer().
//
//		05/30/97	JMI	Added a generic function for adding a force vector to
//							the external force.
//
//		06/02/97 BRH	Added another state for Hunting.
//
//		06/02/97	JMI	Added State_Climb.
//
//		06/05/97	JMI	Removed m_sHitPoints and added a CStockPile, m_stockpile,
//							instead.
//
//		06/13/97	JMI	Added State_ObjectReleased.
//
//		06/15/97	JMI	CThing3d() now Zero()s the m_stockpile b/c I removed its
//							constructor b/c I wanted CStockPile to be an aggregate type.
//
//		06/25/97 BRH	Added 2D shadow sprite which will be drawn if there
//							is a resource for it and it is visible.  Derived classes
//							can call PrepareShadow to load the default shadow and
//							make it visible.  PrepareShadow will first check the
//							shadow sprite's image to make sure it is NULL before
//							loading the default shadow so that you can lad a different
//							shadow resource beforehand.
//
//		06/27/97	JMI	Added GetSprite() which returns this thing's sprite or 
//							NULL.
//
//		07/01/97	JMI	Changed DetachChild() and PositionChild() to receive the
//							rigid body transform as a parameter rather than assume it
//							is m_panimCur->m_transRigid.
//							Also, added GetLinkPoint().
//
//		07/06/97 BRH	Added a few states for victims to use.
//
//		07/09/97	JMI	Added MaxStepUpThreshold, MaxForeVel, and MaxBackVel macro
//							enums.
//
//		07/17/97 BRH	Added DelayShoot state.
//
//		07/18/97	JMI	Added PlaySample() that will hook existing PlaySample()
//							calls in CThing3d derived classes and map them to 
//							PlaySample() and, if they don't specify a volume, it will
//							use this object's distance to the ear.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		08/01/97 BRH	Added new state - AvoidFire and DangerNear
//
//		08/02/97 BRH	Added virtual OnHelpMsg function.
//
//		08/03/97 BRH	Changed DangerNear to Helping state.
//
//		08/06/97 JRD	Added tri-axial local scaling to render process
//
//		08/08/97 BRH	Added virtual GetSmash() function to return the smash
//							of a thing.  Also added MarchNext state.
//
//		08/11/97 BRH	Walk to next bouy.
//
//		08/12/97 BRH	Initialized timers.
//
//		08/18/97	JMI	Moved StartAnim() from CDude to CThing3d so more things
//							could use it.
//
//		08/18/97	JMI	Added m_sLayerOverride which directs Render() to use the
//							specified layer rather than the one based on the 
//							attributes.
//
//		08/24/97	JMI	Changed ms_apt3dAttribCheck to ms_apt2dAttribCheckMedium[]
//							and added ms_apt2dAttribCheckSmall[], 
//							ms_apt2dAttribCheckLarge[], and ms_apt2dAttribCheckHuge[].
//							Also, added pointer so that each object can choose one,
//							m_pap2dAttribCheckPoints.
//							Also, changed type of these arrays to local type Point2D.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef THING3D_H
#define THING3D_H

////////////////////////////////////////////////////////////////////////////////
// Includes.
////////////////////////////////////////////////////////////////////////////////
#include "thing.h"
#include "realm.h"
#include "StockPile.h"
#include "AnimThing.h"

class CThing3d : public CThing
	{
	/////////////////////////////////////////////////////////////////////////////
	// Typedefs/enums.
	/////////////////////////////////////////////////////////////////////////////
	public:
		// Macro within CThing3d namespace.
		enum
			{
			MaxStepUpThreshold	= 15,		// In realm units.
			MaxForeVel				= 80,		// In realm units per second.
			MaxBackVel				= -60,	// In realm units per second.
			DefHitPoints			= 125,
			SoundHalfLife			= 1000	// Default sound half life.
			};

		typedef enum
			{
			State_Idle,					// Not rendering.
			State_Shot,					// Hit by a simple projectile like a bullet.
			State_BlownUp,				// Suffering the reprocussion of an explosion.
			State_Burning,				// Currently on fire.
			State_Die,					// Dying.
			State_Dead,					// Dead.
			State_RunOver,				// Run over by large object such as a car.
			State_Vomit,				// Vomitting.
			State_Suicide,				// Committing suicide.

			State_Persistent,			// Returns to last persistent state.
			State_Stand,				// Standing anim, if any.
			State_Throw,				// Throwing.
			State_ThrowRelease,		// Release thrown object.
			State_ThrowFinish,		// Already released, finishing animation.
			State_ThrowDone,			// Throw done, ready to move on.	 Transitional.
			State_Run,					// Running.
			State_Shooting,			// Shoot.
			State_RunAndShoot,		// Run and shoot.
			State_Strafe,				// Strafing when no forward movement.
			State_StrafeAndShoot,	// Shooting while strafing when no forward movement.
			State_Launch,				// Shoot using launcher weapons.
			State_LaunchRelease,		// Release launched object.
			State_LaunchFinish,		// Already released, finishing animation.
			State_LaunchDone,			// Launch anim done.  Transitional.
			State_GetUp,				// Getting up.
			State_Duck,					// Ducking.
			State_Rise,					// Rising.
			State_Jump,					// Jumping vertically.
			State_JumpForward,		// Jump forward.
			State_Land,					// Landing from vertical momentum only.
			State_LandForward,		// Landing from vertical and forward momentum.
			State_Fall,					// Falling through air.

			State_March,				// Marching band
			State_Mingle,				// Mill around after parade
			State_Panic,				// Run around scared

			State_Load,					// Load your weapon
			State_Patrol,				// Patrol an area waiting for action
			State_Shoot,				// Shoot your weapon
			State_Wait,					// Wait 
			State_Stop,					// Stop walking/running
			State_Hunt,					// Actively seek out a target.
			State_HuntNext,			// Go to next bouy en route to final destination
			State_Engage,				// At the end of the hunt, engage the Dude

			State_Guard,				// Guard an area
			State_Reposition,			// Moving to a new shooting position
			State_Retreat,				// Fall back to a safe position
			State_PopBegin,			// Begin the popout sequence
			State_PopWait,				// Wait at safe location until Dude is near
			State_Popout,				// Popout from a safe Bouy & shoot, then duck back
			State_PopShoot,			// Shoot from the Popped-out location

			State_RunShootBegin,		// Begin run and shoot sequence
			State_RunShootRun,		// Run towards pylon
			State_RunShootWait,		// After you reach the end, wait a bit
			State_RunShoot,			// Run between cover bouys and shoot in between

			State_Delete,				// Delete this thing3d next chance.

			State_Writhing,			// Writhing in predeath, waiting to be executed.

			State_Execute,				// Execute people.

			State_PutDown,				// Put something down.
			State_PickUp,				// Pick something up.

			State_PutOutFire,			// Put out fire.
			
			State_Walk,					// Walk around normally (probably for victims)
			State_Hide,					// Hide so Dudes can't find you
			State_MoveNext,			// Move to next bouy position.
			State_PositionSet,		// Setup to get into position for fighting
			State_PositionMove,		// Move to desired position for fighting
			State_HideBegin,			// Move to hiding pylon
			State_ShootRun,			// Shoot while running - for use with new animations
			State_HuntHold,			// Hold when closest to Dude as possible on the bouy network.

			State_Climb,				// Climbing (probably ladder or something).

			State_ObjectReleased,	// An object was released, finish animation and return
											// to persistent state.
			State_PanicBegin,			// Select a bouy to run to
			State_PanicContinue,		// Continue to panic 
			State_WalkBegin,			// Select a bouy to stroll to
			State_WalkContinue,		// Continue to walk
			State_DelayShoot,			// Wait for timer, then shoot.
			State_AvoidFire,			// Wait for fire to burn out before advancing
			State_Helping,				// When someone nearby gets shot, shoot from where you are
			State_MarchNext,			// March to next bouy.
			State_WalkNext,			// Walk to next bouy

			// Insert additional states above this one.
			NumThing3dStates
			} State;

		// A 2D point.  Used by the attrib check arrays.
		typedef struct
			{
			short	sX;
			short	sZ;
			} Point2D;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		// General position, motion and time variables
		double m_dX;							// x coord.
		double m_dY;							// y coord.
		double m_dZ;							// z coord.
		double m_dVel;							// Velocity.
		double m_dDeltaVel;					// Change in velocity.
		double m_dAcc;							// Acceleration.
		double m_dRot;							// Rotation (in degrees, 0 to 359.999999) around Y.
		double m_dRotZ;						// Rotation (in degrees, 0 to 359.999999) around Z.
		double m_dScaleX;						// X-scale relative to scene scale
		double m_dScaleY;						// Y-scale relative to scene scale
		double m_dScaleZ;						// Z-scale relative to scene scale
		double m_dDrag;						// Amount of deceleration that applies to internal
													// forces.
		double m_dExtHorzVel;				// Velocity due to external force on X/Z plane.
		double m_dExtHorzDeltaVel;			// Change in external force velocity.
		double m_dExtHorzRot;				// Direction of m_dExternalHorzVel.
		double m_dExtHorzDrag;				// Amount of deceleration that applies to external
													// forces.
		double m_dExtVertVel;				// Vertical velocity due to external force.
		double m_dExtVertDeltaVel;			// Change in external force vertical velocity.
		double m_dExtRotVelY;				// Rate of rotation in degrees per second around
													// Y axis.
		double m_dExtRotVelZ;				// Rate of rotation in degrees per second around
													// Z axis.
		long   m_lTimer;						// General purpose timer for states
		long   m_lPrevTime;					// Previous update time
		short  m_sPrevHeight;				// Previous height
		short  m_sSuspend;					// Suspend flag

		U16	m_u16IdFire;					// ID of fire to carry around when you are burning.

		CSprite3 m_sprite;					// 3D Sprite used to render the 3D Thing.
		CSprite2	m_spriteShadow;			// 2D shadow sprite to be shown on the ground

		CSmash	m_smash;						// Smash for collision detection.
		
		// Animation specific variables.
		CAnim3D*	m_panimCur;					// Pointer to current animation.
		long		m_lAnimTime;				// Time from start of animation.
		long		m_lAnimPrevUpdateTime;	// Last time m_lAnimTime was updated.
													// Used to determine the delta time to add
													// to m_lAnimTime.
		RTransform	m_trans;					// Transform to apply on Render.

		State	m_state;							// Current state of this thing3d.

		short	m_sBrightness;					// Normal brightness level or dark if burnt

		bool	m_bAboveTerrain;				// true, if in the air, false if on terrain.

		U16	m_u16IdParent;					// Instance ID of parent.

		CStockPile	m_stockpile;			// Stockpile of ammo and health.

		short	m_sLayerOverride;				// Layer override.  If > 0, used instead of
													// layer dictated by attributes.

		const Point2D*	m_pap2dAttribCheckPoints;	// Points to the ms_apt3dAttribCheck*[]
																// that best reflects this object.


	//---------------------------------------------------------------------------
	// Static Variables
	//---------------------------------------------------------------------------
	public:
		static double	ms_dDefaultSurfaceDrag;	// Default drag along surfaces.
		static double	ms_dDefaultAirDrag;		// Default drag due to air friction.
		static short	ms_sBurntBrightness;		// Brightness level after being burnt
		static char*	ms_apszStateNames[];		// Strings describing states, indexed by
															// the state enum.

		// These are arrays of pts to be checked on the attribute map for various
		// size of CThing3d derived things.
		static const Point2D		ms_apt2dAttribCheckSmall[];
		static const Point2D		ms_apt2dAttribCheckMedium[];
		static const Point2D		ms_apt2dAttribCheckLarge[];
		static const Point2D		ms_apt2dAttribCheckHuge[];

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CThing3d(CRealm* pRealm, CThing::ClassIDType id)
			: CThing(pRealm, id)
			{
			// Must call Zero() to initialize the stockpile since it has
			// no constructor.
			m_stockpile.Zero();

			m_state				= State_Idle;
			m_panimCur			= NULL;
			m_dExtHorzVel		= 0.0;
			m_dExtHorzRot		= 0.0;
			m_dExtHorzDrag		= 0.0;
			m_dExtVertVel		= 0.0;
			m_dExtRotVelY		= 0.0;
			m_dExtRotVelZ		= 0.0;
			m_dVel				= 0.0;
			m_dAcc				= 0.0;
			m_dRot				= 0.0;
			m_dRotZ				= 0.0;
			m_dScaleX			= 1.0;
			m_dScaleY			= 1.0;
			m_dScaleZ			= 1.0;
			m_dDrag				= 0.0;
			m_sprite.m_pthing	= this;
			m_sSuspend			= 0;
			m_sBrightness		= 0;
			m_u16IdFire			= CIdBank::IdNil;
			m_bAboveTerrain	= false;
			m_stockpile.m_sHitPoints	= DefHitPoints;
			m_u16IdParent		= CIdBank::IdNil;
			m_spriteShadow.m_sInFlags = CSprite::InHidden;
			m_spriteShadow.m_pImage = NULL;
			m_spriteShadow.m_pthing = this;
			m_lAnimTime = 0;
			m_lTimer = 0;
			m_sLayerOverride	= -1;

			// Default to the standard.
			m_pap2dAttribCheckPoints	= ms_apt2dAttribCheckMedium;
			}

	public:
		// Destructor
		~CThing3d()
			{
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_sprite);
			// Remove sprite from scene (this is safe even if it was already removed!)
			m_pRealm->m_scene.RemoveSprite(&m_spriteShadow);
			// Remove smash from smashatorium (this is safe even if it was already 
			// removed).
			m_pRealm->m_smashatorium.Remove(&m_smash);
			// Free the shadow resource
			if (m_spriteShadow.m_pImage)
				rspReleaseResource(&g_resmgrGame, &(m_spriteShadow.m_pImage));
			}

	//---------------------------------------------------------------------------
	// Required static functions - None for this non-instantiable object.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Not necessarily required virtual functions (implementing them as inlines 
	// doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Shutdown object
		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Render object
		void Render(void);

		// Called by editor to init new object at specified position
		short EditNew(												// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to modify object
		short EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		short EditMove(											// Returns 0 if successfull, non-zero otherwise
			short sX,												// In:  New x coord
			short sY,												// In:  New y coord
			short sZ);												// In:  New z coord

		// Called by editor to render object
		void EditRender(void);

		// Give Edit a rectangle around this object
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		// (virtual (Overridden here)).
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.

		// Get the sprite for this thing.  If there's more than one, pick one
		// or none to return.
		virtual	// If you override this, do NOT call this base class.
		CSprite* GetSprite(void)	// Returns the sprite for this thing or NULL.
			{ return &m_sprite; }

		// Get the coordinates of this thing.
		virtual					// Overriden here.
		double GetX(void)	{ return m_dX; }

		virtual					// Overriden here.
		double GetY(void)	{ return m_dY; }

		virtual					// Overriden here.
		double GetZ(void)	{ return m_dZ; }

		// Returns a pointer to the smash
		virtual
		CSmash* GetSmash(void)
		{
			return &m_smash;
		}

	//---------------------------------------------------------------------------
	// Useful generic thing3d message-specific functionality.
	//---------------------------------------------------------------------------
	public:

		// Process all messages currently in the message queue through 
		// ProcessMessage().
		virtual			// Override to implement additional functionality.
		void ProcessMessages(void);

		// Process the specified message.  For most messages, this function
		// will call the equivalent On* function.
		virtual			// Override to implement additional functionality.
		void ProcessMessage(		// Returns nothing.
			GameMessage* pmsg);	// Message to process.

		// Message handling functions ////////////////////////////////////////////

		// Handles a Shot_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnShotMsg(					// Returns nothing.
			Shot_Message* pshotmsg);	// In:  Message to handle.

		// Handles an Explosion_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnExplosionMsg(							// Returns nothing.
			Explosion_Message* pexplosionmsg);	// In:  Message to handle.

		// Handles a Burn_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnBurnMsg(					// Returns nothing.
			Burn_Message* pburnmsg);	// In:  Message to handle.

		// Handles an ObjectDelete_Message.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnDeleteMsg(								// Returns nothing.
			ObjectDelete_Message* pdeletemsg);	// In:  Message to handle.

		// Handles a call for help message Help_Message
		virtual			// Override to implement additional functionality
							// Call base class to get default functionality
		void OnHelpMsg(								// Returns nothing
			Help_Message* phelpmsg);				// In:  Message to handle

		// Handles the put me down message
		virtual			// Override to implemetn additional functionality
							// Call base class to get default functionality
		void OnPutMeDownMsg(							// Returns nothing
			PutMeDown_Message* pputmedownmsg);	// In:  Message to handle

	//---------------------------------------------------------------------------
	// Useful generic thing3d state-specific functionality.
	//---------------------------------------------------------------------------
	public:

		// Implements basic one-time functionality for each time State_Shot is
		// entered.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void OnShot(void);

		// Implements basic functionality while being shot and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileShot(void);

		// Implements basic functionality while being blown up and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileBlownUp(void);	// Returns true until state is complete.

		// Implements basic functionality while being on fire and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileBurning(void);	// Returns true until state is complete.

		// Implements basic functionality while being run over and returns true
		// until the state is completed.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool WhileRunOver(void);	// Returns true until state is complete.

	//---------------------------------------------------------------------------
	// Useful generic thing3d functionality.
	//---------------------------------------------------------------------------
	public:

		// Applies accelerations to velocities keeping them within specified
		// limits.
		void UpdateVelocities(		// Returns nothing.
			double	dSeconds,		// Seconds since last update.
			double	dMaxForeVel,	// Maximum forward velocity.
			double	dMaxBackVel);	// Maximum backward velocity.

		// Applies velocities to positions.
		void GetNewPosition(					// Returns nothing.
			double*	pdNewX,					// Out: New x position.
			double*	pdNewY,					// Out: New y position.
			double*	pdNewZ,					// Out: New z position.
			double	dSeconds);				// Seconds since last update.
//#ifdef MOBILE
		// Applies velocities to positions, but take in angle of movement
		void GetNewPositionAngle(				// Returns nothing.
				double*	pdNewX,					// Out: New x position.
				double*	pdNewY,					// Out: New y position.
				double*	pdNewZ,					// Out: New z position.
				double	dSeconds,               // Seconds since last update.
				double  dAngle
				);
//#endif
		// Determines if supplied position is valid tweaking it if necessary.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		bool MakeValidPosition(				// Returns true, if new position was validitable.
													// Returns false, if could not reach new position.
			double*	pdNewX,					// In:  x position to validate.
													// Out: New x position.
			double*	pdNewY,					// In:  y position to validate.
													// Out: New y position.
			double*	pdNewZ,					// In:  z position to validate.
													// Out: New z position.
			short	sVertTolerance = 0);		// Vertical tolerance.

		// Deluxe does all for updating position.
		void DeluxeUpdatePosVel(	// Returns nothing.
			double dSeconds);			// In:  Duration since last update in seconds.

		// Update fire animation's position.
		void UpdateFirePosition(void);

		// Get Floor attributes (height & floor attributes like nowalk, ladder, cliff)
		void GetFloorAttributes(	// Returns nothing
			short  sX,					// In:  X coord.
			short  sZ,					// In:  Z coord.
			U16*   pu16Attrib,		// Out: Combined attributes, if not NULL
			short* psHeight);			// Out: Max height, if not NULL

		// Get Effect attributes (effects attributes like light, camera, oil, blood)
		void GetEffectAttributes(	// Returns nothing
			short  sX,					// In:  X coord.
			short  sZ,					// In:  Z coord.
			U16*   pu16Attrib,		// Out: Combined attributes, if not NULL
			short* psLightBits);		// Out: Tally of light bits set, if not NULL.

		// Get the layer based on the attribute points array.
		void GetLayer(
			short  sX,					// In:  X coord.
			short  sZ,					// In:  Z coord.
			short* psLayer);			// Out: Combined layer.

		// Detach the specified Thing3d.
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		CThing3d* DetachChild(			// Returns ptr to the child or NULL, if none.
			U16*		pu16InstanceId,	// In:  Instance ID of child to detach.
												// Out: CIdBank::IdNil.
			RTransform*	ptrans);			// In:  Transform for positioning child.

		// Detach the specified child sprite (can be any sprite type).
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void DetachChild(					// Returns nothing.
			CSprite*		psprite,			// In:  Child sprite to detach.
			RTransform*	ptrans,			// In:  Transform for positioning child.
			double*		pdX,				// Out: New position of child.
			double*		pdY,				// Out: New position of child.
			double*		pdZ);				// Out: New position of child.


		// Position the specified child sprite (can be any sprite type).
		virtual			// Override to implement additional functionality.
							// Call base class to get default functionality.
		void PositionChild(				// Returns nothing.
			CSprite*	psprite,				// In:  Child sprite to detach.
			RTransform*	ptrans,			// In:  Transform for positioning child.
			double*	pdX,					// Out: New position of child.
			double*	pdY,					// Out: New position of child.
			double*	pdZ);					// Out: New position of child.

		// Get the link point specified by the provided transform.
		void GetLinkPoint(				// Returns nothing.
			RTransform*	ptrans,			// In:  Transform specifying point.
			double*	pdX,					// Out: Point speicfied.
			double*	pdY,					// Out: Point speicfied.
			double*	pdZ);					// Out: Point speicfied.


		// Add a force vector to this thing's external horizontal velocity vector.
		void AddForceVector(			// Returns nothing.
			double	dAddVel,			// In:  Magnitude of additional vector.
			short		sRot);			// In:  Direction (in degrees) of additional vector.

		// Load the default shadow resource unless one has already been loaded
		// and set the shadow to visible
		virtual			// Override to implement additional functionality
							// Call base class to get default functionality
		short PrepareShadow(void);	// Returns SUCCESS if set successfully

		// Plays a sample and purges the resource after playing (as long as nobody
		// else has used the same sample resource).  
		// Plays a sample with volume adjustment.  This may require load from disk.
		// Also, if no initial volume is specified, the distance to the ear is used.
		void PlaySample(										// Returns nothing.
																	// Does not fail.
			SampleMasterID	id,								// In:  Identifier of sample you want played.
			SampleMaster::SoundCategory eType,			// In:  Sound Volume Category for user adjustment
			short	sInitialVolume	= -1,						// In:  Initial Sound Volume (0 - 255)
																	// Negative indicates to use the distance to the
																	// ear to determine the volume.
			SampleMaster::SoundInstance*	psi = NULL,	// Out: Handle for adjusting sound volume
			long* plSampleDuration = NULL,				// Out: Sample duration in ms, if not NULL.
			long lLoopStartTime = -1,						// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
			long lLoopEndTime = 0,							// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
			bool bPurgeSample = false);					// In:  Call ReleaseAndPurge rather than Release after playing


		// Start a CAnimThing.
		CAnimThing* StartAnim(					// Returns ptr to CAnimThing on success; NULL otherwise.
			char* pszAnimResName,				// In:  Animation's resource name.
			short	sX,								// In:  Position.
			short	sY,								// In:  Position.
			short	sZ,								// In:  Position.
			bool	bLoop);							// In:  true to loop animation.


	};

#endif	// THING3D_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
