#ifndef SERVERERROR_H
#define SERVERERROR_H

void serverErrorfSetNeverShowDialog();
void serverErrorfSetForceShowDialog();
void serverErrorfCallback(char* errMsg);

char *errorGetQueued();

#endif