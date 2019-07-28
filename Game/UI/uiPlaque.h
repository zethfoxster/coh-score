#ifndef UIPLAQUE_H
#define UIPLAQUE_H

typedef enum INFO_BOX_MSGS;

int plaqueShowDialog(void);
void plaquePopup(char* str, INFO_BOX_MSGS spam_chat);
void plaqueClearQueue(void);

#endif 
