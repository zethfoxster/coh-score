#ifndef UIBODY_H
#define UIBODY_H

extern int gFakeArachnosSoldier;
extern int gFakeArachnosWidow;

void resetBodyMenu();
void bodyMenu();

void bodyMenuEnterCode(void);
void bodyMenuExitCode(void);
void drawScaleSliders(F32 x, F32 y, F32 z, F32 sc, F32 wd, F32 sliderWd );
void drawFaceSlider(F32 x, F32 y, F32 z, F32 sc, F32 wd, int part);
void genderChangeMenuStart();
int bodyCanBeEdited(int redirect);

#endif