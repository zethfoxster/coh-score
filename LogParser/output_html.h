#ifndef _OUTPUT_HTML_H
#define _OUTPUT_HTML_H

#include "parse_util.h"
#include <stdio.h>


void OutputHTML(const char * webDir, const char * postProcessScript, const char * mapImageDir);
void OutputConnectionStats(FILE * file);
void OutputPlayerStats(FILE * file);
void OutputMissionStats(FILE * file);
void OutputCityZones(FILE * file);
void OutputSpecificCityZone(FILE * file, char * zoneName);
void OutputPlayerCategoryStats(FILE * file);
void OutputActionCounts(FILE * file);
void OutputDailyStats(FILE * file);
void OutputInfluenceStats(FILE * file);
void OutputContactStats(FILE * file);


#endif  // _OUTPUT_HTML_H