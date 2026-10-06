#ifndef _SNOWGLOBE_INTERNAL_H
#define _SNOWGLOBE_INTERNAL_H

#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <cstring>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>

#define PI        M_PI
#define PIdiv2    M_PI_2
#define toDegrees (180.0f * M_1_PI)
#define toRadians (M_PI / 180.0f)
#define LRAND() ((long) (random () & 0x7fffffff))
#define NRAND(n) ((int) (LRAND () % (n)))
float randf (float x);

typedef struct _snowflakeRec
{
    float x, y, z;
    float theta, psi;
    float dpsi, dtheta;
    float speed, size;
} snowflakeRec;

typedef struct _Vertex
{
    float v[3];
    float n[3];
} Vertex;

typedef struct _Water
{
    int      size;
    float    distance;
    int      sDiv;

    float  bh;
    float  wa;
    float  swa;
    float  wf;
    float  swf;

    Vertex       *vertices;
    unsigned int *indices;

    unsigned int nVertices;
    unsigned int nIndices;

    unsigned int nSVer;
    unsigned int nSIdx;
    unsigned int nWVer;
    unsigned int nWIdx;

    float    wave1;
    float    wave2;
} Water;

class SnowglobeScreen;
#include "snowglobe.h"

#define SNOWGLOBE_SCREEN(s) SnowglobeScreen *as = SnowglobeScreen::get (s)

void updateWater (CompScreen *s, float time);
void updateGround (CompScreen *s, float time);
void updateHeight (Water *w);
void freeWater (Water *w);
void drawWater (Water *w, Bool full, Bool wire);
void drawGround (Water *w, Water *g);
void drawBottomGround (int size, float distance, float bottom);
Bool isInside (CompScreen *s, float x, float y, float z);
float getHeight (Water *w, float x, float z);
void SnowflakeTransform (snowflakeRec *);
void newSnowflakePosition (SnowglobeScreen *, int);
void SnowflakeDrift (CompScreen *, int);
void initializeWorldVariables (CompScreen *);
void updateSpeedFactor (float);
void RenderWater (int, float, Bool, Bool);
void DrawSnowflake (int);
void initDrawSnowflake (void);
void finDrawSnowflake (void);
void DrawSnowman (int);

#endif
