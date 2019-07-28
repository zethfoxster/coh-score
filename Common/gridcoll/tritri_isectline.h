#ifndef _TRITRI_ISECTLINE_H
#define _TRITRI_ISECTLINE_H

int tri_tri_intersect_with_isectline(float V0[3],float V1[3],float V2[3], float U0[3],float U1[3],float U2[3],int *coplanar, float isectpt1[3],float isectpt2[3]);
int tri_tri_overlap_test_2d(float p1[2], float q1[2], float r1[2], float p2[2], float q2[2], float r2[2]);

#endif
