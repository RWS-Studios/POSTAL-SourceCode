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
// localize.h
// Project: Postal
//
//	History:
//		02/03/97	JMI	Added g_pszBadNoSakDirPath[].
//
//		04/11/97	JMI	Added g_pszSaveDemoTitle and g_pszGeneralWriteError.
//
//		04/16/97 MJR	Added a few more messages.
//
//							And then added a few more video messages, too.
//
//		04/21/97 MJR	Created generic version of "bad path" message.
//
//		05/14/97	JMI	Added g_pszPickedUpMessage_ld_s for CDude.
//
//		06/03/97	JMI	Changed g_pszAudioModeError to g_pszAudioModeGeneralError
//							and added g_pszAudioModeInUseError, 
//							g_pszAudioModeNoDeviceError, 
//							g_pszAudioModeNotSupportedError.
//
//		06/04/97	JMI	Added g_pszNotOnCDROM.
//
//		06/06/97	JMI	Changed message for g_pszPickedUpMessage_ld_s and its name
//							to g_pszPickedUpMessage_s.
//
//		06/14/97 MJR	Added/enhanced general file error messages.
//
//		07/13/97	JMI	Changed g_pszAudioModeNotSupportedError to 
//							g_pszAudioModeNotSupportedError_s and changed to ask the
//							user if they want to try the vanilla mode.
//							Also, changed the other audio errors to include the
//							sprintf format parameters in their names.
//							Also, added g_pszAudioVanillaModeNotSupportedError_s for
//							the case when the hardware does not support the vanilla 
//							mode.
//
//		07/21/97	JMI	Added g_pszNoWeaponButHaveAmmo_s_s and g_pszNoWeapon_s.
//
//		07/28/97	JMI	Added g_pszDispenserNoDispenseeTypeChosen.
//
//		08/05/97	JMI	Added g_pszDontHaveExecuteWeapon_s and 
//							g_pszDontHaveSuicideWeapon_s.
//
//		08/12/97	JMI	Added g_pszGenericBrowseFor_s_Title and 
//							g_pszGenericMustBeRelativePath_s.
//
//		08/17/97	JMI	Got rid of m_szMessages and all message related functions
//							and variables from CDude since we are now using the toolbar 
//							for dude status feedback to the user.  This includes:  
//							MsgTypeInfo, m_lNextStatusUpdateTime, m_lMsgUpdateDoneTime, 
//							m_print, m_bClearedStatus, m_szMessages[], m_sDeadMsgNum, 
//							ms_amtfMessages[], ms_u8FontForeIndex, ms_u8FontBackIndex,
//							ms_u8FontShadowIndex, DrawStatus(), StatusChange(), 
//							MessageChange(), Message(), UpdateFontColors(), 
//							CPowerUp::ms_apszPowerUpTypeNames[], 
//							CPowerUp::GetDescription(), and some strings and a string
//							array in localize.*.
//
//		08/20/97	JMI	Added g_pszDontDropYourselfMORON.
//
//		08/21/97	JMI	Added g_pszDoofusCannotFindNavNet_EditMode_hu_hu and
//							g_pszDoofusCannotFindNavNet_PlayMode_hu_hu.
//
//		08/25/97	JMI	Added g_pszCannotOpenSoundFiles_s_s.
//
//		09/11/97	JMI	Added g_pszPlayOneRealmOnlyMessage.
//
//		09/18/97	JMI	Added localization vars for menus.
//
//		09/18/97	JMI	Added localization vars for SampleMaster categories.
//
//		09/29/97	JMI	Added g_pszNoSoundFiles.
//
//		10/07/97 BRH	Added score localizations.
//
//		10/09/97	JMI	Added g_pszVideoChangeDepthErrorUnderGDI_s.
//
//		10/13/97	JMI	Added g_pszControlsMenu_UseJoystick to localizable texts.
//
//		12/04/97 BRH	Added g_pszStartSinglePlayerMenu_AddOn.
//
//		03/05/98 BRH	Added g_pszWrongCD message that tells the player that they
//							must have the original Postal CD in the drive in order to
//							play the Postal Add on Pack.
//
//		02/04/00 MJR	Added g_pszPromptForOriginalCD.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LOCALIZE_H
#define LOCALIZE_H


// I went with variables instead of macros for one primary reason.  At some
// point, most likely near the end of the project, when time is in short supply,
// someone will need to go through and clean up all these strings.  If these
// were macros, then nearly every file in the project would need to recompile
// each time this file was changed.  With strings, you generally don't modify
// this file, but instead modify the .CPP file, so only that file needs to be
// recompiled.  The disadvantage is that whenever you need to add, delete, or
// rename strings, you have to do it both here and in the .CPP file.

extern char g_pszAppName[];
extern char g_pszCriticalErrorTitle[];
extern char g_pszBadBlueInit[];
extern char g_pszVideoModeError[];
extern char g_pszVideoChangeDepthError[];
extern char g_pszVideoDepthError[];
extern char g_pszVideoResolutionError[];
extern char g_pszVideoPagesError[];
extern char	g_pszVideoChangeDepthErrorUnderGDI_s[];

extern char g_pszAudioModeGeneralError_s[];
extern char g_pszAudioModeInUseError_s[];
extern char g_pszAudioModeNoDeviceError_s[];
extern char g_pszAudioModeNotSupportedError_s[];
extern char g_pszAudioVanillaModeNotSupportedError_s[];

extern char g_pszPrefFileName[];
extern char g_pszPrefOpenError[];
extern char g_pszPrefReadError[];
extern char g_pszPrefWriteError[];
extern char g_pszPrefReadOnly[];
extern char g_pszTitleError[];
extern char g_pszCantFindAssets[];
extern char g_pszWrongCD[];
extern char g_pszPromptForOriginalCD[];
extern char g_pszGeneralError[];

extern char g_pszBadPath_s_s[];
extern char g_pszBadCDPath_s_s[];

extern char	g_pszAssetsMissingError[];
extern char g_pszNotOnCDROM[];
extern char	g_pszSaveFileQuery[];

extern char g_pszSaveDemoTitle[];
extern char g_pszSaveGameTitle[];
extern char g_pszSaveGameErrorTitle[];
extern char g_pszSaveGameErrorText[];
extern char g_pszLoadGameTitle[];

extern char	g_pszFileOpenError_s[];
extern char	g_pszFileReadError_s[];
extern char	g_pszFileWriteError_s[];

extern char g_pszDispenserNoDispenseeTypeChosen[];

extern char g_pszGenericBrowseFor_s_Title[];

extern char	g_pszGenericMustBeRelativePath_s[];

extern char g_pszDontDropYourselfMORON[];

extern char g_pszDoofusCannotFindNavNet_EditMode_hu_hu[];
extern char g_pszDoofusCannotFindNavNet_PlayMode_hu_hu[];

extern char g_pszCannotOpenSoundFiles_s_s[];

extern char g_pszNoSoundFiles[];

extern char g_pszPlayOneRealmOnlyMessage[];


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Misc  ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern char g_pszEditorDisabled[];
extern char g_pszMultiplayerDisabled[];
extern char g_pszBuy[];
	
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Menus ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern char	g_pszMainMenu_Title[];
extern char g_pszMainMenu_Start[];
extern char g_pszMainMenu_Options[];
extern char g_pszMainMenu_Editor[];
extern char g_pszMainMenu_Buy[];
extern char g_pszMainMenu_Exit[];

extern char g_pszVerifyExitMenu_Title[];
extern char g_pszVerifyExitMenu_Yes[];
extern char g_pszVerifyExitMenu_No[];

extern char g_pszVerifyQuitMenu_Title[];
extern char g_pszVerifyQuitMenu_Yes[];
extern char g_pszVerifyQuitMenu_No[];

extern char g_pszGameMenu_Title[];
extern char g_pszGameMenu_Continue[];
extern char g_pszGameMenu_Save[];
extern char g_pszGameMenu_Options[];
extern char g_pszGameMenu_Quit[];

extern char g_pszEditorMenu_Title[];
extern char g_pszEditorMenu_Continue[];
extern char g_pszEditorMenu_Options[];
extern char g_pszEditorMenu_Quit[];

extern char g_pszOptionsMenu_Title[];
extern char g_pszOptionsMenu_Video[];
extern char g_pszOptionsMenu_Audio[];
extern char g_pszOptionsMenu_Controls[];
extern char g_pszOptionsMenu_Multiplayer[];
extern char g_pszOptionsMenu_Performance[];
extern char g_pszOptionsMenu_Difficulty[];
extern char g_pszOptionsMenu_Crosshair[];

extern char g_pszDifficultyMenu_Title[];
extern char g_pszDifficultyMenu_SetDifficulty[];

extern char g_pszOrganMenu_Title[];
extern char g_pszOrganMenu_SpecialKeysHeading[];
extern char g_pszOrganMenu_NumericKeysFunction[];
extern char g_pszOrganMenu_AlphaKeysFunction[];
extern char g_pszOrganMenu_TabKeyFunction[];
extern char g_pszOrganMenu_Exit[];

extern char g_pszAudioMenu_Title[];
extern char g_pszAudioMenu_Mixer[];
extern char g_pszAudioMenu_SoundTest[];

extern char g_pszVideoMenu_Title[];
extern char g_pszVideoMenu_Gamma[];

extern char g_pszControlsMenu_Title[];
extern char g_pszControlsMenu_KeyboardSetup[];
extern char g_pszControlsMenu_MouseSetup[];
extern char g_pszControlsMenu_JoystickSetup[];
extern char g_pszControlsMenu_TurningSpeeds[];
extern char g_pszControlsMenu_UseMouse[];
extern char g_pszControlsMenu_HorizMouseSensitivity[];
extern char g_pszControlsMenu_VertMouseSensitivity[];

extern char g_pszKeyboardSetupMenu_Title[];

extern char g_pszMouseSetupMenu_Title[];

extern char g_pszJoystickSetupMenu_Title[];

extern char g_pszPerformanceMenu_Title[];
extern char g_pszPerformanceMenu_Transparency[];
extern char g_pszPerformanceMenu_3dLighting[];
extern char g_pszPerformanceMenu_Particles[];
extern char g_pszPerformanceMenu_DynamicVolume[];
extern char g_pszPerformanceMenu_AmbientSounds[];

extern char g_pszRotationSetupMenu_Title[];
extern char g_pszRotationSetupMenu_RunningSlow[];
extern char g_pszRotationSetupMenu_RunningFast[];
extern char g_pszRotationSetupMenu_StandingSlow[];
extern char g_pszRotationSetupMenu_StandingFast[];
extern char g_pszRotationSetupMenu_TapDegrees[];
extern char g_pszRotationSetupMenu_RestoreDefaults[];
extern char g_pszRotationSetupMenu_RestoreDefaultsOld[];

extern char g_pszVolumesMenu_Title[];

extern char g_pszStartGameMenu_Title[];
extern char g_pszStartGameMenu_SinglePlayer[];
extern char g_pszStartGameMenu_Multiplayer[];
extern char g_pszStartGameMenu_Demo[];

extern char g_pszStartSinglePlayerMenu_Title[];
extern char g_pszStartSinglePlayerMenu_New[];
extern char g_pszStartSinglePlayerMenu_AddOn[];
extern char g_pszStartSinglePlayerMenu_LoadLevel[];
extern char g_pszStartSinglePlayerMenu_LoadGame[];
extern char g_pszStartSinglePlayerMenu_Challenge[];

extern char g_pszStartChallengeMenu_Title[];
extern char g_pszStartChallengeMenu_Gauntlet[];
extern char g_pszStartChallengeMenu_Timed[];
extern char g_pszStartChallengeMenu_Goal[];
extern char g_pszStartChallengeMenu_Flag[];
extern char g_pszStartChallengeMenu_CheckPoint[];

extern char g_pszStartMultiplayerMenu_Title[];
extern char g_pszStartMultiplayerMenu_Join[];
extern char g_pszStartMultiplayerMenu_Host[];
extern char g_pszStartMultiplayerMenu_Options[];


extern char g_pszJoinGameMenu_Title[];
extern char g_pszJoinGameMenu_Browse[];
extern char g_pszJoinGameMenu_ConnectTo[];

extern char g_pszHostGameMenu_Title[];
extern char g_pszHostGameMenu_Start[];

extern char g_pszStartDemoMenu_Title[];
extern char g_pszStartDemoMenu_Browse[];
extern char g_pszStartDemoMenu_Play[];
extern char g_pszStartDemoMenu_Record[];

extern char g_pszMultiplayerSetupMenu_Title[];
extern char g_pszMultiplayerSetupMenu_Name[];
extern char g_pszMultiplayerSetupMenu_Color[];
extern char g_pszMultiplayerSetupMenu_Protocol[];
extern char g_pszMultiplayerSetupMenu_Connection[];

extern char* g_apszScoreDisplayText[];
extern char* g_apszScoreGoalText[];
extern char* g_apszScoreUnits[];
extern char* g_apszScoreExplanations[];

// Keep at end -- was not in original localizable text.
extern char g_pszControlsMenu_UseJoystick[];

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Sample Master ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern char* g_apszSoundCategories[];


#endif // LOCALIZE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
