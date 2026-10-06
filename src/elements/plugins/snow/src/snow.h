/**
 * Compiz elements plugin
 * elements.c
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

#include <elements/elements.h>
#include <core/core.h>

#include "snow_options.h"

class SnowScreen :
    public PluginClassHandler <SnowScreen, CompScreen>,
    public SnowOptions
{
    public:

	SnowScreen (CompScreen *);
	~SnowScreen ();

	static bool
	toggle (CompAction *,
		CompAction::State,
		CompOption::Vector &);

	void autostartChanged (CompOption *opt, SnowOptions::Options num);

	ElementType *type;

};

class SnowElement :
    public Element
{
    public:

	bool init ();
	void move ();
	void fini ();

	static Element *
	create ();

	~SnowElement ();
	SnowElement ();
    private:
};

class SnowPluginVTable :
    public CompPlugin::VTableForScreen <SnowScreen>
{
    public:

	ElementType *type;

	bool init ();
};
