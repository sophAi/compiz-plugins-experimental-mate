#ifndef _CUBEMODEL_INTERNAL_H
#define _CUBEMODEL_INTERNAL_H

#include <cmath>
#include <cfloat>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <pthread.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>

#define LRAND()   ((long) (random () & 0x7fffffff))
#define NRAND(n)  ((int) (LRAND () % (n)))
#define MAXRAND   (2147483648.0)

#define PI        M_PI
#define PIdiv2    M_PI_2
#define toDegrees (180.0f * M_1_PI)
#define toRadians (M_PI / 180.0f)

#define randf(x) ((float) (rand () / (((double) RAND_MAX + 1) / (x))))

typedef struct _vect3d
{
    float r[3];
} vect3d;

typedef struct _vect2d
{
    float r[2];
} vect2d;

typedef struct _groupIndices
{
    int polyCount;
    int complexity;

    int startV;
    int numV;
    int startT;
    int numT;
    int startN;
    int numN;

    int materialIndex;

    Bool texture;
    Bool normal;
} groupIndices;

typedef struct _mtlStruct
{
    char *name;

    GLfloat Ka[4];
    GLfloat Kd[4];
    GLfloat Ks[4];

    GLfloat Ns[1];
    GLfloat Ni[1];

    int illum;

    unsigned height, width;

    int map_Ka;
    int map_Kd;
    int map_Ks;
    int map_d;

    int map_params;
} mtlStruct;

typedef struct _CompTexture
{
    GLuint name;
    GLenum target;
    GLenum filter;
    GLTexture::Matrix matrix;
    GLTexture *glTexture;
    GLTexture::List glList;
} CompTexture;

typedef GLTexture::Matrix CompMatrix;
#define COMP_TEXTURE_FILTER_GOOD 0

#undef COMP_TEX_COORD_X
#undef COMP_TEX_COORD_Y
#define COMP_TEX_COORD_X(m, vx) ((m)->xx * (vx) + (m)->x0)
#define COMP_TEX_COORD_Y(m, vy) ((m)->yy * (vy) + (m)->y0)
#define cubemodelGetConcurrentLoad(s) (CubemodelScreen::get (s)->optionGetConcurrentLoad ())

typedef struct _CubemodelObject
{
    pthread_t thread;
    Bool      threadRunning;
    Bool      finishedLoading;
    Bool      updateAttributes;
    char *filename;
    char *post;

    int size;
    int lenBaseFilename;
    int startFileNum;
    int maxNumZeros;

    GLuint dList;
    Bool   compiledDList;

    float  rotate[4], translate[3], scale[3];
    float  rotateSpeed, scaleGlobal;
    float  color[4];

    int   fileCounter;
    Bool  animation;
    int   fps;
    float time;

    vect3d **reorderedVertex;
    vect2d **reorderedTexture;
    vect3d **reorderedNormal;

    unsigned int *indices;
    groupIndices *group;

    vect3d *reorderedVertexBuffer;
    vect2d *reorderedTextureBuffer;
    vect3d *reorderedNormalBuffer;

    int nVertex;
    int nTexture;
    int nNormal;
    int nGroups;
    int nIndices;
    int nUniqueIndices;
    int *nMaterial;

    mtlStruct    **material;
    CompTexture  *tex;
    char         **texName;
    unsigned int *texWidth;
    unsigned int *texHeight;

    int nTex;
} CubemodelObject;

typedef struct _fileParser
{
    FILE *fp;
    char *oldStrline;
    char *buf;
    int  bufferSize;
    int  cp;
    Bool lastTokenOnLine;
    int  tokenCount;
} fileParser;

class CubemodelScreen;
#include "cubemodel.h"

#define CUBEMODEL_SCREEN(s) CubemodelScreen *cms = CubemodelScreen::get (s)
#define CUBE_SCREEN(s) CubeScreen *cs = CubeScreen::get (s)

static inline void initTexture (CompScreen *s, CompTexture *t)
{
    t->name = 0;
    t->target = GL_TEXTURE_2D;
    t->filter = GL_LINEAR;
    t->glTexture = NULL;
    t->matrix.xx = 1.0f;
    t->matrix.yy = 1.0f;
    t->matrix.xy = 0.0f;
    t->matrix.yx = 0.0f;
    t->matrix.x0 = 0.0f;
    t->matrix.y0 = 0.0f;
}

static inline void finiTexture (CompScreen *s, CompTexture *t)
{
    t->glList.clear ();
    t->glTexture = NULL;
    t->name = 0;
}

static inline void enableTexture (CompScreen *s, CompTexture *t, int filter)
{
    if (t->glTexture)
	t->glTexture->enable (GLTexture::Good);
    else if (t->name)
    {
	glEnable (t->target);
	glBindTexture (t->target, t->name);
    }
}

static inline void disableTexture (CompScreen *s, CompTexture *t)
{
    if (t->glTexture)
	t->glTexture->disable ();
    else if (t->name)
	glDisable (t->target);
}

static inline Bool readImageToTexture (CompScreen *s, CompTexture *t, const char *file, unsigned int *width, unsigned int *height)
{
    CompSize size (0, 0);
    CompString imgFile = file ? file : "";
    CompString pluginName = "cubemodel";
    t->glList = GLTexture::readImageToTexture (imgFile, pluginName, size);
    if (!t->glList.empty ())
    {
	t->glTexture = t->glList.front ();
	t->name = t->glTexture->name ();
	t->target = t->glTexture->target ();
	t->matrix = t->glTexture->matrix ();
	if (width)
	    *width = size.width ();
	if (height)
	    *height = size.height ();
	return TRUE;
    }
    return FALSE;
}

Bool
cubemodelDrawModelObject (CompScreen      *s,
			  CubemodelObject *data,
			  float           scale);

Bool
cubemodelDeleteModelObject (CompScreen      *s,
			    CubemodelObject *data);

Bool
cubemodelModifyModelObject (CubemodelObject *data,
			    CompOption      *option,
			    int             nOption);

Bool
cubemodelUpdateModelObject (CompScreen      *s,
			    CubemodelObject *data,
			    float           time);

Bool
cubemodelAddModelObject (CompScreen      *s,
			 CubemodelObject *modelData,
			 char            *file,
			 float           *translate,
			 float           *rotate,
			 float           rotateSpeed,
			 float           *scale,
			 float           *color,
			 Bool            animation,
			 float           fps);

Bool
cubemodelDrawVBOModel (CompScreen      *s,
		       CubemodelObject *data,
		       float           *vertex,
		       float           *normal);

fileParser *
initFileParser (FILE *fp,
                int bufferSize);

void
updateFileParser (fileParser *fParser,
                  FILE *fp);

void
freeFileParser (fileParser *);

char *
getLine (fileParser *);

char *
getLineToken (fileParser *);

char *
getLineToken2 (fileParser *, Bool);

void
skipLine (fileParser *);

char *
strsep2 (char **strPtr, const char *delim);

#endif
