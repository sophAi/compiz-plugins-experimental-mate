/**
 * Compiz elements plugin
 * elements.cpp
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

/* Tomorrow: Make factories for elementAnimation, elementType etc,
   	     c++ versions of element specific code
 */

#include <cmath>
#include "private.h"

using namespace boost::placeholders;

COMPIZ_PLUGIN_20090315 (elements, ElementsPluginVTable);

int displayPrivateIndex;
int functionsPrivateIndex;

int
ElementScreen::getRand (int min,
		 	int max)
{
    return (rand () % (max - min + 1) + min);
}

float
ElementScreen::mmRand (int   min,
		       int   max,
		       float divisor)
{
    return ((float) getRand (min, max)) / divisor;
}

int
ElementScreen::boxing ()
{
    return priv->optionGetScreenBoxing ();
}

int
ElementScreen::depth ()
{
    return priv->optionGetScreenDepth ();
}

int
ElementScreen::updateDelay ()
{
    return priv->optionGetUpdateDelay ();
}

bool
PrivateElementScreen::displayTextInfo (CompOption::Value::Vector cType)
{

    if (cType.size () < 1)
    {

	if (text)
	    delete text;

	text = new ElementsTextSurface ();

	if (text && text->valid)
	    text->render (CompString ("No elements have been defined"));

	return false;

    }

    if (cType.size () > 0)
    {
	ElementType_ *type;
	type = ElementType::find (cType.at (listIter).s ());

	if (!type)
        {
	    if (text)
	        delete text;

	    text = new ElementsTextSurface ();

	    if (text && text->valid)
	        text->render (CompString("Error - Element image was not found"
				         " or is invalid"));
	    return false;
	}
	else
	{

	    if (text)
	        delete text;

	    text = NULL;

	    text = new ElementsTextSurface ();

	    if (text && text->valid)
	        text->render (type, animIter);

	    return true;

	}

    }

    return false;

}

bool
PrivateElementScreen::nextElement (CompAction              *action,
			     	   CompAction::State       state,
		 	     	   CompOption::Vector      &options)
{
    CompOption::Value::Vector   cType, cPath, cIter;
    CompString                  name;
    unsigned int                i;
    bool                        foundHigher = false;

    cType = optionGetElementType ();
    cPath = optionGetElementImage ();
    cIter = optionGetElementIter ();

    if (!((cType.size ()  == cIter.size ()) &&
         (cPath.size ()  == cIter.size ())))
    {
	compLogMessage ("elements", CompLogLevelWarn,
    			"Options are not set correctly,"
    			" cannot read this setting.");
	return false;
    }

    for (i = 0; i < cIter.size (); i++)
    {
	if (cIter.at (i).i () > animIter)
	{
	    foundHigher  = true;
	    listIter = i;
	    animIter = cIter.at (i).i ();
	    break;
	}
    }

    if (!foundHigher)
    {
	int lowest = MAXSHORT;
	listIter = 0;
	for (i = 0; i < cIter.size (); i++)
	    if (cIter.at (i).i () < lowest)
		lowest = cIter.at (i).i ();

	animIter = lowest;
    }

    displayTextInfo (cType);

    return true;
}

bool
PrivateElementScreen::prevElement (CompAction              *action,
		     	    	   CompAction::State       state,
	 	     	    	   CompOption::Vector      &options)
{
    CompOption::Value::Vector   cType, cPath, cIter;
    CompString                name;
    int              	      i;
    bool                      foundLower = false;

    cType = optionGetElementType ();
    cPath = optionGetElementImage ();
    cIter = optionGetElementIter ();

    if (!((cType.size ()  == cIter.size ()) &&
         (cPath.size ()  == cIter.size ())))
    {
	compLogMessage ("elements", CompLogLevelWarn,
    			"Options are not set correctly,"
    			" cannot read this setting.");
	return false;
    }

    for (i = cIter.size () -1; i > -1; i--)
    {
	if (cIter.at (i).i () < animIter)
	{
	    foundLower  = true;
	    listIter = i;
	    animIter = cIter.at (i).i ();
	    break;
	}
    }

    if (!foundLower)
    {
	int highest = 0;
	listIter = 0;
	for (i = 0; i < (int) cIter.size (); i++)
	    if (cIter.at (i).i () > highest)
		highest = cIter.at (i).i ();

	animIter = highest;
	for (i = 0; i < (int) cIter.size (); i++)
	    if (cIter.at (i).i () == highest)
		break;

	listIter = i;
    }

    displayTextInfo (cType);


    return true;
}

bool
PrivateElementScreen::isAnimationActive (const CompString &type)
{
    foreach (ElementAnimation &anim, animations)
    {
	if (anim.type ()->name () == type)
	{
	    if (anim.active ())
		return true;
	}
    }
    return false;
}

void
PrivateElementScreen::stopAnimation (const CompString &type)
{
    foreach (ElementAnimation &anim, animations)
    {
	if (anim.type ()->name () == type)
	{
	    if (anim.active ())
		anim.stop ();
	}
    }
}

void
PrivateElementScreen::startAnimation (const CompString &type)
{
    bool started = false;

    foreach (ElementAnimation &anim, animations)
    {
	if (anim.type ()->name () == type)
	{
	    if (!anim.active ())
		anim.start ();
	    started = true;
	}
    }

    if (!started)
    {
	CompOption::Value::Vector cIter  = optionGetElementIter  ();
	CompOption::Value::Vector cType  = optionGetElementType  ();
	CompOption::Value::Vector cCap   = optionGetElementCap   ();
	CompOption::Value::Vector cSize  = optionGetElementSize  ();
	CompOption::Value::Vector cSpeed = optionGetElementSpeed ();
	CompOption::Value::Vector cRot   = optionGetElementRotate ();

	if (cType.size () == cIter.size () &&
	    cCap.size () == cIter.size () &&
	    cSize.size () == cIter.size () &&
	    cSpeed.size () == cIter.size () &&
	    cRot.size () == cIter.size ())
	{
	    if (!redrawTimer.active ())
		redrawTimer.start ();

	    int savedIter = 0;
	    for (unsigned int i = 0; i < cIter.size (); i++)
	    {
		if (cType.at (i).s () == type &&
		    savedIter != cIter.at (i).i ())
		{
		    savedIter = cIter.at (i).i ();

		    ElementAnimation &anim =
			       ElementAnimation::create (cType.at (i).s (),
							 cCap.at (i).i (),
							 cSize.at (i).i (),
							 cSpeed.at (i).i (),
							 cIter.at (i).i (),
							 cRot.at (i).b ());

		    anim.start ();
		    break;
		}
	    }
	}
    }
}

void
PrivateElementScreen::setAnimationRunning (const CompString &type, bool running)
{
    if (running)
	startAnimation (type);
    else
	stopAnimation (type);
}

bool
PrivateElementScreen::toggleElementName (CompAction         *action,
					 CompAction::State  state,
					 CompOption::Vector &options)
{
    CompString elementType = CompOption::getStringOptionNamed (options, "type", "");

    if (elementType.empty ())
	return false;

    if (isAnimationActive (elementType))
	stopAnimation (elementType);
    else
	startAnimation (elementType);

    return true;
}


bool
PrivateElementScreen::toggleSelected (CompAction         *action,
				      CompAction::State  state,
				      CompOption::Vector &options)
{
    CompString             	   string;
    bool			   foundAnim = false;
    CompOption::Value::Vector    cIter  = optionGetElementIter  ();
    CompOption::Value::Vector    cType  = optionGetElementType  ();
    CompOption::Value::Vector    cPath  = optionGetElementImage ();
    CompOption::Value::Vector    cCap   = optionGetElementCap   ();
    CompOption::Value::Vector    cSize  = optionGetElementSize  ();
    CompOption::Value::Vector    cSpeed = optionGetElementSpeed ();
    CompOption::Value::Vector    cRot   = optionGetElementRotate ();

    if (!((cType.size ()  == cIter.size ()) &&
	  (cPath.size ()  == cIter.size ()) &&
	  (cCap.size ()   == cIter.size ()) &&
	  (cSize.size ()  == cIter.size ()) &&
	  (cSpeed.size () == cIter.size ()) &&
	  (cRot.size ()   == cIter.size ())))
    {
	compLogMessage ("elements", CompLogLevelWarn,
			"Options are not set correctly,"
			" cannot read this setting.");
	return false;
    }

    if (optionGetTitleOnToggle ())
	if (!displayTextInfo (cType))
	    return true;

    foreach (ElementAnimation &anim, animations)
    {
	if (anim.id () == animIter)
	{
	    if (anim.active ())
		anim.stop ();
	    else
		anim.start ();

	    foundAnim = true;

	    break;
	}
    }

    if (!foundAnim)
    {
	if (!redrawTimer.active ())
	    redrawTimer.start ();

	ElementAnimation &anim =
		   ElementAnimation::create (cType.at (listIter).s (),
					     cCap.at (listIter).i () ,
					     cSize.at (listIter).i (),
					     cSpeed.at (listIter).i (),
					     animIter,
					     cRot.at (listIter).b ());

	anim.start ();
    }

    return true;
}

bool
PrivateElementScreen::switchTimeout ()
{
    if (!text || animations.empty ())
	return false;

    if (text)
    {
	if (!text->eTextures.empty ())
	{
	    text->nTexture = 
		(text->nTexture + 1) %
		    (text->eTextures.size ());
	}
    }

    if (!animations.empty ())
    {
	foreach (ElementAnimation &anim, animations)
	{
	    anim.setNTexture ((anim.nTexture () + 1) % (anim.textures ().size ()));
	}
    }

    return true;
}

void
PrivateElementScreen::setupDisplayList ()
{
    displayList = glGenLists (1);
    glNewList (displayList, GL_COMPILE);
    glBegin (GL_QUADS);
    glColor4f (1.0, 1.0, 1.0, 1.0);
    glVertex3f (0, 0, -0.0);
    glColor4f (1.0, 1.0, 1.0, 1.0);
    glVertex3f (0, 1.0, -0.0);
    glColor4f (1.0, 1.0, 1.0, 1.0);
    glVertex3f (1.0, 1.0, -0.0);
    glColor4f (1.0, 1.0, 1.0, 1.0);
    glVertex3f (1.0, 0, -0.0);
    glEnd ();
    glEndList ();
}

void
PrivateElementScreen::render (const GLMatrix &transform)
{
    glDisable (GL_CULL_FACE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (needUpdate)
    {
	setupDisplayList ();
	needUpdate = false;
    }

    foreach (ElementAnimation &anim, animations)
    {
	if (anim.textures ().size () > 0)
	{
	    int		    n = anim.nTexture () % anim.textures ().size ();
	    ElementTexture *eTex =  (anim.textures () [n]);

	    for (unsigned int i = 0; i < anim.elements ().size (); i++)
	    {
		const Element &e = anim.elements ()[i];
		glPushMatrix ();
		glLoadMatrixf (transform.getMatrix ());
		glColor4f (1.0, 1.0, 1.0, e.opacity);
		glTranslatef (e.x, e.y, e.z);
		glRotatef (e.rAngle, 0, 0, 1);
		eTex->draw ();
		glRotatef (-e.rAngle, 0, 0, 1);
		glTranslatef (-e.x, -e.y, -e.z);
		glPopMatrix ();
	    }
	}
    }

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}


bool
PrivateElementScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
				     const GLMatrix            &transform,
				     const CompRegion          &region,
				     CompOutput                *output,
				     unsigned int              mask)
{
    bool status;
    if (!animations.empty () && optionGetOverWindows ())
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (textTimer.active () || (!animations.empty () && optionGetOverWindows ()))
    {
	GLMatrix sTransform = transform;
	CompRect outputRect;

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);
	outputRect = (CompRect) *output;

	/* TODO: Option to determine where text goes */

        int x = (outputRect.x1 () + (outputRect.width () / 2));
	int y = (outputRect.y1 () + (outputRect.height () / 2));

	if (!animations.empty () && optionGetOverWindows ())
	    render (sTransform);
	if (textTimer.active () && text)
	{
	    glPushMatrix ();
	    glLoadMatrixf (sTransform.getMatrix ());
	    text->draw (sTransform, x, y);
	    glPopMatrix ();
	}
    }
    return status;
}

bool
ElementsWindow::glDraw (const GLMatrix            &transform,
			const GLWindowPaintAttrib &attrib,
			const CompRegion          &region,
			unsigned int              mask)
{
    bool status;
    ELEMENTS_SCREEN (screen);

    /* Draw the window first then draw our elements in this region */

    status = gWindow->glDraw (transform, attrib, region, mask);

    if (!es->priv->animations.empty () && (window->type () & CompWindowTypeDesktopMask)
	&& !es->priv->optionGetOverWindows ())
    {
	es->priv->render (transform);
    }

    return status;
}

bool
PrivateElementScreen::redrawTimeout ()
{
    std::list <ElementAnimation>::iterator it = animations.begin ();

    if (animations.empty ())
	return false;

    while (it != animations.end ())
    {
	ElementAnimation &anim = *it;
	boost::ptr_vector <Element>::iterator eit = anim.priv->mElements.begin ();

	if (anim.active ())
	{
	    while (eit != anim.priv->mElements.end ())
	    {
		/* This tests to see if the element is offscreen, if so
		 * replace it with one at the beginning of its animation
		 * the screen if not, move it */
		Element &e = *eit;
		e.regenerateOffscreen ();
		eit++;
	    }
	}
	else
	{
	    /* This tests to see if all elements are offscreen. If they are,
	     * the animation is deleted (this is there to ensure that the animations
	     * finish "nicely", ie they don't just disappear when untoggled)
	     */

	    if (anim.priv->removeOffscreenElements ())
	    {
		anim.priv->mTexture.clear ();
		anim.priv->mElements.clear ();
		animations.erase (it);

		/* We must reset the list count here otherwise foreach will
		 * attempt to access an item in a list that we just deleted
		 */

		it = animations.begin ();
		continue;
	    }
	}

	eit = anim.priv->mElements.begin ();

	while (eit != anim.priv->mElements.end ())
	{
	    Element &e = *eit;
	    e.move ();
	    eit++;
	}

	it++;
    }

    /* Here, we damage the windows if drawing beneath them as the windows
     * are drawn on top of the elements. We damage the screen if drawing on
     * top.
     */

    if (!optionGetOverWindows ())
    {
	bool damaged = false;
	foreach (CompWindow *w, screen->windows ())
	{
	    if (w->type () & CompWindowTypeDesktopMask)
	    {
		ELEMENTS_WINDOW (w);
		ew->cWindow->addDamage ();
		damaged = true;
	    }
	}
	if (!damaged)
	    CompositeScreen::get (screen)->damageScreen ();
    }
    else
    {
	CompositeScreen::get (screen)->damageScreen ();
    }

    return true;
}

/* TODO: */

void
PrivateElementScreen::updateElementTextures ()
{
    foreach (ElementAnimation &anim, animations)
    {
	int           iter, nElement, size,speed;
	bool	      rotate;
	CompString    type;
        CompOption::Value::Vector    cIter  = optionGetElementIter  ();
        CompOption::Value::Vector    cType  = optionGetElementType  ();
        CompOption::Value::Vector    cPath  = optionGetElementImage ();
        CompOption::Value::Vector    cCap   = optionGetElementCap   ();
        CompOption::Value::Vector    cSize  = optionGetElementSize  ();
        CompOption::Value::Vector    cSpeed = optionGetElementSpeed ();
	CompOption::Value::Vector    cRot   = optionGetElementRotate ();

	if (!((cType.size ()  == cIter.size ()) &&
	      (cPath.size ()  == cIter.size ()) &&
	      (cCap.size ()   == cIter.size ()) &&
	      (cSize.size ()  == cIter.size ()) &&
	      (cRot.size ()   == cIter.size ()) &&
	      (cSpeed.size () == cIter.size ())))
	{
	    compLogMessage ("elements", CompLogLevelWarn,
			    "Options are not set correctly,"
			    " cannot read this setting.");
	    return;
	}

	iter     = anim.id ();
	nElement = cCap.at (anim.id () - 1).i ();
	type     = cType.at (anim.id () - 1).s ();
	size     = cSize.at (anim.id () - 1).i ();
	speed    = cSpeed.at (anim.id () - 1).i ();
	rotate   = cRot.at (anim.id () - 1).b ();

	anim.priv->mTexture.clear ();

	if (type != anim.type ()->name ())
	{
	    bool found = false;

	    foreach (ElementType_ *t, types)
	    {
		if (t->name () == type)
		    found = true;
	    }

	    if (!found)
		compLogMessage ("elements", CompLogLevelWarn,
				"Could not find element movement pattern %s",
				type.c_str ());

	    anim.stop ();

	    for (unsigned int i = 0; i < anim.priv->mElements.size (); i++)
	    {
		Element &e = anim.priv->mElements[i];
		e.fini ();
		i++;
	    }
	}

	if (anim.priv->applyTextures (anim.type ()->name (), &cPath, &cIter, size, iter))
	{
	    if (nElement != anim.nElement ())
	    {
		/* Implicitly or subtracts elements and handles memory */

		anim.setNElement (nElement);
		/* FIXME: NULL check? */
	    }

	    if (type != anim.type ()->name ())
	    {
		anim.setType (type);
	    }

	    anim.setRotate (rotate);
	    anim.setSize (size);
	    anim.setSpeed (speed);

	    for (unsigned int i = 0; i < anim.priv->mElements.size (); i++)
	    {
		Element &e = anim.priv->mElements[i];
		e.init ();
		i++;
	    }

	    anim.start ();

	}
    }
}

void
PrivateElementScreen::optionChanged (CompOption      	       *option,
			      	     ElementsOptions::Options num)
{
    switch (num)
    {
	case ElementsOptions::ElementType:
	case ElementsOptions::ElementImage:
	case ElementsOptions::ElementCap:
	case ElementsOptions::ElementSize:
	case ElementsOptions::ElementSpeed:
	case ElementsOptions::ElementRotate:
	{
	    needUpdate = true;
	    updateElementTextures ();

	}
	break;
    	case ElementsOptions::UpdateDelay:
	{
	    if (redrawTimer.active ())
		redrawTimer.stop ();
	    redrawTimer.setCallback (boost::bind (&PrivateElementScreen::redrawTimeout, this));
	    redrawTimer.setTimes (optionGetUpdateDelay (),
				  (float) optionGetUpdateDelay () * 1.2);
	    redrawTimer.start ();
	}
	break;
	default:
	    break;
    }
}

CompOption::Vector &
ElementScreen::getOptions ()
{
    return priv->getOptions ();
}

bool
ElementScreen::setOption (const CompString  &name,
			  CompOption::Value &value)
{
    return priv->setOption (name, value);
}

ElementsWindow::ElementsWindow (CompWindow *window) :
    PluginClassHandler <ElementsWindow, CompWindow> (window),
    window (window),
    gWindow (GLWindow::get (window)),
    cWindow (CompositeWindow::get (window))
{
    GLWindowInterface::setHandler (gWindow);
}

void
PrivateElementScreen::postLoad ()
{
    std::list<ElementAnimation>::iterator ait = animations.begin ();

    CompOption::Value::Vector cIter = optionGetElementIter ();

    /* The multi options list may have changed while we were disabled
     * if this happens then it really isn't worth trying to restore what
     * was, as this will likely cause havoc.
     */

    if (cIter.size () != multiOptionsListSize)
    {
	animations.clear ();
	return;
    }

    /* At this point, all the types should be loaded .... */

    while (ait != animations.end ())
    {
	ElementAnimation &anim = *ait;

	/* Don't need to start clearing and reloading lists here ...
	 * circumvent
	 */

	unsigned int tempNElement = anim.priv->mNElement;
	anim.priv->mNElement = 0;

	if (!anim.setType (anim.priv->mTypename))
	{
	    /* Couldn't find the type, bail and kill it */
	    animations.erase (ait);
	    ait = animations.begin ();
	    continue;
	}

	anim.setNElement (tempNElement);

	if (anim.priv->mActive)
	{
	    if (!redrawTimer.active ())
	    {
		redrawTimer.start ();
	    }
	}

	ait++;
    }

    updateElementTextures ();
}


PrivateElementScreen::PrivateElementScreen (CompScreen *screen) :
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    text (NULL),
    listIter (0),
    animIter (0),
    needUpdate (false)
{
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen);

    int lowest = MAXSHORT;
    unsigned int i;
    CompOption::Value::Vector cIter = optionGetElementIter ();

    for (i = 0; i < cIter.size (); i++)
	if (cIter.at (i).i () < lowest)
	    lowest = cIter.at (i).i ();


    multiOptionsListSize = cIter.size ();

    optionSetNextElementKeyInitiate (boost::bind
					  (&PrivateElementScreen::nextElement,
					   this, _1, _2, _3));

    optionSetPrevElementKeyInitiate (boost::bind
					   (&PrivateElementScreen::prevElement,
					    this, _1, _2, _3));

    optionSetToggleSelectedKeyInitiate (boost::bind
					      (&PrivateElementScreen::toggleSelected,
					       this, _1, _2, _3));

    optionSetToggleNameInitiate (boost::bind
					(&PrivateElementScreen::toggleElementName, this, _1, _2, _3));

    setupDisplayList ();
}

ElementScreen::ElementScreen (CompScreen *screen) :
    PluginClassHandler <ElementScreen, CompScreen> (screen)
{
    priv = new PrivateElementScreen (screen);

    if (!priv)
	setFailed ();

    if (priv)
    {

	priv->redrawTimer.setCallback (boost::bind (&PrivateElementScreen::redrawTimeout, this->priv));
	priv->redrawTimer.setTimes (priv->optionGetUpdateDelay (),
				priv->optionGetUpdateDelay () * 1.2);

	priv->textTimer.setCallback (boost::bind (&PrivateElementScreen::removeText, this->priv));
	priv->textTimer.setTimes (priv->optionGetTitleDisplayTime (),
				  priv->optionGetTitleDisplayTime () * 1.2);


	priv->updateElementTextures ();

#define optionNotify(name) \
priv->optionSet##name##Notify(boost::bind(&PrivateElementScreen::optionChanged, this->priv, _1, _2))

	optionNotify (ElementType);
	optionNotify (ElementImage);
	optionNotify (ElementSize);
	optionNotify (ElementSpeed);
	optionNotify (ElementCap);
	optionNotify (ElementRotate);
	optionNotify (UpdateDelay);

#undef optionNotify
    }
}

bool
ElementScreen::isAnimationActive (const CompString &type)
{
    return priv->isAnimationActive (type);
}

void
ElementScreen::startAnimation (const CompString &type)
{
    priv->startAnimation (type);
}

void
ElementScreen::stopAnimation (const CompString &type)
{
    priv->stopAnimation (type);
}

void
ElementScreen::setAnimationRunning (const CompString &type, bool running)
{
    priv->setAnimationRunning (type, running);
}

ElementScreen::~ElementScreen ()
{
    if (priv->redrawTimer.active ())
	priv->redrawTimer.stop ();

    if (priv->switchTimer.active ())
	priv->switchTimer.stop ();

    if (priv->textTimer.active ())
	priv->textTimer.stop ();

    delete priv;
}

PrivateElementScreen::~PrivateElementScreen ()
{
    while (!types.empty ())
    {
	ElementType_ *type = types.front ();
	types.remove (type);
    }
}

bool
ElementsPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;

    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    {
	compLogMessage ("elements", CompLogLevelError,
			"No compatible rendering plugin found!");
	return false;
    }

    CompPrivate p;
    p.uval = COMPIZ_ELEMENTS_ABI;
    screen->storeValue ("elements_ABI", p);

    return true;
}

void
ElementsPluginVTable::fini ()
{
    screen->eraseValue ("elements_ABI");
}
