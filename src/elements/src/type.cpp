/**
 * Compiz elements plugin
 * type.cpp
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

/* Type Info Management */

ElementType::ElementType (CompString 	 name,
			  CompString 	 desc,
			  elementAddFunc add) :
    addFunc  (add),
    mName (name),
    mDesc (desc)
{
}

ElementType::ElementType ()
{
    ELEMENTS_SCREEN (screen);

    es->priv->types.push_back (this);
}

ElementType::~ElementType ()
{
    ELEMENTS_SCREEN (screen);

    es->priv->types.remove ((ElementType_ *) this);
}

bool
ElementType::KillTimer::killTimeout (CompString elementName)
{
    std::list<ElementType::KillTimer>::iterator kit;
    std::list<ElementAnimation>::iterator it;
    ELEMENTS_SCREEN (screen);
    
    for (kit = es->priv->killAnimationsTimers.begin ();
    	 kit != es->priv->killAnimationsTimers.end ();
    	 kit++)
    {
	ElementType::KillTimer &kObj = *this;
	ElementType::KillTimer &kItr = *kit;
	if (memcmp (&kItr, &kObj, sizeof (KillTimer)) == 0)
	    break;
    }

    it = es->priv->animations.begin ();

    while (it != es->priv->animations.end ())
    {
	ElementAnimation &anim = *it;

	if (elementName == anim.priv->mTypename)
	{
	    anim.priv->mTexture.clear ();
	    anim.priv->mElements.clear ();
	    it = es->priv->animations.erase (it);
	}
	else
	    it++;
    }
    
    /* Remove from the master list of timeouts, since we're done now */
    
    if (kit != es->priv->killAnimationsTimers.end ())
	es->priv->killAnimationsTimers.erase (kit);
    
    return false;
}

ElementType::KillTimer::KillTimer ()
{
}

void
ElementType::KillTimer::registerKillType (ElementType &t)
{
    ELEMENTS_SCREEN (screen);

    es->priv->killAnimationsTimers.push_back (KillTimer ());
    
    KillTimer &killTimer = es->priv->killAnimationsTimers.back ();
    
    killTimer.setTimes (0, 0);
    killTimer.setCallback (boost::bind (&ElementType::KillTimer::killTimeout,
    				        &killTimer, t.mName));
    killTimer.start ();
}

void
ElementType::destroy ()
{
    ELEMENTS_SCREEN (screen);

    std::list<ElementAnimation>::iterator it = es->priv->animations.begin ();
    while (it != es->priv->animations.end ())
    {
	ElementAnimation &anim = *it;
	if (mName == anim.priv->mTypename)
	{
	    anim.priv->mTexture.clear ();
	    anim.priv->mElements.clear ();
	    it = es->priv->animations.erase (it);
	}
	else
	    it++;
    }

    delete this;
}

ElementType *
ElementType::create (CompString     name,
		     CompString	    desc,
		     elementAddFunc addFunc)
{
    ElementType *type = new ElementType (name, desc, addFunc);

    ELEMENTS_SCREEN (screen);

    es->priv->types.push_back (type);

    return type;
}

ElementType *
ElementType::create ()
{
    ElementType *type = new ElementType ();

    ELEMENTS_SCREEN (screen);

    es->priv->types.push_back (type);

    return type;
}

void
ElementType::setName (CompString name,
		      CompString desc)
{
    mName = name;
    mDesc = desc;
}

void
ElementType::setCallbacks (elementAddFunc add)
{
    addFunc = add;
}

CompString
ElementType::name ()
{
    return mName;
}

CompString
ElementType::desc ()
{
    return mDesc;
}

ElementType *
ElementType::find (CompString name)
{
    ELEMENTS_SCREEN (screen);

    foreach (ElementType *type, es->priv->types)
    {
	if (type->name () == name)
	{
	    return type;
	}
    }

    return NULL;
}
