/**
 * Compiz elements plugin
 * animation.cpp
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

ElementAnimation &
ElementAnimation::create (CompString type,
			  int	     nElement,
			  int	     size,
			  int	     speed,
			  int	     iter,
			  bool	     rotate)
{
    ELEMENTS_SCREEN (screen);
    
    es->priv->animations.push_back (ElementAnimation (type, nElement, size,
    						      speed, iter, rotate));

    ElementAnimation &anim = es->priv->animations.back ();
    CompOption::Value::Vector paths, iters;

    if (anim.type ())
    {
	paths = es->priv->optionGetElementImage ();
	iters = es->priv->optionGetElementIter  ();

	anim.priv->applyTextures (type, &paths, &iters, size, iter);

	for (int i = 0; i < anim.nElement (); i++)
	{
	    Element *e = anim.type ()->addFunc ();
	    e->anim = &anim;
	    e->defaultInit ();
	    e->init ();
	    anim.priv->mElements.push_back (e);
	}
	
	anim.priv->mValid = true;
	return es->priv->animations.back ();

    }
    else
    {
	compLogMessage ("elements", CompLogLevelWarn,
			"Could not find element movement pattern %s, "
			"disabling this element", type.c_str ());
	anim.priv->mValid = false;
	return es->priv->animations.back ();
    }
}

PrivateElementAnimation::PrivateElementAnimation (CompString type,
						  int nElement,
						  int size,
						  int speed,
						  int iter,
						  bool rotate) :
    mNElement (nElement),
    mSize (size),
    mSpeed (speed),
    mId (iter),
    mNTexture (0),
    mType (ElementType::find (type)),
    mTypename (type),
    mActive (false),
    mValid  (true),
    mRotate (rotate)
{
}

ElementAnimation::ElementAnimation (CompString type,
				    int nElement,
			  	    int size,
			  	    int speed,
			  	    int iter,
				    bool rotate)
{
    priv = new PrivateElementAnimation (type, nElement, size, speed, iter,
				        rotate);
}

ElementAnimation::~ElementAnimation ()
{
}

/* Check to see if an element is still within the screen. If it is,
 * return false and move all elements. If every element isn't onscreen,
 * return true
 */

bool
PrivateElementAnimation::removeOffscreenElements ()
{
    ELEMENTS_SCREEN (screen);
    bool    ret = true;
    int	    count = 0;
    int	    nElem = mElements.size ();

    /* Reverse the stack, remove elements that are no longer needed */

    while (count != nElem)
    {
	Element &e = mElements.back ();

	if (e.opacity > 0.0f)
	    e.opacity -= 0.03f;
	if (!(e.y >= screen->height () + 200                       ||
	      e.x <= -200                                          ||
	      e.x >= screen->width () + 200                        ||
	      e.y >= screen->height () + 200                       ||
	      e.z <= -((float) es->depth () / 500.0) 		||
	      e.z >= 1                                             ||
	      e.y <= -((float) es->boxing () / 5.0f)) &&
	      e.opacity > 0.0f)
	{
	    ret = false;
	}
	else
	{
	    e.fini ();
	    mElements.pop_back ();
	}
	
	count++;
    }

    return ret;
}

/* Apply textures to an animation */

bool
PrivateElementAnimation::applyTextures (CompString		   type,
					CompOption::Value::Vector *paths,
					CompOption::Value::Vector *iters,
					int			   size,
					int			   iter)
{
    mTexture.setTextures (type, paths, iters, size, iter);

    return true;
}

bool
ElementAnimation::start ()
{
    ELEMENTS_SCREEN (screen);

    priv->mActive = true;

    if (!es->priv->redrawTimer.active ())
	es->priv->redrawTimer.start ();

    return true;
}

void
ElementAnimation::stop ()
{
    priv->mActive = false;
}

void
ElementAnimation::setNElement (int n)
{
    if (priv->mNElement > n)
    {
	int count = priv->mNElement - n;

	/* Keep on removing elements off the top of list */

	while (count--)
	{
	    Element &e = priv->mElements.back ();
	    e.fini ();
	    priv->mElements.pop_back ();
	}
   }
   else if (n > priv->mNElement)
   {
	int count = n - priv->mNElement;

	/* Add the difference */

	while (count--)
	{
	    Element *e = priv->mType->addFunc ();
	    e->anim = this;
	    e->defaultInit ();
	    e->init ();
	    priv->mElements.push_back (e);
	}
   }

    priv->mNElement = n;
}

void
ElementAnimation::setSize (int n)
{
    priv->mSize = n;
}

void
ElementAnimation::setSpeed (int n)
{
    priv->mSpeed = n;
}

void
ElementAnimation::setId (int n)
{
    priv->mId = n;
}

bool
ElementAnimation::setType (CompString name)
{
    priv->mTypename = name;

    priv->mType = ElementType::find (name);

    if (priv->mType)
    {

	/* Fini all elements */
	int nElement = priv->mNElement;

	while (nElement--)
	{
	    Element &e = priv->mElements.back ();
	    e.fini ();
	    priv->mElements.pop_back ();
	}

	nElement = priv->mNElement;

	/* Re-create all elements with new type */

	while (nElement--)
	{
	    priv->mElements.push_back (priv->mType->addFunc ());
	}
    }   
    else
    {
	return false;
    }

    return true;
}

void
ElementAnimation::setRotate (bool r)
{
    priv->mRotate = r;
}

ElementTexture::List
ElementAnimation::textures ()
{
    return priv->mTexture;
}

const boost::ptr_vector<Element> &
ElementAnimation::elements ()
{
    return priv->mElements;
}

void
ElementAnimation::setNTexture (int c)
{
    priv->mNTexture = c;
}

int
ElementAnimation::nElement ()
{
    return priv->mNElement;
}

int ElementAnimation::size ()
{
    return priv->mSize;
}

int ElementAnimation::speed ()
{
    return priv->mSpeed;
}

int ElementAnimation::id ()
{
    return priv->mId;
}

int ElementAnimation::nTexture ()
{
    return priv->mNTexture;
}

bool ElementAnimation::valid ()
{
    return priv->mValid;
}

ElementType *
ElementAnimation::type ()
{
    return priv->mType;
}

bool
ElementAnimation::rotate ()
{
    return priv->mRotate;
}

bool
ElementAnimation::active ()
{
    return priv->mActive;
}
