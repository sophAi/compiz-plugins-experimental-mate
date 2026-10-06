#include "snowglobe.h"

COMPIZ_PLUGIN_20090315 (snowglobe, SnowglobePluginVTable);

void
SnowglobeScreen::initializeWorldVariables ()
{
    speedFactor = optionGetSpeedFactor ();
    hsize = screen->vpSize ().width () * csScreen->nOutput ();
    arcAngle = 360.0f / hsize;
    radius = csScreen->distance () / sinf (0.5 * (PI - arcAngle * toRadians));
    distance = csScreen->distance ();
}

void
SnowglobeScreen::initSnowglobe ()
{
    water = NULL;
    ground = NULL;
    numSnowflakes = optionGetNumSnowflakes ();
    snow = (snowflakeRec *) calloc (numSnowflakes, sizeof (snowflakeRec));

    initializeWorldVariables ();

    for (int i = 0; i < numSnowflakes; i++)
    {
	snow[i].size = optionGetSnowflakeSize () + sqrt (randf (optionGetSnowflakeSize ()));
	newSnowflakePosition (this, i);
	snow[i].psi = randf (2 * PI);
	snow[i].theta = randf (PI);
	snow[i].dpsi = randf (5);
	snow[i].dtheta = randf (5);
	snow[i].speed = randf (0.4) + 0.2;
    }

    waterHeight = 50000;
    snowflakeDisplayList = glGenLists (1);
    glNewList (snowflakeDisplayList, GL_COMPILE);
    DrawSnowflake (0);
    glEndList ();
}

void
SnowglobeScreen::freeSnowglobe ()
{
    if (snow)
    {
	free (snow);
	snow = NULL;
    }
    if (snowflakeDisplayList)
    {
	glDeleteLists (snowflakeDisplayList, 1);
	snowflakeDisplayList = 0;
    }
    if (water)
    {
	freeWater (water);
	water = NULL;
    }
    if (ground)
    {
	freeWater (ground);
	ground = NULL;
    }
}

void
SnowglobeScreen::updateSnowglobe ()
{
    freeSnowglobe ();
    initSnowglobe ();
}

void
SnowglobeScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{
    csScreen->cubeClearTargetOutput (xRotate, vRotate);
    glClear (GL_DEPTH_BUFFER_BIT);
}

void
SnowglobeScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
				  const GLMatrix            &transform,
				  CompOutput                *output,
				  int                       size,
				  const GLVector            &normal)
{
    if (csScreen->invert () == -1)
	return;

    waterHeight = 50000;
    if (hsize != (int)(screen->vpSize ().width () * csScreen->nOutput ()))
	updateSnowglobe ();

    static const float mat_shininess[] = { 60.0f };
    static const float mat_specular[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
    static const float mat_diffuse[]   = { 0.46f, 0.66f, 0.795f, 1.0f };
    static const float mat_ambient[]   = { 0.1f, 0.1f, 0.3f, 1.0f };
    static const float lmodel_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float lmodel_localviewer[] = { 0.0f };

    if (optionGetShowWater ())
	updateHeight (water);

    GLScreenPaintAttrib sA = sAttrib;
    sA.yRotate += csScreen->invert () * (360.0f / size) *
		  (csScreen->xRotations () - (screen->vp ().x () * csScreen->nOutput ()));

    GLMatrix mT = transform;
    gScreen->glApplyTransform (sA, output, &mT);

    glPushMatrix ();
    glLoadMatrixf (mT.getMatrix ());
    glTranslatef (csScreen->outputXOffset (), -csScreen->outputYOffset (), 0.0f);
    glScalef (csScreen->outputXScale (), csScreen->outputYScale (), 1.0f);

    bool enabledCull = glIsEnabled (GL_CULL_FACE);

    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int cull;
    glGetIntegerv (GL_CULL_FACE_MODE, &cull);
    glEnable (GL_CULL_FACE);

    glCullFace (~cull & (GL_FRONT | GL_BACK));
    if (optionGetShowWater ())
    {
	glColor4usv (optionGetWaterColor ());
	drawWater (water, TRUE, FALSE);
    }
    glCullFace (cull);

    if (optionGetShowGround ())
    {
	glColor4f (0.8f, 0.8f, 0.8f, 1.0f);
	drawGround (NULL, ground);
    }

    glPushMatrix ();
    glColor4usv (defaultColor);

    glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv (GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_localviewer);

    glEnable (GL_NORMALIZE);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHT0);
    glEnable (GL_COLOR_MATERIAL);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    for (int i = 0; i < numSnowflakes; i++)
    {
	glPushMatrix ();
	SnowflakeTransform (&(snow[i]));
	float sc = 0.01f * snow[i].size;
	glScalef (sc, sc, sc);
	initDrawSnowflake ();
	glCallList (snowflakeDisplayList);
	finDrawSnowflake ();
	glPopMatrix ();
    }

    if (optionGetShowSnowman ())
    {
	glPushMatrix ();
	float bottom = -0.5f;
	if (optionGetShowGround ())
	    bottom = getHeight (ground, 0.0f, 0.0f);
	glTranslatef (0.0f, bottom, 0.0f);
	float sc = 0.4f * optionGetSnowmanSize () * (0.5f - bottom);
	glScalef (sc, sc, sc);
	glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	DrawSnowman (0);
	glPopMatrix ();
    }

    glPopMatrix ();

    if (optionGetShowWater ())
    {
	glEnable (GL_CULL_FACE);
	glColor4usv (optionGetWaterColor ());
	drawWater (water, optionGetShowWater (), 0);
    }

    if (optionGetShowGround ())
    {
	glColor4f (0.8f, 0.8f, 0.8f, 1.0f);
	drawBottomGround (screen->vpSize ().width () * csScreen->nOutput (), csScreen->distance (), -0.4999f);
    }

    glDisable (GL_LIGHT1);
    glDisable (GL_NORMALIZE);

    if (!gScreen->lighting ())
	glDisable (GL_LIGHTING);

    glDisable (GL_DEPTH_TEST);
    if (enabledCull)
	glEnable (GL_CULL_FACE);
    else
	glDisable (GL_CULL_FACE);

    glPopMatrix ();
    glPopAttrib ();

    damage = true;

    csScreen->cubePaintInside (sAttrib, transform, output, size, normal);
}

void
SnowglobeScreen::preparePaint (int ms)
{
    for (int i = 0; i < numSnowflakes; i++)
    {
	SnowflakeDrift (screen, i);
    }
    updateWater (screen, (float)ms / 1000.0f);
    updateGround (screen, (float)ms / 1000.0f);

    cScreen->preparePaint (ms);
}

void
SnowglobeScreen::donePaint ()
{
    if (damage)
    {
	cScreen->damageScreen ();
	damage = false;
    }
    cScreen->donePaint ();
}

SnowglobeScreen::SnowglobeScreen (CompScreen *screen) :
    PluginClassHandler <SnowglobeScreen, CompScreen> (screen),
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    csScreen (CubeScreen::get (screen)),
    damage (false),
    numSnowflakes (0),
    snow (NULL),
    water (NULL),
    ground (NULL),
    xRotate (0.0f),
    vRotate (0.0f),
    waterHeight (50000.0f),
    hsize (0),
    distance (0.0f),
    radius (0.0f),
    arcAngle (0.0f),
    speedFactor (1.0f),
    snowflakeDisplayList (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (csScreen);

    static const float ambient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };
    static const float diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float position[] = { 0.0f, 1.0f, 0.0f, 0.0f };

    glLightfv (GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv (GL_LIGHT1, GL_POSITION, position);

    initSnowglobe ();

    optionSetSpeedFactorNotify (boost::bind (&SnowglobeScreen::updateSnowglobe, this));
    optionSetNumSnowflakesNotify (boost::bind (&SnowglobeScreen::updateSnowglobe, this));
    optionSetSnowflakeSizeNotify (boost::bind (&SnowglobeScreen::updateSnowglobe, this));
}

SnowglobeScreen::~SnowglobeScreen ()
{
    freeSnowglobe ();
}

bool
SnowglobePluginVTable::init ()
{
    if (CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) &&
	CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) &&
	CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) &&
	CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	return true;

    return false;
}
