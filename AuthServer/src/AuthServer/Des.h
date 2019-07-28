//
// Des
//
// $Revision: 1 $
// $Date: 01-11-06 11:27p $
// $Author: Anteros $

#pragma once

#ifdef __cplusplus	// for avoiding c++ identifier decoration
extern "C"
{
#endif

void DesKeyInit(char *password);
void DesWriteBlock(void *buf, int len);
void DesReadBlock(void *buf, int len);

#ifdef __cplusplus	// for avoiding c++ identifier decoration
}
#endif
