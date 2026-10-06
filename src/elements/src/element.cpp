/**
 * Compiz elements plugin
 * element.cpp
 *
 * This plugin allows you to draw different 'elements' on your screen
 * such as snow, fireflies, starts, leaves and bubbles. It also has
 * a pluggable element creation interface
 *
 * Copyright (c) 2008 Sam Spilsbury <smspillaz@gmail.com>
 * Copyright (c) 2008 Patrick Fisher <pat@elementsplugin.com>
 *
 * This plugin was based on the works of the following authors:
 *
 * Snow Plugin:
 * Copyright (c) 2006 Eckhart P. <beryl@cornergraf.net>
 * Copyright (c) 2006 Brian Jørgensen <qte@fundanemt.com>
 *
 * Fireflies Plugin:
 * Copyright (c) 2006 Eckhart P. <beryl@cornergraf.net>
 * Copyright (c) 2006 Brian Jørgensen <qte@fundanemt.com>
 *
 * Stars Plugin:
 * Copyright (c) 2007 Kyle Mallory <kyle.mallory@utah.edu>
 *
 * Autumn Plugin
 * Copyright (c) 2007 Patrick Fisher <pat@elementsplugin.com>
 *
 * Extensions interface largely based off the Animation plugin
 * Copyright (c) 2006 Erkin Bahceci <erkinbah@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 **/

#include "private.h"

/* Check to see if an element is outside the screen. If it is
 * remove it and re-initiate it. Otherwise move it */

void
Element::regenerateOffscreen ()
{
    ELEMENTS_SCREEN (screen);

    if ((y >= screen->height () + 200				  ||
	 x <= -200                                          	  ||
	 x >= screen->width () + 200                              ||
	 y >= screen->height () + 200                             ||
	 z <= -((float) es->depth () / 500.0) ||
	 z >= ((float)  es->boxing () / 5.0f)))
    {
	defaultInit ();
	init ();
    }
}

void
Element::defaultInit ()
{
    ELEMENTS_SCREEN (screen);
    x = 0;
    y = 0;
    z = es->mmRand (-es->depth (), 0.1, 5000);
    dx = 0;
    dy = 0;
    dz = es->mmRand (-500, 500, 500000);
    rAngle = es->mmRand (-1000, 1000, 50);
    rSpeed = anim->rotate () ? es->mmRand (-2100, 2100, 700) : 0;
    opacity = 1.0f;
    glowAlpha = 0.0f;
}

Element::Element ()
{
}

bool
Element::init ()
{
    defaultInit ();
    return true;
}

void
Element::move ()
{
}

void
Element::fini ()
{
}
