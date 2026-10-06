#ifndef _COMPIZ_PHOTO_SRC_PHOTO_H
#define _COMPIZ_PHOTO_SRC_PHOTO_H

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <vector>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>
#include "photo_options.h"

struct PhotoTextureItem
{
    GLTexture::List textures;
    unsigned int width;
    unsigned int height;
    GLuint dList;
};

class PhotoScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler <PhotoScreen, CompScreen>,
    public PhotoOptions
{
    public:
	PhotoScreen (CompScreen *s);
	~PhotoScreen ();

	CompScreen      *screen;
	CompositeScreen *cScreen;
	GLScreen        *gScreen;
	CubeScreen      *csScreen;

	void cubeClearTargetOutput (float xRotate, float vRotate);
	void cubePaintInside (const GLScreenPaintAttrib &sAttrib,
			      const GLMatrix            &transform,
			      CompOutput                *output,
			      int                       size,
			      const GLVector            &normal);

	void preparePaint (int ms);
	void donePaint ();

	void photoTextureChange ();
	void photoParamChange ();
	void photoCapChange ();

    private:
	bool damage;
	bool warningPrinted;
	float angle;
	float rspeed;
	float h;
	float l;
	float xpos, ypos, zpos;
	bool transformOrder;
	bool enablecull;
	int n;
	GLuint dList;

	std::vector<PhotoTextureItem> photoTextures;
};

class PhotoPluginVTable :
    public CompPlugin::VTableForScreen<PhotoScreen>
{
    public:
	bool init ();
};

#endif
