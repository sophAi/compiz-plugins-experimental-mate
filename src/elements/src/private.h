/**
 * Compiz elements plugin
 * private.h
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
#include "elements_options.h"

/* This is the number used to make sure that elements don't get all
   created in one place. Bigger number, more distance.
   Possibly fixable later. -> s->width? */
#define BIG_NUMBER		20000

typedef ElementType ElementType_; // Bug in gcc?

class ElementsTextSurface
{
    public:

	ElementsTextSurface ();
	~ElementsTextSurface ();

	int nTexture;

	void
	draw (const GLMatrix &transform, int x, int y);

	bool
	render (ElementType *, int);

	bool
	render (CompString);

	void
	damageRegion ();

	void
	createTextureForType (CompString type, int iter, int size);

	bool switchTextures ();

    private:

	CompText::Attrib textAttrib;
	CompText *text;
	ElementTexture::List eTextures;
	bool valid;

	int drawX, drawY, border;

    friend class ElementScreen;
    friend class PrivateElementScreen;
};

class PrivateElementAnimation
{
    public:

	PrivateElementAnimation (CompString type,
				  int nElement,
				  int size,
				  int speed,
				  int iter,
				  bool rotate);
	
	PrivateElementAnimation () {};

	bool applyTextures (CompString,
			    CompOption::Value::Vector *paths,
			    CompOption::Value::Vector *iters,
			    int			      size,
			    int			      iter);

	bool removeOffscreenElements ();
	
	template <class Archive>
	void serialize (Archive &ar, const unsigned int version)
	{
	    ar & mNElement;
	    ar & mSize;
	    ar & mSpeed;
	    ar & mId;
	    ar & mNTexture;
	    ar & mTypename;
	    ar & mActive;
	    ar & mValid;
	    ar & mRotate;
	    // ar & mTexture;
	    /* Until we can resolve serialization type registration at
	     * runtime, it will not be possible to save individual elements
	     * ar & mElements;
	     */
	}

	int mNElement;
	int mSize;
	int mSpeed;
	int mId;

	int mNTexture;

	ElementType *mType;
	CompString  mTypename; // Used for serialization purposes

	bool mActive;
	bool mValid;

	bool mRotate;

	ElementTexture::List mTexture;
	boost::ptr_vector<Element> mElements;
};

class PrivateElementScreen :
    public ElementsOptions,
    public GLScreenInterface,
    public CompositeScreenInterface
{
    public:

	PrivateElementScreen (CompScreen *screen);
	~PrivateElementScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	CompTimer redrawTimer;
	CompTimer switchTimer;
	CompTimer textTimer;


	/* Used to kill animations at the top
	 * of handleEvent (), so that we can
	 * serialize animation data before
	 * it gets killed by all the subplugins
	 * unloading. */
	std::list <ElementType::KillTimer> killAnimationsTimers; 
	
	template <class Archive>
	void serialize (Archive &ar, const unsigned int version)
	{
	    ar & listIter;
	    ar & animIter;
	    ar & needUpdate;
	    ar & animations;
	    //ar & types;
	}
	
	void postLoad ();

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix &,
		       const CompRegion &,
		       CompOutput *,
		       unsigned int);

	bool      drawText (ElementType_ *type);
	bool	  removeText ();

	bool	  addTimeouts (bool switchIt);

	bool	  switchTimeout ();
	bool	  redrawTimeout ();

	bool      displayTextInfo (CompOption::Value::Vector cType);

	bool      nextElement (CompAction         *action,
			       CompAction::State  state,
			       CompOption::Vector &options);

	bool      prevElement (CompAction         *action,
			       CompAction::State  state,
			       CompOption::Vector &options);

	bool      toggleSelected (CompAction         *action,
				  CompAction::State  state,
				  CompOption::Vector &options);

	bool      toggleElementName (CompAction         *action,
				     CompAction::State  state,
				     CompOption::Vector &options);

	bool      isAnimationActive (const CompString &type);
	void      startAnimation (const CompString &type);
	void      stopAnimation (const CompString &type);
	void      setAnimationRunning (const CompString &type, bool running);

	void	  render (const GLMatrix &);
	void      setupDisplayList ();
	void      updateElementTextures ();
	void	  optionChanged (CompOption*, ElementsOptions::Options);

	ElementsTextSurface *text;

	int listIter;
	int animIter;
	
	unsigned int multiOptionsListSize;

	GLuint displayList;
	bool   needUpdate;

	std::list<ElementAnimation> animations;
	std::list<ElementType_ *>  types;
};

class ElementsWindow :
    public PluginClassHandler<ElementsWindow, CompWindow>,
    public GLWindowInterface
{
	public:

	ElementsWindow (CompWindow *);

	CompWindow *window;

	GLWindow *gWindow;
	CompositeWindow *cWindow;

	bool
	glDraw (const GLMatrix &, const GLWindowPaintAttrib &,
		const CompRegion &, unsigned int);
};

class ElementsPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ElementScreen, ElementsWindow>
{
    public:

	bool init ();
	void fini ();
};

#define ELEMENTS_WINDOW(window)						      \
    ElementsWindow *ew = ElementsWindow::get (window)
