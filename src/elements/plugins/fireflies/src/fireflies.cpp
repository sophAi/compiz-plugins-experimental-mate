#include "fireflies.h"

#define GLOW_STAGES 5

COMPIZ_PLUGIN_20090315 (fireflies, FireflyPluginVTable);

/* FIREFLIES SPECIFIC FUNCTIONS */

static float
glowCurve[GLOW_STAGES][4] = {
    { 0.0, 0.5, 0.5, 1.0 },
    { 1.0, 1.0, 0.5, 0.75 },
    { 0.75, 0.3, 1.2, 1.0 },
    { 1.0, 0.7, 1.5, 1.0 },
    { 1.0, 0.5, 0.5, 0.0 }
};

FireflyElement::FireflyElement ()
{
}

FireflyElement::~FireflyElement ()
{
}

static float
fireflyBezierCurve (float p[4],
		    float time)
{
    float out;
    float a = p[0];
    float b = p[0] + p[1];
    float c = p[3] + p[2];
    float d = p[3];
    float e = 1.0 - time;

    out = (a * pow (e, 3)) + (3.0 * b * pow (e, 2) * time) +
	  (3.0 * c * e * pow (time, 2)) + (d * pow (time, 3));

    return out;
}

Element *
FireflyElement::create ()
{
    Element *e = (Element *) new FireflyElement;

    return e;
}

bool
FireflyElement::init ()
{
    int            i;

    ELEMENTS_SCREEN (screen);

    x = es->mmRand (0, screen->width (), 1);
    y = es->mmRand (0, screen->height (), 1);

    lifespan = es->mmRand (50,1000, 100);
    age      = 0.0;

    for (i = 0; i < 4; i++)
    {
	dx[i] = es->mmRand (-3000, 3000, 1000);
	dy[i] = es->mmRand (-3000, 3000, 1000);
	dz[i] = es->mmRand (-1000, 1000, 500000);
    }

    return true;
}

void
FireflyElement::move ()
{
    float          ffSpeed = anim->speed () / 700.0f;
    int            glowStage;
    float          xs, ys, zs;

    ELEMENTS_SCREEN (screen);

    age       += 0.01;
    lifecycle = (age / 10) / lifespan * (ffSpeed * 70);
    glowStage = (int) fmodf ((lifecycle * GLOW_STAGES), (GLOW_STAGES - 1));
    glowAlpha = fireflyBezierCurve (glowCurve[glowStage], lifecycle);

    xs = fireflyBezierCurve (dx, lifecycle);
    ys = fireflyBezierCurve (dy, lifecycle);
    zs = fireflyBezierCurve (dz, lifecycle);

    x += xs * es->updateDelay () * ffSpeed;
    y += ys * es->updateDelay () * ffSpeed;
    z += zs * es->updateDelay () * ffSpeed;
}

bool
FirefliesScreen::toggle (CompAction         *action,
		    CompAction::State  state,
		    CompOption::Vector &options)
{
    CompPlugin *p = CompPlugin::find ("elements");

    if (!p)
	return true;

    foreach (CompOption &opt, p->vTable->getOptions ())
    {
	if (opt.type () == CompOption::TypeAction && 
	    opt.name () == "toggle_name")
	{
	    CompOption o;
	    CompOption::Vector newOptions = options;

	    o.setName ("type", CompOption::TypeString);
	    o.value ().set ("fireflies");

	    newOptions.push_back (o);

	    opt.value ().action ().initiate () (action, state, newOptions);

	    break;
	}
    }

    return false;
}



void
FirefliesScreen::autostartChanged (CompOption *opt, FirefliesOptions::Options num)
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->setAnimationRunning ("fireflies", optionGetAutostart ());
}

FirefliesScreen::FirefliesScreen (CompScreen *screen) :
    PluginClassHandler <FirefliesScreen, CompScreen> (screen)
{
    type =  ElementType::create ("fireflies", "Firefly", FireflyElement::create);

    optionSetToggleFireflyInitiate (FirefliesScreen::toggle);
    optionSetAutostartNotify (boost::bind (&FirefliesScreen::autostartChanged, this,
					   boost::placeholders::_1,
					   boost::placeholders::_2));

    if (!type)
	setFailed ();
    else if (optionGetAutostart ())
    {
	ElementScreen *es = ElementScreen::get (screen);
	if (es)
	    es->startAnimation ("fireflies");
    }
}

FirefliesScreen::~FirefliesScreen ()
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->stopAnimation ("fireflies");

    type->destroy ();
}

void
FireflyElement::fini ()
{
}

bool
FireflyPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("elements", COMPIZ_ELEMENTS_ABI))
	return false;

    return true;
}
