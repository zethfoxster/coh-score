#include <stdio.h>
//#include "tga.h"
#include <string.h>
#include <stdlib.h>
#include "tex.h"

static int 
dithmat[4][4] = {       0,  8,  2, 10, 
                           12,  4, 14,  6, 
                            3, 11,  1,  9, 
                           15,  7, 13,  5 };

// for error diffusion.
static int      errR[2048], errG[2048], errB[2048];     

U16 quantARGB4444_trunc(unsigned long argb, int x, int y, int w)
{
    return (
                    ((argb >> 12) & 0x0F00) |
                        ((argb >>  8) & 0x00F0) |
                        ((argb >>  4) & 0x000F) |       
                        ((argb >> 16) & 0xF000) );
}

U16 quantARGB4444_4x4 (unsigned long argb, int x, int y, int w)
{
    int d = dithmat[y&3][x&3];
    int n, t;

    n = (int) (((argb >> 16) & 0xFF) * 0xF0/255.0f + 0.5f) + d; 
    t = (n>>4)<<8;
    n = (int) (((argb >>  8) & 0xFF) * 0xF0/255.0f + 0.5f) + d; 
    t |= (n>>4)<<4;
    n = (int) (((argb      ) & 0xFF) * 0xF0/255.0f + 0.5f) + d; 
    t |= (n>>4)<<0;
    t |= (argb >> 16) & 0xF000;
    return t & 0xFFFF;
}

U16 quantARGB4444_fs(unsigned long argb, int x, int y, int w)
{
    static int          qr, qg, qb;             // quantized incoming values.
    int                         ir, ig, ib;             // incoming values.
    int                         t;

    ir = (argb >> 16) & 0xFF;   // incoming pixel values.
    ig = (argb >>  8) & 0xFF;
    ib = (argb      ) & 0xFF;

    if (x == 0) qr = qg = qb = 0;

    ir += errR[x] + qr;
    ig += errG[x] + qg;
    ib += errB[x] + qb;

    qr = ir;            // quantized pixel values. 
    qg = ig;            // qR is error from pixel to left, errR is
    qb = ib;            // error from pixel to the top & top left.

    if (qr < 0) qr = 0; if (qr > 255) qr = 255;         // clamp.
    if (qg < 0) qg = 0; if (qg > 255) qg = 255;
    if (qb < 0) qb = 0; if (qb > 255) qb = 255;

    // To RGB565.
    qr = (int) (qr * 0xFFF/255.0f);     qr >>= 8;
    qg = (int) (qg * 0xFFF/255.0f);     qg >>= 8;
    qb = (int) (qb * 0xFFF/255.0f);     qb >>= 8;

    t  = (qr <<  8) | (qg << 4) | qb;   // this is the value to be returned.
    t |= (argb >> 16) & 0xF000;

    // Now dequantize the input, and compute & distribute the errors.
    qr = (qr << 4) | (qr >> 0);
    qg = (qg << 4) | (qg >> 0);
    qb = (qb << 4) | (qb >> 0);
    qr = ir - qr;
    qg = ig - qg;
    qb = ib - qb;

    // 3/8 (=0.375) to the EAST, 3/8 to the SOUTH, 1/4 (0.25) to the SOUTH-EAST.
    errR[x]  = ((x == 0) ? 0 : errR[x]) + ((int) (qr * 0.375f));
    errG[x]  = ((x == 0) ? 0 : errG[x]) + ((int) (qg * 0.375f));
    errB[x]  = ((x == 0) ? 0 : errB[x]) + ((int) (qb * 0.375f));

    errR[x+1] = (int) (qr * 0.250f);
    errG[x+1] = (int) (qg * 0.250f);
    errB[x+1] = (int) (qb * 0.250f);

    qr = (int) (qr * 0.375f);           // Carried to the pixel on the right.
    qg = (int) (qg * 0.375f);
    qb = (int) (qb * 0.375f);

    return t & 0xFFFF;
}


U16 quantRGB565_trunc(unsigned long argb, int x, int y, int w)
{
    return (((argb >> 8) & 0xF800) |
            ((argb >> 5) & 0x07E0) 
            |((argb >> 3) & 0x001F)          );
}


U16 quantRGB565_4x4(U32 argb, int x, int y, int w)
{
    int d = dithmat[y&3][x&3];
    int n, t;

    n = (int) (((argb >> 16) & 0xFF) * 0x1F0/255.0f + 0.5f) + d;        
    t = (n>>4)<<11;
    n = (int) (((argb >>  8) & 0xFF) * 0x3F0/255.0f + 0.5f) + d;        
    t |= (n>>4)<<5;
    n = (int) (((argb      ) & 0xFF) * 0x1F0/255.0f + 0.5f) + d;        
    t |= (n>>4)<<0;
    return t & 0xFFFF;
}

U16 quantRGB565_fs(U32 argb, int x, int y, int w)
{
    static int          qr, qg, qb;             // quantized incoming values.
    int                         ir, ig, ib;             // incoming values.
    int                         t;

    ir = (argb >> 16) & 0xFF;   // incoming pixel values.
    ig = (argb >>  8) & 0xFF;
    ib = (argb      ) & 0xFF;

    if (x == 0) qr = qg = qb = 0;

    ir += errR[x] + qr;
    ig += errG[x] + qg;
    ib += errB[x] + qb;

    qr = ir;            // quantized pixel values. 
    qg = ig;            // qR is error from pixel to left, errR is
    qb = ib;            // error from pixel to the top & top left.

    if (qr < 0) qr = 0; if (qr > 255) qr = 255;         // clamp.
    if (qg < 0) qg = 0; if (qg > 255) qg = 255;
    if (qb < 0) qb = 0; if (qb > 255) qb = 255;

    // To RGB565.
    qr = (int) (qr * 0x1FFF/255.0f);    qr >>= 8;
    qg = (int) (qg * 0x3FFF/255.0f);    qg >>= 8;
    qb = (int) (qb * 0x1FFF/255.0f);    qb >>= 8;

    t  = (qr << 11) | (qg << 5) | qb;   // this is the value to be returned.

    // Now dequantize the input, and compute & distribute the errors.
    qr = (qr << 3) | (qr >> 2);
    qg = (qg << 2) | (qg >> 4);
    qb = (qb << 3) | (qb >> 2);
    qr = ir - qr;
    qg = ig - qg;
    qb = ib - qb;

    // 3/8 (=0.375) to the EAST, 3/8 to the SOUTH, 1/4 (0.25) to the SOUTH-EAST.
    errR[x]  = ((x == 0) ? 0 : errR[x]) + ((int) (qr * 0.375f));
    errG[x]  = ((x == 0) ? 0 : errG[x]) + ((int) (qg * 0.375f));
    errB[x]  = ((x == 0) ? 0 : errB[x]) + ((int) (qb * 0.375f));

    errR[x+1] = (int) (qr * 0.250f);
    errG[x+1] = (int) (qg * 0.250f);
    errB[x+1] = (int) (qb * 0.250f);

    qr = (int) (qr * 0.375f);           // Carried to the pixel on the right.
    qg = (int) (qg * 0.375f);
    qb = (int) (qb * 0.375f);

    return t & 0xFFFF;
}


void quantTex(void *data,int w,int h)
{
int		x,y,i,r,g,b,a;
U32		*src;
U16		*dst16_data,*dst16;

	dst16_data = calloc(w * 2,h);
	memset(errR,0,sizeof(errR));
	memset(errG,0,sizeof(errG));
	memset(errB,0,sizeof(errB));
	src = data;
	dst16 = dst16_data;
    for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			*dst16++ = quantRGB565_4x4(*src, x, y, w);
			src++;
		}
    }
	src = data;
	dst16 = dst16_data;
	for(i=0;i<w*h;i++,src++,dst16++)
	{
#if 1
		r = ((*dst16) >> 11) << 3;
		g = (((*dst16) >> 5) & 63) << 2;
		b = ((*dst16) & 31) << 3;
		a = (*src) >> 24;
#else
		r = ((*dst16) >> 8) << 4;
		g = (((*dst16) >> 4) & 15) << 4;
		b = ((*dst16) & 15) << 4;
		a = (((*dst16) >> 12) & 15) << 4;
#endif
		*src = (a << 24) | (r << 16) | (g << 8) | (b);
	}
	free(dst16_data);
}

