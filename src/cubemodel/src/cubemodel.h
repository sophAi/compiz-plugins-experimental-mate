#ifndef _COMPIZ_CUBEMODEL_H
#define _COMPIZ_CUBEMODEL_H

#include "cubemodel-internal.h"
#include "cubemodel_options.h"

class CubemodelScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler <CubemodelScreen, CompScreen>,
    public CubemodelOptions
{
    public:
	CubemodelScreen (CompScreen *s);
	~CubemodelScreen ();

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

	void initCubemodel ();
	void freeCubemodel ();
	void updateCubemodel ();
	void initializeWorldVariables ();
	void setLightPosition (GLenum light);
	void cubemodelInitLightPosition ();
	void updateModel (int start, int end);
	void updateModelAttributes (int i);

	bool damage;

	int   hsize;
	float sideDistance;
	float topDistance;
	float radius;
	float arcAngle;
	float ratio;

	GLuint objDisplayList;

	CubemodelObject **models;
	char            **modelFilename;
	int             numModels;
};

class CubemodelPluginVTable :
    public CompPlugin::VTableForScreen<CubemodelScreen>
{
    public:
	bool init ();
};

#endif
