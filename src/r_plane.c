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
// R_planes.c

#include <stdlib.h>
#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "r_local.h"

#include "c_io.h"

planefunction_t floorfunc, ceilingfunc;

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;
fixed_t skyiscale;

//
// opening
//

//visplane_t		visplanes[MAXVISPLANES];		// CHANGED FOR HIRES
visplane_t*		visplanes = NULL;			// CHANGED FOR HIRES
visplane_t *lastvisplane;
visplane_t *floorplane, *ceilingplane;

static int		numvisplanes;				// ADDED FOR HIRES

/*
short			openings[MAXOPENINGS];			// CHANGED FOR HIRES
short*			lastopening;				// CHANGED FOR HIRES
*/
int			*openings;				// CHANGED FOR HIRES
int*			lastopening;				// CHANGED FOR HIRES
size_t                  maxopenings;

//
// clip values are the solid pixel bounding the range
// floorclip starts out SCREENHEIGHT
// ceilingclip starts out -1
//
/*
short			floorclip[SCREENWIDTH];			// CHANGED FOR HIRES
short			ceilingclip[SCREENWIDTH];		// CHANGED FOR HIRES
*/
int			floorclip[SCREENWIDTH];			// CHANGED FOR HIRES
int			ceilingclip[SCREENWIDTH];		// CHANGED FOR HIRES

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int spanstart[SCREENHEIGHT];
int spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t **planezlight;
fixed_t planeheight;

fixed_t yslope[SCREENHEIGHT];
fixed_t distscale[SCREENWIDTH];
fixed_t basexscale, baseyscale;

fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];


/*
================
=
= R_InitSkyMap
=
= Called whenever the view size changes
=
================
*/

void R_InitSkyMap(void)
{
    skyflatnum = R_FlatNumForName(DEH_String("F_SKY1"));
    skytexturemid = 200 * FRACUNIT;
//    skyiscale = FRACUNIT;					// CHANGED FOR HIRES
    skyiscale = FRACUNIT >> hires;				// CHANGED FOR HIRES
}


/*
====================
=
= R_InitPlanes
=
= Only at game startup
====================
*/

void R_InitPlanes(void)
{
}


/*
================
=
= R_MapPlane
=
global vars:

planeheight
ds_source
basexscale
baseyscale
viewx
viewy

BASIC PRIMITIVE
================
*/

void R_MapPlane(int y, int x1, int x2)
{
    angle_t angle;
    fixed_t distance, length;
    unsigned index;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || (unsigned) y > viewheight)
        I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
#endif

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);

        ds_xstep = cachedxstep[y] = FixedMul(distance, basexscale);
        ds_ystep = cachedystep[y] = FixedMul(distance, baseyscale);
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    length = FixedMul(distance, distscale[x1]);
    angle = (viewangle + xtoviewangle[x1]) >> ANGLETOFINESHIFT;
    ds_xfrac = viewx + FixedMul(finecosine[angle], length);
    ds_yfrac = -viewy - FixedMul(finesine[angle], length);

    if (fixedcolormap)
        ds_colormap = fixedcolormap;
    else
    {
        index = distance >> LIGHTZSHIFT;
        if (index >= MAXLIGHTZ)
            index = MAXLIGHTZ - 1;
        ds_colormap = planezlight[index];
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    spanfunc();                 // high or low detail
}

//=============================================================================

/*
====================
=
= R_ClearPlanes
=
= At begining of frame
====================
*/

void R_ClearPlanes(void)
{
    int i;
    angle_t angle;

//
// opening / clipping determination
//      
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

//
// texture calculation
//
    memset(cachedheight, 0, sizeof(cachedheight));
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;    // left to right mapping

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(finecosine[angle], centerxfrac);
    baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}


static void R_RaiseVisplanes (visplane_t** vp)
{
    if (lastvisplane - visplanes == numvisplanes)
    {
	int numvisplanes_old = numvisplanes;
	visplane_t* visplanes_old = visplanes;

	numvisplanes = numvisplanes ? 2 * numvisplanes : MAXVISPLANES;
	visplanes = realloc(visplanes, numvisplanes * sizeof(*visplanes));
	memset(visplanes + numvisplanes_old, 0, (numvisplanes - numvisplanes_old) * sizeof(*visplanes));

	lastvisplane = visplanes + numvisplanes_old;
	floorplane = visplanes + (floorplane - visplanes_old);
	ceilingplane = visplanes + (ceilingplane - visplanes_old);

	if (numvisplanes_old)
	    C_Printf("R_FindPlane: Hit MAXVISPLANES limit at %d, raised to %d.\n", numvisplanes_old, numvisplanes);

	// keep the pointer passed as argument in relation to the visplanes pointer
	if (vp)
	    *vp = visplanes + (*vp - visplanes_old);
    }
}

/*
===============
=
= R_FindPlane
=
===============
*/

visplane_t *R_FindPlane(fixed_t height, int picnum,
                        int lightlevel, int special)
{
    visplane_t *check;

    if (picnum == skyflatnum)
    {
        // all skies map together
        height = 0;
        lightlevel = 0;
    }

    for (check = visplanes; check < lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel && special == check->special)
            break;
    }

    if (check < lastvisplane)
    {
        return (check);
    }

/*		
    if (lastvisplane - visplanes == MAXVISPLANES)		// CHANGED FOR HIRES
	I_Error ("R_FindPlane: no more visplanes");		// CHANGED FOR HIRES
*/		
    R_RaiseVisplanes(&check);

    lastvisplane++;
    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->special = special;
    check->minx = SCREENWIDTH;
    check->maxx = -1;
    memset(check->top, 0xff, sizeof(check->top));
    return (check);
}

/*
===============
=
= R_CheckPlane
=
===============
*/

visplane_t *R_CheckPlane(visplane_t * pl, int start, int stop)
{
    int intrl, intrh;
    int unionl, unionh;
    int x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x = intrl; x <= intrh; x++)
//	if (pl->top[x] != 0xff)					// CHANGED FOR HIRES
	if (pl->top[x] != 0xffffffffu)				// CHANGED FOR HIRES
            break;

    if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;
        return pl;              // use the same one
    }

// make a new visplane

    R_RaiseVisplanes(&pl);					// ADDED FOR HIRES

    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    lastvisplane->special = pl->special;
    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;
    memset(pl->top, 0xff, sizeof(pl->top));

    return pl;
}



//=============================================================================

/*
================
=
= R_MakeSpans
=
================
*/

void
R_MakeSpans
( int		x,
/*
  int	t1,							// CHANGED FOR HIRES
  int	b1,							// CHANGED FOR HIRES
  int	t2,							// CHANGED FOR HIRES
  int	b2 )							// CHANGED FOR HIRES
*/
  unsigned int	t1,						// CHANGED FOR HIRES
  unsigned int	b1,						// CHANGED FOR HIRES
  unsigned int	t2,						// CHANGED FOR HIRES
  unsigned int	b2 )						// CHANGED FOR HIRES
{
    while (t1 < t2 && t1 <= b1)
    {
        R_MapPlane(t1, spanstart[t1], x - 1);
        t1++;
    }
    while (b1 > b2 && b1 >= t1)
    {
        R_MapPlane(b1, spanstart[b1], x - 1);
        b1--;
    }

    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}



/*
================
=
= R_DrawPlanes
=
= At the end of each frame
================
*/

void R_DrawPlanes(void)
{
    visplane_t *pl;
    int light;
    int x, stop;
    int lumpnum;
    int angle;
    byte *tempSource;

    byte *dest;
    int count;
    fixed_t frac, fracstep;

    extern byte *ylookup[MAXHEIGHT];
    extern int columnofs[MAXWIDTH];

#ifdef RANGECHECK
/*
    if (ds_p - drawsegs > MAXDRAWSEGS)				// CHANGED FOR HIRES
	I_Error ("R_DrawPlanes: drawsegs overflow (%i)",	// CHANGED FOR HIRES
		 ds_p - drawsegs);				// CHANGED FOR HIRES
    
    if (lastvisplane - visplanes > MAXVISPLANES)		// CHANGED FOR HIRES
	I_Error ("R_DrawPlanes: visplane overflow (%i)",	// CHANGED FOR HIRES
		 lastvisplane - visplanes);			// CHANGED FOR HIRES
*/    
    if (ds_p - drawsegs > numdrawsegs)				// CHANGED FOR HIRES
	I_Error ("R_DrawPlanes: drawsegs overflow (%i)",	// CHANGED FOR HIRES
		 ds_p - drawsegs);				// CHANGED FOR HIRES
    
    if (lastvisplane - visplanes > numvisplanes)		// CHANGED FOR HIRES
	I_Error ("R_DrawPlanes: visplane overflow (%i)",	// CHANGED FOR HIRES
		 lastvisplane - visplanes);			// CHANGED FOR HIRES
    if (lastopening - openings > MAXOPENINGS)
        I_Error("R_DrawPlanes: opening overflow (%i)",
                lastopening - openings);
#endif

    for (pl = visplanes; pl < lastvisplane; pl++)
    {
        if (pl->minx > pl->maxx)
            continue;
        //
        // sky flat
        //
        if (pl->picnum == skyflatnum)
        {
//	    dc_iscale = pspriteiscale>>detailshift;			// CHANGED FOR HIRES
	    dc_iscale = pspriteiscale>>(detailshift && !hires);		// CHANGED FOR HIRES
            dc_colormap = colormaps;    // sky is allways drawn full bright
            dc_texturemid = skytexturemid;
            for (x = pl->minx; x <= pl->maxx; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];
//		if (dc_yl <= dc_yh)					// CHANGED FOR HIRES
		if ((unsigned) dc_yl <= dc_yh)				// CHANGED FOR HIRES
                {
                    angle = (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);

                    count = dc_yh - dc_yl;
                    if (count < 0)
                        return;

#ifdef RANGECHECK
                    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0
                        || dc_yh >= SCREENHEIGHT)
                        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh,
                                dc_x);
#endif

                    dest = ylookup[dc_yl] + columnofs[dc_x];
/*
                    fracstep = 1;						// CHANGED FOR HIRES
                    frac = (dc_texturemid >> FRACBITS) + (dc_yl - centery);	// CHANGED FOR HIRES
*/
		    fracstep = dc_iscale;					// CHANGED FOR HIRES
		    frac = dc_texturemid + (dc_yl - centery) * fracstep;	// CHANGED FOR HIRES
                    do
                    {
//                        *dest = dc_source[frac];				// CHANGED FOR HIRES
			*dest = dc_source[frac >> FRACBITS];			// CHANGED FOR HIRES
                        dest += SCREENWIDTH;
                        frac += fracstep;
                    }
                    while (count--);

//                                      colfunc ();
                }
            }
            continue;
        }

        //
        // regular flat
        //
        lumpnum = firstflat + flattranslation[pl->picnum];

        tempSource = W_CacheLumpNum(lumpnum, PU_STATIC);

        switch (pl->special)
        {
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:           // Scroll_North
                ds_source = tempSource;
                break;
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:           // Scroll_East
                ds_source = tempSource + ((63 - ((leveltime >> 1) & 63)) <<
                                          (pl->special - 20) & 63);
                //ds_source = tempSource+((leveltime>>1)&63);
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:           // Scroll_South
                ds_source = tempSource;
                break;
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:           // Scroll_West
                ds_source = tempSource;
                break;
            case 4:            // Scroll_EastLavaDamage
                ds_source =
                    tempSource + (((63 - ((leveltime >> 1) & 63)) << 3) & 63);
                break;
            default:
                ds_source = tempSource;
        }
        planeheight = abs(pl->height - viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;
        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS - 1;
        if (light < 0)
            light = 0;
        planezlight = zlight[light];
/*
	pl->top[pl->maxx+1] = 0xff;				// CHANGED FOR HIRES
	pl->top[pl->minx-1] = 0xff;				// CHANGED FOR HIRES
*/
	pl->top[pl->maxx+1] = 0xffffffffu;			// CHANGED FOR HIRES
	pl->top[pl->minx-1] = 0xffffffffu;			// CHANGED FOR HIRES

        stop = pl->maxx + 1;
        for (x = pl->minx; x <= stop; x++)
            R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x],
                        pl->bottom[x]);

        W_ReleaseLumpNum(lumpnum);
    }
}
