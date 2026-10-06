/**
 * Compiz elements plugin
 * elements.h
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
#ifndef _COMPIZ_ELEMENTS_H
#define _COMPIZ_ELEMENTS_H

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <opengl/opengl.h>
#include <composite/composite.h>
#include <text/text.h>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>

#define COMPIZ_ELEMENTS_ABI     1

class ElementType;
class Element;
class ElementAnimation;
class ElementTexture;

typedef boost::function<Element * ()> elementAddFunc;

class PrivateElementScreen;
class PrivateElementAnimation;
class ElementsTextSurface;

class ElementTexture
{
    public:

	ElementTexture (CompString, int, CompString);
	~ElementTexture ();

	class List : public std::vector <ElementTexture *> {

	    public:
		List ();
		List (unsigned int);
		List (CompString		name,
		      CompOption::Value::Vector *paths,
		      CompOption::Value::Vector *iters,
		      int		        &size,
		      int		        &iter);
		~List ();

		List & operator= (const ElementTexture::List);

		void setTextures (CompString name,
				  CompOption::Value::Vector *paths,
				  CompOption::Value::Vector *iters,
				  int			    &size,
				  int			    &iter);

		int   nTexture;
	};

	bool valid;

	void
	setTexture (CompString path, int texSize, CompString name);

	CompSize size ();

	void draw ();

    private:

	GLTexture::List textures;
	unsigned int width, height;
	std::vector<GLuint>	dLists;
    friend class ElementAnimation;
};

class Element
{
    public:

	Element ();

	ElementAnimation *anim;

	float x, y, z;
	float dx, dy, dz;
	float rSpeed;
	int   rDirection;
	int   rAngle;

	float opacity;
	float glowAlpha;	/* Needs to be painted */
	
	template <class Archive>
	void serialize (Archive &ar, const unsigned int version)
	{
	    fprintf (stderr, "saving an element\n");
	    ar & x;
	    ar & y;
	    ar & z;
	    ar & dx;
	    ar & dy;
	    ar & dz;
	    ar & rSpeed;
	    ar & rDirection;
	    ar & rAngle;
	    ar & opacity;
	    ar & glowAlpha;
	}

	virtual bool init ();
	virtual void move ();
	virtual void fini ();
	virtual ~Element () {}
    private:

	void defaultInit ();

	void regenerateOffscreen ();

    friend class ElementAnimation;
    friend class PrivateElementScreen;
};

class ElementType
{
    public:
    
	class KillTimer:
	    public CompTimer
	{
	    private:
		KillTimer ();
		static void registerKillType (ElementType &);
	    public:
		bool killTimeout (CompString elementName);
	
	    friend class PrivateElementScreen;
	    friend class ElementType;
	};

	void
	setName (CompString, CompString);

	void
	setCallbacks (elementAddFunc);

	CompString
	name ();

	CompString
	desc ();

	static ElementType *
	find (CompString);

	static ElementType *
	create (CompString, CompString, elementAddFunc);

	static ElementType *
	create ();

	void
	destroy ();

	elementAddFunc addFunc;

    private:
	ElementType ();
	ElementType (CompString, CompString, elementAddFunc);
	~ElementType ();

	CompString mName;
	CompString mDesc;

    friend class ElementAnimation;
    friend class ElementTextSurface;
    friend class boost::serialization::access;
};

class ElementAnimation
{
    public:

	~ElementAnimation ();
	ElementAnimation () {};
	
	template <class Archive>
	void serialize (Archive &ar, const unsigned int version)
	{
	    ar & priv;
	}

	void setNElement (int n);
	void setSize (int s);
	void setSpeed (int s);
	void setId (int i);
	bool setType (CompString);
	void setRotate (bool r);

	int nElement ();
	int size ();
	int speed ();
	int id ();
	ElementType * type ();
	bool rotate ();
	bool valid ();

	int nTexture ();
	void setNTexture (int);

	ElementTexture::List textures ();
	
	const boost::ptr_vector<Element> & elements ();

	bool start ();
	void stop ();

	bool active ();

	static ElementAnimation &
	create (CompString type,
		int	   nElement,
		int	   size,
		int	   speed,
		int	   iter,
		bool	   rotate);
    private:
	ElementAnimation (CompString type,
			  int nElement,
			  int size,
			  int speed,
			  int iter,
			  bool rotate);

	PrivateElementAnimation *priv;

    friend class PrivateElementScreen;	
    friend class ElementType;
    friend class ElementType::KillTimer;
    friend class boost::serialization::access;
};

class ElementScreen :
    public PluginClassHandler<ElementScreen, CompScreen>,
    public CompOption::Class
{
    public:

	ElementScreen (CompScreen *screen);
	~ElementScreen ();

	CompOption::Vector &
	getOptions ();

	bool
	setOption (const CompString &name, CompOption::Value &value);

	int getRand (int, int);
	float mmRand (int, int, float);

	int boxing ();
	int depth ();
	int updateDelay ();

	bool isAnimationActive (const CompString &type);
	void startAnimation (const CompString &type);
	void stopAnimation (const CompString &type);
	void setAnimationRunning (const CompString &type, bool running);

    private:

	PrivateElementScreen *priv;

    friend class ElementsWindow;
    friend class ElementType;
    friend class ElementType::KillTimer;
    friend class ElementAnimation;
    friend class ElementsTextSurface;
    friend class Element;
};

#define ELEMENTS_SCREEN(s)						       \
    ElementScreen *es = ElementScreen::get (s)
#endif
