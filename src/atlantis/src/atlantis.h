#ifndef _COMPIZ_ATLANTIS_H
#define _COMPIZ_ATLANTIS_H

#include "atlantis-internal.h"
#include "atlantis_options.h"

class AtlantisScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler <AtlantisScreen, CompScreen>,
    public AtlantisOptions
{
    public:
	AtlantisScreen (CompScreen *s);
	~AtlantisScreen ();

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

	void initAtlantis ();
	void freeAtlantis ();
	void updateAtlantis ();
	void initializeWorldVariables ();
	void loadModels ();
	void freeModels ();
	void setLightPosition (GLenum light);
	void atlantisInitLightPosition ();

	bool damage;

	int numFish;
	int numCrabs;
	int numCorals;
	int numAerators;

	fishRec *fish;
	crabRec *crab;
	coralRec *coral;
	aeratorRec *aerator;

	Water *water;
	Water *ground;

	float waterHeight;

	int hsize;
	float sideDistance;
	float topDistance;
	float radius;
	float arcAngle;
	float ratio;

	float speedFactor;

	float oldProgress;

	GLuint crabDisplayList;
	GLuint coralDisplayList;
	GLuint coral2DisplayList;
	GLuint bubbleDisplayList;
};

class AtlantisPluginVTable :
    public CompPlugin::VTableForScreen<AtlantisScreen>
{
    public:
	bool init ();
};

#endif
