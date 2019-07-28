#ifndef _WIND_H
#define _WIND_H

void ClothWindSetDir(Vec3 dir, F32 magnitude);
F32 ClothWindGetRandom(Vec3 wind, F32 *windScale, F32 *windDirScale);

void ClothWindSetRipplePeriod(F32 period);
void ClothWindSetRippleSlope(F32 slope);

#endif // _WIND_H


