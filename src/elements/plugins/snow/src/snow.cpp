#include "snow.h"

/* SNOW SPECIFIC FUNCTIONS */

COMPIZ_PLUGIN_20090315 (snow, SnowPluginVTable);

SnowElement::SnowElement ()
{
}

SnowElement::~SnowElement ()
{
}

Element *
SnowElement::create ()
{
    Element *e = (Element *) new SnowElement;

    return e;
}

bool
SnowElement::init ()
{
    SnowScreen *ss = SnowScreen::get (screen);

    ELEMENTS_SCREEN (screen);

    int snowSway = ss->optionGetSnowSway ();
    int boxing = es->boxing ();

    switch (ss->optionGetWindDirection ())
    {
	case SnowOptions::WindDirectionNone:
	    x  = es->mmRand (-boxing, screen->width () + boxing, 1);
	    dx = es->mmRand (-snowSway, snowSway, 1.0);
	    y  = es->mmRand (-300, 0, 1);
	    dy = es->mmRand (1, 3, 1);
	    break;
	case SnowOptions::WindDirectionUp:
	    x  = es->mmRand (-boxing, screen->width () + boxing, 1);
	    dx = es->mmRand (-snowSway, snowSway, 1.0);
	    y  = es->mmRand (screen->height () + 300, 0, 1);
	    dy = -es->mmRand (1, 3, 1);
	    break;
	case SnowOptions::WindDirectionLeft:
	    x  = es->mmRand (screen->width (), screen->width () + 300, 1);
	    dx = -es->mmRand (1, 3, 1);
	    y  = es->mmRand (-boxing, screen->height () + boxing, 1);
	    dy = es->mmRand (-snowSway, snowSway, 1.5);
	    break;
	case SnowOptions::WindDirectionRight:
	    x  = es->mmRand (-300, 0, 1);
	    dx = es->mmRand (1, 3, 1);
	    y  = es->mmRand (-boxing, screen->height () + boxing, 1);
	    dy = es->mmRand (-snowSway, snowSway, 1.5);
	    break;
	default:
	    break;
    }

    return true;

}

void
SnowElement::move ()
{
    ELEMENTS_SCREEN (screen);

    float snowSpeed = anim->speed () / 500.0f;

    x += (dx * (float) es->updateDelay ()) * snowSpeed;
    y += (dy * (float) es->updateDelay ()) * snowSpeed;
    z += (dz * (float) es->updateDelay ()) * snowSpeed;
    rAngle += ((float) es->updateDelay ()) / (10.1f - rSpeed);
}

void
SnowElement::fini ()
{
}

bool
SnowScreen::toggle (CompAction         *action,
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
	    o.value ().set ("snow");

	    newOptions.push_back (o);

	    opt.value ().action ().initiate () (action, state, newOptions);

	    break;
	}
    }

    return false;
}

void
SnowScreen::autostartChanged (CompOption *opt, SnowOptions::Options num)
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->setAnimationRunning ("snow", optionGetAutostart ());
}

SnowScreen::SnowScreen (CompScreen *screen) :
    PluginClassHandler <SnowScreen, CompScreen> (screen)
{
    type = ElementType::create ("snow", "Snow", SnowElement::create);

    optionSetToggleSnowInitiate (SnowScreen::toggle);
    optionSetAutostartNotify (boost::bind (&SnowScreen::autostartChanged, this,
					   boost::placeholders::_1,
					   boost::placeholders::_2));

    if (!type)
	setFailed ();
    else if (optionGetAutostart ())
    {
	ElementScreen *es = ElementScreen::get (screen);
	if (es)
	    es->startAnimation ("snow");
    }
}

SnowScreen::~SnowScreen ()
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->stopAnimation ("snow");

    type->destroy ();
}

bool
SnowPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("elements", COMPIZ_ELEMENTS_ABI))
	return false;

    return true;
}

