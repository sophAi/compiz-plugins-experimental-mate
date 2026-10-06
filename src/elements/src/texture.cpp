/**
 * Compiz elements plugin
 * texture.cpp
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

#include <unistd.h>
#include "private.h"

ElementTexture::ElementTexture (CompString path,
				int        texSize,
				CompString name) :
    valid (true)
{
    setTexture (path, texSize, name);
}

void
ElementTexture::draw ()
{

    if (textures.empty ())
	return;

    for (unsigned int i = 0; i < textures.size (); i++)
    {
	GLTexture         *tex = textures[i];
	GLuint		  dList = dLists[i];

	tex->enable (GLTexture::Good);

	glCallList (dList);

	tex->disable ();
    }
}

CompSize
ElementTexture::size ()
{
    return CompSize (width, height);
}

void
ElementTexture::setTexture (CompString path, int texSize, CompString name)
{
    CompSize textureSize (texSize, texSize);
    CompString pname ("elements");
    textures = GLTexture::readImageToTexture (path, pname, textureSize);

    if (textures.empty ())
    {
	std::vector<CompString> candidates;
	candidates.push_back ("/usr/share/compiz/elements/images/" + path);
	candidates.push_back ("/usr/share/compiz/elements/" + path);
	candidates.push_back ("/usr/share/compiz/" + path);

	const char *home = getenv ("HOME");
	if (home)
	{
	    candidates.push_back (CompString (home) + "/.compiz-1/elements/images/" + path);
	    candidates.push_back (CompString (home) + "/.compiz-1/elements/" + path);
	    candidates.push_back (CompString (home) + "/.compiz-1/data/elements/images/" + path);
	    candidates.push_back (CompString (home) + "/.compiz-1/data/elements/" + path);
	}

	if (path.length () > 4 && path.substr (path.length () - 4) == ".svg")
	{
	    CompString pngPath = path.substr (0, path.length () - 4) + ".png";
	    candidates.push_back (pngPath);
	    candidates.push_back ("/usr/share/compiz/elements/images/" + pngPath);
	    candidates.push_back ("/usr/share/compiz/elements/" + pngPath);
	    if (home)
	    {
		candidates.push_back (CompString (home) + "/.compiz-1/elements/images/" + pngPath);
		candidates.push_back (CompString (home) + "/.compiz-1/elements/" + pngPath);
		candidates.push_back (CompString (home) + "/.compiz-1/data/elements/images/" + pngPath);
	    }
	}
	else if (path.length () > 4 && path.substr (path.length () - 4) == ".png")
	{
	    CompString svgPath = path.substr (0, path.length () - 4) + ".svg";
	    candidates.push_back (svgPath);
	    candidates.push_back ("/usr/share/compiz/elements/images/" + svgPath);
	    candidates.push_back ("/usr/share/compiz/elements/" + svgPath);
	}

	for (size_t i = 0; i < candidates.size (); i++)
	{
	    if (access (candidates[i].c_str (), R_OK) == 0)
	    {
		CompSize fallbackSize (texSize, texSize);
		textures = GLTexture::readImageToTexture (candidates[i], pname, fallbackSize);
		if (!textures.empty ())
		{
		    textureSize = fallbackSize;
		    compLogMessage ("elements", CompLogLevelInfo,
				    "Loaded Texture from fallback %s for animation %s",
				    candidates[i].c_str (), name.c_str ());
		    break;
		}
	    }
	}
    }

    width = texSize;
    height = texSize;

    foreach (GLuint dList, dLists)
    {
	glDeleteLists (dList, 1);
    }
    dLists.clear ();

    if (!textures.size ())
    {
	compLogMessage ("elements", CompLogLevelWarn,
			"Texture for animation %s not found at"
			" location %s or invalid",
     		   	name.c_str (), path.c_str ());
	valid = false;
    }
    else
    {
	valid = true;
	compLogMessage ("elements", CompLogLevelInfo,
			"Loaded Texture %s for animation %s",
			path.c_str (), name.c_str ());
    }

    foreach (GLTexture *tex, textures)
    {
	GLTexture::Matrix mat = tex->matrix ();
	GLuint	      dList = glGenLists (1);

	glNewList (dList, GL_COMPILE);
	glBegin (GL_QUADS);

	glTexCoord2f (COMP_TEX_COORD_X (mat, 0),
		     COMP_TEX_COORD_Y (mat, 0));
	glVertex2f (0, 0);
	glTexCoord2f (COMP_TEX_COORD_X (mat, 0),
		     COMP_TEX_COORD_Y (mat, textureSize.height ()));
	glVertex2f (0, texSize * height / width);
	glTexCoord2f (COMP_TEX_COORD_X (mat, textureSize.width ()),
		      COMP_TEX_COORD_Y (mat, textureSize.height ()));
	glVertex2f (texSize, texSize * height / width);
	glTexCoord2f (COMP_TEX_COORD_X (mat, textureSize.width ()),
		      COMP_TEX_COORD_Y (mat, 0));
	glVertex2f (texSize, 0);

	glEnd ();
	glEndList ();

	dLists.push_back (dList);
    }
}

ElementTexture::List::List () :
    std::vector<ElementTexture *> (0)
{
}

ElementTexture::List::List (unsigned int size) :
    std::vector<ElementTexture *> (size)
{
    for (unsigned int i = 0; i < size; i++)
	at (i) = NULL;
}

ElementTexture::List::List (CompString		      name,
			    CompOption::Value::Vector *paths,
			    CompOption::Value::Vector *iters,
			    int			      &size,
			    int			      &iter)
{
    setTextures (name, paths, iters, size, iter);
}

ElementTexture::List::~List ()
{
}

void
ElementTexture::List::setTextures (CompString		      name,
			    	   CompOption::Value::Vector *paths,
			    	   CompOption::Value::Vector *iters,
			    	   int			      &size,
			    	   int			      &iter)
{
    for (unsigned int i = 0; i < iters->size (); i++)
    {
	if (iters->at (i).i () == iter)
	{
	    if (paths->at (i).s ().c_str ())
	    {
		ElementTexture *eTex =
			new ElementTexture (paths->at (i).s (), size, name.c_str ());

		if (eTex->valid)
		    push_back (eTex);
	    }
	}
    }
}

ElementTexture::List &
ElementTexture::List::operator= (const ElementTexture::List c)
{
    this->clear ();
    resize (c.size ());
    for (unsigned int i = 0; i < c.size (); i++)
    {
	at (i) = c[i];
    }
    return *this;
}

ElementTexture::~ElementTexture ()
{
    foreach (GLuint dList, dLists)
    {
	glDeleteLists (dList, 1);
    }
}
