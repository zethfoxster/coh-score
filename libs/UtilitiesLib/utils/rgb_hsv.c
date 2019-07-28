#include "rgb_hsv.h"

#define RETURN_HWB(h, w, b, success) {HWB[0] = 60*(h); HWB[1] = w; HWB[2] = b; return success;} 
#define RETURN_HSV(h, w, b, success) {HSV[0] = 60*(h); HSV[1] = w; HSV[2] = b; return success;} 
#define RETURN_RGB(r, g, b, success) {RGB[0] = r; RGB[1] = g; RGB[2] = b; return success;} 

#define UNDEFINED (-1.f/60)


// Theoretically, hue 0 (pure red) is identical to hue 6 in these transforms. Pure 
// red always maps to 6 in this implementation. Therefore UNDEFINED can be 
// defined as 0 in situations where only unsigned numbers are desired.

//when using the functions, Hue is passed in and returned on a scale of [0-360],
//though in the code it is scaled to [0-6]

int rgbToHwb(Vec3 RGB,Vec3 HWB)
{
	// RGB are each on [0, 1]. W and B are returned on [0, 1] and H is  
	// returned on [0, 360]. Exception: H is returned UNDEFINED if W == 1 - B.  
	float	R = RGB[0], G = RGB[1], B = RGB[2], w, v, b, f;  
	int		i;  

	if (R < G)  { w = R;	v = G; }
	else		{ w = G;	v = R; }
	w = MIN(MIN(R,G),B);
	v = MAX(MAX(R,G),B);
	b = 1 - v;  
	if (v == w) RETURN_HWB(UNDEFINED, w, b, 0);  
	f = (R == w) ? G - B : ((G == w) ? B - R : R - G);  
	i = (R == w) ? 3 : ((G == w) ? 5 : 1);  
	RETURN_HWB(i - f /(v - w), w, b, 1);  
}

int hwbToRgb(Vec3 HWB,Vec3 RGB) 
{
	// H is given on [0, 360] or UNDEFINED. W and B are given on [0, 1].  
	// RGB are each returned on [0, 1].  
	float	h = HWB[0] * 1.f/60.f, w = HWB[1], b = HWB[2], v, n, f;  
	int		i;  

	v = 1 - b;  
	if (h == UNDEFINED) RETURN_RGB(v, v, v, 0);  
	i = floor(h);  
	f = h - i;  
	if (i & 1) f = 1 - f; // if i is odd  
	n = w + f * (v - w); // linear interpolation between w and v  
	switch (i)
	{  
		case 6:  
		case 0: RETURN_RGB(v, n, w, 1);  
		case 1: RETURN_RGB(n, v, w, 1);  
		case 2: RETURN_RGB(w, v, n, 1);  
		case 3: RETURN_RGB(w, n, v, 1);  
		case 4: RETURN_RGB(n, w, v, 1);  
		case 5: RETURN_RGB(v, w, n, 1);  
	}
	RETURN_RGB(v, v, v, 0);
}

int rgbToHsv(const Vec3 RGB, Vec3 HSV)
{ 
	// RGB are each on [0, 1]. S and V are returned on [0, 1] and H is  
	// returned on [0, 360]. Exception: H is returned UNDEFINED if S==0.  
	float R = RGB[0], G = RGB[1], B = RGB[2], v, x, f;  
	int i;  

	x = MIN(MIN(R,G),B);
	v = MAX(MAX(R,G),B);
	if(v == x) RETURN_HSV(UNDEFINED, 0, v, 0);  
	f = (R == x) ? G - B : ((G == x) ? B - R : R - G);  
	i = (R == x) ? 3 : ((G == x) ? 5 : 1);
	RETURN_HSV(i - f /(v - x), (v - x)/v, v, 1);  
}

int hsvToRgb(const Vec3 HSV, Vec3 RGB)
{ 

	// H is given on [0, 360] or UNDEFINED. S and V are given on [0, 1].  
	// RGB are each returned on [0, 1].  
	float h = HSV[0] / 60.f, s = HSV[1], v = HSV[2], m, n, f;  
	int i;  

	if(h == UNDEFINED) RETURN_RGB(v, v, v, 0);  
	i = floor(h);  
	f = h - i;  
	if(!(i & 1)) f = 1 - f; // if i is even  
	m = v * (1 - s);  
	n = v * (1 - s * f);  
	switch (i)
	{  
		case 6:  
		case 0: RETURN_RGB(v, n, m, 1);  
		case 1: RETURN_RGB(n, v, m, 1); 
		case 2: RETURN_RGB(m, v, n, 1); 
		case 3: RETURN_RGB(m, n, v, 1); 
		case 4: RETURN_RGB(n, m, v, 1); 
		case 5: RETURN_RGB(v, m, n, 1); 
	}  
	RETURN_RGB(v, v, v, 0);  
} 


void hsvAdd(const Vec3 hsv_a,const Vec3 hsv_b,Vec3 hsv_out)
{
	hsv_out[0] = fmod(hsv_a[0] + hsv_b[0] + 3600,360);
	hsv_out[1] = MINMAX(hsv_a[1] + hsv_b[1],0,1);
	hsv_out[2] = MINMAX(hsv_a[2] + hsv_b[2],0,1);
}

// Shifts the hue, scales the Saturation and Value
void hsvShiftScale(const Vec3 hsv_a,const Vec3 hsv_b,Vec3 hsv_out)
{
	if (hsv_a[0]==UNDEFINED || hsv_b[0]==UNDEFINED) {
		hsv_out[0] = UNDEFINED;
	} else {
		hsv_out[0] = fmod(hsv_a[0] + hsv_b[0] + 3600,360);
	}
	hsv_out[1] = MINMAX(hsv_a[1] * hsv_b[1],0,1);
	hsv_out[2] = MINMAX(hsv_a[2] * hsv_b[2],0,1);
}

