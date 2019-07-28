#ifndef UIRAIDRESULT_H
#define UIRAIDRESULT_H

typedef struct Packet Packet;

void uiRaidResultReceive(Packet * pak);
int uiRaidResultDisplay();

#endif