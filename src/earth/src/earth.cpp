/*
 * Compiz Earth plugin
 *
 * earth.cpp - Ported to Compiz 0.9 C++ Architecture
 */

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "earth.h"
#include <unistd.h>
#include <ctime>

COMPIZ_PLUGIN_20090315 (earth, EarthPluginVTable);

static void
makeSphere (GLdouble radius, GLboolean inside)
{
    GLint i, j;
    const GLfloat PI = M_PI;
    GLfloat sinCache1a[65], cosCache1a[65];
    GLfloat sinCache2a[65], cosCache2a[65];
    GLfloat sinCache1b[65], cosCache1b[65];
    GLfloat sinCache2b[65], cosCache2b[65];
    GLfloat angle, zLow, zHigh;
    GLfloat sintemp1 = 0.0f, sintemp2 = 0.0f, sintemp3 = 0.0f, sintemp4 = 0.0f;
    GLfloat costemp3 = 0.0f, costemp4 = 0.0f;

    for (i = 0; i < 64; i++)
    {
	angle = 2.0f * PI * i / 64.0f;
	sinCache1a[i] = sinf (angle);
	cosCache1a[i] = cosf (angle);
	sinCache2a[i] = sinCache1a[i];
	cosCache2a[i] = cosCache1a[i];
    }

    for (j = 0; j <= 64; j++)
    {
	angle = PI * j / 64.0f;
	if (!inside)
	{
	    sinCache2b[j] = sinf (angle);
	    cosCache2b[j] = cosf (angle);
	}
	else
	{
	    sinCache2b[j] = -sinf (angle);
	    cosCache2b[j] = -cosf (angle);
	}
	sinCache1b[j] = radius * sinf (angle);
	cosCache1b[j] = radius * cosf (angle);
    }
    sinCache1b[0] = 0;
    sinCache1b[64] = 0;

    sinCache1a[64] = sinCache1a[0];
    cosCache1a[64] = cosCache1a[0];
    sinCache2a[64] = sinCache2a[0];
    cosCache2a[64] = cosCache2a[0];

    for (j = 0; j < 64; j++)
    {
	zLow = cosCache1b[j];
	zHigh = cosCache1b[j+1];

	sintemp1 = sinCache1b[j];
	sintemp2 = sinCache1b[j+1];

	if (!inside)
	{
	    sintemp3 = sinCache2b[j+1];
	    costemp3 = cosCache2b[j+1];
	    sintemp4 = sinCache2b[j];
	    costemp4 = cosCache2b[j];
	}
	else
	{
	    sintemp3 = sinCache2b[j];
	    costemp3 = cosCache2b[j];
	    sintemp4 = sinCache2b[j+1];
	    costemp4 = cosCache2b[j+1];
	}

	glBegin (GL_QUAD_STRIP);
	for (i = 0; i <= 64; i++)
	{
	    glNormal3f (sinCache2a[i] * sintemp3, cosCache2a[i] * sintemp3, costemp3);
	    if (!inside)
	    {
		glTexCoord2f (1.0f - (float) i / 64.0f, 1.0f - (float) (j + 1) / 64.0f);
		glVertex3f (sintemp2 * sinCache1a[i], sintemp2 * cosCache1a[i], zHigh);
	    }
	    else
	    {
		glTexCoord2f (1.0f - (float) i / 64.0f, 1.0f - (float) j / 64.0f);
		glVertex3f (sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zLow);
	    }

	    glNormal3f (sinCache2a[i] * sintemp4, cosCache2a[i] * sintemp4, costemp4);
	    if (!inside)
	    {
		glTexCoord2f (1.0f - (float) i / 64.0f, 1.0f - (float) j / 64.0f);
		glVertex3f (sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zLow);
	    }
	    else
	    {
		glTexCoord2f (1.0f - (float) i / 64.0f, 1.0f - (float) (j + 1) / 64.0f);
		glVertex3f (sintemp2 * sinCache1a[i], sintemp2 * cosCache1a[i], zHigh);
	    }
	}
	glEnd ();
    }
}

static char*
LoadSource (const char* filename)
{
    FILE* fp = fopen (filename, "r");
    if (!fp)
	return NULL;
    fseek (fp, 0, SEEK_END);
    long size = ftell (fp);
    rewind (fp);
    char* src = (char *) malloc (size + 1);
    if (!src)
    {
	fclose (fp);
	return NULL;
    }
    fread (src, 1, size, fp);
    src[size] = '\0';
    fclose (fp);
    return src;
}

void
EarthScreen::createShaders ()
{
    shadersupport = GL_FALSE;
    vert = 0;
    frag = 0;
    prog = 0;

    const char *vertfile = NULL;
    const char *fragfile = NULL;

    if (access ("/usr/share/compiz/earth/earth.vert", R_OK) == 0)
	vertfile = "/usr/share/compiz/earth/earth.vert";
    else if (access ("data/earth/earth.vert", R_OK) == 0)
	vertfile = "data/earth/earth.vert";

    if (access ("/usr/share/compiz/earth/earth.frag", R_OK) == 0)
	fragfile = "/usr/share/compiz/earth/earth.frag";
    else if (access ("data/earth/earth.frag", R_OK) == 0)
	fragfile = "data/earth/earth.frag";

    if (!vertfile || !fragfile)
	return;

    vert = glCreateShader (GL_VERTEX_SHADER);
    frag = glCreateShader (GL_FRAGMENT_SHADER);

    char *vertsource = LoadSource (vertfile);
    char *fragsource = LoadSource (fragfile);

    if (!vertsource || !fragsource)
    {
	if (vertsource) free (vertsource);
	if (fragsource) free (fragsource);
	return;
    }

    glShaderSource (vert, 1, (const GLchar**)&vertsource, NULL);
    glShaderSource (frag, 1, (const GLchar**)&fragsource, NULL);

    glCompileShader (vert);
    glCompileShader (frag);

    free (vertsource);
    free (fragsource);

    GLint status = 0;
    glGetShaderiv (vert, GL_COMPILE_STATUS, &status);
    if (!status)
    {
	glDeleteShader (vert);
	glDeleteShader (frag);
	vert = frag = 0;
	return;
    }

    glGetShaderiv (frag, GL_COMPILE_STATUS, &status);
    if (!status)
    {
	glDeleteShader (vert);
	glDeleteShader (frag);
	vert = frag = 0;
	return;
    }

    prog = glCreateProgram ();
    glAttachShader (prog, vert);
    glAttachShader (prog, frag);
    glLinkProgram (prog);

    glGetProgramiv (prog, GL_LINK_STATUS, &status);
    if (!status)
    {
	glDeleteShader (vert);
	glDeleteShader (frag);
	glDeleteProgram (prog);
	vert = frag = prog = 0;
	return;
    }

    shadersupport = GL_TRUE;
}

void
EarthScreen::deleteShaders ()
{
    if (!shadersupport)
	return;

    if (prog)
    {
	if (vert) glDetachShader (prog, vert);
	if (frag) glDetachShader (prog, frag);
	glDeleteProgram (prog);
	prog = 0;
    }

    if (vert)
    {
	glDeleteShader (vert);
	vert = 0;
    }
    if (frag)
    {
	glDeleteShader (frag);
	frag = 0;
    }

    shadersupport = GL_FALSE;
}

void
EarthScreen::createLists ()
{
    list[0] = glGenLists (4);

    for (int i = 0; i < 4; i++)
    {
	GLdouble radius = 0.75;
	GLboolean inside = GL_FALSE;

	switch (i)
	{
	case SUN:
	    radius = 0.1;
	    inside = GL_TRUE;
	    break;
	case EARTH:
	    radius = 0.75;
	    inside = GL_FALSE;
	    break;
	case CLOUDS:
	    radius = 0.76;
	    inside = GL_FALSE;
	    break;
	case SKY:
	    radius = 10.0;
	    inside = GL_TRUE;
	    break;
	}

	list[i] = list[0] + i;
	glNewList (list[i], GL_COMPILE);
	makeSphere (radius, inside);
	glEndList ();
    }
}

void
EarthScreen::loadTextures ()
{
    CompSize size (0, 0);
    CompString pluginName = "earth";

    const char *names[4] = {
	"/usr/share/compiz/earth/day.png",
	"/usr/share/compiz/earth/night.png",
	"/usr/share/compiz/earth/clouds.png",
	"/usr/share/compiz/earth/skydome.png"
    };

    char cloudsPath[512];
    const char *home = getenv ("HOME");
    if (home)
    {
	snprintf (cloudsPath, sizeof (cloudsPath), "%s/.compiz/images/clouds.png", home);
	if (access (cloudsPath, R_OK) == 0)
	    names[CLOUDS] = cloudsPath;
    }

    for (int i = 0; i < 4; i++)
    {
	CompString texfile = names[i];
	texList[i] = GLTexture::readImageToTexture (texfile, pluginName, size);
	if (!texList[i].empty ())
	    tex[i] = texList[i].front ();
	else
	    tex[i] = NULL;
    }
}

static size_t
write_clouds_file (void *buffer, size_t size, size_t nmemb, void *stream)
{
    CloudsFile* out = (CloudsFile*) stream;
    if (out && !out->stream)
    {
	out->stream = fopen (out->filename, "wb");
	if (!out->stream)
	    return 0;
    }
    return fwrite (buffer, size, nmemb, out->stream);
}

static void*
DownloadClouds_t (void* threaddata)
{
    CloudsThreadData* data = (CloudsThreadData*) threaddata;
    EarthScreen* es = data->es;

    data->started = 1;
    data->finished = 0;

    if (es->curlhandle)
	curl_easy_perform (es->curlhandle);

    if (es->cloudsfile.stream)
    {
	fclose (es->cloudsfile.stream);
	es->cloudsfile.stream = NULL;
    }

    data->finished = 1;
    return NULL;
}

void
EarthScreen::optionChanged ()
{
    lat = optionGetLatitude ();
    lon = optionGetLongitude ();
    tz = optionGetTimezone ();
    clouds = optionGetClouds ();
    rotspeed = optionGetRotation ();
    earth_size = optionGetEarthSize ();
    shaders = optionGetShaders ();

    if (shaders)
    {
	light[EARTH].specular[0] = 0.5f;
	light[EARTH].specular[1] = 0.5f;
	light[EARTH].specular[2] = 0.4f;
	light[EARTH].specular[3] = 1.0f;
    }
    else
    {
	light[EARTH].specular[0] = 0.0f;
	light[EARTH].specular[1] = 0.0f;
	light[EARTH].specular[2] = 0.0f;
	light[EARTH].specular[3] = 0.0f;
    }

    CompString url = optionGetCloudsUrl ();
    if (curlhandle)
    {
	clouds_url_changed = true;
	curl_easy_setopt (curlhandle, CURLOPT_URL, url.c_str ());
    }
}

void
EarthScreen::initEarth ()
{
    lat = optionGetLatitude ();
    lon = optionGetLongitude ();
    tz = optionGetTimezone ();
    clouds = optionGetClouds ();
    rotspeed = optionGetRotation ();
    earth_size = optionGetEarthSize ();
    shaders = optionGetShaders ();

    previousoutput = 0;
    dec = 0.0f;
    gha = 0.0f;
    rotation = 0.0f;

    cloudsthreaddata.started = 0;
    cloudsthreaddata.finished = 0;
    cloudsthreaddata.url = NULL;
    clouds_url_changed = false;

    const char *home = getenv ("HOME");
    char cfile[512];
    if (home)
	snprintf (cfile, sizeof (cfile), "%s/.compiz/images/clouds.jpg", home);
    else
	snprintf (cfile, sizeof (cfile), "/tmp/clouds.jpg");
    cloudsfile.filename = strdup (cfile);
    cloudsfile.stream = NULL;

    curl_global_init (CURL_GLOBAL_DEFAULT);
    curlhandle = curl_easy_init ();

    if (curlhandle)
    {
	CompString url = optionGetCloudsUrl ();
	curl_easy_setopt (curlhandle, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curlhandle, CURLOPT_WRITEFUNCTION, write_clouds_file);
	curl_easy_setopt (curlhandle, CURLOPT_WRITEDATA, &cloudsfile);
    }

    createShaders ();

    for (int i = 0; i < 4; i++)
    {
	light[SUN].ambient[i] = 0.2f;
	light[SUN].diffuse[i] = 1.0f;
	light[SUN].specular[i] = 1.0f;
	light[SUN].position[i] = 0.0f;

	light[EARTH].ambient[i] = 0.1f;
	light[EARTH].diffuse[i] = 1.0f;
	light[EARTH].specular[i] = (shaders ? 0.5f : 0.0f);

	light[CLOUDS].specular[i] = 0.0f;
    }
    light[SUN].position[1] = 1.0f;
    light[EARTH].shininess = 50.0f;

    createLists ();
    loadTextures ();
}

void
EarthScreen::freeEarth ()
{
    glDeleteLists (list[0], 4);
    deleteShaders ();

    for (int i = 0; i < 4; i++)
    {
	texList[i].clear ();
	tex[i] = NULL;
    }

    if (curlhandle)
    {
	curl_easy_cleanup (curlhandle);
	curlhandle = NULL;
    }
    curl_global_cleanup ();

    if (cloudsfile.filename)
    {
	free (cloudsfile.filename);
	cloudsfile.filename = NULL;
    }
}

void
EarthScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{
    glDisable (GL_LIGHTING);
    glPushMatrix ();

    float ratio = (float) screen->height () / (float) screen->width ();
    glScalef (ratio, 1.0f, ratio);

    glRotatef (vRotate - 90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef (xRotate + rotation, 0.0f, 0.0f, 1.0f);

    glRotatef (lat, 1.0f, 0.0f, 0.0f);
    glRotatef (lon + 180.0f, 0.0f, 0.0f, 1.0f);

    if (tex[SKY])
    {
	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, tex[SKY]->name ());
	glCallList (list[SKY]);
	glBindTexture (GL_TEXTURE_2D, 0);
	glDisable (GL_TEXTURE_2D);
    }

    glRotatef (-gha * 15.0f, 0.0f, 0.0f, 1.0f);
    glRotatef (dec, 1.0f, 0.0f, 0.0f);

    glTranslatef (0.0f, -5.0f, 0.0f);
    glCallList (list[SUN]);

    glPopMatrix ();
    glEnable (GL_LIGHTING);

    glClear (GL_DEPTH_BUFFER_BIT);
}

void
EarthScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
			      const GLMatrix            &transform,
			      CompOutput                *output,
			      int                       size,
			      const GLVector            &normal)
{
    if (csScreen->invert () == -1)
	return;

    GLScreenPaintAttrib sA = sAttrib;
    sA.yRotate += csScreen->invert () * (360.0f / size) *
		  (csScreen->xRotations () - (screen->vp ().x () * csScreen->nOutput ()));

    GLMatrix mT = transform;
    gScreen->glApplyTransform (sA, output, &mT);

    glPushMatrix ();
    glLoadMatrixf (mT.getMatrix ());
    glTranslatef (csScreen->outputXOffset (), -csScreen->outputYOffset (), 0.0f);
    glScalef (csScreen->outputXScale (), csScreen->outputYScale (), 1.0f);

    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT);
    glEnable (GL_DEPTH_TEST);

    glPushMatrix ();

    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT1);
    glEnable (GL_BLEND);
    glDisable (GL_COLOR_MATERIAL);

    float ratio = (float) output->height () / (float) output->width ();
    glScalef (ratio * earth_size, earth_size, ratio * earth_size);
    previousoutput = output->id ();

    glRotatef (rotation, 0.0f, 1.0f, 0.0f);

    glRotatef (lat - 90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef (lon, 0.0f, 0.0f, 1.0f);

    glPushMatrix ();
    glRotatef (-gha * 15.0f, 0.0f, 0.0f, 1.0f);
    glRotatef (-dec, 1.0f, 0.0f, 0.0f);

    glLightfv (GL_LIGHT1, GL_POSITION, light[SUN].position);
    glLightfv (GL_LIGHT1, GL_AMBIENT, light[SUN].ambient);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, light[SUN].diffuse);
    glLightfv (GL_LIGHT1, GL_SPECULAR, light[SUN].specular);
    glPopMatrix ();

    glBlendFunc (GL_ONE, GL_ZERO);

    glMaterialfv (GL_FRONT, GL_AMBIENT, light[EARTH].ambient);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, light[EARTH].diffuse);
    glMaterialfv (GL_FRONT, GL_SPECULAR, light[EARTH].specular);
    glMaterialf (GL_FRONT, GL_SHININESS, light[EARTH].shininess);

    if (shadersupport && shaders && prog)
    {
	glUseProgram (prog);

	glActiveTexture (GL_TEXTURE0);
	if (tex[DAY])
	{
	    glEnable (GL_TEXTURE_2D);
	    glBindTexture (GL_TEXTURE_2D, tex[DAY]->name ());
	}

	glActiveTexture (GL_TEXTURE1);
	if (tex[NIGHT])
	{
	    glEnable (GL_TEXTURE_2D);
	    glBindTexture (GL_TEXTURE_2D, tex[NIGHT]->name ());
	}

	glUniform1i (glGetUniformLocation (prog, "daytex"), 0);
	glUniform1i (glGetUniformLocation (prog, "nighttex"), 1);
    }
    else
    {
	glActiveTexture (GL_TEXTURE0);
	if (tex[DAY])
	{
	    glEnable (GL_TEXTURE_2D);
	    glBindTexture (GL_TEXTURE_2D, tex[DAY]->name ());
	}
    }

    glCallList (list[EARTH]);

    if (shadersupport && shaders && prog)
    {
	glUseProgram (0);
	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, 0);
	glDisable (GL_TEXTURE_2D);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, 0);
	glDisable (GL_TEXTURE_2D);
    }
    else
    {
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, 0);
	glDisable (GL_TEXTURE_2D);
    }

    if (tex[CLOUDS])
    {
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMaterialfv (GL_FRONT, GL_SPECULAR, light[CLOUDS].specular);
	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, tex[CLOUDS]->name ());
	glCallList (list[CLOUDS]);
	glBindTexture (GL_TEXTURE_2D, 0);
	glDisable (GL_TEXTURE_2D);
    }

    glDisable (GL_LIGHT1);

    glPopMatrix ();
    glPopAttrib ();
    glPopMatrix ();

    damage = true;

    csScreen->cubePaintInside (sAttrib, transform, output, size, normal);
}

void
EarthScreen::preparePaint (int ms)
{
    time_t timer = time (NULL);
    struct tm* currenttime = localtime (&timer);
    struct stat attrib;

    dec = 23.4400f * cosf ((6.2831f / 365.0f) * ((float) currenttime->tm_yday + 10.0f));
    gha = (float) currenttime->tm_hour - (tz + (float) currenttime->tm_isdst) +
	  (float) currenttime->tm_sec / 3600.0f;

    rotation += rotspeed;
    if (rotation > 360.0f)
	rotation -= 360.0f;
    else if (rotation < -360.0f)
	rotation += 360.0f;

    int res = stat (cloudsfile.filename, &attrib);
    if (clouds && (cloudsthreaddata.started == 0) &&
	(clouds_url_changed || (res != 0) || difftime (timer, attrib.st_mtime) > (3600 * 3)))
    {
	cloudsthreaddata.es = this;
	pthread_create (&cloudsthreaddata.tid, NULL, DownloadClouds_t, (void*) &cloudsthreaddata);
	clouds_url_changed = false;
    }

    if (cloudsthreaddata.finished == 1)
    {
	pthread_join (cloudsthreaddata.tid, NULL);
	CompSize size (0, 0);
	CompString pluginName = "earth";
	CompString cFile = cloudsfile.filename;
	texList[CLOUDS] = GLTexture::readImageToTexture (cFile, pluginName, size);
	if (!texList[CLOUDS].empty ())
	    tex[CLOUDS] = texList[CLOUDS].front ();
	cloudsthreaddata.finished = 0;
	cloudsthreaddata.started = 0;
    }

    cScreen->preparePaint (ms);
}

void
EarthScreen::donePaint ()
{
    if (damage)
    {
	cScreen->damageScreen ();
	damage = false;
    }
    cScreen->donePaint ();
}

EarthScreen::EarthScreen (CompScreen *screen) :
    PluginClassHandler <EarthScreen, CompScreen> (screen),
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    csScreen (CubeScreen::get (screen)),
    damage (false),
    lon (0.0f),
    lat (0.0f),
    tz (0.0f),
    shaders (true),
    clouds (false),
    rotspeed (0.0f),
    clouds_url_changed (false),
    earth_size (0.7f),
    previousoutput (0),
    dec (0.0f),
    gha (0.0f),
    rotation (0.0f),
    curlhandle (NULL),
    shadersupport (GL_FALSE),
    vert (0),
    frag (0),
    prog (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (csScreen);

    for (int i = 0; i < 4; i++)
	tex[i] = NULL;

    initEarth ();

    optionSetLatitudeNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetLongitudeNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetTimezoneNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetShadersNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetCloudsNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetRotationNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetCloudsUrlNotify (boost::bind (&EarthScreen::optionChanged, this));
    optionSetEarthSizeNotify (boost::bind (&EarthScreen::optionChanged, this));
}

EarthScreen::~EarthScreen ()
{
    freeEarth ();
}

bool
EarthPluginVTable::init ()
{
    if (CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) &&
	CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) &&
	CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) &&
	CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	return true;

    return false;
}
