#include "autumn.h"

COMPIZ_PLUGIN_20090315 (autumn, AutumnPluginVTable);

/* AUTUMN FUNCTIONS */

AutumnElement::AutumnElement ()
{
}

AutumnElement::~AutumnElement ()
{
}

Element *
AutumnElement::create ()
{
    Element *e = (Element *) new AutumnElement;

    return e;
}

bool
AutumnElement::init ()
{
    int		i;
    float	xSway, ySway;
    AutumnScreen *as = AutumnScreen::get (screen);

    ELEMENTS_SCREEN (screen);

    xSway = es->mmRand (as->optionGetXSway (),
			as->optionGetXSway (),
			2.0f);
    ySway = as->optionGetYSway () / 20.0;

    for (i = 0; i < MAX_AUTUMN_AGE; i++)
	autumnFloat[0][i] = -xSway +
				(i * ((2 * xSway) / (MAX_AUTUMN_AGE - 1)));

    for (i = 0; i < (MAX_AUTUMN_AGE / 2); i++)
	autumnFloat[1][i] = -ySway +
				(i * ((2 * ySway) / (MAX_AUTUMN_AGE - 1)));

    for (; i < MAX_AUTUMN_AGE; i++)
	autumnFloat[1][i] = ySway -
				(i * ((2 * ySway) / (MAX_AUTUMN_AGE - 1)));

    autumnAge[0] = es->getRand (0, MAX_AUTUMN_AGE - 1);
    autumnAge[1] = es->getRand (0, (MAX_AUTUMN_AGE / 2) - 1);
    autumnChange = 1;

    x  = es->mmRand (0, screen->width (), 1);    autumnChange = 1;
    y  = -es->mmRand (100, screen->height (), 1);
    dy = es->mmRand (-2, -1, 5);

    return true;

}

void
AutumnElement::move ()
{
    float autumnSpeed = anim->speed () / 30.0f;

    ELEMENTS_SCREEN (screen);

    x += (autumnFloat[0][autumnAge[0]] * (float) es->updateDelay ()) / 80;
    y += (autumnFloat[1][autumnAge[1]] * (float) es->updateDelay ()) / 80 +
	    autumnSpeed;
    z += (dz * (float) es->updateDelay ()) * autumnSpeed / 100.0;
    rAngle += ((float) es->updateDelay ()) / (10.1f - rSpeed);

    autumnAge[0] += autumnChange;
    autumnAge[1] += 1;

    if (autumnAge[1] >= MAX_AUTUMN_AGE)
    {
	autumnAge[1] = 0;
    }
    if (autumnAge[0] >= MAX_AUTUMN_AGE)
    {
	autumnAge[0] = MAX_AUTUMN_AGE - 1;
	autumnChange = -1;
    }
    if (autumnAge[0] <= -1)
    {
	autumnAge[0] = 0;
	autumnChange = 1;
    }
}

bool
AutumnScreen::toggle (CompAction         *action,
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
	    o.value ().set ("autumn");

	    newOptions.push_back (o);

	    opt.value ().action ().initiate () (action, state, newOptions);

	    break;
	}
    }

    return false;
}

void
AutumnElement::fini ()
{
}

void
AutumnScreen::autostartChanged (CompOption *opt, AutumnOptions::Options num)
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->setAnimationRunning ("autumn", optionGetAutostart ());
}

AutumnScreen::AutumnScreen (CompScreen *) :
    PluginClassHandler <AutumnScreen, CompScreen> (screen)
{
    type = ElementType::create ("autumn", "Autumn", AutumnElement::create);

    optionSetToggleAutumnInitiate (AutumnScreen::toggle);
    optionSetAutostartNotify (boost::bind (&AutumnScreen::autostartChanged, this,
					   boost::placeholders::_1,
					   boost::placeholders::_2));

    if (!type)
	setFailed ();
    else if (optionGetAutostart ())
    {
	ElementScreen *es = ElementScreen::get (screen);
	if (es)
	    es->startAnimation ("autumn");
    }
}

AutumnScreen::~AutumnScreen ()
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->stopAnimation ("autumn");

    type->destroy ();
}

bool
AutumnPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("elements", COMPIZ_ELEMENTS_ABI))
	return false;

    return true;
}
