#ifndef RENDERSSAO_H
#define RENDERSSAO_H

enum ssaoQuality {
	ssaoOff = 0,
	ssaoFastest,
	ssaoLow,
	ssaoMedium,
	ssaoHigh,
	ssaoUltra,
	ssaoTunable,
};

void rdrAmbientStart();
void rdrAmbientDeInit();
void rdrAmbientOcclusion();
void rdrAmbientUpdate();
void rdrAmbientDebug();
void rdrAmbientDebugInput();

#endif
