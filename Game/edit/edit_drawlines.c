/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "edit_drawlines.h"
#include "model.h"
#include "rendermodel.h"
#include "mathutil.h"
#include "camera.h"
#include "edit_info.h"
#include "entclient.h"
#include "edit_cmd.h"
#include "edit_select.h"
#include "entity.h"
#include "cmdcommon.h"
#include "uiGame.h"
#include "cmdgame.h"
#include "renderprim.h"
#include "grouptrack.h"
#include "gfxwindow.h"
#include "renderstats.h"

Mat4 globalmatx;
int globaledit_axisp[3];
int globaledit_axis[3];
int globalstyle;
int globalshowGrid;

void editProfiler()
{
}

void drawLineCheckOrient(const Mat3 orient_mat,const Vec3 base_pos,const Vec3 la,const Vec3 lb,const U8 c[4],F32 width)
{
	if(orient_mat)
	{
		Vec3 temp,a,b;

		subVec3(la, base_pos, temp);
		mulVecMat3(temp, orient_mat, a);
		addVec3(a, base_pos, a);

		subVec3(lb, base_pos, temp);
		mulVecMat3(temp, orient_mat, b);
		addVec3(b, base_pos, b);

		drawLine3DWidthQueued(a,b,*((int*)c),width);
	}
	else
		drawLine3DWidthQueued(la,lb,*((int*)c),width);
}

void add3dCursor(const Vec3 pos,const Mat3 orient_mat)
{
	int		i,j,k;
	Vec3	dv,dvb,dva;
	U8		c[4];

	subVec3(pos,cam_info.cammat[3],dv);
	normalVec3(dv);
	scaleVec3(dv,-0.1,dv);
	addVec3(dv,pos,dv);
	for(i=0;i<3;i++)
	{
		U8* ci = c + 2 - i;
		
		copyVec3(dv,dva);
		copyVec3(dv,dvb);
		dvb[i] += 20;
		c[0] = c[1] = c[2] = 128;
		*ci = 255;
		c[3] = 255;
		drawLineCheckOrient(orient_mat,dv,dva,dvb,c,3);

		copyVec3(dv,dva);
		copyVec3(dv,dvb);
		dva[i] -= 20;
		c[0] = c[1] = c[2] = 0;
		*ci = 255;
		c[3] = 255;
		drawLineCheckOrient(orient_mat,dv,dva,dvb,c,3);

		for(j = 1; j < 3; j++){
			for(k = 1; k < 2; k++){
				copyVec3(dv,dva);
				copyVec3(dv,dvb);
				dva[i] += -20 + k * 37;
				dva[(i+j)%3] += 1 * (k?1:0.6);
				dvb[i] += -17 + k * 37;
				c[0] = c[1] = c[2] = 192;
				*ci = 255;
				c[3] = 255;
				drawLineCheckOrient(orient_mat,dv,dva,dvb,c,1);

				copyVec3(dv,dva);
				copyVec3(dv,dvb);
				dva[i] += -20 + k * 37;
				dva[(i+j)%3] -= 1 * (k?1:0.6);
				dvb[i] += -17 + k * 37;
				c[0] = c[1] = c[2] = 192;
				*ci = 255;
				c[3] = 255;
				drawLineCheckOrient(orient_mat,dv,dva,dvb,c,1);

				copyVec3(dv,dva);
				copyVec3(dv,dvb);
				dva[i] += -20 + k * 37;
				dva[(i+j)%3] += 1 * (k?1:0.6) * (j==1 ? 1 : -1);
				dvb[i] += -20 + k * 37;
				dvb[(i+(3-j))%3] += 1 * (k?1:0.6);
				c[0] = c[1] = c[2] = 192;
				*ci = 255;
				c[3] = 255;
				drawLineCheckOrient(orient_mat,dv,dva,dvb,c,1);

				copyVec3(dv,dva);
				copyVec3(dv,dvb);
				dva[i] += -20 + k * 37;
				dva[(i+j)%3] -= 1 * (k?1:0.6) * (j==1 ? 1 : -1);
				dvb[i] += -20 + k * 37;
				dvb[(i+(3-j))%3] -= 1 * (k?1:0.6);
				c[0] = c[1] = c[2] = 192;
				*ci = 255;
				c[3] = 255;
				drawLineCheckOrient(orient_mat,dv,dva,dvb,c,1);
			}
		}

		for(j = -20; j < 20; j++){
			if(j){
				copyVec3(dv,dva);
				copyVec3(dv,dvb);
				dva[i] += j;
				dva[(i+1)%3] += (j%5) ? 0.2 : 0.4;
				dvb[i] += j;
				c[0] = c[1] = c[2] = (j%5) ? 64 : 255;
				*ci = 255;
				c[3] = 255;
				drawLineCheckOrient(orient_mat,dv,dva,dvb,c,1);
			}

			copyVec3(dv,dva);
			copyVec3(dv,dvb);
			dva[i] += j + 0.5f;
			dva[(i+1)%3] += 0.1;
			dvb[i] += j + 0.5f;
			c[0] = c[1] = c[2] = 64;
			*ci = 192;
			c[3] = 255;
			drawLineCheckOrient(orient_mat,dv,dva,dvb,c,1);
		}
	}
	copyVec3(pos,edit_stats.cursor_pos);
}

void showmat(const Mat4 mat) {
	if (mat==NULL) return;
	printf("%f %f %f\n%f %f %f\n%f %f %f\n\n",
		mat[0][0],mat[0][1],mat[0][2],
		mat[1][0],mat[1][1],mat[1][2],
		mat[2][0],mat[2][1],mat[2][2]);

}

double getRadiusOfSelected(void)
{
	DefTracker	*tracker;
	Vec3		min = {8e16,8e16,8e16}, max = {-8e16,-8e16,-8e16};
	int			i;
	Vec3		dv;
	Mat4		mat;

	if (sel_count==0) return -1;

	if (sel_count==1 && sel.fake_ref.def)	//handle the case where one thing was loaded from the
		return sel.fake_ref.def->radius;	//library but has not yet been placed

	for(i=0;i<sel_count;i++)
	{
		tracker = sel_list[i].def_tracker;
		trackerGetMat(tracker,groupRefFind(sel_list[i].id),mat);
		mulVecMat4(tracker->def->min,mat,dv);
		MINVEC3(min,dv,min);
		mulVecMat4(tracker->def->max,mat,dv);
		MAXVEC3(max,dv,max);
	}
	subVec3(max,min,dv);
	return lengthVec3(dv);
//	scaleVec3(dv,0.5,dv);
//	addVec3(dv,min,view_pos);
}


//drawRotationGuide
//pos is the center of rotation
//orientation is the matrix describing the current orientation of the selected object(s)
//axis is the axis of rotation
//purpose:	draws a red line on the axis of rotation, and a blue circle in the plane perpendicular
//			to the axis.  Circle has lines indicating what angles the rotation is snapped to
void drawRotationGuide(const Vec3 pos,const Mat3 orientation,int axis)
{
	int		i;
	int plane1,plane2;					//axes of the plane of rotation
	static double size=-1;				//these static variables are used to determine
	static int lastSelCount=-1;			//if the objects selected have changed since the 
	static DefTracker * lastDef=NULL;	//last time the function was called, that way we
										//don't have to calculate their radius every time
	Vec3	dv,buf,temp,start,end;
	int sides=25;						//number of sides the circle that describes the direction
										//of rotation will have
	int color;

	if (sel_count==0 && sel.fake_ref.def==NULL)
		return;

	if (sel_count==0) {
		size=sel.fake_ref.def->radius;
	} else if (lastSelCount!=sel_count || lastDef!=sel_list[0].def_tracker) {
		lastSelCount=sel_count;
		lastDef=sel_list[0].def_tracker;
		size=getRadiusOfSelected();
	}

	if (size/20>sides)			//add a few more sides if the default isn't enough to look good
		sides=(int)(size/20);

	if (sides > 1000)			// However keep it reasonably sane.
		sides = 1000;

	subVec3(pos,cam_info.cammat[3],dv);
	normalVec3(dv);
	scaleVec3(dv,-0.1,dv);
	addVec3(dv,pos,dv);

	axis=(axis+1)%3;		//for some reason the axis passed to this function is never quite the one
	plane1=(axis+1)%3;		//i would expect it to be
	plane2=(axis+2)%3;	


	//draw the red axis
	copyVec3(dv,start);
	copyVec3(dv,end);
	start[axis] -= size/2.0;
	end[axis] += size/2.0;

	copyVec3(start,buf);						//these four lines make the orientation matrix apply 
	subVec3(start, dv, temp);				//without any translation (so only rotation)
	mulVecMat3(temp, orientation,start);
	addVec3(start, dv,start);

	copyVec3(end,buf);						//same four lines (and again below)
	subVec3(end, dv, temp);
	mulVecMat3(temp, orientation,end);
	addVec3(end, dv,end);

	color=0xFFFF0000;

	drawLine3DQueued(start,end,color);
	//end red axis


	size*=1.0;
	//draw blue circle, perpendicular to red axis
	for (i=0;i<sides;i++) {
		copyVec3(dv,start);
		copyVec3(dv,end);
		start[plane1] += size/2.0*cos(((double)i/(double)sides)*2*3.1415926535);
		start[plane2] += size/2.0*sin(((double)i/(double)sides)*2*3.1415926535);
		end[plane1] += size/2.0*cos(((double)(i+1)/(double)sides)*2*3.1415926535);
		end[plane2] += size/2.0*sin(((double)(i+1)/(double)sides)*2*3.1415926535);

		copyVec3(start,buf);
		subVec3(start, dv, temp);
		mulVecMat3(temp, orientation,start);
		addVec3(start, dv,start);

		copyVec3(end,buf);
		subVec3(end, dv, temp);
		mulVecMat3(temp, orientation,end);
		addVec3(end, dv,end);

		color=0xFF0000FF;		
		drawLine3DQueued(start,end,color);
	}
	//end blue circle
	if (!edit_state.nosnap) {
		int colorDegree[3]={0xFF0000FF,0xFF00FF00,0xFFFFFFFF};	//Red, Orange, Yellow
		double interval=lock_angs[edit_state.rotsize];
		double degree;
		for (degree=0;degree<=180.0;degree+=interval) {
			copyVec3(dv,start);
			copyVec3(dv,end);
			start[plane1] += size/2.0*cos(degree*3.1415926535/180.0);
			start[plane2] += size/2.0*sin(degree*3.1415926535/180.0);
			end[plane1] += size/2.0*cos((degree+180)*3.1415926535/180.0);
			end[plane2] += size/2.0*sin((degree+180)*3.1415926535/180.0);

			copyVec3(start,buf);
			subVec3(start, dv, temp);
			mulVecMat3(temp, orientation,start);
			addVec3(start, dv,start);

			copyVec3(end,buf);
			subVec3(end, dv, temp);
			mulVecMat3(temp, orientation,end);
			addVec3(end, dv,end);

			color=colorDegree[0];
			{
				double x=(double)((int)(degree/45.0));
				double y=degree/45.0;
				double z=((degree/45.0)-(double)((int)(degree/45.0)));
				x=x;
				y=y;
			}
			if (((degree/45.0)-(double)((int)(degree/45.0)))*((degree/45.0)-(double)((int)(degree/45.0)))<1e-9)
				color=colorDegree[1];
			if (((degree/90.0)-(double)((int)(degree/90.0)))*((degree/90.0)-(double)((int)(degree/90.0)))<1e-9)
				color=colorDegree[2];
			drawLine3DQueued(start,end,color);
		}
		
	}
}

void drawLasso(int ulx,int uly,int llx,int lly) {
	Vec3 v[4];
	Vec3 buf,start,end;
	int i;

	gfxCursor3d(ulx,uly,1.125,buf,v[0]);
	gfxCursor3d(ulx,lly,1.125,buf,v[1]);
	gfxCursor3d(llx,lly,1.125,buf,v[2]);
	gfxCursor3d(llx,uly,1.125,buf,v[3]);

	for (i=0;i<4;i++) {
		copyVec3(v[i],start);
		copyVec3(v[(i+1)%4],end);
		drawLine3DQueued(start,end,0xFFFFFF00);
	}
}


void drawSphere(const Mat4 mat,F32 rad,int nsegs,U32 color)
{
	int		i,j;
	Vec3	dva,dvb;
	Vec3	lna,lnb;
	Vec3	l2a,l2b;
	F32		yaw,r2,ratio;
	U8		color_list[4];

 	color_list[0] = (color >> 16) & 255;
	color_list[1] = (color >> 8) & 255;
	color_list[2] = (color >> 0) & 255;
	color_list[3] = (color >> 24) & 255;
	if (!color_list[3])
		color_list[3] = 255;
	for(i=0;i<=nsegs;i++)
	{
		for(j=0;j<=nsegs;j++)
		{
			yaw = (j * 2 * PI) / nsegs;
			ratio = (i - nsegs/2) * 2.f / nsegs;
			r2 = rad * cos(ratio * PI * 0.5f);
			r2 = rad * sqrt(1 - ratio * ratio);
			lnb[0] = sin(yaw) * r2;
			lnb[1] = rad * ratio;
			lnb[2] = cos(yaw) * r2;
			if (j)
			{
				mulVecMat4(lna,mat,dva);
				mulVecMat4(lnb,mat,dvb);
				drawLine3DQueued(dva,dvb,*((int*)color_list));

				l2a[0] = lna[1];
				l2a[1] = lna[0];
				l2a[2] = lna[2];
				l2b[0] = lnb[1];
				l2b[1] = lnb[0];
				l2b[2] = lnb[2];

				mulVecMat4(l2a,mat,dva);
				mulVecMat4(l2b,mat,dvb);
				drawLine3DQueued(dva,dvb,*((int*)color_list));
			}
			copyVec3(lnb,lna);
		}
	}
}

void drawSphereMidRad(const Vec3 mid,F32 rad,int nsegs,U32 color)
{
	int		i,j;
	Vec3	dva,dvb;
	Vec3	lna,lnb;
	Vec3	l2a,l2b;
	F32		yaw,r2,ratio;
	U8		color_list[4];

	color_list[0] = (color >> 16) & 255;
	color_list[1] = (color >> 8) & 255;
	color_list[2] = (color >> 0) & 255;
	color_list[3] = (color >> 24) & 255;
	if (!color_list[3])
		color_list[3] = 255;
	for(i=0;i<=nsegs;i++)
	{
		for(j=0;j<=nsegs;j++)
		{
			yaw = (j * 2 * PI) / nsegs;
			ratio = (i - nsegs/2) * 2.f / nsegs;
			r2 = rad * cos(ratio * PI * 0.5f);
			r2 = rad * sqrt(1 - ratio * ratio);
			lnb[0] = sin(yaw) * r2;
			lnb[1] = rad * ratio;
			lnb[2] = cos(yaw) * r2;
			if (j)
			{
				addVec3(lna,mid,dva);
				addVec3(lnb,mid,dvb);
				drawLine3DQueued(dva,dvb,*((int*)color_list));

				l2a[0] = lna[1];
				l2a[1] = lna[0];
				l2a[2] = lna[2];
				l2b[0] = lnb[1];
				l2b[1] = lnb[0];
				l2b[2] = lnb[2];

				addVec3(l2a,mid,dva);
				addVec3(l2b,mid,dvb);
				drawLine3DQueued(dva,dvb,*((int*)color_list));
			}
			copyVec3(lnb,lna);
		}
	}
}

#define NUMLINES 256

U32 setLineColor(int gridsize,int idx,int style)
{
	U8	color[4];

	memset(color,255,4);
	color[3] = 45;
	if (style == 1)
	{
		color[0] = 0;
		color[3] = 75;
	}
	if (idx==NUMLINES/2)
		return 0xFFFFFFFF;

	if ((idx & 3) == 0)
	{
		color[3] = 65;
		color[1] = 0;
	}
	if ((idx & 15) == 0)
	{
		color[3] = 85;
		color[1] = 255;
		color[2] = 0;
	}
	return(*((int*)color));
}

U32 setLineInfo2(int gridsize,int idx)
{
	U8	color[4];

	memset(color,255,4);
	color[3] = 45;
	return(*((int*)color));
}

void showGrid(const Mat4 matx,int *edit_axisp,int *edit_axis,int style) {
	copyMat4(matx,globalmatx);
	globaledit_axisp[0]=edit_axisp[0];
	globaledit_axisp[1]=edit_axisp[1];
	globaledit_axisp[2]=edit_axisp[2];
	globaledit_axis[0]=edit_axis[0];
	globaledit_axis[1]=edit_axis[1];
	globaledit_axis[2]=edit_axis[2];
	globalstyle=style;
	globalshowGrid=1;
}

void showGridExternal()
{
	Mat4	matx;
	int *	edit_axisp;
	int *	edit_axis;
	int		style;
	int		i,count=0;
	Vec3	lna,lnb,dva,dvb;
	Mat4	mat;
	int		suppress[3] = { 1, 2, 2},dontdraw=0;
	F32		gridsize,linesize;

	if (!globalshowGrid)
		return;

	copyMat4(globalmatx,matx);
	edit_axisp=globaledit_axisp;
	edit_axis=globaledit_axis;
	style=globalstyle;

	if (edit_axisp[0] + edit_axisp[1] + edit_axisp[2])
		copyVec3(edit_axisp,edit_axis);
	gridsize = 0.5f * (1 << edit_state.gridsize);
	linesize = MAX(gridsize,0.5) * NUMLINES;
	for(i=0;i<3;i++)
		count += edit_axis[i];
	for(i=0;i<3;i++)
	{
		if (edit_axis[i])
		{
			if (!dontdraw)
				dontdraw = suppress[i];
			else
				dontdraw = 0;
		}
/*		if (((count > 1 && edit_state.snap3) || edit_axis[i]) && gridsize && !edit_state.nosnap && !edit_state.snaptovert)
		{
			if (0 && i == 1)
				t = 2 * matx[3][i] + gridsize;
			else
				t = 2 * matx[3][i];// - gridsize;
			t &= ~(((int)(gridsize*2))-1);
			matx[3][i] = 0.5f * t;
		}
*/
	}
	if (edit_state.snaptovert)
	{
		copyVec3(sel.snap_pos,matx[3]);
	}

	copyMat4(matx,mat);
	if (!gridsize)
		gridsize = 1;

//as far as i can tell this code is only for snapping a point to some multiple of 16
//which i don't want to happen
/*	for(i=0;i<3;i++)
	{
		if (edit_axis[i] && gridsize)
		{
			if (i == 1)
				t = mat[3][i] + (((F32)gridsize) * 0.5);
			else
				t = mat[3][i] + (((F32)gridsize) * 0.5);
			t &= ~(((int)(gridsize * 16))-1);		// a comment
			mat[3][i] = t;
		}
	}
*/
	if (dontdraw != 1) for(i=0;i<NUMLINES+1;i++)
	{
		lna[0] = -linesize/2;
		lna[1] = 0;
		lna[2] = (linesize * i / (NUMLINES)) - linesize/2;

		lnb[0] = linesize/2;
		lnb[1] = 0;
		lnb[2] = (linesize * i / (NUMLINES)) - linesize/2;

		mulVecMat4(lna,mat,dva);
		mulVecMat4(lnb,mat,dvb);
		drawLine3DQueued(dva,dvb,setLineColor(gridsize,i,style));
	}
	if (dontdraw != 2) for(i=0;i<NUMLINES+1;i++)
	{
		lna[2] = -linesize/2;
		lna[1] = 0;
		lna[0] = (linesize * i / (NUMLINES)) - linesize/2;

		lnb[2] = linesize/2;
		lnb[1] = 0;
		lnb[0] = (linesize * i / (NUMLINES)) - linesize/2;

		mulVecMat4(lna,mat,dva);
		mulVecMat4(lnb,mat,dvb);
		drawLine3DQueued(dva,dvb,setLineColor(gridsize,i,style));
	}
	globalshowGrid=0;
}

void showGrid2(const Mat4 matx,int *edit_axisp,int *edit_axis)
{
	int		i,count=0;
	Vec3	lna,lnb,dva,dvb;
	Mat4	mat;
	int		suppress[3] = { 1, 2, 2},dontdraw=0;
	F32		gridsize,linesize;
	#define NUMLINES2 64

	gridsize = 128.f;
	linesize = MAX(gridsize,0.5) * NUMLINES2;
		gridsize = 1;
	copyMat4(matx,mat);

	for(i=0;i<NUMLINES+1;i++)
	{
		lna[0] = -linesize/2;
		lna[1] = 0;
		lna[2] = (linesize * i / (NUMLINES2)) - linesize/2;

		lnb[0] = linesize/2;
		lnb[1] = 0;
		lnb[2] = (linesize * i / (NUMLINES2)) - linesize/2;

		mulVecMat4(lna,mat,dva);
		mulVecMat4(lnb,mat,dvb);
		drawLine3DQueued(dva,dvb,setLineInfo2(gridsize,i));
	}
	for(i=0;i<NUMLINES+1;i++)
	{
		lna[2] = -linesize/2;
		lna[1] = 0;
		lna[0] = (linesize * i / (NUMLINES2)) - linesize/2;

		lnb[2] = linesize/2;
		lnb[1] = 0;
		lnb[0] = (linesize * i / (NUMLINES2)) - linesize/2;

		mulVecMat4(lna,mat,dva);
		mulVecMat4(lnb,mat,dvb);
		drawLine3DQueued(dva,dvb,setLineInfo2(gridsize,i));
	}
}

#include "sound.h"

void editDrawSoundSpheres()
{
	int		i;
	Mat4	mat;

	copyMat3(unitmat,mat);
	for(i=0;i<sound_editor_count;i++)
	{
		SoundEditorInfo	*sei = &sound_editor_infos[i];
		DefTracker		*tracker = sei->def_tracker;
		U32				color = 0xffff00;

		if (tracker && tracker->def->sound_exclude)
			color = 0xff0000;
		copyVec3(sei->pos,mat[3]);
		drawSphere(mat,sei->radius,11,color);
	}
}

/* End of File */
