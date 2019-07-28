#ifndef _GROUPDRAWINLINE_H
#define _GROUPDRAWINLINE_H

int drawDefInternal( GroupEnt *groupEnt, Mat4 parent_mat_p, DefTracker *tracker, VisType vis, DrawParams *draw, int uid );
int drawDefInternal_shadowmap( GroupEnt *groupEnt, Mat4 parent_mat_p, DefTracker *tracker, VisType vis, DrawParams *draw, int uid );

#endif

