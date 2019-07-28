#include <stdlib.h>
#include <string.h>
#include "light.h"
#include "mathutil.h"
#include "anim.h"
#include "grid.h"
#include "model.h"
#include "gridcoll.h"
#include "memcheck.h"
#include "group.h"
#include "grouptrack.h"
#include "utils.h"
#include "font.h"
#include "cmdcommon.h"
#include "rendermodel.h"
#include "cmdgame.h"
#include "entclient.h"
#include "assert.h"
#include "model_cache.h"
#include "renderutil.h"
#include "rgb_hsv.h"
#include "sun.h"
#include "tricks.h"
#include "renderprim.h"
#include "file.h"
#include "seq.h"
#include "gridfind.h"
#include "vistray.h"
#include "player.h"

#define MAX_SHADOW_CALCS 5

#define MAX_LIGHTS 1000

static F32 glob_num_lights;
static F32 glob_light_sum;

static void addLightToGrid(DefTracker *tracker,Grid *light_grid)
{
	static Vec3 luminance_vector = {0.35, 0.45, 0.2}; // {0.2125, 0.7154, 0.0721};
	Vec3		dv,min,max;
	GroupDef	*def = tracker->def;
	F32			luminance;

	if (def->light_radius > 10000)
		def->light_radius = 10000;

	luminance = dotVec3(def->color[0], luminance_vector);
	glob_light_sum += luminance*def->light_radius;
	glob_num_lights+=1; //def->light_radius;

	setVec3(dv,def->light_radius,def->light_radius,def->light_radius);
	subVec3(tracker->mat[3],dv,min);
	addVec3(tracker->mat[3],dv,max);
	gridAdd(light_grid,(void*)tracker,min,max,2,0);
}

void lightsToGrid(DefTracker *tracker,Grid *light_grid)
{
	int			i;

	if (tracker->def && tracker->def->is_tray)
		vistrayDetailLightsToGrid(tracker, light_grid);

	if (!(tracker->def->child_light || tracker->def->has_light))
		return;
	trackerOpen(tracker);
	if (tracker->def->has_light)
		addLightToGrid(tracker,light_grid);

	for(i=0;i<tracker->count;i++)
	{
		if (!tracker->entries[i].def->has_ambient)
			lightsToGrid(&tracker->entries[i],light_grid);
	}

}

static F32 lumMapping(F32 lum)
{
	// Mapping of values determined empirically
	F32 x0=2.3, y0=0.85, x1=10.5, y1=1.5;
	return (lum - x0) * (y1-y0)/(x1-x0) + y0;
}

void lightTracker(DefTracker *tracker)
{
#if 1
	if (tracker->light_grid)
	{
		gridFreeAll(tracker->light_grid);
		destroyGrid(tracker->light_grid);
	}
#else
	lightGridDel(tracker);
#endif
	glob_light_sum = 0;
	glob_num_lights = 0;
	tracker->light_grid = createGrid(64);
	lightsToGrid(tracker,tracker->light_grid);
	if (glob_num_lights)
		tracker->light_luminance = lumMapping(glob_light_sum/(glob_num_lights*255.f));
	else
		tracker->light_luminance = 0;

}

void getDefColorVec3(GroupDef *def,int which,Vec3 rgb_out)
{
	Vec3	rgb;

	scaleVec3(def->color[which],1.f/63,rgb);
	sceneAdjustColor(rgb,rgb_out);
	scaleVec3(rgb_out,0.25,rgb_out);
}

void getDefColorU8(GroupDef *def,int which,U8 color[3])
{
	Vec3	v;

	getDefColorVec3(def,which,v);
	scaleVec3(v,255,color);
}

int light_recalc_count;
void lightModel(Model *model,Mat4 mat,U8 *rgb,DefTracker *light_trkr, F32 brightScale)
{
	int			is_light=0,i,j,count=0,c;
	Vec3		v,nv,color,dv,ambient;
	DefTracker	*tracker,*lights[MAX_LIGHTS];
	GroupDef	*def;
	F32			falloff,angle,intensity,*scene_amb;
	GridCell	*cell;
	Grid		*light_grid;
	U8			maxbright[4],*base=0;
	static Vec3	*verts,*norms;
	static int	vert_max,norm_max;

	assert(rgb && light_trkr);
	light_recalc_count++;
	if (isDevelopmentMode() && light_trkr->def && light_trkr->def->has_ambient && !light_trkr->light_grid) // For Trick reloading
		lightTracker(light_trkr);

	light_grid = light_trkr->light_grid;
	if (!light_grid || !modelHasVerts(model) || !modelHasNorms(model))
		return;

	dynArrayFit((void**)&verts,sizeof(Vec3),&vert_max,model->vert_count);
	dynArrayFit((void**)&norms,sizeof(Vec3),&norm_max,model->vert_count);

	modelGetVerts(verts, model);
	modelGetNorms(norms, model);

	getDefColorVec3(light_trkr->def,0,ambient);
	scene_amb = sceneGetAmbient();
	if (scene_amb)
		scaleVec3(scene_amb,.25,ambient);
		
	copyVec3(light_trkr->def->color[1],maxbright);
	if ((maxbright[0]|maxbright[1]|maxbright[2]) == 0)
		maxbright[0] = maxbright[1] = maxbright[2] = 255;
	else
		getDefColorU8(light_trkr->def,1,maxbright);

	if (strstri(model->name,"omni"))
		is_light = 1;
	for(i=0;i<model->vert_count;i++)
	{
		mulVecMat4(verts[i],mat,v);
		copyVec3(ambient,color);
		++light_grid->tag;
		cell = gridFindCell(light_grid,v,(void *)lights,&count,MAX_LIGHTS);

		for(j=0;j<count;j++)
		{
			F32 dist;

			tracker	= lights[j];
			def		= tracker->def;
			if (!def || def->key_light)
				continue;
			subVec3(v,tracker->mat[3],dv);
			dist = normalVec3(dv);
			if (is_light && dist > 3.f)
				continue;
			if (dist > def->light_radius)
				continue;

			mulVecMat3(norms[i],mat,nv);
			angle = -dotVec3(dv,nv);
			if (model->flags & OBJ_NOLIGHTANGLE)
				angle = 1.f;
			if (angle < 0)
				continue;

			falloff = def->light_radius * 0.333f;
			if (dist < falloff)
				intensity = 1;
			else
			{
				intensity = (def->light_radius - dist) / (def->light_radius - falloff);
				intensity = intensity * intensity;
			}
			intensity *= angle;
			if (intensity < 0.05)
				continue;

			getDefColorVec3(def,0,dv);
			scaleVec3(dv,intensity,dv);
			if (def->color[0][3] & 1)  // negative light
				subVec3(color,dv,color);
			else
				addVec3(color,dv,color);
		}
		for(j=0;j<3;j++)
		{
			c = color[j] * brightScale * 255;
			if (c > maxbright[j])
				c = maxbright[j];
			if (c < 0)
				c = 0;
			*rgb++ = c;
		}
		*rgb++ = 255;
	}
}

DefTracker *findLightsRecur(DefTracker *tracker,Vec3 pos,F32 *rad)
{
	Vec3		dv;
	int			i;
	F32			min_rad=16e16,d;
	DefTracker	*curr,*best=0;

	if (!tracker->def->child_ambient && !tracker->def->has_ambient)
		return 0;
	mulVecMat4(tracker->def->mid,tracker->mat,dv);
	subVec3(pos,dv,dv);
	d = lengthVec3(dv);
	if (d > tracker->def->radius)
		return 0;

	if (tracker->def->has_ambient)
	{
		best = tracker;
		min_rad = 1.f / (tracker->def->radius - d);
	}
	if (tracker->def->child_ambient)
	{
		for(i=0;i<tracker->count;i++)
		{
			curr = findLightsRecur(&tracker->entries[i],pos,rad);
			if (curr && *rad < min_rad)
			{
				best = curr;
				min_rad = *rad;
			}
		}
	}
	if (!best || !best->def->has_ambient)
		return 0;
	*rad = min_rad;
	return best;
}

DefTracker *findLights(Vec3 pos)
{
	DefTracker	*ref;
	int				i;
	DefTracker		*tracker=0,*curr;
	F32				min_rad=16e16,rad;

	for(i=0;i<group_info.ref_count;i++)
	{
		ref = group_info.refs[i];
		if (ref->def)
		{
			curr = findLightsRecur(ref,pos,&rad);
			if (curr && rad < min_rad)
			{
				tracker = curr;
				min_rad = rad;
			}
		}
	}
	return tracker;
}

#include "entity.h"
#include "groupdraw.h"

#define DISTANCE_TO_MOVE_BEFORE_RECALCULATING_LIGHT 2.0
static int no_light_interp = 0; //debug

void lightEnt(EntLight *light,Vec3 pos, F32 min_base_ambient, F32 max_ambient )
{
	DefTracker	*tracker;
	GridCell	*cell;
	DefTracker	*lights[MAX_LIGHTS];
	F32			dist;
	int			j,count;
	GroupDef	*def;
	Vec3		dv;
	Grid		*light_grid;
	Vec3		color;

	Vec3	diffuse_dir;
	Vec3	one_light_dir;
	F32		total_intensity;
	F32		len_diffuse_dir;
	F32		part_diffuse;
	Vec3	diffuse_added;
	Vec3	ambient_added;
	Vec3	gross_diffuse_dir;
	F32* scene_amb;
	F32		ambient_scale = 1.0;

//	Line * line;
#define LIGHTINFO 0
	//extern int camera_is_inside;
    
	PERFINFO_AUTO_START("lightEnt", 1);

	if(	light->hasBeenLitAtLeastOnce )
		no_light_interp = 0; //debug
	else
		no_light_interp = 1;
		
	light->hasBeenLitAtLeastOnce = 1;	

	//IF anyone wnats to custom set the ambinet or diffuse sclae, they have to do it every frame
	/*
	light->use &= ~ENTLIGHT_CUSTOM_AMBIENT;
	light->use &= ~ENTLIGHT_CUSTOM_DIFFUSE;
	light->ambientScale = 1.0;
	light->diffuseScale = 1.0;
	light->target_ambientScale;
	light->target_diffuseScale;
	light->ambientScaleUp;
	light->ambientScale
	*/
	light->ambientScale = 1.0; 	  
	light->diffuseScale = 1.0; 
	if( light->use & ( ENTLIGHT_CUSTOM_AMBIENT | ENTLIGHT_CUSTOM_DIFFUSE ) )
	{
		F32 scale;
		F32 pulseEndTime = light->pulseEndTime;
		F32 pulsePeakTime = light->pulsePeakTime;
		F32	pulseBrightness = light->pulseBrightness;

		light->pulseTime += TIMESTEP;

		if( light->pulseTime > pulseEndTime )     
		{
			light->use &= ~ENTLIGHT_CUSTOM_AMBIENT;
			light->use &= ~ENTLIGHT_CUSTOM_DIFFUSE; 
			light->ambientScale = 1.0; 
			light->diffuseScale = 1.0;
			light->pulseTime = 0;
			light->pulseClamp = 0;
			scale = 0.0;
		}
		else if( light->pulseTime > pulsePeakTime )  
		{
			scale = ( 1.0 - ( (light->pulseTime - pulsePeakTime) / (pulseEndTime - pulsePeakTime ) ) );
		}
		else 
		{
			scale = ( light->pulseTime / pulsePeakTime );
		}

		//Override : if you are told to clamp to a certain brightness, just be that
		//AS long as you are repeatedly told by the fx to hold your pulse value, hold it.
		if( light->pulseClamp )  
		{
			pulseBrightness = light->pulseBrightness;
			scale = 1.0;
		}

		scale = 1 + (scale * (pulseBrightness - 1.0) );
 
		//xyprintf( 20, 20, "Light %f at age %f ", scale, light->pulseTime);

		//if( light->pulseTime > pulsePeakTime && scale2 > light->pulseClamp )
		//	light->pulseTime = pulsePeakTime * scale;

		light->ambientScale = scale;
		light->diffuseScale = scale; 

		//light->ambientScale = 1.0;
		//light->diffuseScale = 1.0;
	}
	
	//xyprintf( 22, 21, "ambientScale %f", light->ambientScale); 
	//xyprintf( 22, 22, "diffuseScale %f", light->diffuseScale); 
 
	//Only update special lighting when you've moved more than 2 feet from the last pos at which you updated light
	subVec3(pos,light->calc_pos,dv);   

	if ( lengthVec3Squared(dv) > DISTANCE_TO_MOVE_BEFORE_RECALCULATING_LIGHT*DISTANCE_TO_MOVE_BEFORE_RECALCULATING_LIGHT ) //4.0    
	{
		copyVec3(pos,light->calc_pos);

		//What is the player inside of?  Only use special lighting if it has special lighting
		tracker = groupFindLightTracker(pos);
		if (!tracker || !tracker->light_grid)
		{
			//this tracker hasn't been processed yet, so dont give up on it for lighting
			//slightly hacky way to say this light still needs to be processed even though it hasn't moved lately
			if( tracker && tracker->def && tracker->def->has_ambient )
				light->calc_pos[1] = -1000;  

			light->use &= ~ENTLIGHT_INDOOR_LIGHT;

			PERFINFO_AUTO_STOP();
			return;
		}
		light->use |= ENTLIGHT_INDOOR_LIGHT; 

		//Get Base Ambient Light (from the group tracker) (can be augmented by dispersed diffuse lights)
		getDefColorVec3(tracker->def,0,light->tgt_ambient);
		scaleVec3(light->tgt_ambient,155.f/255.f,light->tgt_ambient);

		//Override to the tracker lighting from the scene file
		scene_amb = sceneGetAmbient();
		if (scene_amb)
			scaleVec3(scene_amb,.25,light->tgt_ambient);

		//scaleVec3(tracker->def->color[0],(1.f/155.f),light->tgt_ambient); 
		for(j=0;j<3;j++) //mm 
			light->tgt_ambient[j] = MAX( light->tgt_ambient[j], min_base_ambient );
		//Get Diffuse Light(get all the lights around you and add their influences based on distance to a single color)
		//(Where is the direction of the diffuse light figured out?)
		light_grid = tracker->light_grid;
		light_grid->tag++;
		count = 0;
		cell = gridFindCell(light_grid,pos,(void *)lights,&count,MAX_LIGHTS);

		zeroVec3(color);
		zeroVec3(diffuse_dir);
		diffuse_dir[1] = 0.001; //so if there are no ligths, the direction will be up and left
		diffuse_dir[0] = 0.001; // 

		total_intensity = 0;

		for(j=0;j<count;j++)
		{
			F32	falloff,intensity;

			def	= lights[j]->def;

			subVec3(lights[j]->mat[3],pos,one_light_dir);
			dist = normalVec3(one_light_dir);
			if (dist >= def->light_radius)
				continue;

			falloff = def->light_radius * 0.333f;
			if (dist < falloff)
			{
				intensity = 1;
			}
			else 
			{
				intensity = (def->light_radius - dist) / (def->light_radius - falloff);
				intensity = intensity * intensity;
			}
			intensity *= 1.f / 255.f;

			if (def->key_light)
			{
				//Find diffuse_dir(*= -1 ?)
				scaleVec3(one_light_dir, intensity * 1000 , one_light_dir); //*1000 to make it work
				addVec3( diffuse_dir, one_light_dir, diffuse_dir  );
			}
			else
			{
				getDefColorVec3(def,0,dv);
				scaleVec3(dv,intensity * 255,dv);
				//scaleVec3(def->color[0],intensity,dv);
				if ( def->color[0][3] & 1)  // negative light
					subVec3(color,dv,color);
				else
					addVec3(color,dv,color);
				total_intensity += intensity; 
			}
		}

		////////////////////////////////////////////////////////
		copyVec3(diffuse_dir, gross_diffuse_dir); //rem gross diffuse dir for debug
		len_diffuse_dir = normalVec3(diffuse_dir); 

		part_diffuse = MIN( 1.0, (len_diffuse_dir / (total_intensity * 0.5)) ); 

		if (color[0] < 0)
			color[0] = 0;
		if (color[1] < 0)
			color[1] = 0;
		if (color[2] < 0)
			color[2] = 0;

		scaleVec3(color, part_diffuse,     diffuse_added );    
		scaleVec3(color, ((1.0 - part_diffuse)/2.0), ambient_added ); 

		copyVec3(diffuse_added,light->tgt_diffuse);

		addVec3(light->tgt_ambient, ambient_added, light->tgt_ambient );  

		if (game_state.scaleEntityAmbient)
		{
			for (j = 0; j < 3; j++)
			{
				if (light->tgt_ambient[j] > max_ambient)
					ambient_scale = MIN(max_ambient, light->tgt_ambient[j] / max_ambient);
			}

			for(j = 0; j < 3; j++)
				light->tgt_ambient[j] *= ambient_scale;
		}
		else
		{
			for(j = 0; j < 3; j++)
				light->tgt_ambient[j] = MIN(max_ambient, light->tgt_ambient[j]);
		}

		copyVec3( diffuse_dir, light->tgt_direction ); 
		////////////////////////////////////////////////////////

		light->diffuse[3] = 1.0; 
		light->ambient[3] = 1.0;
		light->direction[3] = 0.0;

		light->interp_rate = 7.0; //magic number, how many frames to interpolate the new light over
	}

	if( light->interp_rate || no_light_interp)
	{
		F32 percent_new; 

		light->interp_rate -= TIMESTEP;

		if( light->interp_rate <= 0.0 || no_light_interp)
		{
			light->interp_rate = 0.0;
			copyVec3(light->tgt_ambient, light->ambient);
			copyVec3(light->tgt_diffuse, light->diffuse);
			copyVec3(light->tgt_direction, light->direction);
		}
		else
		{
			//interpolate between the tgt_and current.
			percent_new = 1 / (1 + light->interp_rate);//true linear

			posInterp(percent_new, light->ambient,  light->tgt_ambient,   light->ambient);
			posInterp(percent_new, light->diffuse,  light->tgt_diffuse,   light->diffuse);
			posInterp(percent_new, light->direction,light->tgt_direction, light->direction);
			normalVec3(light->direction);
		}
		copyVec3(pos,light->calc_pos);
	}


	if( game_state.fxdebug & FX_CHECK_LIGHTS)
	{
		Vec3 dbig; 
		F32 len;
		Vec3	p1,p2;

		total_intensity = 1.0;
	
		//Draw line in direction of diffuse light, it's length is the intensity of the diffuse light
		len = normalVec3(light->direction); 
		scaleVec3( light->direction, -5 * total_intensity, dbig);  
		subVec3(pos, dbig, p1); 
		copyVec3(pos, p2);

		drawLine3D(p1,p2, 0xff70ff70);

		//Draw line of all lights added up
		//line = &(lightlines[lightline_count++]);

		//scaleVec3( gross_diffuse_dir, -5 , dbig);  
		//subVec3(pos, dbig, line->a); 
		//copyVec3(pos, line->b);
		//line->color[0] = line->color[1] = 255;
		//line->color[2] = line->color[3] = 100;

		//line->a[2] += .2;
		//line->b[2] += .2;
		{
			extern Entity * current_target;
			if( (!current_target && light == &playerPtr()->seq->seqGfxData.light)  || (current_target && &(current_target->seq->seqGfxData.light) == light) )
			{
				xyprintf(5,16,"blue: total diffuse intensity");  
				xyprintf(5,15,"red: combined lights.%%<.5 blue=ambient");

				xyprintf(5,19,"light->tgt_ambient  : %d %d %d",(int)(light->tgt_ambient[0] * 255),(int)(light->tgt_ambient[1] * 255),(int)(light->tgt_ambient[2] * 255));
				xyprintf(5,20,"light->ambient      : %d %d %d",(int)(light->ambient[0] * 255),(int)(light->ambient[1] * 255),(int)(light->ambient[2] * 255));
				xyprintf(5,21,"light->tgt_diffuse  : %d %d %d",(int)(light->tgt_diffuse[0] * 255),(int)(light->tgt_diffuse[1] * 255),(int)(light->tgt_diffuse[2] * 255));
				xyprintf(5,22,"light->diffuse      : %d %d %d",(int)(light->diffuse[0] * 255),(int)(light->diffuse[1] * 255),(int)(light->diffuse[2] * 255));
				xyprintf(5,23,"light->tgt_direction: %d %d %d",(int)(light->tgt_direction[0] * 255),(int)(light->tgt_direction[1] * 255),(int)(light->tgt_direction[2] * 255));
				xyprintf(5,24,"light->direction    : %d %d %d",(int)(light->direction[0] * 255),(int)(light->direction[1] * 255),(int)(light->direction[2] * 255));
				xyprintf(5,25,"light->interp_rate  : %2.2f", light->interp_rate); 
				//xyprintf(5,26,"part_diffuse        : %2.2f", part_diffuse); 
				xyprintf(5,18,"%f %f %f",pos[0],pos[1],pos[2]);
			}
		}
	}

	PERFINFO_AUTO_STOP();
}

void lightGiveLightTrackerToMyDoor(DefTracker * tracker, DefTracker * lighttracker)
{
	Entity * e;
	
	assert(tracker && lighttracker);
	e = entFindEntAtThisPosition(tracker->mat[3], 0.1);

	if( e && e->seq && e->seq->static_light_done != STATIC_LIGHT_DONE )
	{
		if(seqSetStaticLight(e->seq, lighttracker))
			e->seq->static_light_done = STATIC_LIGHT_DONE;
	}
}

#include "gridcache.h"
static int reapply_color_swaps;

void lightReapplyColorSwaps()
{
	reapply_color_swaps = 1;
}

void lightCheckReapplyColorSwaps()
{
	extern Grid obj_grid;
	int		i;

	if (!reapply_color_swaps)
		return;
	reapply_color_swaps = 0;
	gridCacheReset();
	obj_grid.valid_id++;
	for(i=0;i<group_info.ref_count;i++)
	{
		DefTracker	*ref = group_info.refs[i];

		if (!ref)
			continue;
		trackerClose(ref);
		groupRefActivate(ref);
	}
	entResetTrickFlags(); // Relight entities
	vistrayDetailTrays(); // relink objects plopped into tray
	groupTrackersHaveBeenReset();
}

