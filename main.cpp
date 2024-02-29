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
// main.cpp
// Project: Nostril (aka Postal)
//
// This module sets up the system and RSPiX.
//
// History:
//		11/19/96 MJR	Started.
//
//		02/04/97	JMI	Now defaults to using RSP_DOSYSTEM_SLEEP instead of
//							RSP_DOSYSTEM_HOGCPU for rspSetDoSystemMode().
//
//		02/19/97	JMI	rspInitBlue was being called before 
//							rspSetWin32Video/AudioType().
//
//		02/21/97	JMI	Now gets specific display settings from game settings(INI).
//
//		02/21/97	JMI	Now gets audio lag time from INI.
//
//		03/14/97	JMI	Now, under Win32, gets value from INI indicating whether to
//							manage Windows' GUI components when the screen is resized.
//
//		03/24/97	JMI	No longer exits if SetMode() fails.  This allows the game
//							to be played on systems without sound cards.
//
//		04/14/97	JMI	Now uses zeroinit flag to SmartHeap.
//
//		04/16/97 MJR	Added more advanced audio init that retries for several
//							seconds, and then asks the user whether to abort, retry,
//							or ignore.
//
//							If video mode can't be set, it now tries to figure out
//							the exact reason and reports it to the user.
//
//							Fixed a stupid bug that prevented audio from ever working.
//
//		04/17/97 MJR	Futher clarified video error messages.
//
//							Discovered that the whole suggest-vide-mode thing is
//							different than I expected, so I restructured everything.
//
//		05/20/97	JMI	Changed the #if block around the SmartHeap specific stuff
//							to include only Intel Processor or MAC OS.
//
//		06/03/97	JMI	Now, in the case an audio mode cannot be obtained, we
//							attempt to use a message specific to the error returned
//							by RMix::SetMode() for the message box displayed.
//
//		06/16/97	JMI	Now sets do system mode to highest cooperation level in
//							debug and lowest coop level in release.
//
//		07/05/97 MJR	Added <smrtheap.hpp> if NOT in debug mode so we could
//							properly set one of its options.  Not sure why this worked
//							without <smrtheap.hpp> on the PC, but it didn't on the mac.
//
//		07/06/97 MJR	Added call to new rspSetWin32StaticColors() so we'll have
//							a common set of colors across platforms.
//
//		07/07/97 MJR	Added RSPiX profiling stuff (in disabled form).
//
//		07/13/97	JMI	Added MAIN_VANILLA_AUDIO_* macro overrides to MAIN_AUDIO_*
//							defaults.  These should be used when the INI or default
//							audio mode fails.
//
//		07/26/97	JMI	We were using the video type as the audio type.  Whoopsee.
//
//		09/03/97 PPL	Added the pragma to turn off far data for the mempool
//							initialization flag so that the smartheap library can reach it.
//
//		09/05/97 BRH	Added #if defined(MAC) around pragmas which didn't compile
//							on the PC.
//
//		09/09/97	JMI	Added check of INI flag in setting of Blue Shield Cursor
//							Mode on PC.
//
//		09/25/97	JMI	Now, on the PC, when setting the video type, sets the 
//							rspLock/Unlock behavior to be strict even when in simpler
//							modes that don't require that level of behavior (like GDI).
//
//		10/09/97	JMI	Added g_pszVideoChangeDepthErrorUnderGDI_s in the event
//							the colordepth could not be changed and the user had 
//							specified to use GDI (i.e., not DirectX).
//
//		10/21/97	JMI	Put back the play movie hack.  Also, added ability to turn
//							it off via the INI.
//
//		10/21/97	JMI	Now disables RipCord static logo only if movie successfully 
//							plays.
//
//		10/24/97	JMI	Now switches the video mode back if AVI changes (this seems
//							to happen when we're in a DirectX mode and we launch the
//							AVI).
//
//		10/31/97	JMI	Now uses MixBits entry in INI (in section Audio) to
//							determine what bit depth to mix samples with (defaults to
//							the device depth).
//
//		01/05/98	JMI	Now the MixBits var defaults to 16.
//
//		01/21/98	JMI	No longer plays movie regardless of OS or INI setting.
//
////////////////////////////////////////////////////////////////////////////////
#define MAIN_CPP

#ifdef WIN32
    #include <direct.h>
#else
    #include <sys/time.h>
#endif

#include "RSPiX.h"
#include "WishPiX/Prefs/prefs.h"

#include "localize.h"
#include "game.h"
#include "main.h"
#include "menus.h"

#include "title.h"
#include "input.h"

//#define RSP_PROFILE_ON
#include "ORANGE/Debug/profile.h"

#if PLATFORM_MACOSX
// This redefines main() to something else, since libSDLmain-osx.a will have
//  the actual application entry point...that will setup some Cocoa stuff and
//  then call the redefined main() in this file...
#include "SDL.h"
#endif

#if WITH_STEAMWORKS
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif
#include "steam/steam_api.h"
#include "WishPiX/Menu/menu.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Amount of time to retry audio before telling the user it's not working
#define AUDIO_RETRY_TIME	5000


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////
int wideScreenWidth;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Set up video for the game.
////////////////////////////////////////////////////////////////////////////////
static short SetupVideo(					// Returns 0 on success.
	short	sUseCurrentDeviceDimensions,	// In:  1 to use current video area.
	short	sDeviceWidth,						// In:  Desired video hardware width.
	short	sDeviceHeight)						// In:  Desired video hardware height.
	{
	short	sResult	= 0;

#ifdef MOBILE
	wideScreenWidth = 850;
#elif defined(PANDORA)
	wideScreenWidth = 800;
#else
	//wideScreenWidth = 640;

	// Attempt to grab desired resolution from desktop
	short sDepth, sWidth, sHeight;
	rspQueryVideoModeReset();
	while (!rspQueryVideoMode(&sDepth, &sWidth, &sHeight));
	TRACE("rspQueryVideoMode result: %ix%ix%i\n", sWidth, sHeight, sDepth);
	// Sanity-check result
	if (sHeight > 480 && sWidth > 640)
		wideScreenWidth = 480 * sWidth / sHeight;
	else // fallback to specified resolution
		wideScreenWidth = 480 * sDeviceWidth / sDeviceHeight;
#endif
	// If "use current settings" is specified, we get the current device settings
	// instead of using those specified in the prefs file.
	if (sUseCurrentDeviceDimensions != FALSE)
		rspGetVideoMode(NULL, &sDeviceWidth, &sDeviceHeight);

	// Try setting video mode using device size specified in prefs file
	sResult = rspSetVideoMode(
		MAIN_SCREEN_DEPTH,
		sDeviceWidth,
		sDeviceHeight,
		MAIN_WINDOW_WIDTH,
		MAIN_WINDOW_HEIGHT,
		MAIN_SCREEN_PAGES,
		MAIN_SCREEN_SCALING);
	if (sResult != 0)
		{

		// Create description of video mode for error messages
		char acVideoMode[100];
		sprintf(acVideoMode, "%hd by %hd Pixels", (short)MAIN_WINDOW_WIDTH, (short)MAIN_WINDOW_HEIGHT);
		if (MAIN_SCREEN_DEPTH <= 16)
			sprintf(&(acVideoMode[strlen(acVideoMode)]), ", %hd Colors", (short)pow(2.0, (double)MAIN_SCREEN_DEPTH));
		else if (MAIN_SCREEN_DEPTH == 24)
			sprintf(&(acVideoMode[strlen(acVideoMode)]), ", True Color (24 bit)");
		else
			sprintf(&(acVideoMode[strlen(acVideoMode)]), ", True Color (32 bit)");
		if (MAIN_SCREEN_PAGES > 1)
			sprintf(&(acVideoMode[strlen(acVideoMode)]), ", %hd Pages", (short)MAIN_SCREEN_PAGES);

		// Get current device depth (before we try changing it)
		short sCurrentDeviceDepth;
		rspGetVideoMode(&sCurrentDeviceDepth);

		// Find closest available device size for the settings we need.  This function knows about
		// ALL available video modes (the same ones you get in Winows' Display Settings Dialog), so
		// if it can't find a matching mode, then it isn't available.  However, just because it does
		// find a match doesn't mean we can set it, because under Win95, changing the color depth
		// requires a reboot (unless they have DirectX, a fancy video driver, or the QuickRes utility).
		sResult = rspSuggestVideoMode(
			MAIN_SCREEN_DEPTH,
			MAIN_WINDOW_WIDTH,
			MAIN_WINDOW_HEIGHT,
			MAIN_SCREEN_PAGES,
			MAIN_SCREEN_SCALING,
			&sDeviceWidth,
			&sDeviceHeight,
			NULL);
		if (sResult == 0)
			{

			// Try to set suggested mode
			sResult = rspSetVideoMode(
				MAIN_SCREEN_DEPTH,
				sDeviceWidth,
				sDeviceHeight,
				MAIN_WINDOW_WIDTH,
				MAIN_WINDOW_HEIGHT,
				MAIN_SCREEN_PAGES,
				MAIN_SCREEN_SCALING);
			if (sResult != 0)
				{

				// If current depth is different from required depth, then that is most likely the
				// reason for the failure.
				if (sCurrentDeviceDepth != MAIN_SCREEN_DEPTH)
					{
					rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
						g_pszCriticalErrorTitle,
						g_pszVideoChangeDepthError,
						acVideoMode);
					TRACE("SetupVideo(): Error returned by rspSetVideoMode() -- most likely due to attempted change in depth!\n");
					}
				else
					{
					TRACE("SetupVideo(): Error returned by rspSetVideoMode()!\n");
					rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
						g_pszCriticalErrorTitle,
						g_pszVideoModeError,
						acVideoMode);
					}
				}
			}
		else
			{

			// Since rspSuggestVideoMode() failed, we know that the requested mode is
			// not available.  Now we just want to figure out the EXACT problem so
			// we can report it to the user.  Since modes are returned in sorted order
			// by increasing depth, height, width, and pages, it makes this a bit easier.

			// Look for mode with the requested depth.  If there isn't one, depth is the problem.
			rspQueryVideoModeReset();
			short sDeviceDepth;
			short sDevicePages;
			do	{
				sResult = rspQueryVideoMode(
					&sDeviceDepth,
					&sDeviceWidth,
					&sDeviceHeight,
					&sDevicePages);
				} while ((sResult == 0) && (sDeviceDepth < MAIN_SCREEN_DEPTH));
			if ((sResult == 0) && (sDeviceDepth == MAIN_SCREEN_DEPTH))
				{
				// We got the depth, now find a mode with the requested resolution.  If
				// there isn't one, then resolution at this depth is the problem.
				while ( (sResult == 0) &&
						  (sDeviceDepth == MAIN_SCREEN_DEPTH) &&
						  ((sDeviceWidth < MAIN_WINDOW_WIDTH) || (sDeviceHeight < MAIN_WINDOW_HEIGHT)) )
					{
					sResult = rspQueryVideoMode(
						&sDeviceDepth,
						&sDeviceWidth,
						&sDeviceHeight,
						&sDevicePages);
					}
				if ( (sResult == 0) &&
					  (sDeviceDepth == MAIN_SCREEN_DEPTH) &&
					  (sDeviceWidth >= MAIN_WINDOW_WIDTH) &&
					  (sDeviceHeight >= MAIN_WINDOW_HEIGHT) )
					{
					// We got the depth and resolution, which only leaves pages or scaling
					// as possible problems.  RSPiX doesn't support scaling as of 04/16/97
					// and probably never will, so if we eliminate that with an ASSERT(),
					// then we can assume that the problem is the number of pages.
					ASSERT(MAIN_SCREEN_SCALING == 0);
					sResult = -1;
					TRACE("SetupVideo(): No video modes available at %dx%d, %d-bit, with %d pages!\n",
						MAIN_WINDOW_WIDTH , MAIN_WINDOW_HEIGHT, MAIN_SCREEN_DEPTH, MAIN_SCREEN_PAGES);
					rspMsgBox(
						RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
						g_pszCriticalErrorTitle,
						g_pszVideoPagesError,
						acVideoMode);
					}
				else
					{
					sResult = -1;
					TRACE("SetupVideo(): No %hd-bit video modes go up to %hdx%hd resolution!\n",
						(short)MAIN_SCREEN_DEPTH, (short)MAIN_WINDOW_WIDTH, (short)MAIN_WINDOW_HEIGHT);
					rspMsgBox(
						RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
						g_pszCriticalErrorTitle,
						g_pszVideoResolutionError,
						acVideoMode);
					}
				}
			else
				{
				sResult = -1;
				TRACE("SetupVideo(): No %hd-bit video modes are available!\n",
					(short)MAIN_SCREEN_DEPTH);
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					g_pszCriticalErrorTitle,
					g_pszVideoDepthError,
					acVideoMode);
				}
			}
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Allocates a chunk and resizes so that we may be able to have some large
// blocks of contiguous memory.
////////////////////////////////////////////////////////////////////////////////
static char* CreateChunk(	// Returns the memory ptr that will hold the chunk
									// in place.  Needs to be freed with free() when done
									// with the chunk.
	long lChunkSize)			// In:  Size of chunk to create.
	{
	char*	pcOrig		= (char*)malloc(lChunkSize);
	char* pcReAlloc	= (char*)realloc(pcOrig, 1024);
	ASSERT(pcOrig == pcReAlloc);
	if (pcReAlloc)
		{
		return pcReAlloc;
		}
	else
		{
		return pcOrig;
		}
	}


static void assert_types_are_sane(void)
{
    ASSERT(sizeof (S8) == 1);
    ASSERT(sizeof (U8) == 1);
    ASSERT(sizeof (S16) == 2);
    ASSERT(sizeof (U16) == 2);
    ASSERT(sizeof (S32) == 4);
    ASSERT(sizeof (U32) == 4);
    ASSERT(sizeof (S64) == 8);
    ASSERT(sizeof (U64) == 8);

    U32 val = 0x02000001;
#if SYS_ENDIAN_BIG
    ASSERT(*((U8*) &val) == 0x02);
#else
    ASSERT(*((U8*) &val) == 0x01);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// Set up the system environment and if all goes well, start the game.
//
////////////////////////////////////////////////////////////////////////////////

// Global versions of argc/argv...
int _argc = 0;
char **_argv = NULL;

long playthroughMS = 0;

#if WITH_STEAMWORKS
bool WaitingForInitialSteamStats = true;
bool EnableSteamAchievements = true;
bool EnableSteamCloud = true;
bool StoreSteamStatsPending = false;
bool EarnedSteamAchievements[ACHIEVEMENT_MAX];
uint32 SteamAppID = 0;

static const char *GetAchievementName(const Achievement ach)
{
    switch (ach)
    {
        #define ACH_CASE(name) case ACHIEVEMENT_##name: return #name
        ACH_CASE(KILL_FIRST_VICTIM);
        ACH_CASE(START_SECOND_LEVEL);
        ACH_CASE(DUCK_UNDER_ROCKET);
        ACH_CASE(RUN_5_MINUTES);
        ACH_CASE(PERFORM_FIRST_EXECUTION);
        ACH_CASE(KILL_100);
        ACH_CASE(KILL_1000);
        ACH_CASE(KILL_10000);
        ACH_CASE(COMPLETE_LEVEL_10);
        ACH_CASE(COMPLETE_GAME);
        ACH_CASE(FIRE_1000000_BULLETS);
        ACH_CASE(HIT_100000_TARGETS);
        ACH_CASE(TAKE_10000_HITS);
        ACH_CASE(KILL_EVERYTHING);
        ACH_CASE(KILL_ONLY_HOSTILES);
        ACH_CASE(USE_ONLY_M16);
        ACH_CASE(USE_EVERY_WEAPON);
        ACH_CASE(COMPLETE_LEVEL_ON_LOW_HEALTH);
        ACH_CASE(FIGHT_AN_OSTRICH);
        ACH_CASE(WATCH_ALL_CREDITS);
        ACH_CASE(PLAY_ON_NON_WINDOWS_PLATFORM);
        ACH_CASE(FIREBOMB_THE_BAND);
        ACH_CASE(ROCKET_TO_THE_FACE);
        ACH_CASE(KILL_A_NAKED_PERSON);
        ACH_CASE(ENABLE_CHEATS);
        ACH_CASE(COMMIT_SUICIDE);
        ACH_CASE(TOUCH_SOMEONE_WHILE_BURNING);
        ACH_CASE(COMPLETE_GAME_IN_X_MINUTES);
        ACH_CASE(COMPLETE_GAME_ON_HARDEST);
        #undef ACH_CASE
        case ACHIEVEMENT_MAX: break;  // not a real achievement, keep compiler happy.
    }

    return NULL;
}

class SteamworksEvents
{
public:
    SteamworksEvents()
        : m_CallbackUserStatsReceived(this, &SteamworksEvents::OnUserStatsReceived)
    {}

    STEAM_CALLBACK(SteamworksEvents, OnUserStatsReceived, UserStatsReceived_t /* *pParam */, m_CallbackUserStatsReceived)
    {
        //printf("STEAMWORKS: OnUserStatsReceived\n");

        if (pParam->m_nGameID != SteamAppID)
            return;
        else if (pParam->m_eResult != k_EResultOK)
            return;

        //printf("STEAMWORKS: Accepting these stats.\n");

        // Update our stats and achievements.
        int32 val;
        ISteamUserStats *stats = SteamUserStats();
        #define UPDATESTAT(st) { \
            stats->GetStat(#st, &val); \
            /*printf("STEAMWORKS: Got stat '%s' (+%d)\n", #st, (int) val);*/ \
            Stat_##st += (int) val; \
            if (Stat_##st < 0) Stat_##st = 0x7FFFFFFF; \
        }
        UPDATESTAT(BulletsFired);
        UPDATESTAT(BulletsHit);
        UPDATESTAT(BulletsMissed);
        UPDATESTAT(Deaths);
        UPDATESTAT(Suicides);
        UPDATESTAT(Executions);
        UPDATESTAT(HitsTaken);
        UPDATESTAT(DamageTaken);
        UPDATESTAT(Burns);
        UPDATESTAT(TimeRunning);
        UPDATESTAT(KilledHostiles);
        UPDATESTAT(KilledCivilians);
        UPDATESTAT(TotalKilled);
        UPDATESTAT(LevelsPlayed);
        #undef UPDATESTAT

        for (int i = 0; i < ACHIEVEMENT_MAX; i++)
        {
            const char *name = GetAchievementName((Achievement) i);
            if (!name) break;  // just in case.
            bool unlocked = false;
            if (!stats->GetAchievement(name, &unlocked))
                unlocked = false;
            //printf("STEAMWORKS: Achievement '%s': %slocked\n", name, unlocked ? "un" : "");
            EarnedSteamAchievements[i] = unlocked;
        }

        WaitingForInitialSteamStats = false;
    }
};


static bool touchFile(const char *fname, const int64 stamp)
{
#ifdef WIN32
    HANDLE hFile = CreateFileA(fname, GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                               FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    ULARGE_INTEGER val;
    val.QuadPart = (ULONGLONG) stamp;
    val.QuadPart += 11644473600LL;  // epoch difference. Ignoring leap seconds, oh well.
    val.QuadPart *= 10000000LL;  // convert to nanoseconds.
    FILETIME ft;
    ft.dwLowDateTime = val.LowPart;
    ft.dwHighDateTime = val.HighPart;
    const BOOL rc = SetFileTime(hFile, NULL, NULL, &ft);
    CloseHandle(hFile);
    return (rc != 0);
#else
    timeval ft[2];
    ft[0].tv_sec = ft[1].tv_sec = (time_t) stamp;
    ft[0].tv_usec = ft[1].tv_usec = 0;
    return (utimes(fname, ft) == 0);
#endif
}


static bool prepareSteamworks()
{
    ISteamUtils *utils;

    if ((!SteamAPI_Init()) || ((utils = SteamUtils()) == NULL))
    {
        rspMsgBox(RSP_MB_BUT_OK | RSP_MB_ICN_STOP,
                  "Error!", "%s", "Can't initialize Steamworks, aborting...");
        return false;
    }

    SteamAppID = utils->GetAppID();

    new SteamworksEvents;

    ISteamUserStats *stats = SteamUserStats();
    if ((!stats) || (rspCommandLine("nosteamachievements")))
        EnableSteamAchievements = false;

    if (EnableSteamAchievements)
    {
        bool nukeAchievements = rspCommandLine("nukesteamachievements") != 0;
        if (nukeAchievements)
        {
            if (rspMsgBox(RSP_MB_BUT_OKCANCEL | RSP_MB_ICN_QUERY, "Whoa!", "%s", "Really nuke your Steam Achievements? This can't be undone!") != RSP_MB_RET_OK)
                nukeAchievements = false;
        }

        if (nukeAchievements)
        {
            stats->ResetAllStats(true);
            stats->StoreStats();
        }

        stats->RequestCurrentStats();
    }

    ISteamRemoteStorage *cloud = SteamRemoteStorage();
    if ((!cloud) || (rspCommandLine("nosteamcloud")))
        EnableSteamCloud = false;

    if (EnableSteamCloud)
    {
        bool nukeCloud = rspCommandLine("nukesteamcloud") != 0;
        if (nukeCloud)
        {
            if (rspMsgBox(RSP_MB_BUT_OKCANCEL | RSP_MB_ICN_QUERY, "Whoa!", "%s", "Really nuke the Steam Cloud? This can't be undone!") != RSP_MB_RET_OK)
                nukeCloud = false;
        }

        if (nukeCloud)
        {
            const int Max = cloud->GetFileCount();
            for (int i = 0; i < Max; i++)
            {
                int32 fsize = 0;
                const char *fname = cloud->GetFileNameAndSize(i, &fsize);
                if (fname)
                    cloud->FileDelete(fname);
            }
        }

        // no files in the cloud? Add local ones.
        if (cloud->GetFileCount() == 0)
        {
            const int Max = MAX_SAVE_SLOTS;
            for (int i = 0; i < Max; i++)
            {
                char fname[64];
                snprintf(fname, sizeof (fname), "savegame/%d.gme", i);
                FILE *io = fopen(FindCorrectFile(fname, "rb"), "rb");
                if (io != NULL)
                {
                    char buf[1024];
                    const size_t br = fread(buf, 1, sizeof (buf), io);
                    fclose(io);
                    if (br > 0)
                    {
                        snprintf(fname, sizeof (fname), "savegame_%d.gme", i);
                        cloud->FileWrite(fname, buf, (int32) br);
                    }
                }
            }
        }

        // Copy files from the Cloud back to us.
        const int Max = MAX_SAVE_SLOTS;
        for (int i = 0; i < Max; i++)
        {
            char src[64];
            snprintf(src, sizeof (src), "savegame_%d.gme", i);
            char dst[64];
            snprintf(dst, sizeof (dst), "steamcloud/%d.gme", i);

            remove(FindCorrectFile(dst, "wb"));

            if (!cloud->FileExists(src))
                continue;

            char buf[1024];
            const int32 br = cloud->FileRead(src, buf, (int32) sizeof (buf));
            if (br <= 0)
                continue;

            FILE *io = fopen(FindCorrectFile(dst, "wb"), "wb");
            if (!io)
                continue;

            const size_t bw = fwrite(buf, (size_t) br, 1, io);
            fclose(io);
            if (bw != 1)
                remove(FindCorrectFile(dst, "wb"));
            else
            {
                const int64 stamp = cloud->GetFileTimestamp(src);
                if (stamp > 0)
                    touchFile(FindCorrectFile(dst, "wb"), stamp);
            }
        }
    }

    return true;  // good to go.
}

void RequestSteamStatsStore()
{
    if (EnableSteamAchievements)
        StoreSteamStatsPending = true;
}

void UnlockAchievement(const Achievement ach)
{
    //if ((ach < 0) || (ach >= ACHIEVEMENT_MAX))
    //    return;
    if (Flag_Achievements & FLAG_USED_CHEATS)
        return;  // denied.
    else if (EarnedSteamAchievements[ach])
        return;  // already have it.
    else if (!EnableSteamAchievements)
        return;
    else if (GetInputMode() == INPUT_MODE_PLAYBACK)
        return;   // you have to actually be playing, not a demo run.  :)

    const char *achstr = GetAchievementName(ach);
    if (!achstr)
        return;

    ISteamUserStats *stats = SteamUserStats();
    if (!stats)
        return;

    //printf("STEAMWORKS: Unlocking achievement '%s'!\n", achstr);

    EarnedSteamAchievements[ach] = true;
    stats->SetAchievement(achstr);
    RequestSteamStatsStore();
}

void RunSteamworksUpkeep()
{
    SteamAPI_RunCallbacks();

    if ((StoreSteamStatsPending) && (!WaitingForInitialSteamStats))
    {
        ISteamUserStats *stats = SteamUserStats();
        if (stats)
        {
            //printf("STEAMWORKS: Pushing stats/achievements...\n");

            // since we're pushing here anyhow, might as well update counters...
            #define SETSTAT(st) { \
                /*printf("STEAMWORKS: Storing stat '%s' (%d)\n", #st, Stat_##st);*/ \
                stats->SetStat(#st, Stat_##st); \
            }
            SETSTAT(BulletsFired);
            SETSTAT(BulletsHit);
            SETSTAT(BulletsMissed);
            SETSTAT(Deaths);
            SETSTAT(Suicides);
            SETSTAT(Executions);
            SETSTAT(HitsTaken);
            SETSTAT(DamageTaken);
            SETSTAT(Burns);
            SETSTAT(TimeRunning);
            SETSTAT(KilledHostiles);
            SETSTAT(KilledCivilians);
            SETSTAT(TotalKilled);
            SETSTAT(LevelsPlayed);
            #undef SETSTAT

            if (stats->StoreStats())
                StoreSteamStatsPending = false;
        }
    }
}
#endif


int main(int argc, char **argv)
	{
	short sResult = 0;

    _argc = argc;
    _argv = argv;

    assert_types_are_sane();
    rspPlatformInit();

    #if WITH_STEAMWORKS
    if (!prepareSteamworks())
        return 1;
    #endif

	//------------------------------------------------------------------------
	// Get hardware-related settings from prefs file
	//------------------------------------------------------------------------

	// Open the preference file.  If this file doesn't exist then we can't
	// continue (we could use defaults for video, audio, etc., but we can't
	// guess where the assets are!)  The preference file must be located in
	// the same directory as this application, and it is assumed that that
	// directory is the current directory.  This will be the case unless the
	// user does something stupid.
	RPrefs prefs;
	if (prefs.Open(g_pszPrefFileName, "rt") == 0)
		{
		// Get video preferences
		short sDeviceWidth;
		short sDeviceHeight;
		short	sUseCurrentDeviceDimensions;
		prefs.GetVal("Video", "DeviceWidth", MAIN_SCREEN_MIN_WIDTH, &sDeviceWidth);
		prefs.GetVal("Video", "DeviceHeight", MAIN_SCREEN_MIN_HEIGHT, &sDeviceHeight);
		prefs.GetVal("Video", "UseCurrentDeviceDimensions", 1, &sUseCurrentDeviceDimensions);

		// Get audio preferences
		short	sAudioSamplesPerSec;
		short	sDeviceBitsPerSample;
		short	sBufTime;
		short	sMixBitsPerSample;
		prefs.GetVal("Audio", "DeviceRate", MAIN_AUDIO_RATE, &sAudioSamplesPerSec);
		prefs.GetVal("Audio", "DeviceBits", MAIN_AUDIO_BITS, &sDeviceBitsPerSample);
		prefs.GetVal("Audio", "DeviceBufTime", MAIN_AUDIO_BUFTIME, &sBufTime);
		prefs.GetVal("Audio", "MixBits", 16, &sMixBitsPerSample);

		// Close preferences file
		prefs.Close();

		// Make sure no errors occurred
		if (prefs.IsError() == 0)
			{

			//---------------------------------------------------------------------------
			// Init blue layer
			//---------------------------------------------------------------------------
			if (rspInitBlue() == 0)
				{

// Turn on profile (if enabled via macro)
rspProfileOn();

// Set profile report file name
rspSetProfileOutput("profile.out");	

				//------------------------------------------------------------------------
				// Set system stuff
				//------------------------------------------------------------------------

				// Set app name
				rspSetApplicationName(g_pszAppName);

#if defined(_DEBUG)
				// Set mode to minimum use of CPU
				rspSetDoSystemMode(RSP_DOSYSTEM_SLEEP);
#else
				// Set mode to maximum use of CPU
				rspSetDoSystemMode(RSP_DOSYSTEM_HOGCPU);
#endif

				//------------------------------------------------------------------------
				// Setup video
				//------------------------------------------------------------------------

				sResult	= SetupVideo(				// Returns 0 on success.
					sUseCurrentDeviceDimensions,	// In:  1 to use current video area.
					sDeviceWidth,						// In:  Desired video hardware width.
					sDeviceHeight);					// In:  Desired video hardware height.

				if (sResult == 0)
					{
					// Set Win32 static colors and lock them.
					rspSetWin32StaticColors(1);
					
					//---------------------------------------------------------------
					// Setup audio
					//---------------------------------------------------------------

					// If the INI or default mode fails b/c it is incompatible with the
					// hardware, we will try vanilla settings.
					bool	bSwitchedToVanillaSettings	= false;
					// A common reason why the audio mode can't be set is that another
					// process started a sound that hasn't finished playing by the time
					// this app starts.  Therefore, it often pays to keep trying for a
					// few seconds to give that other sound time to finish.  If it still
					// doesn't work after a few seconds, we ask the user what to do.
					// He can abort (end game), retry (for another few seconds), or ignore
					// (play the game without audio).
					bool bRetry = true;

					while (bRetry)
						{
						// Keep trying until it works or time runs out, whichever comes first
						long	lTime = rspGetMilliseconds();
						bool	bDone	= false;
						do	{
							// Try to set mode
							sResult = RMix::SetMode(
								sAudioSamplesPerSec,
								sDeviceBitsPerSample,
								MAIN_AUDIO_CHANNELS,
								sBufTime,
								MAIN_AUDIO_MAXBUFTIME,
								sMixBitsPerSample,
								sMixBitsPerSample);

							switch (sResult)
								{
								case 0:
									// Alrighty.
									bDone	= true;
									break;
								case BLU_ERR_DEVICE_IN_USE:
									// Try again until timer expires.
									if ((rspGetMilliseconds() - lTime) < AUDIO_RETRY_TIME)
										{
										// Continue.
										}
									else
										{
										// Done.
										bDone	= true;
										}
									break;
								case BLU_ERR_NO_DEVICE:
									// Not much we can do about this.  Note that we'll still
									// need to be able to open the sample files to query info
									// about them (even if NO sound).  This is handled by game.cpp.
									bDone	= true;
									break;
								case BLU_ERR_NOT_SUPPORTED:
									// Trying more won't help.  Jump out of this loop so the
									// user can choose what to do.
									bDone	= true;
									break;
								}

							} while (bDone == false);

						// If it worked, clear the retry flag
						if (sResult == 0)
							{
							bRetry = false;
							}
						else
							{
							TRACE("main(): Audio didn't work, using msgbox to find out what to do...\n");
							char buf[100];
							sprintf(buf, "%.3f kHz, %hd Bit, %s",
								(float)sAudioSamplesPerSec/(float)1000,
								(short)sDeviceBitsPerSample,
								(MAIN_AUDIO_CHANNELS == 1) ? "Mono" : "Stereo");
							
							// Default to generic error.
							char*	pszMsg;
							USHORT usFlags; 
							// Try to find a better one, though, based on the return value.
							switch (sResult)
								{
								case BLU_ERR_DEVICE_IN_USE:
									pszMsg	= g_pszAudioModeInUseError_s;
									usFlags	= RSP_MB_ICN_QUERY | RSP_MB_BUT_ABORTRETRYIGNORE;
									break;
								case BLU_ERR_NO_DEVICE:
									pszMsg	= g_pszAudioModeNoDeviceError_s;
									usFlags	= RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNO;
									break;
								case BLU_ERR_NOT_SUPPORTED:
									// If we haven't already tried vanilla settings . . .
									if (bSwitchedToVanillaSettings == false)
										{
										pszMsg	= g_pszAudioModeNotSupportedError_s;
										usFlags	= RSP_MB_ICN_QUERY | RSP_MB_BUT_ABORTRETRYIGNORE;
										// Fall back on our most Vanilla mode.
										sAudioSamplesPerSec		= MAIN_VANILLA_AUDIO_RATE;
										sDeviceBitsPerSample		= MAIN_VANILLA_AUDIO_BITS;
										
										// Should we alter sMixBitsPerSample????
										// Let's not -- that way they should be able to use the
										// assets they originally installed.
										
										// Remember.
										bSwitchedToVanillaSettings	= true;
										}
									else
										{
										pszMsg	= g_pszAudioVanillaModeNotSupportedError_s;
										usFlags	= RSP_MB_ICN_QUERY | RSP_MB_BUT_YESNO;
										}
									break;
								default:
									pszMsg	= g_pszAudioModeGeneralError_s;
									usFlags	= RSP_MB_ICN_QUERY | RSP_MB_BUT_ABORTRETRYIGNORE;
									break;
								}

							short sButton = rspMsgBox(
								usFlags,
								g_pszCriticalErrorTitle,
								pszMsg,
								buf);
							switch (sButton)
								{
								case RSP_MB_RET_NO:
								case RSP_MB_RET_ABORT:
									// To abort, just clear the retry flag.  
									// Keep the error, though.
									bRetry = false;
									break;
								case RSP_MB_RET_RETRY:
									// To retry, just clear the error (not really necessary, but seems like a good thing)
									sResult = 0;
									break;
								case RSP_MB_RET_YES:
								case RSP_MB_RET_IGNORE:
									// To ignore, just clear the error and the retry flag
									sResult = 0;
									bRetry = false;
									break;
								}
							}
						}

					if (sResult == 0)
						{

						//------------------------------------------------------------
						// Run the game
						//------------------------------------------------------------

						// Hide system cursor
						rspHideMouseCursor();

						// Run the game
						TheGame();

						// Restore system cursor
						rspShowMouseCursor();
						
						// Kill audio
						RMix::KillMode();
						}

					// Kill video
					rspKillVideoMode();
					}

// Turn off profile (if enabled via macro)
rspProfileOff();

				// Kill blue layer
				rspKillBlue();
				}
			else
				{
				// Can't init blue
				TRACE("main(): Error returned by rspInitBlue()!\n");
				rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszBadBlueInit);
				}
			}
		else
			{
			// Error reading preference file
			TRACE("main(): Error reading prefs file!\n");
			rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszPrefReadError);
			}
		}
	else
		{
		// Can't open preference file
		TRACE("main(): Couldn't open prefs file: %s !\n", g_pszPrefFileName);
		rspMsgBox(RSP_MB_ICN_STOP | RSP_MB_BUT_OK, g_pszCriticalErrorTitle, g_pszPrefOpenError);
		}

    return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
/////////////////////////////////////////////////////////////////////////////////
