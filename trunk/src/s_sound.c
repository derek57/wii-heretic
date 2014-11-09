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

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <stdlib.h>

#include "doomdef.h"
#include "i_system.h"
#include "m_random.h"
#include "sounds.h"
#include "s_sound.h"
#include "i_sound.h"
#include "r_local.h"
#include "p_local.h"

#include "sounds.h"

#include "w_wad.h"
#include "z_zone.h"

#include "doomfeatures.h"

/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

void S_ShutDown(void);
boolean S_StopSoundID(int sound_id, int priority);

static channel_t channel[MAX_CHANNELS];

static void *rs;          // Handle for the registered song
int mus_song = -1;
int mus_lumpnum;
void *mus_sndptr;
byte *soundCurve;

int snd_MaxVolume = 10;
int snd_MusicVolume = 10;
int snd_Channels = 16;

int AmbChan;

int musicPlaying = 0;            //Is the music playing, or not?

short songlist[51];
short currentsong = 0;

//extern boolean opl;
extern boolean forced;
extern boolean fake;

extern int faketracknum;
extern int tracknum;

void S_Start(void)
{
    int i;

#ifdef OGG_SUPPORT
    if(opl)
#endif
	S_StartSong((gameepisode - 1) * 9 + gamemap - 1, true);
#ifdef OGG_SUPPORT
    else
	S_StartMP3Music(0, -1); 
#endif

    //stop all sounds
    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].handle)
        {
            S_StopSound(channel[i].mo);
        }
    }
    memset(channel, 0, 8 * sizeof(channel_t));
}

void S_StartSong(int song, boolean loop)
{
    int mus_len;
/*
    if (song == mus_song)
    {                           // don't replay an old song
        return;
    }
*/
    if (rs != NULL)
    {
        I_StopSong();
        I_UnRegisterSong(rs);
    }

    if (song < mus_e1m1 || song > NUMMUSIC)
    {
        return;
    }
    mus_lumpnum = W_GetNumForName(S_music[song].name);
    mus_sndptr = W_CacheLumpNum(mus_lumpnum, PU_MUSIC);
    mus_len = W_LumpLength(mus_lumpnum);
    rs = I_RegisterSong(mus_sndptr, mus_len);
#ifdef OGG_SUPPORT
    if(opl)
#endif
	I_PlaySong(rs, loop);       //'true' denotes endless looping.
#ifdef OGG_SUPPORT
    else
	S_StartMP3Music(0, -1);
#endif
    mus_song = song;
}

static mobj_t *GetSoundListener(void)
{
    static degenmobj_t dummy_listener;

    // If we are at the title screen, the console player doesn't have an
    // object yet, so return a pointer to a static dummy listener instead.

    if (players[consoleplayer].mo != NULL)
    {
        return players[consoleplayer].mo;
    }
    else
    {
        dummy_listener.x = 0;
        dummy_listener.y = 0;
        dummy_listener.z = 0;

        return (mobj_t *) &dummy_listener;
    }
}

void S_StartSound(void *_origin, int sound_id)
{
    mobj_t *origin = _origin;
    mobj_t *listener;
    int dist, vol;
    int i;
    int priority;
    int sep;
    int angle;
    int absx;
    int absy;

    static int sndcount = 0;
    int chan;

    listener = GetSoundListener();

    if (sound_id == 0 || snd_MaxVolume == 0)
        return;
    if (origin == NULL)
    {
        origin = listener;
    }

// calculate the distance before other stuff so that we can throw out
// sounds that are beyond the hearing range.
    absx = abs(origin->x - listener->x);
    absy = abs(origin->y - listener->y);
    dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
    dist >>= FRACBITS;
//  dist = P_AproxDistance(origin->x-viewx, origin->y-viewy)>>FRACBITS;

    if (dist >= MAX_SND_DIST)
    {
//      dist = MAX_SND_DIST - 1;
        return;                 //sound is beyond the hearing range...
    }
    if (dist < 0)
    {
        dist = 0;
    }
    priority = S_sfx[sound_id].priority;
    priority *= (10 - (dist / 160));
    if (!S_StopSoundID(sound_id, priority))
    {
        return;                 // other sounds have greater priority
    }
    for (i = 0; i < snd_Channels; i++)
    {
        if (origin->player)
        {
            i = snd_Channels;
            break;              // let the player have more than one sound.
        }
        if (origin == channel[i].mo)
        {                       // only allow other mobjs one sound
            S_StopSound(channel[i].mo);
            break;
        }
    }
    if (i >= snd_Channels)
    {
        if (sound_id >= sfx_wind)
        {
            if (AmbChan != -1 && S_sfx[sound_id].priority <=
                S_sfx[channel[AmbChan].sound_id].priority)
            {
                return;         //ambient channel already in use
            }
            else
            {
                AmbChan = -1;
            }
        }
        for (i = 0; i < snd_Channels; i++)
        {
            if (channel[i].mo == NULL)
            {
                break;
            }
        }
        if (i >= snd_Channels)
        {
            //look for a lower priority sound to replace.
            sndcount++;
            if (sndcount >= snd_Channels)
            {
                sndcount = 0;
            }
            for (chan = 0; chan < snd_Channels; chan++)
            {
                i = (sndcount + chan) % snd_Channels;
                if (priority >= channel[i].priority)
                {
                    chan = -1;  //denote that sound should be replaced.
                    break;
                }
            }
            if (chan != -1)
            {
                return;         //no free channels.
            }
            else                //replace the lower priority sound.
            {
                if (channel[i].handle)
                {
                    if (I_SoundIsPlaying(channel[i].handle))
                    {
                        I_StopSound(channel[i].handle);
                    }
                    if (S_sfx[channel[i].sound_id].usefulness > 0)
                    {
                        S_sfx[channel[i].sound_id].usefulness--;
                    }

                    if (AmbChan == i)
                    {
                        AmbChan = -1;
                    }
                }
            }
        }
    }
    if (S_sfx[sound_id].lumpnum == 0)
    {
        S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
    }

    // calculate the volume based upon the distance from the sound origin.
//      vol = (snd_MaxVolume*16 + dist*(-snd_MaxVolume*16)/MAX_SND_DIST)>>9;
    vol = soundCurve[dist];

    if (origin == listener)
    {
        sep = 128;
    }
    else
    {
        angle = R_PointToAngle2(listener->x, listener->y,
                                origin->x, origin->y);
        angle = (angle - viewangle) >> 24;
        sep = angle * 2 - 128;
        if (sep < 64)
            sep = -sep;
        if (sep > 192)
            sep = 512 - sep;
    }

    // TODO: Play pitch-shifted sounds as in Vanilla Heretic

    channel[i].pitch = (byte) (127 + (M_Random() & 7) - (M_Random() & 7));
    channel[i].handle = I_StartSound(&S_sfx[sound_id], i, vol, sep);
    channel[i].mo = origin;
    channel[i].sound_id = sound_id;
    channel[i].priority = priority;
    if (sound_id >= sfx_wind)
    {
        AmbChan = i;
    }
    if (S_sfx[sound_id].usefulness == -1)
    {
        S_sfx[sound_id].usefulness = 1;
    }
    else
    {
        S_sfx[sound_id].usefulness++;
    }
}

void S_StartSoundAtVolume(void *_origin, int sound_id, int volume)
{
    mobj_t *origin = _origin;
    mobj_t *listener;
    int i;

    listener = GetSoundListener();

    if (sound_id == 0 || snd_MaxVolume == 0)
        return;
    if (origin == NULL)
    {
        origin = listener;
    }

    if (volume == 0)
    {
        return;
    }
    volume = (volume * (snd_MaxVolume + 1) * 8) >> 7;

// no priority checking, as ambient sounds would be the LOWEST.
    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].mo == NULL)
        {
            break;
        }
    }
    if (i >= snd_Channels)
    {
        return;
    }
    if (S_sfx[sound_id].lumpnum == 0)
    {
        S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
    }

    // TODO: Pitch shifting.
    channel[i].pitch = (byte) (127 - (M_Random() & 3) + (M_Random() & 3));
    channel[i].handle = I_StartSound(&S_sfx[sound_id], i, volume, 128);
    channel[i].mo = origin;
    channel[i].sound_id = sound_id;
    channel[i].priority = 1;    //super low priority.
    if (S_sfx[sound_id].usefulness == -1)
    {
        S_sfx[sound_id].usefulness = 1;
    }
    else
    {
        S_sfx[sound_id].usefulness++;
    }
}

boolean S_StopSoundID(int sound_id, int priority)
{
    int i;
    int lp;                     //least priority
    int found;

    if (S_sfx[sound_id].numchannels == -1)
    {
        return (true);
    }
    lp = -1;                    //denote the argument sound_id
    found = 0;
    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].sound_id == sound_id && channel[i].mo)
        {
            found++;            //found one.  Now, should we replace it??
            if (priority >= channel[i].priority)
            {                   // if we're gonna kill one, then this'll be it
                lp = i;
                priority = channel[i].priority;
            }
        }
    }
    if (found < S_sfx[sound_id].numchannels)
    {
        return (true);
    }
    else if (lp == -1)
    {
        return (false);         // don't replace any sounds
    }
    if (channel[lp].handle)
    {
        if (I_SoundIsPlaying(channel[lp].handle))
        {
            I_StopSound(channel[lp].handle);
        }
        if (S_sfx[channel[i].sound_id].usefulness > 0)
        {
            S_sfx[channel[i].sound_id].usefulness--;
        }
        channel[lp].mo = NULL;
    }
    return (true);
}

void S_StopSound(void *_origin)
{
    mobj_t *origin = _origin;
    int i;

    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].mo == origin)
        {
            I_StopSound(channel[i].handle);
            if (S_sfx[channel[i].sound_id].usefulness > 0)
            {
                S_sfx[channel[i].sound_id].usefulness--;
            }
            channel[i].handle = 0;
            channel[i].mo = NULL;
            if (AmbChan == i)
            {
                AmbChan = -1;
            }
        }
    }
}

void S_SoundLink(mobj_t * oldactor, mobj_t * newactor)
{
    int i;

    for (i = 0; i < snd_Channels; i++)
    {
        if (channel[i].mo == oldactor)
            channel[i].mo = newactor;
    }
}

void S_PauseSound(void)
{
    I_PauseSong();
}

void S_ResumeSound(void)
{
    I_ResumeSong();
}

void S_UpdateSounds(mobj_t * listener)
{
    int i, dist, vol;
    int angle;
    int sep;
    int priority;
    int absx;
    int absy;

    listener = GetSoundListener();
    if (snd_MaxVolume == 0)
    {
        return;
    }

    for (i = 0; i < snd_Channels; i++)
    {
        if (!channel[i].handle || S_sfx[channel[i].sound_id].usefulness == -1)
        {
            continue;
        }
        if (!I_SoundIsPlaying(channel[i].handle))
        {
            if (S_sfx[channel[i].sound_id].usefulness > 0)
            {
                S_sfx[channel[i].sound_id].usefulness--;
            }
            channel[i].handle = 0;
            channel[i].mo = NULL;
            channel[i].sound_id = 0;
            if (AmbChan == i)
            {
                AmbChan = -1;
            }
        }
        if (channel[i].mo == NULL || channel[i].sound_id == 0
         || channel[i].mo == listener || listener == NULL)
        {
            continue;
        }
        else
        {
            absx = abs(channel[i].mo->x - listener->x);
            absy = abs(channel[i].mo->y - listener->y);
            dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
            dist >>= FRACBITS;
//          dist = P_AproxDistance(channel[i].mo->x-listener->x, channel[i].mo->y-listener->y)>>FRACBITS;

            if (dist >= MAX_SND_DIST)
            {
                S_StopSound(channel[i].mo);
                continue;
            }
            if (dist < 0)
                dist = 0;

// calculate the volume based upon the distance from the sound origin.
//          vol = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE)+dist)*(snd_MaxVolume*8))>>7;
            vol = soundCurve[dist];

            angle = R_PointToAngle2(listener->x, listener->y,
                                    channel[i].mo->x, channel[i].mo->y);
            angle = (angle - viewangle) >> 24;
            sep = angle * 2 - 128;
            if (sep < 64)
                sep = -sep;
            if (sep > 192)
                sep = 512 - sep;
            // TODO: Pitch shifting.
            I_UpdateSoundParams(channel[i].handle, vol, sep);
            priority = S_sfx[channel[i].sound_id].priority;
            priority *= (10 - (dist >> 8));
            channel[i].priority = priority;
        }
    }
}

void S_Init(void)
{
    soundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);
    I_InitSound(false);
    if (snd_Channels > 8)
    {
        snd_Channels = 8;
    }
    I_SetMusicVolume(snd_MusicVolume * 8);
    S_SetMaxVolume(true);

    I_AtExit(S_ShutDown, true);

    I_PrecacheSounds(S_sfx, NUMSFX);
}

void S_GetChannelInfo(SoundInfo_t * s)
{
    int i;
    ChanInfo_t *c;

    s->channelCount = snd_Channels;
    s->musicVolume = snd_MusicVolume;
    s->soundVolume = snd_MaxVolume;
    for (i = 0; i < snd_Channels; i++)
    {
        c = &s->chan[i];
        c->id = channel[i].sound_id;
        c->priority = channel[i].priority;
        c->name = S_sfx[c->id].name;
        c->mo = channel[i].mo;

        if (c->mo != NULL)
        {
            c->distance = P_AproxDistance(c->mo->x - viewx, c->mo->y - viewy)
                >> FRACBITS;
        }
        else
        {
            c->distance = 0;
        }
    }
}

void S_SetMaxVolume(boolean fullprocess)
{
    int i;

    if (!fullprocess)
    {
        soundCurve[0] =
            (*((byte *) W_CacheLumpName("SNDCURVE", PU_CACHE)) *
             (snd_MaxVolume * 8)) >> 7;
    }
    else
    {
        for (i = 0; i < MAX_SND_DIST; i++)
        {
            soundCurve[i] =
                (*((byte *) W_CacheLumpName("SNDCURVE", PU_CACHE) + i) *
                 (snd_MaxVolume * 8)) >> 7;
        }
    }
}

static boolean musicPaused;

void S_SetMusicVolume(void)
{
    I_SetMusicVolume(snd_MusicVolume * 8);
    if (snd_MusicVolume == 0)
    {
        I_PauseSong();
        musicPaused = true;
    }
    else if (musicPaused)
    {
        musicPaused = false;
        I_ResumeSong();
    }
}

void S_ShutDown(void)
{
    I_StopSong();
    I_UnRegisterSong(rs);
    I_ShutdownSound();
}

#ifdef OGG_SUPPORT
short S_GetLevelSongNum(int episode, int map)
{
    if(episode < 1 || map < 1)
    {
	return 0;
    }
    short songnum = 0;

//    if(gamemission == doom)
        songnum = songlist[9*(episode-1)+(map-1)];
/*
    else if(gamemission == pack_tnt)
        songnum = songlist[76+(map-1)];
    else if(gamemission == pack_plut)
        songnum = songlist[112+(map-1)];
    else if(gamemission == doom2)
        songnum = songlist[40+(map-1)];
*/
    return songnum;
}

void musicFinished()
{
   //Music is done!
   musicPlaying = 0;
}

void SDL_InitOGG(void)
{
    int audio_rate = 22050;			//Frequency of audio playback
    int audio_channels = 2;			//2 channels = stereo
    int audio_buffers = 4096;			//Size of the audio buffers in memory

    Uint16 audio_format = AUDIO_S16MSB; 	//Format of the audio we're playing

    songlist[0] = 1, // HERETIC: E1M1
    songlist[1] = 2; // HERETIC: E1M2
    songlist[2] = 3; // HERETIC: E1M3
    songlist[3] = 4; // HERETIC: E1M4
    songlist[4] = 5; // HERETIC: E1M5
    songlist[5] = 6; // HERETIC: E1M6
    songlist[6] = 7; // HERETIC: E1M7
    songlist[7] = 8; // HERETIC: E1M8
    songlist[8] = 9; // HERETIC: E1M9
    songlist[9] = 10; // HERETIC: E2M1
    songlist[10] = 11; // HERETIC: E2M2
    songlist[11] = 12; // HERETIC: E2M3
    songlist[12] = 13; // HERETIC: E2M4
    songlist[13] = 4; // HERETIC: E2M5
    songlist[14] = 14; // HERETIC: E2M6
    songlist[15] = 15; // HERETIC: E2M7
    songlist[16] = 16; // HERETIC: E2M8
    songlist[17] = 17; // HERETIC: E2M9
    songlist[18] = 1; // HERETIC: E3M1
    songlist[19] = 18; // HERETIC: E3M2
    songlist[20] = 19; // HERETIC: E3M3
    songlist[21] = 6; // HERETIC: E3M4
    songlist[22] = 3; // HERETIC: E3M5
    songlist[23] = 2; // HERETIC: E3M6
    songlist[24] = 5; // HERETIC: E3M7
    songlist[25] = 9; // HERETIC: E3M8
    songlist[26] = 14; // HERETIC: E3M9
    songlist[27] = 6; // HERETIC: E4M1
    songlist[28] = 2; // HERETIC: E4M2
    songlist[29] = 3; // HERETIC: E4M3
    songlist[30] = 4; // HERETIC: E4M4
    songlist[31] = 5; // HERETIC: E4M5
    songlist[32] = 1; // HERETIC: E4M6
    songlist[33] = 7; // HERETIC: E4M7
    songlist[34] = 8; // HERETIC: E4M8
    songlist[35] = 9; // HERETIC: E4M9
    songlist[36] = 10; // HERETIC: E5M1
    songlist[37] = 11; // HERETIC: E5M2
    songlist[38] = 12; // HERETIC: E5M3
    songlist[39] = 13; // HERETIC: E5M4
    songlist[40] = 4; // HERETIC: E5M5
    songlist[41] = 14; // HERETIC: E5M6
    songlist[42] = 15; // HERETIC: E5M7
    songlist[43] = 16; // HERETIC: E5M8
    songlist[44] = 17; // HERETIC: E5M9
    songlist[45] = 18; // HERETIC: E6M1
    songlist[46] = 19; // HERETIC: E6M2
    songlist[47] = 6; // HERETIC: E6M3
    songlist[48] = 21; // HERETIC: INTERMISSION
    songlist[49] = 20; // HERETIC: TITLE
    songlist[50] = 22; // HERETIC: FINALE

    //Initialize SDL audio
    if (SDL_Init(SDL_INIT_AUDIO) != 0) 
    {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
    }
	
    //Initialize SDL_mixer with our chosen audio settings
    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) 
    {
	printf("Unable to initialize audio: %s\n", Mix_GetError());
    }
}

/*static*/ void I_OPL_ShutdownMusic(void);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"	// ADDED FOR THE MP3 MUSIC SWITCHING BUG

void S_StartMP3Music(int type, int mode)
{
    int modex = 0;

    if(mode == -1)
	modex = 0 - 1;
    else
	modex = mode;

    char path[20];

    I_OPL_ShutdownMusic();

    Mix_Music *music;

    if(type == 0)
    {
        if(gameepisode < 1 || gamemap < 1)
            return;

        currentsong = S_GetLevelSongNum(gameepisode, gamemap);
    }
    else if(type == 1)
    {
//        if(gamemission == doom)
            currentsong = songlist[48];
/*
        else if(gamemission == pack_tnt)
            currentsong = songlist[108];
        else if(gamemission == pack_plut)
            currentsong = songlist[144];
        else if(gamemission == doom2)
            currentsong = songlist[72];
*/
    }
    else if(type == 2)
    {
//        if(gamemission == doom)
            currentsong = songlist[49];
/*
        else if(gamemission == pack_tnt)
            currentsong = songlist[109];
        else if(gamemission == pack_plut)
            currentsong = songlist[145];
        else if(gamemission == doom2)
            currentsong = songlist[73];
*/
    }
    else if(type == 3)
    {
//        if(gamemission == doom)
            currentsong = songlist[50];
/*
        else if(gamemission == pack_tnt)
            currentsong = songlist[110];
        else if(gamemission == pack_plut)
            currentsong = songlist[146];
        else if(gamemission == doom2)
            currentsong = songlist[74];
*/
    }

    if(!forced)
	sprintf(path, "usb:/apps/wiiheretic/music/song%i.ogg", currentsong);
    else
    {
	if(fake)
	    sprintf(path, "usb:/apps/wiiheretic/music/song%i.ogg", faketracknum);
	else
	    sprintf(path, "usb:/apps/wiiheretic/music/song%i.ogg", tracknum);
    }
				// FIXME: THIS PRINTF FIXES A BUG WHEN USING MP3 "CHOOSE TRACK" FROM
    printf("",tracknum);	// WITHIN THE MAIN MENU. WHILE THIS COMMAND IS NOT REQUIRED FOR WIIDOOM,
				// IT IS FOR WIIHERETIC OR ELSE THE GAME CRASHES UPON MUSIC SWITCHING.
    music = Mix_LoadMUS(path);

    if(music == NULL) 
    {
	printf("Unable to load music file: %s\n", Mix_GetError());
    }

    //Play music!
    if(Mix_PlayMusic(music, modex) == -1) 
    {
	printf("Unable to play music file: %s\n", Mix_GetError());
    }
	
    //The music is playing!
    musicPlaying = 1;

    //Make sure that the musicFinished() function is called when the music stops playing
    Mix_HookMusicFinished(musicFinished);
}
#endif

