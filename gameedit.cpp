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
// GameEdit.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the game editor.
//
// History:
//		12/18/96 MJR	Started.
//
//		01/14/97 BRH	Added CDoofus and CTkachuk keys to the editor
//
//		01/17/97 BRH	Added CRocketMan key to the editor
//
//		01/21/97	JMI	Converted from rspGetKey() and rspGetMouseEvent() to
//							RInputEvent API (rspGetNextInputEvent() ).
//
//		01/21/97	JMI	Added Realm Bar GUI and Object Picker GUI.
//							Now open and save realm invoke file system dialogs
//							to browse.
//
//		01/22/97	JMI	Now requests that you save before doing anything that
//							might be detremental(sp!) to the current level (e.g., New,
//							Close, etc.).
//
//		01/22/97	JMI	PlayRealm() now saves and loads the current level in 
//							TEMP_FILE_NAME.
//
//		01/22/97	JMI	The object picker list box now only adds items that are
//							user creatable in the editor (as indicated by 
//							CThing::ms_aClassInfo[ID].bEditorCreatable.
//
//		01/22/97 MJR	Added use of realm's time object.  Also, it now uses a
//							separate realm object in the play loop so as to avoid any
//							possible interference between it and the editor's realm.
//
//		01/23/97	JMI	Fixed 'assing' comment because we definitely wouldn't
//							want the word 'assing' appearing in our code anywhere.
//							Who's the assinine jack ass for a dumb ass that wrote
//							'assing' in the first place anyway?!  What an asswipe.
//							Now you are only asked if you want to save before an op
//							that destroys the current realm if that realm contains
//							anything.
//							Now hots for individual GUIs are called independently in
//							the editor.
//
//		01/23/97	JMI	Now when you click an item in the list box, the item is
//							created, the EditNew() is called, and then the item can
//							be moved to the drop spot.
//
//		01/23/97	JMI	Now shows the system cursor when not placing an object.
//
//		01/24/97	JMI	Can now adjust display size with Numpad + and -.  Always
//							restores display size when exiting editor to the original
//							size (when entered editor).  
//							External effects:  g_pimScreenBuf.
//
//		01/24/97	JMI	Now PlayRealm() also supports the adjustment of the display
//							area.  Also, cursor no longer does height cursor's drag
//							logic unless in drop dude mode.
//
//		01/25/97	JMI	Added scrollbars to edit mode.
//
//		01/26/97	JMI	Forgot to update scrollbars in PlayRealm().
//
//		01/26/97	JMI	Added selection.  This required creating a root RHot for
//							the Hood and a child RHot for each of the other CThings.
//
//		01/27/97	JMI	Now you can double click an existing item to move it,
//							hit enter while it is selected to modify it, or hit
//							delete while it is selected to delete it.
//
//		01/27/97	JMI	Now if you delete the item with the selection and then try
//							to move it, it doesn't crash.  A technological breakthrough
//							enabled this by collapsing ms_p*Move into ms_p*Sel and 
//							adding an ms_sMoving flag.
//
//		01/28/97	JMI	Now you can toggle layers on and off using the layers list-
//							box.
//
//		01/29/97	JMI	Quickly changed UI...will update cleaner later...just need
//							to get this to Bill so he can do cool Buoy stuff.
//
//		01/29/97 BRH	Added ability to link Bouys.  It draws the link between
//							two bouy's that you are attempting to link.  Also added
//							a CNavigationNet pointer to the editor to keep track
//							of the currently selected NavNet to which new Bouys 
//							are added.  Will need to add a GUI item to let the user
//							select the 'current' NavNet when you have more than one.
//							Also still need to add the function to display the lines
//							between all bouys once the network is linked together.
//
//		01/30/97	JMI	Now when you open/load or new a realm, the size affected
//							stuff is updated via a call to SizeUpdate().
//
//		01/30/97 BRH	Added key 'B' as a toggle for showing the bouy lines.
//							Added a set of bouy lines to the editor which can be
//							added to when a new line is drawn or completely set
//							up with all bouy lines by calling UpdateNetLines.  
//							Now I need the CThing unique ID numbers so that when
//							bouys are reloaded, they will know what CNavigationNet
//							to relink themselves to.
//
//		01/30/97	JMI	Now the editor assigns a realm unique ID to every item it 
//							creates.
//
//		01/31/97	JMI	If CreateNewThing() failed to create a new CBuoy b/c there
//							was no current NavNet, it returned success.  Now it returns
//							failure.
//
//		02/02/97	JMI	If a CThing derived class has an EditHotSpot() defined, the
//							editor will be able to smoothly drag the thing.  If it does
//							not have this function, the cursor will jump when a drag
//							is started.
//							Also, made cursor aware of the height on the attribute map.
//							Also, made the distinction between an editor hotbox event
//							and another event.
//
//		02/03/97	JMI	Moved all input in editor loop into DoInput() and all output
//							in editor loop to DoOutput().  This makes it a little easier
//							to figure out what's going on, but DoInput() is still rather
//							large (mainly due to the two switch statements).
//
//		02/03/97	JMI	Can track a particular CThing during PlayRealm().  To track
//							a CThing, hold down shift and then click the thing.  There
//							should be a beep signifying the item was set as the tracked
//							thing.  Hit 'T' in PlayRealm() mode to toggle tracking mode
//							vs. scrollbar mode.
//
//		02/03/97	JMI	Now loads GUIs and cursor images via 
//							FullPath(GAME_PATH_VD, "...") and uses temp path 
//							FullPath(GAME_PATH_HD, "...") for realms in PlayRealm().
//							In PlayRealm(), the grip is now reset when switching from
//							scrollbar to track target mode.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/04/97	JMI	SizeUpdate() was directly accessing members of CHood.  Now
//							calls CHood.GetWidth(),GetHeight() instead.
//
//		02/04/97	JMI	You can now change the game's priority level via the keys
//							'1', '2', & '3' where '1' is the highest priority and
//							'3' is lowest.
//
//		02/05/97 BRH	NewRealm now automatically creates a NavNet for the realm
//							right after it creates a Hood.  Many objects depend on the
//							NavNet object being in place before they are added so
//							the editor now creates one by default.
//
//		02/05/97	JMI	The toggle of layers is now stimulated by the actual list-
//							box item, rather than the "Toggle Button" which I will
//							remove from the LAYERS.GUI after checking this in.
//
//		02/06/97 BRH	Checked for CBouy and CNavigationNet when deleting things
//							in the editor.  You may not delete the last remaining
//							NavNet.  If you delete a Bouy, it will unlink the bouy from
//							its network first and update the lines if necessary.  If
//							you delete a NavNet, it will delete all of its bouys.
//
//		02/07/97	JMI	Added use of new CGameEditThing.
//
//		02/12/97	JMI	Added pause feature in PlayRealm().  Also, added keys for
//							speeding up, slowing down, and normalizing game time, but
//							have not implemented that yet b/c it requires a change to
//							CTime.
//
//		02/12/97	JMI	Temp fix until GetMaxHeight() returns *4 value (or the *4
//							an app thing?).  Also, fixed bug where I was getting the
//							height via the cursor's screen pos (not the cursor's world
//							pos).
//
//		02/13/97	JMI	Now the editor sets the Camera's Hood.  This fixed the prob
//							the grip was having finding the realm limits and allows us
//							to utilize the newest scene which gets its alpha stuff from
//							the hood passed to it by the camera.  The camera's connect-
//							ed to the  realm bone, ... .
//
//		02/13/97	JMI	In the previous update, the camera's hood was being set
//							after the load realm, but too late.  Now it's set earlier.
//							Since before, it was never set, it was never a problem b/c
//							the camera's hood was NULL.
//
//		02/13/97	JMI	On the second entrance to GameEdit(), ms_pcameraCur was
//							already pointing at an old deleted camera.
//							Also, I guess the camera and realm were not being delete'd
//							in GameEdit().
//
//		02/14/97	JMI	SizeUpdate() now makes sure the camera's hood is up to 
//							date.
//		02/17/97	JMI	Now checks rspGetQuitStatus().
//
//		02/19/97	JMI	PlayRealm() now checks for camera tracking object 
//							iteratively.
//
//		02/20/97	JMI	Now uses input interface for user input.
//
//		02/21/97	JMI	Now makes sure scrollbars don't have focus in PlayRealm().
//							Also, makes all CDudes X-Rayable.
//
//		02/22/97	JMI	Now SetDisplayArea() only changes the display device
//							settings if g_GameSettings.m_sUseCurrentDeviceDimensions
//							is FALSE.
//
//		02/22/97	JMI	Now sets working dir to levels dir when entering editor.
//
//		02/24/97	JMI	CloseRealm() now purges g_resmgrGame.
//
//		02/25/97	JMI	CloseRealm() now also purges prealm->m_resmgr.
//							Also, CloseRealm() now clears ms_pcameraCur's hood ptr
//							and calls SizeUpdate().
//
//		02/25/97	JMI	Resized viewable area and repositioned GUIs into non-
//							viewable areas.
//							Also, added SetCameraArea() function which will resize the
//							device mode and the display area such that the camera area
//							will be the specified size.
//
//		02/28/97	JMI	Added satellite cameras.  No current way to resize them
//							though.  That kinda sux.  But they're there.
//
//		03/03/97	JMI	Now cameras work in PlayRealm() mode.
//							Also, now, on entry to GameEdit(), stores device settings
//							and uses them in restoring the display area on exit.
//							Previously, only the display area was restored with the
//							device mode as returned by rspSuggestVideoMode().
//
//		03/05/97	JMI	Dude now commits suicide when user exits PlayRealm().
//							CreateNewThing() now uses CThing::ConstructWithID() to
//							create new things.
//
//		03/06/97	JMI	PlayRealm() now sends a suicide message to the local dude
//							instead of setting his state.
//
//		03/06/97 BRH	Added NetLog() function for debugging NavNet links.  Prints
//							out a text file of connected bouys.  
//
//		03/09/97	JMI	Added GUIs bar for showing/hiding GUIs, NavNets listbox,
//							and Map GUI.
//							Also, CloseRealm() now clears ms_pgething.
//
//		03/10/97	JMI	Now using correct rspBlitT scale version so map actually 
//							gets blit (but transparent for color 0 (which it should
//							not contain)).
//
//		03/10/97	JMI	Now you can click on the map to move the main camera to the
//							clicked area.
//
//		03/10/97 BRH	Added a route table printout to NetLog function to verify
//							that the routing tables are correct.
//
//		03/13/97 BRH	Made a change to the route table printout.
//
//		03/13/97	JMI	Now calls CDude::DrawStatus() for the local dude.
//
//		03/17/97	JMI	Now scrolls the view if a dragged item is on or beyond an
//							edge.
//							Also, now cancels current drag in CloseRealm() and before
//							calling  PlayRealm().
//
//		03/19/97	JMI	Added flag indicating the view should not be scrolled while
//							dragging/moving a CThing.  This is utilized when a CThing
//							is first created from the CThing pick list so it doesn't
//							cause immediate scrolling.  It is re-enabled as soon as the
//							thing is dragged into the view, destroyed, or placed.
//							Also, now the view will start scrolling within 
//							DRAG_SCROLL_BUFFER pixels of any view edge.
//
//		03/19/97	JMI	Switched to the new input event hotbox callback for thing
//							hotboxes.
//
//		03/19/97 BRH	Made sure network line draw is turned off when the current
//							realm is closed so that lines don't show up on the next
//							realm and it doesn't crash when you quit the editor with
//							lines turned on.
//
//		03/24/97	JMI	PlayRealm() now passes a ptr to the realm to GetLocalInput.
//
//		03/25/97	JMI	Changed CURSOR_BASE_IMAGE_FILE, CURSOR_TIP_IMAGE_FILE, and
//							PICK_OBJECT_FILE to 8.3 compliant names.
//
//		03/27/97	JMI	ThingHotCall now makes sure the event it processes has not
//							yet been used.  Also, you may no longer drag the hood.
//
//		03/28/97	JMI	Now the scrollbar tray and button clicks will advance more
//							than just a few pixels.
//							Now, if there is no current camera focus, it is set to the
//							first CDude created.
//
//		03/28/97	JMI	Fixed exit button.  Also my last change made change had 
//							caused the Hood hotbox to parent itself which caused hosed
//							behavior.  Fixed.
//							
//		03/28/97	JMI	Now resets directory back to original when done.
//
//		04/03/97	JMI	Main dude's status is now displayed below the view (instead
//							of on it).
//
//		04/04/97	JMI	ms_sbVert and ms_sbHorz are now smooth scrollers.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps using the helper functions.
//
//		04/11/97	JMI	Changed GetLocalInput() to take the RInputEvent.
//
//		04/16/97 BRH	Added an #ifdef REALM_NONSTL for testing the new
//							non-stl lists for the CThings in the realm.  References
//							to either method are available by changing the
//							REALM_NONSTRL macro in realm.h
//
//		04/17/97 BRH	Fixed a bug in the new list code where it wasn't 
//							advancing the list pointer at the end of the loop, cauing
//							an infinite loop.
//
//		04/17/97 BRH	Changed call to Startup and Shutdown in realm, changed
//							calls in this file from 1 parameter to 0 parameters.
//
//		04/21/97	JMI	Menu is now activated by escape key.
//
//		04/21/97 BRH	Bouys are now linked by right clicking on the bouy, or
//							on the Mac, hold down open-apple & click to connect
//							bouy lines.  Double click on a bouy now brings up
//							its EditModify dialog box.
//
//		04/22/97	JMI	No longer uses chdir() to get the open dialog into the
//							correct directory.  Now we simply default ms_szFileName
//							to the path of the dir we want to save under.
//
//		04/22/97	JMI	Removed #include <direct.h>.
//
//		04/29/97	JMI	Added key motivated new item placement.
//
//		05/01/97	JMI	Added ability to create regions to be associated with 
//							pylons.  These regions are saved with the .RLM with the
//							same name except the .RLM is changed to .RGN.
//							To edit a pylon's region, double right click it, use left
//							and right mouse buttons to paint, Num pad + and - to
//							control brush size, and Escape to end trigger region paint
//							mode.
//							Also, added focus next and prev.
//							Also, made enlarge/reduce screen during PlayRealm() update
//							the CDude's status area.
//
//		05/02/97	JMI	Now, while editing a pylon's trigger region, it is drawn
//							with a game sprite so we can have alpha so you can see
//							what you're drawing on.
//
//		05/02/97	JMI	Changed number of pylons trigger regions to 256 b/c I didn't
//							want to bother adding 1 in all the special locations.
//
//		05/04/97 BRH	Removed #ifdef sections referring to STL lists.
//
//		05/09/97	JMI	Assigned pylon instance IDs into ms_argns[].u16InstanceId 
//							so the multigrid for pylon triggers can contain a mapping
//							from pylon ID to instance ID.
//
//		05/12/97 JRD	Added the creation of the Trigger Attribute map into the
//							save realm process.
//
//		05/15/97	JMI	Added a common SaveRealm(char*) that does all the stuff
//							necessary to save the realm and triggers so that the
//							PlayRealm() gets everything done that would happen normally.
//
//		05/23/97 BRH	Added support for NavNets in the editor's list box.  
//							Added code for button callbacks to switch the current
//							Nav Net, and to remove them from the list when they are
//							deleted.
//
//		05/25/97	JMI	Holding down CONTROL, ALT, and/or SHIFT provides different
//							variations on the amount pylon regions are moved by arrow
//							keys.
//
//		05/26/97	JMI	If you hold down CONTROL while hitting DEL, the editor will
//							delete all objects of the type of the currently selected
//							objects.
//
//		05/29/97	JMI	Removed references to m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/07/97	JMI	Now, in PlayRealm(), if no dude exists, we attempt to use
//							CWarp::WarpInAnywhere() to create one.  This will fail, if 
//							there are no warps in the specified realm.
//							Changed message for CTRL-DEL group deletes as per 
//							humiliation.
//							Added realm status info to PlayRealm().
//
//		06/09/97	JMI	Now generates a unique temp filename for PlayRealm() that
//							is generated at least in part by the current OS.
//
//		06/11/97 MJR	If mouse is enabled (via InputSettings) then the mouse
//							cursor is now hidden and then restored while playing
//							the realm.
//
//		06/12/97	JMI	Commented out use of rspSetDoSystemMode() in PlayRealm().
//
//		06/16/97	JMI	Now passes destination buffer in StartMenu() call.
//
//		06/16/97	JMI	Added copy/paste.
//
//		06/16/97	JMI	PlayRealm() now calls the realm's hood's SetPalette().
//							It also clears the key status array.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/26/97	JMI	Now enables RMix's autopump feature during the PlayRealm()
//							and disables it before exiting.
//
//		06/26/97	JMI	DrawCursor() now uses the realm's Map3Dto2D().
//
//		06/26/97	JMI	Now editor's cursor is fully 3D and there are two functions
//							for mapping between screen & realm coords 
//							(MapScreen2Realm() and Maprealm2Screen() ).
//
//		06/30/97	JMI	Converted to use new m_pLayers member of CScene (was using
//							m_layers).
//
//					MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//					JMI	Now all bouy lines are mapped through Maprealm2Screen()
//							before being drawn.
//
//		07/03/97	JMI	Converted calls to rspOpen/SaveBox() to new parm 
//							conventions.
//
//		07/05/97	JMI	Now you can clear the current camera focus thing by
//							SHIFT-Left-clicking on the hood.
//
//		07/05/97 MJR	Changed to RSP_BLACK_INDEX instead of 0.
//
//		07/07/97	JMI	Added extremely CHEEZY thing that checks for an active 
//							input settings menu and, if so, calls EditInputSettings().
//
//		07/07/97	JMI	Now SetCameraArea() sets the display area based on user
//							settings from g_GameSettings.  Also, SetDisplayArea()
//							always updates these settings when the user changes the 
//							display area.
//
//		07/07/97 BRH	Added processing for the properties button on the Realm
//							bar which calls the realm's EditModify function to put
//							up its properties dialog box for the realm.
//
//					MJR	Added regular plus and minus in addition to numpad versions
//							for changing display size.
//
//		07/10/97	JMI	Now you cannot select an item in the 'PickObj' list when
//							you are dragging an item.
//
//		07/14/97	JMI	Now there is a place to put exceptions to what can be 
//							copied (via Copy/Paste).  Currently, you cannot copy
//							Bouys, Pylons, NavNets, and Hoods.
//
//		07/15/97	JMI	Now you can display the attributes with a green mist over
//							the hood by checking boxes in 'View Attributes'.  You can
//							also use NUMPAD_MULTIPLY and NUMPAD_DIVIDE to vary the
//							opacity of this mist.
//
//		07/16/97	JMI	Can now take snap shots from editor.  Also, F3 does auto
//							X ray.
//
//		07/18/97	JMI	Now updates the sound location with the camera target.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/19/97	JMI	Now calls InitLocalInput() in PlayRealm() just prior to
//							starting.
//
//		07/20/97	JMI	Now uses back/foreground callbacks to suspend/resume the
//							realm (PlayRealm() only), adjust CPU hoggage, and make
//							sure the cursor is visible when in the background.
//
//		07/20/97	JMI	DelThing() was dereferencing the ptr to the object to be
//							deleted after it was cleared by the special cases that
//							don't allow deletion.  Fixed.
//
//		07/20/97 MJR	Added conditional compile to disable save & play.
//							Added use of RFile callback during load to call Update().
//
//		07/20/97	JMI	Fixed syntax error produced when we added the above
//							conditional compile option.
//
//		07/21/97	JMI	PlayRealm() now sets the input mode to live via 
//							SetInputMode().
//
//		07/22/97	JMI	Now shows the info for the currently selected thing.
//
//		07/22/97	JMI	Now PlayRealm() displays the "display info" (FPS, etc.).
//
//		07/23/97	JMI	Changed EDIT_KEY_TRACKING_TOGGLE to CONTROL T (was T).
//
//		07/25/97 BRH	Fixed problem of last bouy link causing the editor
//							to crash when the highlight was left active.  Also fixed
//							bug that crashed the editor if you hit space bar before
//							loading a hood.
//
//		07/25/97 MJR	Made separate function to get temp file names and added
//							mac-specific version of that code.
//							Also added message box if there's an error playing the
//							realm.
//
//		07/27/97	JMI	Now maps pylon trigger regions' initial positions from 3D
//							to 2D viewing plane.
//							Also, added '=' as another option for enlarging the display
//							since most games don't actually require you to hit SHIFT to
//							get the actual '+'.
//							Also, converted movement keys for pylon trigger regions to
//							use the key status array (instead of 
//							rspGetNextInputEvent() ).
//							Also, CTRL, ALT, and SHIFT speed ups for movement now speed
//							up draw block sizing.
//
//		07/30/97 BRH	Changed the toggle bouy key to turn off the bouys in 
//							addition to the bouy lines.
//
//		07/31/97	JMI	Now allows you to turn off clipping to the realm edges
//							via the '?' key so you can scroll off the world looking
//							for lost souls or whatever.
//							Also, added realm statistics activated by 'I' key in both
//							edit and play modes.
//
//		07/31/97 BRH	Made the hots on the bouys inactive when they are hidden.
//
//		07/31/97	JMI	Now shows the cursor, if hidden, in ShowRealmStatistics()
//							and restores its show level when done.  Also, clears inputs
//							before returning.
//							Also, PlayRealm() Suspend()s the realm before calling
//							ShowRealmStatistics() and Resume()s it afterward.
//
//		07/31/97	JMI	Changed EDIT_KEY_REALM_STATISTICS to F11 (was 'I') so it
//							wouldn't interfere with typing cheats.
//
//		08/01/97	JMI	Changed %.2f sprintf format specifiers to %g.  It's not as
//							pretty but it's more compact.  The problem if the %f is it
//							will put hundreds of 0's after the decimal point if it 
//							needs to display a very small number.
//
//		08/02/97	JMI	Now makes sure the CBouys' show/hidden status is in synch
//							with the ms_bDrawNetwork flag (the flag defaults to false
//							and the bouys default to shown).
//							Also, now does not actiavte bouys' hots when loading a 
//							level while ms_bDrawNetwork is false.
//							Also, if a new bouy is created while ms_bDrawNetwork is 
//							false, its hot is initially inactive.
//
//		08/05/97 BRH	Defaulted the bouy network to ON.
//
//		08/06/97	JMI	Changed instances of InitLocalInput() to ClearLocalInput().
//
//		08/07/97	JMI	Now PlayRealm() and GameEdit() set the appropriate realm
//							flags so things can know what we're doing.
//
//		08/08/97 MJR	Moved background/foreground callbacks to game.cpp.
//
//		08/08/97 BRH	Fixed display of NavNet names in the listbox after loading,
//							also clears them on CloseRealm.
//
//		08/09/97	JMI	Now shows wait cursor during CRealm::Load() calls.
//
//		08/09/97	JMI	Added some pretty UNdeluxe, ugly, not-too-useful load/save
//							progress feedback.
//
//		08/10/97	JMI	Now allows ALT-ENTER to function as EditModify() as well as
//							the original ENTER.
//
//		08/10/97	JMI	Removed unreferenced local var u32ColorIndex.
//
//		08/12/97	JMI	No longer sets the realm's progress callback.
//							Also, now passes input events to GetLocalInput() which
//							GetLocalInput() now uses for finding cheat key combos.
//
//		08/13/97	JMI	No longer attempts to show the attribute map when there
//							is no hood.
//
//		08/14/97	JMI	Now you can select a thing from the realm stats list to
//							be selected by the editor in 'edit mode' (not edit play
//							mode which could be done later using IDs) which would be
//							cool.
//
//		08/16/97 BRH	Added the feature that deletes all but the pylons, bouys,
//							soundthings and sound relays on a level so that it is 
//							easy to strip a level down to a template.  Also fixed a 
//							problem with left over bouy lines if you had a level
//							showing bouys and then started a new level, the lines
//							would still be showing on that new level.  Now it deletes
//							the bouy lines when a new level is started.
//
//		08/17/97	JMI	Now unlocks the composite buffer before a video mode change
//							and relocks it again after a video mode change.  This 
//							occurred in two spots.
//
//					JMI	Now hides the mouse cursor while the menu is up.
//
//					JMI	Now ignores all input when in a menu.
//
//					JMI	Also, disables postal organ option from within the editor.
//
//		08/21/97	JMI	PlayRealm() now sets the difficulty to the global settings
//							difficulty.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JMI	Changed occurrences of UpdateDisplay() back to 
//							rspUpdateDisplay().  Now that we are using the lock 
//							functions correctly, we don't need them.
//							Also, removed rspLockBuffer() and rspUnlockBuffer() that
//							were used to encapsulate the entire app in a lock.
//							Incorporated correct usage of rspLock/UnlockBuffer().
//
//		08/23/97	JMI	Now does not save the .rgns when in PlayRealm().
//
//		08/25/97	JMI	Made the dude show up with the user selected texture.
//
//		08/27/97	JMI	Changed file counter messages to be very short (no file
//							name) and be in the upper left corner.
//
//		08/27/97	JMI	Now sets the editor GUIs font back to the editor's pref
//							after ending a menu b/c the menu uses the global RGuiItem
//							RPrint too.
//
//		08/27/97	JMI	Set video mode was being called while the composite buffer
//							was locked which, perhaps, should not be allowed.  Fixed.
//
//		08/30/97	JMI	Now, if a warp is selected when Play is chosen, PlayRealm()
//							will use that warp.  Also, tried to make a generic 
//							mechanism for PlayRealm() to utilize the current selection
//							in choosing among multiple things.
//
//		09/04/97	JMI	If the quit status was non zero, it wasn't calling 
//							CloseRealm() before exiting.
//
//		09/12/97 MJR	Now the entire file doesn't compile when the editor is
//							disabled.
//
//		10/15/97	JMI	Now LoadRealm() converts the system path returned from
//							rspOpenBox() to a RSPiX path before passing it to
//							CRealm::Load() which then converts it back to system.  Even
//							though it was working without this change on the PC, it did
//							not work on the Mac.
//
//		10/15/97	JMI	Applied above change to PlayRealm() as well.
//
//		12/02/97	JMI	Now uses gsi.sAlwaysSetHWMode on Win32 platforms to avoid
//							flicker when changing display area without changing video
//							hardware settings.
//
//		10/03/99	JMI	Fixed pasto in RealmOpProgress() where buffer was unlocked
//							twice rather than being locked and unlocked.
//
////////////////////////////////////////////////////////////////////////////////
//
// Features to be added:
//
// Add ability for cursor to track the attribute map height.  Maybe just a key
// you hit and it adjust the cursor height to whatever the attribute map says
// at the current cursor position.
//
// Add ability to move screen using cursor keys in play mode.  Use key to
// toggle between that and the ability to track the currently tagged target.
// When you aren't using keys to move screen, they can be used by the main
// dude.  Need to create a mechanism by which main dude gets keyboard and mouse
// input indirectly.
//
// Add a way to tag any object in the game so the camera will follow it.
// This might actually require a fundamental CThing change, because there
// would need to be a standardized method of getting an object's position.
// CDude's would need to have such a feature, but I don't know if it's worth
// generalizing this to all CThings.
//
// Modify EditNew() and EditModify(), or add an EditRect(), so that we can
// get a rect that will be used as the basis for the object's hotbox.  Maybe
// EditRect() would be better because it could double as a way for us to track
// an object!  In fact, it HAS to be a separate function because when we load
// a realm, we need to go through all the objects that were loaded and adding
// new hotboxes to them, since the hotbox info doesn't get saved.  A separate
// function makes this easy.
//
// Once we have a separate EditRect() function, we'd be very tempted not to
// use RHot anymore, because all we need is a simple list of rects that we
// scan through for collisions.  The main advantage would be that I could
// impliment a simple method of re-ordering the priorities, which helps when
// you've got multiple objects in the same area.  RHot could be made to do
// this, but it seems like it would be clumsy.
//
// Add psuedo-xray mode to scene so we can see objects when they go behind
// buildings.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

#include <ctype.h>
#include <set>

// This is used to get rid of all trace of the editor code when it's disabled
#if !defined(EDITOR_DISABLED)

#include "game.h"
#include "update.h"
#include "realm.h"
#include "camera.h"
#include "localize.h"
#include "hood.h"
#include "bouy.h"
#include "navnet.h"
#include "grip.h"
#include "menus.h"
#include "SampleMaster.h"
#include "gameedit.h"
#include "input.h"
#include "pylon.h"
#include "TriggerRgn.h"
#include "TriggerRegions.h"
#include "trigger.h"

#include "dude.h"
#include "warp.h"

#include "InputSettingsDlg.h"

#include "play.h"

#include "score.h"

#include "CompileOptions.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define EDIT_KEY_LOADREALM		'L'
#define EDIT_KEY_OPENREALM		'O'
#define EDIT_KEY_SAVEREALM		'S'
#define EDIT_KEY_NEWREALM		'N'
#define EDIT_KEY_TOGGLEBOUY	'B'
#define EDIT_KEY_PICKOBJECT1	'1'
#define EDIT_KEY_PICKOBJECT2	'2'
#define EDIT_KEY_PICKOBJECT3	'3'
#define EDIT_KEY_PICKOBJECT4	'4'
#define EDIT_KEY_PICKOBJECT5	'5'
#define EDIT_KEY_PICKOBJECT6	'6'
#define EDIT_KEY_EXIT			27
#define EDIT_KEY_MENU			27
#define EDIT_KEY_CANCEL			27
#define EDIT_KEY_PLAY			'P'
#define EDIT_KEY_SCROLL_L		RSP_GK_LEFT
#define EDIT_KEY_SCROLL_R		RSP_GK_RIGHT
#define EDIT_KEY_SCROLL_U		RSP_GK_UP
#define EDIT_KEY_SCROLL_D		RSP_GK_DOWN
#define EDIT_KEY_ENDPLAY		27

#define EDIT_KEY_PAUSE			RSP_GK_PAUSE
#define EDIT_KEY_SPEED_UP		RSP_GK_NUMPAD_ASTERISK
#define EDIT_KEY_SPEED_DOWN	RSP_GK_NUMPAD_DIVIDE
#define EDIT_KEY_SPEED_NORMAL	RSP_GK_NUMPAD_DECIMAL

#define EDIT_KEY_MODIFY1		'\r'
#define EDIT_KEY_MODIFY2		('\r' | RSP_GKF_ALT)
#define EDIT_KEY_DELETE			RSP_GK_DELETE
#define EDIT_KEY_DELETE_GROUP	(RSP_GK_DELETE | RSP_GKF_CONTROL)
#define EDIT_KEY_DELETE_MOST	(RSP_GKF_CONTROL | 'D')

#define EDIT_KEY_ENLARGE_DISPLAY1	RSP_GK_NUMPAD_PLUS
#define EDIT_KEY_ENLARGE_DISPLAY2	'+'
#define EDIT_KEY_ENLARGE_DISPLAY3	'='
#define EDIT_KEY_REDUCE_DISPLAY1		RSP_GK_NUMPAD_MINUS
#define EDIT_KEY_REDUCE_DISPLAY2		'-'

#define EDIT_KEY_INCREASE_OPACITY	RSP_GK_NUMPAD_ASTERISK	
#define EDIT_KEY_DECREASE_OPACITY	RSP_GK_NUMPAD_DIVIDE  

#define EDIT_KEY_CAMERA_TRACKING	(RSP_GKF_CONTROL | 'T')

#define EDIT_KEY_SENDTOBACK		RSP_SK_CONTROL
#define EDIT_KEY_SETCAMERATRACK	RSP_SK_SHIFT

#define EDIT_KEY_NEWITEM			' '

#define EDIT_KEY_NEXTITEM			'\t'
#define EDIT_KEY_PREVITEM			(RSP_GKF_SHIFT | '\t')

#define EDIT_KEY_COPY				(RSP_GKF_CONTROL | 'C')
#define EDIT_KEY_CUT					(RSP_GKF_CONTROL | 'X')
#define EDIT_KEY_PASTE				(RSP_GKF_CONTROL | 'V')

#define EDIT_KEY_DOS_COPY			(RSP_GKF_CONTROL | RSP_GK_INSERT)
#define EDIT_KEY_DOS_CUT			(RSP_GKF_SHIFT | RSP_GK_DELETE)
#define EDIT_KEY_DOS_PASTE			(RSP_GKF_SHIFT | RSP_GK_INSERT)

#define EDIT_KEY_TOGGLE_DISP_INFO	RSP_GK_F4
#define EDIT_KEY_SHOW_MISSION			RSP_GK_F5

#define EDIT_KEY_OVERSHOOT_EDGES1	(RSP_GKF_SHIFT | '?')
#define EDIT_KEY_OVERSHOOT_EDGES2	('?')

#define EDIT_KEY_REALM_STATISTICS	(RSP_GK_F11)

// Note that this uses RSP_SK_* macros for use the rspGetKeyStatusArray() key interface.
#define KEY_XRAY_ALL						RSP_SK_F3
#define KEY_SNAP_PICTURE				RSP_SK_ENTER
#define KEY_ABORT_REALM_OPERATION	RSP_SK_ESCAPE

#define EDIT_SCROLL_AMOUNT		16

#define CURSOR_BASE_IMAGE_FILE	"res/editor/CursBase.bmp"
#define CURSOR_BASE_HOTX			7
#define CURSOR_BASE_HOTY			7
#define CURSOR_TIP_IMAGE_FILE		"res/editor/CursTip.bmp"
#define CURSOR_TIP_HOTX				7
#define CURSOR_TIP_HOTY				7

#define DRAG_MIN_X				1
#define DRAG_MIN_Y				1
#define DRAG_MIN_TIME			500

#define CURSOR_NOTHING					0
#define CURSOR_LEFT_BUTTON_UP			1
#define CURSOR_LEFT_DOUBLE_CLICK		2
#define CURSOR_LEFT_DRAG_BEGIN		3
#define CURSOR_LEFT_DRAG_END			4
#define CURSOR_RIGHT_BUTTON_UP		5
#define CURSOR_RIGHT_DOUBLE_CLICK	6
#define CURSOR_RIGHT_DRAG_BEGIN		7
#define CURSOR_RIGHT_DRAG_END			8

#define REALM_BAR_FILE			"res/editor/RealmBar.gui"
#define PICK_OBJECT_FILE		"res/editor/PickObj.gui"
#define LAYERS_GUI_FILE			"res/editor/Layers.gui"
#define CAMERAS_GUI_FILE		"res/editor/Cameras.gui"
#define SHOWATTRIBS_GUI_FILE	"res/editor/Attribs.gui"
#define INFO_GUI_FILE			"res/editor/Info.gui"

#define VIEW_GUI_FILE			"res/editor/camera.gui"
#define REALM_STATISTICS_GUI_FILE	"res/editor/stats.gui"

#define GUIS_FONT_HEIGHT		15

#define REALM_BAR_X				0
#define REALM_BAR_Y				(g_pimScreenBuf->m_sHeight - DISPLAY_BOTTOM_BORDER)

#define PICKOBJ_LIST_X			(g_pimScreenBuf->m_sWidth - DISPLAY_RIGHT_BORDER)
#define PICKOBJ_LIST_Y			0

#define LAYERS_LIST_X			(g_pimScreenBuf->m_sWidth - DISPLAY_RIGHT_BORDER)
#define LAYERS_LIST_Y			(ms_pguiPickObj->m_sY + ms_pguiPickObj->m_im.m_sHeight)

#define CAMERAS_LIST_X			(LAYERS_LIST_X)
#define CAMERAS_LIST_Y			(ms_pguiLayers->m_sY + ms_pguiLayers->m_im.m_sHeight)

#define GUIS_BAR_X				0
#define GUIS_BAR_Y				(ms_pguiRealmBar->m_sY + ms_pguiRealmBar->m_im.m_sHeight)

#define MAP_X						MAX(ms_pguiRealmBar->m_sX + ms_pguiRealmBar->m_im.m_sWidth, \
											ms_pguiGUIs->m_sX + ms_pguiGUIs->m_im.m_sWidth)
#define MAP_Y						(g_pimScreenBuf->m_sHeight - DISPLAY_BOTTOM_BORDER)

#define NAVNETS_X					(g_pimScreenBuf->m_sWidth - DISPLAY_RIGHT_BORDER)
#define NAVNETS_Y					(ms_pguiCameras->m_sY + ms_pguiCameras->m_im.m_sHeight)

#define SHOWATTRIBS_X			0
#define SHOWATTRIBS_Y			(ms_pguiGUIs->m_sY + ms_pguiGUIs->m_im.m_sHeight)

#define INFO_X						(ms_pguiShowAttribs->m_sX + ms_pguiShowAttribs->m_im.m_sWidth)
#define INFO_Y						SHOWATTRIBS_Y

#define SCROLL_BAR_THICKNESS	15

// IDs:
#define GUI_ID_NEW_REALM		1
#define GUI_ID_OPEN_REALM		2
#define GUI_ID_SAVE_REALM		3
#define GUI_ID_CLOSE_REALM		4
#define GUI_ID_PLAY_REALM		5
#define GUI_ID_SAVE_REALM_AS	6
#define GUI_ID_TOGGLE_LAYER	7
#define GUI_ID_EXIT				8
#define GUI_ID_PROPERTIES		23

#define GUI_ID_NEW_CAMERA		9
#define GUI_ID_CLOSE_CAMERA	10
#define GUI_ID_CAMERA_LIST		11
#define GUI_ID_NAVNET_LIST		11

#define GUI_ID_REALM_VISIBLE		12
#define GUI_ID_LAYERS_VISIBLE		13
#define GUI_ID_THINGS_VISIBLE		14
#define GUI_ID_CAMERAS_VISIBLE	15
#define GUI_ID_NAVNETS_VISIBLE	16
#define GUI_ID_MAP_VISIBLE			17

#define GUI_ID_MAP_REFRESH			18

#define GUI_ID_NEW_THING			19

#define GUI_ID_COPY					20
#define GUI_ID_CUT					21
#define GUI_ID_PASTE					22

#define GUI_ID_PICK_LIST		3

#define GUI_ID_LAYER_LIST		3

#define GUI_ID_MAP_ZONE			1

#define GUI_ID_ATTRIB_LAYERS		100
#define GUI_ID_ATTRIB_HEIGHT		101
#define GUI_ID_ATTRIB_NOWALK		102
#define GUI_ID_ATTRIB_LIGHT		103

#define GUI_ID_INFO_X_POS			100
#define GUI_ID_INFO_Y_POS			101
#define GUI_ID_INFO_Z_POS			102

#define GUI_ID_REALM_STATISTICS	1000

// The ID of the item to be selected by default.
#define DEFAULT_THING_ID		CThing::CDudeID

// Each item added to the Pick Object listbox will have an GUI ID of this plus
// their class ID (e.g., CDude's list item would be LIST_ITEM_GUI_ID_BASE + CDudeId).
// Don't define any editor GUI items with GUI IDs above this.
#define LIST_ITEM_GUI_ID_BASE	0x70000000

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

// Amount to inc or dec display area when adjusted.
// Indicate here as inc (i.e., positive).
#define DISPLAY_SIZE_DELTA_X	20
#define DISPLAY_SIZE_DELTA_Y	20

// Amount of bottom border.
#define DISPLAY_BOTTOM_BORDER	200
#define DISPLAY_RIGHT_BORDER	100

// Initial camera viewable area.
#define INITIAL_CAMERA_X		640
#define INITIAL_CAMERA_Y		400

// Front most priority for a hotbox.
#define FRONTMOST_HOT_PRIORITY	-32767

// The colors used for the selection rectangle surrounding the currently
// selected CThing.
#define SELECTION_COLOR1				RSP_BLACK_INDEX
#define SELECTION_COLOR2				RSP_WHITE_INDEX
#define SELECTION_THICKNESS			1

#define GUIS_GUI_FILE				"res/editor/GUIs.gui"
#define MAP_GUI_FILE					"res/editor/Map.gui"
#define NAVNETS_GUI_FILE			"res/editor/NavNets.gui"

#define DISP_INFO_FONT_H			15

#define DUDE_STATUS_RECT_X			10
#define DUDE_STATUS_RECT_Y			(REALM_STATUS_RECT_Y + REALM_STATUS_RECT_H + 3)
#define DUDE_STATUS_RECT_W			(ms_pcameraCur->m_sViewW - DUDE_STATUS_RECT_X)
#define DUDE_STATUS_RECT_H			(g_GameSettings.m_sDisplayInfo ? (INFO_STATUS_RECT_Y - DUDE_STATUS_RECT_Y) : (g_pimScreenBuf->m_sHeight - DUDE_STATUS_RECT_Y) )// (g_pimScreenBuf->m_sHeight - DUDE_STATUS_RECT_Y)

#define REALM_STATUS_RECT_X		10
#define REALM_STATUS_RECT_Y		(ms_pcameraCur->m_sFilmViewY + ms_pcameraCur->m_sViewH + 3)
#define REALM_STATUS_RECT_W		(ms_pcameraCur->m_sViewW - REALM_STATUS_RECT_X)
#define REALM_STATUS_RECT_H		(40)

#define INFO_STATUS_RECT_X			10
#define INFO_STATUS_RECT_Y			(g_pimScreenBuf->m_sHeight - INFO_STATUS_RECT_H)
#define INFO_STATUS_RECT_W			(g_pimScreenBuf->m_sWidth - INFO_STATUS_RECT_X)
#define INFO_STATUS_RECT_H			(DISP_INFO_FONT_H + 1)


// This value is mutliplied by the amount the cursor is off the edge of the view
// and the result is used to scroll the view.
#define EDGE_MOVEMENT_MULTIPLIER	1

// This value indicates the number of pixels around the edge of the view that
// imply scrolling while dragging or moving an object.
#define DRAG_SCROLL_BUFFER			10

// Amount the scroll bar buttons will scroll.
#define SCROLL_BTN_INCDEC			10

// Directory to get *.RLM files from.
#define INITIAL_REALM_DIR			"res/Levels/."

// Defines the alpha level used when blitting the trigger region currently being
// editted.
#define DEF_TRIGGER_RGN_ALPHA_LEVEL	80

// The alpha level used when showing the attributes.
#define DEF_ATTRIBUTES_ALPHA_LEVEL	80

#define TRIGGER_RGN_DRAW_INDEX		250
#define SHOW_ATTRIBS_DRAW_INDEX		250

// Initial size for paste buffer.
#define PASTE_BUFFER_INITIAL_SIZE	1024
// Amount paste buffer grows as it gets larger.
#define PASTE_BUFFER_GROW_SIZE		1024

#define MAX_ALPHA_LEVEL					255
#define MIN_ALPHA_LEVEL					15

#define INCDEC_ALPHA_LEVEL				10


#define MY_RFILE_CALLBACK_INTERVAL		100

#define PROGRESS_CALLBACK_INTERVAL		100
#define PROGRESS_X							(ms_pcameraCur->m_sFilmViewX + 10)
#define PROGRESS_Y							(ms_pcameraCur->m_sFilmViewY + 30)
#define PROGRESS_W							(ms_pcameraCur->m_sViewW - PROGRESS_X - 10)
#define PROGRESS_H							10
#define PROGRESS_COLOR_INDEX				252
#define PROGRESS_OUTLINE_COLOR_INDEX	249

// The font for the editor GUIs.
#define EDITOR_GUI_FONT						g_fontBig

////////////////////////////////////////////////////////////////////////////////
// Typedefs/structures
////////////////////////////////////////////////////////////////////////////////

typedef struct TAG_Line
{
	short sX0;
	short sY0;
	short sX1;
	short sY1;

	bool operator < (const TAG_Line& rhs) const
	{
		if (this->sX0 > rhs.sX0)
			return false;
		
		if (this->sX0 < rhs.sX0)
			return true;
	
		// sX0 is equal at this point
		if (this->sY0 > rhs.sY0)
			return false;

		if (this->sY0 < rhs.sY0)
			return true;

		// sX0 & sY0 are equal at this point

		if (this->sX1 > rhs.sX1)
			return false;

		if (this->sX1 < rhs.sX1)
			return true;

		// sX0 & sY0 & sX1 are equal at this point

		if (this->sY1 > rhs.sY1)
			return false;

		if (this->sY1 < rhs.sY1)
			return true;

		// Else the whole thing is equal
		return false;
	}

	bool operator == (const TAG_Line& rhs) const
	{
		return (this->sX0 == rhs.sX0 &&
				  this->sY0 == rhs.sY0 &&
				  this->sX1 == rhs.sX1 &&
				  this->sY1 == rhs.sY1);
	}

} LINKLINE;

#if _MSC_VER >= 1020 || __MWERKS__ >= 0x1100
	#if __MWERKS >= 0x1100
		ITERATOR_TRAITS(const LINKLINE);
	#endif
	typedef set <LINKLINE, less<LINKLINE>, allocator<LINKLINE> > lineset;
#else
	typedef set <LINKLINE, less<LINKLINE> > lineset;
#endif

// Container for a GUI and Camera pair.
typedef struct
	{
	RGuiItem*	pgui;
	RScrollBar*	psbVert;
	RScrollBar*	psbHorz;
	CCamera		cam;
	short			sViewW;
	short			sViewH;
	} View;

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

RImage* m_pimCursorBase;
RImage* m_pimCursorTip;

CBouy*	m_pBouyLink0;
CBouy*	m_pBouyLink1;

lineset m_NetLines;
static bool	ms_bDrawNetwork = true;

// ID of item most recently pressed or 0, if none.
static long	ms_lPressedId	= 0;
// Realm filename.  Assuming only one Realm loaded at once.
static char	ms_szFileName[RSP_MAX_PATH]	= "";

static short	ms_sMoving		= FALSE;	// TRUE, if moving/placing a thing (ms_pthingSel).

static CThing*	ms_pthingSel	= NULL;	// CThing* to thing currently selected.
static RHot*	ms_photSel		= NULL;	// RHot* to hotbox associated with selected thing.

// Initial width and height of display so we can
// restore video mode when done editting.
static short	ms_sInitialDisplayWidth		= 0;
static short	ms_sInitialDisplayHeight	= 0;
static short	ms_sInitialDeviceDepth		= 0;
static short	ms_sInitialDeviceWidth		= 0;
static short	ms_sInitialDeviceHeight		= 0;

// The current camera.
// Scrollbars' callback update camera pointed to by this.
static CCamera*	ms_pcameraCur	= NULL;

// Global GUIs (used in both editing and edit play modes).
static RScrollBar	ms_sbVert;
static RScrollBar	ms_sbHorz;

// This points to the CHood's hotbox which is the root of all other CThing
// hotboxes.
static RHot*	ms_photHood	= NULL;

// This is the hotbox priority of the farthest item from the user.
// Start out as close to front as possible.
static short	ms_sBackPriority	= FRONTMOST_HOT_PRIORITY;

// Made this global (was static in GetCursor()) for temp.
static short	ms_sDragState = 0;

// Root level GUIs to load from *.gui files.
static RGuiItem*	ms_pguiRealmBar		= NULL;
static RGuiItem*	ms_pguiPickObj			= NULL;
static RGuiItem*	ms_pguiLayers			= NULL;
static RGuiItem*	ms_pguiCameras			= NULL;
static RGuiItem*	ms_pguiGUIs				= NULL;
static RGuiItem*	ms_pguiMap				= NULL;
static RGuiItem*	ms_pguiNavNets			= NULL;
static RGuiItem*	ms_pguiShowAttribs	= NULL;
static RGuiItem*	ms_pguiInfo				= NULL;

// Child GUIs that need to be frequently accessed.
static RListBox*	ms_plbLayers		= NULL;
static RGuiItem*	ms_pguiMapZone		= NULL;
static RGuiItem*	ms_pguiInfoXPos	= NULL;
static RGuiItem*	ms_pguiInfoYPos	= NULL;
static RGuiItem*	ms_pguiInfoZPos	= NULL;

// The current ratio being used for the map.
static double		ms_dMapRatio		= 0.0;

// The current CGameEditThing.
static CGameEditThing*	ms_pgething	= NULL;

// List of cameras and their GUI.
static RList<View>	ms_listViews;

// If true, scrolling via drag/move of CThing is allowed.
static bool				ms_bDragScroll	= true;

// Trigger regions for pylons.
static TriggerRgn		ms_argns[256];

// Current pylon being editted.
static CPylon*			ms_pylonEdit	= NULL;	// NULL for none.

// Current block size for drawing.
static short			ms_sDrawBlockSize	= 5;

// Sprite used to draw pylon trigger region in the editor.
static CSprite2		ms_spriteTriggerRgn;

// Copy/Paste buffer.
static RFile			ms_filePaste;

// File count used for items in the paste buffer (always decremented so we
// can guarantee that statics are saved).
static short			ms_sFileCount		= -1;

// Type of item to paste.
static CThing::ClassIDType	ms_idPaste;

// Sprite used to draw attributes.
static CSprite2		ms_spriteAttributes;

// Attribute masks to draw.
static U16				ms_u16TerrainMask;
static U16				ms_u16LayerMask;

// Used by RFile callback function
static long		ms_lRFileCallbackTime;
static long		ms_lFileBytesSoFar;
static char		ms_szFileOpDescriptionFrmt[512];
static RPrint	ms_printFile;
static short	ms_sFileOpTextX;
static short	ms_sFileOpTextY;
static short	ms_sFileOpTextW;
static short	ms_sFileOpTextH;

// Amount to scroll off edge of realm.
static long	ms_lEdgeOvershoot	= 1000;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

// Do ALL editor input.
static bool DoInput(		// Returns true when done.
	CRealm*	prealm,		// Ptr to current realm.
	CCamera*	pcamera,		// Ptr to current camera.
	short*	psCursorX,	// Out: Cursor X position.
	short*	psCursorY,	// Out: Cursor Y position.
	short*	psCursorZ);	// Out: Cursor Z position.

// Do ALL editor output.
static void DoOutput(	// Returns nothing.
	CRealm*	prealm,		// Ptr to current realm.
	CCamera*	pcamera,		// Ptr to current camera.
	short		sCursorX,	// Cursor X position.
	short		sCursorY,	// Cursor Y position.
	short		sCursorZ);	// Cursor Z position.

static void GetCursor(	// Returns nothing.
	RInputEvent*	pie,	// In:  Input event.
								// Out: pie->sUsed = TRUE, if used.
	short* psX,				// Out: X coord of event.
	short* psY,				// Out: Y coord of event.
	short* psZ,				// Out: Z coord of event.
	short* psEvent);		// Out: Event type.

static short InitCursor(
	void);

static void KillCursor(
	void);

static void DrawCursor(
	short sCursorX,										// In:  Cursor hotspot x coord
	short sCursorY,										// In:  Cursor hotspot y coord
	short sCursorZ,										// In:  Cursor hotspot z coord
	RImage* pimDst,										// In:  Image to draw to
	CRealm* prealm,										// In:  Realm.
	CCamera* pcamera);									// In:  Camera on prealm.

static short NewRealm(
	CRealm* prealm);

static short CloseRealm(
	CRealm* prealm);

static short LoadRealm(
	CRealm* prealm);

static short SaveRealm(				// Returns 0 on success.             
	CRealm* prealm,					// In:  Realm to save.               
	char* pszRealmName,				// In:  Filename to save as.         
	bool	bSaveTriggerRegions);	// In:  Save the trigger regions too.

static short SaveRealm(
	CRealm* prealm);

static short SaveRealmAs(
	CRealm* prealm);

static void PlayRealm(				// Returns nothing.
	CRealm*	prealm,					// In:  Realm to play.
	CThing*	pthingSel);				// In:  Currently selected CThing which can
											// be used to give PlayRealm() a hint on which
											// of several things the user wants to use.
											// For example, a selected warp is the used
											// as the warp in point.

// Create a new CThing derived object of type id in prealm at the specified
// position.
static short CreateNewThing(		// Returns 0 on success.
	CRealm*	prealm,					// In:  Realm to add new CThing to.
	CThing::ClassIDType	id,		// ID of new CThing type to create.
	short		sPosX,					// Position for new CThing.
	short		sPosY,					// Position for new CThing.
	short		sPosZ,					// Position for new CThing.
	CThing**	ppthing,					// Out: Pointer to new thing.
	RHot**	pphot,					// Out: Pointer to new hotbox for thing.
	RFile*	pfile = NULL);			// In:  Optional file to load from (instead of EditNew()).

// Move a thing to the specified location and update its RHot with an
// EditRect() call.
static void MoveThing(				// Returns nothing.
	CThing*	pthing,					// Thing to move.
	RHot*		phot,						// Thing's hotbox.
	short		sPosX,					// New position.
	short		sPosY,					// New position.
	short		sPosZ);					// New position.

// Enlarges the display area.
static void OnEnlargeDisplay(
	CCamera* pcamera,					// Camera to update.
	CRealm*	prealm);					// Realm to update.

// Reduces the display area.
static void OnReduceDisplay(
	CCamera* pcamera,					// Camera to update.
	CRealm*	prealm);					// Realm to update.

// Set display mode such that display area is as
// specified.
static short SetDisplayArea(	// Returns 0 on success.
	short	sDisplayD,				// New depth of display.
	short	sDisplayW,				// New width of display area.
	short	sDisplayH);				// New height of display area.

// Set display mode and display area such that camera view
// is specified size.
static short SetCameraArea(void);	// Returns 0 on success.

// Adjust the display area by the specified deltas.
static short AdjustDisplaySize(	// Returns 0 on success.
	short	sAdjustX,					// Amount to increase width of display area.
											// Can be negative to decrease.
	short	sAdjustY,					// Amount to increase height of display area.
											// Can be negative to decrease.
	CCamera* pcamera,					// Camera to update.
	CRealm*	prealm);					// Realm to update.

// Update screen size sensitive objects.
static short SizeUpdate(		// Returns 0 on success.
	CCamera* pcamera,				// Camera to update.
	CRealm*	prealm);				// Realm to update.

// Resets all RHots including and descended from ms_photHood
// to FRONTMOST_HOT_PRIORITY.
static void ResetHotPriorities(void);	// Returns nothing.

// Callback from GUIs.  This will set ms_lPressedId to pgui->m_lId.
static void GuiPressedCall(	// Returns nothing.
	RGuiItem*	pgui);			// GUI item pressed.

// Callback from pressed list items.
static void ListItemPressedCall(	// Returns nothing.
	RGuiItem*	pgui);				// GUI item pressed.

// Callback from scrollbars indicating change in thumb position.
static void ScrollPosUpdate(	// Returns nothing.
	RScrollBar* psb);				// ScrollBar that was updated.

// Callback from RHot when an event occurs within it.
static void ThingHotCall(	// Returns nothing.
	RHot*	phot,					// Ptr to RHot that generated event.
	RInputEvent*	pie);		// In:  Most recent user input event.
									// Out: Depends on callbacks.  Generally,
									// pie->sUsed = TRUE, if used.

// Draws the rubber band type line while creating bouy links.
static void DrawBouyLink(		// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera);			// In:  View of prealm.

// AddLine - add the newly drawn line to the set of lines.
static void AddNewLine(short sX0, short sY0, short sX1, short sY1);

// Writes out a log of connected bouys.  This is for debugging only
static void NetLog(CNavigationNet* pNavNet);

// UpdateNetLines - build the full list of link lines from the
// navigation net.  This function can be used after the bouys have
// been loaded for a realm before the first time the DrawNetwork is
// called.  There may also be a key to refresh the lines.  This will
// also be called when switching the 'current' network
static void UpdateNetLines(CNavigationNet* pNavNet);

// Draw network - draw the lines 
static void DrawNetwork(		// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera);			// In:  View of prealm.

// Get the Editor Thing from the specified realm.
static CGameEditThing* GetEditorThing(	// Returns ptr to editor thing for 
													// specified realm or NULL.
	CRealm*	prealm);							// Realm to get editor thing from.

// Creates a view and adds it to the list of views.
static short AddView(		// Returns 0 on success.
	CRealm*	prealm);			// In:  Realm in which to setup camera.

// Kills a view and removes it from the list of views.
static void RemoveView(		// Returns nothing.
	View*	pview);				// In: View to remove or NULL to remove currently
									// selected view.

// Removes all current views.
static void RemoveViews(void);

// Creates a new View and adds it to the list of Views.
static short CreateView(					// Returns 0 on success.
	View**	ppview,							// Out: New view, if not NULL.
	CRealm*	prealm);							// In:  Realm in which to setup camera.

// Destroys a View and removes it from the list of Views.
static void KillView(						// Returns nothing.
	View*		pview);							// View to kill.

// Draw specified view.
static void DrawView(						// Returns nothing.
	View*		pview,							// View to draw.
	CRealm*	prealm);							// Realm to draw.

// Draw all views.
static void DrawViews(						// Returns nothing.
	CRealm*	prealm);							// Realm to draw.

// Clear the specified GUI's area.
static void ClearGUI(						// Returns nothing.
	RGuiItem*	pgui);						// In:  GUI's whose area we should clean.

// Clear all views.
static void ClearViews(void);				// Returns nothing.

// Do focus hotbox logic and such for GUIs.
static void DoView(							// Returns nothing.
	View*				pview,					// View to do.
	RInputEvent*	pie);						// Input event to process.

// Do all views.
static void DoViews(							// Returns nothing.
	RInputEvent*	pie);						// Input event to process.

// Draw the map.
static void RefreshMap(						// Returns nothing.
	CRealm*	prealm);							// Realm to map.

// Cancel any thing drag.
static void CancelDrag(CRealm* prealm);// Returns nothing.

// Place any thing being dragged.
static void DragDrop(	// Returns nothing.
	short sDropX,			// In:  Drop x position.
	short sDropY,			// In:  Drop y position.
	short sDropZ);			// In:  Drop z position.

// Move focus to next item in realm's thing list.
static void NextItem(	// Returns nothing.
	CRealm*	prealm);		// In:  The realm we want the next thing in.

// Move focus to previous item in realm's thing list.
static void PrevItem(	// Returns nothing.
	CRealm*	prealm);		// In:  The realm we want the next thing in.

// Load the trigger regions for the specified realm.
static short LoadTriggerRegions(	// Returns 0 on success.
	char*	pszRealmName);				// In:  Name of the REALM (*.RLM) file.
											// The .ext is stripped and .rgn is appended.

// Save the trigger regions for the specified realm.
static short SaveTriggerRegions(	// Returns 0 on success.
	char*	pszRealmName, 				// In:  Name of the REALM (*.RLM) file.
	CRealm* prealm);					// The .ext is stripped and .rgn is appended.

// Create the trigger attribute layer for the realm
static short CreateTriggerRegions(	// Returns 0 on success.
	CRealm*	prealm		);			// In:  Access of Realm Info

// Change or clear the current pylon being edited.
static void EditPylonTriggerRegion(	// Returns nothing.
	CThing* pthingPylon);				// In:  Pylon whose trigger area we want to

// Set the selection to the specified CThing.
static CThing*	SetSel(	// Returns CThing that previously was selected.
	CThing* pthingSel,	// In:  CThing to be selected.
	RHot*	photSel);		// In:  Hotbox of CThing to be selected.

// Delete the specified item.
static void DelThing(	// Returns nothing.
	CThing* pthingDel,	// In:  CThing to be deleted.
	RHot*	photDel,			// In:  Hotbox of CThing to be deleted.
	CRealm* prealm);		// In:  Current realm

// Delete all the items in the currently selected class.
static void DelClass(	// Returns nothing.
	CThing* pthingDel,	// In:  CThing to be deleted.
	CRealm* prealm);		// In:  Current realm

// Delete all but the basic items from the realm in order to make template levels
static void DelMost(		// Return nothing
	CRealm* prealm);		// In:  Current realm

// Copy a thing to the paste buffer.
static short CopyItem(	// Returns 0 on success.
	CThing* pthingCopy);	// In:  CThing to copy.

// Copy a thing to the paste buffer.
static short PasteItem(	// Returns 0 on success.
	CRealm*	prealm,		// In:  The realm to paste into.
	short		sX,			// In:  Location for new thing.
	short		sY,			// In:  Location for new thing.
	short		sZ);			// In:  Location for new thing.

// Map a screen coordinate to a realm coordinate.
// Note that this function's *psRealmY output is always
// the height specified by the realm's attribute map
// at the resulting *psRealmX, *psRealmZ.
static void MapScreen2Realm(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	short sScreenX,				// In:  Screen x coord.
	short sScreenY,				// In:  Screen y coord.
	short* psRealmX,				// Out: Realm x coord.
	short* psRealmY,				// Out: Realm y coord (always via realm's height map).
	short* psRealmZ);				// Out: Realm z coord.

// Map a realm coordinate to a screen coordinate.
static void Maprealm2Screen(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	short		sRealmX,				// In:  Realm x coord.
	short		sRealmY,				// In:  Realm y coord.
	short		sRealmZ,				// In:  Realm z coord.
	short*	psScreenX,			// Out: Screen x coord.
	short*	psScreenY);			// Out: Screen y coord.

// Blit attribute areas lit by the specified mask into the specified image.
static void AttribBlit(			// Returns nothing.
	RMultiGrid*	pmg,				// In:  Multigrid of attributes.
	U16			u16Mask,			// In:  Mask of important attributes.
	RImage*		pimDst,			// In:  Destination image.
	short			sSrcX,			// In:  Where in Multigrid to start.
	short			sSrcY,			// In:  Where in Multigrid to start.
	short			sW,				// In:  How much of multigrid to use.
	short			sH);				// In:  How much of multigrid to use.

// Callback for attrib mask multibtns.
static void AttribMaskBtnPressed(	// Returns nothing.
	RGuiItem*	pgui_pmb);				// In:  Ptr to the pressed GUI (which is a multibtn).

// Resizes the attribute sprite, if allocated.
static short SizeShowAttribsSprite(void);	// Returns 0 on success.

// Convert a .RLM filename to a .RGN one.
static void RlmNameToRgnName(	// Returns nothing.
	char*	pszRealmName,		// In:  .RLM name.
	char* pszRgnName);		// Out: .RGN name.

// Our RFile callback
static void MyRFileCallback(long lBytes);

// Update selection info in the info GUI.
static void UpdateSelectionInfo(	// Returns nothing.
	bool bTitle = false);			// In:  true to update the title info as well.

static short TmpFileName(								// Returns 0 if successfull, non-zero otherwise
	char* pszFileName,									// Out: Temp file name returned here
	short sMaxSize);										// In:  Maximum size of file name

// Show statistics for the specified realm.
static short ShowRealmStatistics(	// Returns 0 on success.
	CRealm*	prealm,						// In:  Realm to get stats on.
	CThing** ppthing);					// Out: Selected thing, if not NULL.

// Init load/save counter.  You should call KillFileCounter() after
// done with the file access.
static void InitFileCounter(			// Returns nothing.
	char*	pszDescriptionFrmt);			// In:  sprintf format for description of 
												// operation.

// Kill load/save counter.  Can be called multiple times w/o corresponding
// Init().
static void KillFileCounter(void);	// Returns nothing.

// Callback from realm during time intensive operations.
static bool RealmOpProgress(			// Returns true to continue; false to
												// abort operation.
	short	sLastItemProcessed,			// In:  Number of items processed so far.
	short	sTotalItemsToProcess);		// In:  Total items to process.

////////////////////////////////////////////////////////////////////////////////
//
// Set some basic stuff for specified item.
//
////////////////////////////////////////////////////////////////////////////////
inline void SetPressedCall(	// Returns nothing.
	RGuiItem*	pguiRoot,		// Root item.
	long	lId)						// ID of GUI item to set.
	{
	ASSERT(pguiRoot != NULL);

	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui != NULL)
		{
		// Set callback.
		pgui->m_bcUser	= GuiPressedCall;
		}
	else
		{
		TRACE("SetPressedCall(): ID %ld missing.\n", lId);
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Set the user values of the specified GUI.
//
////////////////////////////////////////////////////////////////////////////////
inline void SetMBValsAndCallback(		// Returns nothing.
	RGuiItem*	pguiRoot,					// In:  Root item.
	long			lId,							// In:  ID of child to set user vals on.
	U32			u32UserInstance,			// In:  Value for m_ulUserInstance.
	U32			u32UserData,				// In:  Value for m_ulUserData.
	short			sState)						// In:  Initial MultiBtn state.
	{
	RMultiBtn*	pmb	= (RMultiBtn*)pguiRoot->GetItemFromId(lId);
	if (pmb)
		{
		pmb->m_ulUserInstance	= u32UserInstance;
		pmb->m_ulUserData			= u32UserData;
		pmb->m_bcUser				= AttribMaskBtnPressed;
		pmb->m_sState				= sState;
		// Reflect new state.
		pmb->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Run the game editor
//
////////////////////////////////////////////////////////////////////////////////
extern void GameEdit(
	void)
	{
	// Clear any members that need to be intialized on each entrance.
	ms_pcameraCur		= NULL;
	ms_pthingSel		= NULL;
	ms_photSel			= NULL;
	ms_plbLayers		= NULL;
	ms_pgething			= NULL;
	ms_photHood			= NULL;

	// Disable 'Organ' on 'Audio Options' menu.
	menuAudioOptions.ami[1].sEnabled	= FALSE;

	// Set alpha level to default.
	ms_spriteTriggerRgn.m_sAlphaLevel	= DEF_TRIGGER_RGN_ALPHA_LEVEL;
	ms_spriteAttributes.m_sAlphaLevel	= DEF_ATTRIBUTES_ALPHA_LEVEL;

	// Set attrib masks to not display any attributes.
	ms_u16TerrainMask	= 0;
	ms_u16LayerMask	= 0;

	// Set filename such that initially open and save dialogs start in
	// *.RLM dir.
	strcpy(ms_szFileName, FullPathVD(INITIAL_REALM_DIR));

	// Clear mouse and keyboard events
	rspClearAllInputEvents();

	// Remember initial display settings.
	rspGetVideoMode(
		&ms_sInitialDeviceDepth,
		&ms_sInitialDeviceWidth,
		&ms_sInitialDeviceHeight,
		NULL,
		&ms_sInitialDisplayWidth,
		&ms_sInitialDisplayHeight);

	// Clear screen
	rspLockBuffer();

	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);
	
	rspUnlockBuffer();

	rspUpdateDisplay();

	// Display message if necessary
	#ifdef DISABLE_EDITOR_SAVE_AND_PLAY
		rspMsgBox(
			RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
			"Postal Editor",
			"This is the same game editor we used to create the entire game.  "
			"The only things that aren't working in it are the \"Save\" and "
			"\"Play\" functions. We had to disable these for various reasons, "
			"not the least of which is that it would have taken up more space. "
			"But please, check it out and let us know what you think!");
	#endif
	
	// Clear rubber band links for bouys
	m_pBouyLink0 = m_pBouyLink1 = NULL;

	// Init cursor stuff
	if (InitCursor() == 0)
		{
		//////////////////////////////////////////////////////////////////////
		// Create realm.
		//////////////////////////////////////////////////////////////////////

		// Create a realm.  At some point we will probably extend the editor to
		// be able to handle multiple realms at once.  For now it just has one.
		CRealm* prealm = new CRealm;
		ASSERT(prealm != 0);

		// This realm will only be used for editting, so it make it known.
		prealm->m_flags.bEditing	= true;

		// Setup progress callback right away.
//		prealm->m_fnProgress			= RealmOpProgress;

		// Reset realm time here and never do anything to it while in the editor.
		// This way, game objects will be able to access the time, but will never
		// see any change, which seems like a good thing.
		prealm->m_time.Reset();

		//////////////////////////////////////////////////////////////////////
		// Setup camera.
		//////////////////////////////////////////////////////////////////////

		CCamera* pcamera = new CCamera;
		ASSERT(pcamera != NULL);
		pcamera->SetScene(&(prealm->m_scene));
		// Update display size sensitive objects.
		SizeUpdate(pcamera, prealm);

		pcamera->SetFilm(g_pimScreenBuf, 0, 0);

		ms_pcameraCur	= pcamera;

		/////////////////////////////////////////////////////////////////////////
		// Set up GUIs.
		/////////////////////////////////////////////////////////////////////////

		// Make sure there's a font in the default RPrint for GUIs.
		// Also, sets the size the GUIs will use when printing text.
		RGuiItem::ms_print.SetFont(GUIS_FONT_HEIGHT, &EDITOR_GUI_FONT);

		// Load GUIs.
		ms_pguiRealmBar		= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, REALM_BAR_FILE) );
		ms_pguiPickObj			= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, PICK_OBJECT_FILE) );
		ms_pguiLayers			= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, LAYERS_GUI_FILE) );
		ms_pguiCameras			= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, CAMERAS_GUI_FILE) );
		ms_pguiGUIs				= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, GUIS_GUI_FILE) );
		ms_pguiMap				= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, MAP_GUI_FILE) );
		ms_pguiNavNets			= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, NAVNETS_GUI_FILE) );
		ms_pguiShowAttribs	= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, SHOWATTRIBS_GUI_FILE) );
		ms_pguiInfo				= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, INFO_GUI_FILE) );

		// Make sure we got all our GUIs . . .
		if (	ms_pguiRealmBar != NULL 
			&& ms_pguiPickObj != NULL 
			&& ms_pguiLayers != NULL 
			&& ms_pguiCameras != NULL
			&& ms_pguiGUIs != NULL
			&& ms_pguiMap != NULL
			&& ms_pguiNavNets != NULL
			&& ms_pguiShowAttribs
			&& ms_pguiInfo)
			{
			// Set items that are to notify us.
			SetPressedCall(ms_pguiRealmBar, GUI_ID_NEW_REALM);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_OPEN_REALM);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_SAVE_REALM);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_CLOSE_REALM);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_PLAY_REALM);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_SAVE_REALM_AS);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_EXIT);
			SetPressedCall(ms_pguiRealmBar, GUI_ID_PROPERTIES);

			// Show the Realm Bar.
			ms_pguiRealmBar->SetVisible(ms_pguiRealmBar->m_sVisible);

			// Get object picker list box.
			RListBox*	plbPicker	= (RListBox*)ms_pguiPickObj->GetItemFromId(GUI_ID_PICK_LIST);

			ASSERT(plbPicker != NULL);
			ASSERT(plbPicker->m_type == RGuiItem::ListBox);
			
			// Add available objects to listbox.
			CThing::ClassIDType	idCur;
			RGuiItem*				pguiItem;
			for (idCur	= 1; idCur < CThing::TotalIDs; idCur++)
				{
				// If item is editor creatable . . .
				if (CThing::ms_aClassInfo[idCur].bEditorCreatable == true)
					{
					// Add string for each item to listbox.
					pguiItem	= plbPicker->AddString((char*)CThing::ms_aClassInfo[idCur].pszClassName);
					if (pguiItem != NULL)
						{
						pguiItem->m_lId			= LIST_ITEM_GUI_ID_BASE + idCur;
						pguiItem->m_ulUserData	= (ULONG)idCur;

						// Set the callback on pressed.
						pguiItem->m_bcUser		= ListItemPressedCall;
						}
					}
				}

			// Format list items.
			plbPicker->AdjustContents();

			// Select the DEFAULT_THING_ID.
			RGuiItem*	pguiDefThing	= ms_pguiPickObj->GetItemFromId(DEFAULT_THING_ID + LIST_ITEM_GUI_ID_BASE);
			if (pguiDefThing != NULL)
				{
				plbPicker->SetSel(pguiDefThing);
				// Ensure that the default item is visible.  Not scrolled off somewhere.
				plbPicker->EnsureVisible(pguiDefThing);
				}

			// Activate pick object box.
			ms_pguiPickObj->SetVisible(ms_pguiPickObj->m_sVisible);

			// Get layer tweaker list box.
			ms_plbLayers	= (RListBox*)ms_pguiLayers->GetItemFromId(GUI_ID_LAYER_LIST);

			ASSERT(ms_plbLayers != NULL);
			ASSERT(ms_plbLayers->m_type == RGuiItem::ListBox);

			// Add available layers to listbox.
			short	sLayer;
			for (sLayer	= CRealm::LayerBg; sLayer < CRealm::TotalLayers; sLayer++)
				{
				// Add string for each item to listbox.
				pguiItem	= ms_plbLayers->AddString(CRealm::ms_apszLayerNames[sLayer]);
				if (pguiItem != NULL)
					{
					// Remember which layer it's associated with.
					pguiItem->m_ulUserData	= (ULONG)sLayer;
					// We'll need to know when these are pressed.
					pguiItem->m_bcUser	= ListItemPressedCall;
					pguiItem->m_lId		= GUI_ID_TOGGLE_LAYER;
					// These are push buttons.
					ASSERT(pguiItem->m_type == RGuiItem::PushBtn);
					((RPushBtn*)pguiItem)->m_state	
						= (prealm->m_scene.m_pLayers[sLayer].m_bHidden == false) ? RPushBtn::On : RPushBtn::Off;
					// Realize state.
					pguiItem->Compose();
					}
				}

			// Format list items.
			ms_plbLayers->AdjustContents();

			ms_pguiLayers->SetVisible(ms_pguiLayers->m_sVisible);

			// Set callbacks.
			SetPressedCall(ms_pguiCameras, GUI_ID_NEW_CAMERA);
			SetPressedCall(ms_pguiCameras, GUI_ID_CLOSE_CAMERA);

			ms_pguiCameras->SetVisible(ms_pguiCameras->m_sVisible);

			// --------- GUIs bar --------

			// Set items that are to notify us.
			SetPressedCall(ms_pguiGUIs, GUI_ID_REALM_VISIBLE);
			SetPressedCall(ms_pguiGUIs, GUI_ID_LAYERS_VISIBLE);
			SetPressedCall(ms_pguiGUIs, GUI_ID_THINGS_VISIBLE);
			SetPressedCall(ms_pguiGUIs, GUI_ID_CAMERAS_VISIBLE);
			SetPressedCall(ms_pguiGUIs, GUI_ID_NAVNETS_VISIBLE);
			SetPressedCall(ms_pguiGUIs, GUI_ID_MAP_VISIBLE);

			// Set items' states.

			ms_pguiGUIs->SetVisible(TRUE);

			// ---------- NavNets list --------
			ms_pguiNavNets->SetVisible(ms_pguiNavNets->m_sVisible);

			// ---------- Show Attribs --------
			SetMBValsAndCallback(ms_pguiShowAttribs, GUI_ID_ATTRIB_LAYERS, (U32)(&ms_u16LayerMask), REALM_ATTR_LAYER_MASK, 1);
			SetMBValsAndCallback(ms_pguiShowAttribs, GUI_ID_ATTRIB_HEIGHT, (U32)(&ms_u16TerrainMask), REALM_ATTR_HEIGHT_MASK, 1);
			SetMBValsAndCallback(ms_pguiShowAttribs, GUI_ID_ATTRIB_NOWALK, (U32)(&ms_u16TerrainMask), REALM_ATTR_NOT_WALKABLE, 1);
			SetMBValsAndCallback(ms_pguiShowAttribs, GUI_ID_ATTRIB_LIGHT, (U32)(&ms_u16TerrainMask), REALM_ATTR_LIGHT_BIT, 1);

			ms_pguiShowAttribs->SetVisible(TRUE);

			// ---------- Info ----------------
			ms_pguiInfoXPos	= ms_pguiInfo->GetItemFromId(GUI_ID_INFO_X_POS);
			ms_pguiInfoYPos	= ms_pguiInfo->GetItemFromId(GUI_ID_INFO_Y_POS);
			ms_pguiInfoZPos	= ms_pguiInfo->GetItemFromId(GUI_ID_INFO_Z_POS);

			ms_pguiInfo->SetVisible(TRUE);

			// ---------- Map -----------------
			SetPressedCall(ms_pguiMap, GUI_ID_MAP_REFRESH);

			ms_pguiMapZone	= ms_pguiMap->GetItemFromId(GUI_ID_MAP_ZONE);

			ms_pguiMap->SetVisible(ms_pguiMap->m_sVisible);

			// Copy colors and such to scrollbars.
			ms_pguiPickObj->CopyParms(&ms_sbVert);
			ms_pguiPickObj->CopyParms(&ms_sbHorz);

			ms_sbHorz.m_oOrientation	= RScrollBar::Horizontal;

			ms_sbVert.SetVisible(TRUE);
			ms_sbHorz.SetVisible(TRUE);

			ms_sbVert.m_upcUser	= ScrollPosUpdate;
			ms_sbHorz.m_upcUser	= ScrollPosUpdate;

			// Smooth scrolling.
			ms_sbVert.m_scrollage		= RScrollBar::Smooth;
			ms_sbHorz.m_scrollage		= RScrollBar::Smooth;


			// Set display size based on the user's settings.
			SetCameraArea();

			// Update size sensitive objects (including scrollbars).
			SizeUpdate(pcamera, prealm);

			//////////////////////////////////////////////////////////////////////
			// Prepare user input.
			//////////////////////////////////////////////////////////////////////

			// Clear mouse and keyboard events
			rspClearAllInputEvents();

			// Make sure the system cursor is visible.
			// Store show level so we can restore it.
			short	sCursorShowLevel	= rspGetMouseCursorShowLevel();
			rspSetMouseCursorShowLevel(1);

			// User must load an existing realm or start a new one to go any further
			bool bGoEdit = false;
			bool bExit = false;

			// Set currently select thing to default value
			CThing::ClassIDType idCurrent = DEFAULT_THING_ID;
			// Clear mouse and keyboard events
			rspClearAllInputEvents();

			//////////////////////////////////////////////////////////////////////
			// Editor's main loop.
			//////////////////////////////////////////////////////////////////////

			bExit = false;
			short	sCursorX, sCursorY, sCursorZ;
			while (!bExit)
				{
				///////////////////////////////////////////////////////////////////
				// System update.
				///////////////////////////////////////////////////////////////////
				UpdateSystem();

				///////////////////////////////////////////////////////////////////
				// Input.
				///////////////////////////////////////////////////////////////////
				bExit	= DoInput(prealm, pcamera, &sCursorX, &sCursorY, &sCursorZ);

				///////////////////////////////////////////////////////////////////
				// Output.
				///////////////////////////////////////////////////////////////////
				DoOutput(prealm, pcamera, sCursorX, sCursorY, sCursorZ);
				}

			// Done with the views.
			RemoveViews();

			// Restore cursor show level.
			rspSetMouseCursorShowLevel(sCursorShowLevel);

			// Done with GUIs.
			delete ms_pguiRealmBar;
			ms_pguiRealmBar	= NULL;
			delete ms_pguiPickObj;
			ms_pguiPickObj		= NULL;
			delete ms_pguiLayers;
			ms_pguiLayers		= NULL;
			delete ms_pguiCameras;
			ms_pguiCameras		= NULL;
			delete ms_pguiGUIs;
			ms_pguiGUIs			= NULL;
			delete ms_pguiMap;
			ms_pguiMap			= NULL;
			delete ms_pguiNavNets;
			ms_pguiNavNets		= NULL;
			delete ms_pguiShowAttribs;
			ms_pguiShowAttribs	= NULL;
			delete ms_pguiInfo;
			ms_pguiInfo				= NULL;
			}
		else
			{
			rspMsgBox(
				RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
				g_pszCriticalErrorTitle,
				g_pszAssetsMissingError);
			}

		// Done with the realm.
		delete prealm;
		prealm	= NULL;
		// Done with the camera.
		delete pcamera;
		pcamera	= NULL;
		ms_pcameraCur	= NULL;

		// Kill cursor stuff
		KillCursor();
		}

	// If there is an image, delete it.
	delete ms_spriteAttributes.m_pImage;
	ms_spriteAttributes.m_pImage	= NULL;

	// If paste buffer open . . .
	if (ms_filePaste.IsOpen() != FALSE)
		{
		ms_filePaste.Close();
		}

	// Restore display mode.
	rspSetVideoMode(
		ms_sInitialDeviceDepth, 
		ms_sInitialDeviceWidth,
		ms_sInitialDeviceHeight,
		ms_sInitialDisplayWidth, 
		ms_sInitialDisplayHeight);

	// Give Jeff a chance to update his cool wrapper.
	// I'm not sure if this is necessary, but let's be safe.
	rspNameBuffers(&g_pimScreenBuf);

	// Clear mouse and keyboard events
	rspClearAllInputEvents();

	// Clear screen
	rspLockBuffer();

	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);

	rspUnlockBuffer();

	rspUpdateDisplay();

	// Re-enable 'Organ' on 'Audio Options' menu.
	menuAudioOptions.ami[1].sEnabled	= TRUE;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Do ALL editor input.
//
////////////////////////////////////////////////////////////////////////////////
static bool DoInput(		// Returns true when done.
	CRealm*	prealm,		// Ptr to current realm.
	CCamera*	pcamera,		// Ptr to current camera.
	short*	psCursorX,	// Out: Cursor X position.
	short*	psCursorY,	// Out: Cursor Y position.
	short*	psCursorZ)	// Out: Cursor Z position.
	{
	bool	bExit	= false;

	short sCursorX;
	short sCursorY;
	short sCursorZ;
	short sCursorEvent;
	RInputEvent	ie;

	// Get next input event.
	ie.type	= RInputEvent::None;
	rspGetNextInputEvent(&ie);
	
	Menu*	pmenu	= GetCurrentMenu();
	// If there is a menu . . .
	if (pmenu != NULL)
		{
		// This is CHEEZY AS HELL but the normal menu callback calls
		// game.cpp which sets its action flag telling it to call this
		// function.  Not sure how to do it here.  Will we need to call
		// game.cpp, play.cpp, and gameedit.cpp whenever this menu is
		// activated?
		if (pmenu == &menuJoystick || pmenu == &menuMouse || pmenu == &menuKeyboard)
			{
			// Do the input settings.
			EditInputSettings();
			}

		// Menu on top (even of cursor).
		DoMenuInput(&ie, 0);

		// If there is no longer a menu . . .
		if (GetCurrentMenu() == NULL)
			{
			// Show the mouse.
			rspShowMouseCursor();

			// Fade in and restore colors.
			PalTranOff();
			}
		}
	else
		{
		// If editting a pylon trigger region . . .
		if (ms_pylonEdit != NULL)
			{
			static short	sButtons	= 0;
			// Get cursor position and event
			GetCursor(&ie, &sCursorX, &sCursorY, &sCursorZ, &sCursorEvent);
			
			UCHAR	ucId	= ms_pylonEdit->m_ucID;

			static U8*	pau8KeyStatus	= rspGetKeyStatusArray();

			// Move unit for arrow key movement of region.
			// Note: Combinations of SHIFT, CONTROL, and ALT
			// provide super fast movement.
			short	sMoveUnit	= 1;
			if (pau8KeyStatus[RSP_SK_SHIFT] & 1)
				sMoveUnit	*= 2;
			if (pau8KeyStatus[RSP_SK_CONTROL] & 1)
				sMoveUnit	*= 3;
			if (pau8KeyStatus[RSP_SK_ALT] & 1)
				sMoveUnit	*= 4;

			if (pau8KeyStatus[RSP_SK_LEFT] & 1)
				{
				ms_argns[ucId].sX	-= sMoveUnit;
				ms_spriteTriggerRgn.m_sX2 -= sMoveUnit;

				// Update sprite in scene.
				prealm->m_scene.UpdateSprite(&ms_spriteTriggerRgn);
				}
			else if (pau8KeyStatus[RSP_SK_RIGHT] & 1)
				{
				ms_argns[ucId].sX += sMoveUnit;
				ms_spriteTriggerRgn.m_sX2 += sMoveUnit;

				// Update sprite in scene.
				prealm->m_scene.UpdateSprite(&ms_spriteTriggerRgn);
				}
			else if (pau8KeyStatus[RSP_SK_UP] & 1)
				{
				ms_argns[ucId].sY -= sMoveUnit;
				ms_spriteTriggerRgn.m_sY2 -= sMoveUnit;

				// Update sprite in scene.
				prealm->m_scene.UpdateSprite(&ms_spriteTriggerRgn);
				}
			else if (pau8KeyStatus[RSP_SK_DOWN] & 1)
				{
				ms_argns[ucId].sY += sMoveUnit;
				ms_spriteTriggerRgn.m_sY2 += sMoveUnit;

				// Update sprite in scene.
				prealm->m_scene.UpdateSprite(&ms_spriteTriggerRgn);
				}

			// If unused event . . .
			if (ie.sUsed == FALSE)
				{
				if (ie.type == RInputEvent::Mouse)
					{
					sButtons	= ie.sButtons;
					}
				else
					{
					if (ie.type == RInputEvent::Key)
						{
						switch (ie.lKey & 0x0000FFFF)
							{
							case 27:
								EditPylonTriggerRegion(NULL);
								break;
							case EDIT_KEY_ENLARGE_DISPLAY1:
							case EDIT_KEY_ENLARGE_DISPLAY2:
							case EDIT_KEY_ENLARGE_DISPLAY3:
								if (ms_sDrawBlockSize + sMoveUnit > ms_sDrawBlockSize)
									{
									ms_sDrawBlockSize	+= sMoveUnit;
									}
								break;
							case EDIT_KEY_REDUCE_DISPLAY1:
							case EDIT_KEY_REDUCE_DISPLAY2:
								ms_sDrawBlockSize	-= sMoveUnit;
								if (ms_sDrawBlockSize < 2)
									{
									ms_sDrawBlockSize	= 2;
									}
								break;
							case EDIT_KEY_DECREASE_OPACITY:
								ms_spriteTriggerRgn.m_sAlphaLevel -= INCDEC_ALPHA_LEVEL;
								if (ms_spriteTriggerRgn.m_sAlphaLevel < MIN_ALPHA_LEVEL)
									{
									ms_spriteTriggerRgn.m_sAlphaLevel = MIN_ALPHA_LEVEL;
									}
								break;
							case EDIT_KEY_INCREASE_OPACITY:
								ms_spriteTriggerRgn.m_sAlphaLevel += INCDEC_ALPHA_LEVEL;
								if (ms_spriteTriggerRgn.m_sAlphaLevel > MAX_ALPHA_LEVEL)
									{
									ms_spriteTriggerRgn.m_sAlphaLevel = MAX_ALPHA_LEVEL;
									}
								break;
							}
						}
					}
				}

			switch (sButtons)
				{
				case 1:
					rspRect(
						TRIGGER_RGN_DRAW_INDEX,		// *** FUDGE ***.
						ms_argns[ucId].pimRgn,
						pcamera->m_sScene2FilmX + sCursorX - ms_argns[ucId].sX - ms_sDrawBlockSize / 2,
						pcamera->m_sScene2FilmY + sCursorZ - ms_argns[ucId].sY - ms_sDrawBlockSize / 2,
						ms_sDrawBlockSize,
						ms_sDrawBlockSize);
					break;
				case 2:
					rspRect(
						0,
						ms_argns[ucId].pimRgn,
						pcamera->m_sScene2FilmX + sCursorX - ms_argns[ucId].sX - ms_sDrawBlockSize / 2,
						pcamera->m_sScene2FilmY + sCursorZ - ms_argns[ucId].sY - ms_sDrawBlockSize / 2,
						ms_sDrawBlockSize,
						ms_sDrawBlockSize);
					break;
				}
			}
		else
			{
			// If unused key event . . .
			if (ie.type == RInputEvent::Key && ie.sUsed == FALSE)
				{
				// Force alpha keys to upper keys
				if (isalpha(ie.lKey & 0xffff))
					ie.lKey = (ie.lKey & 0xffff0000) | toupper(ie.lKey & 0xffff);

				// In case we're gonna scroll, set amount based on CTRL key status
				short sScrollX = EDIT_SCROLL_AMOUNT;
				short sScrollY = EDIT_SCROLL_AMOUNT;
				if (ie.lKey & RSP_GKF_CONTROL)
					{
					sScrollX = pcamera->m_sViewW;
					sScrollY = pcamera->m_sViewH;
					}

				// Check for special editor keys
				switch (ie.lKey)
					{
					case EDIT_KEY_LOADREALM:
					case EDIT_KEY_OPENREALM:
						ms_lPressedId	= GUI_ID_OPEN_REALM;

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_SAVEREALM:
						ms_lPressedId	= GUI_ID_SAVE_REALM_AS;

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_NEWREALM:
						ms_lPressedId	= GUI_ID_NEW_REALM;

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_TOGGLEBOUY:
						// Toggle the draw bouy flag
						ms_bDrawNetwork = !ms_bDrawNetwork;
						if (ms_bDrawNetwork)
						{
							CThing*	pThing;
							CListNode<CThing>* pNext = prealm->m_aclassHeads[CThing::CBouyID].m_pnNext;
							while (pNext->m_powner != NULL)
							{
								pThing = pNext->m_powner;
								if (pThing)
									if (pThing->m_phot)
										pThing->m_phot->SetActive(TRUE);
								pNext = pNext->m_pnNext;
							}	
							
							UpdateNetLines(prealm->GetCurrentNavNet());
							CBouy::Show();
						}
						else
						{
							CThing*	pThing;
							CListNode<CThing>* pNext = prealm->m_aclassHeads[CThing::CBouyID].m_pnNext;
							while (pNext->m_powner != NULL)
							{
								pThing = pNext->m_powner;
								if (pThing)
									if (pThing->m_phot)
										pThing->m_phot->SetActive(FALSE);
								pNext = pNext->m_pnNext;
							}	
							CBouy::Hide();
						}

						// Used the key
						ie.sUsed = TRUE;
						break;

					case EDIT_KEY_PLAY:
						ms_lPressedId	= GUI_ID_PLAY_REALM;

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_MENU:	// EDIT_KEY_CANCEL
						// If moving . . .
						if (ms_sMoving != FALSE)
							{
							CancelDrag(prealm);
							}
						else
							{
		#if 1
							// If menu is not running . . .
							if (GetCurrentMenu() == NULL)
								{
								// Fade out and preserve colors.
								PalTranOn();

								if (StartMenu(&menuEditor, &g_resmgrShell, g_pimScreenBuf) == 0)
									{
									// Hide the mouse.
									rspHideMouseCursor();
									}
								}
		#else
							ms_lPressedId	= GUI_ID_EXIT;
		#endif
							}

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_SCROLL_L:
						// Scroll left
						ms_sbHorz.SetPos(pcamera->m_sSceneViewX - sScrollX);

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_SCROLL_R:
						// Scroll right
						ms_sbHorz.SetPos(pcamera->m_sSceneViewX + sScrollX);

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_SCROLL_U:
						// Scroll up
						ms_sbVert.SetPos(pcamera->m_sSceneViewY - sScrollY);

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_SCROLL_D:
						// Scroll down
						ms_sbVert.SetPos(pcamera->m_sSceneViewY + sScrollY);

						// Used the key.
						ie.sUsed	= TRUE;
						break;

					case EDIT_KEY_ENLARGE_DISPLAY1:
					case EDIT_KEY_ENLARGE_DISPLAY2:
					case EDIT_KEY_ENLARGE_DISPLAY3:
						OnEnlargeDisplay(pcamera, prealm);
						break;

					case EDIT_KEY_REDUCE_DISPLAY1:
					case EDIT_KEY_REDUCE_DISPLAY2:
						OnReduceDisplay(pcamera, prealm);
						break;

					case EDIT_KEY_MODIFY1:
					case EDIT_KEY_MODIFY2:
						// Verify there is a selection . . .
						if (ms_pthingSel != NULL)
							{
							ASSERT(ms_photSel != NULL);
							// Modify.
							ms_pthingSel->EditModify();
							// Size may have changed.
							RRect	rc;
							ms_pthingSel->EditRect(&rc);
							// Update hot.
							ms_photSel->m_sX	= rc.sX;
							ms_photSel->m_sY	= rc.sY;
							ms_photSel->m_sW	= rc.sW;
							ms_photSel->m_sH	= rc.sH;
							}
						break;

					case EDIT_KEY_NEWITEM:
						ms_lPressedId	= GUI_ID_NEW_THING;
						break;

					case EDIT_KEY_NEXTITEM:
						NextItem(prealm);
						break;

					case EDIT_KEY_PREVITEM:
						PrevItem(prealm);
						break;

					case EDIT_KEY_DELETE_GROUP:
						DelClass(ms_pthingSel, prealm);
						break;

					case EDIT_KEY_DELETE_MOST:
						DelMost(prealm);
						break;
						
					case EDIT_KEY_DELETE:
						// Delete item.
						DelThing(ms_pthingSel, ms_photSel, prealm);
						break;

					case EDIT_KEY_DOS_COPY:
					case EDIT_KEY_COPY:
						ms_lPressedId	= GUI_ID_COPY;
						break;

					case EDIT_KEY_DOS_CUT:
					case EDIT_KEY_CUT:
						ms_lPressedId	= GUI_ID_CUT;
						break;

					case EDIT_KEY_DOS_PASTE:
					case EDIT_KEY_PASTE:
						ms_lPressedId	= GUI_ID_PASTE;
						break;

					case EDIT_KEY_DECREASE_OPACITY:
						if (ms_spriteAttributes.m_pImage)
							{
							ms_spriteAttributes.m_sAlphaLevel -= INCDEC_ALPHA_LEVEL;
							if (ms_spriteAttributes.m_sAlphaLevel < MIN_ALPHA_LEVEL)
								{
								ms_spriteAttributes.m_sAlphaLevel = MIN_ALPHA_LEVEL;
								}
							}
						break;

					case EDIT_KEY_INCREASE_OPACITY:
						if (ms_spriteAttributes.m_pImage)
							{
							ms_spriteAttributes.m_sAlphaLevel += INCDEC_ALPHA_LEVEL;
							if (ms_spriteAttributes.m_sAlphaLevel > MAX_ALPHA_LEVEL)
								{
								ms_spriteAttributes.m_sAlphaLevel = MAX_ALPHA_LEVEL;
								}
							}
						break;

					case EDIT_KEY_OVERSHOOT_EDGES1:
					case EDIT_KEY_OVERSHOOT_EDGES2:
						if (pcamera->m_bClip == true)
							{
							pcamera->m_bClip = false;
							// Update visual components.
							SizeUpdate(pcamera, prealm);
							}
						else
							{
							pcamera->m_bClip = true;
							// Update visual components.
							SizeUpdate(pcamera, prealm);
							}
						break;

					case EDIT_KEY_REALM_STATISTICS:
						{
						
						CThing* pthing;
						ShowRealmStatistics(prealm, &pthing);

						if (pthing)
							{
							SetSel(pthing, NULL);
							}

						break;
						}

					default:
						break;
					}
				}

			// Pass events to hotboxes before sucked up by edit cursor.
			ms_pguiGUIs->m_hot.Do(&ie);
			ms_pguiRealmBar->m_hot.Do(&ie);
			//If there is a hood . . .
			if (prealm->m_asClassNumThings[CThing::CHoodID] > 0)
				{
				// Do extra views.
				DoViews(&ie);
				// Do other GUIs.
				ms_pguiPickObj->m_hot.Do(&ie);
				ms_pguiLayers->m_hot.Do(&ie);
				ms_pguiNavNets->m_hot.Do(&ie);
				ms_pguiMap->m_hot.Do(&ie);
				ms_pguiShowAttribs->m_hot.Do(&ie);
				ms_pguiInfo->m_hot.Do(&ie);
				}

			ms_sbVert.m_hot.Do(&ie);
			ms_sbHorz.m_hot.Do(&ie);
			// If there's a hood hotbox and no item being dragged . . .
			if (ms_photHood != NULL && ms_sMoving == FALSE)
				{
				// Pass the event to the Thing hotboxes in Realm coords.
				RInputEvent	ieRealm	= ie;
				ieRealm.sPosX			+= pcamera->m_sScene2FilmX;
				ieRealm.sPosY			+= pcamera->m_sScene2FilmY;
				ms_photHood->Do(&ieRealm);

				// If used by editor hotbox . . .
				if (ie.sUsed != ieRealm.sUsed)
					{
					// Note that it was used by editor hotbox.
					ie.lUser	= 1;
					}
				}
			else
				{
				// If not yet used . . .
				if (ie.sUsed == FALSE)
					{
					// Make it an editor event.
					ie.lUser	= 1;
					}
				}

			// Get cursor position and event
			GetCursor(&ie, &sCursorX, &sCursorY, &sCursorZ, &sCursorEvent);
			
			// If there is a realm . . .
			if (prealm != NULL)
				{
				// Map the cursor position to a realm coordinate.
				MapScreen2Realm(prealm, pcamera, sCursorX, sCursorZ, &sCursorX, &sCursorY, &sCursorZ);
				}

			switch(sCursorEvent)
				{
				case CURSOR_LEFT_BUTTON_UP:
					break;

				case CURSOR_RIGHT_BUTTON_UP:
					break;

				case CURSOR_LEFT_DOUBLE_CLICK:
					break;

				case CURSOR_RIGHT_DOUBLE_CLICK:
					break;

				case CURSOR_LEFT_DRAG_BEGIN:
					// If there is a selection . . .
					if (ms_pthingSel != NULL)
						{
						// Let's not drag the hood . . .
						if (ms_pthingSel->GetClassID() != CThing::CHoodID)
							{
							// Move mode.
							ms_sMoving			= TRUE;

							// Make sure this is valid.
							ASSERT(ms_photSel != NULL);

							// Get hotspot for item in 2D.
							short	sOffsetX;
							short	sOffsetY;
							ms_pthingSel->EditHotSpot(&sOffsetX, &sOffsetY);

							// Reposition cursor to hotspot.
							rspSetMouse(
								(ms_photSel->m_sX - pcamera->m_sScene2FilmX) + sOffsetX,
								(ms_photSel->m_sY - pcamera->m_sScene2FilmY) + sOffsetY);

							// If cursor shown . . .
							if (rspGetMouseCursorShowLevel() > 0)
								{
								// Hide it.
								rspHideMouseCursor();
								}
							}
						}

					break;

				case CURSOR_LEFT_DRAG_END:
					
					DragDrop(
						sCursorX,		// x
						sCursorY,		// y
						sCursorZ);		// z
					
					break;

				case CURSOR_NOTHING:
					// If there is an item being moved . . .
					if (ms_sMoving != FALSE)
						{
						ASSERT(ms_pthingSel != NULL);
						ASSERT(ms_photSel != NULL);

						MoveThing(
							ms_pthingSel, 
							ms_photSel, 
							sCursorX,		// x
							sCursorY,		// y
							sCursorZ);		// z

						bool	bInsideNonScroll	= true;
						CNavigationNet* pNavNet = prealm->GetCurrentNavNet();
						if (pNavNet)
							UpdateNetLines(pNavNet);

						short	sScreenX;
						short	sScreenY;

						Maprealm2Screen(
							prealm,
							pcamera,
							sCursorX,
							0,
							sCursorZ,
							&sScreenX,
							&sScreenY);

						// If on or beyond any edge of the view . . .
						if (sScreenX < DRAG_SCROLL_BUFFER)
							{
							if (ms_bDragScroll == true)
								{
								ms_sbHorz.SetPos(ms_sbHorz.GetPos() + (sScreenX - DRAG_SCROLL_BUFFER) * EDGE_MOVEMENT_MULTIPLIER);
								}
							
							// Note that the cursor is not in the non-scroll area.
							bInsideNonScroll	= false;
							}
						else if (sScreenX > pcamera->m_sViewW - DRAG_SCROLL_BUFFER)
							{
							if (ms_bDragScroll == true)
								{
								ms_sbHorz.SetPos(ms_sbHorz.GetPos() + (sScreenX - (pcamera->m_sViewW - DRAG_SCROLL_BUFFER)) * EDGE_MOVEMENT_MULTIPLIER);
								}
							
							// Note that the cursor is not in the non-scroll area.
							bInsideNonScroll	= false;
							}

						if (sScreenY < DRAG_SCROLL_BUFFER)
							{
							if (ms_bDragScroll == true)
								{
								ms_sbVert.SetPos(ms_sbVert.GetPos() + (sScreenY - DRAG_SCROLL_BUFFER) * EDGE_MOVEMENT_MULTIPLIER);
								}
							
							// Note that the cursor is not in the non-scroll area.
							bInsideNonScroll	= false;
							}
						else if (sScreenY > pcamera->m_sViewH - DRAG_SCROLL_BUFFER)
							{
							if (ms_bDragScroll == true)
								{
								ms_sbVert.SetPos(ms_sbVert.GetPos() + (sScreenY - (pcamera->m_sViewH - DRAG_SCROLL_BUFFER)) * EDGE_MOVEMENT_MULTIPLIER);
								}
							
							// Note that the cursor is not in the non-scroll area.
							bInsideNonScroll	= false;
							}

						// If inside the non-scroll area . . .
						if (bInsideNonScroll == true)
							{
							// Enable scrolling.
							ms_bDragScroll	= true;
							}
						}
					break;

				default:
					break;
				}

			// Do GUI input focus stuff.
			RGuiItem::DoFocus(&ie);

			// Switch on ID of item clicked.
			switch (ms_lPressedId)
				{
				case 0:	// Nothin'.
					break;

				case GUI_ID_NEW_REALM:
					// Create new realm
					NewRealm(prealm);
					// Setup camera.
					pcamera->SetHood(prealm->m_phood);
					m_pBouyLink0 = NULL;
					m_pBouyLink1 = NULL;
					break;

				case GUI_ID_OPEN_REALM:
					// Load realm
					LoadRealm(prealm);
					m_pBouyLink0 = NULL;
					m_pBouyLink1 = NULL;
					break;

				case GUI_ID_SAVE_REALM:
					// Save realm
					SaveRealm(prealm);
					break;

				case GUI_ID_SAVE_REALM_AS:
					// Save realm
					SaveRealmAs(prealm);
					break;

				case GUI_ID_PLAY_REALM:
					if (prealm->m_phood != NULL)
						{
						CancelDrag(prealm);

						// Simulate playing the realm
						PlayRealm(prealm, ms_pthingSel);

						// Restore global camera.
						ms_pcameraCur	= pcamera;

						// Display size may have changed.
						SizeUpdate(pcamera, prealm);
						}
					else
						{
						// ***LOCALIZE***.
						rspMsgBox(
							RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
							"Play a Realm",
							"There is no hood.  Cannot play.");
						}

					break;

				case GUI_ID_CLOSE_REALM:
					// Clear the ream.
					CloseRealm(prealm);
					m_pBouyLink0 = NULL;
					m_pBouyLink1 = NULL;
					break;

				case GUI_ID_EXIT:
					// If we can close the realm . . .
					if (CloseRealm(prealm) == 0)
						{
						// Exit the editor
						bExit = true;
						m_pBouyLink0 = NULL;
						m_pBouyLink1 = NULL;
						}
					break;

				case GUI_ID_PROPERTIES:
					// Call the realm's edit modify dialog function
					prealm->EditModify();
					break;

				case GUI_ID_TOGGLE_LAYER:
					{
					// Get currently selected list item.
					RGuiItem*	pguiSel	= ms_plbLayers->GetSel();
					if (pguiSel != NULL)
						{
						// Get layer to toggle.
						CRealm::Layer	layer	= (CRealm::Layer)pguiSel->m_ulUserData;
						// Should be push btn.
						ASSERT(pguiSel->m_type == RGuiItem::PushBtn);
						// Toggle.
						prealm->m_scene.m_pLayers[layer].m_bHidden 
							= (((RPushBtn*)pguiSel)->m_state == RPushBtn::On) ? false : true;
						}
					break;
					}

				case GUI_ID_NEW_CAMERA:
					{
					AddView(prealm);
					break;
					}

				case GUI_ID_CLOSE_CAMERA:
					{
					RemoveView(NULL);
					break;
					}

				case GUI_ID_REALM_VISIBLE:
					ms_pguiRealmBar->SetVisible(!ms_pguiRealmBar->m_sVisible);
					break;
				case GUI_ID_LAYERS_VISIBLE:
					ms_pguiLayers->SetVisible(!ms_pguiLayers->m_sVisible);
					break;
				case GUI_ID_THINGS_VISIBLE:
					ms_pguiPickObj->SetVisible(!ms_pguiPickObj->m_sVisible);
					break;
				case GUI_ID_CAMERAS_VISIBLE:
					ms_pguiCameras->SetVisible(!ms_pguiCameras->m_sVisible);
					break;
				case GUI_ID_NAVNETS_VISIBLE:
					ms_pguiNavNets->SetVisible(!ms_pguiNavNets->m_sVisible);
					break;
				case GUI_ID_MAP_VISIBLE:
					ms_pguiMap->SetVisible(!ms_pguiMap->m_sVisible);
					break;
				
				case GUI_ID_MAP_REFRESH:
					RefreshMap(prealm);
					break;

				case GUI_ID_COPY:
					// Copy the selection to the paste buffer.
					CopyItem(ms_pthingSel);
					break;

				case GUI_ID_CUT:
					// Copy the selection to the paste buffer.
					CopyItem(ms_pthingSel);
					// Delete the selection.
					DelThing(ms_pthingSel, ms_photSel, prealm);
					break;

				case GUI_ID_PASTE:
					// Paste from the paste buffer.
					PasteItem(
						prealm,		// In:  The realm to paste into.
						sCursorX,	// x
						sCursorY,	// y
						sCursorZ);	// z
					break;

				case GUI_ID_NEW_THING:
					{
					// Drop anything currently being dragged.
					DragDrop(
						sCursorX,		// x
						sCursorY,		// y
						sCursorZ);		// z

					// Get object picker list box.
					RListBox*	plbPicker	= (RListBox*)ms_pguiPickObj->GetItemFromId(GUI_ID_PICK_LIST);
					if (plbPicker != NULL)
						{
						RGuiItem*	pguiSel	= plbPicker->GetSel();
						// If there is a selection . . .
						if (pguiSel != NULL)
							{
							// Get ID of list item representing thing to create.
							ms_lPressedId	= pguiSel->m_lId;
							}
						else
							{
							break;
							}
						}
					else
						{
						break;
						}
					}

					////// INTENTIONAL FALL THROUGH TO DEFAULT CASE //////

				default:
					// If the range of CThings . . .
					if (	ms_lPressedId >= LIST_ITEM_GUI_ID_BASE
						&&	ms_lPressedId < LIST_ITEM_GUI_ID_BASE + CThing::TotalIDs
						&& ms_sMoving == FALSE)
						{
						CThing*	pthingNew;
						RHot*		photNew;
						if (CreateNewThing(								// CThing* to new thing.
							prealm,											// Realm to create in.
							ms_lPressedId - LIST_ITEM_GUI_ID_BASE,	// ID to create.
							sCursorX,										// x
							sCursorY,										// y
							sCursorZ,										// z
							&pthingNew,										// New thing.
							&photNew)	== 0)								// New hotbox for thing.
							{
							// Select the new item.
							SetSel(pthingNew, photNew);

							// Start moving/placing object.
							ms_sMoving	= TRUE;

							// Disable drag view scrolling.
							ms_bDragScroll	= false;

							// Enter drag.  Cheesy...Just need to get this to Bill and
							// Then I'll clean up.
							ms_sDragState	= 4;

							if (rspGetMouseCursorShowLevel() > 0)
								{
								rspHideMouseCursor();
								}
							}
						}
					break;
				}

			// If the mapped is pressed . . .
			if (ms_pguiMapZone != NULL)
				{
				if (ms_pguiMapZone->m_sPressed != FALSE)
					{
					// Determine position.
					short	sPosX, sPosY;
					rspGetMouse(&sPosX, &sPosY, NULL);
					ms_pguiMapZone->TopPosToClient(&sPosX, &sPosY);
					// Map position into map coords scroll to it.
					ms_sbVert.SetPos(sPosY / ms_dMapRatio - pcamera->m_sViewH / 2);
					ms_sbHorz.SetPos(sPosX / ms_dMapRatio - pcamera->m_sViewW / 2);
					}
				}
			}
	
		// Clear ID.
		ms_lPressedId	= 0;
		}

	// If quitting . . .
	if (rspGetQuitStatus() != FALSE)
		{
		bExit	= true;
		// Make sure to close the realm...kind've a hack doing it here.
		CloseRealm(prealm);
		}

	// Return cursor position values.
	*psCursorX	= sCursorX;
	*psCursorY	= sCursorY;
	*psCursorZ	= sCursorZ;

	return bExit;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Do ALL editor output.
//
////////////////////////////////////////////////////////////////////////////////
static void DoOutput(	// Returns nothing.
	CRealm*	prealm,		// Ptr to current realm.
	CCamera*	pcamera,		// Ptr to current camera.
	short		sCursorX,	// Cursor X position.
	short		sCursorY,	// Cursor Y position.
	short		sCursorZ)	// Cursor Z position.
	{
	// If not in menu . . .
	if (GetCurrentMenu() == NULL)
		{
		// Lock the composite buffer for much access.
		rspLockBuffer();

		// Update and render realm (in edit mode)
		prealm->EditUpdate();
		prealm->EditRender();

		// Need hood for this . . .
		if (prealm->m_asClassNumThings[CThing::CHoodID] > 0)
			{
			// If showing any attributes . . .
			if (ms_spriteAttributes.m_pImage)
				{
				// Clear the image.
				rspRect(
					0,
					ms_spriteAttributes.m_pImage,
					0, 0,
					ms_spriteAttributes.m_pImage->m_sWidth,
					ms_spriteAttributes.m_pImage->m_sHeight);

				prealm->m_scene.UpdateSprite(&ms_spriteAttributes);
				}
			else
				{
				// Get outta there.
				prealm->m_scene.RemoveSprite(&ms_spriteAttributes);
				}

			// If showing any terrain attributes . . .
			if (ms_u16TerrainMask)
				{
				AttribBlit(									// Returns nothing.                   
					prealm->m_pTerrainMap,				// In:  Multigrid of attributes.      
					ms_u16TerrainMask,					// In:  Mask of important attributes. 
					ms_spriteAttributes.m_pImage,		// In:  Destination image.            
					pcamera->m_sScene2FilmX,			// In:  Where in Multigrid to start.  
					pcamera->m_sScene2FilmY,			// In:  Where in Multigrid to start.  
					pcamera->m_sViewW,					// In:  How much of multigrid to use. 
					pcamera->m_sViewH);					// In:  How much of multigrid to use. 
				}

			// If showing any layer attributes . . .
			if (ms_u16LayerMask)
				{
				AttribBlit(									// Returns nothing.                   
					prealm->m_pLayerMap,					// In:  Multigrid of attributes.      
					ms_u16LayerMask,						// In:  Mask of important attributes. 
					ms_spriteAttributes.m_pImage,		// In:  Destination image.            
					pcamera->m_sScene2FilmX,			// In:  Where in Multigrid to start.  
					pcamera->m_sScene2FilmY,			// In:  Where in Multigrid to start.  
					pcamera->m_sViewW,					// In:  How much of multigrid to use. 
					pcamera->m_sViewH);					// In:  How much of multigrid to use. 
				}
			}

		// If avoiding clipping . . .
		if (pcamera->m_bClip == false)
			{
			// Keep it clean.
			pcamera->SnapWithLensCoverOn();
			}

		// Snap a picture.
		pcamera->Snap();

		// Draw the bouy connection line
		DrawBouyLink(prealm, pcamera);
		DrawNetwork(prealm, pcamera);

		// If there's a selected item . . .
		if (ms_pthingSel != NULL)
			{
			static RRect	rc;
			static short	sColorSwap	= 0;
			sColorSwap	= (sColorSwap + 1) % 2;

			ms_pthingSel->EditRect(&rc);
			rspRect(
				SELECTION_THICKNESS, 
				(sColorSwap == 0) ? SELECTION_COLOR1 : SELECTION_COLOR2, 
				g_pimScreenBuf, 
				rc.sX - pcamera->m_sScene2FilmX - 1, 
				rc.sY - pcamera->m_sScene2FilmY - 1, 
				rc.sW + 2, 
				rc.sH + 2);
			rspRect(
				SELECTION_THICKNESS, 
				(sColorSwap == 1) ? SELECTION_COLOR1 : SELECTION_COLOR2, 
				g_pimScreenBuf, 
				rc.sX - pcamera->m_sScene2FilmX - SELECTION_THICKNESS - 1, 
				rc.sY - pcamera->m_sScene2FilmY - SELECTION_THICKNESS - 1, 
				rc.sW + SELECTION_THICKNESS * 2 + 2, 
				rc.sH + SELECTION_THICKNESS * 2 + 2);
			}

		// If there's no current hood . . .
		if (prealm->m_phood == NULL)
			{
			// Erase entire screen.
			rspRect(
				RSP_BLACK_INDEX,
				g_pimScreenBuf,
				0, 0,
				g_pimScreenBuf->m_sWidth,
				g_pimScreenBuf->m_sHeight);
			}
		else
			{
			// Erase non-view area of screen.
			rspRect(
				RSP_BLACK_INDEX,
				g_pimScreenBuf,
				g_pimScreenBuf->m_sWidth - DISPLAY_RIGHT_BORDER, 
				0,
				DISPLAY_RIGHT_BORDER,
				g_pimScreenBuf->m_sHeight);
			rspRect(
				RSP_BLACK_INDEX,
				g_pimScreenBuf,
				0, 
				g_pimScreenBuf->m_sHeight - DISPLAY_BOTTOM_BORDER,
				g_pimScreenBuf->m_sWidth,
				DISPLAY_BOTTOM_BORDER);
			}

		// Update GUIs to composite buffer.
		ms_sbVert.Draw(g_pimScreenBuf);
		ms_sbHorz.Draw(g_pimScreenBuf);
		//If there is a hood . . .
		if (prealm->m_asClassNumThings[CThing::CHoodID] > 0)
			{
			ms_pguiLayers->Draw(g_pimScreenBuf);
			ms_pguiPickObj->Draw(g_pimScreenBuf);
			ms_pguiInfo->Draw(g_pimScreenBuf);
			ms_pguiShowAttribs->Draw(g_pimScreenBuf);
			ms_pguiMap->Draw(g_pimScreenBuf);
			ms_pguiNavNets->Draw(g_pimScreenBuf);
			// Draw extra views.
			DrawViews(prealm);
			}

		ms_pguiRealmBar->Draw(g_pimScreenBuf);
		ms_pguiGUIs->Draw(g_pimScreenBuf);

		// If editting a pylon . . .
		if (ms_pylonEdit != NULL)
			{
			UCHAR	ucId	= ms_pylonEdit->m_ucID;
			ASSERT(ms_argns[ucId].pimRgn != NULL);

			// Draw trigger region.
#if 0	// Now done with a sprite.
			rspBlitT(
				0,															// Src transparent color or index.
				ms_argns[ucId].pimRgn,								// Src.
				g_pimScreenBuf,										// Dst.
				0,															// Src.
				0,															// Src.
				ms_argns[ucId].sX - pcamera->m_sScene2FilmX,	// Dst.
				ms_argns[ucId].sY - pcamera->m_sScene2FilmY,	// Dst.
				ms_argns[ucId].pimRgn->m_sWidth,					// Both.
				ms_argns[ucId].pimRgn->m_sHeight,				// Both.
				NULL,														// Dst.
				NULL														// Src.
				);
#endif

			// Draw border bounding max bounding rect for region.
			rspRect(
				1,
				246 + (GetRand() % 10),	// *** HOKEY(sp?) ***
				g_pimScreenBuf,
				ms_argns[ucId].sX - pcamera->m_sScene2FilmX - 1,
				ms_argns[ucId].sY - pcamera->m_sScene2FilmY - 1,
				ms_argns[ucId].pimRgn->m_sWidth + 2,
				ms_argns[ucId].pimRgn->m_sHeight + 2
				);

			// Draw cursor.
			rspRect(
				1,
				246 + (GetRand() % 10),	// *** HOKEY(sp?) ***
				g_pimScreenBuf,
				sCursorX - ms_sDrawBlockSize / 2,
				sCursorZ - ms_sDrawBlockSize / 2,
				ms_sDrawBlockSize,
				ms_sDrawBlockSize
				);
			}

		// If placing/moving . . .
		if (ms_sMoving != FALSE)
			{
			// Draw cursor at current mouse position
			DrawCursor(sCursorX, sCursorY, sCursorZ, g_pimScreenBuf, prealm, pcamera);
			}

		// Whew.  Unlock buffer now that we're done.
		rspUnlockBuffer();
		}
	else
		{
		// Menu.
		DoMenuOutput(g_pimScreenBuf);
		}

	// Update screen
	rspUpdateDisplay();
	}

////////////////////////////////////////////////////////////////////////////////
//
// Get cursor position and event
// Ignores events that have already been used.
//
////////////////////////////////////////////////////////////////////////////////
static void GetCursor(	// Returns nothing.
	RInputEvent*	pie,	// In:  Input event.
								// Out: pie->sUsed = TRUE, if used.
	short* psX,				// Out: X coord of event.
	short* psY,				// Out: Y coord of event.
	short* psZ,				// Out: Z coord of event.
	short* psEvent)		// Out: Event type.
	{
	// Init mouse drag stuff
	static short sDragX;
	static short sDragY;
	static long lDragTime;

	// Init mouse pressed stuff.
	static short sPressed	= FALSE;

	// Init cursor stuff
	static short sCursorY = 0;
	static short sOrigCursorY;

	// Default to no event
	*psEvent = CURSOR_NOTHING;

	ASSERT(pie != NULL);

	short sMouseX			= pie->sPosX;
	short sMouseY			= pie->sPosY;
	short sButtonEvent	= pie->sEvent;

	// If mouse event . . .
	if (pie->type == RInputEvent::Mouse)
		{
		// Evaluate button stuff
		switch (sButtonEvent)
			{
			case RSP_MB0_PRESSED:
				// If occurred in one of our hots . . .
				if (pie->lUser == 1)
					{
					// Note pressed.
					sPressed	= TRUE;

					if (ms_sDragState == 0)
						{
						ms_sDragState = 1;
						sDragX = sMouseX;
						sDragY = sMouseY;
						// Might as well use the actual time the event occurred, no?
						lDragTime = pie->lTime;

						// We used the event.
						pie->sUsed	= TRUE;
						}
					}
				break;

			case RSP_MB0_RELEASED:
				// If we didn't get the down, we should not process this event.
				if (sPressed != FALSE)
					{
					// If button is released after drag mode was already started, then
					// it acts to end drag mode.  Otherwise, we kill any chance of drag
					// mode and use the button for whatever else it might be needed for.
					if (ms_sDragState >= 2)
						{
						if (ms_sDragState == 3)
							{
							// Set mouse position back to where it was where when the drag
							// started.  The idea is to hide the drag so the cursor doesn't "jump".
							sMouseX = sDragX;
							sMouseY = sDragY;
							rspSetMouse(sMouseX, sMouseY);

							// We used the event.
							pie->sUsed	= TRUE;
							}
						else
							{
							// Return drag event
							*psEvent = CURSOR_LEFT_DRAG_END;

							// We used the event.
							pie->sUsed	= TRUE;
							}
						}
					else
						{
						// If not yet used . . .
						if (pie->sUsed == FALSE)
							{
							// Return button event
							*psEvent = CURSOR_LEFT_BUTTON_UP;

							// We used the event.
							pie->sUsed	= TRUE;
							}
						}

					// Clera pressed state.
					sPressed		= FALSE;

					// Clear drag state
					ms_sDragState = 0;
					}

				break;

			case RSP_MB0_DOUBLECLICK:
				// A double-click SHOULD end drag mode, except that it's possible
				// we assumed it was a drag, and started doing a drag, but now the
				// system sees a second click and decides it was a double-click
				// instead.  So I decided that if we're already in "full" drag
				// mode, we ignore the double-click.  Otherwise, it ends drag mode.
				if (ms_sDragState < 2)
					{
					ms_sDragState = 0;
					*psEvent = CURSOR_LEFT_DOUBLE_CLICK;
					}

				// We used the event.
				pie->sUsed	= TRUE;

				break;

			case RSP_MB1_DOUBLECLICK:
				// If occurred in one of our hots . . .
				if (pie->lUser == 1)
					{
					*psEvent	= CURSOR_RIGHT_DOUBLE_CLICK;
					// We used the event.
					pie->sUsed	= TRUE;
					}
				break;

			default:
				break;
			}
		}
	else
		{
		// If there was no mouse event, just get the current mouse position
		rspGetMouse(&sMouseX, &sMouseY, NULL);
		}
	
	// If we're in "maybe" drag mode, check if we should go to "full" drag mode
	if (ms_sDragState == 1)
		{
		// Full drag mode is entered if a minimum amount of time has elapsed or
		// if a minimum movement of the mouse has occurred.  This is assuming
		// that the mouse button is still down from when it first went down.
		if (((rspGetMilliseconds() - lDragTime) >= DRAG_MIN_TIME) ||
			(ABS(sMouseX - sDragX) >= DRAG_MIN_X) ||
			(ABS(sMouseY - sDragY) >= DRAG_MIN_Y))
			ms_sDragState = 2;
		}

	// If we reach full drag mode, then we still need to determine whether to
	// pass this drag event back to the caller or use it ourselves as a way to
	// change the cursor's Y-coord (height).  I'm not sure how to do it, but
	// it will probably mean checking for collisions with objects at this
	// level.  If we are over an object, we would return the drag event.  If
	// not, we would use the drag ourselves.
	if (ms_sDragState == 2)
		{
		// For now, it's hardwired to never assume the drag is used to modify the cursor
		if (0)
			{
			// Save info we need when adjusting cursor's y coord
			sOrigCursorY = sCursorY;
			ms_sDragState	= 3;	// Stretchy cursor mode.
			}
		else
			{
			// Return drag event
			*psEvent = CURSOR_LEFT_DRAG_BEGIN;
			ms_sDragState	= 4;	// Drag mode.
			}

		}

	// This is the special drag state that means we're adjusting the cursor's y coord
	if (ms_sDragState == 3)
		{
		sCursorY = sOrigCursorY + (sDragY - sMouseY);
		*psX = sDragX;		// Keep cursor position where it was at start of drag!
		*psY = sCursorY;
		*psZ = sDragY;		// Keep cursor position where it was at start of drag!
		}
	else
		{
		// Move cursor to latest mouse position
		*psX = sMouseX;
		*psY = sCursorY;
		*psZ = sMouseY;
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Init cursor
//
////////////////////////////////////////////////////////////////////////////////
static short InitCursor(
	void)
	{
	short sResult = 0;

	m_pimCursorBase = new RImage;
	ASSERT(m_pimCursorBase != NULL);
	if (m_pimCursorBase->Load(FullPath(GAME_PATH_VD, CURSOR_BASE_IMAGE_FILE) ) != 0)
		{
		sResult = -1;
		TRACE("LoadCursor(): Couldn't load cursor file: %s\n", 
			FullPath(GAME_PATH_VD, CURSOR_BASE_IMAGE_FILE));
		goto Exit;
		}

	if (m_pimCursorBase->Convert(RImage::FSPR8) != RImage::FSPR8)
		{
		sResult = -1;
		TRACE("LoadCursor(): Couldn't convert cursor base to FSPR8!\n");
		goto Exit;
		}

	m_pimCursorTip = new RImage;
	ASSERT(m_pimCursorTip != NULL);
	if (m_pimCursorTip->Load(FullPath(GAME_PATH_VD, CURSOR_TIP_IMAGE_FILE) ) != 0)
		{
		sResult = -1;
		TRACE("LoadCursor(): Couldn't load cursor file: %s\n", 
			FullPath(GAME_PATH_VD, CURSOR_TIP_IMAGE_FILE) );
		goto Exit;
		}

	if (m_pimCursorTip->Convert(RImage::FSPR8) != RImage::FSPR8)
		{
		sResult = -1;
		TRACE("LoadCursor(): Couldn't convert cursor tip to FSPR8!\n");
		goto Exit;
		}

Exit:
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Kill cursor
//
////////////////////////////////////////////////////////////////////////////////
static void KillCursor(
	void)
	{
	// Delete cursors (delete doesn't mind if pointer is already 0)
	delete m_pimCursorBase;
	m_pimCursorBase = 0;
	delete m_pimCursorTip;
	m_pimCursorTip = 0;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Draw edit cursor at specified location on specified image
//
////////////////////////////////////////////////////////////////////////////////
static void DrawCursor(
	short sCursorX,										// In:  Cursor hotspot x coord
	short sCursorY,										// In:  Cursor hotspot y coord
	short sCursorZ,										// In:  Cursor hotspot z coord
	RImage* pimDst,										// In:  Image to draw to
	CRealm* prealm,										// In:  Realm.
	CCamera* pcamera)										// In:  Camera on prealm.
	{
	// Convert to 2D.
	short	sBaseX2;
	short	sBaseY2;
	Maprealm2Screen(prealm, pcamera, sCursorX, 0, sCursorZ, &sBaseX2, &sBaseY2);

	short	sTipX2;
	short	sTipY2;
	Maprealm2Screen(prealm, pcamera, sCursorX, sCursorY, sCursorZ, &sTipX2, &sTipY2);

	// Draw the base (I think this is the hotspot).
	rspBlit(
		m_pimCursorBase, 
		pimDst, 
		sBaseX2 - CURSOR_BASE_HOTX, 
		sBaseY2 - CURSOR_BASE_HOTY);

	// Draw the top offset from the base by y coord (needs to be negated here!)
	rspBlit(
		m_pimCursorTip, 
		pimDst, 
		sTipX2 - CURSOR_TIP_HOTX, 
		sTipY2 - CURSOR_TIP_HOTY);

	// If height is not 0 . . .
	if (sCursorY != 0)
		{
		// Draw a line connecting the base and the tip.
#if 0
		short sMin = MIN(sCursorZ, (short)(sCursorZ - sCursorY));
		short sMax = MAX(sCursorZ, (short)(sCursorZ - sCursorY));
		for (short y = sMin; y <= sMax; y++)
			rspPlot((UCHAR)255, pimDst, sCursorX, y);
#else
		rspLine(
			(UCHAR)255,
			pimDst,
			sBaseX2,
			sBaseY2,
			sTipX2,
			sTipY2);
#endif
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Create new realm
//
////////////////////////////////////////////////////////////////////////////////
static short NewRealm(
	CRealm* prealm)
	{
	short sResult = 0;

	// Close realm in case it contains anything
	sResult	= CloseRealm(prealm);
	if (sResult == 0)
		{
		// Set the bouy lines to default to on
		ms_bDrawNetwork = true;

		// Clear the network lines from the previous level (if any)
		UpdateNetLines(NULL);

		// Set disk path for this realm.
		prealm->m_resmgr.SetBasePath(g_GameSettings.m_szNoSakDir);

		CThing*	pthing;
		short		sResult	= CreateNewThing(prealm, CThing::CHoodID, 0, 0, 0, &pthing, &ms_photHood);
		// Create hood object because we can't really do anything without it
		if (sResult == 0)
			{
			RHot*		photdummy;
			sResult	= CreateNewThing(prealm, CThing::CGameEditThingID, 0, 0, 0, &pthing, &photdummy);
			// Create editor object . . .
			if (sResult == 0)
				{
				// Store ptr to GameEdit thing.
				ms_pgething	= (CGameEditThing*)pthing;
				ms_pgething->m_plbNavNetList = (RListBox*) ms_pguiNavNets->GetItemFromId(GUI_ID_NAVNET_LIST);

				sResult	= CreateNewThing(prealm, CThing::CNavigationNetID, 150, 0, 50, &pthing, &ms_photSel);
				if (sResult == 0)
					{
					// Success.
					// Update size affected stuff.
					SizeUpdate(ms_pcameraCur, prealm);

					// Set the palette.
					prealm->m_phood->SetPalette();
					
					// Freshen the map.
					RefreshMap(prealm);

					sResult = CreateNewThing(prealm, CThing::CTriggerID, 0, 0, 0, &pthing, &photdummy);
					if (sResult == 0) 
						{
						prealm -> m_pTriggerMap = ((CTrigger*)pthing)->m_pmgi;
						}
					}
				else
					{
					TRACE("GameEdit:NewRealm - Failed to create default NavNet.\n");
					TRACE(" other objects depending on one may not work correctly.\n");
					}
				}
			}
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Close realm.
//
////////////////////////////////////////////////////////////////////////////////
static short CloseRealm(
	CRealm* prealm)
	{
	short sResult = 0;

	// Cancel drag, if one is in progress.
	CancelDrag(prealm);

	// Cancle network line draw if it is on
	if (ms_bDrawNetwork)
		ms_bDrawNetwork = false;

	// Clear the Navigation Net list box in the editor
	if (ms_pgething && ms_pgething->m_plbNavNetList)
		ms_pgething->m_plbNavNetList->RemoveAll();

	// If the realm has anything worth saving . . .
	if (prealm->m_sNumThings > 0)
		{
		// Check for save . . .
		switch (rspMsgBox(
			RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNOCANCEL,
			g_pszAppName,
			g_pszSaveFileQuery) )
			{
			case RSP_MB_RET_YES:
				sResult	= SaveRealm(prealm);
				break;
			case RSP_MB_RET_NO:
				break;
			case RSP_MB_RET_CANCEL:
				// User abort.
				sResult	= 1;
				break;
			}
		}

	// If successful so far . . .
	if (sResult == 0)
		{
		if (ms_pcameraCur != NULL)
			{
			// Clear hood ptr.
			ms_pcameraCur->SetHood(NULL);
			}
		
		// Clear realm in case it contains anything
		prealm->Clear();

		// This's gone now.
		ms_pgething	= NULL;

		// If there are any . . .
		if (ms_photHood != NULL)
			{
			// Destroy all child hotboxes.
			RHot*	phot	= ms_photHood->m_listChildren.GetHead();
			while (phot != NULL)
				{
				delete phot;

				phot	= ms_photHood->m_listChildren.GetNext();
				}

			// Destroy root/hood hotbox.
			delete ms_photHood;
			ms_photHood	= NULL;
			}

		// Clean up trigger regions.
		short i;
		for (i = 0; i < NUM_ELEMENTS(ms_argns); i++)
			{
			ms_argns[i].Destroy();
			}

		// Better clear these.
		ms_sMoving		= FALSE;
		SetSel(NULL, NULL);

		// Set filename such that initially open and save dialogs start in
		// *.RLM dir.
		strcpy(ms_szFileName, FullPathVD(INITIAL_REALM_DIR));

		// Update size dependent stuff.
		SizeUpdate(ms_pcameraCur, prealm);

		// Purge resource managers.
		g_resmgrGame.Purge();
		prealm->m_resmgr.Purge();
		}
	
	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Load realm.
//
////////////////////////////////////////////////////////////////////////////////
static short LoadRealm(
	CRealm* prealm)
	{
	short sResult = 0;

	// Close current realm.
	sResult	= CloseRealm(prealm);
	if (sResult == 0)
		{
		// Attempt to get filename . . .
		// ***LOCALIZE***
		sResult	= rspOpenBox(
			"Load Realm", 
			ms_szFileName, 
			ms_szFileName, 
			sizeof(ms_szFileName), 
			".rlm");
		if (sResult == 0)
			{
			// Attach file counter callback to RFile and setup necessary components.
			InitFileCounter("Loading -- %ld bytes so far.");
			
			// Convert to RSPiX format b/c CRealm::Load() now likes it that way.
			char	szRealmName[sizeof(ms_szFileName)];
			strcpy(szRealmName, rspPathFromSystem(ms_szFileName) );

			// Load realm in edit mode
			sResult = prealm->Load(szRealmName, true);
			// Clean and detach file counter.
			KillFileCounter();
			// If load was successful . . .
			if (sResult == 0)
				{
				// Get the editor thing.
				ms_pgething	= GetEditorThing(prealm);

				// Start the realm.
				prealm->Startup();

				if (prealm->m_phood)
					{
					// Set hood's palette.
					prealm->m_phood->SetPalette();
					}

				// Store ptr to GameEdit thing.
				CListNode<CThing>* pEditorList = prealm->m_aclassHeads[CThing::CGameEditThingID].m_pnNext;
				CGameEditThing* peditor = (CGameEditThing*) pEditorList->m_powner;
				if (peditor)
					{
					peditor->m_plbNavNetList = (RListBox*) ms_pguiNavNets->GetItemFromId(GUI_ID_NAVNET_LIST);
					}

				///////////////////////////////////////////////////////////////////
				// Create Hot for every CThing.
				///////////////////////////////////////////////////////////////////
				// Get the hood . . .
				if (prealm->m_asClassNumThings[CThing::CHoodID] > 0)
					{
					RRect		rc;
					// Get first and only Hood iterator.
					CHood* phood = (CHood*) prealm->m_aclassHeads[CThing::CHoodID].m_pnNext->m_powner;
					// Get rectangle.
					phood->EditRect(&rc);
					// Create and setup RHot for hood.
					phood->m_phot	= ms_photHood	= new RHot(
						rc.sX,							// Position.
						rc.sY,							// Position.
						rc.sW,							// Dimensions.
						rc.sH,							// Dimensions.
						ThingHotCall,					// Callback.
						TRUE,								// TRUE, if active.
						(U32)phood,						// User value (CThing*).
						FRONTMOST_HOT_PRIORITY);	// New items towards front.
					// If successful . . .
					if (ms_photHood != NULL)
						{
						// Setup hotboxes for all objects.
						CListNode<CThing>* pList;
						CThing*  pthing;
						short	sActivateHot;
						pList = prealm->m_everythingHead.m_pnNext;
						while (pList->m_powner != NULL && sResult == 0)
							{
							pthing	= pList->m_powner;
							// Already got one for Hood.  If not the Hood . . .
							if (pthing->GetClassID() != CThing::CHoodID)
								{
								sActivateHot	= TRUE;

								// Some types may need to hook in here.
								switch (pthing->GetClassID() )
									{
									case CThing::CBouyID:
										// If bouy lines are hidden . . .
										if (ms_bDrawNetwork == false)
											{
											// Don't activate new bouy hots.
											sActivateHot	= FALSE;
											}
										break;

									case CThing::CNavigationNetID:
										((CNavigationNet*) pthing)->EditPostLoad();
										break;
									}

								// Get rectangle.
								pthing->EditRect(&rc);
								// Create and setup RHot.
								pthing->m_phot	= new RHot(
									rc.sX,							// Position.
									rc.sY,							// Position.
									rc.sW,							// Dimensions.
									rc.sH,							// Dimensions.
									ThingHotCall,					// Callback.
									sActivateHot,					// TRUE, if initially active.
									(U32)pthing,					// User value (CThing*).
									FRONTMOST_HOT_PRIORITY);	// New items towards front.

								// If successful . . .
								if (pthing->m_phot != NULL)
									{
									// Make child of Hood's hotbox.
									pthing->m_phot->SetParent(ms_photHood);
									}
								else
									{
									TRACE("LoadRealm(): Unable to allocate hotbox for thing.\n");
									sResult	= 1;
									}
								}
								// Go to next item
								pList = pList->m_pnNext;
							}
						}
					else
						{
						TRACE("LoadRealm(): Unable to allocate hotbox for Hood.\n");
						sResult	= 1;
						}

					}
				else
					{
					TRACE("LoadRealm(): Realm has no hood.\n");
					sResult	= 1;
					}
				
				// If any errors occurred . . .
				if (sResult != 0)
					{
					// Clean up any partial stuff.
					CloseRealm(prealm);
					// Report.
					rspMsgBox(
						RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
						g_pszCriticalErrorTitle,
						g_pszGeneralError);
					}
				else
					{
					// Make sure our loaded settings are unaffected.
					short	sViewPosX	= 0;
					short	sViewPosY	= 0;
					if (ms_pgething != NULL)
						{
						sViewPosX	= ms_pgething->m_sViewPosX;
						sViewPosY	= ms_pgething->m_sViewPosY;
						}

					// Set camera's hood.
					ms_pcameraCur->SetHood(prealm->m_phood);

					// Update size affected stuff.
					SizeUpdate(ms_pcameraCur, prealm);

					// If there is an editor thing . . .
					if (ms_pgething != NULL)
						{
						// Update view position.
						ms_sbVert.SetPos(sViewPosY);
						ms_sbHorz.SetPos(sViewPosX);
						}
					
					// Freshen the map.
					RefreshMap(prealm);

					// Load the regions for the pylons.
					if (LoadTriggerRegions(ms_szFileName) < 0)
						{
						rspMsgBox(			// ****LOCALIZE****
							RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
							"LoadRealm()",
							"Pylon editor regions failed to load.");
						}

					// Make sure the bouys' show/hidden state reflects this flag's status.
					if (ms_bDrawNetwork == true)
						{
						CBouy::Show();
						}
					else
						{
						CBouy::Hide();
						}
					}
				}
			else
				{
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					g_pszCriticalErrorTitle,
					g_pszGeneralError);
				}
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Save realm as . . .
//
////////////////////////////////////////////////////////////////////////////////
static short SaveRealmAs(
	CRealm* prealm)
	{
	short sResult = 0;
	
	#ifdef DISABLE_EDITOR_SAVE_AND_PLAY
		rspMsgBox(
			RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
			"Postal Editor",
			"Sorry, but the save feature is disabled.");
		sResult = -1;
	#else
		// Attempt to get filename . . .
		// ***LOCALIZE***
		if (rspSaveBox(
			"Save Realm", 
			ms_szFileName, 
			ms_szFileName, 
			sizeof(ms_szFileName), 
			".rlm") == 0)
			{
			ASSERT(ms_szFileName[0] != '\0');
			// Save realm.
			// Note that SaveRealm() can call SaveRealmAs(), but should not in this
			// case (since we have a filename now).
			sResult = SaveRealm(prealm);
			}
		else
			{
			// Cancelled.
			sResult	= 1;
			}
	#endif

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Save realm
//
////////////////////////////////////////////////////////////////////////////////
static short SaveRealm(
	CRealm* prealm)
	{
	short sResult = 0;
	
	// If filename . . .
	if (strcmp(ms_szFileName, FullPathVD(INITIAL_REALM_DIR)) != 0)
		{
		// Save realm with trigger regions.
		sResult	= SaveRealm(prealm, ms_szFileName, true);
		}
	else
		{
		// Get filename before save.
		// Note that SaveRealmAs() calls this SaveRealm().
		sResult	= SaveRealmAs(prealm);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Save realm with specified name.
//
////////////////////////////////////////////////////////////////////////////////
static short SaveRealm(			// Returns 0 on success.
	CRealm* prealm,				// In:  Realm to save.
	char* pszRealmName,			// In:  Filename to save as.
	bool	bSaveTriggerRegions)	// In:  Save the trigger regions too.
	{
	short sResult = 0;

	#ifdef DISABLE_EDITOR_SAVE_AND_PLAY
		rspMsgBox(
			RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
			"Postal Editor",
			"Sorry, but the save feature is disabled.");
		sResult = -1;
	#else
		// Convert the Trigger Regions into an attribute map:
		CreateTriggerRegions(prealm);

		// Attach file counter callback to RFile and setup necessary components.
		InitFileCounter("Saving -- %ld bytes so far.");
		// Save realm
		sResult = prealm->Save(pszRealmName);
		// Clean and detach file counter.
		KillFileCounter();
		// If successful . . .
		if (sResult == 0)
			{
			// If we are to save the trigger regions . . .
			if (bSaveTriggerRegions == true)
				{
				// Save the regions for the pylons.
				if (SaveTriggerRegions(pszRealmName, prealm) < 0)
					{
					rspMsgBox(			// ****LOCALIZE****
						RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
						"SaveRealm()",
						"Pylon editor regions failed to save.");
					sResult	= -1;
					}
				}
			}
		else
			{
			rspMsgBox(
				RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
				g_pszCriticalErrorTitle,
				g_pszGeneralError);
			}
	#endif

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Editor's simulated play loop
//
////////////////////////////////////////////////////////////////////////////////
static void PlayRealm(
	CRealm*	pEditRealm,				// In:  Realm to play.
	CThing*	pthingSel)				// In:  Currently selected CThing which can
											// be used to give PlayRealm() a hint on which
											// of several things the user wants to use.
											// For example, a selected warp is the used
											// as the warp in point.
	{
	#ifdef DISABLE_EDITOR_SAVE_AND_PLAY
		rspMsgBox(
			RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
			"Postal Editor",
			"Sorry, but the play feature is disabled.");
	#else
		// This is not returned by the function, but used internally
		short sResult = 0;

		// Enable RMix's autopump.
		RMix::SetAutoPump(TRUE);

		rspClearAllInputEvents();

		// If mouse is being used, hide mouse cursor
		if (g_InputSettings.m_sUseMouse)
			rspHideMouseCursor();

		// Create a temporary filename
		char	szFileName[RSP_MAX_PATH];
		if (TmpFileName(szFileName, sizeof(szFileName)) == 0)
			{
			// Attach file counter callback to RFile and setup necessary components.
			InitFileCounter("Saving -- %ld bytes so far.");
			// Save realm being edited without the trigger regions.
			sResult	= SaveRealm(pEditRealm, szFileName, false);
			// Clean and detach file counter.
			KillFileCounter();
			// If successful . . .
			if (sResult == 0)
				{
				// Create a new realm so we don't accidentally rely on any side-effects
				// that may result from using the editor's realm.
				CRealm* prealm = new CRealm;
				ASSERT(prealm != NULL);

				// Setup progress callback right away.
//				prealm->m_fnProgress	= RealmOpProgress;

				// Clear realm (just in case)
				prealm->Clear();

				// Note that we are playing from the editor.  Might be useful.
				prealm->m_flags.bEditPlay		= true;
				prealm->m_flags.sDifficulty	= g_GameSettings.m_sDifficulty;

				// Reset time here so that objects can use it when they are loaded
				prealm->m_time.Reset();

				// No memorex here.
				SetInputMode(INPUT_MODE_LIVE);

				// Attach file counter callback to RFile and setup necessary components.
				InitFileCounter("Loading -- %ld bytes so far.");
			
				// Convert to RSPiX format b/c CRealm::Load() now likes it that way.
				char	szRealmName[sizeof(szFileName)];
				strcpy(szRealmName, rspPathFromSystem(szFileName) );

				// Load realm (false indicates NOT edit mode)
				sResult	= prealm->Load(szRealmName, false);
				// Clean and detach file counter.
				KillFileCounter();

				// If successful . . .
				if (sResult == 0)
					{
					// Set up the score module - initialize the font etc.
					ScoreInit();
					// Set the multiplayer scores to zero
					ScoreReset();
					// Reset the display timer
					ScoreResetDisplay();
					// Set score mode.
					ScoreSetMode(CScoreboard::SinglePlayer);



					// Startup the realm
					if (prealm->Startup() == 0)
						{
						U16	idSpecificWarp	= CIdBank::IdNil;

						//////////// Special behaviors based on the current selection ////////////
						// Note that pthingSel does not exist in prealm (it is in pEditRealm).
						// There is, however, a clone of it with the same ID in prealm.  These
						// special behaviors should, therefore, get the thing's ID here and use
						// it to get the ID of the specificied thing in the playing realm.

						// If there is a selection . . .
						if (pthingSel)
							{
							switch (pthingSel->GetClassID() )
								{
								case CThing::CWarpID:
									// Get user's preferred warp.
									idSpecificWarp	= pthingSel->GetInstanceID();
									break;
								}
							}

						//////////////////////////////////////////////////////////////////////////


						rspLockBuffer();

						// Erase screen.
						rspRect(
							RSP_BLACK_INDEX, 
							g_pimScreenBuf, 
							0,
							0,
							g_pimScreenBuf->m_sWidth,
							g_pimScreenBuf->m_sHeight);

						rspUnlockBuffer();

						// Could do rspUpdateDisplay() here to guarantee no palette flash
						// but that should not happen anyways b/c the current palette 
						// (from the realm's hood loaded in the editor) should be this
						// realm's hood's palette.

						// Set hood palette.
						prealm->m_phood->SetPalette();

						// Clear local input.
						ClearLocalInput();

						// We'll need access to the key status array.
						U8* pau8KeyStatus = rspGetKeyStatusArray();
						
						// Setup camera
						CCamera* pcamera = new CCamera;
						ASSERT(pcamera != NULL);
						pcamera->SetScene(&(prealm->m_scene));

						// Update display size sensitive objects.
						SizeUpdate(pcamera, prealm);

						pcamera->SetFilm(g_pimScreenBuf, 0, 0);
						pcamera->SetHood(prealm->m_phood);

						ms_pcameraCur	= pcamera;

						ScrollPosUpdate(&ms_sbVert);
						ScrollPosUpdate(&ms_sbHorz);

						// Set grip to control camera
						CGrip grip;
						grip.SetParms(100, 1, 1, 8, 8, 1, 1, true);
						grip.SetCamera(pcamera);
						grip.ResetTarget(0, 0, 30);

						// Default to tracking the track ID.
						bool	bTracking	= true;

						// Get thing to track . . .
						CThing*	pthingTrack	= NULL;
						U16		u16IdTrack	= CIdBank::IdNil;
						if (ms_pgething != NULL)
							{
							u16IdTrack = ms_pgething->m_u16CameraTrackId;
							}

						// Make sure no Scrollbars have focus.
						RGuiItem::SetFocus(NULL);

						CListNode<CThing>* pNext = prealm->m_aclassHeads[CThing::CDudeID].m_pnNext;
						U16 u16IdDude = CIdBank::IdNil;
						while (pNext->m_powner != NULL)
						{
							CDude*	pdude = (CDude*) pNext->m_powner;
							// if this is the local dude...
							if (pdude->m_sDudeNum == 0)
							{
								// Store 'im
								u16IdDude = pdude->GetInstanceID();
								// Make him X-Rayable.
								pdude->m_sprite.m_sInFlags |= CSprite::InXrayee;
							}
							pNext = pNext->m_pnNext;
						}

						// If no dude yet . . .
						if (u16IdDude == CIdBank::IdNil)
							{
							// Create one using warps (if any):
							CDude*	pdude	= NULL;
							CWarp*	pwarp	= NULL;
							// If there's a specific warp desired . . .
							if (prealm->m_idbank.GetThingByID( (CThing**)&pwarp, idSpecificWarp) == 0)
								{
								// Use the specific warp to create the dude . . .
								if (pwarp->WarpIn(		// Returns 0 on success.                                 
									&pdude,					// In:  CDude to 'warp in', *ppdude = NULL to create one.
																// Out: Newly created CDude, if no CDude passed in.      
									CWarp::None) == 0)	// In:  Options for 'warp in'.
									{
									// Success.
									}
								else
									{
									TRACE("PlayRealm(): Failed to use user specified warp.\n");
									}
								}

							// If no dude yet . . .
							if (pdude == NULL)
								{
								// Warp one in anywhere . . .
								if (CWarp::WarpInAnywhere(	// Returns 0 on success.                                 
									prealm,						// In:  Realm in which to choose CWarp.                  
									&pdude,						// In:  CDude to 'warp in', *ppdude = NULL to create one.
																	// Out: Newly created CDude, if no CDude passed in.      
									CWarp::None) == 0)		// In:  Options for 'warp in'.                           
									{
									// Success.
									}
								}

							// If a dude was created . . .
							if (pdude)
								{
								// Store 'im
								u16IdDude = pdude->GetInstanceID();
								// Make him X-Rayable.
								pdude->m_sprite.m_sInFlags |= CSprite::InXrayee;
								// If nothing to track yet . . .
								if (u16IdTrack == CIdBank::IdNil)
									{
									// Use the local dude.
									u16IdTrack = u16IdDude;
									}
								}
							}

						CDude*	pdudeLocal	= NULL;
						if (prealm->m_idbank.GetThingByID((CThing**)&pdudeLocal, u16IdDude) == 0)
							{
							pdudeLocal->m_sTextureIndex = MAX((short)0, MIN((short)(CDude::MaxTextures - 1), g_GameSettings.m_sPlayerColorIndex));

							// Don't use later.
							pdudeLocal	= NULL;
							}

						RInputEvent	ie;
						// Setup rectangular area for dude's status.
						RRect	rcDudeStatus(
							DUDE_STATUS_RECT_X, 
							DUDE_STATUS_RECT_Y, 
							DUDE_STATUS_RECT_W, 
							DUDE_STATUS_RECT_H);
						// Setup rectangular area for realm's status.
						RRect rcRealmStatus(
							REALM_STATUS_RECT_X, 
							REALM_STATUS_RECT_Y, 
							REALM_STATUS_RECT_W, 
							REALM_STATUS_RECT_H);
						// Setup rectangular area for display info.
						RRect	rcInfoStatus(
							INFO_STATUS_RECT_X,
							INFO_STATUS_RECT_Y,
							INFO_STATUS_RECT_W,
							INFO_STATUS_RECT_H);

						long	lLastDispTime			= 0;
						long	lFramesTime				= 0;
						long	lUpdateDisplayTime	= 0;
						long	lNumFrames				= 0;

						RPrint	printDisp;
						printDisp.SetFont(DISP_INFO_FONT_H, &g_fontBig);
						printDisp.SetColor(250, 0, 0);
						printDisp.SetDestination(g_pimScreenBuf);

						char	szFileDescriptor[512];
						Play_GetApplicationDescriptor(szFileDescriptor, sizeof(szFileDescriptor) );

						// Reset time again so that the first time update doesn't show (much) elapsed time
						prealm->m_time.Reset();

						bool	bDone				= false;
						bool	bExitRequest	= false;
						bool	bSuspended		= false;

						// Do the loop
						while (bDone == false)
							{
							// Update the realm's game time
							prealm->m_time.Update();

							// System update
							UpdateSystem();

							ie.type	= RInputEvent::None;
							rspGetNextInputEvent(&ie);
							if (ie.type == RInputEvent::Key)
								{
								// Force alpha keys to upper keys
								if (isalpha(ie.lKey & 0xffff))
									ie.lKey = (ie.lKey & 0xffff0000) | toupper(ie.lKey & 0xffff);

								switch (ie.lKey)
									{
									case EDIT_KEY_PAUSE:
										if (bSuspended == false)
											{
											prealm->Suspend();
											}
										else
											{
											prealm->Resume();
											}
										bSuspended	= !bSuspended;
										break;
		#if 0
									case EDIT_KEY_SPEED_UP:
										break;
									case EDIT_KEY_SPEED_DOWN:
										break;
									case EDIT_KEY_SPEED_NORMAL:
										break;
		#endif

									case EDIT_KEY_ENDPLAY:
										// If first request . . .
										if (bExitRequest == false)
											{
											bExitRequest	= true;
											}
										else
											{
											bDone	= true;
											}
										break;

									case EDIT_KEY_ENLARGE_DISPLAY1:
									case EDIT_KEY_ENLARGE_DISPLAY2:
									case EDIT_KEY_ENLARGE_DISPLAY3:
										OnEnlargeDisplay(pcamera, prealm);
										// Update status areas.  That is, repaginate now.
										rcDudeStatus.sX	= DUDE_STATUS_RECT_X;
										rcDudeStatus.sY	= DUDE_STATUS_RECT_Y;
										rcDudeStatus.sW	= DUDE_STATUS_RECT_W;
										rcDudeStatus.sH	= DUDE_STATUS_RECT_H;
										rcRealmStatus.sX	= REALM_STATUS_RECT_X;
										rcRealmStatus.sY	= REALM_STATUS_RECT_Y;
										rcRealmStatus.sW	= REALM_STATUS_RECT_W;
										rcRealmStatus.sH	= REALM_STATUS_RECT_H;
										rcInfoStatus.sX	= INFO_STATUS_RECT_X;
										rcInfoStatus.sY	= INFO_STATUS_RECT_Y;
										rcInfoStatus.sW	= INFO_STATUS_RECT_W;
										rcInfoStatus.sH	= INFO_STATUS_RECT_H;
										break;

									case EDIT_KEY_REDUCE_DISPLAY1:
									case EDIT_KEY_REDUCE_DISPLAY2:
										OnReduceDisplay(pcamera, prealm);
										// Update status areas.  That is, repaginate now.
										rcDudeStatus.sX	= DUDE_STATUS_RECT_X;
										rcDudeStatus.sY	= DUDE_STATUS_RECT_Y;
										rcDudeStatus.sW	= DUDE_STATUS_RECT_W;
										rcDudeStatus.sH	= DUDE_STATUS_RECT_H;
										rcRealmStatus.sX	= REALM_STATUS_RECT_X;
										rcRealmStatus.sY	= REALM_STATUS_RECT_Y;
										rcRealmStatus.sW	= REALM_STATUS_RECT_W;
										rcRealmStatus.sH	= REALM_STATUS_RECT_H;
										rcInfoStatus.sX	= INFO_STATUS_RECT_X;
										rcInfoStatus.sY	= INFO_STATUS_RECT_Y;
										rcInfoStatus.sW	= INFO_STATUS_RECT_W;
										rcInfoStatus.sH	= INFO_STATUS_RECT_H;
										break;

									case EDIT_KEY_CAMERA_TRACKING:
										bTracking	= !bTracking;

										// If we are now using grip . . .
										if (bTracking == true)
											{
											// Erase scrollbars.
											rspRect(
												RSP_BLACK_INDEX, 
												g_pimScreenBuf, 
												ms_sbVert.m_sX,
												ms_sbVert.m_sY,
												ms_sbVert.m_im.m_sWidth,
												ms_sbVert.m_im.m_sHeight);
											rspRect(
												RSP_BLACK_INDEX, 
												g_pimScreenBuf, 
												ms_sbHorz.m_sX,
												ms_sbHorz.m_sY,
												ms_sbHorz.m_im.m_sWidth,
												ms_sbHorz.m_im.m_sHeight);

											// Reset the grip, if necessary.
											if (pthingTrack != NULL && bTracking == true)
												{
												RRect	rc;
												pthingTrack->EditRect(&rc);
												short	sHotX, sHotY;
												pthingTrack->EditHotSpot(&sHotX, &sHotY);

												grip.ResetTarget(rc.sX + sHotX, rc.sY + sHotY, rc.sH / 2);

												// Make sure no Scrollbars have focus.
												RGuiItem::SetFocus(NULL);
												}
											}
										else	// We are now using scrollbars.
											{
											// Update scroll pos via camera pos.
											ms_sbVert.SetPos(ms_pcameraCur->m_sSceneViewY);
											ms_sbHorz.SetPos(ms_pcameraCur->m_sSceneViewX);
											}
										break;

									// Change priority level.
									case '1':
									case '2':
									case '3':
		//								rspSetDoSystemMode(ie.lKey - '1');
										break;

									case EDIT_KEY_TOGGLE_DISP_INFO:
										// Toggle display info flag.
										if (g_GameSettings.m_sDisplayInfo == FALSE)
											{
											g_GameSettings.m_sDisplayInfo	= TRUE;
											}
										else
											{
											g_GameSettings.m_sDisplayInfo	= FALSE;
											}

										// Repaginate now.
										rcDudeStatus.sH	= DUDE_STATUS_RECT_H;
										break;

									case EDIT_KEY_SHOW_MISSION:
										// Show the mission goal line again for about 5 seconds.
										ScoreDisplayStatus(prealm);
										break;

									case EDIT_KEY_REALM_STATISTICS:
										
										prealm->Suspend();

										ShowRealmStatistics(prealm, NULL);

										prealm->Resume();

										break;
									}
								}

							// If xray all pressed . . .
							if (pau8KeyStatus[KEY_XRAY_ALL] & 1)
								{
								prealm->m_scene.SetXRayAll(TRUE);
								}
							else
								{
								prealm->m_scene.SetXRayAll(FALSE);
								}

							// Lock the composite buffer for access.
							rspLockBuffer();

							// Process input for extra camera GUIs.
							DoViews(&ie);

							// Only do scrollbars if not tracking . . .
							if (bTracking == false)
								{
								// Update hots.
								ms_sbVert.m_hot.Do(&ie);
								ms_sbHorz.m_hot.Do(&ie);
								}

							// Do GUI input focus stuff.
							RGuiItem::DoFocus(&ie);

							// Get local player input and
							// Set controls for the one-and-only CDude
							// Allow cheats.
							SetInput(0, GetLocalInput(prealm, &ie));

							// If exit requested . . .
							if (bExitRequest == true)
								{
								CDude*	pdudeLocal;
								// If there's a local dude . . .
								if (prealm->m_idbank.GetThingByID((CThing**)&pdudeLocal, u16IdDude) == 0)
									{
									// If dead . . .
									if (pdudeLocal->m_state == CCharacter::State_Dead)
										{
										// Okay, done.
										bDone	= true;
										}
									else
										{
										// Commit suicide.
										GameMessage	msg;
										msg.msg_Generic.eType		= typeSuicide;
										msg.msg_Generic.sPriority	= 0;
										pdudeLocal->SendThingMessage(&msg, pdudeLocal);
										}
									}
								else
									{
									bDone	= true;
									}
								}

							// Update and render realm
							prealm->Update();
							prealm->Render();

							if (u16IdTrack != CIdBank::IdNil)
								{
								if (prealm->m_idbank.GetThingByID(&pthingTrack, u16IdTrack) != 0)
									{
									u16IdTrack	= CIdBank::IdNil;
									}
								}

							// Update grip/camera
							if (pthingTrack != NULL)
								{
								RRect	rc;
								pthingTrack->EditRect(&rc);
								short	sHotX, sHotY;
								pthingTrack->EditHotSpot(&sHotX, &sHotY);

								sHotX	+= rc.sX;
								sHotY	+= rc.sY;

								short	sRealmX, sRealmY, sRealmZ;
								// Convert to realm.
								MapScreen2Realm(
									prealm,									// In:  Realm.
									pcamera,									// In:  View of prealm.
									sHotX - pcamera->m_sScene2FilmX,	// In:  Screen x coord.
									sHotY - pcamera->m_sScene2FilmY,	// In:  Screen y coord.
									&sRealmX,								// Out: Realm x coord.
									&sRealmY,								// Out: Realm y coord (always via realm's height map).
									&sRealmZ);								// Out: Realm z coord.

								// Set the location of our ear.
								SetSoundLocation(sRealmX, sRealmY, sRealmZ);

								if (bTracking == true)
									{
									grip.TrackTarget(sHotX, sHotY, rc.sH / 2);
									}
								}

							// If quitting . . .
							if (rspGetQuitStatus() != FALSE)
								{
								bDone	= true;
								}

							// Snap picture of scene
							pcamera->Snap();

							// If there is a local dude . . .
							CDude*	pdudeLocal	= NULL;
							// If there's a local dude, get him.
							prealm->m_idbank.GetThingByID((CThing**)&pdudeLocal, u16IdDude);

							// Only do scrollbars if not tracking . . .
							if (bTracking == false)
								{
								// Update GUIs to composite buffer.
								ms_sbVert.Draw(g_pimScreenBuf);
								ms_sbHorz.Draw(g_pimScreenBuf);
								}

							// Draw extra cameras.
							DrawViews(prealm);

							// Done with the composite buffer (except to update it to
							// the screen).
							rspUnlockBuffer();

// Until the editor is merged into the new play, just do it this way.  The only "loss" is that
// we don't get the various status stuff being displayed while in the editor.
#if 0
							Play_UpdateDisplays(						// Returns nothing.
								prealm,									// In:  Realm.
								pcamera,									// In:  Camera.
								pdudeLocal,								// In:  Local dude.
								&rcDudeStatus,							// In:  Rect for dude status display.
								&rcRealmStatus,						// In:  Rect for realm status display.
								&rcInfoStatus,							// In:  Rect for info status display.
								&printDisp,								// In:  Print to output display info.
								szFileDescriptor,						// In:  Date/Time stamp of exe.
								&lLastDispTime,						// In/Out:  Last time display info was output.
								&lFramesTime,							// In/Out:  Total time of all frames since last display info output.
								&lUpdateDisplayTime,					// In/Out:  Total time of all rspUpdateDisplay()s since last display info output.
								&lNumFrames,							// In/Out:  Number of frames since last display info output.
								0,											// In:  Current input sequence number.
								0,											// In:  Current frame number.
								NULL,										// In:  pointer to net client to get player names for Score
								TRUE);									// In:  Update entire display.
#else
							rspUpdateDisplay();
#endif

							// Lock buffer for clearage.
							rspLockBuffer();

							// Clear extra cameras.
							ClearViews();

							rspUnlockBuffer();

							// If snap key is pressed . . .
							if (pau8KeyStatus[KEY_SNAP_PICTURE])
								{
								// Take the snap shot.
								Play_SnapPicture();

								// Clear key status.
								pau8KeyStatus[KEY_SNAP_PICTURE]	= 0;
								}
							}

						// Shutdown realm
						prealm->Shutdown();

						// Done with the camera.
						delete pcamera;
						pcamera			= NULL;
						ms_pcameraCur	= NULL;
						}
					else
						{
						TRACE("EditPlay(): Error starting-up temporary realm!\n");
						sResult = -1;
						}
					}
				else
					{
					TRACE("EditPlay(): Error loading temporary realm!\n");
					}

				// Delete the realm
				delete prealm;

				// Delete temporary file
				remove(szFileName);
				}
			else
				{
				TRACE("EditPlay(): Error saving temporary realm!\n");
				}
			}
		else
			{
			TRACE("PlayRealm(): Error getting temporary file name!\n");
			sResult = -1;
			}

		// If mouse is being used, restore mouse cursor
		if (g_InputSettings.m_sUseMouse)
			rspShowMouseCursor();

		rspClearAllInputEvents();

		// Disable autopump.
		RMix::SetAutoPump(FALSE);

		// If something went wrong, let the user know
		if (sResult)
			{
			rspMsgBox(
				RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
				"Postal Editor",
				"An error has occurred that prevents this realm from being played.");
			}

	#endif // DISABLE_EDITOR_SAVE_AND_PLAY
	}

////////////////////////////////////////////////////////////////////////////////
//
// Create a new CThing derived object of type id in prealm at the specified
// position.
//
////////////////////////////////////////////////////////////////////////////////
static short CreateNewThing(		// Returns 0 on success.
	CRealm*	prealm,					// In:  Realm to add new CThing to.
	CThing::ClassIDType	id,		// ID of new CThing type to create.
	short		sPosX,					// Position for new CThing.
	short		sPosY,					// Position for new CThing.
	short		sPosZ,					// Position for new CThing.
	CThing**	ppthing,					// Out: Pointer to new thing.
	RHot**	pphot,					// Out: Pointer to new hotbox for thing.
	RFile*	pfile/* = NULL*/)		// In:  Optional file to load from (instead of EditNew()).
	{
	short		sError		= 0;

	// Don't allow more than one CHood . . .
	if ((id == CThing::CHoodID) && (prealm->m_asClassNumThings[CThing::CHoodID] > 0))
		{
		// ***LOCALIZE***
		rspMsgBox(
			RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
			"Editor: ",
			"Can't have multiple CHood's!");
		//STRACE("Editor: Can't have multiple CHood's!\n");
		sError	= 1;
		}
	else
		{
		if (prealm->m_asClassNumThings[CThing::CHoodID] > 0 || id == CThing::CHoodID)
			{
			if (!(id == CThing::CBouyID && prealm->GetCurrentNavNet() == NULL))
				{
				// Create new object of currently selected type
				if (CThing::ConstructWithID(id, prealm, ppthing) == 0)
					{
					// Successfully allocated object.

					// If loading from file specified . . .
					if (pfile != NULL)
						{
						// Remember its ID.
						U16	idInstance	= (*ppthing)->GetInstanceID();
						// Release its ID.
						(*ppthing)->SetInstanceID(CIdBank::IdNil);

						// Load object . . .
						if ((*ppthing)->Load(pfile, true, ms_sFileCount--, CRealm::FileVersion) == 0)
							{
							// Loaded.
							// Reset ID.
							(*ppthing)->SetInstanceID(idInstance);
							// Reserve ID.
							prealm->m_idbank.Take(*ppthing, idInstance);
							// Startup.
							(*ppthing)->Startup();
							// Move.
							(*ppthing)->EditMove(sPosX, sPosY, sPosZ);
							}
						else
							{
							TRACE("CreateNewThing(): Load() failed for object.\n");
							sError	= 4;
							}
						}
					else
						{
						// Edit new object (required to get object up and running)
						if ((*ppthing)->EditNew(sPosX, sPosY, sPosZ) == 0)
							{
							// Newed.
							}
						else
							{
							TRACE("CreateNewThing(): EditNew() failed for object.\n");
							sError	= 4;
							}
						}

					// If successful so far . . .
					if (sError == 0)
						{
						short sActivateHot	= TRUE;

						// Some types may need to hook in here.
						switch ( (*ppthing)->GetClassID() )
							{
							case CThing::CBouyID:
								// If bouy lines are hidden . . .
								if (ms_bDrawNetwork == false)
									{
									// Don't activate new bouy hots.
									sActivateHot	= FALSE;
									}
								break;
							}

						// Get pos and dimensions for hot.
						RRect	rc;
						(*ppthing)->EditRect(&rc);

						// Allocate a RHot for the item . . .
						(*ppthing)->m_phot	= *pphot	= new RHot(
							rc.sX,							// Position.
							rc.sY,							// Position.
							rc.sW,							// Dimensions.
							rc.sH,							// Dimensions.
							ThingHotCall,					// Callback.
							sActivateHot,					// TRUE, if initially active.
							(U32)*ppthing,					// User value (CThing*).
							FRONTMOST_HOT_PRIORITY);	// New items towards front.

						if (*pphot != NULL)
							{
							// If this is not THE HOOD . . .
							if (id != CThing::CHoodID)
								{
								(*pphot)->SetParent(ms_photHood);
								}

							// Special things.
							switch (id)
								{
								case CThing::CDudeID:
									// If there is an editor thing . . .
									if (ms_pgething != NULL)
										{
										// If no camera focus yet . . .
										if (ms_pgething->m_u16CameraTrackId == CIdBank::IdNil)
											{
											// Track this dude.
											ms_pgething->m_u16CameraTrackId	= (*ppthing)->GetInstanceID();
											}
										}

									
									CDude*	pdude	= (CDude*)(*ppthing);
									// If this is the local dude . . .
									if (pdude->m_sDudeNum == 0)
										{
										// Set him to the user selected color.
										pdude->m_sTextureIndex = MAX((short)0, MIN((short)(CDude::MaxTextures - 1), g_GameSettings.m_sPlayerColorIndex));
										}

									break;
								}

							// If an error occurred after allocation . . .
							if (sError != 0)
								{
								// On error, destroy object
								delete *pphot;
								*pphot	= NULL;
								}
							}
						else
							{
							TRACE("CreateNewThing(): Failed to allocate new RHot.\n");
							sError	= 3;
							}
						}

					// If an error occurred after allocation . . .
					if (sError != 0)
						{
						// On error, destroy object
						delete *ppthing;
						*ppthing	= NULL;
						}
					}
				}
			else
				{
				TRACE("CreateNewThing(): Cannot create a buoy when there's no current NavNet.\n");
				// ***LOCALIZE***
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					"No current NavNet",
					"Cannot create new Buoy when there is no current NavNet.");
				sError	= 6;
				}
			}
		else
			{
			sError = 7;
			}
		}
	return sError;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Move a thing to the specified location and update its RHot with an
// EditRect() call.
//
////////////////////////////////////////////////////////////////////////////////
static void MoveThing(				// Returns nothing.
	CThing*	pthing,					// Thing to move.
	RHot*		phot,						// Thing's hotbox.
	short		sPosX,					// New position.
	short		sPosY,					// New position.
	short		sPosZ)					// New position.
	{
	ASSERT(pthing != NULL);
	ASSERT(phot != NULL);

	// Move to 3D position.
	pthing->EditMove(
		sPosX,
		sPosY,
		sPosZ);

	// Get 2D position.
	RRect	rc;
	pthing->EditRect(
		&rc);

	// Copy to hot.
	phot->m_sX	= rc.sX;
	phot->m_sY	= rc.sY;
	phot->m_sW	= rc.sW;
	phot->m_sH	= rc.sH;

	// Update info GUI.
	UpdateSelectionInfo(false);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Enlarges the display area.
//
////////////////////////////////////////////////////////////////////////////////
static void OnEnlargeDisplay(
	CCamera* pcamera,					// Camera to update.
	CRealm*	prealm)					// Realm to update.
	{
	AdjustDisplaySize(DISPLAY_SIZE_DELTA_X, DISPLAY_SIZE_DELTA_Y, pcamera, prealm);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Reduces the display area.
//
////////////////////////////////////////////////////////////////////////////////
static void OnReduceDisplay(
	CCamera* pcamera,					// Camera to update.
	CRealm*	prealm)					// Realm to update.
	{
	AdjustDisplaySize(-DISPLAY_SIZE_DELTA_X, -DISPLAY_SIZE_DELTA_Y, pcamera, prealm);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Set display mode such that display area is as
// specified.
//
////////////////////////////////////////////////////////////////////////////////
static short SetDisplayArea(	// Returns 0 on success.
	short	sDeviceD,				// New depth of display.
	short	sDisplayW,				// New width of display area.
	short	sDisplayH)				// New height of display area.
	{
	short	sError	= 0;
	short	sDeviceW, sDeviceH;
	short	sDeviceScaling	= FALSE;
	// Get device size/mode for this display size.
	sError = rspSuggestVideoMode(
		sDeviceD,
		sDisplayW,
		sDisplayH,
		1,
		sDeviceScaling,
		&sDeviceW,
		&sDeviceH,
		&sDeviceScaling);

	// If we are not supposed to change the device dimensions . . .
	if (g_GameSettings.m_sUseCurrentDeviceDimensions != FALSE)
		{
		// Get current device settings.
		rspGetVideoMode(
			&sDeviceD,
			&sDeviceW,
			&sDeviceH);
		// Limit display area.
		sDisplayW	= MIN(sDeviceW, sDisplayW);
		sDisplayH	= MIN(sDeviceH, sDisplayH);
		}

	// If successful so far . . .
	if (sError == 0)
		{
		// Set the adjusted mode . . .
		sError	= rspSetVideoMode(
			sDeviceD,
			sDeviceW,
			sDeviceH,
			sDisplayW,
			sDisplayH,
			1,
			sDeviceScaling);

		// Give Jeff a chance to update his cool wrapper.
		// I'm not sure if this is necessary, but let's be safe.
		rspNameBuffers(&g_pimScreenBuf);

		if (sError == 0)
			{
			}
		else
			{
			TRACE("SetDisplayArea(): rspSetVideoMode() failed.\n");
			}
		}
	else
		{
		TRACE("SetDisplayArea(): rspSuggestVideoMode() failed.\n");
		}

	// Store latest values (in success or failure).
	g_GameSettings.m_sEditorViewWidth	= g_pimScreenBuf->m_sWidth;
	g_GameSettings.m_sEditorViewHeight	= g_pimScreenBuf->m_sHeight;

	return sError;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Set display mode and display area such that camera view
// is specified size.
//
////////////////////////////////////////////////////////////////////////////////
static short SetCameraArea(void)	// Returns 0 on success.
	{
	short	sDepth	= 8;	// Safety.
	rspGetVideoMode(&sDepth);
	
	// Determine display area for this camera size.
	short	sDisplayW	= g_GameSettings.m_sEditorViewWidth;//sCameraW + DISPLAY_RIGHT_BORDER;
	short	sDisplayH	= g_GameSettings.m_sEditorViewHeight;//sCameraH + DISPLAY_BOTTOM_BORDER;

	// Set the display area/mode.
	return SetDisplayArea(sDepth, sDisplayW, sDisplayH);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Adjust the display area by the specified deltas.
//
////////////////////////////////////////////////////////////////////////////////
static short AdjustDisplaySize(	// Returns 0 on success.
	short	sAdjustX,					// Amount to increase width of display area.
											// Can be negative to decrease.
	short	sAdjustY,					// Amount to increase height of display area.
											// Can be negative to decrease.
	CCamera* pcamera,					// Camera to update.
	CRealm*	prealm)					// Realm to update.
	{
	short	sError		= 0;

	short	sDisplayW	= 640;	// Safety.
	short	sDisplayH	= 480;	// Safety.
	short	sDeviceD		= 8;		// Safety.

	// Get current settings.
	rspGetVideoMode(
		&sDeviceD,
		NULL,
		NULL,
		NULL,
		&sDisplayW,
		&sDisplayH);

	// New area.
	if (SetDisplayArea(
		sDeviceD,
		sDisplayW	+ sAdjustX,
		sDisplayH	+ sAdjustY) == 0)
		{
		SizeUpdate(pcamera, prealm);
		}
	else
		{
		sError	= 1;
		}

	return sError;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Update screen size sensitive objects.
//
////////////////////////////////////////////////////////////////////////////////
static short SizeUpdate(		// Returns 0 on success.
	CCamera* pcamera,				// Camera to update.
	CRealm*	prealm)				// Realm to update.
	{
	short	sRes	= 0;	// Assume success.

	short	sDisplayW	= 640;	// Safety.
	short	sDisplayH	= 480;	// Safety.
	short	sDisplayD	= 8;		// Safety.

	// Get current settings.
	rspGetVideoMode(
		&sDisplayD,
		NULL,
		NULL,
		NULL,
		&sDisplayW,
		&sDisplayH);

	short	sViewW	= sDisplayW - DISPLAY_RIGHT_BORDER - SCROLL_BAR_THICKNESS;
	short	sViewH	= sDisplayH - DISPLAY_BOTTOM_BORDER - SCROLL_BAR_THICKNESS;

	// Get the hood . . .
	CHood*	phood = NULL;
	if (prealm->m_asClassNumThings[CThing::CHoodID] > 0)
		phood = (CHood*) prealm->m_aclassHeads[CThing::CHoodID].GetNext();
	else
		{
		TRACE("SizeUpdate(): No hood.\n");
		sRes	= -1;
		}

	// Give Jeff a chance to update his cool wrapper.
	// I'm not sure if this is necessary, but let's be safe.
	rspNameBuffers(&g_pimScreenBuf);

	// If there's a camera to update . . .
	if (pcamera != NULL)
		{
		pcamera->SetHood(phood);
		// Adjust camera by same amount.
		pcamera->SetView(
			pcamera->m_sSceneViewX, 
			pcamera->m_sSceneViewY,
			sViewW,
			sViewH);

		pcamera->SetFilm(g_pimScreenBuf, 0, 0);
		}

	// Put GUIs somewhere out of the way.
	if (ms_pguiRealmBar != NULL)
		{
		ms_pguiRealmBar->Move(REALM_BAR_X, REALM_BAR_Y);
		}
	if (ms_pguiPickObj != NULL)
		{
		ms_pguiPickObj->Move(PICKOBJ_LIST_X, PICKOBJ_LIST_Y);
		}
	if (ms_pguiLayers != NULL)
		{
		ms_pguiLayers->Move(LAYERS_LIST_X, LAYERS_LIST_Y);
		}
	if (ms_pguiCameras != NULL)
		{
		ms_pguiCameras->Move(CAMERAS_LIST_X, CAMERAS_LIST_Y);
		}
	if (ms_pguiGUIs != NULL)
		{
		ms_pguiGUIs->Move(GUIS_BAR_X, GUIS_BAR_Y);
		}
	if (ms_pguiNavNets != NULL)
		{
		ms_pguiNavNets->Move(NAVNETS_X, NAVNETS_Y);
		}
	if (ms_pguiMap != NULL)
		{
		ms_pguiMap->Move(MAP_X, MAP_Y);
		}
	if (ms_pguiShowAttribs)
		{
		ms_pguiShowAttribs->Move(SHOWATTRIBS_X, SHOWATTRIBS_Y);
		}
	if (ms_pguiInfo)
		{
		ms_pguiInfo->Move(INFO_X, INFO_Y);
		}

	long	lEdgeOvershoot;
	// If not clipping to realm . . .
	if (pcamera->m_bClip == false)
		{
		lEdgeOvershoot	= ms_lEdgeOvershoot;
		}
	else
		{
		lEdgeOvershoot	= 0;
		}

	// (Re)Create scrollbars at appropriate sizes . . .
	if (ms_sbVert.Create(
		sViewW,
		0,
		SCROLL_BAR_THICKNESS,
		sViewH + SCROLL_BAR_THICKNESS,
		sDisplayD) == 0)
		{
		// Set scroll range.
		if (phood != NULL)
			{
			ms_sbVert.SetRange(-lEdgeOvershoot, phood->GetHeight() - sViewH + lEdgeOvershoot);
			}
		else
			{
			ms_sbVert.SetRange(0, 0);
			}

		ms_sbVert.m_lTrayIncDec		= sViewH - SCROLL_BTN_INCDEC;
		ms_sbVert.m_lButtonIncDec	= SCROLL_BTN_INCDEC;
		ms_sbVert.m_lPosPerSecond	= sViewH * 2;
		}
	else
		{
		TRACE("SizeUpdate(): ms_sbVert.Create() failed.\n");
		sRes	= -3;
		}

	if (ms_sbHorz.Create(
		0,
		sViewH,
		sViewW,
		SCROLL_BAR_THICKNESS,
		sDisplayD) == 0)
		{
		// Set scroll range.
		if (phood != NULL)
			{
			ms_sbHorz.SetRange(-lEdgeOvershoot, phood->GetWidth() - sViewW + lEdgeOvershoot);
			}
		else
			{
			ms_sbHorz.SetRange(0, 0);
			}

		ms_sbHorz.m_lTrayIncDec		= sViewW - SCROLL_BTN_INCDEC;
		ms_sbHorz.m_lButtonIncDec	= SCROLL_BTN_INCDEC;
		ms_sbHorz.m_lPosPerSecond	= sViewW * 2;
		}
	else
		{
		TRACE("SizeUpdate(): ms_sbHorz.Create() failed.\n");
		sRes	= -3;
		}

	// Re-Create attribute displayer sprite.
	SizeShowAttribsSprite();

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Get the Editor Thing from the specified realm.
//
////////////////////////////////////////////////////////////////////////////////
static CGameEditThing* GetEditorThing(	// Returns ptr to editor thing for 
													// specified realm or NULL.
	CRealm*	prealm)							// Realm to get editor thing from.
	{
	CGameEditThing*	pgething	= NULL;

	if (prealm->m_asClassNumThings[CThing::CGameEditThingID] > 0)
	{
		pgething = (CGameEditThing*) prealm->m_aclassHeads[CThing::CGameEditThingID].GetNext();
	}
	else
	{
		TRACE("GetEditorThing(): No editor thing.\n");
	}

	return pgething;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from GUIs.  This will set ms_lPressedId to pgui->m_lId.
//
////////////////////////////////////////////////////////////////////////////////
static void GuiPressedCall(	// Returns nothing.
	RGuiItem*	pgui)				// GUI item pressed.
	{
	// Set ID of item pressed.
	ms_lPressedId	= pgui->m_lId;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from pressed list items.
//
////////////////////////////////////////////////////////////////////////////////
static void ListItemPressedCall(	// Returns nothing.
	RGuiItem*	pgui)					// GUI item pressed.
	{
	// If not dragging . . .
	if (ms_sMoving == FALSE)
		{
		// Default logic.
		GuiPressedCall(pgui);
		// Select item.
		RListBox*	plb	= (RListBox*)pgui->m_ulUserInstance;
		if (plb != NULL)
			{
			// Set new item as selection.
			plb->SetSel(pgui);
			}
		else
			{
			TRACE("ListItemPressedCall(): ListItem has no parent? That would be wrong.\n");
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from Nav Net listbox
//
////////////////////////////////////////////////////////////////////////////////
void NavNetListPressedCall(	// Returns nothing
	RGuiItem* pgui)						// GUI item pressed
	{
	// Default logic
//	GuiPressedCall(pgui);
	// Select item
	RGuiItem* pitem = (RGuiItem*) pgui->GetParent();
	RListBox* plb = (RListBox*) pitem->GetParent();
	if (plb != NULL)
		{
		// Set selection
		plb->SetSel(pgui);
		// Set as default Nav Net
		((CNavigationNet*) pgui->m_ulUserInstance)->SetAsDefault();
		// Make the net lines redraw
		UpdateNetLines((CNavigationNet*) pgui->m_ulUserInstance);
		}
	else
		{
		TRACE("NavNetListPressedCall(): List Item has no parent?\n");
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from scrollbars indicating change in thumb position.
//
////////////////////////////////////////////////////////////////////////////////
static void ScrollPosUpdate(	// Returns nothing.
	RScrollBar* psb)				// ScrollBar that was updated.
	{
	// Switch on orientation. Say....
	switch (psb->m_oOrientation)
		{
		case RScrollBar::Vertical:
			// If there's a camera . . .
			if (ms_pcameraCur != NULL)
				{
				ms_pcameraCur->m_sSceneViewY	= psb->GetPos();
				ms_pcameraCur->Update();
				}

			// If there is an editor object . . .
			if (ms_pgething != NULL)
				{
				ms_pgething->m_sViewPosY	= psb->GetPos();
				}
			break;
		case RScrollBar::Horizontal:
			// If there's a camera . . .
			if (ms_pcameraCur != NULL)
				{
				ms_pcameraCur->m_sSceneViewX	= psb->GetPos();
				ms_pcameraCur->Update();
				}

			// If there is an editor object . . .
			if (ms_pgething != NULL)
				{
				ms_pgething->m_sViewPosX	= psb->GetPos();
				}
			break;
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from RHot when an event occurs within it.
//
////////////////////////////////////////////////////////////////////////////////
static void ThingHotCall(	// Returns nothing.
	RHot*	phot,					// Ptr to RHot that generated event.
	RInputEvent*	pie)		// In:  Most recent user input event.
									// Out: Depends on callbacks.  Generally,
									// pie->sUsed = TRUE, if used.
	{
	CThing*	pthing	= (CThing*)phot->m_ulUser;
	ASSERT(pthing != NULL);

	// If not used . . .
	if (pie->sUsed == FALSE)
		{
		// Switch on event.
		switch (pie->sEvent)
			{
			case RSP_MB0_PRESSED:
				{
				SetSel(pthing, phot);

				// Note that we used the event.
				pie->sUsed		= TRUE;
				pie->lUser		= 1;

				break;
				}

			case RSP_MB0_RELEASED:
				{
				SetSel(pthing, phot);
				// If EDIT_KEY_SENDTOBACK held down . . .
				UCHAR	aucKeys[128];
				rspScanKeys(aucKeys);
				if (aucKeys[EDIT_KEY_SENDTOBACK] != 0)
					{
					// Send this hot to back.
					phot->SetPriority(++ms_sBackPriority);
					// If we hit the back . . .
					if (ms_sBackPriority == RHOT_NO_PRIORITY)
						{
						ResetHotPriorities();
						}

					// Unselect.
					SetSel(NULL, NULL);
					}
				// If EDIT_KEY_SETCAMERATRACK held down . . .
				else if (aucKeys[EDIT_KEY_SETCAMERATRACK] != 0)
					{
					// If there is an editor thing . . .
					if (ms_pgething != NULL)
						{
						// If this thing is not the hood . . .
						if (pthing->GetClassID() != CThing::CHoodID)
							{
							// Get ID of item to track.
							ms_pgething->m_u16CameraTrackId	= pthing->GetInstanceID();
							}
						else
							{
							// Clear camera track ID.
							ms_pgething->m_u16CameraTrackId	= CIdBank::IdNil;
							}

						// User feedback.
						PlaySample(g_smidGeneralBeep, SampleMaster::UserFeedBack);
						}
					else
						{
						TRACE("ThingHotCall(): No Editor CThing.\n");
						}
					}

				// Note that we used the event.
				pie->sUsed		= TRUE;
				pie->lUser		= 1;

				break;
				}

			// This works on the Mac too because we faked a right mouse button
			// earlier in DoInput.  
			// System_Key+Mouse_Button on the mac == PC right mouse button
			case RSP_MB1_PRESSED:
				{
				SetSel(pthing, phot);

				// Size may have changed.
				RRect	rc;
				ms_pthingSel->EditRect(&rc);
				// Update hot.
				ms_photSel->m_sX	= rc.sX;
				ms_photSel->m_sY	= rc.sY;
				ms_photSel->m_sW	= rc.sW;
				ms_photSel->m_sH	= rc.sH;

				// See if a bouy was double clicked on
				if (ms_pthingSel->GetClassID() == CThing::CBouyID)
				{
					// If no bouys have been clicked on yet, set the
					// starting bouy link and start drawing the line
					if ((m_pBouyLink0 == NULL && m_pBouyLink1 == NULL) ||
						 (m_pBouyLink0 != NULL && m_pBouyLink1 != NULL))
					{
						m_pBouyLink0 = (CBouy*) ms_pthingSel;
						m_pBouyLink1 = NULL;
							
					}
					// This is the ending bouy
					else if (m_pBouyLink0 != NULL && m_pBouyLink1 == NULL)
					{
						m_pBouyLink1 = (CBouy*) ms_pthingSel;					
						m_pBouyLink0->AddLink(m_pBouyLink1);
						m_pBouyLink1->AddLink(m_pBouyLink0);
						AddNewLine(m_pBouyLink0->GetX(),
									  m_pBouyLink0->GetZ(),
									  m_pBouyLink1->GetX(),
									  m_pBouyLink1->GetZ());
					}
				}
				// If they double clicked on something other than a bouy
				// while they were drawing a bouy link line, then abort
				// the link draw.
				else if (m_pBouyLink0 != NULL && m_pBouyLink1 == NULL)
				{
					m_pBouyLink0 = m_pBouyLink1 = NULL;				
				}

				// Note that we used the event.
				pie->sUsed		= TRUE;
				pie->lUser		= 1;

				break;
				}


			case RSP_MB0_DOUBLECLICK:
				{
				SetSel(pthing, phot);

				// Modify.
				ms_pthingSel->EditModify();
				// Size may have changed.
				RRect	rc;
				ms_pthingSel->EditRect(&rc);
				// Update hot.
				ms_photSel->m_sX	= rc.sX;
				ms_photSel->m_sY	= rc.sY;
				ms_photSel->m_sW	= rc.sW;
				ms_photSel->m_sH	= rc.sH;

				// Note that we used the event.
				pie->sUsed		= TRUE;
				pie->lUser		= 1;

				break;
				}

			case RSP_MB1_DOUBLECLICK:
				{
				SetSel(pthing, phot);

				// If this is a pylon . . .
				if (ms_pthingSel->GetClassID() == CThing::CPylonID)
					{
					// Enter pylon trigger region edit mode.
					EditPylonTriggerRegion(ms_pthingSel);

					// Note that we used the event.
					pie->sUsed		= TRUE;
					pie->lUser		= 1;
					}

				break;
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
//
// Draw the link between the first bouy selected and the mouse cursor
//
////////////////////////////////////////////////////////////////////////////////

static void DrawBouyLink(	// Returns nothing.
	CRealm*	prealm,			// In:  Realm.
	CCamera*	pcamera)			// In:  View of prealm.
{
	short sMouseX;
	short sMouseY;
	short sButtons;

	if (ms_bDrawNetwork && m_pBouyLink0 != NULL && m_pBouyLink1 != NULL &&
	    m_pBouyLink0->Visible() && m_pBouyLink1->Visible())
	{
		// These lines show the paths the characters would take so
		// they should be only on the X/Z plane and, therefore, Y
		// is ignored.
		short	sBouyLink0X, sBouyLink0Y;
		Maprealm2Screen(
			prealm,
			pcamera,
			m_pBouyLink0->GetX(), 
			0, 
			m_pBouyLink0->GetZ(),
			&sBouyLink0X,
			&sBouyLink0Y);

		short	sBouyLink1X, sBouyLink1Y;
		Maprealm2Screen(
			prealm,
			pcamera,
			m_pBouyLink1->GetX(), 
			0, 
			m_pBouyLink1->GetZ(),
			&sBouyLink1X,
			&sBouyLink1Y);

		rspLine(249, g_pimScreenBuf, 
		        sBouyLink0X,
		        sBouyLink0Y, 
		        sBouyLink1X,
		        sBouyLink1Y);

	}
	else if (m_pBouyLink0 != NULL && m_pBouyLink1 == NULL)
	{
		// These lines show the paths the characters would take so
		// they should be only on the X/Z plane and, therefore, Y
		// is ignored.
		short	sBouyLink0X, sBouyLink0Y;
		Maprealm2Screen(
			prealm,
			pcamera,
			m_pBouyLink0->GetX(), 
			0, 
			m_pBouyLink0->GetZ(),
			&sBouyLink0X,
			&sBouyLink0Y);

		rspGetMouse(&sMouseX, &sMouseY, &sButtons);
		rspLine(249, g_pimScreenBuf, 
		        sBouyLink0X,
		        sBouyLink0Y, 
		        sMouseX, sMouseY);

	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Resets all RHots including and descended from ms_photHood
// to FRONTMOST_HOT_PRIORITY.
//
////////////////////////////////////////////////////////////////////////////////
static void ResetHotPriorities(void)	// Returns nothing.
	{
	if (ms_photHood != NULL)
		{
		// Set Hood's priority.
		ms_photHood->SetPriority(FRONTMOST_HOT_PRIORITY);
		// Do children.
		RHot*	phot	= ms_photHood->m_listChildren.GetHead();
		while (phot != NULL)
			{
			phot->SetPriority(FRONTMOST_HOT_PRIORITY);

			phot	= ms_photHood->m_listChildren.GetNext();
			}

		// Reset backmost priority.
		ms_sBackPriority	= FRONTMOST_HOT_PRIORITY;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// AddNewLine
////////////////////////////////////////////////////////////////////////////////

static void AddNewLine(short sX0, short sY0, short sX1, short sY1)
{
	LINKLINE newLine;

	// Sort the lines by smaller x.  If the X is the same then sort
	// by Y.  By presorting the lines before adding them, a line with
	// endpoints (x,y) (a,b) and a line (a,b) (x,y) will only be added to
	// the set once as the line (a,b) (x,y) which saves memory and
	// speeds the drawing.  
	if (sX0 == sX1)
	{
		if (sY0 < sY1)
		{
			newLine.sX0 = sX0;
			newLine.sY0 = sY0;
			newLine.sX1 = sX1;
			newLine.sY1 = sY1;
		}
		else
		{
			newLine.sX0 = sX1;
			newLine.sY0 = sY1;
			newLine.sX1 = sX0;
			newLine.sY1 = sY0;
		}
	}
	else 
	{
		if (sX0 < sX1)
		{
			newLine.sX0 = sX0;
			newLine.sY0 = sY0;
			newLine.sX1 = sX1;
			newLine.sY1 = sY1;
		}
		else
		{
			newLine.sX0 = sX1;
			newLine.sY0 = sY1;
			newLine.sX1 = sX0;
			newLine.sY1 = sY0;
		}
	}

	// Add the line to the set of lines
	m_NetLines.insert(newLine);
}

////////////////////////////////////////////////////////////////////////////////
// Write a NavNet log file to display the connections.- for debugging purposes
////////////////////////////////////////////////////////////////////////////////

static void NetLog(CNavigationNet* pNavNet)
{
/*
	ofstream txtout;
	ofstream routeout;
	CNavigationNet::nodeMap::iterator ibouy;
	CBouy::linkset::iterator ilink;
	UCHAR i;
	UCHAR ucHops;


	txtout.open("c:\\temp\\navnet.txt");
	routeout.open("c:\\temp\\navroute.txt");
	if (txtout.is_open() && routeout.is_open())
	{
		txtout << ";\n";
		txtout << "; NavNet connections file" << endl;
		txtout << ";" << endl;
		txtout << "; Node: Directly connected nodes\n";
		txtout << ";------------------------------------------------------------------\n";
		routeout << ";Bouy Routing Tables" << endl;
		routeout << ";" << endl;
		routeout << ";-----------------------------------------------------------------\n";

		if (pNavNet)
		{
			for (ibouy = pNavNet->m_NodeMap.begin(); 
				  ibouy != pNavNet->m_NodeMap.end(); ibouy++)
			{
				txtout << (USHORT) ((*ibouy).second->m_ucID) << " : "; 
				for (ilink = (*ibouy).second->m_apsDirectLinks.begin(); 
					  ilink != (*ibouy).second->m_apsDirectLinks.end(); ilink++)
				{
					txtout << (USHORT) (*ilink)->m_ucID << ", ";
				}
				txtout << endl;
				// Show routing table for reachable links
				routeout << "bouy " << (USHORT) ((*ibouy).second->m_ucID) << endl;
				for (i = 1; i < pNavNet->GetNumNodes(); i++)
				{
					ucHops = (*ibouy).second->NextRouteNode(i);
					if (ucHops < 255)
					{
						routeout << " vialink[" << (USHORT) i << "] = " << (USHORT) ucHops << endl;
					}
				}

					
			}
		}
		
		txtout.close();
		routeout.close();
	}
	else
	{
		TRACE("NetLog: Error opening log file for output\n");
	}
*/
}


////////////////////////////////////////////////////////////////////////////////
// UpdateNetLines
////////////////////////////////////////////////////////////////////////////////

static void UpdateNetLines(CNavigationNet* pNavNet)
{

	CNavigationNet::nodeMap::iterator ibouy;
	CBouy* pLinkedBouy = NULL;

	if (pNavNet)
	{
		m_NetLines.erase(m_NetLines.begin(), m_NetLines.end());

		for (ibouy = pNavNet->m_NodeMap.begin(); 
		     ibouy != pNavNet->m_NodeMap.end(); ibouy++)
		{
			pLinkedBouy = (*ibouy).second->m_aplDirectLinks.GetHead();
			while (pLinkedBouy)
			{
				AddNewLine((*ibouy).second->GetX(),
							  (*ibouy).second->GetZ(),
							  pLinkedBouy->GetX(),
							  pLinkedBouy->GetZ());
				pLinkedBouy = (*ibouy).second->m_aplDirectLinks.GetNext();
			}
		}
	}
	else
	{
		m_NetLines.erase(m_NetLines.begin(), m_NetLines.end());
	}

/*
	CNavigationNet::nodeMap::iterator ibouy;
	CBouy::linkset::iterator ilink;

	if (pNavNet)
	{
		m_NetLines.erase(m_NetLines.begin(), m_NetLines.end());

		for (ibouy = pNavNet->m_NodeMap.begin(); 
		     ibouy != pNavNet->m_NodeMap.end(); ibouy++)
		{
			for (ilink = (*ibouy).second->m_apsDirectLinks.begin(); 
			     ilink != (*ibouy).second->m_apsDirectLinks.end(); ilink++)
			{
				AddNewLine((*ibouy).second->GetX(),
				           (*ibouy).second->GetZ(),
							  (*ilink)->GetX(),
							  (*ilink)->GetZ());
			}
		}
	}
*/
//	NetLog(pNavNet);
}



////////////////////////////////////////////////////////////////////////////////
// DrawNetwork
////////////////////////////////////////////////////////////////////////////////

static void DrawNetwork(	// Returns nothing.
	CRealm*	prealm,			// In:  Realm.
	CCamera*	pcamera)			// In:  View of prealm.
{
	lineset::iterator i;
	
	if (ms_bDrawNetwork)
	{
		for (i = m_NetLines.begin(); i != m_NetLines.end(); i++)
		{
			// These lines show the paths the characters would take so
			// they should be only on the X/Z plane and, therefore, Y
			// is ignored.
			short	sBouyLink0X, sBouyLink0Y;
			Maprealm2Screen(
				prealm,
				pcamera,
				(*i).sX0, 
				0, 
				(*i).sY0,
				&sBouyLink0X,
				&sBouyLink0Y);

			short	sBouyLink1X, sBouyLink1Y;
			Maprealm2Screen(
				prealm,
				pcamera,
				(*i).sX1, 
				0, 
				(*i).sY1,
				&sBouyLink1X,
				&sBouyLink1Y);

			rspLine(250, g_pimScreenBuf,
				sBouyLink0X,
				sBouyLink0Y,
				sBouyLink1X,
				sBouyLink1Y);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Creates a view and adds it to the list of views.
////////////////////////////////////////////////////////////////////////////////
static short AddView(		// Returns 0 on success.
	CRealm*	prealm)			// In:  Realm in which to setup camera.
	{
	static short	sNum	= 0;
	short	sRes	= 0;	// Assume success.
	RListBox*	plb	= (RListBox*)ms_pguiCameras->GetItemFromId(GUI_ID_CAMERA_LIST);
	if (plb != NULL)
		{
		View*	pview;
		if (CreateView(&pview, prealm) == 0)
			{
			char	szTitle[256];
			sprintf(szTitle, "Camera %d", ++sNum);
			RGuiItem*	pgui			= plb->AddString(szTitle);
			pgui->m_lId					= (long)pview;
			plb->AdjustContents();

			pview->pgui->SetText("%s", szTitle);
			pview->pgui->Compose();
			}
		else
			{
			sRes	= -2;
			}
		}
	else
		{
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Kills a view and removes it from the list of views.
////////////////////////////////////////////////////////////////////////////////
static void RemoveView(		// Returns nothing.
	View*	pview)				// In: View to remove or NULL to remove currently
									// selected view.
	{
	RListBox*	plb	= (RListBox*)ms_pguiCameras->GetItemFromId(GUI_ID_CAMERA_LIST);
	if (plb != NULL)
		{
		RGuiItem*	pguiRemove;
		if (pview != NULL)
			{
			pguiRemove	= plb->GetItemFromId((long)pview);
			}
		else
			{
			pguiRemove	= plb->GetSel();
			}

		if (pguiRemove != NULL)
			{
			KillView((View*)(pguiRemove->m_lId));
			plb->RemoveItem(pguiRemove);
			plb->AdjustContents();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Removes all current views.
////////////////////////////////////////////////////////////////////////////////
static void RemoveViews(void)
	{
	View*	pview	= ms_listViews.GetHead();
	while (pview != NULL)
		{
		RemoveView(pview);
		pview	= ms_listViews.GetNext();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Creates a new View and adds it to the list of Views.
////////////////////////////////////////////////////////////////////////////////
static short CreateView(					// Returns 0 on success.
	View**	ppview,							// Out: New view, if not NULL.
	CRealm*	prealm)							// In:  Realm in which to setup camera.
	{
	short	sRes	= 0;	// Assume success.

	if (ppview != NULL)
		{
		*ppview	= NULL;	// Safety.
		}

	// Create view . . .
	View*	pview	= new View;
	if (pview != NULL)
		{
		// Create and load GUI . . .
		pview->pgui	= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, VIEW_GUI_FILE) );
		if (pview->pgui != NULL)
			{
			pview->pgui->GetClient(NULL, NULL, &(pview->sViewW), &(pview->sViewH) );

			// Get the scrollbars.
			pview->psbVert	= (RScrollBar*)pview->pgui->GetItemFromId(3);
			pview->psbHorz	= (RScrollBar*)pview->pgui->GetItemFromId(4);
			// Adjust dimensions to make scrollbars visible.
			if (pview->psbVert != NULL)
				{
				pview->sViewW	-= pview->psbVert->m_im.m_sWidth;
				}
			if (pview->psbHorz != NULL)
				{
				pview->sViewH	-= pview->psbHorz->m_im.m_sHeight;
				}

			// Set scrollbar range based on view size and realm size.
			if (pview->psbVert != NULL)
				{
				pview->psbVert->SetRange(0, prealm->m_phood->GetHeight() - pview->sViewH);
				}
			if (pview->psbHorz != NULL)
				{
				pview->psbHorz->SetRange(0, prealm->m_phood->GetWidth() - pview->sViewW);
				}

			// Add to list . . .
			if (ms_listViews.Add(pview) == 0)
				{
				// Activate.
				pview->pgui->SetVisible(TRUE);

				// Succcess.
				if (ppview != NULL)
					{
					*ppview	= pview;
					}
				}
			else
				{
				TRACE("CreateView(): Failed to add view to list.\n");
				sRes	= -3;
				}

			// If an error occurred after allocating GUI . . .
			if (sRes != 0)
				{
				delete pview->pgui;
				}
			}
		else
			{
			TRACE("CreateView(): Failed to load %s.\n", VIEW_GUI_FILE);
			sRes	= -2;
			}

		// If an error occurred after allocating View . . .
		if (sRes != 0)
			{
			delete pview;
			}
		}
	else
		{
		TRACE("CreateView(): Failed to allocate new View.\n");
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Destroys a View and removes it from the list of Views.
////////////////////////////////////////////////////////////////////////////////
static void KillView(						// Returns nothing.
	View*		pview)							// View to kill.
	{
	// Remove from list.
	ms_listViews.Remove(pview);
	// Delete.
	delete pview;
	}

////////////////////////////////////////////////////////////////////////////////
// Draw specified view.
////////////////////////////////////////////////////////////////////////////////
static void DrawView(						// Returns nothing.
	View*		pview,							// View to draw.
	CRealm*	prealm)							// Realm to draw.
	{
	// Get client.
	short	sClientPosX, sClientPosY;
	pview->pgui->GetClient(&sClientPosX, &sClientPosY, NULL, NULL);
	
	// Add in GUI's position.
	sClientPosX	+= pview->pgui->m_sX;
	sClientPosY	+= pview->pgui->m_sY;
	
	// Use scrollbars for position.
	short	sViewX	= 0;	// Safety.
	short	sViewY	= 0;	// Safety.
	if (pview->psbVert != NULL)
		{
		sViewY	= pview->psbVert->GetPos();
		}
	
	if (pview->psbHorz != NULL)
		{
		sViewX	= pview->psbHorz->GetPos();
		}

	// Draw GUI.
	pview->pgui->Draw(g_pimScreenBuf);
	
	// Draw scene.
	pview->cam.Snap(
		pview->sViewW,			// In:  View's width                         
		pview->sViewH,			// In:  View's height                        
		&(prealm->m_scene),	// In:  Scene to take picture of             
		prealm->m_phood,		// In:  Da hood.
		sViewX,					// In:  View's upper left x (in scene coords)
		sViewY,					// In:  View's upper left y (in scene coords)
		g_pimScreenBuf,		// In:  Film (where the picture ends up)     
		sClientPosX,			// In:  View's upper left x (in film coords) 
		sClientPosY				// In:  View's upper left y (in film coords) 
		);
	}

////////////////////////////////////////////////////////////////////////////////
// Draw all views.
////////////////////////////////////////////////////////////////////////////////
static void DrawViews(						// Returns nothing.
	CRealm*	prealm)							// Realm to draw.
	{
	View*	pview	= ms_listViews.GetHead();
	while (pview != NULL)
		{
		DrawView(pview, prealm);
		pview	= ms_listViews.GetNext();
		}

	ms_pguiCameras->Draw(g_pimScreenBuf);
	}

////////////////////////////////////////////////////////////////////////////////
// Clear specified view.
////////////////////////////////////////////////////////////////////////////////
static void ClearGUI(						// Returns nothing.
	RGuiItem*	pgui)							// In:  GUI's whose area we should clean.
	{
	rspRect(
		RSP_BLACK_INDEX, 
		g_pimScreenBuf,
		pgui->m_sX,
		pgui->m_sY,
		pgui->m_im.m_sWidth,
		pgui->m_im.m_sHeight);
	}

////////////////////////////////////////////////////////////////////////////////
// Clear all views.
////////////////////////////////////////////////////////////////////////////////
static void ClearViews(void)					// Returns nothing.
	{
	View*	pview	= ms_listViews.GetHead();
	while (pview != NULL)
		{
		ClearGUI(pview->pgui);

		pview	= ms_listViews.GetNext();
		}

	ClearGUI(ms_pguiCameras);
	}

////////////////////////////////////////////////////////////////////////////////
// Do focus hotbox logic and such for GUIs.
////////////////////////////////////////////////////////////////////////////////
static void DoView(							// Returns nothing.
	View*				pview,					// View to do.
	RInputEvent*	pie)						// Input event to process.
	{
	pview->pgui->m_hot.Do(pie);
	}

////////////////////////////////////////////////////////////////////////////////
// Do all views.
////////////////////////////////////////////////////////////////////////////////
static void DoViews(
	RInputEvent*	pie)						// Input event to process.
	{
	ms_pguiCameras->m_hot.Do(pie);

	View*	pview	= ms_listViews.GetTail();
	while (pview != NULL)
		{
		DoView(pview, pie);
		pview	= ms_listViews.GetPrev();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Draw the map.
////////////////////////////////////////////////////////////////////////////////
static void RefreshMap(						// Returns nothing.
	CRealm*	prealm)							// Realm to map.
	{
	ASSERT(ms_pguiMap != NULL);
	// Get area to draw in.
	if (ms_pguiMapZone != NULL)
		{
		// Clear.
		ms_pguiMapZone->Compose();

		short	sClientX, sClientY, sClientW, sClientH;
		ms_pguiMapZone->GetClient(&sClientX, &sClientY, &sClientW, &sClientH);
		
		// Allocate temp camera and film . . .
		CCamera	camera;
		RImage	imFilm;
		short	sViewW	= prealm->m_phood->m_pimBackground->m_sWidth;
		short	sViewH	= prealm->m_phood->m_pimBackground->m_sHeight;
		if (imFilm.CreateImage(
			sViewW,
			sViewH,
			RImage::BMP8) == 0)
			{
			// Take a shot.
			camera.Snap(
				sViewW,						// In:  View's width
				sViewH,						// In:  View's height
				&(prealm->m_scene),		// In:  Scene to take picture of
				prealm->m_phood,			// In:  Hood for this scene.
				0,								// In:  View's upper left x (in scene coords)
				0,								// In:  View's upper left y (in scene coords)
				&imFilm,						// In:  Film (where the picture ends up)
				0,								// In:  View's upper left x (in film coords)
				0);							// In:  View's upper left y (in film coords)

			// Stretch it as necessary to the map view.
			double	dRatioX	= (double)sClientW / (double)sViewW;
			double	dRatioY	= (double)sClientH / (double)sViewH;
			ms_dMapRatio		= MIN(dRatioX, dRatioY);
			RRect	rcClip(sClientX, sClientY, sClientW, sClientH);
			rspBlitT(
				&imFilm,							// Src.
				&(ms_pguiMapZone->m_im),	// Dst.
				sClientX,						// Dst x.
				sClientY,						// Dst y.
				ms_dMapRatio,					// Scale x.
				ms_dMapRatio,					// Scale y.
				&rcClip);						// Dst clip.
			}
		else
			{
			TRACE("RefreshMap(): Failed to allocate image at size of the realm.\n");
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Cancel any thing drag.
////////////////////////////////////////////////////////////////////////////////
static void CancelDrag(CRealm* prealm)		// Returns nothing.
	{
	// If there is an item being moved . . .
	if (ms_sMoving != FALSE)
		{
		DelThing(ms_pthingSel, ms_photSel, prealm);
		ms_sMoving		= FALSE;

		rspShowMouseCursor();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Place any thing being dragged.
////////////////////////////////////////////////////////////////////////////////
static void DragDrop(	// Returns nothing.
	short sDropX,			// In:  Drop x position.
	short sDropY,			// In:  Drop y position.
	short sDropZ)			// In:  Drop z position.
	{
	// If there is an item being moved . . .
	if (ms_sMoving != FALSE)
		{
		ASSERT(ms_pthingSel != NULL);
		ASSERT(ms_photSel != NULL);

		// Final position.
		MoveThing(
			ms_pthingSel, 
			ms_photSel, 
			sDropX,
			sDropY,
			sDropZ);

		SetSel(NULL, NULL);
		ms_sMoving		= FALSE;

		rspShowMouseCursor();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by the menu callback when it wants to tell the editor to continue 
// editting (end the menu).
////////////////////////////////////////////////////////////////////////////////
extern void Edit_Menu_Continue(void)
	{
	StopMenu();

	// Restore our favorite font...size is no matter.
	RGuiItem::ms_print.SetFont(15, &EDITOR_GUI_FONT);
	}

////////////////////////////////////////////////////////////////////////////////
// Called by the menu callback when it wants to tell the editor to quit the
// editor.
////////////////////////////////////////////////////////////////////////////////
extern void Edit_Menu_ExitEditor(void)
	{
	ms_lPressedId	= GUI_ID_EXIT;

	StopMenu();

	// Restore our favorite font...size is no matter.
	RGuiItem::ms_print.SetFont(15, &EDITOR_GUI_FONT);
	}

////////////////////////////////////////////////////////////////////////////////
// Convert a .RLM filename to a .RGN one.
////////////////////////////////////////////////////////////////////////////////
static void RlmNameToRgnName(	// Returns nothing.
	char*	pszRealmName,	// In:  .RLM name.
	char* pszRgnName)		// Out: .RGN name.
	{
	short	sIndex	= strlen(pszRealmName);
	while (sIndex-- >= 0)
		{
		if (pszRealmName[sIndex] == '\\')
			{
			sIndex	= strlen(pszRealmName);
			break;
			}

		if (pszRealmName[sIndex] == '.')
			{
			break;
			}
		}

	if (sIndex < 0)
		{
		sIndex	= strlen(pszRealmName);
		}

	strncpy(pszRgnName, pszRealmName, sIndex);
	pszRgnName[sIndex]	= '\0';
	strcat(pszRgnName, ".rgn");
	}

////////////////////////////////////////////////////////////////////////////////
// Move focus to next item in realm's thing list.
////////////////////////////////////////////////////////////////////////////////
static void NextItem(	// Returns nothing.
	CRealm*	prealm)		// In:  The realm we want the next thing in.
	{
	if (ms_pthingSel != NULL)
		{
		// This may make ms_pthingSel NULL.
		SetSel(ms_pthingSel->m_everything.GetNext(), NULL);
		}

	if (ms_pthingSel == NULL)
		{
		SetSel(prealm->m_everythingHead.GetNext(), NULL);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Move focus to previous item in realm's thing list.
////////////////////////////////////////////////////////////////////////////////
static void PrevItem(	// Returns nothing.
	CRealm*	prealm)		// In:  The realm we want the next thing in.
	{
	if (ms_pthingSel != NULL)
		{
		// This may make ms_pthingSel NULL.
		SetSel(ms_pthingSel->m_everything.GetPrev(), NULL);
		}

	if (ms_pthingSel == NULL)
		{
		SetSel(prealm->m_everythingTail.GetPrev(), NULL);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Load the trigger regions for the specified realm.
////////////////////////////////////////////////////////////////////////////////
static short LoadTriggerRegions(	// Returns 0 on success.
	char*	pszRealmName)				// In:  Name of the REALM (*.RLM) file.
											// The .ext is stripped and .rgn is appended.
	{
	short sRes	= 0;	// Assume success.

	// Change filename to .RGN name.
	char	szRgnName[RSP_MAX_PATH];
	RlmNameToRgnName(pszRealmName, szRgnName);

	RFile	file;
	if (file.Open(szRgnName, "rb", RFile::LittleEndian) == 0)
		{
		short	i;
		for (i = 0; i < NUM_ELEMENTS(ms_argns) && sRes == 0; i++)
			{
			sRes	= ms_argns[i].Load(&file);
			}

		file.Close();
		}
	else
		{
		TRACE("LoadTriggerRegions(): WARNING:  \"%s\" could not be opened.\n", szRgnName);
		sRes	= 1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Save the trigger regions for the specified realm.
////////////////////////////////////////////////////////////////////////////////
static short SaveTriggerRegions(	// Returns 0 on success.
	char*	pszRealmName,				// In:  Name of the REALM (*.RLM) file.
	CRealm*	prealm					// In:  Access of Realm Info
	)										// The .ext is stripped and .rgn is appended.
	{
	short sRes	= 0;	// Assume success.

	// Change filename to .RGN name.
	char	szRgnName[RSP_MAX_PATH];
	RlmNameToRgnName(pszRealmName, szRgnName);

	RFile	file;
	if (file.Open(szRgnName, "wb", RFile::LittleEndian) == 0)
		{
		short	i;
		for (i = 0; i < NUM_ELEMENTS(ms_argns) && sRes == 0; i++)
			{
			sRes	= ms_argns[i].Save(&file);
			}

		file.Close();
		}
	else
		{
		TRACE("SaveTriggerRegions(): Could not open \"%s\" for write.\n", szRgnName);
		sRes	= -1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Create an attribute map for the trigger regions for the specified realm.
// (And attaches it to the Realm for saving...)
// The current technique is that the CREALM must ALWAYS have a cTrigger in it,
// which in itself contains the pointer m_pmgi.
////////////////////////////////////////////////////////////////////////////////
static short CreateTriggerRegions(	// Returns 0 on success.
	CRealm*	prealm					// In:  Access of Realm Info
	)										
	{
	short sRes	= 0;	// Assume success.
	if (prealm->m_pTriggerMapHolder == NULL)
		{
		short sResult;

		TRACE("CreateTriggerRegions(): No default CThing to hold triggers!\n");
		TRACE("CreateTriggerRegions(): Adding one for your convenience!\n");
		CThing*	pThing = NULL;
		RHot*	photdummy;

		sResult = CreateNewThing(prealm, CThing::CTriggerID, 0, 0, 0, &pThing, &photdummy);
		if (sResult == 0) 
			{
			prealm -> m_pTriggerMap = ((CTrigger*)pThing)->m_pmgi;
			}
		else
			{
			TRACE("CreateTriggerRegions(): I really, REALLY tried, but there won't be trigger info\n");
			return -1;
			}
		
		}

	// REMOVE THE OLD TRIGGER MAP!
	if (prealm->m_pTriggerMap) delete prealm->m_pTriggerMap;
	prealm->m_pTriggerMap = NULL;

	////////////////////////////////////////////////////////////////////////////////
	// Create, Compress, and Save the trigger attribute map for the specified realm.
	////////////////////////////////////////////////////////////////////////////////

	RMultiGridIndirect* pTriggers = NULL;
	// Let's try going with a maximum of EIGHT overlapping regions for now.
	// And we'll try 32 x 32 tiles for now.  Assume tries largest to smallest.
	pTriggers = CreateRegionMap(prealm->m_phood->GetWidth(),
			prealm->m_phood->GetHeight(),8, 32,32);
	if (!pTriggers)
		{
		TRACE("CreateTriggerRegions(): Could not create region attributes!\n");
		sRes = -1;
		}
	else
		{
		if (StrafeAddRegion(pTriggers,ms_argns))
			{
			TRACE("CreateTriggerRegions(): Problem adding attributes!\n");
			sRes = -1;
			delete pTriggers;
			}
		else
			{
			// Create a seondary palette table index
			for (short i = 0;i < 256; i++)
				{
				prealm->m_asPylonUIDs[i] = 0;
				prealm->m_pTriggerMapHolder->m_ausPylonUIDs[i] = 0;

				if (ms_argns[i].pimRgn) 
					{
					prealm->m_asPylonUIDs[i] = ms_argns[i].u16InstanceId;
					// Set into the CThing, thich for now is what REALLY counts!
					prealm->m_pTriggerMapHolder->m_ausPylonUIDs[i] = ms_argns[i].u16InstanceId;
					}
				}

			// again, need interactive logic!
			if (CompressMap(pTriggers,16,16) != SUCCESS)
				{
				TRACE("CreateTriggerRegions(): Problem compressing attributes!\n");
				sRes = -1;
				delete pTriggers;
				}
			else
				{
				// Instead os saving it, make it part of the realm:
				// Then, when the realm is saved, so will it be

				// FINALLY, INSTALL IT INTO THE REALM
				// Add it using the official member:
				prealm->m_pTriggerMapHolder->AddData(pTriggers);
				}
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Change or clear the current pylon being edited.
////////////////////////////////////////////////////////////////////////////////
static void EditPylonTriggerRegion(	// Returns nothing.
	CThing* pthingPylon)					// In:  Pylon whose trigger area we want to
	{
	// If there's a current pylon being edited . . .
	if (ms_pylonEdit != NULL)
		{
		// Remove sprite from scene.
		ms_pylonEdit->m_pRealm->m_scene.RemoveSprite(&ms_spriteTriggerRgn);

		ms_spriteTriggerRgn.m_pImage	= NULL;

		// Put it into storage mode.
		ms_argns[ms_pylonEdit->m_ucID].SetMode(TriggerRgn::Storage);
		// Clear.
		ms_pylonEdit	= NULL;
		// Show mouse.
		rspShowMouseCursor();
		}

	// If there's a new pylon . . .
	if (pthingPylon != NULL)
		{
		ASSERT(pthingPylon->GetClassID() == CThing::CPylonID);

		CPylon*	pylon	= (CPylon*)pthingPylon;

		// If the region did not previously exist . . .
		if (ms_argns[pylon->m_ucID].pimRgn == NULL)
			{
			// Set location by mapping 3D pylon position to viewing surface 
			// position.
			pthingPylon->m_pRealm->Map3Dto2D(
				pylon->GetX(),
				0,
				pylon->GetZ(),
				&(ms_argns[pylon->m_ucID].sX),
				&(ms_argns[pylon->m_ucID].sY) );

			ms_argns[pylon->m_ucID].sX	-= TriggerRgn::MaxRgnWidth / 2;
			ms_argns[pylon->m_ucID].sY	-= TriggerRgn::MaxRgnHeight / 2;

			ms_argns[pylon->m_ucID].u16InstanceId	= pylon->GetInstanceID();
			}
		
		// Attempt to put it into edit mode . . .
		if (ms_argns[pylon->m_ucID].SetMode(TriggerRgn::Edit) == 0)
			{
			ms_spriteTriggerRgn.m_pImage			= ms_argns[pylon->m_ucID].pimRgn;
			ms_spriteTriggerRgn.m_sLayer			= CRealm::LayerSprite16;
			ms_spriteTriggerRgn.m_sPriority		= 32767;	// Always on top.
			ms_spriteTriggerRgn.m_sX2				= ms_argns[pylon->m_ucID].sX;
			ms_spriteTriggerRgn.m_sY2				= ms_argns[pylon->m_ucID].sY;

			// Add sprite to scene.
			pylon->m_pRealm->m_scene.UpdateSprite(&ms_spriteTriggerRgn);

			// Success.  Remember.
			ms_pylonEdit	= pylon;
			// Hide mouse.
			rspHideMouseCursor();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Set the selection to the specified CThing.
////////////////////////////////////////////////////////////////////////////////
static CThing*	SetSel(	// Returns CThing that previously was selected.
	CThing* pthingSel,	// In:  CThing to be selected.
	RHot*	photSel)			// In:  Hotbox of CThing to be selected.
	{
	CThing*	pthingRes	= ms_pthingSel;

	ms_pthingSel	= pthingSel;
	ms_photSel		= photSel;

	// If this is an actual CThing . . .
	if (ms_pthingSel != NULL)
		{
		// Don't allow GUI focus.
		RGuiItem::SetFocus(NULL);
		
		// If the corresponding hotbox was not specified . . .
		if (photSel == NULL)
			{
			// Get it.
			ms_photSel	= ms_pthingSel->m_phot;
			}
		}

	UpdateSelectionInfo(true);

	return pthingRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Delete the specified item.
////////////////////////////////////////////////////////////////////////////////
static void DelThing(	// Returns nothing.
	CThing* pthingDel,	// In:  CThing to be deleted.
	RHot*	photDel,			// In:  Hotbox of CThing to be deleted.
	CRealm* prealm)		// In:  Current Realm
	{
	if (pthingDel != NULL)
		{
		// If hot not specified . . .
		if (photDel == NULL)
			{
			photDel	= pthingDel->m_phot;
			}

		CNavigationNet* pNavNet = NULL;

		switch (pthingDel->GetClassID())
			{
			case CThing::CHoodID:
				// *** LOCALIZE ***
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					"Delete Hood",
					"You may not delete the hood.");
				SetSel(NULL, NULL);
				pthingDel = NULL;
				photDel = NULL;
				break;

			case CThing::CGameEditThingID:
				// *** LOCALIZE ***
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					"Delete Hood",
					"You may not delete the CGameEditThing.");
				SetSel(NULL, NULL);
				pthingDel = NULL;
				photDel = NULL;
				break;

			case CThing::CPylonID:
				// Destroy its associated trigger region.
				ms_argns[((CPylon*)pthingDel)->m_ucID].Destroy();
				break;

			case CThing::CBouyID:
				// If it is a Bouy, unlink the bouy from the network and
				// update the network.
				((CBouy*) ms_pthingSel)->Unlink();
				pNavNet = prealm->GetCurrentNavNet();
				pNavNet->RemoveBouy(((CBouy*) ms_pthingSel)->m_ucID);
				pNavNet->UpdateRoutingTables();
				// If you deleted one that the connection line was being
				// drawn to, then clear the connection line.
				if ((CBouy*) ms_pthingSel == m_pBouyLink0 ||
					 (CBouy*) ms_pthingSel == m_pBouyLink1)
					m_pBouyLink0 = m_pBouyLink1 = NULL;
				// If the network lines are being shown, update them, otherwise
				// they will get updated when view lines is turned on.
				if (ms_bDrawNetwork)
					UpdateNetLines(pNavNet);
				break;

			case CThing::CNavigationNetID:
				// If it is a NavNet, delete all of the Bouys associated
				// with the NavNet.  Make sure its not the last NavNet, 
				// and make sure there is still a current NavNet for the
				// realm after it is deleted.
				if (prealm->m_asClassNumThings[CThing::CNavigationNetID] > 1)
				{
					// Remove the Net from the list box
					RListBox* plbRemove = (RListBox*) ms_pguiNavNets->GetItemFromId(GUI_ID_NAVNET_LIST);
					plbRemove->RemoveItem(plbRemove->GetItemFromId(ms_pthingSel->GetInstanceID()));
					plbRemove->Compose();

					// Delete the network and select a new current Net if this was the one.
					((CNavigationNet*) ms_pthingSel)->DeleteNetwork();
					if (ms_pthingSel == prealm->GetCurrentNavNet())
					{
						CListNode<CThing>* pNext = prealm->m_aclassHeads[CThing::CNavigationNetID].m_pnNext;
						bool bSearching = true;
						CNavigationNet* pNet = NULL;
						while (bSearching && pNext->m_powner != NULL)
						{
							pNet = (CNavigationNet*) pNext->m_powner;
							if (ms_pthingSel != pNet)
							{
								pNet->SetAsDefault();
								bSearching = false;
								RListBox* plb = (RListBox*) ms_pguiNavNets->GetItemFromId(GUI_ID_NAVNET_LIST);
								plb->SetSel(plb->GetItemFromId(pNet->GetInstanceID()));											
								UpdateNetLines(pNet);
							}
							pNext = pNext->m_pnNext;
						}									
					}
					if (ms_bDrawNetwork)
						UpdateNetLines((CNavigationNet*) ms_pthingSel);
				}
				else
				{
					// *** LOCALIZE ***
					rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					"Delete Navigation Net",
					"You may not delete the last remaining Navigation Net.");
					SetSel(NULL, NULL);
					pthingDel = NULL;
					photDel = NULL;
				}
				break;
			}

		// If we still have anything to delete . . .
		if (pthingDel)
			{
			// If there is an editor thing . . .
			if (ms_pgething != NULL)
				{
				// If this item was being tracked by camera . . .
				if (pthingDel->GetInstanceID() == ms_pgething->m_u16CameraTrackId)
					{
					// Done with that.
					ms_pgething->m_u16CameraTrackId	= CIdBank::IdNil;
					}
				}

			// If this is the selection . . .
			if (pthingDel == ms_pthingSel)
				{
				SetSel(NULL, NULL);
				}
			}

		// If we were moving . . .
		if (ms_sMoving != FALSE)
			{
			// If nothing left to move . . .
			if (ms_pthingSel == NULL)
				{
				// Stop moving.
				ms_sMoving		= FALSE;
				// Show mouse cursor.
				rspShowMouseCursor();
				}
			}
		}

	// Safe if NULL.
	delete pthingDel;
	delete photDel;
	}

////////////////////////////////////////////////////////////////////////////////
// Delete all the items in the currently selected class.
////////////////////////////////////////////////////////////////////////////////
static void DelClass(	// Returns nothing.
	CThing* pthingDel,	// In:  CThing to be deleted.
	CRealm* prealm)		// In:  Current realm
	{
	char	szTitle[512];
	sprintf(
		szTitle, 
		"Delete entire \"%s\" class",
		(pthingDel != NULL) 
			? CThing::ms_aClassInfo[pthingDel->GetClassID()].pszClassName
			: "CThing"
		);
	// VERIFY . . .
	if (rspMsgBox(
		RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNO,
		szTitle,
		"Are you sure you want to perform this group delete?!"
		) == RSP_MB_RET_YES)
		{
		CListNode<CThing>*	plnDel;
		CListNode<CThing>*	plnTail;
		// If thing specified . . .
		if (pthingDel != NULL)
			{
			// Use category of item.
			plnDel	= prealm->m_aclassHeads[pthingDel->GetClassID()].m_pnNext;
			plnTail	= &(prealm->m_aclassTails[pthingDel->GetClassID()]);
			}
		else
			{
			// Use all CThings.
			plnDel	= prealm->m_everythingHead.m_pnNext;
			plnTail	= &(prealm->m_everythingTail);
			}

		CListNode<CThing>*	plnNext;
		while (plnDel != plnTail)
			{
			plnNext	= plnDel->m_pnNext;

			DelThing(plnDel->m_powner, NULL, prealm);

			plnDel	= plnNext;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Delete all but the pylons, bouys, soundthings and soundrelays.
// This is useful for making a template of a level that is already fully \
// populated.
////////////////////////////////////////////////////////////////////////////////
static void DelMost(	// Returns nothing.
	CRealm* prealm)		// In:  Current realm
	{
	char	szTitle[512];
	sprintf(
		szTitle, 
		"Delete Everything except bouys, pylons, warps, soundthings and soundrelays?"
		);
	// VERIFY . . .
	if (rspMsgBox(
		RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNO,
		szTitle,
		"Are you sure you want to perform such a delete?"
		) == RSP_MB_RET_YES)
		{
		CListNode<CThing>*	plnDel;
		CListNode<CThing>*	plnTail;
		CThing::ClassIDType classType;
		// Use all CThings.
		plnDel	= prealm->m_everythingHead.m_pnNext;
		plnTail	= &(prealm->m_everythingTail);

		CListNode<CThing>*	plnNext;
		while (plnDel != plnTail)
			{
			plnNext	= plnDel->m_pnNext;

			classType = plnDel->m_powner->GetClassID();
			switch (classType)
			{
				default:
					DelThing(plnDel->m_powner, NULL, prealm);
					break;

				case CThing::CHoodID:
				case CThing::CPylonID:
				case CThing::CBouyID:
				case CThing::CNavigationNetID:
				case CThing::CSoundThingID:
				case CThing::CSndRelayID:
				case CThing::CGameEditThingID:
				case CThing::CWarpID:
					break;
			}

			plnDel	= plnNext;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Copy a thing to the paste buffer.
////////////////////////////////////////////////////////////////////////////////
static short CopyItem(	// Returns 0 on success.
	CThing* pthingCopy)	// In:  CThing to copy.
	{
	short		sRes	= 0;	// Assume success.

	// If anything to copy . . .
	if (pthingCopy != NULL)
		{
		// Switch for exceptions by type.
		switch (pthingCopy->GetClassID() )
			{
			case CThing::CBouyID:
			case CThing::CPylonID:
			case CThing::CNavigationNetID:
			case CThing::CHoodID:
				rspMsgBox(
					RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
					"Cannot Copy",
					"Cannot Copy %s",
					CThing::ms_aClassInfo[pthingCopy->GetClassID()].pszClassName);
				break;
			default:
				{
				// If paste buffer open . . .
				if (ms_filePaste.IsOpen() != FALSE)
					{
					ms_filePaste.Close();
					}

				// Open paste buffer.
				if (ms_filePaste.Open(
					PASTE_BUFFER_INITIAL_SIZE,
					PASTE_BUFFER_GROW_SIZE,
					RFile::NeutralEndian) == 0)
					{
					if (pthingCopy->Save(&ms_filePaste, ms_sFileCount--) == 0)
						{
						// Success.  Store the type.
						ms_idPaste	= pthingCopy->GetClassID();
						}
					else
						{
						TRACE("CopyItem(): pthingCopy->Save() failed.\n");
						sRes	= -2;
						}
					}
				else
					{
					TRACE("CopyItem(): Failed to open paste buffer.\n");
					sRes	= -1;
					}
				break;
				}
			}

		// If any errors . . .
		if (sRes != 0)
			{
			ms_filePaste.Close();
			}
		}
	else
		{
		sRes	= 1;
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Copy a thing to the paste buffer.
////////////////////////////////////////////////////////////////////////////////
static short PasteItem(	// Returns 0 on success.
	CRealm*	prealm,		// In:  The realm to paste into.
	short		sX,			// In:  Location for new thing.
	short		sY,			// In:  Location for new thing.
	short		sZ)			// In:  Location for new thing.
	{
	short	sRes	= 0;	// Assume success.

	// Drop anything we currently are holding onto.
	DragDrop(
		sX,
		sY,
		sZ);

	// If open . . .
	if (ms_filePaste.IsOpen() != FALSE)
		{
		// Go to beginning.
		ms_filePaste.Seek(0, SEEK_SET);

		// Create the thing using the paste buffer . . .
		if (CreateNewThing(								// Returns 0 on success.
			prealm,											// Realm to create in.
			ms_idPaste,										// ID to create.
			sX,												// x
			sY,												// y
			sZ,												// z
			&ms_pthingSel,									// New thing.
			&ms_photSel,									// New hotbox for thing.
			&ms_filePaste)	== 0)							// RFile src.
			{
			// Success.

			// Start moving/placing object.
			ms_sMoving	= TRUE;

			// Disable drag view scrolling.
			ms_bDragScroll	= false;

			// Enter drag.  Cheesy...Just need to get this to Bill and
			// Then I'll clean up.
			ms_sDragState	= 4;

			if (rspGetMouseCursorShowLevel() > 0)
				{
				rspHideMouseCursor();
				}
			}
		else
			{
			TRACE("PasteItem(): CreateNewThing() failed.\n");
			sRes	= -1;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Map a screen coordinate to a realm coordinate.
// Note that this function's *psRealmY output is always
// the height specified by the realm's attribute map
// at the resulting *psRealmX, *psRealmZ.
////////////////////////////////////////////////////////////////////////////////
static void MapScreen2Realm(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	short sScreenX,				// In:  Screen x coord.
	short sScreenY,				// In:  Screen y coord.
	short* psRealmX,				// Out: Realm x coord.
	short* psRealmY,				// Out: Realm y coord (always via realm's height map).
	short* psRealmZ)				// Out: Realm z coord.
	{
	// Get coordinate on 2D viewing plane.
	short	sRealmX2	= sScreenX + pcamera->m_sScene2FilmX;
	short	sRealmY2	= sScreenY + pcamera->m_sScene2FilmY;

	// If there's a hood . . .
	if (prealm->m_phood != NULL)
		{
		// Map to realm's X/Z plane:
		// Z is stretched.
		prealm->MapY2DtoZ3D(sRealmY2, psRealmZ);
		// X is trivial.
		*psRealmX	= sRealmX2;

		// If there is an attribute map . . .
		if (prealm->m_pTerrainMap != NULL)
			{
			// The realm y is the height indicated by the attribute map
			// in the given location on the X/Z plane.
			*psRealmY = prealm->GetHeight(
				*psRealmX,
				*psRealmZ);
			}
		else
			{
			// Safety.
			*psRealmY	= 0;
			}
		}
	else
		{
		// Safety.
		*psRealmX	= sRealmX2;
		*psRealmY	= 0;
		*psRealmZ	= sRealmY2;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Map a realm coordinate to a screen coordinate.
////////////////////////////////////////////////////////////////////////////////
static void Maprealm2Screen(	// Returns nothing.
	CRealm*	prealm,				// In:  Realm.
	CCamera*	pcamera,				// In:  View of prealm.
	short		sRealmX,				// In:  Realm x coord.
	short		sRealmY,				// In:  Realm y coord.
	short		sRealmZ,				// In:  Realm z coord.
	short*	psScreenX,			// Out: Screen x coord.
	short*	psScreenY)			// Out: Screen y coord.
	{
	// Map coordinate onto 2D viewing plane.
	short	sViewX2;
	short	sViewY2;
	prealm->Map3Dto2D(
		sRealmX,
		sRealmY,
		sRealmZ,
		&sViewX2,
		&sViewY2);

	// Offset to screen.
	*psScreenX	= sViewX2 - pcamera->m_sScene2FilmX;
	*psScreenY	= sViewY2 - pcamera->m_sScene2FilmY;
	}

////////////////////////////////////////////////////////////////////////////////
// Blit attribute areas lit by the specified mask into the specified image.
////////////////////////////////////////////////////////////////////////////////
static void AttribBlit(			// Returns nothing.
	RMultiGrid*	pmg,				// In:  Multigrid of attributes.
	U16			u16Mask,			// In:  Mask of important attributes.
	RImage*		pimDst,			// In:  Destination image.
	short			sSrcX,			// In:  Where in Multigrid to start.
	short			sSrcY,			// In:  Where in Multigrid to start.
	short			sW,				// In:  How much of multigrid to use.
	short			sH)				// In:  How much of multigrid to use.
	{
	// Keep it at the edge of the film.
	ms_spriteAttributes.m_sX2				= sSrcX;
	ms_spriteAttributes.m_sY2				= sSrcY;

	ASSERT(pimDst->m_sDepth == 8);
	ASSERT(pimDst->m_sWidth >= sW);
	ASSERT(pimDst->m_sHeight >= sH);

	// Get dst start.
	U8*	pu8RowDst	= pimDst->m_pData;
	U8*	pu8Dst;
	long	lPitch;

	short	sGridW, sGridH;
	pmg->GetGridDimensions(&sGridW, &sGridH);

	short	sIterX;

	while (sH--)
		{
		lPitch	= pimDst->m_lPitch;
		sIterX	= sSrcX;
		pu8Dst	= pu8RowDst;
		while (lPitch--)
			{
			if (pmg->GetVal(sIterX++, sSrcY, 0x0000) & u16Mask)
				{
				*pu8Dst	= SHOW_ATTRIBS_DRAW_INDEX;
				}

			pu8Dst++;
			}

		pu8RowDst	+= pimDst->m_lPitch;
		sSrcY++;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Callback for attrib mask multibtns.
////////////////////////////////////////////////////////////////////////////////
static void AttribMaskBtnPressed(	// Returns nothing.
	RGuiItem*	pgui_pmb)				// In:  Ptr to the pressed GUI (which is a multibtn).
	{
	ASSERT(pgui_pmb->m_type == RGuiItem::MultiBtn);

	RMultiBtn*	pmb	= (RMultiBtn*)pgui_pmb;

	// Get mask to affect.
	U16*	pu16AttribMask	= (U16*)(pmb->m_ulUserInstance);

	// Add or subtract mask dependent on state.
	switch (pmb->m_sState)
		{
		case 0:	// Pressing.
			break;
		case 1:	// Off.
			*pu16AttribMask	&= ~pmb->m_ulUserData;
			break;
		case 2:	// On.
			*pu16AttribMask	|= pmb->m_ulUserData;
			break;
		}

	// If we now have a mask value . . .
	if (*pu16AttribMask)
		{
		// If there's no image . . .
		if (ms_spriteAttributes.m_pImage == NULL)
			{
			short	sErr	= 0;

			// Create it . . .
			ms_spriteAttributes.m_pImage	= new RImage;
			if (ms_spriteAttributes.m_pImage)
				{
				sErr	= SizeShowAttribsSprite();

				// If an error occurred after allocating the image . . .
				if (sErr)
					{
					delete ms_spriteAttributes.m_pImage;
					ms_spriteAttributes.m_pImage	= NULL;
					}
				}
			else
				{
				TRACE("AttribMaskBtnPressed(): Error allocating RImage.\n");
				sErr	= 1;
				}

			// If an error occurred . . .
			if (sErr)
				{
				// Clear mask so we don't use it later.
				*pu16AttribMask	= 0;
				}
			}
		}
	else
		{
		// If no more attribute masks . . .
		if (ms_u16TerrainMask == 0 && ms_u16LayerMask == 0)
			{
			// If there is an image, delete it.  No longer needed.
			delete ms_spriteAttributes.m_pImage;
			ms_spriteAttributes.m_pImage	= NULL;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Resizes the attribute sprite, if allocated.
////////////////////////////////////////////////////////////////////////////////
static short SizeShowAttribsSprite(void)	// Returns 0 on success.
	{
	short	sRes	= 0;	// Assume success.

	if (ms_spriteAttributes.m_pImage)
		{
		ms_spriteAttributes.m_pImage->DestroyData();

		if (ms_spriteAttributes.m_pImage->CreateImage(
			ms_pcameraCur->m_sViewW,
			ms_pcameraCur->m_sViewH,
			RImage::BMP8) == 0)
			{
			ms_spriteAttributes.m_sLayer			= CRealm::LayerSprite16;
			ms_spriteAttributes.m_sPriority		= 32767;	// Always on top.
			}
		else
			{
			TRACE("SizeShowAttribsSprite(): RImage::CreateImage() failed.\n");
			sRes	= -1;
			}

		// If an error occurred . . .
		if (sRes)
			{
			delete ms_spriteAttributes.m_pImage;
			ms_spriteAttributes.m_pImage	= NULL;
			}
		}

	return sRes;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Our RFile callback
//
////////////////////////////////////////////////////////////////////////////////
static void MyRFileCallback(long lBytes)
	{
	ms_lFileBytesSoFar	+= lBytes;

	long lNow = rspGetMilliseconds();
	if ((lNow - ms_lRFileCallbackTime) > MY_RFILE_CALLBACK_INTERVAL)
		{
		// Do an update
		UpdateSystem();

		// Lock the composite buffer for access.
		rspLockBuffer();

		// Clear area.  Note that this function locks and unlocks the display
		// when necessary.
		rspRect(
			RSP_BLACK_INDEX,
			g_pimScreenBuf,
			ms_sFileOpTextX,
			ms_sFileOpTextY,
			ms_sFileOpTextW,
			ms_sFileOpTextH);

		// Draw text in cleared area.
		ms_printFile.print(
			g_pimScreenBuf,
			ms_sFileOpTextX,
			ms_sFileOpTextY,
			ms_szFileOpDescriptionFrmt,
			ms_lFileBytesSoFar);

		// Done with buffer other than to update the screen with it.
		rspUnlockBuffer();

		// Update the display.
		rspUpdateDisplay(
			ms_sFileOpTextX,
			ms_sFileOpTextY,
			ms_sFileOpTextW,
			ms_sFileOpTextH);

		// Save new time
		ms_lRFileCallbackTime = rspGetMilliseconds();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Init load/save counter.  You should call KillFileCounter() after
// done with the file access.
////////////////////////////////////////////////////////////////////////////////
static void InitFileCounter(			// Returns nothing.
	char*	pszDescriptionFrmt)			// In:  sprintf format for description of 
												// operation.
	{
	// Make sure string (with some digits) will fit . . .
	if (strlen(pszDescriptionFrmt) < sizeof(ms_szFileOpDescriptionFrmt) - 10 )
		{
		strcpy(ms_szFileOpDescriptionFrmt, pszDescriptionFrmt);
		}
	else
		{
		strcpy(ms_szFileOpDescriptionFrmt, "Progress: %ld bytes");
		}

	// Hook the RFile callback and reset the timer.  The callback does nothing
	// more than call Update() every once in a while, which gives the system
	// a bit of time (very important on Mac) and also allows update events to
	// be processed, which is nice for both Mac and Win32.
	RFile::ms_criticall = MyRFileCallback;
	ms_lRFileCallbackTime = rspGetMilliseconds();
	ms_lFileBytesSoFar	= 0;

	// A little feedback would be nice.
	rspSetCursor(RSP_CURSOR_WAIT);

	// Setup printer.
	ms_printFile.SetWordWrap(TRUE);
	ms_printFile.SetJustifyLeft();
	ms_printFile.SetColor(249, 0, 0);
	ms_printFile.SetFont(19, &g_fontBig);
	// Determine size and position of text.
	// Create sample string.
	char	szSample[sizeof(ms_szFileOpDescriptionFrmt)];
	sprintf(szSample, ms_szFileOpDescriptionFrmt, LONG_MAX);
	// Just use entire width for now.
	ms_sFileOpTextW	= ms_printFile.GetWidth(szSample);
	// Get height.
	ms_printFile.GetPos(NULL, NULL, NULL, &ms_sFileOpTextH);
	ms_sFileOpTextX	= PROGRESS_X;
	ms_sFileOpTextY	= PROGRESS_Y - (ms_sFileOpTextH + 5);

	RRect rect(ms_sFileOpTextX, ms_sFileOpTextY, ms_sFileOpTextW, ms_sFileOpTextH);
	ms_printFile.SetDestination(g_pimScreenBuf, &rect);

	// Don't need keys that occurred before the operation.
	rspClearAllInputEvents();
	}

////////////////////////////////////////////////////////////////////////////////
// Kill load/save counter.  Can be called multiple times w/o corresponding
// Init().
////////////////////////////////////////////////////////////////////////////////
static void KillFileCounter(void)	// Returns nothing.
	{
	// Unhook the RFile callback
	RFile::ms_criticall = 0;
	// Restore arrow cursor.
	rspSetCursor(RSP_CURSOR_ARROW);
	// Clear op descriptor.
	ms_szFileOpDescriptionFrmt[0]	= '\0';

	// Don't need keys that occurred during the operation.
	rspClearAllInputEvents();
	}

////////////////////////////////////////////////////////////////////////////////
// Helper function explicity for UpdateSelectionInfo().
////////////////////////////////////////////////////////////////////////////////
inline
void SetPosInfo(			// Returns nothing.
	RGuiItem*	pgui,		// In:  GUI to update.  NULL is safe.
	double		dVal)		// In:  Value to udpate to GUI.
	{
	if (pgui)
		{
		if (dVal != CThing::InvalidPosition)
			{
			pgui->SetText("%g", dVal);
			}
		else
			{
			pgui->SetText("%s", "N/A");
			}

		pgui->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Update selection info in the info GUI.
////////////////////////////////////////////////////////////////////////////////
static void UpdateSelectionInfo(	// Returns nothing.
	bool bTitle /*= false*/)		// In:  true to update the title info as well.
	{
	if (ms_pguiInfo)
		{
		if (bTitle == true)
			{
			// Update the title info.
			if (ms_pthingSel)
				{
				ms_pguiInfo->SetText(
					"Info for \"%s\" [%u]",
					CThing::ms_aClassInfo[ms_pthingSel->GetClassID()].pszClassName,
					ms_pthingSel->GetInstanceID() );
				}
			else
				{
				ms_pguiInfo->SetText("No selection");
				}

			ms_pguiInfo->Compose();
			}

		// Always update other fields.
		if (ms_pthingSel)
			{
			SetPosInfo(ms_pguiInfoXPos, ms_pthingSel->GetX() );
			SetPosInfo(ms_pguiInfoYPos, ms_pthingSel->GetY() );
			SetPosInfo(ms_pguiInfoZPos, ms_pthingSel->GetZ() );
			}
		else
			{
			SetPosInfo(ms_pguiInfoXPos, CThing::InvalidPosition);
			SetPosInfo(ms_pguiInfoYPos, CThing::InvalidPosition);
			SetPosInfo(ms_pguiInfoZPos, CThing::InvalidPosition);
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
//
// Get a temporary file name
//
////////////////////////////////////////////////////////////////////////////////
static short TmpFileName(								// Returns 0 if successfull, non-zero otherwise
	char* pszFileName,									// Out: Temp file name returned here
	short sMaxSize)										// In:  Maximum size of file name
	{
	short sResult = 0;

	#if defined(WIN32)

		char	szPath[RSP_MAX_PATH];
		ULONG	ulLen	= GetTempPath(sizeof(szPath), szPath);
		if (ulLen >= sizeof(szPath) )
			{
			TRACE("TmpFileName(): GetTempPath() could not fit the path and filename into our string.\n");
			sResult = -1;
			}
		else if (ulLen == 0)
			{
			TRACE("TmpFileName(): GetTempPath() failed.\n");
			sResult = -1;
			}

		if (GetTempFileName(szPath, "RLM", 0, pszFileName) == FALSE)
			{
			TRACE("TmpFileName(): GetTempFileName() failed.\n");
			sResult = -1;
			}

	#else

		// Impliment for some other system!
		ASSERT(0);

	#endif

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Helper for ShowRealmStatistics() to display an item position or N/A,
// if invalid.
////////////////////////////////////////////////////////////////////////////////
inline
void Pos2Str(		// Returns nothing.
	double dPos,	// In:  CThing position.
	char* pszStr)	// Out: String representation.
	{
	if (dPos != CThing::InvalidPosition)
		{
		sprintf(pszStr, "%g", dPos);
		}
	else
		{
		strcpy(pszStr, "N/A");
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Show statistics for the specified realm.
////////////////////////////////////////////////////////////////////////////////
static short ShowRealmStatistics(	// Returns 0 on success.
	CRealm*	prealm,						// In:  Realm to get stats on.
	CThing** ppthing)						// Out: Selected thing, if not NULL.
	{
	short	sRes	= 0;	// Assume success.

	// Store cursor show level so we can restore it when done.
	short sOrigCursorShowLevel	= rspGetMouseCursorShowLevel();
	// Set the mouse so we can see it.
	rspSetMouseCursorShowLevel(1);

	// Get GUI . . .
	RGuiItem*	pguiRoot	= RGuiItem::LoadInstantiate(FullPath(GAME_PATH_VD, REALM_STATISTICS_GUI_FILE) );
	if (pguiRoot)
		{
		// Get list box . . .
		RListBox*	plb	= (RListBox*)pguiRoot->GetItemFromId(GUI_ID_REALM_STATISTICS);
		if (plb)
			{
			ASSERT(plb->m_type == RGuiItem::ListBox);

			// Go through the list of everything adding each item to the list.
			char	szThingDescription[2048];
			char	szX[256];
			char	szY[256];
			char	szZ[256];
			double	dX, dY, dZ;
			long	lNum	= 0;
			CListNode<CThing>*	pthingnode	= prealm->m_everythingHead.m_pnNext;
			CThing*	pthing;
			while (pthingnode != &(prealm->m_everythingTail))
				{
				lNum++;
				pthing	= pthingnode->m_powner;
				dX			= pthing->GetX();
				dY			= pthing->GetY();
				dZ			= pthing->GetZ();

				Pos2Str(dX, szX);
				Pos2Str(dY, szY);
				Pos2Str(dZ, szZ);

				sprintf(
					szThingDescription, 
					"%ld) \"%s\" ID: %u X: %s Y: %s Z: %s",
					lNum,
					CThing::ms_aClassInfo[pthing->GetClassID()].pszClassName,
					pthing->GetInstanceID(),
					szX,
					szY,
					szZ);

				RGuiItem*	pguiThing	= plb->AddString(szThingDescription);

				if (pguiThing)
					{
					// Success.
					pguiThing->m_lId	= (long)pthing;
					}
				else
					{
					TRACE("ShowRealmStatistics(): Error adding <<%s>> to the list box.\n", szThingDescription);
					sRes	= -3;
					}

				pthingnode	= pthingnode->m_pnNext;
				}

			// Repaginate now.
			plb->AdjustContents();

			// Display until dismissed.
			CThing::DoGui(pguiRoot);

			if (ppthing)
				{
				// If there's a selection . . .
				RGuiItem* pguiSel	= plb->GetSel();
				if (pguiSel)
					{
					*ppthing	= (CThing*)(pguiSel->m_lId);
					}
				else
					{
					*ppthing	= NULL;
					}
				}
			}
		else
			{
			TRACE("ShowRealmStatistics(): Failed to get realm stats listbox.\n");
			sRes	= -2;
			}

		delete pguiRoot;
		pguiRoot	= NULL;
		}
	else
		{
		TRACE("ShowRealmStatistics(): Failed to load realm stats GUI.\n");
		sRes	= -1;
		}

	// Clear queues.
	rspClearAllInputEvents();
	// Re-init input.
	ClearLocalInput();

	// Restore cursor show level.
	rspSetMouseCursorShowLevel(sOrigCursorShowLevel);

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Callback from realm during time intensive operations.
////////////////////////////////////////////////////////////////////////////////
static bool RealmOpProgress(			// Returns true to continue; false to
												// abort operation.
	short	sLastItemProcessed,			// In:  Number of items processed so far.
	short	sTotalItemsToProcess)		// In:  Total items to process.
	{
	static long	lLastProgressPos;
	static long	lProgressX, lProgressY, lProgressW, lProgressH;
	static long	lLastCallTime;

	// Just need to get the key status array once.
	U8*	pau8KeyStatus	= rspGetKeyStatusArray();

	bool	bContinue	= true;	// Assume we want to continue.

	// If new operation . . .
	if (sLastItemProcessed == 0)
		{
		lLastProgressPos	= 0;
		lProgressX			= PROGRESS_X;
		lProgressY			= PROGRESS_Y;
		lProgressW			= PROGRESS_W;
		lProgressH			= PROGRESS_H;
		lLastCallTime		= 0;

		// Clear key status array.
		memset(pau8KeyStatus, 0, 128);

		// Lock the composite buffer for access.
		rspLockBuffer();

		// Draw initial outline.
		rspRect(
			1,
			PROGRESS_OUTLINE_COLOR_INDEX,
			g_pimScreenBuf,
			lProgressX - 1,
			lProgressY - 1,
			lProgressW + 2,
			lProgressH + 2);

		// Done with buffer other than to update the screen with it.
		rspUnlockBuffer();
		
		rspUpdateDisplay(
			lProgressX - 1,
			lProgressY - 1,
			lProgressW + 2,
			lProgressH + 2);
		}

	long lNow = rspGetMilliseconds();
	if ((lNow - lLastCallTime) > PROGRESS_CALLBACK_INTERVAL)
		{
		// Do an update
		UpdateSystem();

		// Compute new position.
		long	lNewProgressPos;
		if (sTotalItemsToProcess > 0)
			{
			lNewProgressPos	= (long)sLastItemProcessed * lProgressW / (long)sTotalItemsToProcess;
			}
		else
			{
			lNewProgressPos	= lProgressW;
			}

		// Lock the composite buffer.
		rspLockBuffer();
		
		rspRect(
			PROGRESS_COLOR_INDEX,
			g_pimScreenBuf,
			lProgressX + lLastProgressPos,
			lProgressY,
			lNewProgressPos - lLastProgressPos,
			lProgressH);

		// Done with buffer other than to update the screen with it.
		rspUnlockBuffer();

		// Update display.
		rspUpdateDisplay(
			lProgressX + lLastProgressPos,
			lProgressY,
			lNewProgressPos - lLastProgressPos,
			lProgressH);

		lLastProgressPos	= lNewProgressPos;

		// If escape hit . . .
		if (pau8KeyStatus[KEY_ABORT_REALM_OPERATION])
			{
			if (rspMsgBox(
				RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNO,
				g_pszAppName,
				"Are you sure you want to abort this operation?")
				== RSP_MB_RET_YES)
				{
				bContinue	= false;
				}
			}

		// Don't use lNow here...let's not count time we spent drawing.
		lLastCallTime	= rspGetMilliseconds();
		}

	return bContinue;
	}


#endif // !defined(EDITOR_DISABLED)

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
