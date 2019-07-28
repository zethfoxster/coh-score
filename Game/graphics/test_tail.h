char * tail_text = 
"LOD 0 0 2\n" \
" \n" \
"Width = 3 \n" \
"Height = 8 \n" \
"Origin = -0.20, 0.0, 0.0 \n" \
"XScale = 1.0 \n" \
"Circle = 0 \n" \
"Grid = .20 -0.20 0.0 \n" \
"Color = 1, .1, .1 \n" \
"ColRad = 0.2 \n" \
"Drag = 0.1\n" \
" \n" \
"#Diagonal 3 \n" \
"Iterate 4 \n" \
" \n" \
"Set 0,2 -.20 -0.60, -0.20 \n" \
"Set 1,2 0.00 -0.60, -0.20 \n" \
"Set 2,2 0.20 -0.60, -0.20 \n" \
"Set 0,3 -.20 -1.00, -0.60 \n" \
"Set 1,3 0.00 -1.00, -0.60 \n" \
"Set 2,3 0.20 -1.00, -0.60 \n" \
"Set 0,4 -.20 -1.20, -1.00 \n" \
"Set 1,4 0.00 -1.20, -1.00 \n" \
"Set 2,4 0.20 -1.20, -1.00 \n" \
"Set 0,5 -.20 -1.10, -1.40 \n" \
"Set 1,5 0.00 -1.10, -1.40 \n" \
"Set 2,5 0.20 -1.10, -1.40 \n" \
"Set 0,6 -.20 -1.00, -1.80 \n" \
"Set 1,6 0.00 -1.00, -1.80 \n" \
"Set 2,6 0.20 -1.00, -1.80 \n" \
"Set 0,7 -.20 -0.60, -2.00 \n" \
"Set 1,7 0.00 -0.60, -2.00 \n" \
"Set 2,7 0.20 -0.60, -2.00 \n" \
" \n" \
"Constrain 3 -1 -1 \n" \
" \n" \
"Attach 0 0,0 \n" \
"Attach 1 1,0 \n" \
"Attach 2 2,0 \n" \
"Attach 3 0,1 \n" \
"Attach 4 1,1 \n" \
"Attach 5 2,1 \n" \
" \n" \
;

#define TAIL_NUMHOOKS 6
Vec3 tail_hook_offsets[TAIL_NUMHOOKS] =
{
	{ -0.20f, -1.0f, -0.2f },
	{  0.00f, -1.0f, -0.2f },
	{  0.20f, -1.0f, -0.2f },
	{ -0.20f, -1.20f, -0.2f },
	{  0.00f, -1.20f, -0.2f },
	{  0.20f, -1.20f, -0.2f },
};
Vec3 tail_hook_curpos[TAIL_NUMHOOKS];

