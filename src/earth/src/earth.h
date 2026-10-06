#ifndef _COMPIZ_EARTH_H
#define _COMPIZ_EARTH_H

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include <curl/curl.h>
#include <pthread.h>
#include <sys/stat.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>

#include "earth_options.h"

#define DAY	0
#define NIGHT	1
#define CLOUDS	2
#define SKY	3

#define EARTH	0
#define SUN	1

typedef struct _LightParam
{
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat position[4];
    GLfloat emission[4];
    GLfloat shininess;
} LightParam;

typedef struct _cloudsFile
{
    char* filename;
    FILE* stream;
} CloudsFile;

class EarthScreen;

typedef struct _cloudsthreaddata
{
    EarthScreen* es;
    pthread_t tid;
    int started;
    int finished;
    char *url;
} CloudsThreadData;

class EarthScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler <EarthScreen, CompScreen>,
    public EarthOptions
{
    public:
	EarthScreen (CompScreen *s);
	~EarthScreen ();

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

	void initEarth ();
	void freeEarth ();
	void createLists ();
	void createShaders ();
	void deleteShaders ();
	void loadTextures ();
	void optionChanged ();

	bool damage;

	/* Config parameters */
	float lon, lat;
	float tz;
	bool  shaders;
	bool  clouds;
	float rotspeed;
	bool  clouds_url_changed;
	float earth_size;

	int previousoutput;

	/* Sun position */
	float dec, gha;

	/* Animation */
	float rotation;

	/* Threads */
	CloudsThreadData cloudsthreaddata;

	/* Clouds */
	CURL* curlhandle;
	CloudsFile cloudsfile;

	/* Textures */
	GLTexture::List texList[4];
	GLTexture* tex[4];

	/* Rendering */
	LightParam light [3];
	GLuint list [4];

	/* Shaders */
	GLboolean shadersupport;
	GLuint vert;
	GLuint frag;
	GLuint prog;
};

class EarthPluginVTable :
    public CompPlugin::VTableForScreen<EarthScreen>
{
    public:
	bool init ();
};

#endif
