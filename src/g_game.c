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

// G_game.c
/*
#include <pspkernel.h>
#include <psprtc.h>
*/
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "doomkeys.h"
#include "deh_str.h"
#include "i_timer.h"
#include "i_system.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

// Macros

#define AM_STARTKEY     9

// Functions

boolean G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t * cmd);
void G_WriteDemoTiccmd(ticcmd_t * cmd);
void G_PlayerReborn(int player);

void G_DoReborn(int playernum);

void G_DoLoadLevel(void);
void G_DoNewGame(void);
void G_DoPlayDemo(void);
void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);
void G_DoSaveGame(void);

void D_PageTicker(void);
void D_AdvanceDemo(void);

struct
{
    mobjtype_t type;
    int speed[2];
} MonsterMissileInfo[] = {
    { MT_IMPBALL, { 10, 20 } },
    { MT_MUMMYFX1, { 9, 18 } },
    { MT_KNIGHTAXE, { 9, 18 } },
    { MT_REDAXE, { 9, 18 } },
    { MT_BEASTBALL, { 12, 20 } },
    { MT_WIZFX1, { 18, 24 } },
    { MT_SNAKEPRO_A, { 14, 20 } },
    { MT_SNAKEPRO_B, { 14, 20 } },
    { MT_HEADFX1, { 13, 20 } },
    { MT_HEADFX3, { 10, 18 } },
    { MT_MNTRFX1, { 20, 26 } },
    { MT_MNTRFX2, { 14, 20 } },
    { MT_SRCRFX1, { 20, 28 } },
    { MT_SOR2FX1, { 20, 28 } },
    { -1, { -1, -1 } }                 // Terminator
};

gameaction_t gameaction;
gamestate_t gamestate;
skill_t gameskill;
boolean respawnmonsters;
int gameepisode;
int gamemap;
int prevmap;

boolean paused;
boolean sendpause;              // send a pause event next tic
boolean sendsave;               // send a save event next tic
boolean usergame;               // ok to save / end game

boolean timingdemo;             // if true, exit with report on completion
int starttime;                  // for comparative timing purposes

boolean viewactive;
/*
boolean deathmatch;             // only if started as net death
boolean netgame;                // only true if packets are broadcast
*/
boolean playeringame[MAXPLAYERS];
player_t players[MAXPLAYERS];

int consoleplayer;              // player taking events and displaying
int displayplayer;              // view being displayed
int gametic;
int levelstarttic;              // gametic at level start
int totalkills, totalitems, totalsecret;        // for intermission

extern int mouseSensitivity;

char demoname[32];
boolean demorecording;
boolean demoplayback;
byte *demobuffer, *demo_p;
boolean singledemo;             // quit after playing a demo from cmdline

boolean precache = true;        // if true, load all graphics at start

// TODO: Heretic uses 16-bit shorts for consistency?
byte consistancy[MAXPLAYERS][BACKUPTICS];
char *savegamedir;
/*
boolean testcontrols = false;
int testcontrols_mousespeed;
*/

//
// controls (have defaults)
//

#define MAXPLMOVE       0x32

extern int		turnspeed;
int		turnspd;

extern fixed_t         forwardmove/*[2] = {0x19, 0x32}*/; 
extern fixed_t         sidemove/*[2] = {0x18, 0x28}*/; 
fixed_t		forwardmve;
fixed_t		sidemve;

fixed_t angleturn/*[3] = { 640, 1280, 320 }*/;      // + slow turn
/*
static int *weapon_keys[] =
{
    &key_weapon1,
    &key_weapon2,
    &key_weapon3,
    &key_weapon4,
    &key_weapon5,
    &key_weapon6,
    &key_weapon7
};

// Set to -1 or +1 to switch to the previous or next weapon.

static int next_weapon = 0;
*/
// Used for prev/next weapon keys.

static const struct
{
    weapontype_t weapon;
    weapontype_t weapon_num;
} weapon_order_table[] = {
    { wp_staff,       wp_staff },
    { wp_gauntlets,   wp_staff },
    { wp_goldwand,    wp_goldwand },
    { wp_crossbow,    wp_crossbow },
    { wp_blaster,     wp_blaster },
    { wp_skullrod,    wp_skullrod },
    { wp_phoenixrod,  wp_phoenixrod },
    { wp_mace,        wp_mace },
    { wp_beak,        wp_beak },
};
/*
#define SLOWTURNTICS    6

int turnheld;                   // for accelerative turning
int lookheld;

boolean mousearray[4];
boolean *mousebuttons = &mousearray[1];	// allow [-1]
*/
int mousex, mousey;             // mouse values are used once
int dclicktime, dclickstate, dclicks;
int dclicktime2, dclickstate2, dclicks2;

#define MAX_JOY_BUTTONS 20

int joyxmove, joyymove;         // joystick values are repeated
boolean joyarray[MAX_JOY_BUTTONS + 1];
boolean *joybuttons = &joyarray[1];     // allow [-1]
static int      joyirx;
static int      joyiry;

int savegameslot;
char savedescription[32];

int inventoryTics;

int key_strafe;
int mouselook;
int joybstrafe;

int joy_a = 1;		// 0
int joy_r = 2;		// 1
int joy_plus = 4;	// 2
int joy_l = 8;		// 3
int joy_minus = 16;	// 4
int joy_b = 32;		// 5
int joy_left = 64;	// 6
int joy_down = 128;	// 7
int joy_right = 256;	// 8
int joy_up = 512;	// 9
int joy_zr = 1024;	// 10
int joy_zl = 2048;	// 11
int joy_home = 4096;	// 12
int joy_x = 8192;	// 13
int joy_y = 16384;	// 14
int joy_1 = 32768;	// 15
int joy_2 = 65536;	// 16

int     joybinvright = 0;
int     joybfire = 1;
int     joybinvuse = 2;
int     joybuse = 3;
int	joybmenu = 4;
int	joybflydown = 5;
int     joybleft = 6;
int	joybmap = 7;
int	joybright = 8;
int	joybcenter = 9;
int	joybmapzoomout = 10;
int	joybmapzoomin = 11;
int	joybjump = 12;
int	joybflyup = 13;
int     joybinvleft = 14;
int	joybspeed = 15;

extern fixed_t mtof_zoommul;    // how far the window zooms in each tic (map coords)
extern fixed_t ftom_zoommul;    // how far the window zooms in each tic (fb coords)

void AM_Start(void);
void AM_Stop(void);

extern boolean askforquit;
extern boolean askforsave;
extern  int typeofask;
extern  int typeofask2;

// haleyjd: removed WATCOMC

//=============================================================================
// Not used - ripped out for Heretic
/*
int G_CmdChecksum(ticcmd_t *cmd)
{
	int     i;
	int sum;

	sum = 0;
	for(i = 0; i < sizeof(*cmd)/4-1; i++)
	{
		sum += ((int *)cmd)[i];
	}
	return(sum);
}


static boolean WeaponSelectable(weapontype_t weapon)
{
    if (weapon == wp_beak)
    {
        return false;
    }

    return players[consoleplayer].weaponowned[weapon];
}

static int G_NextWeapon(int direction)
{
    weapontype_t weapon;
    int i;

    // Find index in the table.

    if (players[consoleplayer].pendingweapon == wp_nochange)
    {
        weapon = players[consoleplayer].readyweapon;
    }
    else
    {
        weapon = players[consoleplayer].pendingweapon;
    }

    for (i=0; i<arrlen(weapon_order_table); ++i)
    {
        if (weapon_order_table[i].weapon == weapon)
        {
            break;
        }
    }

    // Switch weapon.

    do
    {
        i += direction;
        i = (i + arrlen(weapon_order_table)) % arrlen(weapon_order_table);
    } while (!WeaponSelectable(weapon_order_table[i].weapon));

    return weapon_order_table[i].weapon_num;
}
*/

extern boolean automapactive;

#define KEY_1               0x02

void ChangeWeaponRight(void)
{
    static player_t*	plyrweap;
    static event_t	kbevent;

    weapontype_t	num;

    if (gamestate == GS_LEVEL && !(MenuActive || automapactive))
    {
	plyrweap = &players[consoleplayer];

	num = plyrweap->readyweapon;

	while (1)
	{
	    dont_move_forwards = true;

	    num++;

	    if (num > wp_gauntlets)
	        num = wp_staff;

	    if (plyrweap->weaponowned[num])
	    {
	        plyrweap->pendingweapon = num;

	        break;
	    }
        }
        kbevent.type = ev_keydown;
        kbevent.data1 = KEY_1 + num;

        D_PostEvent(&kbevent);

	dont_move_forwards = false;
    }
}

void ChangeWeaponLeft(void)
{
    static player_t*	plyrweap;
    static event_t	kbevent;

    weapontype_t	num;

    if (gamestate == GS_LEVEL && !(MenuActive || automapactive))
    {
	plyrweap = &players[consoleplayer];

	num = plyrweap->readyweapon;

	while (1)
	{
	    dont_move_forwards = true;

	    num--;

	    if (num == -1)
		num = wp_gauntlets;

	    if (plyrweap->weaponowned[num])
	    {
	        plyrweap->pendingweapon = num;

	        break;
	    }
        }
	if (num == wp_gauntlets)
	    num = wp_staff;

        kbevent.type = ev_keydown;
        kbevent.data1 = KEY_1 + num;

        D_PostEvent(&kbevent);

	dont_move_forwards = false;
    }
}

#include <wiiuse/wpad.h>

void ChangeInventoryItemRight(void)
{
    event_t ev; // keyboard event

    int k_inv = 0;

//    if(WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_ZR)
    {
	k_inv = 1; // Select right object
	ev.data1 = ']';
    }

    if(k_inv)
	ev.type = ev_keydown;
    else
	ev.type = ev_keyup;

    D_PostEvent(&ev);
}

void ChangeInventoryItemLeft(void)
{
    event_t ev; // keyboard event

    int k_inv = 0;

//    if(WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_ZL)
    {
	k_inv = 1; // Select right object
	ev.data1 = '[';
    }

    if(k_inv)
	ev.type = ev_keydown;
    else
	ev.type = ev_keyup;

    D_PostEvent(&ev);
}

/*
====================
=
= G_BuildTiccmd
=
= Builds a ticcmd from all of the available inputs or reads it from the
= demo buffer.
= If recording a demo, write it out
====================
*/

extern boolean inventory;
extern int curpos;
extern int inv_ptr;

boolean usearti = true;

void G_BuildTiccmd(ticcmd_t *cmd, int maketic)
{
//    int i;
    boolean strafe/*, use*/;
//    int speed, tspeed, lspeed;
    int forward, side;
    int look, arti;
    int flyheight;

//    extern boolean noartiskip;

    // haleyjd: removed externdriver crap

    memset(cmd, 0, sizeof(*cmd));
    //cmd->consistancy =
    //      consistancy[consoleplayer][(maketic*ticdup)%BACKUPTICS];
    cmd->consistancy = consistancy[consoleplayer][maketic % BACKUPTICS];

//printf ("cons: %i\n",cmd->consistancy);

    strafe = gamekeydown[key_strafe] /*|| mousebuttons[mousebstrafe]*/
        || joybuttons[joybstrafe];
/*
    speed = joybspeed >= MAX_JOY_BUTTONS
         || !gamekeydown[key_speed]
         || joybuttons[joybspeed];
*/
    // haleyjd: removed externdriver crap
    
    forward = side = look = arti = flyheight = 0;

//
// use two stage accelerative turning on the keyboard and joystick
//
/*
    if (joyxmove < 0 || joyxmove > 0
        || gamekeydown[key_right] || gamekeydown[key_left])
        turnheld += ticdup;
    else
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    if (gamekeydown[key_lookdown] || gamekeydown[key_lookup])
    {
        lookheld += ticdup;
    }
    else
    {
        lookheld = 0;
    }
    if (lookheld < SLOWTURNTICS)
    {
        lspeed = 1;
    }
    else
    {
        lspeed = 2;
    }
*/
// let movement keys cancel each other out
//
//    extern int button_layout;

    if(strafe /*|| button_layout == 1*/)
    {
	if (gamekeydown[key_right])
	{
	    side += sidemove/*[speed]*/;
	}
	if (gamekeydown[key_left])
	{
	    side -= sidemove/*[speed]*/;
	}

        if (joyxmove > 0) 
            side += sidemove/*[speed]*/; 
        if (joyxmove < 0) 
            side -= sidemove/*[speed]*/; 
/*
        if (joyxmove > 0)
            side += sidemove[speed];
        if (joyxmove < 0)
            side -= sidemove[speed];
*/
    }
    else
    {
	if (gamekeydown[key_right])
	    cmd->angleturn -= angleturn/*[tspeed]*/;
	if (gamekeydown[key_left])
	    cmd->angleturn += angleturn/*[tspeed]*/;

        if (joyxmove > 20) 
            side += sidemve/*[speed]*/; 
//            cmd->angleturn -= angleturn[tspeed]; 
        if (joyxmove < -20) 
            side -= sidemve/*[speed]*/; 
//            cmd->angleturn += angleturn[tspeed]; 

        if (joyirx > 0)     // calculate wii IR curve based on input
            cmd->angleturn -= turnspd * joyirx;
        if (joyirx < 0)     // calculate wii IR curve based on input
            cmd->angleturn -= turnspd * joyirx;
/*
        if (joyxmove > 0)
            cmd->angleturn -= angleturn[tspeed];
        if (joyxmove < 0)
            cmd->angleturn += angleturn[tspeed];
*/
    }

    extern boolean dont_move_forwards;
    extern boolean dont_move_backwards;

    if (gamekeydown[key_up])
    {
	if(button_layout == 0)
	{
	    if(dont_move_forwards == true)
		forward += forwardmove/*[speed]*/;
	}
	else if(button_layout == 1 && !gamekeydown[key_use])
	    forward += forwardmove/*[speed]*/;
    }

    if (gamekeydown[key_down])
    {
	if(button_layout == 0)
	{
	    if(dont_move_backwards == true)
		forward -= forwardmove/*[speed]*/;
	}
	else if(button_layout == 1 && !gamekeydown[key_use])
	    forward -= forwardmove/*[speed]*/;
    }

    if (joyymove > 20)
        forward += forwardmve/*[speed]*/;
    if (joyymove < -20)
        forward -= forwardmve/*[speed]*/;

    if (/*gamekeydown[key_straferight] || mousebuttons[mousebstraferight]
     ||*/ joybuttons[joybstraferight])
        side += sidemove/*[speed]*/;
    if (/*gamekeydown[key_strafeleft] || mousebuttons[mousebstrafeleft]
     ||*/ joybuttons[joybstrafeleft])
        side -= sidemove/*[speed]*/;

    if (joybuttons[joybspeed]) 
    {
	forwardmve = forwardmove * 6;
	sidemve = sidemove * 6;
//	turnspd = turnspeed * 6;
    }
    else if(!joybuttons[joybspeed])
    {
	forwardmve = forwardmove;
	sidemve = sidemove;
//	turnspd = turnspeed;
    }
    turnspd = turnspeed;
/*
    // Look up/down/center keys
    if (gamekeydown[key_lookup])
    {
        look = lspeed;
    }
    if (gamekeydown[key_lookdown])
    {
        look = -lspeed;
    }

    // haleyjd: removed externdriver crap
    if (gamekeydown[key_lookcenter])
    {
        look = TOCENTER;
    }
*/
    // haleyjd: removed externdriver crap
    
    if(mouselook == 0)
	look = TOCENTER;

    // Fly up/down/drop keys
    if (gamekeydown[key_flyup] || joybuttons[joybflyup])
    {
        flyheight = 5;          // note that the actual flyheight will be twice this
    }
    if (gamekeydown[key_flydown] || joybuttons[joybflydown])
    {
        flyheight = -5;
    }
/*
    if (gamekeydown[key_flycenter])
    {
        flyheight = TOCENTER;
        // haleyjd: removed externdriver crap
        look = TOCENTER;
    }

    // Use artifact key
    if (gamekeydown[key_useartifact] || joybuttons[joybinvuse])
    {
        if (gamekeydown[key_speed] && !noartiskip)
        {
            if (players[consoleplayer].inventory[inv_ptr].type != arti_none)
            {
                gamekeydown[key_useartifact] = false;
		joybuttons[joybinvuse] = false;
                cmd->arti = 0xff;       // skip artifact code
            }
        }
        else
        {
            if (inventory)
            {
                players[consoleplayer].readyArtifact =
                    players[consoleplayer].inventory[inv_ptr].type;
                inventory = false;
                cmd->arti = 0;
                usearti = false;
            }
            else if (usearti)
            {
                cmd->arti = players[consoleplayer].inventory[inv_ptr].type;
                usearti = false;
            }
        }
    }
*/
    if (gamekeydown[127] && !cmd->arti
        && !players[consoleplayer].powers[pw_weaponlevel2])
    {
        gamekeydown[127] = false;
        cmd->arti = arti_tomeofpower;
    }

    if (/*gamekeydown[key_jump] || mousebuttons[mousebjump]
	||*/ joybuttons[joybjump] && !MenuActive)
    {
	if(!demoplayback)
	    cmd->arti |= AFLAG_JUMP;
    }

//
// buttons
//
//    cmd->chatchar = CT_dequeueChatChar();

    if (gamekeydown[key_fire] /*|| mousebuttons[mousebfire]*/
        || joybuttons[joybfire])
        cmd->buttons |= BT_ATTACK;

    WPADData *data = WPAD_Data(0);

    if(data->exp.type == WPAD_EXP_CLASSIC)
    {
	if(data->btns_d)
	{
	    if(joybuttons[joybmenu])
	    {
		if (!MenuActive)
		    MN_ActivateMenu();
		else
		    MN_DeactivateMenu();

		if (askforquit)
		{
		    askforquit = false;
		    typeofask = 0;
		    MN_DeactivateMenu();
		}

		if (askforsave)
		{
		    askforsave = false;
		    typeofask2 = 0;
		    MN_DeactivateMenu();
		}
	    }

	    if(!demoplayback)
	    {
		if(joybuttons[joybinvuse])
		{                           // flag to denote that it's okay to use an artifact
		    if (inventory)
		    {
			players[consoleplayer].readyArtifact =
				players[consoleplayer].inventory[inv_ptr].type;
			inventory = false;
			cmd->arti = 0;
			usearti = false;
		    }
		    else /*if (usearti)*/
		    {
			cmd->arti = players[consoleplayer].inventory[inv_ptr].type;
			usearti = false;
		    }
		}

		if(joybuttons[joybright])
		    ChangeWeaponRight();

		if(joybuttons[joybleft])
		    ChangeWeaponLeft();

		if(joybuttons[joybinvright])
		    ChangeInventoryItemRight();

		if(joybuttons[joybinvleft])
		    ChangeInventoryItemLeft();

		if(joybuttons[joybcenter])
		    look = TOCENTER;

		if(joybuttons[joybmap])
		{
		    if (!automapactive)
		    {
			if(!MenuActive)
			    AM_Start ();
		    }
		    else
		    {
			if(!MenuActive)
			{
			    AM_Stop ();

			    extern int screenblocks;

			    R_SetViewSize (screenblocks, detailLevel);
			}
		    }
		}

		if(automapactive)
		{
		    if(joybuttons[joybmapzoomin])
		    {
			mtof_zoommul = M_ZOOMIN;
			ftom_zoommul = M_ZOOMOUT;
		    }

		    if(joybuttons[joybmapzoomout])
		    {
			mtof_zoommul = M_ZOOMOUT;
			ftom_zoommul = M_ZOOMIN;
		    }
		}
	    }
	}
    }

    if(automapactive)
    {
	if(!(joybuttons[joybmapzoomin] || joybuttons[joybmapzoomout]))
	{
	    mtof_zoommul = FRACUNIT;
	    ftom_zoommul = FRACUNIT;
	}
    }

    if (gamekeydown[key_use] || joybuttons[joybuse] /*|| mousebuttons[mousebuse]*/)
    {
        cmd->buttons |= BT_USE;
        dclicks = 0;            // clear double clicks if hit use button
    }

    // If the previous or next weapon button is pressed, the
    // next_weapon variable is set to change weapons when
    // we generate a ticcmd.  Choose a new weapon.
    // (Can't weapon cycle when the player is a chicken)
/*
    if (players[consoleplayer].chickenTics == 0 && next_weapon != 0)
    {
        i = G_NextWeapon(next_weapon);
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= i << BT_WEAPONSHIFT;
    }
    else
    {
        for (i=0; i<arrlen(weapon_keys); ++i)
        {
            int key = *weapon_keys[i];

            if (gamekeydown[key])
            {
                cmd->buttons |= BT_CHANGE; 
                cmd->buttons |= i<<BT_WEAPONSHIFT; 
                break; 
            }
        }
    }

    next_weapon = 0;

//
// mouse
//
    use = gamekeydown[key_use] || joybuttons[joybuse];
    if (use != dclickstate2 && dclicktime2 > 1 )
    {
	dclickstate2 = use;
	if (dclickstate2)
		dclicks2++;
	if (dclicks2 == 2)
	{
		cmd->buttons |= BT_USE;
		dclicks2 = 0;
	players[consoleplayer].lookdir = 0;
	}
	else
		dclicktime2 = 0;
    }
    else
    {
	dclicktime2 += ticdup;
	if (dclicktime2 > 20)
	{
		dclicks2 = 0;
		dclickstate2 = 0;
	}
    }

    if (mousebuttons[mousebforward])
    {
        forward += forwardmove[speed];
    }

    if (mousebuttons[mousebbackward])
    {
	forward -= forwardmove[speed];
    }

    // Double click to use can be disabled 
   
    if (dclick_use)
    {
	//
	// forward double click
	//
	if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1)
	{
	    dclickstate = mousebuttons[mousebforward];
	    if (dclickstate)
		dclicks++;
	    if (dclicks == 2)
	    {
		cmd->buttons |= BT_USE;
		dclicks = 0;
	    }
	    else
		dclicktime = 0;
	}
	else
	{
	    dclicktime += ticdup;
	    if (dclicktime > 20)
	    {
		dclicks = 0;
		dclickstate = 0;
	    }
	}

	//
	// strafe double click
	//

	bstrafe = mousebuttons[mousebstrafe] || joybuttons[joybstrafe];
	if (bstrafe != dclickstate2 && dclicktime2 > 1)
	{
	    dclickstate2 = bstrafe;
	    if (dclickstate2)
		dclicks2++;
	    if (dclicks2 == 2)
	    {
		cmd->buttons |= BT_USE;
		dclicks2 = 0;
	    }
	    else
		dclicktime2 = 0;
	}
	else
	{
	    dclicktime2 += ticdup;
	    if (dclicktime2 > 20)
	    {
		dclicks2 = 0;
		dclickstate2 = 0;
	    }
	}
    }

    if(button_layout == 0)
*/
    {
	if (strafe)
	    side += mousex*/*2*/0.5;
	else
	    cmd->angleturn -= mousex*0x8;
    }
/*
    else if(button_layout == 1)
    {
    	if (strafe)
    	    side += mousex*2;
    	else
    	    cmd->angleturn -= mousex*0x8;
    }

    // No mouse movement in previous frame?

    if (mousex == 0)
    {
        testcontrols_mousespeed = 0;
    }
*/
    forward += mousey;

    // mouselook, but not when paused
    if (/*joybuttons[joybuse] &&*/ joyiry && !paused && mouselook > 0)	// FOR PSP: mouselook, but...
    {									// ...not when paused & if on
	// We'll directly change the viewing pitch of the console player.
	float adj = ((joyiry * 0x4) << 16) / (float) 0x80000000*180*110.0/85.0;

	// initialiser added to prevent compiler warning
	float newlookdir = 0;

	extern int mspeed;

	// Speed up the X11 mlook a little.
	adj *= mspeed;

	if (mouselook == 1)
	    newlookdir = players[consoleplayer].lookdir + adj;
	else if (mouselook == 2)
	    newlookdir = players[consoleplayer].lookdir - adj;

	// vertical view angle taken from p_user.c line 249.
	if (newlookdir > 90)
	    newlookdir = 90;
	else if (newlookdir < -110)
	    newlookdir = -110;

	players[consoleplayer].lookdir = newlookdir;
/*
	cmd->buttons |= BT_USE;

	// clear double clicks if hit use button
	dclicks = 0;

	dont_move_forwards = false;
*/
    }
/*
    else if (joybuttons[joybuse] && !paused && mouselook > 0)
    {
	cmd->buttons |= BT_USE;

	// clear double clicks if hit use button
	dclicks = 0;
    }
*/
    mousex = mousey = 0;

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    cmd->forwardmove += forward;
    cmd->sidemove += side;
    if (players[consoleplayer].playerstate == PST_LIVE)
    {
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }
    if (flyheight < 0)
    {
        flyheight += 16;
    }
    cmd->lookfly |= flyheight << 4;

//
// special buttons
//
    if (sendpause)
    {
        sendpause = false;
        cmd->buttons = BT_SPECIAL | BTS_PAUSE;
    }

    if (sendsave)
    {
        sendsave = false;
        cmd->buttons =
            BT_SPECIAL | BTS_SAVEGAME | (savegameslot << BTS_SAVESHIFT);
    }

}


/*
==============
=
= G_DoLoadLevel
=
==============
*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"

void G_DoLoadLevel(void)
{
    int i;

    levelstarttic = gametic;    // for time calculation
    gamestate = GS_LEVEL;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;
        memset(players[i].frags, 0, sizeof(players[i].frags));
    }

    P_SetupLevel(gameepisode, gamemap, 0, gameskill);
    displayplayer = consoleplayer;      // view the guy you are playing
    starttime = I_GetTime();
    gameaction = ga_nothing;
    Z_CheckHeap();

//
// clear cmd building stuff
//

    memset(gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joyirx = joyiry = 0; 
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;

//    memset(mousebuttons, 0, sizeof(mousebuttons));
    memset(joybuttons, 0, sizeof(joybuttons));
/*
    if (testcontrols)
    {
        P_SetMessage(&players[consoleplayer], "PRESS ESCAPE TO QUIT.", false);
    }
*/
}
/*
static void SetJoyButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_JOY_BUTTONS; ++i)
    {
        joybuttons[i] = (buttons_mask & (1 << i)) != 0;
    }
}

===============================================================================
=
= G_Responder
=
= get info needed to make ticcmd_ts for the players
=
===============================================================================
*/

boolean G_Responder(event_t * ev)
{
//    WPADData *data = WPAD_Data(0);

    player_t *plr;

    plr = &players[consoleplayer];
/*
    if (ev->type == ev_keyup && ev->data1 == key_useartifact)
    {                           // flag to denote that it's okay to use an artifact
        if (!inventory)
        {
            plr->readyArtifact = plr->inventory[inv_ptr].type;
        }
        usearti = true;
    }

    if(ev->type == ev_joystick && ev->data1 == 4 && data->btns_d)
    {                           // flag to denote that it's okay to use an artifact
        if (!inventory)
        {
            plr->readyArtifact = plr->inventory[inv_ptr].type;
        }
        usearti = true;
    }
    // Check for spy mode player cycle

    if (gamestate == GS_LEVEL && ev->type == ev_keydown
        && ev->data1 == KEY_F12 && !deathmatch)
    {                           // Cycle the display player
        do
        {
            displayplayer++;
            if (displayplayer == MAXPLAYERS)
            {
                displayplayer = 0;
            }
        }
        while (!playeringame[displayplayer]
               && displayplayer != consoleplayer);
        return (true);
    }
*/
    if (gamestate == GS_LEVEL)
    {
/*
        if (CT_Responder(ev))
        {                       // Chat ate the event
            return (true);
        }
*/
        if (SB_Responder(ev))
        {                       // Status bar ate the event
            return (true);
        }
        if (AM_Responder(ev))
        {                       // Automap ate the event
            return (true);
        }
    }
/*
    if (ev->type == ev_mouse)
    {
        testcontrols_mousespeed = abs(ev->data2);
    }

    if (ev->type == ev_keydown && ev->data1 == key_prevweapon)
    {
        next_weapon = -1;
    }
    else if (ev->type == ev_keydown && ev->data1 == key_nextweapon)
    {
        next_weapon = 1;
    }
*/
    switch (ev->type)
    {
        case ev_keydown:

            if (joybuttons[joybinvleft])
            {
                inventoryTics = 5 * 35;
                if (!inventory)
                {
                    inventory = true;
                    break;
                }
                inv_ptr--;
                if (inv_ptr < 0)
                {
                    inv_ptr = 0;
                }
                else
                {
                    curpos--;
                    if (curpos < 0)
                    {
                        curpos = 0;
                    }
                }
                return (true);
            }
	    if(joybuttons[joybinvright])
            {
                inventoryTics = 5 * 35;
                if (!inventory)
                {
                    inventory = true;
                    break;
                }
                inv_ptr++;
                if (inv_ptr >= plr->inventorySlotNum)
                {
                    inv_ptr--;
                    if (inv_ptr < 0)
                        inv_ptr = 0;
                }
                else
                {
                    curpos++;
                    if (curpos > 6)
                    {
                        curpos = 6;
                    }
                }
                return (true);
            }
/*
            if (ev->data1 == KEY_PAUSE && !MenuActive)
            {
                sendpause = true;
                return (true);
            }
*/
            if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = true;
            }
            return (true);      // eat key down events

        case ev_keyup:
            if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = false;
            }
            return (false);     // always let key up events filter down

        case ev_mouse:
/*
            mousebuttons[0] = ev->data1 & 1;
            mousebuttons[1] = ev->data1 & 2;
            mousebuttons[2] = ev->data1 & 4;
*/
            mousex = ev->data2 * (mouseSensitivity + 5) / 7/*10*/;
            mousey = ev->data3 * (mouseSensitivity + 5) / 7/*10*/;
            return (true);      // eat events

        case ev_joystick:
//            SetJoyButtons(ev->data1);
	    joybuttons[0] = (ev->data1 & joy_a) > 0;
	    joybuttons[1] = (ev->data1 & joy_r) > 0;
	    joybuttons[2] = (ev->data1 & joy_plus) > 0;
	    joybuttons[3] = (ev->data1 & joy_l) > 0;
	    joybuttons[4] = (ev->data1 & joy_minus) > 0;
	    joybuttons[5] = (ev->data1 & joy_b) > 0;
	    joybuttons[6] = (ev->data1 & joy_left) > 0;
	    joybuttons[7] = (ev->data1 & joy_down) > 0;
	    joybuttons[8] = (ev->data1 & joy_right) > 0;
	    joybuttons[9] = (ev->data1 & joy_up) > 0;
	    joybuttons[10] = (ev->data1 & joy_zr) > 0;
	    joybuttons[11] = (ev->data1 & joy_zl) > 0;
	    joybuttons[12] = (ev->data1 & joy_home) > 0;
	    joybuttons[13] = (ev->data1 & joy_x) > 0;
	    joybuttons[14] = (ev->data1 & joy_y) > 0;
	    joybuttons[15] = (ev->data1 & joy_1) > 0;
	    joybuttons[16] = (ev->data1 & joy_2) > 0;
	    joyxmove = ev->data2; 
	    joyymove = ev->data3; 
            joyirx = ev->data4;
            joyiry = ev->data5;
            return (true);      // eat events

        default:
            break;
    }
    return (false);
}

/*
===============================================================================
=
= G_Ticker
=
===============================================================================
*/

void G_Ticker(void)
{
    int i/*, buf*/;
    ticcmd_t *cmd = NULL;

//
// do player reborns if needed
//
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && players[i].playerstate == PST_REBORN)
            G_DoReborn(i);

//
// do things to change the game state
//
    while (gameaction != ga_nothing)
    {
        switch (gameaction)
        {
            case ga_loadlevel:
                G_DoLoadLevel();
                break;
            case ga_newgame:
                G_DoNewGame();
                break;
            case ga_loadgame:
                G_DoLoadGame();
                break;
            case ga_savegame:
                G_DoSaveGame();
                break;
            case ga_playdemo:
                G_DoPlayDemo();
                break;
            case ga_screenshot:
//                V_ScreenShot("HTIC%02i.pcx");
                gameaction = ga_nothing;
                break;
            case ga_completed:
                G_DoCompleted();
                break;
            case ga_worlddone:
                G_DoWorldDone();
                break;
            case ga_victory:
                F_StartFinale();
                break;
            default:
                break;
        }
    }


//
// get commands, check consistancy, and build new consistancy check
//
    //buf = gametic%BACKUPTICS;
//    buf = (gametic / ticdup) % BACKUPTICS;

    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
        {
            cmd = &players[i].cmd;

            memcpy(cmd, &netcmds[i], sizeof(ticcmd_t));

            if (demoplayback)
                G_ReadDemoTiccmd(cmd);
            if (demorecording)
                G_WriteDemoTiccmd(cmd);
/*
            if (netgame && !(gametic % ticdup))
            {
                if (gametic > BACKUPTICS
                    && consistancy[i][buf] != cmd->consistancy)
                {
                    I_Error("consistency failure (%i should be %i)",
                            cmd->consistancy, consistancy[i][buf]);
                }
                if (players[i].mo)
                    consistancy[i][buf] = players[i].mo->x;
                else
                    consistancy[i][buf] = rndindex;
            }
*/
        }

//
// check for special buttons
//
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
        {
            if (players[i].cmd.buttons & BT_SPECIAL)
            {
                switch (players[i].cmd.buttons & BT_SPECIALMASK)
                {
                    case BTS_PAUSE:
                        paused ^= 1;
                        if (paused)
                        {
                            S_PauseSound();
                        }
                        else
                        {
                            S_ResumeSound();
                        }
                        break;

                    case BTS_SAVEGAME:
                        if (!savedescription[0])
                        {
/*
                            if (netgame)
                            {
                                strncpy(savedescription, DEH_String("NET GAME"),
                                        sizeof(savedescription));
                            }
                            else
*/
                            {
                                strncpy(savedescription, DEH_String("SAVE GAME"),
                                        sizeof(savedescription));
                            }

                            savedescription[sizeof(savedescription) - 1] = '\0';
                        }
                        savegameslot =
                            (players[i].cmd.
                             buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
                        gameaction = ga_savegame;
                        break;
                }
            }
        }
    // turn inventory off after a certain amount of time
    if (inventory && !(--inventoryTics))
    {
        players[consoleplayer].readyArtifact =
            players[consoleplayer].inventory[inv_ptr].type;
        inventory = false;
        cmd->arti = 0;
    }
//
// do main actions
//
//
// do main actions
//
    switch (gamestate)
    {
        case GS_LEVEL:
            P_Ticker();
            SB_Ticker();
            AM_Ticker();
//            CT_Ticker();
            break;
        case GS_INTERMISSION:
            IN_Ticker();
            break;
        case GS_FINALE:
            F_Ticker();
            break;
        case GS_DEMOSCREEN:
            D_PageTicker();
            break;
    }
}


/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Things
==============================================================================
*/

/*
====================
=
= G_InitPlayer
=
= Called at the start
= Called by the game initialization functions
====================
*/

void G_InitPlayer(int player)
{
    // clear everything else to defaults
    G_PlayerReborn(player);
}


/*
====================
=
= G_PlayerFinishLevel
=
= Can when a player completes a level
====================
*/
extern int playerkeys;

void G_PlayerFinishLevel(int player)
{
    player_t *p;
    int i;

/*      // BIG HACK
	inv_ptr = 0;
	curpos = 0;
*/
    // END HACK
    p = &players[player];
    for (i = 0; i < p->inventorySlotNum; i++)
    {
        p->inventory[i].count = 1;
    }
    p->artifactCount = p->inventorySlotNum;

//    if (!deathmatch)
    {
        for (i = 0; i < 16; i++)
        {
            P_PlayerUseArtifact(p, arti_fly);
        }
    }
    memset(p->powers, 0, sizeof(p->powers));
    memset(p->keys, 0, sizeof(p->keys));
    playerkeys = 0;
//      memset(p->inventory, 0, sizeof(p->inventory));
    if (p->chickenTics)
    {
        p->readyweapon = p->mo->special1.i;       // Restore weapon
        p->chickenTics = 0;
    }
    p->messageTics = 0;
    p->lookdir = 0;
    p->mo->flags &= ~MF_SHADOW; // Remove invisibility
    p->extralight = 0;          // Remove weapon flashes
    p->fixedcolormap = 0;       // Remove torch
    p->damagecount = 0;         // No palette changes
    p->bonuscount = 0;
    p->rain1 = NULL;
    p->rain2 = NULL;
    if (p == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
    }
}

/*
====================
=
= G_PlayerReborn
=
= Called after a player dies
= almost everything is cleared and initialized
====================
*/

void G_PlayerReborn(int player)
{
    player_t *p;
    int i;
    int frags[MAXPLAYERS];
    int killcount, itemcount, secretcount;
    boolean secret;
    unsigned int worldTimer;

    worldTimer = players[player].worldTimer;
    secret = false;
    memcpy(frags, players[player].frags, sizeof(frags));
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;
    players[player].worldTimer = worldTimer;

    p = &players[player];
    if (p->didsecret)
    {
        secret = true;
    }
    memset(p, 0, sizeof(*p));

    memcpy(players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_goldwand;
    p->weaponowned[wp_staff] = true;
    p->weaponowned[wp_goldwand] = true;
    p->messageTics = 0;
    p->lookdir = 0;
    p->ammo[am_goldwand] = 50;
    for (i = 0; i < NUMAMMO; i++)
    {
        p->maxammo[i] = maxammo[i];
    }
    if (gamemap == 9 || secret)
    {
        p->didsecret = true;
    }
    if (p == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
        inv_ptr = 0;            // reset the inventory pointer
        curpos = 0;
    }
}

/*
====================
=
= G_CheckSpot
=
= Returns false if the player cannot be respawned at the given mapthing_t spot
= because something is occupying it
====================
*/

void P_SpawnPlayer(mapthing_t * mthing);

boolean G_CheckSpot(int playernum, mapthing_t * mthing)
{
    fixed_t x, y;
    subsector_t *ss;
    unsigned an;
    mobj_t *mo;

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    players[playernum].mo->flags2 &= ~MF2_PASSMOBJ;
    if (!P_CheckPosition(players[playernum].mo, x, y))
    {
        players[playernum].mo->flags2 |= MF2_PASSMOBJ;
        return false;
    }
    players[playernum].mo->flags2 |= MF2_PASSMOBJ;

// spawn a teleport fog
    ss = R_PointInSubsector(x, y);
    an = ((unsigned) ANG45 * (mthing->angle / 45)) >> ANGLETOFINESHIFT;

    mo = P_SpawnMobj(x + 20 * finecosine[an], y + 20 * finesine[an],
                     ss->sector->floorheight + TELEFOGHEIGHT, MT_TFOG);

    if (players[consoleplayer].viewz != 1)
        S_StartSound(mo, sfx_telept);   // don't start sound on first frame

    return true;
}

/*
====================
=
= G_DeathMatchSpawnPlayer
=
= Spawns a player at one of the random death match spots
= called at level load and each death
====================

void G_DeathMatchSpawnPlayer(int playernum)
{
    int i, j;
    int selections;

    selections = deathmatch_p - deathmatchstarts;
    if (selections < 4)
        I_Error("Only %i deathmatch spots, 4 required", selections);

    for (j = 0; j < 20; j++)
    {
        i = P_Random() % selections;
        if (G_CheckSpot(playernum, &deathmatchstarts[i]))
        {
            deathmatchstarts[i].type = playernum + 1;
            P_SpawnPlayer(&deathmatchstarts[i]);
            return;
        }
    }

// no good spot, so the player will probably get stuck
    P_SpawnPlayer(&playerstarts[playernum]);
}

====================
=
= G_DoReborn
=
====================
*/

void G_DoReborn(int playernum)
{
//    int i;

    if (G_CheckDemoStatus())
        return;
//    if (!netgame)
        gameaction = ga_loadlevel;      // reload the level from scratch
/*
    else
    {                           // respawn at the start
        players[playernum].mo->player = NULL;   // dissasociate the corpse

        // spawn at random spot if in death match
        if (deathmatch)
        {
            G_DeathMatchSpawnPlayer(playernum);
            return;
        }

        if (G_CheckSpot(playernum, &playerstarts[playernum]))
        {
            P_SpawnPlayer(&playerstarts[playernum]);
            return;
        }
        // try to spawn at one of the other players spots
        for (i = 0; i < MAXPLAYERS; i++)
            if (G_CheckSpot(playernum, &playerstarts[i]))
            {
                playerstarts[i].type = playernum + 1;   // fake as other player
                P_SpawnPlayer(&playerstarts[i]);
                playerstarts[i].type = i + 1;   // restore
                return;
            }
        // he's going to be inside something.  Too bad.
        P_SpawnPlayer(&playerstarts[playernum]);
    }
*/
}


void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}


/*
====================
=
= G_DoCompleted
=
====================
*/

boolean secretexit;

void G_ExitLevel(void)
{
    if(HERETIC_BETA && gamemap == 3)
	I_Quit();

    secretexit = false;
    gameaction = ga_completed;
}

void G_SecretExitLevel(void)
{
    secretexit = true;
    gameaction = ga_completed;
}

void G_DoCompleted(void)
{
    int i;
    static int afterSecret[5] = { 7, 5, 5, 5, 4 };

    gameaction = ga_nothing;
    if (G_CheckDemoStatus())
    {
        return;
    }
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            G_PlayerFinishLevel(i);
        }
    }
    prevmap = gamemap;
    if (secretexit == true)
    {
        gamemap = 9;
    }
    else if (gamemap == 9)
    {                           // Finished secret level
        gamemap = afterSecret[gameepisode - 1];
    }
    else if (gamemap == 8)
    {
        gameaction = ga_victory;
        return;
    }
    else
    {
        gamemap++;
    }
    gamestate = GS_INTERMISSION;
    IN_Start();
}

//============================================================================
//
// G_WorldDone
//
//============================================================================

void G_WorldDone(void)
{
    gameaction = ga_worlddone;
}

//============================================================================
//
// G_DoWorldDone
//
//============================================================================

void G_DoWorldDone(void)
{
    gamestate = GS_LEVEL;
    G_DoLoadLevel();
    gameaction = ga_nothing;
    viewactive = true;
}

//---------------------------------------------------------------------------
//
// PROC G_LoadGame
//
// Can be called by the startup code or the menu task.
//
//---------------------------------------------------------------------------

static char *savename = NULL;

void G_LoadGame(char *name)
{
    savename = strdup(name);
    gameaction = ga_loadgame;
}

//---------------------------------------------------------------------------
//
// PROC G_DoLoadGame
//
// Called by G_Ticker based on gameaction.
//
//---------------------------------------------------------------------------

#define VERSIONSIZE 16

void G_DoLoadGame(void)
{
    int i;
    int a, b, c;
    char savestr[SAVESTRINGSIZE];
    char vcheck[VERSIONSIZE], readversion[VERSIONSIZE];

    gameaction = ga_nothing;

    SV_OpenRead(savename);

    free(savename);
    savename = NULL;

    // Skip the description field
    SV_Read(savestr, SAVESTRINGSIZE);

    memset(vcheck, 0, sizeof(vcheck));
    DEH_snprintf(vcheck, VERSIONSIZE, "version %i", HERETIC_VERSION);
    SV_Read(readversion, VERSIONSIZE);

    if (strncmp(readversion, vcheck, VERSIONSIZE) != 0)
    {                           // Bad version
        return;
    }
    gameskill = SV_ReadByte();
    gameepisode = SV_ReadByte();
    gamemap = SV_ReadByte();
    for (i = 0; i < MAXPLAYERS; i++)
    {
        playeringame[i] = SV_ReadByte();
    }
    // Load a base level
    G_InitNew(gameskill, gameepisode, gamemap);

    // Create leveltime
    a = SV_ReadByte();
    b = SV_ReadByte();
    c = SV_ReadByte();
    leveltime = (a << 16) + (b << 8) + c;

    // De-archive all the modifications
    P_UnArchivePlayers();
    P_UnArchiveWorld();
    P_UnArchiveThinkers();
    P_UnArchiveSpecials();

    if (SV_ReadByte() != SAVE_GAME_TERMINATOR)
    {                           // Missing savegame termination marker
        I_Error("Bad savegame");
    }
}


/*
====================
=
= G_InitNew
=
= Can be called by the startup code or the menu task
= consoleplayer, displayplayer, playeringame[] should be set
====================
*/

skill_t d_skill;
int d_episode;
int d_map;

void G_DeferedInitNew(skill_t skill, int episode, int map)
{
    d_skill = skill;
    d_episode = episode;
    d_map = map;
    gameaction = ga_newgame;
}

void G_DoNewGame(void)
{
    G_InitNew(d_skill, d_episode, d_map);
    gameaction = ga_nothing;
}

void G_InitNew(skill_t skill, int episode, int map)
{
    int i;
    int speed;
    static char *skyLumpNames[5] = {
        "SKY1", "SKY2", "SKY3", "SKY1", "SKY3"
    };

    if (paused)
    {
        paused = false;
        S_ResumeSound();
    }
    if (skill < sk_baby)
        skill = sk_baby;
    if (skill > sk_nightmare)
        skill = sk_nightmare;
    if (episode < 1)
        episode = 1;
    // Up to 9 episodes for testing
    if (episode > 9)
        episode = 9;
    if (map < 1)
        map = 1;
    if (map > 9)
        map = 9;
    M_ClearRandom();
    if (respawnparm)
    {
        respawnmonsters = true;
    }
    else
    {
        respawnmonsters = false;
    }
    // Set monster missile speeds
    speed = skill == sk_nightmare;
    for (i = 0; MonsterMissileInfo[i].type != -1; i++)
    {
        mobjinfo[MonsterMissileInfo[i].type].speed
            = MonsterMissileInfo[i].speed[speed] << FRACBITS;
    }
    // Force players to be initialized upon first level load
    for (i = 0; i < MAXPLAYERS; i++)
    {
        players[i].playerstate = PST_REBORN;
        players[i].didsecret = false;
        players[i].worldTimer = 0;
    }
    // Set up a bunch of globals
    usergame = true;            // will be set false if a demo
    paused = false;
    demorecording = false;
    demoplayback = false;
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;
    viewactive = true;
    BorderNeedRefresh = true;

    // Set the sky map
    if (episode > 5)
    {
        skytexture = R_TextureNumForName(DEH_String("SKY1"));
    }
    else
    {
        skytexture = R_TextureNumForName(DEH_String(skyLumpNames[episode - 1]));
    }

//
// give one null ticcmd_t
//
#if 0
    gametic = 0;
    maketic = 1;
    for (i = 0; i < MAXPLAYERS; i++)
        nettics[i] = 1;         // one null event for this gametic
    memset(localcmds, 0, sizeof(localcmds));
    memset(netcmds, 0, sizeof(netcmds));
#endif
    G_DoLoadLevel();
}


/*
===============================================================================

							DEMO RECORDING

===============================================================================
*/

#define DEMOMARKER      0x80

void G_ReadDemoTiccmd(ticcmd_t * cmd)
{
    if (*demo_p == DEMOMARKER)
    {                           // end of demo data stream
        G_CheckDemoStatus();
        return;
    }
    cmd->forwardmove = ((signed char) *demo_p++);
    cmd->sidemove = ((signed char) *demo_p++);
    cmd->angleturn = ((unsigned char) *demo_p++) << 8;
    cmd->buttons = (unsigned char) *demo_p++;
    cmd->lookfly = (unsigned char) *demo_p++;
    cmd->arti = (unsigned char) *demo_p++;
}

void G_WriteDemoTiccmd(ticcmd_t * cmd)
{
/*
    if (gamekeydown['q'])       // press q to end demo recording
        G_CheckDemoStatus();
*/
    *demo_p++ = cmd->forwardmove;
    *demo_p++ = cmd->sidemove;
    *demo_p++ = cmd->angleturn >> 8;
    *demo_p++ = cmd->buttons;
    *demo_p++ = cmd->lookfly;
    *demo_p++ = cmd->arti;
    demo_p -= 6;
    G_ReadDemoTiccmd(cmd);      // make SURE it is exactly the same
}



/*
===================
=
= G_RecordDemo
=
===================
*/

void G_RecordDemo(skill_t skill, int numplayers, int episode, int map/*,
                  char *name*/)
{
    int i;

    G_InitNew(skill, episode, map);
    usergame = false;
/*
    strcpy(demoname, name);
    strcat(demoname, ".lmp");
*/
    demobuffer = demo_p = Z_Malloc(0x20000, PU_STATIC, NULL);
    *demo_p++ = skill;
    *demo_p++ = episode;
    *demo_p++ = map;

    for (i = 0; i < MAXPLAYERS; i++)
        *demo_p++ = playeringame[i];

    demorecording = true;
}


/*
===================
=
= G_PlayDemo
=
===================
*/

char *defdemoname;

void G_DeferedPlayDemo(char *name)
{
    defdemoname = name;
    gameaction = ga_playdemo;
}

void G_DoPlayDemo(void)
{
    skill_t skill;
    int i, episode, map;

    gameaction = ga_nothing;
    demobuffer = demo_p = W_CacheLumpName(defdemoname, PU_STATIC);
    skill = *demo_p++;
    episode = *demo_p++;
    map = *demo_p++;

    for (i = 0; i < MAXPLAYERS; i++)
        playeringame[i] = *demo_p++;

    precache = false;           // don't spend a lot of time in loadlevel
    G_InitNew(skill, episode, map);
    precache = true;
    usergame = false;
    demoplayback = true;
}


/*
===================
=
= G_TimeDemo
=
===================
*/

void G_TimeDemo(char *name)
{
    skill_t skill;
    int episode, map;

    demobuffer = demo_p = W_CacheLumpName(name, PU_STATIC);
    skill = *demo_p++;
    episode = *demo_p++;
    map = *demo_p++;
    G_InitNew(skill, episode, map);
    usergame = false;
    demoplayback = true;
    timingdemo = true;
    singletics = true;
}


/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

boolean G_CheckDemoStatus(void)
{
    int     i, endtime;
    char    lbmname[10];

    FILE    *test_access;

    strcpy(lbmname,"DEMO00.lmp");

    for (i=1 ; i<=98 ; i++)
    {
	lbmname[4] = i/10 + '0';
	lbmname[5] = i%10 + '0';
/*
	if (access(lbmname,0) == -1)
	    break;  // file doesn't exist
*/
	test_access = fopen(lbmname, "rb");
  
	if(test_access)
	    break;
    }

    if (i==100)
	I_Error ("G_CheckDemoStatus: Couldn't create a DEMO");

    if (timingdemo)
    {
        endtime = I_GetTime();
        I_Error("timed %i gametics in %i realtics", gametic,
                endtime - starttime);
    }

    if (demoplayback)
    {
        if (singledemo)
            I_Quit();

        W_ReleaseLumpName(defdemoname);
        demoplayback = false;
        D_AdvanceDemo();
        return true;
    }

    if (demorecording)
    {
        *demo_p++ = DEMOMARKER;
        M_WriteFile(lbmname, demobuffer, demo_p - demobuffer);
        Z_Free(demobuffer);
        demorecording = false;
        I_Error("Demo %s recorded", demoname);
    }

    return false;
}
/**************************************************************************/
/**************************************************************************/

//==========================================================================
//
// G_SaveGame
//
// Called by the menu task.  <description> is a 24 byte text string.
//
//==========================================================================

void G_SaveGame(int slot, char *description)
{
    savegameslot = slot;

    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);

    int day = aTime->tm_mday;
    int month = aTime->tm_mon + 1;
    int year = aTime->tm_year + 1900;
    int hour = aTime->tm_hour;
    int min = aTime->tm_min;

    if(load_extra_wad == 1)
	sprintf(savedescription, "E%dM%d %d/%d/%d %2.2d:%2.2d *",
		gameepisode, gamemap, year, month, day, hour, min);
    else
	sprintf(savedescription, "E%dM%d %d/%d/%d %2.2d:%2.2d",
		gameepisode, gamemap, year, month, day, hour, min);

    sendsave = true;
}

//==========================================================================
//
// G_DoSaveGame
//
// Called by G_Ticker based on gameaction.
//
//==========================================================================

void G_DoSaveGame(void)
{
    int i;
    char *filename;
    char verString[VERSIONSIZE];
    char *description;

    filename = SV_Filename(savegameslot);

    description = savedescription;

    SV_Open(filename);
    SV_Write(description, SAVESTRINGSIZE);
    memset(verString, 0, sizeof(verString));
    DEH_snprintf(verString, VERSIONSIZE, "version %i", HERETIC_VERSION);
    SV_Write(verString, VERSIONSIZE);
    SV_WriteByte(gameskill);
    SV_WriteByte(gameepisode);
    SV_WriteByte(gamemap);
    for (i = 0; i < MAXPLAYERS; i++)
    {
        SV_WriteByte(playeringame[i]);
    }
    SV_WriteByte(leveltime >> 16);
    SV_WriteByte(leveltime >> 8);
    SV_WriteByte(leveltime);
    P_ArchivePlayers();
    P_ArchiveWorld();
    P_ArchiveThinkers();
    P_ArchiveSpecials();
    SV_Close(filename);

    gameaction = ga_nothing;
    savedescription[0] = 0;
    P_SetMessage(&players[consoleplayer], DEH_String(TXT_GAMESAVED), true);

    free(filename);
}

