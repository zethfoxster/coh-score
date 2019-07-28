#ifndef _RGB_HSV_H
#define _RGB_HSV_H

#include "mathutil.h"

int rgbToHsv(const Vec3 RGB,Vec3 HSV);
int hsvToRgb(const Vec3 HSV,Vec3 RGB);
void hsvAdd(const Vec3 hsv_a,const Vec3 hsv_b,Vec3 hsv_out);
void hsvShiftScale(const Vec3 hsv_a,const Vec3 hsv_b,Vec3 hsv_out); // Shifts the hue, scales the Saturation and Value

#endif
