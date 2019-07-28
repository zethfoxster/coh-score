#include "wininclude.h"
#include <assert.h>

#include "dd.h"
#include "error.h"
#include "mathutil.h"
#include "tex.h"


// Apply the fade to a 32-bit or 24-bit uncompressed texture
static void GETTEX_applyFadeUncomressed(int width, int height, int num_mips, int bpp, unsigned char *data, F32 close_mix, F32 far_mix)
{
	int		w,h,i,numPixels,level;
	F32		ratio, prev_ratio = 1.0f, grey = 128;
	int 	bytes_per_pixel;

	// This code currently only supports 32-bit RGBA or 24-bit RGB
	assert(bpp == 32 || bpp == 24);

	w = width;
	h = height;
	bytes_per_pixel = bpp / 8;
	for(level=0;level < num_mips;level++)
	{
		ratio = close_mix - (SQR(level) * (1.0f-far_mix) / SQR(num_mips));
		ratio = MINMAX(ratio,0,1) * prev_ratio;

		numPixels = w * h;

		for(i=0;i<numPixels;i++)
		{
			unsigned char r, g, b;

			// This may actually be BGR instead of RGB, but doesn't matter for our purposes.
			r = data[0];
			g = data[1];
			b = data[2];

			r = qtrunc(r * ratio + grey * (1-ratio));
			g = qtrunc(g * ratio + grey * (1-ratio));
			b = qtrunc(b * ratio + grey * (1-ratio));

			data[0] = r;
			data[1] = g;
			data[2] = b;

			data += bytes_per_pixel;
		}

		w >>= 1;
		h >>= 1;
		prev_ratio = ratio;

		if (w == 0)
			w = 1;
		if (h == 0)
			h = 1;
	}
}

// Apply the fade to a DXT1/DXT3/DXT5 compressed texture
static void GETTEX_applyFadeDXT(int width, int height, int num_mips, unsigned char *data, bool isDXT1, F32 close_mix, F32 far_mix)
{
	int		w,h,i,numBlocks,level;
	F32		ratio, prev_ratio = 1.0f, grey = 128;
	int		alphaBlockSize = isDXT1 ? 0 : 8;

	w = width;
	h = height;
	for(level=0;level < num_mips;level++)
	{
		ratio = close_mix - (SQR(level) * (1.0f-far_mix) / SQR(num_mips));
		ratio = MINMAX(ratio,0,1) * prev_ratio;

		numBlocks = ((w + 3)>>2) * ((h + 3)>>2);

		for(i=0;i<numBlocks;i++)
		{
			unsigned short r, g, b;
			unsigned short c1, c2;
			bool c1Biggerc2;

			// Skip over alpha data
			data += alphaBlockSize;

			// Retrieve the two 16-bit colors
			c1 = ((unsigned short *)data)[0];
			c2 = ((unsigned short *)data)[1];

			// Remember if c1 > c2.  We don't want to change this after the fade
			c1Biggerc2 = c1 > c2;

			// Unpack the first color
			r = (c1 & 0xF800) >> 8;
			g = (c1 & 0x07E0) >> 3;
			b = (c1 & 0x001F) << 3;

			// copy the upper bits into the lower bits
			r |= r >> 5;
			g |= g >> 6;
			b |= b >> 5;

			// Apply fade
			r = qtrunc(r * ratio + grey * (1-ratio));
			g = qtrunc(g * ratio + grey * (1-ratio));
			b = qtrunc(b * ratio + grey * (1-ratio));

			// Repack the color
			c1 = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F);

			// Unpack the second color
			r = (c2 & 0xF800) >> 8;
			g = (c2 & 0x07E0) >> 3;
			b = (c2 & 0x001F) << 3;

			// copy the upper bits into the lower bits
			r |= r >> 5;
			g |= g >> 6;
			b |= b >> 5;

			// Apply fade
			r = qtrunc(r * ratio + grey * (1-ratio));
			g = qtrunc(g * ratio + grey * (1-ratio));
			b = qtrunc(b * ratio + grey * (1-ratio));

			// Repack the color
			c2 = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F);

			// If the two colors become equal
			if (c1 == c2)
			{
				if (c1Biggerc2)
				{
					// No transparency, so set all the indices to 0
					((unsigned int *)data)[1] = 0;
				}
				else
				{
					// Need to preserve 1-bit alpha
					// The indices in this case should be ok
				}
			}
			// See if we need to swap c1 and c2 to preserve the original condition
			else if ((c1Biggerc2 && (c2 > c1)) || (!c1Biggerc2 && (c1 > c2)))
			{
				unsigned short temp = c1;
				unsigned int old_indices, new_indices = 0;
				int j;

				c1 = c2;
				c2 = temp;
				
				// Need to fixup indices
				old_indices = ((unsigned int *)data)[1];

				for (j = 0; j < 16; j++)
				{
					new_indices >>= 2;

					if (c1Biggerc2)
					{
						// No transparency.
						// So, map:
						//   0 -> 1
						//   1 -> 0
						//   2 -> 3
						//   3 -> 2
						switch(old_indices & 3)
						{
							case 0:
								new_indices |= 0x40000000; // 1 in the top 4 bits
								break;
							case 1:
								new_indices |= 0x00000000; // 0 in the top 4 bits
								break;
							case 2:
								new_indices |= 0xC0000000; // 3 in the top 4 bits
								break;
							case 3:
								new_indices |= 0x80000000; // 2 in the top 4 bits
								break;
						}
					}
					else
					{
						// So, map:
						//   0 -> 1
						//   1 -> 0
						//   2 -> 2
						//   3 -> 3
						switch(old_indices & 3)
						{
							case 0:
								new_indices |= 0x40000000; // 1 in the top 4 bits
								break;
							case 1:
								new_indices |= 0x00000000; // 0 in the top 4 bits
								break;
							case 2:
								new_indices |= 0x80000000; // 2 in the top 4 bits
								break;
							case 3:
								new_indices |= 0xC0000000; // 3 in the top 4 bits
								break;
						}
					}

					old_indices >>= 2;
				}

				((unsigned int *)data)[1] = new_indices;
			}

			// Write the modified colors
			((unsigned short *)data)[0] = c1;
			((unsigned short *)data)[1] = c2;

			data += 8;
		}

		w >>= 1;
		h >>= 1;
		prev_ratio = ratio;

		if (w == 0)
			w = 1;
		if (h == 0)
			h = 1;
	}
}

// Generic interface to apply the fade to a dds texture
// (either uncompressed 32-bit or 24-bit or compressed DXT1/DXT3/DXT5)
int GETTEX_applyFade(unsigned char *dds_raw,F32 close_mix, F32 far_mix)
{
	unsigned char *data;
	DDSURFACEDESC2 *dds_header;
	int		width, height, bpp;
	int		num_mips;

	dds_header = (DDSURFACEDESC2 *)(dds_raw + 4);  // skip "DDS "

	width = dds_header->dwWidth;
	height = dds_header->dwHeight;
	data = dds_raw + 4 + sizeof(DDSURFACEDESC2);

	bpp = dds_header->ddpfPixelFormat.dwRGBBitCount;
	num_mips = dds_header->dwMipMapCount;

	switch(dds_header->ddpfPixelFormat.dwFourCC)
	{
		case FOURCC_DXT1:
			GETTEX_applyFadeDXT(width, height, num_mips, data, true, close_mix, far_mix);
			break;

		case FOURCC_DXT3:
		case FOURCC_DXT5:
			GETTEX_applyFadeDXT(width, height, num_mips, data, false, close_mix, far_mix);
			break;

		default:
			if (dds_header->ddpfPixelFormat.dwFlags == DDS_RGBA && bpp == 32 && dds_header->ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000)
				GETTEX_applyFadeUncomressed(width, height, num_mips, bpp, data, close_mix, far_mix);
			else if (dds_header->ddpfPixelFormat.dwFlags == DDS_RGB  && bpp == 24)
				GETTEX_applyFadeUncomressed(width, height, num_mips, bpp, data, close_mix, far_mix);
			else
				Errorf("\n\nGetTex logic error!  Fade ignored and not applied!\n\n");	
	}

	return 0;
}
