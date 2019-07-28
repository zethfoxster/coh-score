char * cape_text = 
"LOD 0 0 2\n" \
" \n" \
"Width = 9 \n" \
"Height = 9 \n" \
"Origin = -1.0, 0.0, 0.0 \n" \
"XScale = 2.5 \n" \
"Circle = 1 \n" \
"Grid = .25 0.0 -.40 \n" \
"Color = 1, .1, .1 \n" \
"ColRad = 0.20 \n" \
"Drag = 0.1\n" \
" \n" \
"#Diagonal 3 \n" \
"Constrain 2 .7 \n" \
" \n" \
"Attach 0 0,0 \n" \
"Attach 1 1,0 \n" \
"Attach 2 2,0 \n" \
"Attach 3 3,0 \n" \
"Attach 4 4,0 \n" \
"Attach 5 5,0 \n" \
"Attach 6 6,0 \n" \
"Attach 7 7,0 \n" \
"Attach 8 8,0 \n" \
" \n" \
"#Weight 0,8 2.0 \n" \
"#Weight 1,8 1.5 \n" \
"#Weight 2,8 1.5 \n" \
"#Weight 3,8 1.5 \n" \
"#Weight 4,8 1.5 \n" \
"#Weight 5,8 1.5 \n" \
"#Weight 6,8 1.5 \n" \
"#Weight 7,8 1.5 \n" \
"#Weight 8,8 2.0 \n" \
" \n" \
"LOD 1 2 -1\n" \
" \n" \
"Width = 5 \n" \
"Height = 5 \n" \
"Origin = -1.0, 0.0, 0.0 \n" \
"XScale = 2.0 \n" \
"Circle = 1 \n" \
"Grid = .5 0.0 -0.80 \n" \
"Color = 1, .1, .1 \n" \
"ColRad = 0.1 \n" \
"Drag = 0.1\n" \
" \n" \
"Constrain 2 .5 \n" \
"Iterate 2\n" \
" \n" \
"Attach 0 0,0 \n" \
"Attach 2 1,0 \n" \
"Attach 4 2,0 \n" \
"Attach 6 3,0 \n" \
"Attach 8 4,0 \n" \
" \n" \
"LOD 2 3 -1\n" \
" \n" \
"Width = 3 \n" \
"Height = 3 \n" \
"Origin = -1.0, 0.0, 0.0 \n" \
"XScale = 1.5 \n" \
"Circle = 1 \n" \
"Grid = 1.0 0.0 -1.6 \n" \
"Color = 1, .1, .1 \n" \
"ColRad = 0.1 \n" \
"Drag = 0.1\n" \
" \n" \
"Constrain 2 .7 \n" \
"Iterate 2\n" \
" \n" \
"Attach 0 0,0 \n" \
"Attach 4 1,0 \n" \
"Attach 8 2,0 \n" \
;

#define CAPE_NUMHOOKS 9
#define tDX  0.75f
#define tDY  0.50f
#define tDZ -0.50f
Vec3 cape_hook_offsets[CAPE_NUMHOOKS] =
{
	{ -1.00f*tDX, tDY, tDZ },
	{ -0.75f*tDX, tDY, tDZ },
	{ -0.50f*tDX, tDY, tDZ },
	{ -0.25f*tDX, tDY, tDZ },
	{  0.00f*tDX, tDY, tDZ },
	{  0.25f*tDX, tDY, tDZ },
	{  0.50f*tDX, tDY, tDZ },
	{  0.75f*tDX, tDY, tDZ },
	{  1.00f*tDX, tDY, tDZ },
};
#undef tDX
#undef tDY
#undef tDZ
Vec3 cape_hook_curpos[CAPE_NUMHOOKS];

