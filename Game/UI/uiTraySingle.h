#ifndef UITRAYSINGLE_H
#define UITRAYSINDLE_H

typedef struct CBox CBox;

int trayWindowSingle();
int trayWindowSingleRazer(void);
void traySingle_getDims( int wdw, float * xout, float * yout, float *wdout, float *htout, int *typeout, CBox *box  );
void showNewTray(void);

#endif
