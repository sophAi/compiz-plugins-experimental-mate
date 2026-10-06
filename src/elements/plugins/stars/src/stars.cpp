#include "stars.h"

/* STARS SPECIFIC FUNCTIONS */

COMPIZ_PLUGIN_20090315 (stars, StarPluginVTable);

StarElement::StarElement ()
{
}

StarElement::~StarElement ()
{
}

static float
starBezierCurve (float p,
		 float time)
{
    float out;

    out = p * (time + 0.01) * 10;

    return out;
}

Element *
StarElement::create ()
{
    Element *e = (Element *) new StarElement;

    return e;
}

bool
StarElement::init ()
{
    float init;
    StarScreen *ss = StarScreen::get (screen);

    ELEMENTS_SCREEN (screen);

    dx = es->mmRand (-50000, 50000, 5000);
    dy = es->mmRand (-50000, 50000, 5000);
    dz = es->mmRand (000, 200, 2000);
    x = screen->width () / 2 + ss->optionGetStarOffsetX (); /* X Offset */
    y = screen->height () / 2 + ss->optionGetStarOffsetY (); /* Y Offset */
    z = es->mmRand (000, 0.1, 5000);
    init = es->mmRand (0,100, 1);

    x += init * dx;
    y += init * dy;

    return true;

}

void
StarElement::move ()
{
    ELEMENTS_SCREEN (screen);

    int updateDelay = es->updateDelay ();

    float xs, ys, zs;
    float starsSpeed = anim->speed () / 500.0f;
    float tmp = 1.0f / (100.0f - starsSpeed);

    xs = starBezierCurve(dx, tmp);
    ys = starBezierCurve(dy, tmp);
    zs = starBezierCurve(dz, tmp);

    x += xs * updateDelay * starsSpeed;
    y += ys * updateDelay * starsSpeed;
    z += zs * updateDelay * starsSpeed;
}

bool
StarScreen::toggle (CompAction         *action,
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
	    o.value ().set ("stars");

	    newOptions.push_back (o);

	    opt.value ().action ().initiate () (action, state, newOptions);

	    break;
	}
    }

    return false;
}


void
StarScreen::autostartChanged (CompOption *opt, StarsOptions::Options num)
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->setAnimationRunning ("stars", optionGetAutostart ());
}

StarScreen::StarScreen (CompScreen *screen) :
    PluginClassHandler <StarScreen, CompScreen> (screen)
{
    type =  ElementType::create ("stars", "Stars", StarElement::create);

    optionSetToggleStarInitiate (StarScreen::toggle);
    optionSetAutostartNotify (boost::bind (&StarScreen::autostartChanged, this,
					   boost::placeholders::_1,
					   boost::placeholders::_2));

    if (!type)
	setFailed ();
    else if (optionGetAutostart ())
    {
	ElementScreen *es = ElementScreen::get (screen);
	if (es)
	    es->startAnimation ("stars");
    }
}

StarScreen::~StarScreen ()
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->stopAnimation ("stars");

    type->destroy ();
}

void
StarElement::fini ()
{
}

bool
StarPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("elements", COMPIZ_ELEMENTS_ABI))
	return false;

    return true;
}
