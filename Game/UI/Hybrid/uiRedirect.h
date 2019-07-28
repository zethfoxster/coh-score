
#ifndef UI_REDIRECT_H
#define UI_REDIRECT_H


void redirectMenu();
void updateRedirect();
int redirectedFromMenu(int menu);


extern int g_redirectedFrom;

typedef enum BlinkRegion
{
	 BLINK_HEROPRAETORIAN,
	 BLINK_ORIGIN,
	 BLINK_ARCHETYPE,
	 BLINK_PRIMARYPOWER,
	 BLINK_SECONDARYPOWER,
	 BLINK_POWERSELECTION,
}BlinkRegion;
F32 getRedirectBlinkScale(int blinkRegion);

typedef int (*RedirectCallback)(int redirect);
void addRedirect(int menu, char * text, int blink_region, RedirectCallback fixedCallback );	
void clearRedirects();

void redirectMenuTrigger(int region);

#endif