#ifndef UIAMOUNTSLIDER_H
#define UIAMOUNTSLIDER_H

typedef struct AtlasTex AtlasTex;

typedef void (*AmountSliderOKCallback)(int amount, void *userData);
typedef void (*AmountSliderDrawCallback)(void *userData, float x, float y, float z, float sc, int clr);

// if there is already an amount slider window displaying, this function returns 0, otherwise it returns 1
int amountSliderWindowSetupAndDisplay(int min, int max, const char *text, AmountSliderDrawCallback drawCallback, AmountSliderOKCallback okCallback, void *userData, int dontLimit); 

int amountSliderWindow(void);

#endif