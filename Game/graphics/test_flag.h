char * flag_text = 
"LOD 0 0 -1\n" \
" \n" \
"Width = 9 \n" \
"Height = 9 \n" \
"Origin = 0.0, 0.0, 0.0 \n" \
"XScale = 1 \n" \
"Circle = 0 \n" \
"Grid = 0.0 0.25 -.40 \n" \
"Color = 1, .1, .1 \n" \
"ColRad = 0.2 \n" \
" \n" \
"Diagonal 3 \n" \
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
;

#define FLAG_NUMHOOKS 9
#define tDX  0.0f
#define tOY  3.0f
#define tDY  1.0f
#define tDZ  0.0f
Vec3 flag_hook_offsets[FLAG_NUMHOOKS] =
{
	{ tDX, tOY + -1.00f*tDY, tDZ },
	{ tDX, tOY + -0.75f*tDY, tDZ },
	{ tDX, tOY + -0.50f*tDY, tDZ },
	{ tDX, tOY + -0.25f*tDY, tDZ },
	{ tDX, tOY +  0.00f*tDY, tDZ },
	{ tDX, tOY +  0.25f*tDY, tDZ },
	{ tDX, tOY +  0.50f*tDY, tDZ },
	{ tDX, tOY +  0.75f*tDY, tDZ },
	{ tDX, tOY +  1.00f*tDY, tDZ },
};
#undef tDX
#undef tOY
#undef tDY
#undef tDZ
Vec3 flag_hook_curpos[FLAG_NUMHOOKS];

