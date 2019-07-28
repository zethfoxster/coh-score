#ifndef _PLOT_UTIL_H
#define _PLOT_UTIL_H


#include "Statistic.h"


void OpenPlotCommandFile(const char * dir);
void ClosePlotCommandFile();

void PLOT_NEW(const char * title, const char * localFolder, const char * filename);
void PLOT_END_LINE();
void PLOT_ENTRY(const char * entry);
void PLOT_LINE_ENTRY(const char * entry);
void PLOT_END();

void PLOT_STATISTIC_HEADER(const char * col);
void PLOT_STATISTIC_HEADER_ROW(const char * col0, const char * col1);
void PLOT_STATISTIC_ROW(const char * title, const Statistic * pStat);
void PLOT_STATISTIC(const Statistic * pStat);
void PLOT_TIME_STATISTIC(const Statistic * pStat);





#endif  // _PLOT_UTIL_H