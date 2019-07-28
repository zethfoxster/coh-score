#ifndef TTDEBUG_H
#define TTDEBUG_H

void ttDebugEnable(int enable);
int ttDebugEnabled();

void ttDebug();


typedef struct TTDebugText TTDebugText;
typedef struct TTDrawContext TTDrawContext;

TTDebugText* ttDebugTextCreate(TTDrawContext* font, float xScale, float yScale, char* text);
void ttDebugTextDestroy(TTDebugText* obj);
void ttDebugTextAlterScale(TTDebugText* obj, float xScale, float yScale);

#endif