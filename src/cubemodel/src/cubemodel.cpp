/*
 * Compiz cube model plugin
 *
 * cubemodel.cpp - Ported to Compiz 0.9 C++ Architecture
 */

#include "cubemodel.h"
#include <algorithm>

COMPIZ_PLUGIN_20090315 (cubemodel, CubemodelPluginVTable);

void
CubemodelScreen::initializeWorldVariables ()
{
    hsize = screen->vpSize ().width () * csScreen->nOutput ();

    arcAngle = 360.0f / hsize;
    radius = csScreen->distance () / cosf (0.5f * (arcAngle * toRadians));
    topDistance = csScreen->distance ();

    if (optionGetRescaleWidth ())
	ratio = (float) screen->width () / (float) screen->height ();
    else
	ratio = 1.0f;

    sideDistance = topDistance * ratio;
}

void
CubemodelScreen::updateModel (int start, int end)
{
    CompOption::Value::Vector & modelScale         = optionGetModelScaleFactor ();
    CompOption::Value::Vector & modelX             = optionGetModelXOffset ();
    CompOption::Value::Vector & modelY             = optionGetModelYOffset ();
    CompOption::Value::Vector & modelZ             = optionGetModelZOffset ();
    CompOption::Value::Vector & modelRotationPlane = optionGetModelRotationPlane ();
    CompOption::Value::Vector & modelRotationRate  = optionGetModelRotationRate ();
    CompOption::Value::Vector & modelAnimation     = optionGetModelAnimation ();
    CompOption::Value::Vector & modelFps           = optionGetModelFps ();

    start = std::max (start, 0);
    end   = std::min (end, numModels);

    for (int i = start; i < end; i++)
    {
	if (!models[i] || !models[i]->finishedLoading)
	    continue;

	if ((int)modelScale.size () > i)
	    models[i]->scaleGlobal = modelScale[i].f ();

	if ((int)modelX.size () > i)
	    models[i]->translate[0] = modelX[i].f () * ratio;
	if ((int)modelY.size () > i)
	    models[i]->translate[1] = modelY[i].f ();
	if ((int)modelZ.size () > i)
	    models[i]->translate[2] = modelZ[i].f () * ratio;

	if ((int)modelRotationPlane.size () > i)
	{
	    int rot = modelRotationPlane[i].i ();

	    switch (rot % 3) {
	    case 0:
		models[i]->rotate[1] = 0;
		models[i]->rotate[2] = 1;
		models[i]->rotate[3] = 0;
		break;
	    case 1:
		models[i]->rotate[1] = 1;
		models[i]->rotate[2] = 0;
		models[i]->rotate[3] = 0;
		break;
	    case 2:
		models[i]->rotate[1] = 0;
		models[i]->rotate[2] = 0;
		models[i]->rotate[3] = 1;
		break;
	    }

	    if (rot / 3 != 0)
	    {
		models[i]->rotate[1] *= -1;
		models[i]->rotate[2] *= -1;
		models[i]->rotate[3] *= -1;
	    }
	}

	if ((int)modelRotationRate.size () > i)
	    models[i]->rotateSpeed = modelRotationRate[i].f ();

	if ((int)modelFps.size () > i)
	{
	    models[i]->fps = modelFps[i].i ();

	    if ((int)modelAnimation.size () > i && modelAnimation[i].i () == 2)
		models[i]->fps *= -1;
	}
    }
}

void
CubemodelScreen::initCubemodel ()
{
    int i;
    float translate[] = { 0, 0, 0 };
    float rotate[]    = { 0, 0, 0, 0 };
    float scale[]     = { 1, 1, 1, 1 };
    float color[]     = { 1, 1, 1, 1 };
    float rotateSpeed = 0;
    Bool  status, animation = FALSE;
    int   fps = 3;

    CompOption::Value::Vector & fnVec              = optionGetModelFilename ();
    CompOption::Value::Vector & modelScale         = optionGetModelScaleFactor ();
    CompOption::Value::Vector & modelX             = optionGetModelXOffset ();
    CompOption::Value::Vector & modelY             = optionGetModelYOffset ();
    CompOption::Value::Vector & modelZ             = optionGetModelZOffset ();
    CompOption::Value::Vector & modelRotationPlane = optionGetModelRotationPlane ();
    CompOption::Value::Vector & modelRotationRate  = optionGetModelRotationRate ();
    CompOption::Value::Vector & modelAnimation     = optionGetModelAnimation ();
    CompOption::Value::Vector & modelFps           = optionGetModelFps ();

    size_t count = fnVec.size ();
    count = std::min (count, modelScale.size ());
    count = std::min (count, modelX.size ());
    count = std::min (count, modelY.size ());
    count = std::min (count, modelZ.size ());
    count = std::min (count, modelRotationPlane.size ());
    count = std::min (count, modelRotationRate.size ());
    count = std::min (count, modelAnimation.size ());
    count = std::min (count, modelFps.size ());

    numModels = count;
    modelFilename = (char **) malloc (numModels * sizeof (char *));
    models = (CubemodelObject **) malloc (numModels * sizeof (CubemodelObject *));

    for (i = 0; i < numModels; i++)
	models[i] = NULL;

    for (i = 0; i < numModels; i++)
    {
	models[i] = (CubemodelObject *) malloc (sizeof (CubemodelObject));
	if (!models[i])
	    break;

	animation = (modelAnimation[i].i () > 0);
	status = FALSE;

	if ((int)fnVec.size () > i)
	    status = cubemodelAddModelObject (screen, models[i],
					      (char *) fnVec[i].s ().c_str (),
					      translate, rotate,
					      rotateSpeed, scale,
					      color, animation, fps);

	if (!status)
	    this->modelFilename[i] = NULL;
	else
	    this->modelFilename[i] = strdup (fnVec[i].s ().c_str ());
    }

    updateModel (0, numModels);
    initializeWorldVariables ();
}

void
CubemodelScreen::freeCubemodel ()
{
    if (models)
    {
	for (int i = 0; i < numModels; i++)
	{
	    if (models[i])
	    {
		cubemodelDeleteModelObject (screen, models[i]);
		free (models[i]);
	    }
	}
	free (models);
	models = NULL;
    }

    if (modelFilename)
    {
	for (int i = 0; i < numModels; i++)
	{
	    if (modelFilename[i])
		free (modelFilename[i]);
	}
	free (modelFilename);
	modelFilename = NULL;
    }
    numModels = 0;
}

void
CubemodelScreen::updateCubemodel ()
{
    freeCubemodel ();
    initCubemodel ();
}

void
CubemodelScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{
    csScreen->cubeClearTargetOutput (xRotate, vRotate);
    glClear (GL_DEPTH_BUFFER_BIT);
}

void
CubemodelScreen::setLightPosition (GLenum light)
{
    float angle = optionGetLightInclination () * toRadians;
    float position[] = { 0.0f, 0.0f, 1.0f, 0.0f };

    if (optionGetRotateLighting ())
	angle = 0;

    position[1] = sinf (angle);
    position[2] = cosf (angle);

    glLightfv (light, GL_POSITION, position);
}

void
CubemodelScreen::cubemodelInitLightPosition ()
{
    glPushMatrix ();
    glLoadIdentity ();
    setLightPosition (GL_LIGHT1);
    glPopMatrix ();
}

void
CubemodelScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
				  const GLMatrix            &transform,
				  CompOutput                *output,
				  int                       size,
				  const GLVector            &normal)
{
    int i;
    static const float matShininess[] = { 60.0f };
    static const float matSpecular[]  = { 0.6f, 0.6f, 0.6f, 1.0f };
    static const float matDiffuse[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float matAmbient[]   = { 0.8f, 0.8f, 0.9f, 1.0f };

    static const float lmodelLocalviewer[] = { 0.0f };
    static       float lmodelTwoside[]     = { 0.0f };
    static       float lmodelAmbient[]     = { 0.4f, 0.4f, 0.4f, 0.4f };
    static       float lmodelDiffuse[]     = { 1.0f, 1.0f, 1.0f, 1.0f };
    static       float lmodelSpecular[]    = { 0.6f, 0.6f, 0.6f, 0.6f };

    GLScreenPaintAttrib sA = sAttrib;
    GLMatrix            mT = transform;
    bool                enabledCull;
    int                 cull;
    float               scale, outputRatio = 1.0f;

    if (hsize != (int)(screen->vpSize ().width () * csScreen->nOutput ()))
    {
	initializeWorldVariables ();
	updateModel (0, numModels);
    }

    sA.yRotate += csScreen->invert () * (360.0f / size) *
		  (csScreen->xRotations () - (screen->vp ().x () * csScreen->nOutput ()));

    gScreen->glApplyTransform (sA, output, &mT);

    glPushMatrix ();

    if (optionGetRotateLighting ())
	setLightPosition (GL_LIGHT1);

    glLoadMatrixf (mT.getMatrix ());

    if (!optionGetRotateLighting ())
	setLightPosition (GL_LIGHT1);

    glTranslatef (csScreen->outputXOffset (), -csScreen->outputYOffset (), 0.0f);
    glScalef (csScreen->outputXScale (), csScreen->outputYScale (), 1.0f);

    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT |
		  GL_LIGHTING_BIT     | GL_DEPTH_BUFFER_BIT);

    glEnable (GL_BLEND);
    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    lmodelAmbient[0]  = optionGetLightAmbient ();
    lmodelDiffuse[0]  = optionGetLightDiffuse ();
    lmodelSpecular[0] = optionGetLightSpecular ();

    for (i = 1; i < 4; i++)
    {
	lmodelAmbient[i]  = lmodelAmbient[0];
	lmodelDiffuse[i]  = lmodelDiffuse[0];
	lmodelSpecular[i] = lmodelSpecular[0];
    }

    lmodelTwoside[0] = (optionGetRenderFrontAndBack () ? 1.0f : 0.0f);

    glLightModelfv (GL_LIGHT_MODEL_LOCAL_VIEWER, lmodelLocalviewer);
    glLightModelfv (GL_LIGHT_MODEL_TWO_SIDE, lmodelTwoside);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT,  lmodelAmbient);
    glLightfv (GL_LIGHT1, GL_DIFFUSE,   lmodelDiffuse);
    glLightfv (GL_LIGHT1, GL_SPECULAR,  lmodelSpecular);

    enabledCull = glIsEnabled (GL_CULL_FACE);

    glGetIntegerv (GL_CULL_FACE_MODE, &cull);
    glEnable (GL_CULL_FACE);

    glCullFace (~cull & (GL_FRONT | GL_BACK));
    glCullFace (cull);

    glPushMatrix ();

    glColor4usv (defaultColor);

    glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, matShininess);
    glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);

    glEnable (GL_NORMALIZE);
    glEnable (GL_DEPTH_TEST);
    glEnable (GL_COLOR_MATERIAL);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT1);
    glDisable (GL_LIGHT0);

    glDepthFunc (GL_LEQUAL);
    glShadeModel (GL_SMOOTH);

    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    scale = csScreen->distance ();

    if (optionGetRescaleWidth ())
    {
	outputRatio = (float) output->width () / (float) output->height ();
    }

    glScalef (scale / outputRatio, scale, scale / outputRatio);

    glPushMatrix ();

    glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

    for (i = 0; i < numModels; i++)
    {
	glPushMatrix ();
	cubemodelDrawModelObject (screen, models[i],
	                          optionGetGlobalModelScaleFactor ());
	glPopMatrix ();
    }
    glPopMatrix ();

    glPopMatrix ();

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
CubemodelScreen::preparePaint (int ms)
{
    for (int i = 0; i < numModels; i++)
    {
	if (!models[i] || !models[i]->finishedLoading)
	    continue;

	if (models[i]->updateAttributes)
	{
	    updateModel (i, i + 1);
	    models[i]->updateAttributes = FALSE;
	}

	cubemodelUpdateModelObject (screen, models[i], ms / 1000.0f);
    }

    cScreen->preparePaint (ms);
}

void
CubemodelScreen::donePaint ()
{
    if (damage)
    {
	cScreen->damageScreen ();
	damage = false;
    }
    cScreen->donePaint ();
}

CubemodelScreen::CubemodelScreen (CompScreen *screen) :
    PluginClassHandler <CubemodelScreen, CompScreen> (screen),
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    csScreen (CubeScreen::get (screen)),
    damage (false),
    hsize (0),
    sideDistance (0.0f),
    topDistance (0.0f),
    radius (0.0f),
    arcAngle (0.0f),
    ratio (1.0f),
    objDisplayList (0),
    models (NULL),
    modelFilename (NULL),
    numModels (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (csScreen);

    initCubemodel ();

    optionSetGlobalModelScaleFactorNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelFilenameNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelScaleFactorNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelXOffsetNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelYOffsetNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelZOffsetNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelRotationPlaneNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelRotationRateNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelAnimationNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetModelFpsNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetRescaleWidthNotify (boost::bind (&CubemodelScreen::updateCubemodel, this));
    optionSetRotateLightingNotify (boost::bind (&CubemodelScreen::cubemodelInitLightPosition, this));
    optionSetLightInclinationNotify (boost::bind (&CubemodelScreen::cubemodelInitLightPosition, this));
}

CubemodelScreen::~CubemodelScreen ()
{
    freeCubemodel ();
}

bool
CubemodelPluginVTable::init ()
{
    if (CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) &&
	CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) &&
	CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) &&
	CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	return true;

    return false;
}
