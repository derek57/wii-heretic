// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2008 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

// D_main.c
/*
#include <pspdebug.h>
#include <pspkernel.h>
*/
#include <SDL/SDL_mixer.h>

#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include "doomfeatures.h"
/*
#include "txt_main.h"
#include "txt_io.h"

#include "net_client.h"
*/
#include "config.h"
#include "ct_chat.h"
#include "doomdef.h"
#include "deh_main.h"
#include "d_iwad.h"
/*
#include "i_endoom.h"
#include "i_joystick.h"
*/
#include "i_sound.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "p_local.h"
#include "s_sound.h"
#include "w_main.h"
#include "v_video.h"

#include "deh_str.h"

#include "m_misc.h"

#include "c_io.h"

#define CT_KEY_GREEN    'g'
#define CT_KEY_YELLOW   'y'
#define CT_KEY_RED      'r'
#define CT_KEY_BLUE     'b'

#define STARTUP_WINDOW_X 17
#define STARTUP_WINDOW_Y 7

//#define printf pspDebugScreenPrintf

GameMode_t gamemode = indetermined;
char *gamedescription = "unknown";

boolean demos_are_disabled = true;
boolean do_not_repeat_music = false;

boolean nomonsters;             // checkparm of -nomonsters
boolean respawnparm;            // checkparm of -respawn
boolean fastparm;               // checkparm of -fast

boolean start_respawnparm;
boolean start_fastparm;

extern boolean debugmode /*= false*/;	// checkparm of -debug
/*
boolean ravpic;                 // checkparm of -ravpic
boolean cdrom;                  // true if cd-rom mode active
*/
boolean singletics;             // debug flag to cancel adaptiveness
boolean noartiskip;             // whether shift-enter skips an artifact

skill_t startskill;
int startepisode;
int startmap;
int UpdateState;
/*
static int graphical_startup = 0;	// FIXME:	ON THE WII IT SHOWS THE TEXT SDL SCREEN,
					//		BUT THEN NOTHING MORE WHILE THE GAME DOES RUN!
static boolean using_graphical_startup;
*/
static boolean main_loop_started = false;
boolean autostart;
extern boolean automapactive;

boolean advancedemo;

//static int show_endoom = 0;

void D_ConnectNetGame(void);
void D_CheckNetGame(void);
void D_PageDrawer(void);
void D_AdvanceDemo(void);
boolean F_Responder(event_t * ev);

boolean			HERETIC_BETA = false;
boolean			HERETIC_SHARE_1_0 = false;
boolean			HERETIC_SHARE_1_2 = false;
boolean			HERETIC_REG_1_0 = false;
boolean			HERETIC_REG_1_2 = false;
boolean			HERETIC_REG_1_3 = false;

extern int warped;
extern int mus_engine;

int		fsize = 0;
int		fsizerw = 0;
int		resource_wad_exists = 0;

static void CreateSavePath(void);

//---------------------------------------------------------------------------
//
// PROC D_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain.
//
//---------------------------------------------------------------------------

void D_ProcessEvents(void)
{
    event_t *ev;

    while ((ev = D_PopEvent()) != NULL)
    {
        if (F_Responder(ev))
        {
            continue;
        }
        if (MN_Responder(ev))
        {
            continue;
        }
        if (C_Responder(ev))
        {
            continue;
        }
        G_Responder(ev);
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMessage
//
//---------------------------------------------------------------------------

void DrawMessage(void)
{
    player_t *player;

    player = &players[consoleplayer];
    if (player->messageTics <= 0 || !player->message)
    {                           // No message
        return;
    }
    MN_DrTextA(player->message, 160 - MN_TextAWidth(player->message) / 2, 1);
}

//---------------------------------------------------------------------------
//
// PROC D_Display
//
// Draw current display, possibly wiping it from the previous.
//
//---------------------------------------------------------------------------

void R_ExecuteSetViewSize(void);

extern boolean finalestage;

void D_Display(void)
{
    extern boolean askforquit;

    // Change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
    }

//
// do buffered drawing
//
    switch (gamestate)
    {
        case GS_LEVEL:
            if (!gametic)
                break;
            if (automapactive)
                AM_Drawer();
            else
                R_RenderPlayerView(&players[displayplayer]);
//            CT_Drawer();
            UpdateState |= I_FULLVIEW;
            SB_Drawer();

	    if(warped == 1)
		paused = true;

            break;
        case GS_INTERMISSION:
            IN_Drawer();
            break;
        case GS_FINALE:
            F_Drawer();
            break;
        case GS_DEMOSCREEN:
            D_PageDrawer();
            break;

        case GS_CONSOLE:
            break;
    }
/*
    if (testcontrols)
    {
        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }
*/
    if (paused && !MenuActive && !askforquit)
    {
//        if (!netgame)
        {
/*
            V_DrawPatch(160, viewwindowy + 5, W_CacheLumpName(DEH_String("PAUSED"),		// CHANGED FOR HIRES
                                                              PU_CACHE));			// CHANGED FOR HIRES
*/
            V_DrawPatch(160, (viewwindowy >> hires) + 5, W_CacheLumpName(DEH_String("PAUSED"),	// CHANGED FOR HIRES
                                                              PU_CACHE));			// CHANGED FOR HIRES
        }
/*
        else
        {
            V_DrawPatch(160, 70, W_CacheLumpName(DEH_String("PAUSED"), PU_CACHE));
        }
*/
    }
    // Handle player messages
    DrawMessage();

    C_Drawer();

    // Menu drawing
    MN_Drawer();

    // Send out any new accumulation
    NetUpdate();

    // Flush buffered stuff to screen
    I_FinishUpdate();
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // when menu is active or game is paused, release the mouse

    if (MenuActive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}

/*static*/ void I_SDL_PollMusic(void);

//---------------------------------------------------------------------------
//
// PROC D_DoomLoop
//
//---------------------------------------------------------------------------

void D_DoomLoop(void)
{

//    if (M_CheckParm("-debugfile"))
//    if(debugmode)
	if(usb)
	    debugfile = fopen("usb:/apps/wiiheretic/debug.txt","w");
	else if(sd)
	    debugfile = fopen("sd:/apps/wiiheretic/debug.txt","w");
/*
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
*/
    I_InitGraphics();

    main_loop_started = true;

    while (1)
    {
	// check if the OGG music stopped playing
	if(usergame && gamestate != GS_DEMOSCREEN && gamestate != GS_CONSOLE)
	    I_SDL_PollMusic();

        // Frame syncronous IO operations
        I_StartFrame();

        // Process one or more tics
        // Will run at least one tic
        TryRunTics();

        // Move positional sounds
        S_UpdateSounds(players[consoleplayer].mo);
        D_Display();
    }
}

/*
===============================================================================

						DEMO LOOP

===============================================================================
*/

int demosequence;
int pagetic;
char *pagename;


/*
================
=
= D_PageTicker
=
= Handles timing for warped projection
=
================
*/

void D_PageTicker(void)
{
    if (--pagetic < 0)
        D_AdvanceDemo();
}


/*
================
=
= D_PageDrawer
=
================
*/

void D_PageDrawer(void)
{
    if(HERETIC_BETA || HERETIC_SHARE_1_0)
    {
	V_DrawPatch(0, 0, W_CacheLumpName(pagename, PU_CACHE));

	if(HERETIC_BETA)
	    MN_DrTextB("BETA", 260, 160);
    }
    else
	V_DrawRawScreen(W_CacheLumpName(pagename, PU_CACHE));

    if (demosequence == 1)
    {
        V_DrawPatch(4, 160, W_CacheLumpName(DEH_String("ADVISOR"), PU_CACHE));
    }
    UpdateState |= I_FULLSCRN;
}

/*
=================
=
= D_AdvanceDemo
=
= Called after each demo or intro demosequence finishes
=================
*/

void D_AdvanceDemo(void)
{
    advancedemo = true;
}

void D_DoAdvanceDemo(void)
{
    players[consoleplayer].playerstate = PST_LIVE;      // don't reborn
    advancedemo = false;
    usergame = false;           // can't save / end game here
    paused = false;
    gameaction = ga_nothing;
    demosequence = (demosequence + 1) % 7;
    switch (demosequence)
    {
        case 0:
            pagetic = 210;
            gamestate = GS_DEMOSCREEN;
	    pagename = DEH_String("TITLE");
	    if(demos_are_disabled && !do_not_repeat_music)
	    {
#ifdef OGG_SUPPORT
		if(opl)
#endif
		    S_StartSong(mus_titl, false);
		    do_not_repeat_music = true;
#ifdef OGG_SUPPORT
		else
		    S_StartMP3Music(2, 0); 
#endif
	    }
            break;
        case 1:
            pagetic = 140;
            gamestate = GS_DEMOSCREEN;
	    pagename = DEH_String("TITLE");
            break;
        case 2:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
//            G_DeferedPlayDemo(DEH_String("demo1"));
            break;
        case 3:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            pagename = DEH_String("CREDIT");
            break;
        case 4:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
//            G_DeferedPlayDemo(DEH_String("demo2"));
            break;
        case 5:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            if (gamemode == shareware)
            {
                pagename = DEH_String("ORDER");
            }
            else
            {
                pagename = DEH_String("CREDIT");
            }
            break;
        case 6:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
//            G_DeferedPlayDemo(DEH_String("demo3"));
            break;
    }
}


/*
=================
=
= D_StartTitle
=
=================
*/

void D_StartTitle(void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo();
}


/*
==============
=
= D_CheckRecordFrom
=
= -recordfrom <savegame num> <demoname>
==============

void D_CheckRecordFrom(void)
{
    int p;
    char *filename;

    //!
    // @vanilla
    // @category demo
    // @arg <savenum> <demofile>
    //
    // Record a demo, loading from the given filename. Equivalent
    // to -loadgame <savenum> -record <demofile>.

    p = M_CheckParmWithArgs("-recordfrom", 2);
    if (!p)
        return;

    filename = SV_Filename(myargv[p + 1][0] - '0');
    G_LoadGame(filename);
    G_DoLoadGame();             // load the gameskill etc info from savegame

    G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p + 2]);
    D_DoomLoop();               // never returns
    free(filename);
}

===============
=
= D_AddFile
=
===============
*/

// MAPDIR should be defined as the directory that holds development maps
// for the -wart # # command

#define MAPDIR "\\data\\"

#define SHAREWAREWADNAME "heretic1.wad"

char *iwadfile;
/*
char *basedefault = "heretic.cfg";

void wadprintf(void)
{
    if (debugmode)
    {
        return;
    }
    // haleyjd FIXME: convert to textscreen code?
#ifdef __WATCOMC__
    _settextposition(23, 2);
    _setbkcolor(1);
    _settextcolor(0);
    _outtext(exrnwads);
    _settextposition(24, 2);
    _outtext(exrnwads2);
#endif
}
*/
boolean D_AddFile(char *file)
{
    wad_file_t *handle;

    if(debugmode && load_extra_wad == 1)
	printf("  adding %s\n", file);

    handle = W_AddFile(file);

    return handle != NULL;
}

//==========================================================
//
//  Startup Thermo code
//
//==========================================================
#define MSG_Y       9
#define THERM_X     14
#define THERM_Y     14

int thermMax;
int thermCurrent;
char smsg[80];                  // status bar line

//
//  Heretic startup screen shit
//
/*
static int startup_line = STARTUP_WINDOW_Y;

void hprintf(char *string)
{
    if (using_graphical_startup)
    {
        TXT_BGColor(TXT_COLOR_CYAN, 0);
        TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

        TXT_GotoXY(STARTUP_WINDOW_X, startup_line);
        ++startup_line;
        TXT_Puts(string);

        TXT_UpdateScreen();
    }

    // haleyjd: shouldn't be WATCOMC-only
    if (debugmode)
        puts(string);
}

void drawstatus(void)
{
    int i;

    TXT_GotoXY(1, 24);
    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    for (i=0; smsg[i] != '\0'; ++i) 
    {
        TXT_PutChar(smsg[i]);
    }
}

void status(char *string)
{
    if (using_graphical_startup)
    {
        strcat(smsg, string);
        drawstatus();
    }
}

void DrawThermo(void)
{
    static int last_progress = -1;
    int progress;
    int i;

    if (!using_graphical_startup)
    {
        return;
    }

#if 0
    progress = (98 * thermCurrent) / thermMax;
    screen = (char *) 0xb8000 + (THERM_Y * 160 + THERM_X * 2);
    for (i = 0; i < progress / 2; i++)
    {
        switch (i)
        {
            case 4:
            case 9:
            case 14:
            case 19:
            case 29:
            case 34:
            case 39:
            case 44:
                *screen++ = 0xb3;
                *screen++ = (THERMCOLOR << 4) + 15;
                break;
            case 24:
                *screen++ = 0xba;
                *screen++ = (THERMCOLOR << 4) + 15;
                break;
            default:
                *screen++ = 0xdb;
                *screen++ = 0x40 + THERMCOLOR;
                break;
        }
    }
    if (progress & 1)
    {
        *screen++ = 0xdd;
        *screen++ = 0x40 + THERMCOLOR;
    }
#else

    // No progress? Don't update the screen.

    progress = (50 * thermCurrent) / thermMax + 2;

    if (last_progress == progress)
    {
        return;
    }

    last_progress = progress;

    TXT_GotoXY(THERM_X, THERM_Y);

    TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
    TXT_BGColor(TXT_COLOR_GREEN, 0);

    for (i = 0; i < progress; i++)
    {
        TXT_PutChar(0xdb);
    }

    TXT_UpdateScreen();
#endif
}

void initStartup(void)
{
    byte *textScreen;
    byte *loading;

    if (!graphical_startup || debugmode || testcontrols)
    {
        using_graphical_startup = false;
        return;
    }

    if (!TXT_Init()) 
    {
        using_graphical_startup = false;
        return;
    }

    I_InitWindowTitle();
    I_InitWindowIcon();

    // Blit main screen
    textScreen = TXT_GetScreenData();
    loading = W_CacheLumpName(DEH_String("LOADING"), PU_CACHE);
    memcpy(textScreen, loading, 4000);

    // Print version string

    TXT_BGColor(TXT_COLOR_RED, 0);
    TXT_FGColor(TXT_COLOR_YELLOW);
    TXT_GotoXY(46, 2);
    TXT_Puts(HERETIC_VERSION_TEXT);

    TXT_UpdateScreen();

    using_graphical_startup = true;
}

static void finishStartup(void)
{
    if (using_graphical_startup)
    {
        TXT_Shutdown();
    }
}

char tmsg[300];
void tprintf(char *msg, int initflag)
{
    // haleyjd FIXME: convert to textscreen code?
#ifdef __WATCOMC__
    char temp[80];
    int start;
    int add;
    int i;

    if (initflag)
        tmsg[0] = 0;
    strcat(tmsg, msg);
    blitStartup();
    DrawThermo();
    _setbkcolor(4);
    _settextcolor(15);
    for (add = start = i = 0; i <= strlen(tmsg); i++)
        if ((tmsg[i] == '\n') || (!tmsg[i]))
        {
            memset(temp, 0, 80);
            strncpy(temp, tmsg + start, i - start);
            _settextposition(MSG_Y + add, 40 - strlen(temp) / 2);
            _outtext(temp);
            start = i + 1;
            add++;
        }
    _settextposition(25, 1);
    drawstatus();
#else
    printf("%s", msg);
#endif
}

// haleyjd: moved up, removed WATCOMC code
void CleanExit(void)
{
    if(debugmode)
	DEH_printf("Exited from HERETIC.\n");
    exit(1);
}

void CheckAbortStartup(void)
{
    // haleyjd: removed WATCOMC
    // haleyjd FIXME: this should actually work in text mode too, but how to
    // get input before SDL video init?
    if(using_graphical_startup)
    {
        if(TXT_GetChar() == 27)
            CleanExit();
    }
}

void IncThermo(void)
{
    thermCurrent++;
    DrawThermo();
//    CheckAbortStartup();
}

void InitThermo(int max)
{
    thermMax = max;
    thermCurrent = 0;
}
*/
//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
/*
    extern int screenblocks;
    extern int snd_Channels;
    int i;

    M_ApplyPlatformDefaults();

    I_BindVideoVariables();
    I_BindJoystickVariables();
*/
    I_BindSoundVariables();

    M_BindBaseControls();
/*
    M_BindHereticControls();
    M_BindWeaponControls();
    M_BindChatControls(MAXPLAYERS);

    key_multi_msgplayer[0] = CT_KEY_GREEN;
    key_multi_msgplayer[1] = CT_KEY_YELLOW;
    key_multi_msgplayer[2] = CT_KEY_RED;
    key_multi_msgplayer[3] = CT_KEY_BLUE;

    M_BindMenuControls();
    M_BindMapControls();

    M_BindVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindVariable("sfx_volume",             &snd_MaxVolume);
    M_BindVariable("music_volume",           &snd_MusicVolume);
    M_BindVariable("screenblocks",           &screenblocks);
    M_BindVariable("snd_channels",           &snd_Channels);
    M_BindVariable("show_endoom",            &show_endoom);
    M_BindVariable("graphical_startup",      &graphical_startup);

    for (i=0; i<10; ++i)
    {
        char buf[12];

        sprintf(buf, "chatmacro%i", i);
        M_BindVariable(buf, &chat_macros[i]);
    }
*/
}

// 
// Called at exit to display the ENDOOM screen (ENDTEXT in Heretic)
//
/*
static void D_Endoom(void)
{
    byte *endoom_data;

    // Disable ENDOOM?

    if (!show_endoom || testcontrols || !main_loop_started)
    {
        return;
    }

    endoom_data = W_CacheLumpName(DEH_String("ENDTEXT"), PU_STATIC);

    I_Endoom(endoom_data);
}
*/
//---------------------------------------------------------------------------
//
// PROC D_DoomMain
//
//---------------------------------------------------------------------------

void W_CheckSize(/*int wad*/void);
void I_QuitSerialFail (void);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

void D_DoomMain(void)
{
    FILE *fprw;
/*
    GameMission_t gamemission = heretic;

    int p;
    char file[256];

    I_PrintBanner(PACKAGE_STRING);
*/
    if(debugmode)
	fsize = 14189976;

//    W_CheckSize(0);

    if(usb)
	fprw = fopen("usb:/apps/wiiheretic/pspheretic.wad","rb");
    else if(sd)
	fprw = fopen("sd:/apps/wiiheretic/pspheretic.wad","rb");

    if(fprw)
    {
	resource_wad_exists = 1;

	fclose(fprw);

	W_CheckSize(/*2*/);
    }
    else
    {
    	resource_wad_exists = 0;
    }

    if(print_resource_pwad_error)
    {
	printf("\n\n\n\n\n");
	printf(" ===============================================================================");
	printf("                         !!! WRONG RESOURCE PWAD FILE !!!                       ");
	printf("                  PLEASE COPY THE WAD 'PSPHERETIC.WAD' THAT CAME                ");
	printf("                    WITH THIS RELEASE, INTO THE GAME DIRECTORY                  \n");
   	printf("                                                                                \n");
	printf("                                 QUITTING NOW ...                               ");
	printf(" ===============================================================================");

	sleep(5);
//	sceKernelDelayThread(5000*1000);

	I_QuitSerialFail();
    }

    if (resource_wad_exists == 0)
    {
	printf("\n\n\n\n\n");
	printf(" ===============================================================================");
	printf("             WARNING: RESOURCE WAD FILE 'PSPHERETIC.WAD' MISSING!!!             ");
	printf("               PLEASE COPY THIS FILE INTO THE GAME'S DIRECTORY!!!               \n");
	printf("                                                                                \n");
	printf("                                QUITTING NOW ...                                ");
	printf(" ===============================================================================");

	sleep(5);
//	sceKernelDelayThread(5000*1000);

	I_QuitSerialFail();
    }

//    I_AtExit(D_Endoom, false);

    if (fsize != 4095992 &&
	fsize != 5120300 &&
	fsize != 5120920 &&
	fsize != 11096488 &&
	fsize != 11095516 &&
	fsize != 14189976)
    {
	printf("\n\n\n\n\n");
	printf(" ===============================================================================");
	printf("       !!! HERETIC MAIN IWAD FILE MISSING, NOT SELECTED OR WRONG IWAD !!!       \n");
	printf("                                                                                \n");
	printf("                                QUITTING NOW ...                                ");
	printf(" ============================================================================== ");

//	sceKernelDelayThread(5000*1000);
	sleep(5);

	I_QuitSerialFail();
    }

    if(fsize == 4095992)
    {
	HERETIC_BETA = true;
        gamemode = shareware;
    }
    else if(fsize == 5120300)
    {
	HERETIC_SHARE_1_0 = true;
        gamemode = shareware;
    }
    else if(fsize == 5120920)
    {
	HERETIC_SHARE_1_2 = true;
        gamemode = shareware;
    }
    else if(fsize == 11096488)
    {
	HERETIC_REG_1_0 = true;
        gamemode = registered;
    }
    else if(fsize == 11095516)
    {
	HERETIC_REG_1_2 = true;
        gamemode = registered;
    }
    else if(fsize == 14189976)
    {
	HERETIC_REG_1_3 = true;
        gamemode = retail;
    }
    else
	fsize = 0;
/*
    pspDebugScreenSetTextColor(0xD0D0D0);	// grey
    pspDebugScreenSetBackColor(0x000000);	// black
*/
    printf("\n");
    printf("C:\\HERETIC>HERETIC.EXE_\n");
    printf("DOS/4GW Professional Protected Mode Run-time  Version 1.97\n");
    printf("Copyright (c) Rational Systems, Inc. 1990-1994\n");

    //!
    // @vanilla
    //
    // Disable monsters.
    //
/*
    nomonsters = M_ParmExists("-nomonsters");

    //!
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_ParmExists("-respawn");

    //!
    // @vanilla
    //
    // Monsters move faster.
    //

    fastparm = M_ParmExists("-fast");

    //!
    // @vanilla
    //
    // Take screenshots when F1 is pressed.
    //

    ravpic = M_ParmExists("-ravpic");

    //!
    // @vanilla
    //
    // Allow artifacts to be used when the run key is held down.
    //

    noartiskip = M_ParmExists("-noartiskip");

    debugmode = M_ParmExists("-debug");
*/
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;

    if(debugmode)
	autostart = true;
    else
	autostart = false;

//
// get skill / episode / map from parms
//

    //!
    // @vanilla
    // @category net
    //
    // Start a deathmatch game.
    //
/*
    if (M_ParmExists("-deathmatch"))
    {
        deathmatch = true;
    }

    //!
    // @arg <skill>
    // @vanilla
    //
    // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
    // 0 disables all monsters.
    //

    p = M_CheckParmWithArgs("-skill", 1);
    if (p)
    {
        startskill = myargv[p + 1][0] - '1';
        autostart = true;
    }

    //!
    // @arg <n>
    // @vanilla
    //
    // Start playing on episode n (1-4)
    //

    p = M_CheckParmWithArgs("-episode", 1);
    if (p)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = 1;
        autostart = true;
    }

    //!
    // @arg <x> <y>
    // @vanilla
    //
    // Start a game immediately, warping to level ExMy.
    //

    p = M_CheckParmWithArgs("-warp", 2);
    if (p && p < myargc - 2)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = myargv[p + 2][0] - '0';
        autostart = true;
    }
*/
//
// init subsystems
//
    DEH_printf("V_Init: allocate screens.\n");
    V_Init();

    // Check for -CDROM
/*
    cdrom = false;

#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\heretic.cd,
    // allowing play from CD.
    //

    if (M_CheckParm("-cdrom"))
    {
        cdrom = true;
    }
#endif

    if (cdrom)
    {
        M_SetConfigDir(DEH_String("c:\\heretic.cd"));
    }
    else
*/
    {
        M_SetConfigDir(NULL);
    }

    // Load defaults before initing other systems
    DEH_printf("M_LoadDefaults: Load system defaults.\n");
    D_BindVariables();
    M_SetConfigFilenames("heretic.cfg"/*, PROGRAM_PREFIX "heretic.cfg"*/);
    M_LoadDefaults();

    respawnparm = false;
    fastparm = false;

    if(mus_engine > 1)
	mus_engine = 2;
    else if(mus_engine < 2)
	mus_engine = 1;

    if(mus_engine == 1)
	snd_musicdevice = SNDDEVICE_SB;
    else
	snd_musicdevice = SNDDEVICE_GENMIDI;

    I_AtExit(M_SaveDefaults, false);

    DEH_printf("Z_Init: Init zone memory allocation daemon.\n");
    Z_Init();

    C_Init();

    printf("DPMI memory: 0x3c9e000, 0x800000 allocated for zone\n");

#ifdef FEATURE_DEHACKED
//    printf("DEH_Init: Init Dehacked support.\n");
    DEH_Init();
#endif

    DEH_printf("W_Init: Init WADfiles.\n");
/*
    iwadfile = D_FindIWAD(IWAD_MASK_HERETIC, &gamemission);

    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate. No IWAD was found. Try specifying\n"
                "one with the '-iwad' command line parameter.");
    }
*/
    if(debugmode)
    {
	if(usb)
	    D_AddFile("usb:/apps/wiiheretic/IWAD/Reg13/HERETIC.WAD");
	else if(sd)
	    D_AddFile("sd:/apps/wiiheretic/IWAD/Reg13/HERETIC.WAD");
    }
    else
	D_AddFile(target);

    if(usb)
	D_AddFile("usb:/apps/wiiheretic/pspheretic.wad");
    else if(sd)
	D_AddFile("sd:/apps/wiiheretic/pspheretic.wad");

    if (debugmode && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	printf("  adding pspheretic.wad\n");

    if ((HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3) && load_extra_wad == 1)
    {
	if(extra_wad_slot_1_loaded == 1)
	    D_AddFile(extra_wad_1);

	if(extra_wad_slot_2_loaded == 1)
	    D_AddFile(extra_wad_2);

	if(extra_wad_slot_3_loaded == 1)
	    D_AddFile(extra_wad_3);
    }

//    W_ParseCommandLine();

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //
/*
    p = M_CheckParmWithArgs("-playdemo", 1);
    if (!p)
    {
        //!
        // @arg <demo>
        // @category demo
        // @vanilla
        //
        // Play back the demo named demo.lmp, determining the framerate
        // of the screen.
        //

        p = M_CheckParmWithArgs("-timedemo", 1);
    }

    if (p)
    {
        DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p + 1]);
        D_AddFile(file);
        DEH_printf("Playing demo %s.lmp.\n", myargv[p + 1]);
    }

    if (W_CheckNumForName(DEH_String("E2M1")) == -1)
    {
        gamemode = shareware;
        gamedescription = "Heretic (shareware)";
    }
    
    if (W_CheckNumForName("EXTENDED") != -1)
    {
        // Presence of the EXTENDED lump indicates the retail version

        gamemode = retail;
        gamedescription = "Heretic: Shadow of the Serpent Riders";
    }
    else
    {
        gamemode = registered;
        gamedescription = "Heretic (registered)";
    }

    I_SetWindowTitle(gamedescription);

    savegamedir = M_GetSaveGameDir(D_SaveGameIWADName(gamemission));
*/
    savegamedir = M_GetSaveGameDir("heretic.wad");

//    printf("savegamedir: %s\n",savegamedir);	// ONLY FOR DEBUGGING

    // Now that the savedir is loaded from .CFG, make sure it exists

    CreateSavePath();
/*
    I_PrintStartupBanner(gamedescription);

    if (M_ParmExists("-testcontrols"))
    {
        startepisode = 1;
        startmap = 1;
        autostart = true;
        testcontrols = true;
    }

#ifdef FEATURE_MULTIPLAYER
    tprintf("NET_Init: Init network subsystem.\n", 1);
    NET_Init ();
#endif
*/
    I_InitTimer();
    D_ConnectNetGame();

    start_respawnparm = respawnparm;
    start_fastparm = fastparm;
/*
    // haleyjd: removed WATCOMC
    initStartup();

    //
    //  Build status bar line!
    //
    smsg[0] = 0;

    if (deathmatch)
        status(DEH_String("DeathMatch..."));

    if (nomonsters)
        status(DEH_String("No Monsters..."));
    if (respawnparm)
        status(DEH_String("Respawning..."));
    if (fastparm)
        status(DEH_String("Fast Monsters..."));

    if (autostart)
    {
        char temp[64];
        DEH_snprintf(temp, sizeof(temp),
                     "Warp to Episode %d, Map %d, Skill %d ",
                     startepisode, startmap, startskill + 1);
        status(temp);
    }

    wadprintf();                // print the added wadfiles

    DEH_printf("MN_Init: Init menu system.\n");
*/
    MN_Init();
/*
    CT_Init();
    if(debugmode)
	DEH_printf("R_Init: Init Heretic refresh daemon.");
*/
	printf("Loading graphics");
//    hprintf(DEH_String("Loading graphics"));

    R_Init();

    printf("\n");
/*
    if(debugmode)
	DEH_printf("P_Init: Init Playloop state.\n");

	printf("Init game engine.");
    hprintf(DEH_String("Init game engine."));
*/
    P_Init();
//    IncThermo();

    DEH_printf("I_Init: Setting up machine state.\n");
/*
    I_CheckIsScreensaver();
    I_InitJoystick();

    IncThermo();

    DEH_printf("S_Init: Setting up sound.\n");
*/
    S_Init();

#ifdef OGG_SUPPORT
    Mix_VolumeMusic(snd_MusicVolume + 25);
    SDL_InitOGG();
#endif

    //IO_StartupTimer();
    S_Start();

    DEH_printf("Checking network game status.\n");
/*
    hprintf(DEH_String("Checking network game status."));
    printf("Checking network game status.");
*/
    D_CheckNetGame();
/*
    IncThermo();

    // haleyjd: removed WATCOMC

    DEH_printf("SB_Init: Loading patches.\n");
*/
    SB_Init();
/*
    IncThermo();

//
// start the apropriate game based on parms
//

    D_CheckRecordFrom();

    //!
    // @arg <x>
    // @category demo
    // @vanilla
    //
    // Record a demo named x.lmp.
    //

    p = M_CheckParmWithArgs("-record", 1);
    if (p)
    {
        G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
        singledemo = true;      // Quit after one demo
        G_DeferedPlayDemo(myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo(myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    //!
    // @arg <s>
    // @vanilla
    //
    // Load the game in savegame slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    if (p && p < myargc - 1)
    {
        char *filename;

	filename = SV_Filename(myargv[p + 1][0] - '0');
        G_LoadGame(filename);
	free(filename);
    }
*/
    // Check valid episode and map
    if (autostart /*|| netgame*/)
    {
        if (!D_ValidEpisodeMap(heretic, gamemode, startepisode, startmap))
        {
            startepisode = 1;
            startmap = 1;
        }
    }

    if (gameaction != ga_loadgame)
    {
        UpdateState |= I_FULLSCRN;
        BorderNeedRefresh = true;
        if (autostart /*|| netgame*/)
        {
            G_InitNew(startskill, startepisode, startmap);
        }
        else
        {
            D_StartTitle();
        }
    }

//    finishStartup();

    D_DoomLoop();               // Never returns
}

//==========================================================================
//
// CreateSavePath
//
//==========================================================================

static void CreateSavePath(void)
{
    char creationPath[121];

//    creationRoot[120] = 0;

    if(usb)
    {
	if(HERETIC_BETA)
	    strcpy(creationPath, SavePathBetaUSB);

	if(HERETIC_SHARE_1_0)
	    strcpy(creationPath, SavePathShare10USB);

	if(HERETIC_SHARE_1_2)
	    strcpy(creationPath, SavePathShare12USB);

	if(HERETIC_REG_1_0)
	    strcpy(creationPath, SavePathReg10USB);

	if(HERETIC_REG_1_2)
	    strcpy(creationPath, SavePathReg12USB);

	if(HERETIC_REG_1_3)
	    strcpy(creationPath, SavePathReg13USB);
    }
    else if(sd)
    {
	if(HERETIC_BETA)
	    strcpy(creationPath, SavePathBetaSD);

	if(HERETIC_SHARE_1_0)
	    strcpy(creationPath, SavePathShare10SD);

	if(HERETIC_SHARE_1_2)
	    strcpy(creationPath, SavePathShare12SD);

	if(HERETIC_REG_1_0)
	    strcpy(creationPath, SavePathReg10SD);

	if(HERETIC_REG_1_2)
	    strcpy(creationPath, SavePathReg12SD);

	if(HERETIC_REG_1_3)
	    strcpy(creationPath, SavePathReg13SD);
    }

    M_MakeDirectory(creationPath);
}

