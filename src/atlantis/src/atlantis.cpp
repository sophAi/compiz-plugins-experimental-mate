/*
 * Compiz cube atlantis plugin
 *
 * atlantis.cpp - Ported to Compiz 0.9 C++ Architecture
 */

#include "atlantis.h"
#include <algorithm>

COMPIZ_PLUGIN_20090315 (atlantis, AtlantisPluginVTable);

static float calculateScreenRatio (CompScreen *s, bool rescale)
{
    if (!rescale)
	return 1.0f;

    float ratio = (float) s->width () / (float) s->height ();

    if (s->outputDevs ().size () <= 1)
	return ratio;

    float temp = 0;
    for (unsigned int i = 0; i < s->outputDevs ().size (); i++)
	temp += (float) s->outputDevs ()[i].width () / (float) s->outputDevs ()[i].height ();

    if (temp != 0)
	ratio = temp / s->outputDevs ().size ();

    return ratio;
}

void
AtlantisScreen::initializeWorldVariables ()
{
    int i = 0, i2 = 0;
    int j;
    int bi;

    coralRec * c;
    aeratorRec * a;

    CompOption::Value::Vector & cType   = optionGetPlantType ();
    CompOption::Value::Vector & cNumber = optionGetPlantNumber ();
    CompOption::Value::Vector & cSize   = optionGetPlantSize ();
    CompOption::Value::Vector & cColor  = optionGetPlantColor ();

    speedFactor = optionGetSpeedFactor ();

    hsize = screen->vpSize ().width () * csScreen->nOutput ();

    arcAngle = 360.0f / hsize;
    radius = (100000.0f - 1.0f) * csScreen->distance () /
	     cosf (0.5f * (arcAngle * toRadians));
    topDistance = (100000.0f - 1.0f) * csScreen->distance ();

    ratio = calculateScreenRatio (screen, optionGetRescaleWidth ());

    sideDistance = topDistance * ratio;

    size_t num = std::min({cType.size(), cNumber.size(), cSize.size(), cColor.size()});

    for (size_t k = 0; k < num; k++)
    {
	for (j = 0; j < cNumber[k].i (); j++)
	{
	    int size = cSize[k].i ();

	    switch (cType[k].i ()) {
	    case 0:
	    case 1:
		c = &(coral[i]);

		c->size = (randf (sqrtf(size)) + size);
		c->type = cType[k].i ();

		if (j == 0)
		    setSimilarColor4us (c->color, cColor[k].c (), 0.2, 0.2);
		else
		    setSimilarColor (c->color, coral[i - j].color, 0.2, 0.2);

		c->psi = randf (360);

		setRandomLocation (screen, &(c->x), &(c->y), 3 * size);
		c->z = -50000;
		i++;
		break;

	    case 2:
		a = &(aerator[i2]);

		a->size = randf (sqrtf (size)) + size;
		a->type = cType[k].i ();

		if (j == 0)
		    setSimilarColor4us (a->color, cColor[k].c (), 0, 0);
		else
		    setSimilarColor (a->color, aerator[i2-j].color, 0.0, 0.0);

		setRandomLocation (screen, &(a->x), &(a->y), size);
		a->z = -50000;

		for (bi = 0; bi < a->numBubbles; bi++)
		{
		    a->bubbles[bi].size = size;
		    a->bubbles[bi].x = a->x;
		    a->bubbles[bi].y = a->y;
		    a->bubbles[bi].z = a->z;
		    a->bubbles[bi].speed = 100 + randf (150);
		    a->bubbles[bi].offset = randf (2 * PI);
		    a->bubbles[bi].counter = 0;
		}

		i2++;
		break;
	    }
	}
    }
}

void
AtlantisScreen::loadModels ()
{
    crabDisplayList = glGenLists (1);
    glNewList (crabDisplayList, GL_COMPILE);
    DrawCrab (0);
    glEndList ();

    coralDisplayList = glGenLists (1);
    glNewList (coralDisplayList, GL_COMPILE);
    optionGetLowPoly () ? DrawCoralLow (0) : DrawCoral (0);
    glEndList ();

    coral2DisplayList = glGenLists (1);
    glNewList (coral2DisplayList, GL_COMPILE);
    optionGetLowPoly () ? DrawCoral2Low (0) : DrawCoral2 (0);
    glEndList ();

    bubbleDisplayList = glGenLists (1);
    glNewList (bubbleDisplayList, GL_COMPILE);
    optionGetLowPoly () ? DrawBubble (0, 6) : DrawBubble (0, 9);
    glEndList ();
}

void
AtlantisScreen::freeModels ()
{
    if (crabDisplayList)
    {
	glDeleteLists (crabDisplayList, 1);
	crabDisplayList = 0;
    }
    if (coralDisplayList)
    {
	glDeleteLists (coralDisplayList, 1);
	coralDisplayList = 0;
    }
    if (coral2DisplayList)
    {
	glDeleteLists (coral2DisplayList, 1);
	coral2DisplayList = 0;
    }
    if (bubbleDisplayList)
    {
	glDeleteLists (bubbleDisplayList, 1);
	bubbleDisplayList = 0;
    }
}

void
AtlantisScreen::initAtlantis ()
{
    int i = 0, i2 = 0;
    int j;

    CompOption::Value::Vector & cType   = optionGetCreatureType ();
    CompOption::Value::Vector & cNumber = optionGetCreatureNumber ();
    CompOption::Value::Vector & cSize   = optionGetCreatureSize ();
    CompOption::Value::Vector & cColor  = optionGetCreatureColor ();

    size_t num = std::min({cType.size(), cNumber.size(), cSize.size(), cColor.size()});

    water = NULL;
    ground = NULL;

    numFish = 0;
    numCrabs = 0;

    for (size_t k = 0; k < num; k++)
    {
	if (cSize[k].i () == 0)
	    continue;

	if (cType[k].i () != CRAB)
	    numFish += cNumber[k].i ();
	else
	    numCrabs += cNumber[k].i ();
    }

    fish = (fishRec *) calloc (numFish, sizeof(fishRec));
    crab = (crabRec *) calloc (numCrabs, sizeof(crabRec));

    if (optionGetShowWater ())
	waterHeight = optionGetWaterHeight () * 100000.0f - 50000.0f;
    else
	waterHeight = 50000.0f;

    oldProgress = 0;

    for (size_t k = 0; k < num; k++)
    {
	for (j = 0; j < cNumber[k].i (); j++)
	{
	    int size = cSize[k].i ();
	    int type = cType[k].i ();

	    if (size == 0)
		break;

	    if (type != CRAB)
	    {
		fishRec * f = &(fish[i]);

		f->type = type;

		if (type == WHALE)
		    size /= 2;
		if (type == DOLPHIN)
		    size *= 2;
		if (type == SHARK)
		    size *= 3;

		f->size = randf (sqrtf (size)) + size;
		f->speed = randf (150) + 50;

		if (j == 0)
		    setSimilarColor4us (f->color, cColor[k].c (), 0.2, 0.1);
		else
		    setSimilarColor (f->color, fish[i - j].color, 0.2, 0.1);

		f->x = randf (size);
		f->y = randf (size);
		f->z = (waterHeight - 50000) / 2 +
		       randf (size * 0.02) - size * 0.01;
		f->psi = randf (360) - 180.0;
		f->theta = randf (100) - 50;
		f->v = 1.0;

		f->group = k;

		f->boidsCounter = i % NUM_GROUPS;
		f->boidsPsi = f->psi;
		f->boidsTheta = f->theta;

		f->smoothTurnCounter = NRAND (3);
		f->smoothTurnAmount = NRAND (3) - 1;

		f->prevRandPsi = 0;
		f->prevRandTh = 0;

		i++;
	    }
	    else
	    {
		crabRec * c = &(crab[i2]);

		c->size = randf (sqrtf (size)) + size;
		c->speed = randf (100) + 50;

		if (j == 0)
		    setSimilarColor4us (c->color, cColor[k].c (), 0.2, 0.1);
		else
		    setSimilarColor (c->color, crab[i2 - j].color, 0.2, 0.1);

		c->x = randf (2 * size) - size;
		c->y = randf (2 * size) - size;

		if (optionGetStartCrabsBottom ())
		{
		    c->z = 50000;
		    c->isFalling = FALSE;
		}
		else
		{
		    c->z = (waterHeight - 50000) / 2;
		    c->isFalling = TRUE;
		}

		c->psi = randf (360);
		c->theta = 0;

		c->scuttlePsi = 0;
		c->scuttleAmount = NRAND (3) - 1;

		i2++;
	    }
	}
    }

    numCorals = 0;
    numAerators = 0;

    CompOption::Value::Vector & pType   = optionGetPlantType ();
    CompOption::Value::Vector & pNumber = optionGetPlantNumber ();
    CompOption::Value::Vector & pSize   = optionGetPlantSize ();
    CompOption::Value::Vector & pColor  = optionGetPlantColor ();

    size_t pNum = std::min({pType.size(), pNumber.size(), pSize.size(), pColor.size()});

    for (size_t k = 0; k < pNum; k++)
    {
	switch (pType[k].i ()) {
	case 0:
	case 1:
	    numCorals += pNumber[k].i ();
	    break;
	case 2:
	    numAerators += pNumber[k].i ();
	    break;
	}
    }

    coral   = (coralRec *) calloc (numCorals, sizeof(coralRec));
    aerator = (aeratorRec *) calloc (numAerators, sizeof(aeratorRec));

    for (int k = 0; k < numAerators; k++)
    {
	aerator[k].numBubbles = 20;
	aerator[k].bubbles = (Bubble *) calloc (aerator[k].numBubbles, sizeof (Bubble));
    }

    initializeWorldVariables ();

    updateWater (screen, 0);
    updateGround (screen, 0);

    loadModels ();
}

void
AtlantisScreen::freeAtlantis ()
{
    if (fish)
    {
	free (fish);
	fish = NULL;
    }
    if (crab)
    {
	free (crab);
	crab = NULL;
    }
    if (coral)
    {
	free (coral);
	coral = NULL;
    }
    if (aerator)
    {
	for (int i = 0; i < numAerators; i++)
	{
	    if (aerator[i].bubbles)
		free (aerator[i].bubbles);
	}
	free (aerator);
	aerator = NULL;
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

    freeModels ();
}

void
AtlantisScreen::updateAtlantis ()
{
    freeAtlantis ();
    initAtlantis ();
}

void
AtlantisScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{
    csScreen->cubeClearTargetOutput (xRotate, vRotate);
    glClear (GL_DEPTH_BUFFER_BIT);
}

void
AtlantisScreen::setLightPosition (GLenum light)
{
    float position[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    float angle = optionGetLightInclination () * toRadians;

    if (optionGetRotateLighting ())
	angle = 0;

    position[1] = sinf (angle);
    position[2] = cosf (angle);

    glLightfv (light, GL_POSITION, position);
}

void
AtlantisScreen::atlantisInitLightPosition ()
{
    glPushMatrix ();
    glLoadIdentity ();
    setLightPosition (GL_LIGHT1);
    glPopMatrix ();
}

void
AtlantisScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
				 const GLMatrix            &transform,
				 CompOutput                *output,
				 int                       size,
				 const GLVector            &normal)
{
    if (csScreen->invert () == -1)
	return;

    int i, j;
    float scale;

    static const float mat_shininess[] = { 60.0f };
    static const float mat_specular[]  = { 0.6f, 0.6f, 0.6f, 1.0f };
    static const float mat_diffuse[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float mat_ambient[]   = { 0.8f, 0.8f, 0.9f, 1.0f };

    static const float lmodel_localviewer[] = { 0.0f };
    static const float lmodel_twoside[]     = { 0.0f };
    static float lmodel_ambient[]           = { 0.4f, 0.4f, 0.4f, 0.4f };

    GLScreenPaintAttrib sA = sAttrib;
    GLMatrix mT = transform;

    int new_hsize = screen->vpSize ().width () * csScreen->nOutput ();
    int drawDeformation = DeformationNone;

    if (optionGetShowWater ())
	waterHeight = optionGetWaterHeight () * 100000.0f - 50000.0f;
    else
	waterHeight = 50000.0f;

    float newRatio = calculateScreenRatio (screen, optionGetRescaleWidth ());

    if (new_hsize < hsize || fabsf (newRatio - ratio) > 0.0001f)
	updateAtlantis ();
    else if (new_hsize > hsize)
    {
	initializeWorldVariables ();
    }

    if (optionGetShowWater () || optionGetShowWaterWire () || optionGetShowGround ())
    {
	updateDeformation (screen, drawDeformation);
	updateHeight (water, optionGetShowGround () ? ground : NULL,
		      optionGetWaveRipple (), drawDeformation);
    }

    sA.yRotate += csScreen->invert () * (360.0f / size) *
		  (csScreen->xRotations () - (screen->vp ().x () * csScreen->nOutput ()));

    gScreen->glApplyTransform (sA, output, &mT);

    glPushMatrix ();
    glLoadMatrixf (mT.getMatrix ());

    if (!optionGetRotateLighting ())
	setLightPosition (GL_LIGHT1);

    glTranslatef (csScreen->outputXOffset (), -csScreen->outputYOffset (), 0.0f);
    glScalef (csScreen->outputXScale (), csScreen->outputYScale (), 1.0f);

    bool enabledCull = glIsEnabled (GL_CULL_FACE);

    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);

    glEnable (GL_BLEND);
    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    for (i = 0; i < 4; i++)
	lmodel_ambient[i] = optionGetLightAmbient ();

    glLightModelfv (GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_localviewer);
    glLightModelfv (GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    if (optionGetShowWater ())
    {
	int cull;
	glGetIntegerv (GL_CULL_FACE_MODE, &cull);
	glEnable (GL_CULL_FACE);

	glCullFace (~cull & (GL_FRONT | GL_BACK));
	setWaterMaterial (optionGetWaterColor ());
	drawWater (water, TRUE, FALSE, drawDeformation);
	glCullFace (cull);
    }

    if (optionGetShowGround ())
    {
	setGroundMaterial (optionGetGroundColor ());
	if (optionGetRenderWaves () && optionGetShowWater () && !optionGetWaveRipple ())
	    drawGround (water, ground, drawDeformation);
	else
	    drawGround (NULL, ground, drawDeformation);
    }

    glPushMatrix ();

    glColor4usv (defaultColor);

    glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

    glEnable  (GL_NORMALIZE);
    glEnable  (GL_DEPTH_TEST);
    glEnable  (GL_COLOR_MATERIAL);
    glEnable  (GL_LIGHTING);
    glEnable  (GL_LIGHT1);
    glDisable (GL_LIGHT0);

    glShadeModel (GL_SMOOTH);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glScalef (0.00001f / ratio, 0.00001f, 0.00001f / ratio);

    for (i = 0; i < numCrabs; i++)
    {
	glPushMatrix ();
	CrabTransform (&(crab[i]));
	scale = crab[i].size / 6000.0f;
	glScalef (scale, scale, scale);
	glColor4fv (crab[i].color);

	initDrawCrab ();
	glCallList (crabDisplayList);
	finDrawCrab ();
	glPopMatrix ();
    }

    for (i = 0; i < numCorals; i++)
    {
	glPushMatrix ();
	glTranslatef (coral[i].y, coral[i].z, coral[i].x);
	glRotatef (-coral[i].psi, 0.0f, 1.0f, 0.0f);

	scale = coral[i].size / 6000.0f;
	glScalef (scale, scale, scale);
	glColor4fv (coral[i].color);

	switch (coral[i].type) {
	case 0:
	    initDrawCoral ();
	    glCallList (coralDisplayList);
	    finDrawCoral ();
	    break;
	case 1:
	    initDrawCoral2 ();
	    glCallList (coral2DisplayList);
	    finDrawCoral2 ();
	    break;
	}
	glPopMatrix ();
    }

    for (i = 0; i < numFish; i++)
    {
	glPushMatrix ();
	FishTransform (&(fish[i]));
	scale = fish[i].size / 6000.0f;
	glScalef (scale, scale, scale);
	glColor4fv (fish[i].color);

	switch (fish[i].type)
	{
	case SHARK:
	    DrawShark (&(fish[i]), 0);
	    break;
	case WHALE:
	    DrawWhale (&(fish[i]), 0);
	    break;
	case DOLPHIN:
	    DrawDolphin (&(fish[i]), 0);
	    break;
	case BUTTERFLYFISH:
	    initDrawBFish (fish[i].color);
	    AnimateBFish  (fish[i].htail);
	    DrawAnimatedBFish ();
	    finDrawBFish ();
	    break;
	case CHROMIS:
	    initDrawChromis (fish[i].color);
	    AnimateChromis  (fish[i].htail);
	    DrawAnimatedChromis ();
	    finDrawChromis ();
	    break;
	case CHROMIS2:
	    initDrawChromis2 (fish[i].color);
	    AnimateChromis   (fish[i].htail);
	    DrawAnimatedChromis ();
	    finDrawChromis ();
	    break;
	case CHROMIS3:
	    initDrawChromis3 (fish[i].color);
	    AnimateChromis   (fish[i].htail);
	    DrawAnimatedChromis ();
	    finDrawChromis ();
	    break;
	case FISH:
	    initDrawFish (fish[i].color);
	    AnimateFish  (fish[i].htail);
	    DrawAnimatedFish ();
	    finDrawFish ();
	    break;
	case FISH2:
	    initDrawFish2 (fish[i].color);
	    AnimateFish2  (fish[i].htail);
	    DrawAnimatedFish2 ();
	    finDrawFish2 ();
	    break;
	default:
	    break;
	}
	glPopMatrix ();
    }

    glEnable (GL_CULL_FACE);

    for (i = 0; i < numAerators; i++)
    {
	for (j = 0; j < aerator[i].numBubbles; j++)
	{
	    glPushMatrix ();
	    BubbleTransform (&(aerator[i].bubbles[j]));
	    scale = aerator[i].bubbles[j].size;

	    glScalef (scale, scale, scale);
	    glColor4fv (aerator[i].color);
	    glCallList (bubbleDisplayList);
	    glPopMatrix ();
	}
    }

    glPopMatrix ();

    if (optionGetShowWater () || optionGetShowWaterWire ())
    {
	glEnable (GL_CULL_FACE);
	setWaterMaterial (optionGetWaterColor ());
	drawWater (water, optionGetShowWater (),
		   optionGetShowWaterWire (), drawDeformation);
    }

    if (optionGetShowGround ())
    {
	setGroundMaterial (optionGetGroundColor ());
	drawBottomGround (ground, csScreen->distance (), -0.5f, drawDeformation);
    }
    else if (optionGetShowWater ())
    {
	setWaterMaterial (optionGetWaterColor ());
	drawBottomWater (water, csScreen->distance (), -0.5f, drawDeformation);
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
AtlantisScreen::preparePaint (int ms)
{
    int i, j;

    updateWater (screen, (float) ms / 1000.0f);
    updateGround (screen, (float) ms / 1000.0f);

    for (i = 0; i < numFish; i++)
    {
	FishPilot (screen, i);

	if (fish[i].type <= FISH2)
	{
	    fish[i].htail = fmodf (fish[i].htail + 0.00025f *
				   fish[i].speed * speedFactor, 1.0f);
	}
    }

    for (i = 0; i < numCrabs; i++)
    {
	CrabPilot (screen, i);
    }

    for (i = 0; i < numCorals; i++)
    {
	coral[i].z = getGroundHeight (screen, coral[i].x, coral[i].y);
    }

    for (i = 0; i < numAerators; i++)
    {
	aeratorRec * a = &(aerator[i]);
	float bottom = getGroundHeight (screen, a->x, a->y);

	if (a->z < bottom)
	{
	    for (j = 0; j < a->numBubbles; j++)
	    {
		if (a->bubbles[j].counter == 0)
		    a->bubbles[j].z = bottom;
	    }
	}
	a->z = bottom;
	for (j = 0; j < a->numBubbles; j++)
	{
	    BubblePilot (screen, i, j);
	}
    }

    cScreen->preparePaint (ms);
}

void
AtlantisScreen::donePaint ()
{
    if (damage)
    {
	cScreen->damageScreen ();
	damage = false;
    }
    cScreen->donePaint ();
}

AtlantisScreen::AtlantisScreen (CompScreen *screen) :
    PluginClassHandler <AtlantisScreen, CompScreen> (screen),
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    csScreen (CubeScreen::get (screen)),
    damage (false),
    numFish (0),
    numCrabs (0),
    numCorals (0),
    numAerators (0),
    fish (NULL),
    crab (NULL),
    coral (NULL),
    aerator (NULL),
    water (NULL),
    ground (NULL),
    waterHeight (50000.0f),
    hsize (0),
    sideDistance (0.0f),
    topDistance (0.0f),
    radius (0.0f),
    arcAngle (0.0f),
    ratio (1.0f),
    speedFactor (1.0f),
    oldProgress (0.0f),
    crabDisplayList (0),
    coralDisplayList (0),
    coral2DisplayList (0),
    bubbleDisplayList (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (csScreen);

    static const float ambient[]  = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };

    glLightfv (GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv (GL_LIGHT1, GL_SPECULAR, specular);
    atlantisInitLightPosition ();

    initAtlantis ();

    optionSetSpeedFactorNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetLowPolyNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetCreatureNumberNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetCreatureSizeNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetCreatureColorNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetCreatureTypeNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetPlantNumberNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetPlantSizeNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetPlantColorNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetPlantTypeNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetRescaleWidthNotify (boost::bind (&AtlantisScreen::updateAtlantis, this));
    optionSetRotateLightingNotify (boost::bind (&AtlantisScreen::atlantisInitLightPosition, this));
    optionSetLightInclinationNotify (boost::bind (&AtlantisScreen::atlantisInitLightPosition, this));
}

AtlantisScreen::~AtlantisScreen ()
{
    freeAtlantis ();
}

bool
AtlantisPluginVTable::init ()
{
    if (CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) &&
	CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) &&
	CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) &&
	CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	return true;

    return false;
}
