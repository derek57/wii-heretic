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

// MN_menu.c
/*
#include <psppower.h>
#include <pspsysmem_kernel.h>
*/
#include <SDL/SDL_mixer.h>

#include <ctype.h>

#include "deh_str.h"
#include "doomdef.h"
#include "doomkeys.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_controls.h"
#include "p_local.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_video.h"

#include "doomfeatures.h"

// Macros

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN     16 + 1 + 5 + 2
#define ASCII_CURSOR '['

#define ITEM_HEIGHT_SMALL 10
#define SELECTOR_XOFFSET_SMALL (-15)
#define SELECTOR_YOFFSET_SMALL (1)
#define FIRSTKEY_MAX	0

// Types

typedef enum
{
    ITT_EMPTY,
    ITT_EFUNC,
    ITT_LRFUNC,
    ITT_SETMENU,
    ITT_INERT,
    ITT_SETKEY
} ItemType_t;

typedef enum
{
    MENU_MAIN,
    MENU_EPISODE,
    MENU_SKILL,
    MENU_OPTIONS,
    MENU_SCREEN,
    MENU_CONTROL,
    MENU_BINDINGS,
    MENU_SOUND,
    MENU_SYSTEM,
    MENU_GAME,
    MENU_DEBUG,
    MENU_KEYS,
    MENU_ARMOR,
    MENU_WEAPONS,
    MENU_ARTIFACTS,
    MENU_CHEATS,
    MENU_FILES,
    MENU_LOAD,
    MENU_SAVE,
    MENU_RECORD,
    MENU_NONE
} MenuType_t;

typedef struct
{
    ItemType_t type;
    char *text;
    void/*boolean*/(*func) (int option);
    int option;
    MenuType_t menu;
} MenuItem_t;

typedef struct
{
    int x;
    int y;
    void (*drawFunc) (void);
    int itemCount;
    MenuItem_t *items;
    int oldItPos;
    MenuType_t prevMenu;
} Menu_t;

// Private Functions

static void InitFonts(void);
static void SetMenu(MenuType_t menu);
static boolean SCNetCheck(int option);

static void SCNetCheck2(int option);

static void/*boolean*/ SCQuitGame(int option);
static void/*boolean*/ SCEpisode(int option);
static void/*boolean*/ SCSkill(int option);
//static void boolean SCMouseSensi(int option);
//static void SCMouseSensi(int option);
static void/*boolean*/ SCSfxVolume(int option);
static void/*boolean*/ SCMusicVolume(int option);
static void/*boolean*/ SCScreenSize(int option);
static void/*boolean*/ SCLoadGame(int option);
static void/*boolean*/ SCSaveGame(int option);
static void/*boolean*/ SCMessages(int option);
static void/*boolean*/ SCEndGame(int option);
static void/*boolean*/ SCInfo(int option);

static void SCWalkingSpeed(int option);
static void SCTurningSpeed(int option);
static void SCStrafingSpeed(int option);
static void SCMouseSpeed(int option);
static void SCNTrack(int option);
//static void SCNTrackLoop(int option);
static void SCWarp(int option);
static void SCWarpNow(int option);
static void SCBrightness(int option);
static void SCDetails(int option);
static void SCWeaponChange(int option);
static void SCCrosshair(int option);
static void SCJumping(int option);
static void SCStats(int option);
static void SCMouselook(int option);

static void SCArtifact(int option);
static void SCQuartz(int option);
static void SCUrn(int option);
static void SCTorch(int option);
static void SCOvum(int option);
static void SCWings(int option);
static void SCRing(int option);
static void SCSphere(int option);
static void SCTimebomb(int option);
static void SCTome(int option);
static void SCChaosDev(int option);
static void SCGod(int option);
static void SCNoclip(int option);
//static void SCCpu(int option);
static void SCTicker(int option);
static void SCFPS(int option);
static void SCWiiLight(int option);
static void SCAllArm(int option);
static void SCSilver(int option);
static void SCShield(int option);
static void SCWeaponA(int option);
static void SCWeaponB(int option);
static void SCWeaponC(int option);
static void SCWeaponD(int option);
static void SCWeaponE(int option);
static void SCWeaponF(int option);
static void SCWeaponG(int option);
static void SCAmmo(int option);
static void SCKeys(int option);
static void SCYellow(int option);
static void SCGreen(int option);
static void SCBlue(int option);
static void SCMap(int option);
static void SCKill(int option);
static void SCChicken(int option);
static void SCIDDQD(int option);
static void SCIDKFA(int option);
static void SCHealth(int option);
static void SCRSkill(int option);
static void SCRMap(int option);
static void SCRecord(int option);
static void SCTimer(int option);
/*
static void SCOther(int option);
static void SCProcessor(int option);
static void SCShowMemory(int option);
static void SCBattery(int option);
*/
static void SCVersion(int option);
#ifdef OGG_SUPPORT
static void SCMusicType(int option);
#endif
static void SCSound(int option);
static void SCCoords(int option);
static void SCMapname(int option);
static void SCGrid(int option);
static void SCFollow(int option);
static void SCRotate(int option);

//static void ButtonLayout(int option);
static void SCSetKey(int option);
static void ClearKeys(int option);
static void ResetKeys(int option);

static void DrawMainMenu(void);
static void DrawEpisodeMenu(void);
static void DrawSkillMenu(void);
static void DrawOptionsMenu(void);
//static void DrawOptions2Menu(void);
static void DrawFileSlots(Menu_t * menu);
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void DrawLoadMenu(void);
static void DrawSaveMenu(void);
static void DrawSlider(Menu_t * menu, int item, int width, int slot);
void MN_LoadSlotText(void);

static void DrawScreenMenu(void);
static void DrawControlMenu(void);
static void DrawBindingsMenu(void);
static void DrawSoundMenu(void);
static void DrawSystemMenu(void);
static void DrawGameMenu(void);
static void DrawDebugMenu(void);
static void DrawKeysMenu(void);
static void DrawArmorMenu(void);
static void DrawWeaponsMenu(void);
static void DrawArtifactsMenu(void);
static void DrawCheatsMenu(void);
static void DrawRecordMenu(void);

// External Data

extern int detailLevel;
extern int screenblocks;

// Public Data

boolean MenuActive;
int InfoType;
boolean messageson;

extern boolean gamekeydown[256];        // The NUMKEYS macro is local to g_game

// Private Data

static int FontABaseLump;
static int FontBBaseLump;
static int SkullBaseLump;
static Menu_t *CurrentMenu;
static int CurrentItPos;
static int MenuEpisode;
static int MenuTime;
static boolean soundchanged;

boolean askforquit;
boolean askforsave;
/*static*/ int typeofask;
/*static*/ int typeofask2;
static boolean FileMenuKeySteal;
static boolean slottextloaded;
static char SlotText[6][SLOTTEXTLEN + 2];
static char oldSlotText[SLOTTEXTLEN + 2];
static int SlotStatus[6];
static int slotptr;
static int currentSlot;
/*
static int quicksave;
static int quickload;
*/

#define BETA_FLASH_TEXT "BETA"
//#define PRESS_BUTTON	"RESTART: PRESS JUMP"

extern default_t doom_defaults_list[];

extern boolean am_rotate;

static boolean askforkey = false;

static int FirstKey = 0;
static int keyaskedfor;

boolean loop = true;
boolean forced = false;
boolean fake = false;

int ninty;
int FramesPerSecond;
int timer;
int version;
int mapname;
int coords;
int followplayer = 1;
int drawgrid;
int crosshair = 0;
int tracknum = 1;
int map = 1;
int continuous=0;
int cheeting;
int suicide = 0;
int ran = 0;
int warped = 0;
int repisode = 1;
int rmap = 1;
int rskill = 0;

int key_bindings_start_in_cfg_at_pos = 19;
int key_bindings_end_in_cfg_at_pos = 35;
/*
int memory_info = 0;
int battery_info = 0;
int cpu_info = 0;
int other_info = 0;
int max_free_ram = 0;
*/
int epi = 1;
int repi = 1;
int mouseSensitivity;

//int mhz333;
int button_layout = 0;

int show_stats = 0;
/*
char unit_plugged_textbuffer[50];
char battery_present_textbuffer[50];
char battery_charging_textbuffer[50];
char battery_charging_status_textbuffer[50];
char battery_low_textbuffer[50];
char battery_lifetime_percent_textbuffer[50];
char battery_lifetime_int_textbuffer[50];
char battery_temp_textbuffer[50];
char battery_voltage_textbuffer[50];
char processor_clock_textbuffer[50];
char processor_bus_textbuffer[50];
char idle_time_textbuffer[50];
char allocated_ram_textbuffer[50];
char free_ram_textbuffer[50];
char max_free_ram_textbuffer[50];
*/
extern int mouselook;
extern int cheating;
//extern int allocated_ram_size;

//int selected = 0;
//int selectedx = 0;
//int actualmusvolume;

int opl = 1;
int wiilight = 1;
int faketracknum = 1;

fixed_t         forwardmove = 29;
fixed_t         sidemove = 21; 

int turnspeed = 7;

char *songtext[] = {
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "08",
    "09",
    "10",
    "11",
    "12",
    "13",
    " ",
    "14",
    "15",
    "16",
    "17",
    " ",
    "18",
    "19",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    "20",
    "21",
    "22"
};

char *songtextbeta[] = {
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "08",
    "09",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    "10",
    "11",
    "12"
};

#ifdef OGG_SUPPORT
char *songtextmp3[] = {
    "01",
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "08",
    "09",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22"
};

char *songtextbetamp3[] = {
    "01",
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "08",
    "09",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    "10",
    "11",
    "12"
};
#endif

char *mlooktext[] = {
    "OFF",
    "NORMAL",
    "INVERSE"
};

char *maptext[] = {
    " ",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E1M1: THE DOCKS",
    "E1M2: THE DUNGEONS",
    "E1M3: THE GATEHOUSE",
    "E1M4: THE GUARD TOWER",
    "E1M5: THE CITADEL",
    "E1M6: THE CATHEDRAL",
    "E1M7: THE CRYPTS",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E1M8: HELL'S MAW",
    "E1M9: THE GRAVEYARD",
    "E2M1: THE CRATER",
    "E2M2: THE LAVA PITS",
    "E2M3: THE RIVER OF FIRE",
    "E2M4: THE ICE GROTTO",
    "E2M5: THE CATACOMBS",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E2M6: THE LABYRINTH",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E2M7: THE GREAT HALL",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E2M8: THE PORTALS OF CHAOS",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E2M9: THE GLACIER",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E3M1: THE STORE HOUSE",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E3M2: THE CESSPOOL",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E3M3: THE CONFLUENCE",
    "E3M4: THE AZURE FORTRESS",
    "E3M5: THE OPHIDIAN LAIR",
    "E3M6: THE HALLS OF FEAR",
    "E3M7: THE CHASM",
    "E3M8: D'SPARIL'S KEEP",
    "E3M9: THE AQUIFER",
    "E4M1: CATAFALQUE",
    "E4M2: BLOCKHOUSE",	// additional but in game non selectable REQUIRED entry (MAP 0 DOESN'T EXIST IN IWAD)
    "E4M3: AMBULATORY",
    "E4M4: SEPULCHER",
    "E4M5: GREAT STAIR",
    "E4M6: HALLS OF THE APOSTATE",
    "E4M7: RAMPARTS OF PERDITION",
    "E4M8: SHATTERED BRIDGE",
    "E4M9: MAUSOLEUM",
    "E5M1: OCHRE CLIFFS",
    "E5M2: RAPIDS",
    "E5M3: QUAY",
    "E5M4: COURTYARD",
    "E5M5: HYDRATYR",
    "E5M6: COLONNADE",
    "E5M7: FOETID MANSE",
    "E5M8: FIELD OF JUDGEMENT",
    "E5M9: SKEIN OF D'SPARIL",
    "E6M1: RAVEN'S LAIR",
    "E6M2: WATER SHRINE",
    "E6M3: AMERICAN'S LEGACY"
};

static MenuItem_t MainItems[] = {
//    {ITT_EFUNC, "NEW GAME", SCNetCheck, 1, MENU_EPISODE},
    {ITT_SETMENU, "NEW GAME", SCNetCheck2, 1, MENU_EPISODE},
    {ITT_SETMENU, "OPTIONS", NULL, 0, MENU_OPTIONS},
    {ITT_SETMENU, "GAME FILES", NULL, 0, MENU_FILES},
    {ITT_EFUNC, "INFO", SCInfo, 0, MENU_NONE},
    {ITT_EFUNC, "QUIT GAME", SCQuitGame, 0, MENU_NONE}
};

static Menu_t MainMenu = {
    110, 56,
    DrawMainMenu,
    5, MainItems,
    0,
    MENU_NONE
};

static MenuItem_t EpisodeItems[] = {
    {ITT_EFUNC, "CITY OF THE DAMNED", SCEpisode, 1, MENU_NONE},
    {ITT_EFUNC, "HELL'S MAW", SCEpisode, 2, MENU_NONE},
    {ITT_EFUNC, "THE DOME OF D'SPARIL", SCEpisode, 3, MENU_NONE},
    {ITT_EFUNC, "THE OSSUARY", SCEpisode, 4, MENU_NONE},
    {ITT_EFUNC, "THE STAGNANT DEMESNE", SCEpisode, 5, MENU_NONE}
};

static Menu_t EpisodeMenu = {
    80, 50,
    DrawEpisodeMenu,
    3, EpisodeItems,
    0,
    MENU_MAIN
};

static MenuItem_t FilesItems[] = {
//    {ITT_EFUNC, "LOAD GAME", SCNetCheck, 2, MENU_LOAD},
    {ITT_SETMENU, "LOAD GAME", SCNetCheck2, 2, MENU_LOAD},
    {ITT_SETMENU, "SAVE GAME", NULL, 0, MENU_SAVE},
    {ITT_EFUNC, "END GAME", SCEndGame, 0, MENU_NONE},
    {ITT_SETMENU, "CHEATS", NULL, 0, MENU_CHEATS},
    {ITT_SETMENU, "DEMO REC.", NULL, 0, MENU_RECORD}
};

static Menu_t FilesMenu = {
    110, 60,
    DrawFilesMenu,
    5, FilesItems,
    0,
    MENU_MAIN
};

static MenuItem_t LoadItems[] = {
    {ITT_EFUNC, NULL, SCLoadGame, 0, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 1, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 2, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 3, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 4, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 5, MENU_NONE}
};

static Menu_t LoadMenu = {
    70, 30,
    DrawLoadMenu,
    6, LoadItems,
    0,
    MENU_FILES
};

static MenuItem_t SaveItems[] = {
    {ITT_EFUNC, NULL, SCSaveGame, 0, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 1, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 2, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 3, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 4, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 5, MENU_NONE}
};

static Menu_t SaveMenu = {
    70, 30,
    DrawSaveMenu,
    6, SaveItems,
    0,
    MENU_FILES
};

static MenuItem_t SkillItems[] = {
    {ITT_EFUNC, "THOU NEEDETH A WET-NURSE", SCSkill, sk_baby, MENU_NONE},
    {ITT_EFUNC, "YELLOWBELLIES-R-US", SCSkill, sk_easy, MENU_NONE},
    {ITT_EFUNC, "BRINGEST THEM ONETH", SCSkill, sk_medium, MENU_NONE},
    {ITT_EFUNC, "THOU ART A SMITE-MEISTER", SCSkill, sk_hard, MENU_NONE},
    {ITT_EFUNC, "BLACK PLAGUE POSSESSES THEE", SCSkill, sk_nightmare, MENU_NONE}
};

static Menu_t SkillMenu = {
    38, 30,
    DrawSkillMenu,
    5, SkillItems,
    2,
    MENU_EPISODE
};

static MenuItem_t OptionsItems[] = {
    {ITT_SETMENU, "SCREEN SETTINGS", NULL, 0, MENU_SCREEN},
    {ITT_SETMENU, "CONTROL SETTINGS", NULL, 0, MENU_CONTROL},
    {ITT_SETMENU, "SOUND SETTINGS", NULL, 0, MENU_SOUND},
    {ITT_SETMENU, "SYSTEM SETTINGS", NULL, 0, MENU_SYSTEM},
    {ITT_SETMENU, "GAME SETTINGS", NULL, 0, MENU_GAME},
    {ITT_SETMENU, "DEBUG SETTINGS", NULL, 0, MENU_DEBUG}
};

static Menu_t OptionsMenu = {
    88, 30,
    DrawOptionsMenu,
    6, OptionsItems,
    0,
    MENU_MAIN
};
/*
static MenuItem_t Options2Items[] = {
    {ITT_LRFUNC, "SCREEN SIZE", SCScreenSize, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "SFX VOLUME", SCSfxVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME", SCMusicVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE}
};

static Menu_t Options2Menu = {
    90, 20,
    DrawOptions2Menu,
    6, Options2Items,
    0,
    MENU_OPTIONS
};
*/

static MenuItem_t ScreenItems[] = {
    {ITT_LRFUNC, "BRIGHTNESS", SCBrightness, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "SCREEN SIZE", SCScreenSize, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_EFUNC, "DETAILS", SCDetails, 0, MENU_NONE}/*,
    {ITT_LRFUNC, "SFX VOLUME", SCSfxVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME", SCMusicVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE}
*/
};

static Menu_t ScreenMenu = {
    100, 40,
    DrawScreenMenu,
    5, ScreenItems,
    0,
    MENU_SCREEN
};

static MenuItem_t ControlItems[] = {
    {ITT_LRFUNC, "", SCWalkingSpeed, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "", SCTurningSpeed, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "", SCStrafingSpeed, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "", SCMouselook, 0, MENU_NONE},
    {ITT_LRFUNC, "", SCMouseSpeed, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_SETMENU, "", NULL, 0, MENU_BINDINGS}
};

static Menu_t ControlMenu = {
    38, 0,
    DrawControlMenu,
    10, ControlItems,
    0,
    MENU_CONTROL
};

static MenuItem_t BindingsItems[] = {
// see defaults[] in m_misc.c for the correct option number:
// key_right corresponds to defaults[3], which means that we
// are using the (index_number - 3) here.
//
    {ITT_SETKEY, "FIRE", SCSetKey, 0, MENU_NONE},
    {ITT_SETKEY, "USE / OPEN", SCSetKey, 1, MENU_NONE},
    {ITT_SETKEY, "MAIN MENU", SCSetKey, 2, MENU_NONE},
    {ITT_SETKEY, "WEAPON LEFT", SCSetKey, 3, MENU_NONE},
    {ITT_SETKEY, "SHOW AUTOMAP", SCSetKey, 4, MENU_NONE},
    {ITT_SETKEY, "WEAPON RIGHT", SCSetKey, 5, MENU_NONE},
    {ITT_SETKEY, "AUTOMAP ZOOM IN", SCSetKey, 6, MENU_NONE},
    {ITT_SETKEY, "AUTOMAP ZOOM OUT", SCSetKey, 7, MENU_NONE},
    {ITT_SETKEY, "INVENTORY LEFT", SCSetKey, 8, MENU_NONE},
    {ITT_SETKEY, "INVENTORY RIGHT", SCSetKey, 9, MENU_NONE},
    {ITT_SETKEY, "INVENTORY USE", SCSetKey, 10, MENU_NONE},
    {ITT_SETKEY, "FLY UP", SCSetKey, 11, MENU_NONE},
    {ITT_SETKEY, "FLY DOWN", SCSetKey, 12, MENU_NONE},
    {ITT_SETKEY, "LOOK CENTER", SCSetKey, 13, MENU_NONE},
    {ITT_SETKEY, "JUMP", SCSetKey, 14, MENU_NONE},
    {ITT_SETKEY, "RUN", SCSetKey, 15, MENU_NONE},
//    {ITT_EMPTY, NULL, NULL, 11, MENU_NONE},
//    {ITT_LRFUNC, "BUTTON LAYOUT :", ButtonLayout, 12, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 16, MENU_NONE},
    {ITT_SETKEY, "CLEAR ALL CONTROLS", ClearKeys, 17, MENU_NONE},
    {ITT_SETKEY, "RESET TO DEFAULTS", ResetKeys, 18, MENU_NONE}
};

static Menu_t BindingsMenu = {
    40, 5,
    DrawBindingsMenu,
    19, BindingsItems,
    0,
    MENU_BINDINGS
};

static MenuItem_t SoundItems[] = {
    {ITT_LRFUNC, "SFX VOLUME", SCSfxVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME", SCMusicVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "CHOOSE TRACK :", SCNTrack, 0, MENU_NONE}
#ifdef OGG_SUPPORT
    ,
    {ITT_LRFUNC, "MUSIC TYPE :", SCMusicType, 0, MENU_NONE}
#endif
    /*,{ITT_LRFUNC, "LOOP TRACK :", SCNTrackLoop, 0, MENU_NONE}*/
};

static Menu_t SoundMenu = {
    75, 30,
    DrawSoundMenu,
#ifdef OGG_SUPPORT
    6, SoundItems,
#else
    5, SoundItems,
#endif
    0,
    MENU_SOUND
};

static MenuItem_t SystemItems[] = {
//    {ITT_EFUNC, "CPU SPEED :", SCCpu, 0, MENU_NONE},
    {ITT_EFUNC, "DISPLAY TICKER", SCTicker, 0, MENU_NONE},
    {ITT_EFUNC, "FPS COUNTER", SCFPS, 0, MENU_NONE},
    {ITT_EFUNC, "WII LIGHT FLASHING", SCWiiLight, 0, MENU_NONE}
};

static Menu_t SystemMenu = {
    70, 60,
    DrawSystemMenu,
    3, SystemItems,
    0,
    MENU_SYSTEM
};

static MenuItem_t GameItems[] = {
    {ITT_EFUNC, "MAP GRID", SCGrid, 0, MENU_NONE},
    {ITT_EFUNC, "FOLLOW MODE", SCFollow, 0, MENU_NONE},
    {ITT_LRFUNC, "ROTATION", SCRotate, 0, MENU_NONE},
    {ITT_EFUNC, "STATISTICS", SCStats, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "MESSAGES", SCMessages, 0, MENU_NONE},
    {ITT_EFUNC, "CROSSHAIR", SCCrosshair, 0, MENU_NONE},
    {ITT_EFUNC, "JUMPING", SCJumping, 0, MENU_NONE},
    {ITT_EFUNC, "WEAPON CHANGE", SCWeaponChange, 0, MENU_NONE}
};

static Menu_t GameMenu = {
    50, 10,
    DrawGameMenu,
    9, GameItems,
    0,
    MENU_GAME
};

static MenuItem_t DebugItems[] = {
    {ITT_EFUNC, "COORDINATES INFO", SCCoords, 0, MENU_NONE},
    {ITT_EFUNC, "MAP NAME INFO", SCMapname, 0, MENU_NONE},
    {ITT_EFUNC, "SOUND INFO", SCSound, 0, MENU_NONE},
    {ITT_EFUNC, "TIMER INFO", SCTimer, 0, MENU_NONE},
    {ITT_EFUNC, "VERSION INFO", SCVersion, 0, MENU_NONE}/*,
    {ITT_EFUNC, "BATTERY INFO", SCBattery, 0, MENU_NONE},
    {ITT_EFUNC, "CPU INFO", SCProcessor, 0, MENU_NONE},
    {ITT_EFUNC, "MEMORY INFO", SCShowMemory, 0, MENU_NONE},
    {ITT_EFUNC, "OTHER INFO", SCOther, 0, MENU_NONE}*/
};

static Menu_t DebugMenu = {
    65, 50,
    DrawDebugMenu,
    5, DebugItems,
    0,
    MENU_DEBUG
};

static MenuItem_t KeysItems[] = {
    {ITT_EFUNC, "GIVE THEM ALL AT ONCE", SCKeys, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "YELLOW KEY", SCYellow, 0, MENU_NONE},
    {ITT_EFUNC, "GREEN KEY", SCGreen, 0, MENU_NONE},
    {ITT_EFUNC, "BLUE KEY", SCBlue, 0, MENU_NONE},
};

static MenuItem_t ArmorItems[] = {
    {ITT_EFUNC, "GIVE THEM ALL AT ONCE", SCAllArm, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "SILVER SHIELD", SCSilver, 0, MENU_NONE},
    {ITT_EFUNC, "ENCHANTED SHIELD", SCShield, 0, MENU_NONE}
};

static MenuItem_t WeaponsItems[] = {
    {ITT_EFUNC, "GIVE THEM ALL AT ONCE", SCWeaponA, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "GIVE AMMO", SCAmmo, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "GAUNTLETS OF THE NECROMANCER", SCWeaponG, 0, MENU_NONE},
    {ITT_EFUNC, "ETHEREAL CROSSBOW", SCWeaponB, 0, MENU_NONE},
    {ITT_EFUNC, "DRAGON CLAW", SCWeaponC, 0, MENU_NONE},
    {ITT_EFUNC, "HELLSTAFF", SCWeaponD, 0, MENU_NONE},
    {ITT_EFUNC, "PHOENIX ROD", SCWeaponE, 0, MENU_NONE},
    {ITT_EFUNC, "FIREMACE", SCWeaponF, 0, MENU_NONE}
};

static MenuItem_t ArtifactsItems[] = {
    {ITT_EFUNC, "GIVE THEM ALL AT ONCE", SCArtifact, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "QUARTZ FLASK", SCQuartz, 0, MENU_NONE},
    {ITT_EFUNC, "TORCH", SCTorch, 0, MENU_NONE},
    {ITT_EFUNC, "MORPH OVUM", SCOvum, 0, MENU_NONE},
    {ITT_EFUNC, "WINGS OF WRATH", SCWings, 0, MENU_NONE},
    {ITT_EFUNC, "RING OF INVINCIBILITY", SCRing, 0, MENU_NONE},
    {ITT_EFUNC, "SHADOWSPHERE", SCSphere, 0, MENU_NONE},
    {ITT_EFUNC, "TIMEBOMB OF THE ANCIENTS", SCTimebomb, 0, MENU_NONE},
    {ITT_EFUNC, "TOME OF POWER", SCTome, 0, MENU_NONE},
    {ITT_EFUNC, "CHAOS DEVICE", SCChaosDev, 0, MENU_NONE},
    {ITT_EFUNC, "MYSTIC URN", SCUrn, 0, MENU_NONE}
};

static MenuItem_t CheatsItems[] = {
    {ITT_EFUNC, "GOD MODE", SCGod, 0, MENU_NONE},
    {ITT_EFUNC, "GIVE HEALTH", SCHealth, 0, MENU_NONE},
    {ITT_SETMENU, "GIVE ARMOR...", NULL, 0, MENU_ARMOR},
    {ITT_SETMENU, "GIVE WEAPONS...", NULL, 0, MENU_WEAPONS},
    {ITT_SETMENU, "GIVE ARTIFACTS...", NULL, 0, MENU_ARTIFACTS},
    {ITT_SETMENU, "GIVE KEYS...", NULL, 0, MENU_KEYS},
    {ITT_EFUNC, "AUTOMAP REVEAL :", SCMap, 0, MENU_NONE},
    {},
    {ITT_LRFUNC, "WARP TO MAP :", SCWarp, 0, MENU_NONE},
    {},
    {},
    {ITT_EFUNC, "EXECUTE WARPING", SCWarpNow, 0, MENU_NONE},
    {},
    {ITT_EFUNC, "KILL ALL ENEMIES", SCKill, 0, MENU_NONE},
    {ITT_EFUNC, "NOCLIP", SCNoclip, 0, MENU_NONE},
    {ITT_EFUNC, "MORPH TO A CHICKEN", SCChicken, 0, MENU_NONE},
    {ITT_EFUNC, "IDDQD", SCIDDQD, 0, MENU_NONE},
    {ITT_EFUNC, "IDKFA", SCIDKFA, 0, MENU_NONE}
};

static MenuItem_t RecordItems[] = {
    {ITT_LRFUNC, "CHOOSE MAP :", SCRMap, 0, MENU_NONE},
    {},
    {},
    {ITT_LRFUNC, "CHOOSE SKILL :", SCRSkill, 0, MENU_NONE},
    {ITT_EFUNC, "START RECORDING", SCRecord, 0, MENU_NONE}
};

static Menu_t KeysMenu = {
    100, 60,
    DrawKeysMenu,
    5, KeysItems,
    0,
    MENU_KEYS
};

static Menu_t ArmorMenu = {
    90, 70,
    DrawArmorMenu,
    4, ArmorItems,
    0,
    MENU_ARMOR
};

static Menu_t WeaponsMenu = {
    70, 50,
    DrawWeaponsMenu,
    10, WeaponsItems,
    0,
    MENU_WEAPONS
};

static Menu_t ArtifactsMenu = {
    80, 30,
    DrawArtifactsMenu,
    12, ArtifactsItems,
    0,
    MENU_ARTIFACTS
};

static Menu_t CheatsMenu = {
    70, 10,
    DrawCheatsMenu,
    18, CheatsItems,
    0,
    MENU_CHEATS
};

static Menu_t RecordMenu = {
    70, 50,
    DrawRecordMenu,
    5, RecordItems,
    0,
    MENU_RECORD
};

static Menu_t *Menus[] = {
    &MainMenu,
    &EpisodeMenu,
    &SkillMenu,
    &OptionsMenu,
    &ScreenMenu,
    &ControlMenu,
    &BindingsMenu,
    &SoundMenu,
    &SystemMenu,
    &GameMenu,
    &DebugMenu,
    &KeysMenu,
    &ArmorMenu,
    &WeaponsMenu,
    &ArtifactsMenu,
    &CheatsMenu,
    &FilesMenu,
    &LoadMenu,
    &SaveMenu,
    &RecordMenu
};

static char *GammaText[] = {
    TXT_GAMMA_LEVEL_OFF,
    TXT_GAMMA_LEVEL_1,
    TXT_GAMMA_LEVEL_2,
    TXT_GAMMA_LEVEL_3,
    TXT_GAMMA_LEVEL_4
};

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
    InitFonts();
    MenuActive = false;
    messageson = true;
    SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

    if (gamemode == retail)
    {                           // Add episodes 4 and 5 to the menu
        EpisodeMenu.itemCount = 5;
        EpisodeMenu.y -= ITEM_HEIGHT;
    }

    if(!debugmode)
    {
	OptionsMenu.itemCount = 5;
	CurrentItPos = 0;
    }
}

//---------------------------------------------------------------------------
//
// PROC InitFonts
//
//---------------------------------------------------------------------------

static void InitFonts(void)
{
    FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
    FontBBaseLump = W_GetNumForName(DEH_String("FONTB_S")) + 1;
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextA
//
// Draw text using font A.
//
//---------------------------------------------------------------------------

void MN_DrTextA(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
            V_DrawPatch(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextAWidth
//
// Returns the pixel width of a string using font A.
//
//---------------------------------------------------------------------------

int MN_TextAWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextB
//
// Draw text using font B.
//
//---------------------------------------------------------------------------

void MN_DrTextB(char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(FontBBaseLump + c - 33, PU_CACHE);
            V_DrawPatch(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextBWidth
//
// Returns the pixel width of a string using font B.
//
//---------------------------------------------------------------------------

int MN_TextBWidth(char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontBBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_Ticker
//
//---------------------------------------------------------------------------

void MN_Ticker(void)
{
    if (MenuActive == false)
    {
        return;
    }
    MenuTime++;
}

//---------------------------------------------------------------------------
//
// PROC MN_Drawer
//
//---------------------------------------------------------------------------

char *QuitEndMsg[] = {
    "ARE YOU SURE YOU WANT TO QUIT?",
    "ARE YOU SURE YOU WANT TO END THE GAME?"/*,
    "DO YOU WANT TO QUICKSAVE THE GAME NAMED",
    "DO YOU WANT TO QUICKLOAD THE GAME NAMED"
*/
};

char *NotPlaying[] = {
    "YOU CAN'T SAVE IF YOU AREN'T PLAYING!",
};

#include <wiiuse/wpad.h>
#include <SDL/SDL.h>

//extern int		display_fps;
char			fpsDisplay[100];
int			fps = 0;		// FOR PSP: calculating the frames per second

u64 GetTicks(void)
{
    return (u64)SDL_GetTicks();
}

void FPS(int FramesPerSecond)
{
    int tickfreq = 1000;

    static int fpsframecount = 0;
    static u64 fpsticks;

    fpsframecount++;

    if(GetTicks() >= fpsticks + tickfreq)
    {
	fps = fpsframecount;
	fpsframecount = 0;
	fpsticks = GetTicks();
    }
    sprintf( fpsDisplay, "FPS: %d", fps );

    if(FramesPerSecond)
    {
	MN_DrTextA(fpsDisplay, 0, 30);
    }
}

void MN_Drawer(void)
{
    int i;
    int x;
    int y;
    MenuItem_t *item;
    char *message;
    char *selName;
/*
    if(other_info)
    	MN_DrTextA(idle_time_textbuffer, 0, 180);

    if(memory_info)
    {
    	MN_DrTextA(allocated_ram_textbuffer, 0, 40);
    	MN_DrTextA(free_ram_textbuffer, 0, 50);
    	MN_DrTextA(max_free_ram_textbuffer, 0, 60);
    }

    if(battery_info)
    {
  	MN_DrTextA(unit_plugged_textbuffer, 0, 70);
    	MN_DrTextA(battery_present_textbuffer, 0, 80);
    	MN_DrTextA(battery_charging_textbuffer, 0, 90);
    	MN_DrTextA(battery_charging_status_textbuffer, 0, 100);
    	MN_DrTextA(battery_low_textbuffer, 0, 110);
    	MN_DrTextA(battery_lifetime_percent_textbuffer, 0, 120);
    	MN_DrTextA(battery_lifetime_int_textbuffer, 0, 130);
    	MN_DrTextA(battery_temp_textbuffer, 0, 140);
    	MN_DrTextA(battery_voltage_textbuffer, 0, 150);
    }

    if(cpu_info)
    {
    	MN_DrTextA(processor_clock_textbuffer, 0, 160);
    	MN_DrTextA(processor_bus_textbuffer, 0, 170);
    }
*/
    if(FramesPerSecond==1)
    {
	FPS(1);
    }
    else if(FramesPerSecond==0)
    {
	FPS(0);
    }

	// Beta blinker ***
    if(HERETIC_BETA && !MenuActive && !askforquit && leveltime & 16 && gamestate == GS_LEVEL)
    {
	MN_DrTextA(BETA_FLASH_TEXT,295-(MN_TextAWidth(BETA_FLASH_TEXT)>>1), 5);	// DISPLAYS BETA MESSAGE
    }

    if(HERETIC_BETA && MenuActive)
    {
	MN_DrTextA(" ", 295, 5);						// DISABLES MESSAGE
    }

    if(!askforquit && gamestate == GS_LEVEL)
    {
	char textBuffer[50];
	char date[50];

	if(mapname==1)
	{
	    level_name = LevelNames[(gameepisode - 1) * 9 + gamemap - 1];
	    MN_DrTextA(DEH_String(level_name), 20, 145);
	}

	if(timer==1)
		DrawWorldTimer();						// DISPLAYS GAME TIME

	if(coords==1)
	{
	    if(gamestate == GS_LEVEL)
	    {
		static player_t* player;
		player = &players[consoleplayer];

    		sprintf(textBuffer, "MAP: E%dM%d - X:%5d - Y:%5d - Z:%5d",
			gameepisode,
			gamemap,
			player->mo->x >> FRACBITS,
			player->mo->y >> FRACBITS,
			player->mo->z >> FRACBITS);

        	MN_DrTextA(textBuffer, 40, 24);
    	    }
	}
	if(version==1)
	{
		sprintf(date,"%d-%02d-%02d", YEAR, MONTH + 1, DAY);
		MN_DrTextA(HERETIC_VERSION_TEXT,55, 36);			// DISPLAYS VERSION
		MN_DrTextA(date, 195, 36);					// DISPLAYS THE DATE
	}
    }

    if (MenuActive == false)
    {
        if (askforquit)
        {
            message = DEH_String(QuitEndMsg[typeofask - 1]);

            MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2, 80);
/*
            if (typeofask == 3)
            {
                MN_DrTextA(SlotText[quicksave - 1], 160 -
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
                MN_DrTextA(DEH_String("?"), 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
            }
            if (typeofask == 4)
            {
                MN_DrTextA(SlotText[quickload - 1], 160 -
                           MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
                MN_DrTextA(DEH_String("?"), 160 +
                           MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
            }
*/
            UpdateState |= I_FULLSCRN;
        }

	if(askforsave)
	{
            message = DEH_String(NotPlaying[typeofask2 - 1]);

            MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2, 80);

            UpdateState |= I_FULLSCRN;
	}
        return;
    }
    else
    {
        UpdateState |= I_FULLSCRN;
        if (InfoType)
        {
            MN_DrawInfo();
            return;
        }
        if (screenblocks < 10)
        {
            BorderNeedRefresh = true;
        }
        if (CurrentMenu->drawFunc != NULL)
        {
            CurrentMenu->drawFunc();
        }
        x = CurrentMenu->x;
        y = CurrentMenu->y;
        item = CurrentMenu->items;

	if (item->type == ITT_SETKEY)
	    item += FirstKey;

        for (i = 0; i < CurrentMenu->itemCount; i++)
        {
            if (item->type != ITT_EMPTY && item->text)
            {
		if(CurrentMenu==&ArmorMenu	||
		   CurrentMenu==&CheatsMenu	||
		   CurrentMenu==&ArtifactsMenu	||
		   CurrentMenu==&KeysMenu	||
		   CurrentMenu==&WeaponsMenu	||
		   CurrentMenu==&BindingsMenu)
			MN_DrTextA(item->text, x, y);
		else if(CurrentMenu)
		    MN_DrTextB(item->text, x, y);
            }
	    if (CurrentMenu==&ArmorMenu		||
		CurrentMenu==&CheatsMenu	||
		CurrentMenu==&ArtifactsMenu	||
		CurrentMenu==&KeysMenu		||
		CurrentMenu==&WeaponsMenu	||
		CurrentMenu==&BindingsMenu)
			y += ITEM_HEIGHT_SMALL;
	    else if(CurrentMenu)
		y += ITEM_HEIGHT;

            item++;
        }
	if(CurrentMenu==&ArmorMenu		||
	   CurrentMenu==&CheatsMenu		||
	   CurrentMenu==&ArtifactsMenu		||
	   CurrentMenu==&KeysMenu		||
	   CurrentMenu==&WeaponsMenu		||
	   CurrentMenu==&BindingsMenu)
	{
	    y = CurrentMenu->y+(CurrentItPos*ITEM_HEIGHT_SMALL)+SELECTOR_YOFFSET_SMALL;
	    selName = MenuTime&16 ? "M_SLCTR3" : "M_SLCTR4";
	    V_DrawPatch(x+SELECTOR_XOFFSET_SMALL, y, 
		W_CacheLumpName(selName, PU_CACHE));
	}
	else if(CurrentMenu)
	{
	    y = CurrentMenu->y+(CurrentItPos*ITEM_HEIGHT)+SELECTOR_YOFFSET;
	    selName = MenuTime&16 ? "M_SLCTR1" : "M_SLCTR2";
	    V_DrawPatch(x+SELECTOR_XOFFSET, y, 
		W_CacheLumpName(selName, PU_CACHE));
	}
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMainMenu
//
//---------------------------------------------------------------------------

static void DrawMainMenu(void)
{
    int frame;

    frame = (MenuTime / 3) % 18;
    V_DrawPatch(88, 0, W_CacheLumpName(DEH_String("M_HTIC"), PU_CACHE));
    V_DrawPatch(40, 10, W_CacheLumpNum(SkullBaseLump + (17 - frame),
                                       PU_CACHE));
    V_DrawPatch(232, 10, W_CacheLumpNum(SkullBaseLump + frame, PU_CACHE));
}

//---------------------------------------------------------------------------
//
// PROC DrawEpisodeMenu
//
//---------------------------------------------------------------------------

static void DrawEpisodeMenu(void)
{
}

//---------------------------------------------------------------------------
//
// PROC DrawSkillMenu
//
//---------------------------------------------------------------------------

static void DrawSkillMenu(void)
{
}

//---------------------------------------------------------------------------
//
// PROC DrawFilesMenu
//
//---------------------------------------------------------------------------

static void DrawFilesMenu(void)
{
// clear out the quicksave/quickload stuff
/*
    quicksave = 0;
    quickload = 0;
*/
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageTics = 1;

    if(debugmode)
	FilesMenu.itemCount = 5;
    else if(!debugmode)
	FilesMenu.itemCount = 4;
}

//---------------------------------------------------------------------------
//
// PROC DrawLoadMenu
//
//---------------------------------------------------------------------------

static void DrawLoadMenu(void)
{
    char *title;

    title = DEH_String("LOAD GAME");

    MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&LoadMenu);

    MN_DrTextA("* INDICATES A SAVEGAME THAT WAS", 55, 151);
    MN_DrTextA("CREATED USING AN OPTIONAL PWAD!", 55, 161);

    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

//---------------------------------------------------------------------------
//
// PROC DrawSaveMenu
//
//---------------------------------------------------------------------------

static void DrawSaveMenu(void)
{
    char *title;

    if(!usergame || demoplayback || gamestate == GS_FINALE || gamestate == GS_INTERMISSION)
    {
	MenuActive = false;
	askforsave = true;
	typeofask2 = 1;              //not in a game
	paused = false;
	return;
    }

    title = DEH_String("SAVE GAME");

    MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&SaveMenu);

    MN_DrTextA("* INDICATES A SAVEGAME THAT WAS", 55, 151);
    MN_DrTextA("CREATED USING AN OPTIONAL PWAD!", 55, 161);

    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

//===========================================================================
//
// MN_LoadSlotText
//
//              Loads in the text message for each slot
//===========================================================================

void MN_LoadSlotText(void)
{
    FILE *fp;
    int i;
    char *filename;

    for (i = 0; i < 6; i++)
    {
        filename = SV_Filename(i);
        fp = fopen(filename, "rb+");
	free(filename);

        if (!fp)
        {
            SlotText[i][0] = 0; // empty the string
            SlotStatus[i] = 0;
            continue;
        }
        fread(&SlotText[i], SLOTTEXTLEN, 1, fp);
        fclose(fp);
        SlotStatus[i] = 1;
    }
    slottextloaded = true;
}

//---------------------------------------------------------------------------
//
// PROC DrawFileSlots
//
//---------------------------------------------------------------------------

static void DrawFileSlots(Menu_t * menu)
{
    int i;
    int x;
    int y;

    x = menu->x;
    y = menu->y;
    for (i = 0; i < 6; i++)
    {
        V_DrawPatch(x, y, W_CacheLumpName(DEH_String("M_FSLOT"), PU_CACHE));
        if (SlotStatus[i])
        {
            MN_DrTextA(SlotText[i], x + 5, y + 5);
        }
        y += ITEM_HEIGHT;
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawOptionsMenu
//
//---------------------------------------------------------------------------

static void DrawOptionsMenu(void)
{
/*
    if (messageson)
    {
        MN_DrTextB(DEH_String("ON"), 196, 50);
    }
    else
    {
        MN_DrTextB(DEH_String("OFF"), 196, 50);
    }
    DrawSlider(&OptionsMenu, 3, 10, mouseSensitivity);
*/
    if(!debugmode)
	OptionsMenu.itemCount = 5;
    else if(debugmode)
	OptionsMenu.itemCount = 6;
}

static void DrawSystemMenu(void)
{
/*
    if(mhz333)
	MN_DrTextB("333 MHZ", 190, 50);
    else
	MN_DrTextB("222 MHZ", 190, 50);
*/
    if(DisplayTicker)
	MN_DrTextB("ON", 240, 60);
    else
	MN_DrTextB("OFF", 240, 60);

    if(FramesPerSecond)
	MN_DrTextB("ON", 240, 80);
    else
	MN_DrTextB("OFF", 240, 80);

    if(wiilight)
	MN_DrTextB("ON", 240, 100);
    else
	MN_DrTextB("OFF", 240, 100);
}

static void DrawGameMenu(void)
{
    if(drawgrid == 1)
	MN_DrTextB("ON", 245, 10);
    else if(drawgrid == 0)
	MN_DrTextB("OFF", 245, 10);

    if(followplayer)
	MN_DrTextB("ON", 245, 30);
    else
	MN_DrTextB("OFF", 245, 30);

    if(am_rotate == true)
	MN_DrTextB("ON", 245, 50);
    else if(am_rotate == false)
	MN_DrTextB("OFF", 245, 50);

    if(show_stats)
	MN_DrTextB("ON", 245, 70);
    else
	MN_DrTextB("OFF", 245, 70);

	MN_DrTextB("----------------------", 50, 90);

    if(messageson)
	MN_DrTextB("ON", 245, 110);
    else
	MN_DrTextB("OFF", 245, 110);

    if(crosshair)
	MN_DrTextB("ON", 245, 130);
    else
	MN_DrTextB("OFF", 245, 130);

    if(jumping)
	MN_DrTextB("ON", 245, 150);
    else
	MN_DrTextB("OFF", 245, 150);

    if(use_vanilla_weapon_change)
	MN_DrTextB("SLOW", 245, 170);
    else
	MN_DrTextB("FAST", 245, 170);

    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

//---------------------------------------------------------------------------
//
// PROC DrawOptions2Menu
//
//---------------------------------------------------------------------------
/*
static void DrawOptions2Menu(void)
{
    DrawSlider(&Options2Menu, 1, 9, screenblocks - 3);
    DrawSlider(&Options2Menu, 3, 16, snd_MaxVolume);
    DrawSlider(&Options2Menu, 5, 16, snd_MusicVolume);
}
*/

//---------------------------------------------------------------------------
//
// PROC SCNetCheck
//
//---------------------------------------------------------------------------

static boolean SCNetCheck(int option)
{
//    if (!netgame)
    {                           // okay to go into the menu
        return true;
    }
    switch (option)
    {
        case 1:
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T START A NEW GAME IN NETPLAY!", true);
            break;
        case 2:
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T LOAD A GAME IN NETPLAY!", true);
            break;
        default:
            break;
    }
    MenuActive = false;
    return false;
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCQuitGame(int option)
{
    MenuActive = false;
    askforquit = true;
    typeofask = 1;              //quit game
    if (/*!netgame &&*/ !demoplayback)
    {
        paused = true;
    }
//    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCEndGame
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCEndGame(int option)
{
    if (demoplayback /*|| netgame*/)
    {
        return /*false*/;
    }
    MenuActive = false;
    askforquit = true;
    typeofask = 2;              //endgame
    if (/*!netgame &&*/ !demoplayback)
    {
        paused = true;
    }
//    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCMessages
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCMessages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], DEH_String("MESSAGES ON"), true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], DEH_String("MESSAGES OFF"), true);
    }
    S_StartSound(NULL, sfx_chat);
//    return true;
}

//===========================================================================
//
// SCNetCheck2
//
//===========================================================================

static void SCNetCheck2(int option)
{
    SCNetCheck(option);
    return;
}

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCLoadGame(int option)
{
    char *filename;

    if (!SlotStatus[option])
    {                           // slot's empty...don't try and load
        return /*false*/;
    }

    filename = SV_Filename(option);
    G_LoadGame(filename);
    free(filename);

    MN_DeactivateMenu();
    BorderNeedRefresh = true;
/*
    if (quickload == -1)
    {
        quickload = option + 1;
        players[consoleplayer].message = NULL;
        players[consoleplayer].messageTics = 1;
    }
    return true;
*/
}

//---------------------------------------------------------------------------
//
// PROC SCSaveGame
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCSaveGame(int option)
{
    char *ptr;

    if (!FileMenuKeySteal)
    {
        FileMenuKeySteal = true;
        strcpy(oldSlotText, SlotText[option]);
        ptr = SlotText[option];
        while (*ptr)
        {
            ptr++;
        }
        *ptr = '[';
        *(ptr + 1) = 0;
        SlotStatus[option]++;
        currentSlot = option;
        slotptr = ptr - SlotText[option];
        return /*false*/;
    }
    else
    {
        G_SaveGame(option, SlotText[option]);
        FileMenuKeySteal = false;
        MN_DeactivateMenu();
    }
    BorderNeedRefresh = true;
/*
    if (quicksave == -1)
    {
        quicksave = option + 1;
        players[consoleplayer].message = NULL;
        players[consoleplayer].messageTics = 1;
    }
    return true;
*/
}

//---------------------------------------------------------------------------
//
// PROC SCEpisode
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCEpisode(int option)
{
    if (gamemode == shareware && option > 1)
    {
        P_SetMessage(&players[consoleplayer],
                     "ONLY AVAILABLE IN THE REGISTERED VERSION", true);
    }
    else
    {
        MenuEpisode = option;
        SetMenu(MENU_SKILL);
    }
//    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCSkill(int option)
{
    G_DeferedInitNew(option, MenuEpisode, 1);
    MN_DeactivateMenu();
//    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCMouseSensi
//
//---------------------------------------------------------------------------
/*
//static void boolean SCMouseSensi(int option)
static void SCMouseSensi(int option)
{
    if (option == RIGHT_DIR)
    {
        if (mouseSensitivity < 13)
        {
            mouseSensitivity++;
        }
    }
    else if (mouseSensitivity)
    {
        mouseSensitivity--;
    }
//    return true;
}
*/
//---------------------------------------------------------------------------
//
// PROC SCSfxVolume
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCSfxVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MaxVolume < 15)
        {
            snd_MaxVolume++;
        }
    }
    else if (snd_MaxVolume)
    {
        snd_MaxVolume--;
    }
    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
    soundchanged = true;        // we'll set it when we leave the menu
//    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCMusicVolume
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCMusicVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MusicVolume < 15)
        {
            snd_MusicVolume++;
        }
    }
    else if (snd_MusicVolume)
    {
        snd_MusicVolume--;
    }
    S_SetMusicVolume();
#ifdef OGG_SUPPORT
    Mix_VolumeMusic(snd_MusicVolume * 2);
#endif
//    return true;
}

#ifdef OGG_SUPPORT
/*static*/ boolean I_OPL_InitMusic(void);

static void SCMusicType(int option)
{
    switch(option)
    {
	case 0:
	    if(gamestate != GS_DEMOSCREEN && gamestate != GS_INTERMISSION  && gamestate != GS_FINALE && !demoplayback)
	    {
		tracknum = 0;
//		faketracknum = 1;
		opl = 0;
		S_StartMP3Music(0, -1);
	    }
	    break;
	case 1:
	    if(gamestate != GS_DEMOSCREEN && gamestate != GS_INTERMISSION  && gamestate != GS_FINALE && !demoplayback)
	    {
		tracknum = 0;
//		faketracknum = 1;
		opl = 1;
		I_OPL_InitMusic();
//		S_StopMusic();
		I_StopSong();
//		if(gamemode != commercial)
		{
		    if(!demoplayback)
			S_StartSong(gamemap - 1, true);
		    else
		    {
			if(gamemode == shareware)
			    S_StartSong(gamemap, true);
			else
			    S_StartSong(gamemap + 9, true);
		    }
		}
	    }
	    break;
    }
    if(gamestate == GS_DEMOSCREEN || gamestate == GS_FINALE || gamestate == GS_INTERMISSION || demoplayback)
	P_SetMessage(&players[consoleplayer], "CANNOT CHANGE STATE IN THIS MODE", true);
}
#endif

//---------------------------------------------------------------------------
//
// PROC SCScreenSize
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCScreenSize(int option)
{
    if (option == RIGHT_DIR)
    {
        if (screenblocks < 11)
        {
            screenblocks++;
        }
    }
    else if (screenblocks > 3)
    {
        screenblocks--;
    }
    R_SetViewSize(screenblocks, detailLevel);
//    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static void/*boolean*/ SCInfo(int option)
{
    InfoType = 1;
    S_StartSound(NULL, sfx_dorcls);
    if (/*!netgame &&*/ !demoplayback)
    {
        paused = true;
    }
//    return true;
}

static char *stupidtable[] =
{
    "A","B","C","D","E",
    "F","G","H","I","J",
    "K","L","M","N","O",
    "P","Q","R","S","T",
    "U","V","W","X","Y",
    "Z"
};

#define CLASSIC_CONTROLLER_A		0x1
#define CLASSIC_CONTROLLER_R		0x2
#define CLASSIC_CONTROLLER_PLUS		0x4
#define CLASSIC_CONTROLLER_L		0x8
#define CLASSIC_CONTROLLER_MINUS	0x10
#define CLASSIC_CONTROLLER_B		0x20
#define CLASSIC_CONTROLLER_LEFT		0x40
#define CLASSIC_CONTROLLER_DOWN		0x80
#define CLASSIC_CONTROLLER_RIGHT	0x100
#define CLASSIC_CONTROLLER_UP		0x200
#define CLASSIC_CONTROLLER_ZR		0x400
#define CLASSIC_CONTROLLER_ZL		0x800
#define CLASSIC_CONTROLLER_HOME		0x1000
#define CLASSIC_CONTROLLER_X		0x2000
#define CLASSIC_CONTROLLER_Y		0x4000
#define CONTROLLER_1			0x8000
#define CONTROLLER_2			0x10000

char *Key2String (int ch)
{
// S.A.: return "[" or "]" or "\"" doesn't work
// because there are no lumps for these chars,
// therefore we have to go with "RIGHT BRACKET"
// and similar for much punctuation.  Probably
// won't work with international keyboards and
// dead keys, either.
//
    switch (ch)
    {
	case CLASSIC_CONTROLLER_UP:	return "UP ARROW";
	case CLASSIC_CONTROLLER_DOWN:	return "DOWN ARROW";
	case CLASSIC_CONTROLLER_LEFT:	return "LEFT ARROW";
	case CLASSIC_CONTROLLER_RIGHT:	return "RIGHT ARROW";
	case CLASSIC_CONTROLLER_MINUS:	return "MINUS";
	case CLASSIC_CONTROLLER_PLUS:	return "PLUS";
	case CLASSIC_CONTROLLER_HOME:	return "HOME";
	case CLASSIC_CONTROLLER_A:	return "A";
	case CLASSIC_CONTROLLER_B:	return "B";
	case CLASSIC_CONTROLLER_X:	return "X";
	case CLASSIC_CONTROLLER_Y:	return "Y";
	case CLASSIC_CONTROLLER_ZL:	return "ZL";
	case CLASSIC_CONTROLLER_ZR:	return "ZR";
	case CLASSIC_CONTROLLER_L:	return "LEFT TRIGGER";
	case CLASSIC_CONTROLLER_R:	return "RIGHT TRIGGER";
	case CONTROLLER_1:		return "1";
	case CONTROLLER_2:		return "2";
    }

    // Handle letter keys
    // S.A.: could also be done with toupper
    if (ch >= 'a' && ch <= 'z')
	return stupidtable[(ch - 'a')];

    return "?";		// Everything else
}

static void ClearControls (int cctrlskey)
{
    int cctrls;

    for (cctrls = key_bindings_start_in_cfg_at_pos; cctrls < key_bindings_end_in_cfg_at_pos; cctrls++)
    {
    	if (*doom_defaults_list[cctrls].location == cctrlskey)
	    *doom_defaults_list[cctrls].location = 0;
    }
}

static void ClearKeys (int option)
{
    *doom_defaults_list[19].location = 0;
    *doom_defaults_list[20].location = 0;
    *doom_defaults_list[21].location = 0;
    *doom_defaults_list[22].location = 0;
    *doom_defaults_list[23].location = 0;
    *doom_defaults_list[24].location = 0;
    *doom_defaults_list[25].location = 0;
    *doom_defaults_list[26].location = 0;
    *doom_defaults_list[27].location = 0;
    *doom_defaults_list[28].location = 0;
    *doom_defaults_list[29].location = 0;
    *doom_defaults_list[30].location = 0;
    *doom_defaults_list[31].location = 0;
    *doom_defaults_list[32].location = 0;
    *doom_defaults_list[33].location = 0;
    *doom_defaults_list[34].location = 0;
}

static void ResetKeys (int option)
{
    *doom_defaults_list[19].location = CLASSIC_CONTROLLER_R;
    *doom_defaults_list[20].location = CLASSIC_CONTROLLER_L;
    *doom_defaults_list[21].location = CLASSIC_CONTROLLER_MINUS;
    *doom_defaults_list[22].location = CLASSIC_CONTROLLER_LEFT;
    *doom_defaults_list[23].location = CLASSIC_CONTROLLER_DOWN;
    *doom_defaults_list[24].location = CLASSIC_CONTROLLER_RIGHT;
    *doom_defaults_list[25].location = CLASSIC_CONTROLLER_ZL;
    *doom_defaults_list[26].location = CLASSIC_CONTROLLER_ZR;
    *doom_defaults_list[27].location = CLASSIC_CONTROLLER_Y;
    *doom_defaults_list[28].location = CLASSIC_CONTROLLER_A;
    *doom_defaults_list[29].location = CLASSIC_CONTROLLER_PLUS;
    *doom_defaults_list[30].location = CLASSIC_CONTROLLER_X;
    *doom_defaults_list[31].location = CLASSIC_CONTROLLER_B;
    *doom_defaults_list[32].location = CLASSIC_CONTROLLER_UP;
    *doom_defaults_list[33].location = CLASSIC_CONTROLLER_HOME;
    *doom_defaults_list[34].location = CONTROLLER_1;
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

WPADData pad;
WPADData lastpad;

boolean MN_Responder(event_t * event)
{
//    int charTyped;
    int /*key,*/ ch;
//    int i;
    MenuItem_t *item;
//    extern boolean automapactive;
    extern void D_StartTitle(void);
    extern void G_CheckDemoStatus(void);
//    char *textBuffer;

//    static  int     joywait = 0;

    // Required for Twilight Hack input bug 
/*
    if (joywait == 0)
	joywait = I_GetTime();
*/	
    ch = -1; // will be changed to a legit char if we're going to use it here

    // Process joystick input
    // For some reason, polling ev.data for joystick input here in the menu code doesn't work when
    // using the twilight hack to launch wiidoom. At the same time, it works fine if you're using the
    // homebrew channel. I don't know why this is so for the meantime I'm polling the wii remote directly.

    WPADData *data = WPAD_Data(0);

    //Classic Controls
    if(data->exp.type == WPAD_EXP_CLASSIC)
    {
	if ((data->btns_d & WPAD_CLASSIC_BUTTON_UP) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_up;                                // phares 3/7/98

//	    joywait = I_GetTime() + 5;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_DOWN) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_down;                              // phares 3/7/98

//	    joywait = I_GetTime() + 5;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_LEFT) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_left;                              // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_RIGHT) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_right;                             // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_B) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_forward;                             // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_A) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_back;                         // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}
/*
	if (data->btns_d & WPAD_CLASSIC_BUTTON_MINUS)
	{
	    ch = key_menu_activate;                         // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}
*/
	if ((data->exp.classic.ljs.pos.y > (data->exp.classic.ljs.center.y + 50)) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_up;

//	    joywait = I_GetTime() + 5;
	}
	else if ((data->exp.classic.ljs.pos.y < (data->exp.classic.ljs.center.y - 50)) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_down;

//	    joywait = I_GetTime() + 5;
	}

	if ((data->exp.classic.ljs.pos.x > (data->exp.classic.ljs.center.x + 50)) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_right;

//	    joywait = I_GetTime() + 5;
	}
	else if ((data->exp.classic.ljs.pos.x < (data->exp.classic.ljs.center.x - 50)) /*&& (joywait < I_GetTime())*/)
	{
	    ch = key_menu_left;

//	    joywait = I_GetTime() + 5;
	}
    }

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.
/*
    if (testcontrols)
    {
        if (event->type == ev_quit
         || (event->type == ev_keydown
          && (event->data1 == key_menu_activate
           || event->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }
*/
    if (askforkey && data->btns_d)		// KEY BINDINGS
    {
	ClearControls(event->data1);
	*doom_defaults_list[keyaskedfor + 19 + FirstKey].location = event->data1;
	askforkey = false;
	return true;
    }

    if (askforkey && event->type == ev_mouse)
    {
	if (event->data1 & 1)
	    return true;
	if (event->data1 & 2)
	    return true;
	if (event->data1 & 4)
	    return true;
	return false;
    }
/*
    // "close" button pressed on window?
    if (event->type == ev_quit)
    {
        // First click on close = bring up quit confirm message.
        // Second click = confirm quit.

        if (!MenuActive && askforquit && typeofask == 1)
        {
            G_CheckDemoStatus();
            I_Quit();
        }
        else
        {
            SCQuitGame(0);
            S_StartSound(NULL, sfx_chat);
        }
        return true;
    }

    if (event->type != ev_keydown)
    {
        return false;
    }

    key = event->data1;
    charTyped = event->data2;
*/
    if (InfoType)
    {
	if(ch == key_menu_forward)		// FIX FOR NINTENDO WII
	{
	    if (gamemode == shareware)
	    {
		InfoType = (InfoType + 1) % 5;
	    }
	    else
	    {
		InfoType = (InfoType + 1) % 4;
	    }
	    if (ch == KEY_ESCAPE)
	    {
		InfoType = 0;
	    }
	    if (!InfoType)
	    {
		paused = false;
		MN_DeactivateMenu();
		SB_state = -1;      //refresh the statbar
		BorderNeedRefresh = true;
	    }
	    S_StartSound(NULL, sfx_dorcls);
	}
        return (true);          //make the info screen eat the keypress
    }
/*
    if (ravpic && key == KEY_F1)
    {
        G_ScreenShot();
        return (true);
    }
*/
    if (askforquit)
    {
        if (ch == key_menu_forward)		// FIX FOR NINTENDO WII
        {
            switch (typeofask)
            {
                case 1:
                    G_CheckDemoStatus();
                    I_Quit();
                    return false;

                case 2:
                    players[consoleplayer].messageTics = 0;
                    //set the msg to be cleared
                    players[consoleplayer].message = NULL;
                    paused = false;
                    I_SetPalette(W_CacheLumpName
                                 ("PLAYPAL", PU_CACHE));
                    D_StartTitle();     // go to intro/demo mode.
                    break;
/*
                case 3:
                    P_SetMessage(&players[consoleplayer],
                                 "QUICKSAVING....", false);
                    FileMenuKeySteal = true;
                    SCSaveGame(quicksave - 1);
                    BorderNeedRefresh = true;
                    break;

                case 4:
                    P_SetMessage(&players[consoleplayer],
                                 "QUICKLOADING....", false);
                    SCLoadGame(quickload - 1);
                    BorderNeedRefresh = true;
                    break;
*/
                default:
                    break;
            }

            askforquit = false;
            typeofask = 0;

            return true;
        }
        else if (ch == key_menu_abort || ch == KEY_ESCAPE)
        {
            players[consoleplayer].messageTics = 1;  //set the msg to be cleared
            askforquit = false;
            typeofask = 0;
            paused = false;
            UpdateState |= I_FULLSCRN;
            BorderNeedRefresh = true;
            return true;
        }

        return false;           // don't let the keys filter thru
    }

    if(askforsave)
    {
        if (ch == key_menu_forward)		// FIX FOR NINTENDO WII
        {
            switch (typeofask2)
            {
                case 1:
                    players[consoleplayer].messageTics = 0;
                    //set the msg to be cleared
                    players[consoleplayer].message = NULL;
                    paused = false;
                    I_SetPalette(W_CacheLumpName
                                 ("PLAYPAL", PU_CACHE));
//                    D_StartTitle();     // go to intro/demo mode.
                    break;

                default:
                    break;
	    }
            askforsave = false;
            typeofask2 = 0;

            return true;
	}
        else if (ch == key_menu_abort || ch == KEY_ESCAPE)
        {
            players[consoleplayer].messageTics = 1;  //set the msg to be cleared
            askforsave = false;
            typeofask2 = 0;
            paused = false;
            UpdateState |= I_FULLSCRN;
            BorderNeedRefresh = true;
            return true;
        }

        return false;           // don't let the keys filter thru
    }
/*
    if (!MenuActive && !chatmodeon)
    {
        if (key == key_menu_decscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            SCScreenSize(LEFT_DIR);
            S_StartSound(NULL, sfx_keyup);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_incscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            SCScreenSize(RIGHT_DIR);
            S_StartSound(NULL, sfx_keyup);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_help)           // F1
        {
            SCInfo(0);      // start up info screens
            MenuActive = true;
            return (true);
        }
        else if (key == key_menu_save)           // F2 (save game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &SaveMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_load)           // F3 (load game)
        {
            if (SCNetCheck(2))
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_volume)         // F4 (volume)
        {
            MenuActive = true;
            FileMenuKeySteal = false;
            MenuTime = 0;
            CurrentMenu = &Options2Menu;
            CurrentItPos = CurrentMenu->oldItPos;
            if (!netgame && !demoplayback)
            {
                paused = true;
            }
            S_StartSound(NULL, sfx_dorcls);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (key == key_menu_detail)          // F5 (detail)
        {
            // F5 isn't used in Heretic. (detail level)
            return true;
        }
        else if (key == key_menu_qsave)           // F6 (quicksave)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                if (!quicksave || quicksave == -1)
                {
                    MenuActive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    CurrentMenu = &SaveMenu;
                    CurrentItPos = CurrentMenu->oldItPos;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_dorcls);
                    slottextloaded = false; //reload the slot text, when needed
                    quicksave = -1;
                    P_SetMessage(&players[consoleplayer],
                                 "CHOOSE A QUICKSAVE SLOT", true);
                }
                else
                {
                    askforquit = true;
                    typeofask = 3;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_chat);
                }
            }
            return true;
        }
        else if (key == key_menu_endgame)         // F7 (end game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                S_StartSound(NULL, sfx_chat);
                SCEndGame(0);
            }
            return true;
        }
        else if (key == key_menu_messages)        // F8 (toggle messages)
        {
            SCMessages(0);
            return true;
        }
        else if (key == key_menu_qload)           // F9 (quickload)
        {
            if (!quickload || quickload == -1)
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
                quickload = -1;
                P_SetMessage(&players[consoleplayer],
                             "CHOOSE A QUICKLOAD SLOT", true);
            }
            else
            {
                askforquit = true;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                typeofask = 4;
                S_StartSound(NULL, sfx_chat);
            }
            return true;
        }
        else if (key == key_menu_quit)            // F10 (quit)
        {
            if (gamestate == GS_LEVEL)
            {
                SCQuitGame(0);
                S_StartSound(NULL, sfx_chat);
            }
            return true;
        }
        else if (key == key_menu_gamma)           // F11 (gamma correction)
        {
            usegamma++;
            if (usegamma > 4)
            {
                usegamma = 0;
            }
            I_SetPalette((byte *) W_CacheLumpName("PLAYPAL", PU_CACHE));
            return true;
        }

    }
*/
    if (!MenuActive)
    {
//        if (key == 4096 || gamestate == GS_DEMOSCREEN || demoplayback)
        if (ch == key_menu_activate /*|| gamestate == GS_DEMOSCREEN || demoplayback*/)
        {
            MN_ActivateMenu();
            return (true);
        }
        return (false);
    }
    if (!FileMenuKeySteal)
    {
        item = &CurrentMenu->items[CurrentItPos];

//        if (key == 128)            // Next menu item
        if (ch == key_menu_down)            // Next menu item
        {
            do
            {
		if (CurrentMenu->items[CurrentItPos].type == ITT_SETKEY
			&& CurrentItPos+1 > CurrentMenu->itemCount-1)
		{
		    if (FirstKey == FIRSTKEY_MAX)
		    {
			CurrentItPos = 0; // End of Key menu
			FirstKey = 0;
		    }
		    else
			FirstKey++;
		}
                else if (CurrentItPos + 1 > CurrentMenu->itemCount - 1)
                {
                    CurrentItPos = 0;
                }
                else
                {
                    CurrentItPos++;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, sfx_switch);
            return (true);
        }
//        else if (key == 512)         // Previous menu item
        else if (ch == key_menu_up)         // Previous menu item
        {
            do
            {
		if (CurrentMenu->items[CurrentItPos].type == ITT_SETKEY && CurrentItPos == 0)
		{
		    if (FirstKey == 0)
		    {
			CurrentItPos = 19; // End of Key menu (14 == 15 (max lines on a page) - 1)
			FirstKey = FIRSTKEY_MAX;
		    }
		    else
			FirstKey--;
		}
                if (CurrentItPos == 0)
                {
                    CurrentItPos = CurrentMenu->itemCount - 1;
                }
                else
                {
                    CurrentItPos--;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, sfx_switch);
            return (true);
        }
//        else if (key == 64)       // Slider left
        else if (ch == key_menu_left)       // Slider left
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(LEFT_DIR);
                S_StartSound(NULL, sfx_keyup);
            }
            return (true);
        }
//        else if (key == 256)      // Slider right
        else if (ch == key_menu_right)      // Slider right
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(RIGHT_DIR);
                S_StartSound(NULL, sfx_keyup);
            }
            return (true);
        }
//        else if (key == 4)    // Activate item (enter)
        else if (ch == key_menu_forward)    // Activate item (enter)
        {
            if (item->type == ITT_SETMENU)
            {
                SetMenu(item->menu);
            }
            else if (item->func != NULL)
            {
                CurrentMenu->oldItPos = CurrentItPos;
                if (item->type == ITT_LRFUNC)
                {
                    item->func(RIGHT_DIR);
                }
                else if (item->type == ITT_EFUNC)
                {
                    /*if (*/item->func(item->option);//)
                    {
/*
                        if (item->menu != MENU_NONE)
                        {
                            SetMenu(item->menu);
                        }
*/
                    }
                }
		else if (item->type == ITT_SETKEY)
		{
		    item->func(item->option);
		}
            }
            S_StartSound(NULL, sfx_dorcls);
            return (true);
        }
//        else if (key == 4096)     // Toggle menu
        else if (ch == key_menu_activate)     // Toggle menu
        {
            MN_DeactivateMenu();
            return (true);
        }
/*
        else if (key == key_menu_back)         // Go back to previous menu
        {
            S_StartSound(NULL, sfx_switch);
            if (CurrentMenu->prevMenu == MENU_NONE)
            {
                MN_DeactivateMenu();
            }
            else
            {
                SetMenu(CurrentMenu->prevMenu);
            }
            return (true);
        }
        else if (charTyped != 0)
        {
            // Jump to menu item based on first letter:

            for (i = 0; i < CurrentMenu->itemCount; i++)
            {
                if (CurrentMenu->items[i].text)
                {
                    if (toupper(charTyped)
                        == toupper(DEH_String(CurrentMenu->items[i].text)[0]))
                    {
                        CurrentItPos = i;
                        return (true);
                    }
                }
            }
        }
*/
        return (false);
    }
    else
    {                           // Editing file names
/*
        textBuffer = &SlotText[currentSlot][slotptr];
        if (key == KEY_BACKSPACE)
        {
            if (slotptr)
            {
                *textBuffer-- = 0;
                *textBuffer = ASCII_CURSOR;
                slotptr--;
            }
            return (true);
        }
        if (key == KEY_ESCAPE)
        {
            memset(SlotText[currentSlot], 0, SLOTTEXTLEN + 2);
            strcpy(SlotText[currentSlot], oldSlotText);
            SlotStatus[currentSlot]--;
            MN_DeactivateMenu();
            return (true);
        }
*/
        if (ch == key_menu_forward)
        {
            SlotText[currentSlot][slotptr] = 0; // clear the cursor
            item = &CurrentMenu->items[CurrentItPos];
            CurrentMenu->oldItPos = CurrentItPos;
            if (item->type == ITT_EFUNC)
            {
                item->func(item->option);
                if (item->menu != MENU_NONE)
                {
                    SetMenu(item->menu);
                }
            }
            return (true);
        }
/*
        if (slotptr < SLOTTEXTLEN && key != KEY_BACKSPACE)
        {
            if (isalpha(charTyped))
            {
                *textBuffer++ = toupper(charTyped);
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
            if (isdigit(charTyped) || charTyped == ' '
              || charTyped == ',' || charTyped == '.' || charTyped == '-'
              || charTyped == '!')
            {
                *textBuffer++ = charTyped;
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
        }
*/
        return (true);
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC MN_ActivateMenu
//
//---------------------------------------------------------------------------

void MN_ActivateMenu(void)
{
    if (MenuActive)
    {
        return;
    }
    if (paused)
    {
        S_ResumeSound();
    }
    MenuActive = true;
    FileMenuKeySteal = false;
    MenuTime = 0;
    CurrentMenu = &MainMenu;
    CurrentItPos = CurrentMenu->oldItPos;
    if (/*!netgame &&*/ !demoplayback)
    {
        paused = true;
    }
    S_StartSound(NULL, sfx_dorcls);
    slottextloaded = false;     //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
    if (CurrentMenu != NULL)
    {
        CurrentMenu->oldItPos = CurrentItPos;
    }

    if(MenuActive)
	warped = 0;

    MenuActive = false;
//    if (!netgame)
    {
        paused = false;
    }
    S_StartSound(NULL, sfx_dorcls);
    if (soundchanged)
    {
        S_SetMaxVolume(true);   //recalc the sound curve
        soundchanged = false;
    }
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageTics = 1;
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawInfo
//
//---------------------------------------------------------------------------

void MN_DrawInfo(void)
{
    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));

    if(HERETIC_BETA || HERETIC_SHARE_1_0)
	V_DrawPatch(0, 0, W_CacheLumpNum(W_GetNumForName("TITLE")+InfoType,
		PU_CACHE));
    else
	V_DrawRawScreen(W_CacheLumpNum(W_GetNumForName("TITLE") + InfoType,
                                   PU_CACHE));

//      V_DrawPatch(0, 0, W_CacheLumpNum(W_GetNumForName("TITLE")+InfoType,
//              PU_CACHE));
}


//---------------------------------------------------------------------------
//
// PROC SetMenu
//
//---------------------------------------------------------------------------

static void SetMenu(MenuType_t menu)
{
    CurrentMenu->oldItPos = CurrentItPos;
    CurrentMenu = Menus[menu];
    CurrentItPos = CurrentMenu->oldItPos;
}

//---------------------------------------------------------------------------
//
// PROC DrawSlider
//
//---------------------------------------------------------------------------

static void DrawSlider(Menu_t * menu, int item, int width, int slot)
{
    int x;
    int y;
    int x2;
    int count;

    x = menu->x + 24;
    y = menu->y + 2 + (item * ITEM_HEIGHT);
    V_DrawPatch(x - 32, y, W_CacheLumpName(DEH_String("M_SLDLT"), PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawPatch(x2, y, W_CacheLumpName(DEH_String(count & 1 ? "M_SLDMD1"
                                           : "M_SLDMD2"), PU_CACHE));
    }
    V_DrawPatch(x2, y, W_CacheLumpName(DEH_String("M_SLDRT"), PU_CACHE));
    V_DrawPatch(x + 4 + slot * 8, y + 7, 
                W_CacheLumpName(DEH_String("M_SLDKB"), PU_CACHE));
}

void DetectState(void)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_DEAD)
    {
	if(suicide == 0)
	    P_SetMessage(&players[consoleplayer], "CHEATING NOT ALLOWED - YOU ARE DEAD", true);
    }
    else if(demoplayback && gamestate == GS_LEVEL && players[consoleplayer].playerstate == PST_LIVE)
	P_SetMessage(&players[consoleplayer], "CHEATING NOT ALLOWED IN DEMO MODE", true);
    else if(demoplayback && gamestate == GS_LEVEL && players[consoleplayer].playerstate == PST_DEAD)
	P_SetMessage(&players[consoleplayer], "CHEATING NOT ALLOWED IN DEMO MODE", true);
    else if(demoplayback && gamestate != GS_LEVEL)
	P_SetMessage(&players[consoleplayer], "CHEATING NOT ALLOWED IN DEMO MODE", true);
    else if(!demoplayback && gamestate != GS_LEVEL)
	P_SetMessage(&players[consoleplayer], "CHEATING NOT ALLOWED IN DEMO MODE", true);

    if(gameskill == sk_nightmare)
	P_SetMessage(&players[consoleplayer], "CHEATING NOT ALLOWED FOR NIGHTMARE SKILL", true);
}

static void SCBrightness(int option)
{
    if(option == RIGHT_DIR)
    {
	if(usegamma < 4)
	    usegamma++;
    }
    else if(usegamma)
	usegamma--;

    I_SetPalette((byte *) W_CacheLumpName("PLAYPAL", PU_CACHE));
    P_SetMessage(&players[consoleplayer], GammaText[usegamma], false);
}

void SCWalkingSpeed(int option)
{
    switch(option)
    {
      case 0:
	if(forwardmove > 19)
	    forwardmove--;
	break;
      case 1:
	if(forwardmove < 47)
	    forwardmove++;
	break;
    }
}

void SCTurningSpeed(int option)
{
    switch(option)
    {
      case 0:
	if(turnspeed > 5)
	    turnspeed--;
	break;
      case 1:
	if(turnspeed < 10)
	    turnspeed++;
	break;
    }
}

void SCStrafingSpeed(int option)
{
    switch(option)
    {
      case 0:
	if(sidemove > 16)
	    sidemove--;
	break;
      case 1:
	if (sidemove < 32)
	    sidemove++;
	break;
    }
}

static void SCDetails(int option)
{
    option = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	P_SetMessage(&players[consoleplayer],
		"HIGH DETAIL", true);
    else
	P_SetMessage(&players[consoleplayer],
		"LOW DETAIL", true);
}

static void DrawScreenMenu(void)
{
    DrawSlider(&ScreenMenu, 1, 5, usegamma);

    if(!debugmode)
	ScreenMenu.itemCount = 4;
    else if(debugmode)
    {
	ScreenMenu.itemCount = 5;

	if(detailLevel)
	    MN_DrTextB("LOW", 170, 120);
	else
	    MN_DrTextB("HIGH", 170, 120);
    }
    DrawSlider(&ScreenMenu, 3, 9, screenblocks-3);
}

static void DrawControlMenu(void)
{
    MN_DrTextB("WALKING SPEED", 38, 0);
    MN_DrTextB("TURNING SPEED", 38, 40);
    MN_DrTextB("STRAFING SPEED", 38, 80);
    MN_DrTextB("FREELOOK :", 38, 124);
    MN_DrTextB("FREELOOK SPEED", 38, 142);
    MN_DrTextB("KEY BINDINGS", 38, 179);

    DrawSlider(&ControlMenu, 1, 29, forwardmove-19);
    DrawSlider(&ControlMenu, 3, 6, turnspeed-5);
    DrawSlider(&ControlMenu, 5, 17, sidemove-16);

    MN_DrTextB(mlooktext[mouselook], 150, 124);

    DrawSlider(&ControlMenu, 8, 9, mspeed-2);

    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

static void SCMouselook(int option)
{
    if(option == RIGHT_DIR)
    {
	if(mouselook < 2)
	    mouselook++;
    }
    else if(mouselook)
	mouselook--;
}

static void SCMouseSpeed(int option)
{
    if(option == RIGHT_DIR)
    {
	if(mspeed < 10)
	    mspeed++;
    }
    else if(mspeed > 2)
	mspeed--;
}

static void SCSetKey(int option)
{
    askforkey = true;
    keyaskedfor = option;

    if (!demoplayback)
	paused = true;
}
/*
static void ButtonLayout(int option)
{
    if(option == RIGHT_DIR)
    {
	if(button_layout < 1)
	    button_layout++;
    }
    else if(button_layout)
	button_layout--;
}
*/
static void SCNTrack(int option)
{
    if(option == RIGHT_DIR)
    {
	if(tracknum < 50)
	{
	    tracknum++;
	    if(HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
	    {
#ifdef OGG_SUPPORT
		if(opl)
#endif
		{
		    if(tracknum == 9)
			tracknum = 48;
		}
#ifdef OGG_SUPPORT
		else
		{
		    if(tracknum == 10)
			tracknum = 20;
		    else if(tracknum == 23)
			tracknum = 22;
		}
#endif
	    }
	    else if(HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3)
	    {
#ifdef OGG_SUPPORT
		if(opl)
#endif
		{
		    if(tracknum == 13)
			tracknum = 14;
		    else if(tracknum == 18)
			tracknum = 19;
		    else if(tracknum == 21)
			tracknum = 48;
		}
#ifdef OGG_SUPPORT
		else
		{
		    if(tracknum == 23)
			tracknum = 22;
		}
#endif
	    }
	}
    }
    else if(tracknum)
    {
	tracknum--;
	if(HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
	{
#ifdef OGG_SUPPORT
	    if(opl)
#endif
	    {
		if(tracknum == 47)
		    tracknum = 8;
	    }
#ifdef OGG_SUPPORT
	    else
	    {
		    if(tracknum == 19)
			tracknum = 9;

		    if(tracknum == 0)
			tracknum = 1;
	    }
#endif
	}
	else if(HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3)
	{
#ifdef OGG_SUPPORT
	    if(opl)
#endif
	    {
		if(tracknum == 47)
		    tracknum = 20;
		else if(tracknum == 18)
		    tracknum = 17;
		else if(tracknum == 13)
		    tracknum = 12;
	    }
/*
#ifdef OGG_SUPPORT
	    else
	    {
		    if(tracknum == 23)
			tracknum = 22;
	    }
#endif
*/
	}
    }
    if(gamestate != GS_DEMOSCREEN && gamestate != GS_INTERMISSION  && gamestate != GS_FINALE && !demoplayback)
    {
	P_SetMessage(&players[consoleplayer], "MUSIC CHANGE", true);
#ifdef OGG_SUPPORT
	if(opl)
#endif
	    S_StartSong(tracknum, true);
#ifdef OGG_SUPPORT
	else
	{
	    forced = true;
	    S_StartMP3Music(0, -1);
	}
#endif
	forced = false;
    }
    else
	P_SetMessage(&players[consoleplayer], "CANNOT CHANGE STATE IN DEMO MODE", true);
}
/*
static void SCNTrackLoop(int option)
{
    switch(option)
    {
    case 0:
        if (loop)
            loop = false;
        break;
    case 1:
        if (loop == false)
            loop = true;
        break;
    }
}
*/
static void DrawBindingsMenu(void)
{
    int ctrls;

    for (ctrls = 0; ctrls < 16; ctrls++)
    {
	if (askforkey && keyaskedfor == ctrls)
	    MN_DrTextA("???", 195, (ctrls*10+5));
	else
	    MN_DrTextA(Key2String(*(doom_defaults_list[ctrls+FirstKey+19].location)),195,(ctrls*10+5));
    }

/*
    if(button_layout == 0)
    {
	MN_DrTextA("WEAPON LEFT", 40, 70);
	MN_DrTextA("SHOW AUTOMAP", 40, 80);
//    	MN_DrTextA("PS VITA", 195, 140);
    }
//    else if(button_layout == 1)
//    {
//	MN_DrTextA("STRAFE LEFT", 40, 40);
//	MN_DrTextA("STRAFE RIGHT", 40, 50);
//    	MN_DrTextA("PSP", 195, 140);
//    }
*/
    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

static void DrawSoundMenu(void)
{
    DrawSlider(&SoundMenu, 1, 16, snd_MaxVolume);
    DrawSlider(&SoundMenu, 3, 16, snd_MusicVolume);

    if(gamestate != GS_DEMOSCREEN && gamestate != GS_INTERMISSION  && gamestate != GS_FINALE && !demoplayback)
    {
#ifdef OGG_SUPPORT
	if(opl)
#endif
	{
	    if(HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
		MN_DrTextB(songtextbeta[tracknum], 220, 110);
	    else
		MN_DrTextB(songtext[tracknum], 220, 110);
	}
#ifdef OGG_SUPPORT
	else
	{
	    if(HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
		MN_DrTextB(songtextbetamp3[tracknum], 220, 110);
	    else
		MN_DrTextB(songtextmp3[tracknum], 220, 110);

	    if(tracknum == 0)
		tracknum = 1;
	}
#endif
    }
    else
	MN_DrTextB("01", 220, 110);

#ifdef OGG_SUPPORT
    if(opl)
	MN_DrTextB("OPL", 220, 130);
    else
	MN_DrTextB("MP3", 220, 130);
#endif
/*
    if(loop)
	MN_DrTextB("YES", 220, 130);
    else
	MN_DrTextB("NO", 220, 130);
*/
}
/*
static void SCCpu(int option)
{
    mhz333 ^= 1;

    if (mhz333)
    {
	P_SetMessage(&players[consoleplayer], "CLOCK NOW AT 333MHZ", true);
	scePowerSetClockFrequency(333, 333, 166);
    }
    else
    {
	P_SetMessage(&players[consoleplayer], "CLOCK NOW AT 222MHZ", true);
	scePowerSetClockFrequency(222, 222, 111);
    }
    S_StartSound(NULL, sfx_chat);
}
*/
static void SCTicker(int option)
{
    DisplayTicker = !DisplayTicker;

    if (DisplayTicker)
	P_SetMessage(&players[consoleplayer], TXT_CHEATTICKERON, true);
    else
	P_SetMessage(&players[consoleplayer], TXT_CHEATTICKEROFF, true);

    I_DisplayFPSDots(DisplayTicker);

    SB_state = -1;      //refresh the statbar
    BorderNeedRefresh = true;

    S_StartSound(NULL, sfx_chat);
}

static void SCFPS(int option)
{
    if(FramesPerSecond < 1)
    {
	FramesPerSecond++;
	P_SetMessage(&players[consoleplayer], "FPS COUNTER ON", true);
    }
    else if(FramesPerSecond)
    {
	FramesPerSecond--;
	P_SetMessage(&players[consoleplayer], "FPS COUNTER OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCWiiLight(int option)
{
    if(wiilight < 1)
    {
	wiilight++;
	P_SetMessage(&players[consoleplayer], "WII LIGHT ON", true);
    }
    else if(wiilight)
    {
	wiilight--;
	P_SetMessage(&players[consoleplayer], "WII LIGHT OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCGrid(int option)
{
    if(drawgrid < 1)
    {
	drawgrid++;
	P_SetMessage(&players[consoleplayer], "AUTOMAP GRID ON", true);
    }
    else if(drawgrid)
    {
	drawgrid--;
	P_SetMessage(&players[consoleplayer], "AUTOMAP GRID OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCFollow(int option)
{
    followplayer ^= 1;

    if(followplayer)
	P_SetMessage(&players[consoleplayer], "FOLLOW MODE ON", true);
    else
	P_SetMessage(&players[consoleplayer], "FOLLOW MODE OFF", true);

    S_StartSound(NULL, sfx_chat);
}

static void SCRotate(int option)
{
    switch(option)
    {
    case 0:
        if (am_rotate)
            am_rotate = false;
	P_SetMessage(&players[consoleplayer], "AUTOMAP ROTATION OFF", true);
        break;
    case 1:
        if (am_rotate == false)
            am_rotate = true;
	P_SetMessage(&players[consoleplayer], "AUTOMAP ROTATION ON", true);
        break;
    }
}

static void SCStats(int option)
{
    if(show_stats < 1)
	show_stats++;
    else if(show_stats)
	show_stats--;
}

static void SCWeaponChange(int option)
{
    if(use_vanilla_weapon_change < 1)
    {
	use_vanilla_weapon_change++;
	P_SetMessage(&players[consoleplayer], "ORIGINAL WEAPON CHANGING STYLE DISABLED", true);
    }
    else if(use_vanilla_weapon_change)
    {
	use_vanilla_weapon_change--;
	P_SetMessage(&players[consoleplayer], "ORIGINAL WEAPON CHANGING STYLE ENABLED", true);
    }
}

static void SCCrosshair(int option)
{
    if(crosshair < 1)
	crosshair++;
    else if(crosshair)
	crosshair--;
}

static void SCJumping(int option)
{
    if(jumping == false)
	jumping = true;
    else if(jumping)
	jumping = false;
}

static void SCCoords(int option)
{
    if(coords < 1)
    {
	coords++;
	P_SetMessage(&players[consoleplayer], "COORDINATES INFO ON", true);
    }
    else if(coords)
    {
	coords--;
	P_SetMessage(&players[consoleplayer], "COORDINATES INFO OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCMapname(int option)
{
    if(mapname < 1)
    {
	mapname++;
	P_SetMessage(&players[consoleplayer], "IN GAME MAP NAME INFO ON", true);
    }
    else if(mapname)
    {
	mapname--;
	P_SetMessage(&players[consoleplayer], "IN GAME MAP NAME INFO OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCSound(int option)
{
    if(!DebugSound)
    {
	DebugSound = true;
	P_SetMessage(&players[consoleplayer], "SOUND INFO ON", true);
    }
    else if(DebugSound)
    {
	DebugSound = false;
	P_SetMessage(&players[consoleplayer], "SOUND INFO OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCTimer(int option)
{
    if(timer < 1)
    {
	timer++;
	P_SetMessage(&players[consoleplayer], "IN GAME TIMER INFO ON", true);
    }
    else if(timer)
    {
	timer--;
	P_SetMessage(&players[consoleplayer], "IN GAME TIMER INFO OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}

static void SCVersion(int option)
{
    if(version < 1)
    {
	version++;
	P_SetMessage(&players[consoleplayer], "VERSION INFO ON", true);
    }
    else if(version)
    {
	version--;
	P_SetMessage(&players[consoleplayer], "VERSION INFO OFF", true);
    }
    S_StartSound(NULL, sfx_chat);
}
/*
static void SCBattery(int option)
{
    if(battery_info < 1)
	battery_info++;
    else if(battery_info)
	battery_info--;
}

static void SCProcessor(int option)
{
    if(cpu_info < 1)
	cpu_info++;
    else if(cpu_info)
	cpu_info--;
}

static void SCShowMemory(int option)
{
    if(memory_info < 1)
	memory_info++;
    else if(memory_info)
	memory_info--;
}

static void SCOther(int option)
{
    if(other_info < 1)
	other_info++;
    else if(other_info)
	other_info--;
}
*/
static void DrawDebugMenu(void)
{

    if(coords==1)
	MN_DrTextB("ON", 240, 50);
    else if(coords==0)
	MN_DrTextB("OFF", 240, 50);

    if(mapname==1)
	MN_DrTextB("ON", 240, 70);
    else if(mapname==0)
	MN_DrTextB("OFF", 240, 70);

    if(DebugSound)
	MN_DrTextB("ON", 240, 90);
    else if(!DebugSound)
	MN_DrTextB("OFF", 240, 90);

    if(timer==1)
	MN_DrTextB("ON", 240, 110);
    else if(timer==0)
	MN_DrTextB("OFF", 240, 110);

    if(version==1)
	MN_DrTextB("ON", 240, 130);
    else if(version==0)
	MN_DrTextB("OFF", 240, 130);
/*
    if(battery_info)
    {
	sprintf(unit_plugged_textbuffer,"UNIT IS PLUGGED IN: %d \n",scePowerIsPowerOnline());
	sprintf(battery_present_textbuffer,"BATTERY IS PRESENT: %d \n",scePowerIsBatteryExist());
	sprintf(battery_charging_textbuffer,"BATTERY IS CHARGING: %d \n",scePowerIsBatteryCharging());
	sprintf(battery_charging_status_textbuffer,"BATTERY CHARGING STATUS: %d \n",scePowerGetBatteryChargingStatus());
	sprintf(battery_low_textbuffer,"BATTERY IS LOW: %d \n",scePowerIsLowBattery());
	sprintf(battery_lifetime_percent_textbuffer,"BATTERY LIFETIME (PERC.): %d \n",scePowerGetBatteryLifePercent());
	sprintf(battery_lifetime_int_textbuffer,"BATTERY LIFETIME (INT.): %d \n",scePowerGetBatteryLifeTime());
	sprintf(battery_temp_textbuffer,"BATTERY TEMP.: %d \n",scePowerGetBatteryTemp());
	sprintf(battery_voltage_textbuffer,"BATTERY VOLTAGE: %d \n",scePowerGetBatteryVolt());

	MN_DrTextB("ON", 240, 110);
    }
    else
	MN_DrTextB("OFF", 240, 110);

    if(cpu_info)
    {
	sprintf(processor_clock_textbuffer,"PROCESSOR CLOCK FREQ.: %d \n",scePowerGetCpuClockFrequencyInt());
	sprintf(processor_bus_textbuffer,"PROCESSOR BUS FREQ.: %d \n",scePowerGetBusClockFrequencyInt());

	MN_DrTextB("ON", 240, 130);
    }
    else
	MN_DrTextB("OFF", 240, 130);

    if(memory_info)
    {
    	sprintf(allocated_ram_textbuffer,"ALLOCATED RAM: %d BYTES\n",allocated_ram_size);

    	sprintf(free_ram_textbuffer,"CURR. FREE RAM: %d BYTES\n",sceKernelTotalFreeMemSize());

    	max_free_ram = sceKernelMaxFreeMemSize();

    	sprintf(max_free_ram_textbuffer,"MAX. FREE RAM: %d BYTES\n",max_free_ram);

	MN_DrTextB("ON", 240, 150);
    }
    else
	MN_DrTextB("OFF", 240, 150);

    if(other_info)
    {
	sprintf(idle_time_textbuffer,"IDLE TIME: %d \n",scePowerGetIdleTimer());

	MN_DrTextB("ON", 240, 170);
    }
    else
	MN_DrTextB("OFF", 240, 170);
*/
    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

static void SCKeys(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->keys[key_yellow] = true;
	player->keys[key_green] = true;
	player->keys[key_blue] = true;

	playerkeys = 7;             // Key refresh flags

	P_SetMessage(player, DEH_String(TXT_CHEATKEYS), false);
    }
    SB_state = -1;      //refresh the statbar
    BorderNeedRefresh = true;
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCYellow(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->keys[key_yellow] = true;

	playerkeys = 7;             // Key refresh flags

	P_SetMessage(player, DEH_String("YELLOW KEY ADDED"), false);
    }
    SB_state = -1;      //refresh the statbar
    BorderNeedRefresh = true;
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCGreen(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->keys[key_green] = true;

	playerkeys = 7;             // Key refresh flags

	P_SetMessage(player, DEH_String("GREEN KEY ADDED"), false);
    }
    SB_state = -1;      //refresh the statbar
    BorderNeedRefresh = true;
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCBlue(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->keys[key_blue] = true;

	playerkeys = 7;             // Key refresh flags

	P_SetMessage(player, DEH_String("BLUE KEY ADDED"), false);
    }
    SB_state = -1;      //refresh the statbar
    BorderNeedRefresh = true;
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCAllArm(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArmor(player, 1);
	P_GiveArmor(player, 2);

	P_SetMessage(&players[consoleplayer], "ALL ARMOR ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCSilver(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArmor(player, 1);

	P_SetMessage(&players[consoleplayer], "SILVER SHIELD ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCShield(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArmor(player, 2);

	P_SetMessage(&players[consoleplayer], "ENCHANTED SHIELD ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponA(int option)
{
    int i;

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	if (!player->backpack)
	{
	    for (i = 0; i < NUMAMMO; i++)
	    {
		player->maxammo[i] *= 2;
	    }
	    player->backpack = true;
	}
	for (i = 0; i < NUMWEAPONS - 1; i++)
	{
	    player->weaponowned[i] = true;
	}
	if (HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
	{
	    player->weaponowned[wp_skullrod] = false;
	    player->weaponowned[wp_phoenixrod] = false;
	    player->weaponowned[wp_mace] = false;
	}
	for (i = 0; i < NUMAMMO; i++)
	{
	    player->ammo[i] = player->maxammo[i];
	}
	P_SetMessage(player, DEH_String(TXT_CHEATWEAPONS), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponB(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->maxammo[am_crossbow] *= 2;
	player->weaponowned[wp_crossbow] = true;
	player->ammo[am_crossbow] = player->maxammo[am_crossbow];

	P_SetMessage(player, DEH_String("ETHEREAL CROSSBOW ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponC(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->maxammo[am_blaster] *= 2;
	player->weaponowned[wp_blaster] = true;
	player->ammo[am_blaster] = player->maxammo[am_blaster];

	P_SetMessage(player, DEH_String("DRAGON CLAW ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponD(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->maxammo[am_skullrod] *= 2;
	player->weaponowned[wp_skullrod] = true;
	player->ammo[am_skullrod] = player->maxammo[am_skullrod];

	P_SetMessage(player, DEH_String("HELLSTAFF ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponE(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->maxammo[am_phoenixrod] *= 2;
	player->weaponowned[wp_phoenixrod] = true;
	player->ammo[am_phoenixrod] = player->maxammo[am_phoenixrod];

	P_SetMessage(player, DEH_String("PHOENIX ROD ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponF(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->maxammo[am_mace] *= 2;
	player->weaponowned[wp_mace] = true;
	player->ammo[am_mace] = player->maxammo[am_mace];

	P_SetMessage(player, DEH_String("FIREMACE ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWeaponG(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	player->weaponowned[wp_gauntlets] = true;

	P_SetMessage(player, DEH_String("GAUNTLETS OF THE NECROMANCER ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCAmmo(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	int i;

	static player_t* player;
	player = &players[consoleplayer];

	if (!player->backpack)
	{
	    for (i = 0; i < NUMAMMO; i++)
    	    {
		player->maxammo[i] *= 2;
    	    }
    	    player->backpack = true;
    	}
	for (i = 0; i < NUMAMMO; i++)
    	{
    	    player->ammo[i] = player->maxammo[i];
    	}
	P_SetMessage(player, DEH_String("AMMO ADDED"), false);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCArtifact(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_health, NULL);
	P_GiveArtifact(player, arti_fly, NULL);
	P_GiveArtifact(player, arti_invulnerability, NULL);
	P_GiveArtifact(player, arti_tomeofpower, NULL);
	P_GiveArtifact(player, arti_invisibility, NULL);
	P_GiveArtifact(player, arti_egg, NULL);
	P_GiveArtifact(player, arti_torch, NULL);
	P_GiveArtifact(player, arti_firebomb, NULL);

	if(HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3)
	{
	    P_GiveArtifact(player, arti_superhealth, NULL);
	    P_GiveArtifact(player, arti_teleport, NULL);
	}
	P_SetMessage(&players[consoleplayer], "ALL ARTIFACTS ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCRing(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_invulnerability, NULL);

	P_SetMessage(&players[consoleplayer], "RING OF INVINCIBILITY", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCQuartz(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_health, NULL);

	P_SetMessage(&players[consoleplayer], "QUARTZ FLASK ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCUrn(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_superhealth, NULL);

	P_SetMessage(&players[consoleplayer], "MYSTIC URN ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCTorch(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_torch, NULL);

	P_SetMessage(&players[consoleplayer], "TORCH ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCOvum(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_egg, NULL);

	P_SetMessage(&players[consoleplayer], "MORPH OVUM ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWings(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_fly, NULL);

	P_SetMessage(&players[consoleplayer], "WINGS OF WRATH ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCChaosDev(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_teleport, NULL);

	P_SetMessage(&players[consoleplayer], "CHAOS DEVICE ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCSphere(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_invisibility, NULL);

	P_SetMessage(&players[consoleplayer], "SHADOWSPHERE ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCTimebomb(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_firebomb, NULL);

	P_SetMessage(&players[consoleplayer], "TIMEBOMB ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCTome(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GiveArtifact(player, arti_tomeofpower, NULL);

	P_SetMessage(&players[consoleplayer], "TOME OF POWER ADDED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCGod(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	if (player->cheats & CF_GODMODE)
	{
	    player->cheats -= CF_GODMODE;
	    P_SetMessage(&players[consoleplayer], "GOD MODE OFF", true);
	}
	else
	{
	    player->cheats += CF_GODMODE;
	    P_SetMessage(&players[consoleplayer], "GOD MODE ON", true);
	}
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCHealth(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    if(player->chickenTics && !demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
        player->health = player->mo->health = MAXCHICKENHEALTH;
    }
    else
    {
        player->health = player->mo->health = MAXHEALTH;
    }
    P_SetMessage(player, DEH_String(TXT_CHEATHEALTH), false);

    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCKill(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
	P_Massacre();

    P_SetMessage(player, DEH_String("MASSACRE"), false);
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCNoclip(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	if (player->cheats & CF_NOCLIP)
	{
	    player->cheats -= CF_NOCLIP;
	    P_SetMessage(&players[consoleplayer], "NOCLIP MODE OFF", true);
	}
	else
	{
	    player->cheats += CF_NOCLIP;
	    P_SetMessage(&players[consoleplayer], "NOCLIP MODE ON", true);
	}
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCChicken(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    extern boolean P_UndoPlayerChicken(player_t *player);

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && player->chickenTics && players[consoleplayer].playerstate == PST_LIVE)
    {
        if (P_UndoPlayerChicken(player))
        {
            P_SetMessage(player, DEH_String(TXT_CHEATCHICKENOFF), false);
        }
    }
    else if (P_ChickenMorphPlayer(player))
    {
        P_SetMessage(player, DEH_String(TXT_CHEATCHICKENON), false);
    }

    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCIDDQD(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	P_DamageMobj(player->mo, NULL, player->mo, 10000);
	P_SetMessage(player, DEH_String(TXT_CHEATIDDQD), true);
    }
    suicide = 1;
    DetectState();
    suicide = 0;
    S_StartSound(NULL, sfx_chat);
}

static void SCIDKFA(int option)
{
    static player_t* player;
    player = &players[consoleplayer];

    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	int i;

	if (player->chickenTics)
	{
	    return;
	}
	for (i = 1; i < 8; i++)
	{
	    player->weaponowned[i] = false;
	}
	player->pendingweapon = wp_staff;
	P_SetMessage(player, DEH_String(TXT_CHEATIDKFA), true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void DrawKeysMenu(void)
{
}

static void DrawArmorMenu(void)
{
}

static void DrawWeaponsMenu(void)
{
    if(HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
	WeaponsMenu.itemCount = 7;
}

static void DrawArtifactsMenu(void)
{
    if(HERETIC_BETA || HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2)
	ArtifactsMenu.itemCount = 10;
}

static void DrawCheatsMenu(void)
{
    static player_t* player;
    player = &players[consoleplayer];

    if (player->cheats & CF_GODMODE)
	MN_DrTextA("ON", 225, 10);
    else
	MN_DrTextA("OFF", 225, 10);

    if(!cheating)
	MN_DrTextA("OFF", 225, 70);
    else if (cheating && cheeting!=2)
	MN_DrTextA("WALLS", 225, 70);
    else if (cheating && cheeting==2)
	MN_DrTextA("WALLS / ITEMS", 225, 70);

    if(epi == 1 && map == 0)
	map = 1;
    else if(epi == 5 && map == 10)
	map = 9;

    if(epi == 1)
	MN_DrTextA(maptext[map], 70, 100);
    else if(epi == 2)
	MN_DrTextA(maptext[map+9], 70, 100);
    else if(epi == 3)
	MN_DrTextA(maptext[map+18], 70, 100);
    else if(epi == 4)
	MN_DrTextA(maptext[map+27], 70, 100);
    else if(epi == 5)
	MN_DrTextA(maptext[map+36], 70, 100);
    else if(epi == 6)
	MN_DrTextA(maptext[map+45], 70, 100);

    if (player->cheats & CF_NOCLIP)
	MN_DrTextA("ON", 225, 150);
    else
	MN_DrTextA("OFF", 225, 150);

    SB_state = -1;      //refresh the statbar
    BorderNeedRefresh = true;
}

static void DrawRecordMenu(void)
{
    if(rskill == 0)
	MN_DrTextB("BABY", 200, 110);
    else if(rskill == 1)
	MN_DrTextB("EASY", 200, 110);		
    else if(rskill == 2)
	MN_DrTextB("MEDIUM", 200, 110);
    else if(rskill == 3)
	MN_DrTextB("HARD", 200, 110);
    else if(rskill == 4)
	MN_DrTextB("NIGHTMARE", 200, 110);

    if(repi == 1 && rmap == 0)
	rmap = 1;
    else if(repi == 5 && rmap == 10)
	rmap = 9;

    if(repi == 1)
	MN_DrTextB(maptext[rmap], 70, 70);
    else if(repi == 2)
	MN_DrTextB(maptext[rmap+9], 70, 70);
    else if(repi == 3)
	MN_DrTextB(maptext[rmap+18], 70, 70);
    else if(repi == 4)
	MN_DrTextB(maptext[rmap+27], 70, 70);
    else if(repi == 5)
	MN_DrTextB(maptext[rmap+36], 70, 70);
    else if(repi == 6)
	MN_DrTextB(maptext[rmap+45], 70, 70);
}

static void SCRMap(int option)
{
    if(option == RIGHT_DIR && !demoplayback)
    {
	if(repi <= 5 && rmap <= 9)
	{
	    rmap++;
	    if(repi == 1 && rmap == 4 && HERETIC_BETA)
	    {
		repi = 1;
		rmap = 3;
	    }
	    else if(repi == 1 && rmap == 10 && (HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2))
	    {
		repi = 1;
		rmap = 9;
	    }
	    else if(repi == 1 && rmap == 10 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	    {
		repi = 2;
		rmap = 1;
	    }
	    else if(repi == 2 && rmap == 10 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	    {
		repi = 3;
		rmap = 1;
	    }
	    else if(repi == 3 && rmap == 10 && (HERETIC_REG_1_0 || HERETIC_REG_1_2))
	    {
		repi = 3;
		rmap = 9;
	    }
	    else if(repi == 3 && rmap == 10 && HERETIC_REG_1_3)
	    {
		repi = 4;
		rmap = 1;
	    }
	    else if(repi == 4 && rmap == 10 && HERETIC_REG_1_3)
	    {
		repi = 5;
		rmap = 1;
	    }								// UNFINISHED BUSINESS ???
	    else if(epi == 5 && map == 10 && HERETIC_REG_1_3)
	    {
		epi = 6;
		map = 1;
	    }
	    else if(epi == 6 && map == 4 && HERETIC_REG_1_3)
	    {
		epi = 6;
		map = 3;
	    }
	}
    }
    else if(repi >= 1 && rmap >= 1)
    {
	if(!demoplayback)
	{
	    rmap--;							// UNFINISHED BUSINESS ???
	    if(epi == 6 && map == 0 && HERETIC_REG_1_3)
	    {
	    	epi = 5;
	    	map = 9;
	    }
	    else if(repi == 5 && rmap == 0 && HERETIC_REG_1_3)
	    {
	    	repi = 4;
	    	rmap = 9;
	    }
	    else if(repi == 4 && rmap == 0 && HERETIC_REG_1_3)
	    {
	    	repi = 3;
	    	rmap = 9;
	    }
	    else if(repi == 3 && rmap == 0 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	    {
	    	repi = 2;
	    	rmap = 9;
	    }
	    else if(repi == 2 && rmap == 0 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	    {
	    	repi = 1;
	    	rmap = 9;
	    }
        }
    }
    if(demoplayback)
    {
	P_SetMessage(&players[consoleplayer], "CANNOT CHANGE STATE WHILE A DEMO IS RUNNING", true);
	S_StartSound(NULL, sfx_chat);
    }
}

static void SCRSkill(int option)
{
    if(option == RIGHT_DIR && !demoplayback)
    {
	if(rskill < 4)
	    rskill++;
    }
    else if(rskill && !demoplayback)
	rskill--;

    if(demoplayback)
    {
	P_SetMessage(&players[consoleplayer], "CANNOT CHANGE STATE WHILE A DEMO IS RUNNING", true);
	S_StartSound(NULL, sfx_chat);
    }
}

static void SCRecord(int option)
{
    if(!demoplayback)
    {
//	SB_SetClassData();
	MN_DeactivateMenu();
//	SB_state = -1;
//	BorderNeedRefresh = true;
//	G_StartNewInit();
	G_RecordDemo(rskill, 1, repisode, rmap);
	D_DoomLoop();               // never returns
    }
    else if(demoplayback)
    {
	P_SetMessage(&players[consoleplayer], "CANNOT CHANGE STATE WHILE A DEMO IS RUNNING", true);
	S_StartSound(NULL, sfx_chat);
    }
}

static void SCMap(int option)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	cheating = (cheating+1) % 3;
	cheeting = (cheeting+1) % 3;

	if(!cheating)
	    P_SetMessage(&players[consoleplayer], "MAP BACK TO NORMAL", true);
	else if (cheating && cheeting!=2)
	    P_SetMessage(&players[consoleplayer], "ALL WALLS REVEALED", true);
	else if (cheating && cheeting==2)
	    P_SetMessage(&players[consoleplayer], "ALL WALLS / ITEMS REVEALED", true);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

static void SCWarp(int option)
{
    if(option == RIGHT_DIR)
    {
	if(epi <= 6 && map <= 9)
	{
	    map++;
	    if(epi == 1 && map == 4 && HERETIC_BETA)
	    {
		epi = 1;
		map = 3;
	    }
	    else if(epi == 1 && map == 10 && (HERETIC_SHARE_1_0 || HERETIC_SHARE_1_2))
	    {
		epi = 1;
		map = 9;
	    }
	    else if(epi == 1 && map == 10 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	    {
		epi = 2;
		map = 1;
	    }
	    else if(epi == 2 && map == 10 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	    {
		epi = 3;
		map = 1;
	    }
	    else if(epi == 3 && map == 10 && (HERETIC_REG_1_0 || HERETIC_REG_1_2))
	    {
		epi = 3;
		map = 9;
	    }
	    else if(epi == 3 && map == 10 && HERETIC_REG_1_3)
	    {
		epi = 4;
		map = 1;
	    }
	    else if(epi == 4 && map == 10 && HERETIC_REG_1_3)
	    {
		epi = 5;
		map = 1;
	    }								// UNFINISHED BUSINESS ???
	    else if(epi == 5 && map == 10 && HERETIC_REG_1_3)
	    {
		epi = 6;
		map = 1;
	    }
	    else if(epi == 6 && map == 4 && HERETIC_REG_1_3)
	    {
		epi = 6;
		map = 3;
	    }
	}
    }
    else if(epi >= 1 && map >= 1)
    {
	map--;								// UNFINISHED BUSINESS ???
	if(epi == 6 && map == 0 && HERETIC_REG_1_3)
	{
	    epi = 5;
	    map = 9;
	}
	else if(epi == 5 && map == 0 && HERETIC_REG_1_3)
	{
	    epi = 4;
	    map = 9;
	}
	else if(epi == 4 && map == 0 && HERETIC_REG_1_3)
	{
	    epi = 3;
	    map = 9;
	}
	else if(epi == 3 && map == 0 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	{
	    epi = 2;
	    map = 9;
	}
	else if(epi == 2 && map == 0 && (HERETIC_REG_1_0 || HERETIC_REG_1_2 || HERETIC_REG_1_3))
	{
	    epi = 1;
	    map = 9;
	}
    }
}

static void SCWarpNow(int option)
{
    if(gamestate == GS_LEVEL && gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE && !demoplayback)
    {
	if(forced)
	    forced = false;

	warped = 1;
        G_DeferedInitNew(gameskill, epi, map);
    }
    DetectState();
    S_StartSound(NULL, sfx_chat);
}

