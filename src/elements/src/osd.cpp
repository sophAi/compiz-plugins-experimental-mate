/**
 * Compiz elements plugin
 * osd.cpp
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
#include <cmath>

/* Element Text Display Management */

ElementsTextSurface::ElementsTextSurface () :
    nTexture (0),
    valid (false),
    drawX (0),
    drawY (0)
{
    ELEMENTS_SCREEN (screen);

    textAttrib.family = "Sans";
    textAttrib.size = es->priv->optionGetTitleFontSize ();
    textAttrib.color[0] = es->priv->optionGetTitleFontColorRed ();
    textAttrib.color[1] = es->priv->optionGetTitleFontColorGreen ();
    textAttrib.color[2] = es->priv->optionGetTitleFontColorBlue ();
    textAttrib.color[3] = es->priv->optionGetTitleFontColorAlpha ();
    textAttrib.flags = (CompText::StyleBold |
		        CompText::WithBackground |
			CompText::Ellipsized);
    
    textAttrib.maxWidth = 512;
    textAttrib.maxHeight = 128;
    
    textAttrib.bgHMargin = 10.0f;
    textAttrib.bgVMargin = 10.0f;
    textAttrib.bgColor[0] = es->priv->optionGetTitleBackColorRed ();
    textAttrib.bgColor[1] = es->priv->optionGetTitleBackColorGreen ();
    textAttrib.bgColor[2] = es->priv->optionGetTitleBackColorBlue ();
    textAttrib.bgColor[3] = es->priv->optionGetTitleBackColorAlpha ();

    text = new CompText ();

    if (text)
	valid = true;
}

bool
ElementsTextSurface::render (ElementType *type,
			     int iter)
{
    ELEMENTS_SCREEN (screen);

    bool success;

    if (!text || !valid || !type)
	return false;

    success = text->renderText (type->desc (), textAttrib);

    /* Create the temporary texture to display alongside
     * the text display */

    if (success)
    {
	createTextureForType (type->name (), iter, text->getHeight ());
	es->priv->addTimeouts ((eTextures.size () > 1));

	es->priv->cScreen->damageScreen ();
    }

    return success;
}

bool
ElementsTextSurface::render (CompString message)
{
    ELEMENTS_SCREEN (screen);

    bool success;

    if (!text && !valid)
	return false;

    success = text->renderText (message, textAttrib);

    if (success)
    {
	es->priv->addTimeouts (false);

	es->priv->cScreen->damageScreen ();

    }

    return success;
}

ElementsTextSurface::~ElementsTextSurface ()
{
}

void
ElementsTextSurface::draw (const GLMatrix &transform, int x, int y)
{
    GLboolean wasBlend;
    GLint     oldBlendSrc, oldBlendDst;
    CompRect  outputRect;// == getOutputRect?;
    CompRegion totalPictureRegion;
    ElementTexture *eTex;
    border = 5;
    drawX = x;
    drawY = y;

    ELEMENTS_SCREEN (screen);

    glGetIntegerv (GL_BLEND_SRC, &oldBlendSrc);
    glGetIntegerv (GL_BLEND_DST, &oldBlendDst);
    wasBlend	= glIsEnabled (GL_BLEND);

    if (!wasBlend)
	glEnable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f (1.0f, 1.0, 1.0f, 0.7f);



    text->draw (transform,
		x - (text->getWidth () / 2) - totalPictureRegion.boundingRect ().width () - 20,
		y - text->getHeight (),
		(float) es->priv->optionGetTitleBackColorAlpha () / OPAQUE);

    /* Draw the elements bubble */

    if (eTextures.size ())
    {
	eTex = eTextures[nTexture];

	glPushMatrix ();

	glColor4f ((float) es->priv->optionGetTitleBackColorRed () / OPAQUE,
	   (float) es->priv->optionGetTitleBackColorGreen () / OPAQUE,
	   (float) es->priv->optionGetTitleBackColorBlue () / OPAQUE,
	   (float) es->priv->optionGetTitleBackColorAlpha () / OPAQUE);

	glTranslatef (x + text->getWidth () - eTex->size ().width (),
	      y - text->getHeight () * 2,
	      0.0f);
	glRectf (0.0f, eTex->size ().height (), eTex->size ().width (), 0.0f);
	glRectf (0.0f, 0.0f, eTex->size ().width (), -border);
	glRectf (0.0f, eTex->size ().height () + border,
	       eTex->size ().width (), eTex->size ().height ());
	glRectf (-border, eTex->size ().height (), 0.0f, 0.0f);
	glRectf (eTex->size ().width (), eTex->size ().height (),
		 eTex->size ().width () + border, 0.0f);
	glTranslatef (-border, -border, 0.0f);

#define CORNER(a,b) \
for (int k = a; k < b; k++) \
{\
float rad = k * (M_PI / 180.0f);\
glVertex2f (0.0f, 0.0f);\
glVertex2f (cos (rad) * border, sin (rad) * border);\
glVertex2f (cos ((k - 1) * (M_PI / 180.0f)) * border, \
	    sin ((k - 1) * (M_PI / 180.0f)) * border);\
}

	glTranslatef (border, border, 0.0f);
	glBegin (GL_TRIANGLES);
	CORNER (180, 270) glEnd ();
	glTranslatef (-border, -border, 0.0f);

	glTranslatef (eTex->size ().width () + border, border, 0.0f);
	glBegin (GL_TRIANGLES);
	CORNER (270, 360) glEnd ();
	glTranslatef (-(eTex->size ().width () + border), -border, 0.0f);

	glTranslatef (border, eTex->size ().height () + border, 0.0f);
	glBegin (GL_TRIANGLES);
	CORNER (90, 180) glEnd ();
	glTranslatef (-border, -(eTex->size ().height () + border), 0.0f);

	glTranslatef (eTex->size ().width () + border, eTex->size ().height () + border, 0.0f);
	glBegin (GL_TRIANGLES);
	CORNER (0, 90) glEnd ();
	glTranslatef (-(eTex->size ().width () + border), -(eTex->size ().height () + border), 0.0f);

	glColor4usv(defaultColor);

	glTranslatef (border, border, 0.0f);

	eTex->draw ();

	glTranslatef ((border), (border), 0.0f);
    }

#undef CORNER

    glPopMatrix ();

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor4usv (defaultColor);

    if (!wasBlend)
	glDisable (GL_BLEND);
    glBlendFunc (oldBlendSrc, oldBlendDst);
}

void
ElementsTextSurface::damageRegion ()
{
    ELEMENTS_SCREEN (screen);

    CompRect totalRect;

    if (eTextures.size ())
    {
	CompRect withElementRect (drawX - text->getWidth () + border * 3,
				  drawY - text->getHeight () * 2 - border,
				  text->getWidth () + border * 4, 
				  text->getHeight () + border * 2);

	totalRect = withElementRect;
    }
    else
    {
	CompRect withoutElementRect (drawX - text->getWidth () / 2 - border * 4,
				     drawY - text->getHeight () * 2,
				     text->getWidth () + border * 2,
				     text->getHeight () + border);

	totalRect = withoutElementRect;
    }

    foreach (ElementTexture *eTex, eTextures)
    {
	if (text->getWidth () + border * 4 + eTex->size ().width () >
	    totalRect.width ())
	    totalRect.setWidth (text->getWidth () + border * 4 +
				eTex->size ().width ());

	if (eTex->size ().height ()  + border * 2 > totalRect.height ())
	    totalRect.setHeight (eTex->size ().height ()  + border * 2);
    }

    CompRegion regionDamage (totalRect); 

    es->priv->cScreen->damageRegion (regionDamage);
}

void
ElementsTextSurface::createTextureForType (CompString type, int iter, int size)
{
    ELEMENTS_SCREEN (screen);


    CompOption::Value::Vector paths = es->priv->optionGetElementImage ();
    CompOption::Value::Vector iters = es->priv->optionGetElementIter ();

    eTextures.setTextures (type, &paths, &iters, size, iter);
}

/* Timeout callbacks */

bool
PrivateElementScreen::removeText ()
{
   /* We have to disable switchTimer here otherwise it will try to execute
    * memory that has been deleted since ElementsTextSurface::switchTextures ()
    * is not a static member function */

    if (switchTimer.active ())
	switchTimer.stop ();

    if (text)
    {
	text->damageRegion ();

	delete text;

	text = NULL;
    }

    return false;
}

bool
ElementsTextSurface::switchTextures ()
{
    nTexture = (nTexture + 1) % (eTextures.size ());
    ElementScreen::get (screen)->priv->cScreen->damageScreen ();

    return true;
}

/* TODO: This */

bool
PrivateElementScreen::addTimeouts (bool switchIt)
{
   int time = optionGetTitleDisplayTime ();

   if (textTimer.active ())
	textTimer.stop ();

   /* The timer might already be active and we may be switching to a non-switching
    * text display */

   if (switchTimer.active ())
	switchTimer.stop ();

   textTimer.start (time, time * 1.2);


   if (switchIt && text)
   {
	switchTimer.start (boost::bind
			     	   (&ElementsTextSurface::switchTextures,
			      	    text),
			     	   time / text->eTextures.size (),
			     	   (time * 2) / text->eTextures.size ());
   }

   return true;
}
