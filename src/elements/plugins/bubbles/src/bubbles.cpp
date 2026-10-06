#include "bubbles.h"

/* BUBBLES SPECIFIC FUNCTIONS */

COMPIZ_PLUGIN_20090315 (bubbles, BubblePluginVTable);

BubbleElement::BubbleElement ()
{
}

BubbleElement::~BubbleElement ()
{
}

Element *
BubbleElement::create ()
{
    Element *e = (Element *) new BubbleElement;

    return e;
}

bool
BubbleElement::init ()
{
    int           i;
    float         temp, xSway;
    BubbleScreen  *bs = BubbleScreen::get (screen);


    ELEMENTS_SCREEN (screen);

    temp  = es->mmRand (bs->optionGetViscosity () / 2.0,
			    bs->optionGetViscosity (), 50.0);
    xSway = 1.0 - temp * temp / 4.0;

    for (i = 0; i < MAX_BUBBLE_AGE; i++)
	bubbleFloat[0][i] = -xSway +
				(i * ((2 * xSway) / (MAX_BUBBLE_AGE - 1)));

    bubbleAge[0] = es->getRand (0, MAX_BUBBLE_AGE - 1);
    bubbleAge[1] = bubbleAge[0];
    bubbleChange = 1;


    x  = es->mmRand (0, screen->width (), 1);
    y  = es->mmRand (screen->height () + 100, screen->height (), 1);
    dy = es->mmRand (-2, -1, 5);

    return true;

}

void
BubbleElement::move ()
{
    float bubblesSpeed = anim->speed () / 30.0f;

    ELEMENTS_SCREEN (screen);

    x += (bubbleFloat[0][bubbleAge[0]] * (float) es->updateDelay ()) / 8;
    y += (dy * (float) es->updateDelay ()) * bubblesSpeed;
    z += (dz * (float) es->updateDelay ()) * bubblesSpeed / 100.0;
    rAngle += ((float) es->updateDelay ()) / (10.1f - rSpeed);

    bubbleAge[0] += bubbleChange;
    if (bubbleAge[0] >= MAX_BUBBLE_AGE)
    {
	bubbleAge[0] = MAX_BUBBLE_AGE - 1;
	bubbleChange = -9;
    }
    if (bubbleAge[0] <= -1)
    {
	bubbleAge[0] = 0;
	bubbleChange = 9;
    }
}

void
BubbleElement::fini ()
{
}

bool
BubbleScreen::toggle (CompAction         *action,
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
	    o.value ().set ("bubbles");

	    newOptions.push_back (o);

	    opt.value ().action ().initiate () (action, state, newOptions);

	    break;
	}
    }

    return false;
}


void
BubbleScreen::autostartChanged (CompOption *opt, BubblesOptions::Options num)
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->setAnimationRunning ("bubbles", optionGetAutostart ());
}

BubbleScreen::BubbleScreen (CompScreen *screen) :
    PluginClassHandler <BubbleScreen, CompScreen> (screen)
{
    type = ElementType::create ("bubbles", "Bubble", BubbleElement::create);

    optionSetToggleBubbleInitiate (BubbleScreen::toggle);
    optionSetAutostartNotify (boost::bind (&BubbleScreen::autostartChanged, this,
					   boost::placeholders::_1,
					   boost::placeholders::_2));

    if (!type)
	setFailed ();
    else if (optionGetAutostart ())
    {
	ElementScreen *es = ElementScreen::get (screen);
	if (es)
	    es->startAnimation ("bubbles");
    }
}

BubbleScreen::~BubbleScreen ()
{
    ElementScreen *es = ElementScreen::get (screen);
    if (es)
	es->stopAnimation ("bubbles");

    type->destroy ();
}

bool
BubblePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("elements", COMPIZ_ELEMENTS_ABI))
	return false;

    return true;
}
