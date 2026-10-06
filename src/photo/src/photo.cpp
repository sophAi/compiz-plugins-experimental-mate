#include "photo.h"

COMPIZ_PLUGIN_20090315 (photo, PhotoPluginVTable);

#define PI 3.14159265358979323846

void
PhotoScreen::photoParamChange ()
{
    h = optionGetHeight ();
    l = optionGetWidth ();
    rspeed = optionGetSpeed ();
    xpos = optionGetXpos ();
    ypos = optionGetYpos ();
    zpos = optionGetZpos ();
    transformOrder = optionGetOrder ();
    enablecull = optionGetCull ();
}

void
PhotoScreen::photoCapChange ()
{
    if (dList)
    {
	glDeleteLists (dList, 1);
	dList = 0;
    }

    if (n <= 0)
	return;

    unsigned short *top = optionGetTopColour ();
    unsigned short *bot = optionGetBottomColour ();

    dList = glGenLists (1);
    glNewList (dList, GL_COMPILE);

    glBegin (GL_POLYGON);
    glColor4us (top[0], top[1], top[2], top[3]);
    for (int i = n - 1; i >= 0; i--)
    {
	glVertex3f (cos (i * 2.0 * PI / n), 1.0f, sin (i * 2.0 * PI / n));
    }
    glEnd ();

    glBegin (GL_POLYGON);
    glColor4us (bot[0], bot[1], bot[2], bot[3]);
    for (int i = 0; i < n; i++)
    {
	glVertex3f (cos (i * 2.0 * PI / n), -1.0f, sin (i * 2.0 * PI / n));
    }
    glEnd ();

    glEndList ();
}

void
PhotoScreen::photoTextureChange ()
{
    for (size_t i = 0; i < photoTextures.size (); i++)
    {
	if (photoTextures[i].dList)
	    glDeleteLists (photoTextures[i].dList, 1);
    }
    photoTextures.clear ();

    CompOption::Value::Vector files = optionGetPhotoTextures ();
    n = files.size ();
    if (n <= 0)
    {
	photoCapChange ();
	return;
    }

    photoTextures.resize (n);
    CompString pluginName = "photo";

    for (int i = 0; i < n; i++)
    {
	CompString filename = files[i].s ();
	CompSize size (0, 0);

	photoTextures[i].dList = 0;
	photoTextures[i].width = 0;
	photoTextures[i].height = 0;

	if (!filename.empty ())
	{
	    photoTextures[i].textures = GLTexture::readImageToTexture (filename, pluginName, size);
	    photoTextures[i].width = size.width ();
	    photoTextures[i].height = size.height ();
	}

	if (!photoTextures[i].textures.empty ())
	{
	    GLTexture *tex = photoTextures[i].textures.front ();
	    const GLTexture::Matrix &mat = tex->matrix ();

	    photoTextures[i].dList = glGenLists (1);
	    glNewList (photoTextures[i].dList, GL_COMPILE);

	    int i2 = (i + 1) % n;

	    glBegin (GL_QUADS);
	    glTexCoord2f (COMP_TEX_COORD_X (mat, photoTextures[i].width),
			  COMP_TEX_COORD_Y (mat, 0));
	    glVertex3f (cos (i * 2.0 * PI / n), 1.0f, sin (i * 2.0 * PI / n));

	    glTexCoord2f (COMP_TEX_COORD_X (mat, 0),
			  COMP_TEX_COORD_Y (mat, 0));
	    glVertex3f (cos (i2 * 2.0 * PI / n), 1.0f, sin (i2 * 2.0 * PI / n));

	    glTexCoord2f (COMP_TEX_COORD_X (mat, 0),
			  COMP_TEX_COORD_Y (mat, photoTextures[i].height));
	    glVertex3f (cos (i2 * 2.0 * PI / n), -1.0f, sin (i2 * 2.0 * PI / n));

	    glTexCoord2f (COMP_TEX_COORD_X (mat, photoTextures[i].width),
			  COMP_TEX_COORD_Y (mat, photoTextures[i].height));
	    glVertex3f (cos (i * 2.0 * PI / n), -1.0f, sin (i * 2.0 * PI / n));
	    glEnd ();

	    glEndList ();
	}
    }

    photoCapChange ();
}

void
PhotoScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{
    csScreen->cubeClearTargetOutput (xRotate, vRotate);
    glClear (GL_DEPTH_BUFFER_BIT);
}

void
PhotoScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
			      const GLMatrix            &transform,
			      CompOutput                *output,
			      int                       size,
			      const GLVector            &normal)
{
    if (csScreen->invert () == -1)
    {
	if (!warningPrinted)
	{
	    compLogMessage ("photo", CompLogLevelWarn,
			    "Photowheel is disabled when viewing cube from inside");
	    warningPrinted = true;
	}
	return;
    }

    GLScreenPaintAttrib sA = sAttrib;
    sA.yRotate += csScreen->invert () * (360.0f / size) *
		  (csScreen->xRotations () - (screen->vp ().x () * csScreen->nOutput ()));

    GLMatrix mT = transform;
    gScreen->glApplyTransform (sA, output, &mT);

    glPushMatrix ();
    glLoadMatrixf (mT.getMatrix ());
    glTranslatef (csScreen->outputXOffset (), -csScreen->outputYOffset (), 0.0f);
    glScalef (csScreen->outputXScale (), csScreen->outputYScale (), 1.0f);

    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool enabledCull = glIsEnabled (GL_CULL_FACE);
    if (enablecull)
	glEnable (GL_CULL_FACE);
    else
	glDisable (GL_CULL_FACE);

    glPushMatrix ();
    glDisable (GL_LIGHTING);
    glDisable (GL_COLOR_MATERIAL);
    glEnable (GL_DEPTH_TEST);

    glScalef (0.25f, 0.25f, 0.25f);
    if (transformOrder)
	glTranslatef (xpos, ypos, zpos);

    glRotatef (angle, 0.0f, 1.0f, 0.0f);

    if (!transformOrder)
	glTranslatef (xpos, ypos, zpos);

    glScalef (l, h, l);

    for (int i = 0; i < n; i++)
    {
	if (!photoTextures[i].textures.empty () && photoTextures[i].dList)
	{
	    GLTexture *tex = photoTextures[i].textures.front ();
	    tex->enable (GLTexture::Good);
	    glCallList (photoTextures[i].dList);
	    tex->disable ();
	}
    }

    if (dList)
	glCallList (dList);

    glPopMatrix ();

    glEnable (GL_COLOR_MATERIAL);
    if (gScreen->lighting ())
	glEnable (GL_LIGHTING);

    glDisable (GL_DEPTH_TEST);
    glDisable (GL_BLEND);

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
PhotoScreen::preparePaint (int ms)
{
    angle += rspeed;
    if (angle >= 360.0f)
	angle = 0.0f;

    cScreen->preparePaint (ms);
}

void
PhotoScreen::donePaint ()
{
    if (damage)
    {
	cScreen->damageScreen ();
	damage = false;
    }
    cScreen->donePaint ();
}

PhotoScreen::PhotoScreen (CompScreen *screen) :
    PluginClassHandler <PhotoScreen, CompScreen> (screen),
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    csScreen (CubeScreen::get (screen)),
    damage (false),
    warningPrinted (false),
    angle (0.0f),
    rspeed (0.0f),
    h (1.0f),
    l (1.0f),
    xpos (0.0f),
    ypos (0.0f),
    zpos (0.0f),
    transformOrder (false),
    enablecull (false),
    n (0),
    dList (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (csScreen);

    optionSetPhotoTexturesNotify (boost::bind (&PhotoScreen::photoTextureChange, this));
    optionSetHeightNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetWidthNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetSpeedNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetXposNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetYposNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetZposNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetOrderNotify (boost::bind (&PhotoScreen::photoParamChange, this));
    optionSetTopColourNotify (boost::bind (&PhotoScreen::photoCapChange, this));
    optionSetBottomColourNotify (boost::bind (&PhotoScreen::photoCapChange, this));
    optionSetCullNotify (boost::bind (&PhotoScreen::photoParamChange, this));

    photoParamChange ();
    photoTextureChange ();
}

PhotoScreen::~PhotoScreen ()
{
    for (size_t i = 0; i < photoTextures.size (); i++)
    {
	if (photoTextures[i].dList)
	    glDeleteLists (photoTextures[i].dList, 1);
    }
    if (dList)
	glDeleteLists (dList, 1);
}

bool
PhotoPluginVTable::init ()
{
    if (CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) &&
	CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) &&
	CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) &&
	CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	return true;

    return false;
}
