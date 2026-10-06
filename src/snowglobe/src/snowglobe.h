#ifndef _COMPIZ_SNOWGLOBE_H
#define _COMPIZ_SNOWGLOBE_H

#include "snowglobe-internal.h"
#include "snowglobe_options.h"

class SnowglobeScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler <SnowglobeScreen, CompScreen>,
    public SnowglobeOptions
{
    public:
	SnowglobeScreen (CompScreen *s);
	~SnowglobeScreen ();

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

	void initSnowglobe ();
	void freeSnowglobe ();
	void updateSnowglobe ();
	void initializeWorldVariables ();

	bool damage;
	int numSnowflakes;
	snowflakeRec *snow;
	Water *water;
	Water *ground;
	float xRotate;
	float vRotate;
	float waterHeight;
	int hsize;
	float distance;
	float radius;
	float arcAngle;
	float speedFactor;
	GLuint snowflakeDisplayList;
};

class SnowglobePluginVTable :
    public CompPlugin::VTableForScreen<SnowglobeScreen>
{
    public:
	bool init ();
};

#endif
