#include "seq.h"
#include "wininclude.h"  // JS: Can't find where this file includes <windows.h> so I'm just sticking this include here.
#include <string.h>
#include <time.h>
#include "cmdcommon.h"
#include "error.h"
#include "utils.h"
#include "assert.h"
#include "anim.h"
#include "file.h"
#include "mathutil.h"

#include "seqanimate.h"
#include "seqsequence.h"
#include "earray.h"
#include "groupfileload.h"
#include "group.h"
#include "StringTable.h"
#include "StringCache.h"
#if SERVER
#include "cmdserver.h"
#include "seqskeleton.h"
#endif
#if CLIENT
#include "seqgraphics.h"
#include "camera.h"
#include "render.h"
#include "sound.h"
#include "font.h"
#include "cmdgame.h"
#include "fx.h"
#include "light.h"
#include "model_cache.h"
#include "rendershadow.h"
#include "costume.h"
#include "costume_client.h"
#include "player.h"
#include "clothnode.h"
#include "fxlists.h"
#include "fxcapes.h"
#endif
#include "fileutil.h"
#include "timing.h"
#include "strings_opt.h"
#include "gfxtree.h"
#include "seqtype.h"
#include "tex.h"
#include "seqstate.h"
#include "entity.h"
#include "entPlayer.h"
#include "character_base.h"
#include "motion.h"
#include "fxinfo.h"
#include "stashtable.h"
#include "AnimBitList.h"
#include "Quat.h"

#define NO_PARTICULAR_VOLUME 0
#define SPLASH_SCALE_MIN 0
#define SPLASH_SCALE_MAX 5.0


#define	SEQ_MEMPOOL_SIZE 200
MemoryPool seqMp = 0;

int seqLoadInstCalls;
int seqFreeInstCalls;

const SeqMove *getFirstSeqMoveByName(const SeqInst* seq, char * name)
{
	//SeqMove *move = 0;
	int i;
	if(!seq || !seq->info || !seq->info->moves || !name)
		return NULL;

	for(i = 0; i<eaSize(&seq->info->moves); i++)
	{
		if(strstriConst(seq->info->moves[i]->name, name))
			return seq->info->moves[i];
	}
	return NULL;
}


#ifdef CLIENT

static int seqCreateSeqTypeFx( SeqInst * seq , const char * fxname, Entity *entParent);
#endif

//This function manages the fx triggered on an entity when it reaches certain health ranges
//Range is the health Range to play these fx lower bound is inclusive, higher bound exclusive, unfortunately this means play on death must be se to 0 0.00001
//Geometry is a library piece.
//OneShotFx are never turned off, and don't go off when the entity is spawned inside the range
//ContinuingFx are turned off when you leave the range, and go off when the entity is spawned in the range
void seqManageHitPointTriggeredFx( SeqInst * seq, F32 percentHealth, int randomNumber )
{
	const HealthFx * healthFx;
	int i;

	seq->noCollWithHealthFx = false;

	for( i = 0 ; i < eaSize( &seq->type->healthFx ) ; i++ )
	{
		healthFx = seq->type->healthFx[i]; //TO DO HealthFXMultiples

		//If I'm in this health range, check for creations
		//Extra or is for fence post
		if( healthFx->range[0] <= percentHealth && ( percentHealth < healthFx->range[1] || (healthFx->range[1] == 1.0 && percentHealth == 1.0) || (healthFx->range[1] == 0.0 && percentHealth == 0.0))  )
		{
			if( healthFx->flags & HEALTHFX_NO_COLLISION )		 
				seq->noCollWithHealthFx = true;

#ifdef CLIENT
			//if I just entered this health range, look for fx to fire
			if( !( healthFx->range[0] <= seq->percentHealthLastCheck && ( seq->percentHealthLastCheck < healthFx->range[1] || (healthFx->range[1] == 1.0 && seq->percentHealthLastCheck == 1.0) )  ) )
			{
				//Only create oneShot fx if I actually saw the guy go to this hp
				if( eaSize( &healthFx->oneShotFx ) && seq->percentHealthLastCheck != -1 )
				{
					int fxid;
					FxParams fxp;
					char * oneShotFx;
					
					oneShotFx = healthFx->oneShotFx[ randomNumber%(eaSize(&healthFx->oneShotFx)) ]; 
					fxInitFxParams( &fxp );
					fxp.keys[0].seq = seq;
					fxp.keycount++;
					fxp.fxtype = FX_ENTITYFX;
					fxid = fxCreate( oneShotFx, &fxp ); 

					//Debug
					if( isDevelopmentMode() && fxid )
					{
						FxDebugAttributes attribs;
						FxDebugGetAttributes( fxid, 0, &attribs );
						if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
							Errorf( "The FX %s is being used as a oneshot FX when this ent's health goes to a certain point, but has no way to die and will hang around forever", oneShotFx );
					}
					//End Debug
				}
				//Create continuing fx even if he just came into view at this level
				if( eaSize( &healthFx->continuingFx ) )
				{
					char * continuingFx = healthFx->continuingFx[ randomNumber%(eaSize(&healthFx->continuingFx)) ]; 
					FxParams fxp;
					fxInitFxParams( &fxp );
					fxp.keys[0].seq = seq;
					fxp.keycount++;
					fxp.fxtype = FX_ENTITYFX;
					seq->healthFx[i] = fxCreate( continuingFx, &fxp );
				}

				//Create continuing geometry fx even if he just came into view at this level
				if( eaSize( &healthFx->libraryPiece ) )
				{
					char * libraryPiece = healthFx->libraryPiece[ randomNumber%(eaSize(&healthFx->libraryPiece)) ];  //HealthFXMultiples 
					GroupDef *def = groupDefFind( libraryPiece );
					if( !def )
					{
						groupFileLoadFromName( libraryPiece );
						def = groupDefFind( libraryPiece );
						if( !def )
							Errorf( "The library piece %s is not loaded or doesn't exist, but the enttype %s wants to make it.  Ask Craig", libraryPiece, seq->type->name );
					}
					if( def )
					{
						FxParams fxp;
						fxInitFxParams(&fxp);
						fxp.keys[0].seq	= seq;
						fxp.keys[0].bone = 0;
						fxp.keycount++;
						fxp.fxtype = FX_ENTITYFX;
						fxp.externalWorldGroupPtr = def;
						seq->healthGeometryFx[i] = fxCreate( "HardCoded/WorldGroup/WorldGroup.fx", &fxp );
						if( !seq->healthGeometryFx[i] )
							Errorf( "Couldn't create hardcoded worldGroup fx. Old data?" );
						else if( seq->type->useStaticLighting )
							fxSetUseStaticLighting( seq->healthGeometryFx[i], seq->type->useStaticLighting);
					}
				}
			}
#endif //CLIENT
		}
		else //If I'm not in this health range, see if I need to delete anything.
		{
#ifdef CLIENT
			if( seq->healthFx[i] )
			{
				fxDelete( seq->healthFx[i], SOFT_KILL );
				seq->healthFx[i] = 0;
			}
			if( seq->healthGeometryFx[i] )
			{
				fxDelete( seq->healthGeometryFx[i], SOFT_KILL );
				seq->healthGeometryFx[i] = 0;
			}
#endif //CLIENT
		}
	}

	seq->percentHealthLastCheck = percentHealth;
}

#ifdef CLIENT
//TO DO: fx that should get played at the very end of a move might not get played
//if the thing has already moved on. IE. last frame is 30, you ask it to play at
//29, and the move cycles like this 25..28..1, then 28 would never have gotten
//played.  Maybe add a check for that only if it cycles?
void seqManageSeqTriggeredFx( SeqInst * seq, int isNewMove, Entity * e )
{
	int		i, seqFxCount;
	F32		curr,prev;
	Animation * anim;
	const SeqFx * seqfx;
	const TypeGfx * typeGfx;
	char * varName;
	char  fxNameBuf[1024];
	char * fxName;
	char * colon;
	int	preserveThisFx[MAX_SEQFX];

	// Manage onClickFx
	if (seq->onClickFx && !fxIsFxAlive(seq->onClickFx))
		seq->onClickFx = 0;

	memset( preserveThisFx, 0, sizeof( int ) * MAX_SEQFX );

	anim = &seq->animation;

	typeGfx = anim->typeGfx; //seqGetTypeGfx( seq->info, anim->move, seq->type->seqTypeName );

	curr = anim->frame		- typeGfx->animP[0]->firstFrame;
	prev = anim->prev_frame - typeGfx->animP[0]->firstFrame;

	seqFxCount = eaSize( &typeGfx->seqFx );

	for ( i = 0 ; i < seqFxCount ; i++ )
	{
		seqfx = typeGfx->seqFx[i];

 		if(curr >= seqfx->delay && prev < seqfx->delay)
		{
			FxParams fxp;

			fxInitFxParams( &fxp );
			fxp.keys[0].seq = seq;
			fxp.keycount++;
			fxp.fxtype = FX_ENTITYFX;

			//VarReplacement
			{
				strcpy( fxNameBuf, seqfx->name );

				// Extract varName, and NULL terminate fxName in fxNameBuf at the colon
				//  (After the seqfx->name can be a colon, followed by it's Var name by which it is replaced in 
				//   the enttype or behavior system Ex:  Fx	generic/shotgun.fx:SHOTGUN )
				varName = 0;
				fxName  = 0;
				colon = strchr( fxNameBuf, ':' ); 
				if( colon )
				{
					*colon = 0;
					varName = colon + 1;
					if( !varName[0] ) //malformed  xyx.fx: with no var`
						varName = 0;
				}

				// Do var replacement
				if( varName )
					fxName = alGetFxNameFromAnimList( e, varName ); 

				if( !fxName ) 
					fxName = fxNameBuf; //Just use the name you got (it was null terminated above)

				//You can have :SHOTGUN with no default
				if( !fxName || !fxName[0] || 0 == stricmp( fxName, "None" ) )
					continue;
			}

			//For Constant SeqFx, if this fx doesn't exist, make it, if it already
			//exists, just give the existing fx an extension (prevents flicker)
			if( seqfx->flags & SEQFX_CONSTANT )
			{
				FxObject * fx;
				int i;
				int already_here = 0;
				int available_slot = -1;

				for( i = 0 ; i < MAX_SEQFX ; i++ )
				{
					fx = hdlGetPtrFromHandle(seq->const_seqfx[i]);
					if( fx )
					{
						if( !stricmp( fx->fxinfo->name, fxName ) )
						{
							preserveThisFx[i] = 1;
							already_here = 1;
						}
					}
					else
					{
						seq->const_seqfx[i] = 0;
						available_slot = i;
					}
				}
				assert( available_slot != -1 ); //this should never happen

				if( !already_here && available_slot != -1 )
				{
					seq->const_seqfx[available_slot] = fxCreate( fxName, &fxp );
					preserveThisFx[available_slot] = 1;
				}
			}

			else //Otherwise, just make it and forget it
			{
				 fxCreate( fxName, &fxp );
			}
		}
	}

	//If we just changed to a new move, destroy all the const_seqfx that weren't
	//asked for by the new move.
	if( isNewMove )
	{
		for( i = 0 ; i < MAX_SEQFX ; i++ )
		{
			if( !preserveThisFx[i] && seq->const_seqfx[i] )
			{
				fxDelete( seq->const_seqfx[i], SOFT_KILL );
				seq->const_seqfx[i] = 0;
			}
		}
	}

}



void seqManageVolumeFx(SeqInst * seq, Mat4 mat, Mat4 prevMat, Vec3 velocityRaw )
{
	FxParams fxp;
	int what_im_in = NO_PARTICULAR_VOLUME;
	char * splash_fx = 0;		//fx to play when you hit the surface.  hard coded for now
	char * ripple_fx = 0;		//fx to be played continually while I'm on this volume surface
	char * big_splash_fx = 0;	//fx to be played when you are tagged as submerged
	char * exit_drops_fx = 0;	//fx to be played when you emerge from the volume
	Vec3 velocity;

	copyVec3( velocityRaw, velocity );
	velocity[1] = -velocityRaw[1];
	
	// 1 ####### Figure out where you are
	if( TSTB(seq->state, STATE_INWATER) )
	{
		what_im_in = STATE_INWATER;
		ripple_fx		= "Volumes/water/Ripple/Ripple.fx";
		splash_fx		= "Volumes/water/ShallowSplash/ShallowSplash.fx";
		big_splash_fx	= "Volumes/water/DeepSplash/DeepSplash.fx";
	}
	else if( TSTB(seq->state, STATE_INLAVA) )
	{
		what_im_in = STATE_INLAVA;
		ripple_fx		= "Volumes/lava/Ripple/Ripple.fx";
		splash_fx		= "Volumes/lava/ShallowSplash/ShallowSplash.fx";
		big_splash_fx	= "Volumes/lava/DeepSplash/DeepSplash.fx";
	}
	else if( TSTB(seq->state, STATE_INSEWERWATER) )
	{
		what_im_in = STATE_INSEWERWATER; //JE: WAS: STATE_INLAVA;
		ripple_fx		= "Volumes/sewerwater/Ripple/Ripple.fx";
		splash_fx		= "Volumes/sewerwater/ShallowSplash/ShallowSplash.fx";
		big_splash_fx	= "Volumes/sewerwater/DeepSplash/DeepSplash.fx";
	}
	else if( TSTB(seq->state, STATE_INREDWATER) )
	{
		what_im_in = STATE_INREDWATER;
		ripple_fx		= "Volumes/redwater/Ripple/Ripple.fx";
		splash_fx		= "Volumes/redwater/ShallowSplash/ShallowSplash.fx";
		big_splash_fx	= "Volumes/redwater/DeepSplash/DeepSplash.fx";
	}

	if( seq->volume_fx_type == STATE_INWATER ) //what I was in
	{
		exit_drops_fx	= "Volumes/water/ExitDrops/ExitDrops.fx";
	}
	else if ( seq->volume_fx_type == STATE_INLAVA )
	{
		exit_drops_fx	= "Volumes/lava/ExitDrops/ExitDrops.fx";
	}
	else if ( seq->volume_fx_type == STATE_INSEWERWATER )
	{
		exit_drops_fx	= "Volumes/sewerwater/ExitDrops/ExitDrops.fx";
	}
	else if ( seq->volume_fx_type == STATE_INREDWATER )
	{
		exit_drops_fx	= "Volumes/redwater/ExitDrops/ExitDrops.fx";
	}

	// 2 #######  Create and destroy Fx as needed
	if( seq->volume_fx_type != what_im_in )  //If you just changed volumes
	{
		if ( seq->volume_fx_type ) //Kill the fx for the old volume
		{
			if( seq->volume_fx )
				seq->volume_fx = fxDelete( seq->volume_fx, SOFT_KILL );
		}

		if( exit_drops_fx && !( seq->type->flags & SEQ_NO_SHALLOW_SPLASH ) ) //Play the emerge from the old volume FX
		{
			//This prec mat thing is kinda hacky: since we have nested volumes now, I need to remember the old volume position
			//(if you hit and leave in one frame so prev_mat is invalid, you won't get here since what_im_in will match what you used to be in ( I think )
			//So there's no need to check prevMat for validity.  Worst case, a splash at 0, 0, 0
			fxInitFxParams(&fxp);
			copyMat4(prevMat, fxp.keys[0].offset);
			fxp.keycount++;
			fxp.fxtype = FX_VOLUMEFX;
			fxCreate( exit_drops_fx, &fxp ); //(no need to track it)
		}

		seq->volume_fx_type = what_im_in;  //Record new volume
		seq->big_splash = 0; //next time you get deep into a volume, know to splash

		if( what_im_in != NO_PARTICULAR_VOLUME ) //If the new volume, is interesting
		{
			if( ripple_fx && !( seq->type->flags & SEQ_NO_SHALLOW_SPLASH ) ) //Turn on the ripple fx (keep track of it)
			{
				fxInitFxParams(&fxp);
				copyMat4(mat, fxp.keys[0].offset);
				fxp.keycount++;
				fxp.fxtype = FX_VOLUMEFX;
				seq->volume_fx = fxCreate( ripple_fx, &fxp );
			}

			if( splash_fx && !( seq->type->flags & SEQ_NO_SHALLOW_SPLASH ) ) //Trigger the little splash (no need to keep track of it)
			{
				fxInitFxParams(&fxp);
				copyMat4(mat, fxp.keys[0].offset);
				fxp.keys[0].offset[3][0] += velocity[0] * 2;
				fxp.keys[0].offset[3][2] += velocity[2] * 2;
  				fxp.keycount++;
				fxp.fxtype = FX_VOLUMEFX;
				fxp.power = magnitudeToFxPower( velocity[1], SPLASH_SCALE_MIN, SPLASH_SCALE_MAX ); //only downward speed is considered...
				fxCreate( splash_fx, &fxp);
			}
		}
	}

	//If you don't make the a ripple when you get in shallow, but you do deep, then when you get in deep handle that.
	if( ( seq->type->flags & SEQ_NO_SHALLOW_SPLASH ) && !( seq->type->flags & SEQ_NO_DEEP_SPLASH ) )
	{
		if( !seq->volume_fx && ripple_fx && TSTB(seq->state, STATE_INDEEP) ) //You just got in deep
		{
			fxInitFxParams(&fxp);
			copyMat4(mat, fxp.keys[0].offset);
			fxp.keycount++;
			fxp.fxtype = FX_VOLUMEFX;
			seq->volume_fx = fxCreate( ripple_fx, &fxp );
		}
		if( seq->volume_fx && !TSTB(seq->state, STATE_INDEEP) )
		{
			seq->volume_fx = fxDelete( seq->volume_fx, SOFT_KILL );
		}
	}

	//If you are deep enough in the volume to to trigger the swim animation, make a big splash
	if( what_im_in != NO_PARTICULAR_VOLUME && TSTB(seq->state, STATE_INDEEP) && !seq->big_splash )
	{
		seq->big_splash = 1; //Record that you've done the big splash for this volume
		if( big_splash_fx && !( seq->type->flags & SEQ_NO_DEEP_SPLASH ) ) //Do the big splas
		{
			fxInitFxParams(&fxp);
			copyMat4(mat, fxp.keys[0].offset);
			fxp.keys[0].offset[3][0] += velocity[0] * 2;
			fxp.keys[0].offset[3][2] += velocity[2] * 2;
			fxp.keycount++;
			fxp.fxtype = FX_VOLUMEFX;
			fxp.power = magnitudeToFxPower( ABS(velocity[1]), SPLASH_SCALE_MIN, SPLASH_SCALE_MAX ); //only downward speed is considered...
			fxCreate( big_splash_fx, &fxp);
		}
	}

	//#######  update the ripple volume fx with the new point of collision with the water plane
	if( fxIsFxAlive( seq->volume_fx ) )
	{
		fxChangeWorldPosition( seq->volume_fx, mat );
	}
	else //if the fx has died for any reason, forget about it...
	{
		seq->volume_fx = 0;
		//seq->volume_fx_type = 0;
	}
}

void seqTriggerOnClickFx( SeqInst * seq, Entity	*entParent )
{
	if (seq->onClickFx)
		seq->onClickFx = fxDelete(seq->onClickFx, SOFT_KILL);
	seq->onClickFx = seqCreateSeqTypeFx(seq, seq->type->onClickFx, entParent);
}


#endif

//################################################################################
//################################################################################
//############ Create and Destroy a Sequencer ####################################


/*Notice the seq type is copied whole into the sequencer, so a sequencer can change it's type
I don't know if anything ever does that*/
//This always reloads in development mode if the seqType is new
const SeqType* seqGetSeqType(const char type_name[])
{
	SeqType * seqtype;

	assert(type_name);

	seqtype = seqTypeFind(type_name);

	if( !seqtype ) {
 		Errorf("SEQ: !!!Bad ent_type!! %s  Using MALE instead", type_name );
		seqtype = seqTypeFind("male");
	}

	return seqtype;
}


#ifdef CLIENT
/*Attaches an FX to a seq.  (Used when the enttype wants an FX stuck to it permentantly. */
static int seqCreateSeqTypeFx( SeqInst * seq , const char * fxname, Entity *entParent)
{
	FxParams fxp;

	fxInitFxParams(&fxp);
	fxp.keys[0].seq	= seq;
	fxp.keys[0].bone = 0;
	fxp.keycount++;
	fxp.fxtype = FX_ENTITYFX;

	if (playerPtr() && playerPtr() == entParent) {
		fxp.isPersonal = 1;
		fxp.isPlayer = 1;
	}

	return fxCreate( fxname, &fxp );
}

static void getCostumeFxColors(U8 colors[][4], const CostumePart *costumeFx, Entity* e, int numColors)
{
	int i, j;
	FxInfo *fxinfo;

	if (!colors || !costumeFx || !e)
		return;

	fxinfo = fxGetFxInfo(costumeFx->pchFxName);

	if (fxinfo && (fxinfo->fxinfo_flags & FX_USE_SKIN_TINT))
	{
		for (j=0; j<4; j++)
			colors[0][j]=e->costume->appearance.colorSkin.rgba[j];

		for (i=1; i<numColors; i++)
			for (j=0; j<4; j++)
				colors[i][j]=costumeFx->color[i-1].rgba[j];
	}
	else
	{
		for (i=0; i<numColors; i++)
			for (j=0; j<4; j++)
				colors[i][j]=costumeFx->color[i].rgba[j];
	}
}

static const char * getFxGeometry(Entity *e, const CostumePart *costumeFx, const BodyPart * bp)
{
	FxInfo * fxinfo = fxGetFxInfo(costumeFx->pchFxName);
	//check if this geo has a cape attached and whether the cape varies based on the chest selected
    if( fxinfo && fxinfo->conditions[0]->events[0]->capeFiles && bp && bodyPartIsLinkedToChest(bp) )
	{
		if (costumeFx->pchGeom && strStartsWith(costumeFx->pchGeom, "geo_")) {
			// full geo name, no auto-gen
			return costumeFx->pchGeom;
		}
		else
		{
			char tmp[MAX_NAME_LEN];
			const char ** possibleChestNames = getChestGeoLink(e);
			int i;

			for (i = 0; i < eaSize(&possibleChestNames); ++i)
			{
				if( !costumeFx->pchGeom || stricmp( costumeFx->pchGeom, "none") == 0 )
				{
					sprintf( tmp, "Geo_CapeHarness_%s", possibleChestNames[i] );
				}
				else
				{
					sprintf( tmp, "Geo_CapeHarness_%s_%s", possibleChestNames[i], costumeFx->pchGeom );
				}

				// check if this geo name exists
				if (fxCapeGeoExists(tmp, fxinfo->conditions[0]->events[0]->capeFiles[0]->params[0]))
				{
					break;
				}
			}
			devassertmsg(i <= eaSize(&possibleChestNames), "Cape Harness Geo Missing: %s", tmp);

			return allocAddString(tmp);
		}
	}
	else
	{
		return costumeFx->pchGeom;
	}
}

FxParams *seqInitCostumeFxParams( Entity *e, const CostumePart *costumeFx, const BodyPart *bp, int numColors)
{
	static FxParams fxp;

	fxInitFxParams(&fxp);
	fxp.keys[0].seq	= e->seq;
	fxp.keys[0].bone = 0;
	fxp.keycount++;
	fxp.numColors = numColors;
	fxp.fxtype = FX_ENTITYFX;

	getCostumeFxColors(fxp.colors, costumeFx, e, numColors);

	fxp.textures[0] = costumeFx->pchTex1;
	fxp.textures[1] = costumeFx->pchTex2;
	fxp.textures[3] = costumeFx->pchTex2;
	fxp.geometry = getFxGeometry(e, costumeFx, bp);

	if (e == playerPtr()) {
		fxp.isPersonal = 1;
		fxp.isPlayer = 1;
	}
	return &fxp;
}

int seqDelAllCostumeFx(SeqInst *seq, int firstIndex)
{
	int i;
	for (i=eaiSize(&seq->seqcostumefx) - 1; i>=firstIndex; i--)
	{
		if (seq->seqcostumefx[i]) {
			fxDelete(seq->seqcostumefx[i], HARD_KILL);
			seq->seqcostumefx[i] = 0;
		}
		eaiSetSize(&seq->seqcostumefx, i);
	}
	return 1;
}

int seqAddCostumeFx(Entity *e, const CostumePart *costumeFx, const BodyPart *bp, int index, int numColors, int incompatibleFxFlags)
{
	bool empty = false;
	SeqInst * seq = e->seq;

	if( !costumeFx->pchFxName || !costumeFx->pchFxName[0] ||
		stricmp( costumeFx->pchFxName, "none" ) == 0 )
	{
		empty = true;
	}

	if(!seq)
	{
		Errorf( "addCostumeFx called on garbage entity");
		return 0;
	} 
	else 
	{
		bool addit=false;
		int size = eaiSize(&seq->seqcostumefx);
		if (!seq->seqcostumefx) eaiCreate(&seq->seqcostumefx);	// Auto-create array.
		if (index >= size)
		{
			eaiSetSize(&seq->seqcostumefx, index+1);
			addit = true;
		}
		if (!empty && !seq->seqcostumefx[index]) 
			addit = true;
		if (empty)
			addit = false;
		if (!addit)
		{
			// Check to see if it has different parameters than the existing one and either modify
			// them or delete and re-add
			if (seq->seqcostumefx[index])
			{
				bool killit=false;
				bool changeit=false;
				FxObject* oldfx = hdlGetPtrFromHandle(seq->seqcostumefx[index]);
				char buffer[MAX_PATH] = "";

				if(costumeFx->pchFxName)
					fxCleanFileName(buffer, costumeFx->pchFxName);
				if (!oldfx || !oldfx->fxinfo || empty || fxInfoHasFlags(oldfx->fxinfo, incompatibleFxFlags, true)) 
					killit = true;
				else if (stricmp(oldfx->fxinfo->name, buffer)!=0)
				{
					// Different FX
					killit = true;
				} 
				else
				{
					int j, i;
					// Compare FxParams value
					const char *oldtex, *newtex;
					U8 costumeFxColors[4][4];
					getCostumeFxColors(costumeFxColors, costumeFx, e, numColors);

					for (i=0; i<numColors; i++)
					{
						for (j=0; j<3; j++) 
						{
							if (oldfx->fxp.colors[i][j]!=costumeFxColors[i][j]) 
								changeit=true;
						}
					}

					oldtex = oldfx->fxp.textures[0];
					newtex = costumeFx->pchTex1;
					if (!!oldtex != !!newtex || // One is null and the other isn't
						oldtex && stricmp(oldtex, newtex)!=0) // or they're not NULL and they're different
					{
						changeit=true;
					}

					oldtex = oldfx->fxp.textures[1];
					newtex = costumeFx->pchTex2;
					if (!!oldtex != !!newtex || // One is null and the other isn't
						oldtex && stricmp(oldtex, newtex)!=0) // or they're not NULL and they're different
					{
						changeit=true;
					}

					if(oldfx->fxp.geometry != getFxGeometry(e, costumeFx, bp))
						killit = true;

					if (!killit && changeit)
					{
						// Texture changed, but we're not killing it, check to see if the old texture had
						//  alpha and this one doesn't
						// The secondary texture defines the cape shape, so that's the one we care about the
						//  alpha on
						bool oldHasAlpha=false, newHasAlpha=false;
						BasicTexture *bind;
						if (oldfx->fxp.textures[1]) 
						{
							bind = texFind(oldfx->fxp.textures[1]);
							if (bind && bind->flags & TEX_ALPHA)
								oldHasAlpha = true;
						}
						if (costumeFx->pchTex2) 
						{
							bind = texFind(costumeFx->pchTex2);
							if (bind && bind->flags & TEX_ALPHA)
								newHasAlpha = true;
						}

						if (newHasAlpha!=oldHasAlpha)
						{
							// Need to kill it!
							killit=true;
						}

					}
				}
				if (killit)
				{
					fxDelete(seq->seqcostumefx[index], HARD_KILL);
					seq->seqcostumefx[index] = 0;
					addit = true;
				} 
				else if (changeit)
				{
					// Parameter changed, but don't need to kill it
					FxParams *fxp = seqInitCostumeFxParams(e, costumeFx, bp, numColors);
					fxChangeParams( seq->seqcostumefx[index], fxp );
				}
			}
		}

		// check for incompatible flags, abort creation if find a match
		if(addit && incompatibleFxFlags)
		{
			FxInfo * fxinfo	= fxGetFxInfo(costumeFx->pchFxName);
			if( fxinfo && fxInfoHasFlags(fxinfo, incompatibleFxFlags, true))
				addit = false;
		}

		if (addit && !empty ) 
		{
			// Add it!
			FxParams *fxp = seqInitCostumeFxParams(e, costumeFx, bp, numColors);
			int newfx = fxCreate( costumeFx->pchFxName, fxp );
			if (newfx) 
			{
				seq->seqcostumefx[index] = newfx;
				return 1;
			} 
			else if( !(game_state.minimized || game_state.inactiveDisplay) ) 
				Errorf( "Failed to make %s ", costumeFx->pchFxName );
		}
	}
	return 0;
}

#endif

//Mild hack. If you are a villain, your seqtype is V_male, V_fem, or V_huge, not male, fem and huge
//But this is changed based on your alignment, so I do the check here, so if you change your alliance dynamically
//You will immediately start getting the right animations
//The bad part is that this a secret change of seqtype, and may be confusing
void calcSeqTypeName( SeqInst * seq, int alignment, int praetorian, int isPlayer )
{
	const char * name;

	if( !seq )
		return;

	name = seq->type->seqTypeName;

	Strncpyt( seq->calculatedSeqTypeName, name );

	// We don't change the seq name for Primal heroes, so there's no need to
	// do the compare and/or copy/cat.
	if( isPlayer && ( praetorian || alignment ) && ( 0==stricmp(name, "male" ) || 0==stricmp(name, "fem" ) || 0==stricmp(name, "huge" ) ) )
	{
		if( praetorian )
		{
			if( alignment == kPlayerType_Villain )
				strcpy( seq->calculatedSeqTypeName, "L_" );
			else
				strcpy( seq->calculatedSeqTypeName, "P_" );
		}
		else
		{
			if( alignment == kPlayerType_Villain )
				strcpy( seq->calculatedSeqTypeName, "V_" );
		}

		strcat( seq->calculatedSeqTypeName, name );
	}
}


void seqAssignBonescaleModels( SeqInst *seq, int *randSeed )
{
	PERFINFO_AUTO_START("assignBoneScaleAnims", 1);
	//Assign BoneScale Anms, if applicable
#if SERVER
	if(seqScaledOnServer(seq))
#endif
	{
		F32 bonescale_ratio;
#if CLIENT
		GeoLoadData * list;
		if( seq->type->bonescalefat)
		{
			list = geoLoad( seq->type->bonescalefat, LOAD_NOW, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
			if( list )
				seq->bonescale_anm_fat = &list->modelheader;
		}
		if( seq->type->bonescaleskinny )
		{
			list = geoLoad( seq->type->bonescaleskinny, LOAD_NOW, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
			if( list )
				seq->bonescale_anm_skinny = &list->modelheader;
		}
#endif
		if( seq->type->randombonescale )
			bonescale_ratio = qfrandFromSeed( randSeed );
		else
			bonescale_ratio = 0.0;

		assert( bonescale_ratio >= -1.0 && bonescale_ratio <= 1.0 );
		changeBoneScale( seq, bonescale_ratio ); //set to defaults

	}
	PERFINFO_AUTO_STOP();
}

//load_type is for SERVER / CLIENT differences.
//forceReload is for development mode regetting of changed animations when the seqInfo itself hasn't changed
static void seqResetSeqType( SeqInst * seq, const char * type_name, int load_type, int forceReload, int randSeed, Entity *entParent )
{
	seq->type = seqGetSeqType(type_name);

	calcSeqTypeName(seq,
					entParent && entParent->pl ? entParent->pl->playerType : kPlayerType_Hero,
					entParent && entParent->pl ? ENT_IS_IN_PRAETORIA(entParent) : 0,
					entParent && ENTTYPE(entParent)==ENTTYPE_PLAYER);

	//Set up seq->healthFfx and healthGeometryFx arrays, if needed //HealthFXMultiples
#ifdef CLIENT
	{
		int healthFxCount = eaSize(&seq->type->healthFx);
		if(healthFxCount)
		{
			eaiSetSize(&seq->healthFx, healthFxCount);
			eaiSetSize(&seq->healthGeometryFx, healthFxCount);
		}
		else 
		{
			eaiDestroy(&seq->healthFx);
			eaiDestroy(&seq->healthGeometryFx);
		}
	}
#endif
	//End healthfx

	//add random variety to this character's height, if applicable
	if(seq->type->geomscalemax[0] && seq->type->geomscalemax[1] && seq->type->geomscalemax[2])
	{
		Vec2 r;
		int i;
		F32 scale;

		scale = qfrandFromSeed( &randSeed ); //in lieu of a qufrandFromSeed function
		scale = ABS(scale);
		for( i = 0 ; i < 3 ; i++ )
		{
			r[0] = seq->type->geomscale[i];
			r[1] = seq->type->geomscalemax[i];

			seq->currgeomscale[i] = ((r[1] - r[0]) * scale) + r[0];
		}
	}
	else
	{
		copyVec3(seq->type->geomscale, seq->currgeomscale);
	}

	//Custom Scaling
	if(seq->currgeomscale[1])
		seq->curranimscale = (1.0 / seq->currgeomscale[1]);
	else
		seq->curranimscale = 1.0;

	//2. Get the SeqInfo (represents .seq and .geo files) and init the animation to ready
	
	PERFINFO_AUTO_START("seqGetSequencer", 1);
		seq->info = seqGetSequencer( seq->type->seqname, load_type, forceReload );
	PERFINFO_AUTO_STOP();
	
	if( !seq->info )
		FatalErrorf( "Error loading the entType file %s.  The sequencer %s doesn't exist or can't be loaded.", type_name, seq->type->seqname );

	seq->percentHealthLastCheck = -1;  //Say I've never checked this guy's health fx before

#if CLIENT
	copyVec3(seq->currgeomscale, seq->basegeomscale);

	if (forceReload) {
		// Cloth wind stuff, it was getting lost with /reloadseqs, this seems to have fixed it
		int num_moves = eaSize(&seq->info->moves);
		int j;
		for (j=0; j<num_moves; j++) {
			// @todo SHAREDMEM breaks const
			(cpp_const_cast(SeqMove*)(seq->info->moves[j]))->windInfo = getWindInfoConst(seq->info->moves[j]->windInfoName);
		}
	}
#endif

	seqClearState( seq->state );
	seq->animation.prev_frame = 0;

	PERFINFO_AUTO_START("seqProcessClientInst", 1);
		seqProcessClientInst( seq, 0.0, 0, 1 );
	PERFINFO_AUTO_STOP();

	seqAssignBonescaleModels(seq, &randSeed);

#if CLIENT
	//Set Extra Light
	seq->seqGfxData.light.ambientScale = 1.0;
	seq->seqGfxData.light.diffuseScale = 1.0;
#endif

	//3. Attach associated models, animations and any associated FX, init interpolation

#if SERVER
	if(seqScaledOnServer(seq))
#endif
		animSetHeader( seq, 0 );

#if CLIENT
	seqLoadInstCalls++;
	{
		int i;
		for(i = eaSize(&seq->type->fx)-1; i >=0; i-- )
		{
			if( seq->type->fx[i][0] ) //if any string is here
			{
				int newfx;
				
				PERFINFO_AUTO_START("seqCreateSeqTypeFx", 1);
					newfx = seqCreateSeqTypeFx( seq, seq->type->fx[i], entParent );
				PERFINFO_AUTO_STOP();
				
				if (newfx) {
					eaiPush(&seq->seqfx, newfx);
				} else if( !(game_state.minimized || game_state.inactiveDisplay) )
				{
					Errorf( "Failed to make %s ", seq->type->fx[i] );
				}
			}
		}
	}
#endif

	seqUpdateWorldGroupPtr(seq,entParent);
	seqUpdateGroupTrackers(seq,entParent);
}

#if CLIENT
void seqResetWorldFxStaticLighting(SeqInst *seq)
{
	fxResetStaticLighting(seq->worldGroupFx);
}
#endif

//HealthFXMultiples
char *  getRootHealthFxLibraryPiece( SeqInst * seq, int randomNumber )
{
	char * geometryName = 0;
	int i;

	if( seq && seq->type && seq->type->healthFx )
	{
		for( i = 0 ; i < eaSize( &seq->type->healthFx ) ; i++ )
		{
			HealthFx * healthFx = seq->type->healthFx[i];
			//If this is your full health geometry
			if( healthFx->range[1] > 0.99999 ) //In case of rounding errors
			{
				int geometryCount = eaSize( &healthFx->libraryPiece );
				if( geometryCount )
					geometryName = healthFx->libraryPiece[ randomNumber % geometryCount ];
			}
		}
	}
	return geometryName;
}

void seqUpdateWorldGroupPtr(SeqInst * seq, Entity *entParent)
{
	int drawAsFx = 1;
	GroupDef *def = NULL;
	const char *dynamicWorldGroup = 0;
	int randomNumber = 0;

	if( entParent )
	{
#ifdef SERVER
		randomNumber = (int)entParent->owner;
#else
		randomNumber = (int)entParent->svr_idx;
#endif
	}

	if( seq->type->flags & SEQ_USE_DYNAMIC_LIBRARY_PIECE )
	{
		dynamicWorldGroup = entParent->costumeWorldGroup;
	}
	else if( getRootHealthFxLibraryPiece( seq, randomNumber ) ) //For now it is unchangin, just using the first health entry//HealthFXMultiples
	{
		dynamicWorldGroup = getRootHealthFxLibraryPiece( seq, randomNumber ); //HealthFXMultiples
		//if( entParent->pchar ) //If this is an entity with hit points and such //Now all but dynmamic uses health fx
			drawAsFx = 0; //The health fx stuff handles drawing this, so don't do it here
	}

	if(dynamicWorldGroup)
	{
		def = groupDefFindWithLoad( dynamicWorldGroup );
		if( !def )
		{
			Errorf( "The library piece %s is not loaded or doesn't exist, but the enttype %s wants to make it.", dynamicWorldGroup, seq->type->name );
		}
	}

	if( def )
	{
#if CLIENT
		if( drawAsFx )
		{
			FxParams fxp;

			PERFINFO_AUTO_START("fxInitFxParams", 1);
			fxInitFxParams(&fxp);
			PERFINFO_AUTO_STOP();

			fxp.keys[0].seq	= seq;
			fxp.keys[0].bone = 0;
			fxp.keycount++;
			fxp.externalWorldGroupPtr = def;
			//fxp.fxtype = ???;
			if( seq->worldGroupFx )
				fxDelete(seq->worldGroupFx, HARD_KILL);
			seq->worldGroupFx = fxCreate( "HardCoded/WorldGroup/WorldGroup.fx", &fxp );
			fxSetUseStaticLighting(seq->worldGroupFx, seq->type->useStaticLighting);
			if( !seq->worldGroupFx )
				Errorf( "Couldn't create hardcoded worldGroup fx. Old data?" );
		}
#endif
		//TO DO does this work properly when you zone? (Doors don't zone, but still)
		seq->worldGroupPtr = def; //record this in case you need it for collisions

		// resetting the collision
		entMotionFreeColl(entParent);
	}
}

void seqUpdateGroupTrackers(SeqInst *seq, Entity *entParent)
{
	if( seq->type->flags & SEQ_USE_DYNAMIC_LIBRARY_PIECE )
	{
		// Stupid ptr may have been set to null just before an entfree (where this needs to be called)
		//  so we might need to fix it up just so we know during the entfree to reset the group trackers
		if (!seq->worldGroupPtr)
			seqUpdateWorldGroupPtr(seq,entParent);

		if (seq->worldGroupPtr
			&& seq->worldGroupPtr->child_properties // Take this out to be more defensive?
			&& seq->worldGroupPtr->child_volume_trigger)
		{
			groupTrackersHaveBeenReset();
		}
	}
}


void seqClearExtraGraphics( SeqInst * seq, int for_reinit, int hard_kill )
{
#ifdef CLIENT
	int i;
	int killType = hard_kill?HARD_KILL:SOFT_KILL;
	for( i = eaiSize(&seq->seqfx)-1; i >= 0; i--)
	{
		fxDelete(seq->seqfx[i], killType);
	}
	eaiDestroy(&seq->seqfx);
	if (!for_reinit) {
		for( i = eaiSize(&seq->seqcostumefx)-1; i >= 0; i--)
		{
			// HACK: this assumes seqcostumefx[i] corresponds to bodyPartList.bodyParts[i]
			//	dont_clear is only set on body parts with a relatively high index,
			//	so this should only affect costumes that have a 1-1 mapping with bodyParts
			if(!bodyPartList.bodyParts[i]->dont_clear)
			{
				fxDelete(seq->seqcostumefx[i], HARD_KILL);
				seq->seqcostumefx[i] = 0;
			}
		}
		eaiDestroy(&seq->seqfx);
	}

	for( i = 0 ; i < MAX_SEQFX ; i++ )
	{
		if( seq->const_seqfx[i] )
		{
			fxDelete( seq->const_seqfx[i], killType );
			seq->const_seqfx[i] = 0;
		}
	}

	fxDelete( seq->worldGroupFx, killType );
	fxDelete( seq->volume_fx, killType );
	fxDelete( seq->onClickFx, killType );

	//HealthFXMultiples
	for( i = 0 ; i < eaiSize( &seq->healthFx ) ; i++ )
	{
		if( seq->healthFx[i] )
		{
			fxDelete( seq->healthFx[i], killType );
			seq->healthFx[i] = 0;
		}
	}

	for( i = 0 ; i < eaiSize( &seq->healthGeometryFx ) ; i++ )
	{
		if( seq->healthGeometryFx[i] )
		{
			fxDelete( seq->healthGeometryFx[i], killType );
			seq->healthGeometryFx[i] = 0;
		}
	}


	if( seq->simpleShadow && seq->simpleShadowUniqueId == seq->simpleShadow->unique_id )
	{
		gfxTreeDelete( seq->simpleShadow );
		seq->simpleShadow = 0;
		seq->simpleShadowUniqueId = 0;
	}

#endif
}

//I have to reload all the seqeucners when this is pressed because I don't
//know if the animations have changed. But I only have to do it once per
//frame, so alreadyReloadedSeqInfos remembers which i have already reloaded
StringTable strReloadedSeqInfos;

void seqPrepForReinit()
{
	if ( strReloadedSeqInfos )
	{
		destroyStringTable(strReloadedSeqInfos);
		strReloadedSeqInfos = NULL;
	}
}

//Regets seqInfos, seqTypes and animations on a sequencer
int seqReinitSeq( SeqInst * seq, int loadType, int randSeed, Entity *entParent )
{
	int forceReload, i;


	//I force reload sequencer files once when seqReinitSeq forces the reInitting of all seqTypes because
	//the anim file dependencies could have changed, too.
	//Only do this once per seq reinitting cycle

	if ( !strReloadedSeqInfos )
	{
		strReloadedSeqInfos = createStringTable();
		initStringTable(strReloadedSeqInfos, 128);
		strTableSetMode(strReloadedSeqInfos, Indexable);
	}


	forceReload = SEQ_RELOAD_FOR_DEV;
	for( i = 0 ; i < strTableGetStringCount(strReloadedSeqInfos) ; i++ )
	{
		if( 0 == stricmp( seq->info->name, strTableGetString(strReloadedSeqInfos, i) ) )
		{
			forceReload = 0;
			break;
		}
	}
	if( forceReload == SEQ_RELOAD_FOR_DEV )
	{
		strTableAddString(strReloadedSeqInfos, seq->info->name);
	}

	//Only through seqReinitSeq will seqInfos that have been loaded but not
	//themselves been changed get any new animations. forceReload does this.
	printf("...reloading %s ", seq->type->name );
	seqClearExtraGraphics( seq, 1, 0 );

	seqResetSeqType( seq, seq->type->name, loadType, forceReload, randSeed, entParent );


	return 1; //I always reload it right now
}

// If the sequencer has a Dynamic worldgroup, this will set or change it
int seqChangeDynamicWorldGroup(const char *pchWorldGroup, Entity *entParent)
{
	if(entParent->costumeWorldGroup)
	{
		if(stricmp(pchWorldGroup, entParent->costumeWorldGroup)==0)
		{
			// It's the same, just return
			return 1;
		}
		else
		{
			// May be switching between costumes with/without volumes
			if(entParent->seq)
				seqUpdateGroupTrackers(entParent->seq, entParent);

			entMotionFreeColl(entParent);

			SAFE_FREE(entParent->costumeWorldGroup);
		}
	}

	entParent->costumeWorldGroup = strdup(pchWorldGroup);

	if(entParent->seq)
	{
#ifdef SERVER 
		seqReinitSeq(entParent->seq, SEQ_LOAD_FULL_SHARED, entParent->randSeed, entParent);
#else
		seqReinitSeq(entParent->seq, SEQ_LOAD_FULL, entParent->randSeed, entParent);
#endif
	}

#if SERVER
	entParent->send_costume = 1;
#endif

	return 1;
}



void seqInitSeqMemPool()
{
	if( seqMp )
		destroyMemoryPoolGfxNode(seqMp);
	seqMp = createMemoryPool();
	initMemoryPool( seqMp, sizeof(SeqInst), SEQ_MEMPOOL_SIZE );
}

/* allocates and initializes a new sequencer */
static SeqInst *seqAllocateNewSeq( GfxNode * parent )
{
SeqInst		*seq;

	if( !seqMp )
		seqInitSeqMemPool();

	seq = mpAlloc(seqMp);

	//xyprintf( 50, 50, "%d",	mpGetAllocatedCount(seqMp) );

	//seq = calloc(sizeof(SeqInst),1);

	seq->anim_incrementor = (U8)rand();

	seq->gfx_root = gfxTreeInsert( parent );
	assert(seq->gfx_root);
	seq->gfx_root_unique_id = seq->gfx_root->unique_id;

	copyMat4(unitmat,seq->gfx_root->mat);

#ifdef CLIENT
	seq->handle = hdlAssignHandle(seq); //handle is only used by fx on client right now
	assert(seq->handle);
	seq->move_predict_seed = 10;
	seq->alphasvr  = 255;
	seq->alphacam  = 255;
	seq->alphadist = 255;
	seq->alphafade = 255;
	seq->alphafx   = 255;
	memset(seq->newcolor1, 0xff, sizeof(seq->newcolor1));
	memset(seq->newcolor2, 0xff, sizeof(seq->newcolor2));
#endif

	return seq;
}

SeqInst * seqLoadInst( const char * type_name, GfxNode * parent, int load_type, int randSeed, Entity *entParent )
{
	SeqInst	* seq;

	PERFINFO_AUTO_START("seqLoadInst", 1);
		PERFINFO_AUTO_START("seqAllocateNewSeq", 1);
			//0. Create the SeqInst (all 3 Load functions take full paths)
			seq = seqAllocateNewSeq( parent );
			assert(seq);
		PERFINFO_AUTO_STOP_START("seqResetSeqType", 1);
			//1. Get the SeqType, SeqInfo and set up all the basics that go with that
			seqResetSeqType( seq, type_name, load_type, 0, randSeed, entParent  ); //dont force reload
		PERFINFO_AUTO_STOP();
	PERFINFO_AUTO_STOP();

	return seq;
}

SeqInst * seqFreeInst(SeqInst * seq)
{
	if (!seq)
		return 0;

	freeLastBoneTransforms(seq);
	
	seqClearExtraGraphics( seq, 0, 1 );

#ifdef CLIENT
	{
		int i;
		for(i = 0; i < ARRAY_SIZE(seq->rgbs_array); i++)
			if( seq->rgbs_array[i] )
				modelFreeRgbBuffer(seq->rgbs_array[i]);
	}
	hdlClearHandle(seq->handle); //currently only of interest to client fx

	if( seq->healthFx ) //HealthFXMultiples
		eaiDestroy( &seq->healthFx );
	if( seq->healthGeometryFx )
		eaiDestroy( &seq->healthGeometryFx );

	if (seq->seqcostumefx)
	{
		seqDelAllCostumeFx(seq, 0);
		eaiDestroy(&seq->seqcostumefx);
	}

#else
	SAFE_FREE(seq->pbonescalesAAA);
#endif

	if( gfxTreeNodeIsValid(seq->gfx_root, seq->gfx_root_unique_id ) )
		gfxTreeDelete(seq->gfx_root);
	seqFreeInstCalls++;

	memset(seq,0,sizeof(SeqInst));
	mpFree( seqMp, seq );
	//free( seq );
	return 0;
}

int seqIsMoveSelectable( SeqInst * seq )
{
	if( seq && seq->animation.move && (seq->animation.move->flags & SEQMOVE_NOTSELECTABLE) )
		return 0;
	return 1;
}

//
void seqGetCapsuleSize(SeqInst * seq, Vec3 size, Vec3 offset)
{
	if( seq )
	{
		if( size )
			copyVec3(seq->type->capsuleSize, size);
		if( offset )
			copyVec3(seq->type->capsuleOffset, offset );
	}
}

void seqPreLoadPlayerCharacterAnimations(int load_type)
{
	seqGetSequencer("player.txt", load_type, 0);
}


/*  - Testing function, should never have to do this
void seqPreLoadAllAnimations(int load_type)
{
	seqGetSequencer("5cw.txt", load_type, 0);
	seqGetSequencer("Amoeba.txt", load_type, 0);
	seqGetSequencer("arachnos_seq_disruptor_screws.txt", load_type, 0);
	seqGetSequencer("arachnos_seq_sideguns.txt", load_type, 0);
	seqGetSequencer("arachnos_seq_sideguns_steve.txt", load_type, 0);
	seqGetSequencer("Beast.txt", load_type, 0);
	seqGetSequencer("BF.txt", load_type, 0);
	seqGetSequencer("bird.txt", load_type, 0);
	seqGetSequencer("bis.txt", load_type, 0);
	seqGetSequencer("Boat_3.txt", load_type, 0);
	seqGetSequencer("briefcase_explode.txt", load_type, 0);
	seqGetSequencer("Brute_explosions.txt", load_type, 0);
	seqGetSequencer("Car_Compact.txt", load_type, 0);
	seqGetSequencer("Car_Lux1.txt", load_type, 0);
	seqGetSequencer("Car_Lux2.txt", load_type, 0);
	seqGetSequencer("Car_Sport1.txt", load_type, 0);
	seqGetSequencer("Car_Sport2.txt", load_type, 0);
	seqGetSequencer("Car_Truck.txt", load_type, 0);
	seqGetSequencer("cav_door01.txt", load_type, 0);
	seqGetSequencer("contact_BM.txt", load_type, 0);
	seqGetSequencer("Copy of Shark.txt", load_type, 0);
	seqGetSequencer("crab_backpack.txt", load_type, 0);
	seqGetSequencer("daemon.txt", load_type, 0);
	seqGetSequencer("Damaged_Crater.txt", load_type, 0);
	seqGetSequencer("Damaged_piles.txt", load_type, 0);
	seqGetSequencer("Damaged_Props.txt", load_type, 0);
	seqGetSequencer("Damaged_Vehicle.txt", load_type, 0);
	seqGetSequencer("def.txt", load_type, 0);
	seqGetSequencer("Devouring_Rock_Wall.txt", load_type, 0);
	seqGetSequencer("door1.txt", load_type, 0);
	seqGetSequencer("Door_5col.txt", load_type, 0);
	seqGetSequencer("Door_5col_Mission.txt", load_type, 0);
	seqGetSequencer("Door_cave.txt", load_type, 0);
	seqGetSequencer("Door_cave_02.txt", load_type, 0);
	seqGetSequencer("Door_Cave_Mission.txt", load_type, 0);
	seqGetSequencer("Door_Cot.txt", load_type, 0);
	seqGetSequencer("Door_Cot_Slanted.txt", load_type, 0);
	seqGetSequencer("Door_Cot_wood.txt", load_type, 0);
	seqGetSequencer("Door_doctor.txt", load_type, 0);
	seqGetSequencer("Door_elevator.txt", load_type, 0);
	seqGetSequencer("Door_glass.txt", load_type, 0);
	seqGetSequencer("Door_hazard.txt", load_type, 0);
	seqGetSequencer("Door_iris.txt", load_type, 0);
	seqGetSequencer("Door_keepout.txt", load_type, 0);
	seqGetSequencer("Door_Left_City.txt", load_type, 0);
	seqGetSequencer("Door_reclpad.txt", load_type, 0);
	seqGetSequencer("Door_Right_City.txt", load_type, 0);
	seqGetSequencer("Door_Rocks.txt", load_type, 0);
	seqGetSequencer("Door_Rollup_med.txt", load_type, 0);
	seqGetSequencer("Door_safe.txt", load_type, 0);
	seqGetSequencer("Door_sewer1.txt", load_type, 0);
	seqGetSequencer("Door_sewer2.txt", load_type, 0);
	seqGetSequencer("Door_sewer_int.txt", load_type, 0);
	seqGetSequencer("Door_Shop01.txt", load_type, 0);
	seqGetSequencer("Door_Spider.txt", load_type, 0);
	seqGetSequencer("Door_tech01.txt", load_type, 0);
	seqGetSequencer("Door_tech02.txt", load_type, 0);
	seqGetSequencer("Door_techelevator.txt", load_type, 0);
	seqGetSequencer("Door_TEST.txt", load_type, 0);
	seqGetSequencer("Door_train.txt", load_type, 0);
	seqGetSequencer("Door_Truck.txt", load_type, 0);
	seqGetSequencer("Door_warehouse01.txt", load_type, 0);
	seqGetSequencer("Door_Warp.txt", load_type, 0);
	seqGetSequencer("Door_WepShop.txt", load_type, 0);
	seqGetSequencer("enemy.txt", load_type, 0);
	seqGetSequencer("Faathim_Imprison.txt", load_type, 0);
	seqGetSequencer("flyaway_seagull.txt", load_type, 0);
	seqGetSequencer("FX_5th_ballturret.txt", load_type, 0);
	seqGetSequencer("FX_Arachno_police_Legs.txt", load_type, 0);
	seqGetSequencer("FX_banner.txt", load_type, 0);
	seqGetSequencer("FX_BellagioWater.txt", load_type, 0);
	seqGetSequencer("FX_billows.txt", load_type, 0);
	seqGetSequencer("FX_birdwings.txt", load_type, 0);
	seqGetSequencer("FX_BodyBag.txt", load_type, 0);
	seqGetSequencer("FX_BodyBagLinger.txt", load_type, 0);
	seqGetSequencer("FX_bow.txt", load_type, 0);
	seqGetSequencer("FX_brain.txt", load_type, 0);
	seqGetSequencer("FX_cape.txt", load_type, 0);
	seqGetSequencer("FX_clawspeed.txt", load_type, 0);
	seqGetSequencer("FX_cot_props.txt", load_type, 0);
	seqGetSequencer("FX_dark.txt", load_type, 0);
	seqGetSequencer("FX_darktenticle.txt", load_type, 0);
	seqGetSequencer("FX_darktenticle_A.txt", load_type, 0);
	seqGetSequencer("FX_darktenticle_B.txt", load_type, 0);
	seqGetSequencer("FX_Devour.txt", load_type, 0);
	seqGetSequencer("FX_Diesel_wake.txt", load_type, 0);
	seqGetSequencer("FX_doug.txt", load_type, 0);
	seqGetSequencer("FX_electro_cage.txt", load_type, 0);
	seqGetSequencer("FX_gearstaff.txt", load_type, 0);
	seqGetSequencer("FX_goblin.txt", load_type, 0);
	seqGetSequencer("FX_GraspingEarth.txt", load_type, 0);
	seqGetSequencer("FX_head.txt", load_type, 0);
	seqGetSequencer("FX_heavyclaw.txt", load_type, 0);
	seqGetSequencer("FX_heavypilot.txt", load_type, 0);
	seqGetSequencer("FX_Horse.txt", load_type, 0);
	seqGetSequencer("FX_hoverbot.txt", load_type, 0);
	seqGetSequencer("FX_Icicles.txt", load_type, 0);
	seqGetSequencer("FX_lifterswirl.txt", load_type, 0);
	seqGetSequencer("FX_Natterling_fuse.txt", load_type, 0);
	seqGetSequencer("FX_Natterling_melt.txt", load_type, 0);
	seqGetSequencer("FX_Natterling_transform.txt", load_type, 0);
	seqGetSequencer("FX_Natterling_wings.txt", load_type, 0);
	seqGetSequencer("FX_newspaper.txt", load_type, 0);
	seqGetSequencer("FX_New_bow.txt", load_type, 0);
	seqGetSequencer("FX_Nova_Jaw.txt", load_type, 0);
	seqGetSequencer("FX_Nova_Pads.txt", load_type, 0);
	seqGetSequencer("FX_Nova_Tail.txt", load_type, 0);
	seqGetSequencer("FX_pigeon.txt", load_type, 0);
	seqGetSequencer("FX_poisonspike.txt", load_type, 0);
	seqGetSequencer("FX_policedrone.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst1.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst2.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst3.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst4.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst5.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst6.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst_large.txt", load_type, 0);
	seqGetSequencer("FX_QuillBurst_large2.txt", load_type, 0);
	seqGetSequencer("FX_reclpad.txt", load_type, 0);
	seqGetSequencer("Fx_RiktiSword02.txt", load_type, 0);
	seqGetSequencer("Fx_RiktiSword_02.txt", load_type, 0);
	seqGetSequencer("FX_Rikti_destroyer.txt", load_type, 0);
	seqGetSequencer("FX_RobotSpawn.txt", load_type, 0);
	seqGetSequencer("FX_seagull.txt", load_type, 0);
	seqGetSequencer("FX_seagull_flock.txt", load_type, 0);
	seqGetSequencer("FX_Sentinel_EYE.txt", load_type, 0);
	seqGetSequencer("FX_Shield.txt", load_type, 0);
	seqGetSequencer("FX_sign_big.txt", load_type, 0);
	seqGetSequencer("FX_sign_sm.txt", load_type, 0);
	seqGetSequencer("FX_Skiff.txt", load_type, 0);
	seqGetSequencer("FX_Skull.txt", load_type, 0);
	seqGetSequencer("FX_spotlight.txt", load_type, 0);
	seqGetSequencer("FX_sprocket_spawn.txt", load_type, 0);
	seqGetSequencer("FX_stonearmor.txt", load_type, 0);
	seqGetSequencer("FX_tech1.txt", load_type, 0);
	seqGetSequencer("FX_teleattack.txt", load_type, 0);
	seqGetSequencer("FX_teleportbase.txt", load_type, 0);
	seqGetSequencer("FX_Temp_Collision.txt", load_type, 0);
	seqGetSequencer("FX_tenticlebase.txt", load_type, 0);
	seqGetSequencer("FX_Titan_transform.txt", load_type, 0);
	seqGetSequencer("FX_Titian.txt", load_type, 0);
	seqGetSequencer("FX_troll_props.txt", load_type, 0);
	seqGetSequencer("FX_turret_flamethrower.txt", load_type, 0);
	seqGetSequencer("FX_WilloWisps.txt", load_type, 0);
	seqGetSequencer("FX_WilloWisps2.txt", load_type, 0);
	seqGetSequencer("FX_wings.txt", load_type, 0);
	seqGetSequencer("Giant.txt", load_type, 0);
	seqGetSequencer("goblin.txt", load_type, 0);
	seqGetSequencer("Hamidon.txt", load_type, 0);
	seqGetSequencer("Hamidon_Mito.txt", load_type, 0);
	seqGetSequencer("hoverbot.txt", load_type, 0);
	seqGetSequencer("Hydra_Pod.txt", load_type, 0);
	seqGetSequencer("Kheldian_Nova.txt", load_type, 0);
	seqGetSequencer("Mannequin.txt", load_type, 0);
	seqGetSequencer("Mblur.txt", load_type, 0);
	seqGetSequencer("Millitary_truck.txt", load_type, 0);
	seqGetSequencer("Mu_seq.txt", load_type, 0);
	seqGetSequencer("Ndrone.txt", load_type, 0);
	seqGetSequencer("NPC_doctor.txt", load_type, 0);
	seqGetSequencer("npc_swat.txt", load_type, 0);
	seqGetSequencer("Octopus.txt", load_type, 0);
	seqGetSequencer("Octopus_Head.txt", load_type, 0);
	seqGetSequencer("plant_root.txt", load_type, 0);
	seqGetSequencer("player.txt", load_type, 0);
	seqGetSequencer("Police_Drone.txt", load_type, 0);
	seqGetSequencer("Prop_basic.txt", load_type, 0);
	seqGetSequencer("Prop_basic_NoColl.txt", load_type, 0);
	seqGetSequencer("Prop_Garbage_Can.txt", load_type, 0);
	seqGetSequencer("Prop_Mine.txt", load_type, 0);
	seqGetSequencer("pump_01.txt", load_type, 0);
	seqGetSequencer("Rikti_dropship.txt", load_type, 0);
	seqGetSequencer("Rikti_Flesh.txt", load_type, 0);
	seqGetSequencer("Rikti_generator.txt", load_type, 0);
	seqGetSequencer("Rikti_Monkey.txt", load_type, 0);
	seqGetSequencer("Sally.txt", load_type, 0);
	seqGetSequencer("Scam_01.txt", load_type, 0);
	seqGetSequencer("search_light.txt", load_type, 0);
	seqGetSequencer("Sentinel.txt", load_type, 0);
	seqGetSequencer("Shark.txt", load_type, 0);
	seqGetSequencer("Skiff_pilot.txt", load_type, 0);
	seqGetSequencer("Sky_skiff.txt", load_type, 0);
	seqGetSequencer("SnakeMan.txt", load_type, 0);
	seqGetSequencer("Snakemen_seq.txt", load_type, 0);
	seqGetSequencer("Snakemen_steve.txt", load_type, 0);
	seqGetSequencer("Spider_seq.txt", load_type, 0);
	seqGetSequencer("ss_monument.txt", load_type, 0);
	seqGetSequencer("static.txt", load_type, 0);
	seqGetSequencer("steve.txt", load_type, 0);
	seqGetSequencer("swat.txt", load_type, 0);
	seqGetSequencer("tank_A.txt", load_type, 0);
	seqGetSequencer("Target.txt", load_type, 0);
	seqGetSequencer("target_drone.txt", load_type, 0);
	seqGetSequencer("Tentacle.txt", load_type, 0);
	seqGetSequencer("Tentacle_head.txt", load_type, 0);
	seqGetSequencer("Tentacle_ranged.txt", load_type, 0);
	seqGetSequencer("Tentacle_ranged_small.txt", load_type, 0);
	seqGetSequencer("Tentacle_small.txt", load_type, 0);
	seqGetSequencer("Tree.txt", load_type, 0);
	seqGetSequencer("tscroll.txt", load_type, 0);
	seqGetSequencer("Turrets.txt", load_type, 0);
	seqGetSequencer("Turrets_Ball.txt", load_type, 0);
	seqGetSequencer("Turrets_Base.txt", load_type, 0);
	seqGetSequencer("Turrets_Missle.txt", load_type, 0);
	seqGetSequencer("Vamp_chamber.txt", load_type, 0);
	seqGetSequencer("zombie.txt", load_type, 0);

}

*/
/*
int seqCheckGeometry( SeqInst * seq )
{
	int i;
	if( seq )
	{
		if ( seq->type->gfxname[0][0] == 0 )
		{
			//No default geometry given
		}
		else if ( !geoLoad( seq->type->graphics, LOAD_NOW, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE) )
		{
			//Bad default geometry given
		}


		for( i = 0 ; i > MAX_BONES ; i++ )
		{
			if( seq->newgeometry[i] == 0 || stricmp( seq->newgeometry[i], "NONE" ) )
			{
				//No geometry on this bone
			}
			else
			{
				if( !modelFind( seq->newgeometry[i] ) )
				{
					//Bad Geometry on this bone
				}
			}
		}
	}
}
*/


#ifdef CLIENT
const float* seqBoneScale(const SeqInst* seq, BoneId bone)
{
	assert(AINRANGE(bone, seq->bonescalesAAA));
	return seq->bonescalesAAA[bone];
}

void seqClearBoneScales(SeqInst* seq)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(seq->bonescalesAAA); ++i)
		copyVec3(onevec3, seq->bonescalesAAA[i]);
}

void seqSetBoneScale(SeqInst* seq, BoneId bone, Vec3 vToSet)
{
	assert(AINRANGE(bone, seq->bonescalesAAA));
	copyVec3(vToSet, seq->bonescalesAAA[bone]);
}
#else
const float* seqBoneScale(const SeqInst* seq, BoneId bone)
{
	assert(seq->pbonescalesAAA && INRANGE0(bone, BONEID_COUNT));
	return seq->pbonescalesAAA[bone];
}

void seqClearBoneScales(SeqInst* seq)
{
	int i;
	if(!seq->pbonescalesAAA)
		seq->pbonescalesAAA = malloc(sizeof(Vec3)*BONEID_COUNT);
	for(i = 0; i < BONEID_COUNT; ++i)
		copyVec3(onevec3, seq->pbonescalesAAA[i]);
}

void seqSetBoneScale(SeqInst* seq, BoneId bone, Vec3 vToSet)
{
	assert(seq->pbonescalesAAA && INRANGE0(bone, BONEID_COUNT));
	copyVec3(vToSet, seq->pbonescalesAAA[bone]);
}
#endif

#define LBT_MEM_POOL 10
MP_DEFINE(LastBoneTransforms);

void allocateLastBoneTransforms(SeqInst* seq)
{
	if ( seq->pLastBoneTransforms )
		return;
	MP_CREATE(LastBoneTransforms, 10);
	seq->pLastBoneTransforms = MP_ALLOC(LastBoneTransforms);
}

void freeLastBoneTransforms(SeqInst* seq)
{
	if ( !seq->pLastBoneTransforms )
		return;
	MP_FREE(LastBoneTransforms, seq->pLastBoneTransforms);
}

void seqGetLastRotations(SeqInst* seq, BoneId bone, Quat q)
{
	assert( seq->pLastBoneTransforms != NULL );
	assert(bone_IdIsValid(bone));
	copyQuat(seq->pLastBoneTransforms->last_rotations[bone], q);
}

void seqSetLastRotations(SeqInst* seq, BoneId bone, const Quat q)
{
	assert( seq->pLastBoneTransforms != NULL );
	assert(bone_IdIsValid(bone));
	copyQuat(q, seq->pLastBoneTransforms->last_rotations[bone]);
}

float* seqLastTranslations(SeqInst* seq, BoneId bone)
{
	assert( seq->pLastBoneTransforms != NULL );
	assert(bone_IdIsValid(bone));
	return seq->pLastBoneTransforms->last_xlates[bone];
}

void seqSetBoneAnimationStored(SeqInst* seq, BoneId bone)
{
	assert(bone_IdIsValid(bone));
	SETB(seq->uiBoneAnimationStored, bone);
}

bool seqIsBoneAnimationStored(SeqInst* seq, BoneId bone)
{
	assert(bone_IdIsValid(bone));
	return	TSTB(seq->uiBoneAnimationStored, bone) ||
			TSTB(seq->uiBoneAnimationStoredLastFrame, bone);
}

void seqClearAllBoneAnimationStoredBits(SeqInst* seq)
{
	CopyArray(seq->uiBoneAnimationStoredLastFrame, seq->uiBoneAnimationStored);
	ZeroArray(seq->uiBoneAnimationStored);
}

void seqSetBoneNeedsRotationBlending(SeqInst* seq, BoneId bone, U8 uiSet)
{
	assert(bone_IdIsValid(bone));
	if ( uiSet )
		SETB(seq->uiBoneRotationBlendingBitField, bone);
	else
		CLRB(seq->uiBoneRotationBlendingBitField, bone);
}

void seqSetBoneNeedsTranslationBlending(SeqInst* seq, BoneId bone, U8 uiSet)
{
	assert(bone_IdIsValid(bone));
	if ( uiSet )
		SETB(seq->uiBoneTranslationBlendingBitField, bone);
	else
		CLRB(seq->uiBoneTranslationBlendingBitField, bone);
}

bool seqBoneNeedsRotationBlending(SeqInst* seq, BoneId bone)
{
	assert(bone_IdIsValid(bone));
	return TSTB(seq->uiBoneRotationBlendingBitField, bone);
}

bool seqBoneNeedsTranslationBlending(SeqInst* seq, BoneId bone)
{
	assert(bone_IdIsValid(bone));
	return TSTB(seq->uiBoneTranslationBlendingBitField, bone);
}


void seqClearAllBlendingBits(SeqInst* seq)
{
	ZeroArray(seq->uiBoneRotationBlendingBitField);
	ZeroArray(seq->uiBoneTranslationBlendingBitField);
}


void manageEntityHitPointTriggeredfx( Entity * e )
{
	SeqInst * seq = e->seq;
	int costumeCritter = 0;

#ifdef CLIENT
	{
		extern int gCostumeCritter;
		costumeCritter = gCostumeCritter;
	}
#endif

	if( seq->type->healthFx && !costumeCritter )
	{
		float fPercentHealth = 1.0;
		int randomNumber = 0; 

		if( e->pchar ) //HealthFXMultiples
			fPercentHealth = e->pchar->attrCur.fHitPoints/e->pchar->attrMax.fHitPoints;

#ifdef SERVER
		randomNumber = (int)e->owner;//using e->owner/svr_idx as a random number because it'll be the same for all clients
#else 
		randomNumber = (int)e->svr_idx;
#endif
		seqManageHitPointTriggeredFx( seq,  fPercentHealth, randomNumber ); 
	}
}

static GfxNode* s_seqFindGfxNodeGivenBoneNumRecur(BoneId bonenum, int seqhandle, GfxNode *node, int requireChild)
{
	for(; node; node = node->next)
	{
		if(node->anim_id == bonenum && (requireChild ? seqhandle != node->seqHandle : seqhandle == node->seqHandle))
			return node;

		if(node->child)
		{
			GfxNode *found = s_seqFindGfxNodeGivenBoneNumRecur(bonenum, seqhandle, node->child, requireChild);
			if(found)
				return found;
		}
	}

	return NULL;
}

GfxNode* seqFindGfxNodeGivenBoneNum(const SeqInst * seq, BoneId bonenum)
{
	if(!seq || !seq->gfx_root || !seq->gfx_root->child)
		return NULL;

	if(!bone_IdIsValid(bonenum))
		return NULL;

	return s_seqFindGfxNodeGivenBoneNumRecur(bonenum, seq->handle, seq->gfx_root->child, 0);
}

GfxNode* seqFindGfxNodeGivenBoneNumOfChild(const SeqInst * seq, BoneId bonenum, bool requireChild)
{
	if(!seq || !seq->gfx_root || !seq->gfx_root->child)
		return NULL;

	if(!bone_IdIsValid(bonenum))
		return NULL;

	return s_seqFindGfxNodeGivenBoneNumRecur(bonenum, seq->handle, seq->gfx_root->child, requireChild);
}

void seqGetBoneLine(Vec3 pos, Vec3 dir, F32 *len, const SeqInst *seq, BoneId bone)
{
	GfxNode *node = seqFindGfxNodeGivenBoneNum(seq, bone);

	Vec3 tip[2], tail[2];	// workspace
	int idx = 0;			// workspace index

	// FIXME: should we use a sibling of node->child instead?
	if(!node || !node->child)
	{
		*len = 0;
		return;
	}

	copyVec3(node->child->mat[3], tip[idx]);
	zeroVec3(tail[idx]);

	// walk up the nodes to find the world coordinates of tip and tail
	for(;; node = node->parent)
	{
		mulVecMat4(tip[idx], node->mat, tip[!idx]);
		mulVecMat4(tail[idx], node->mat, tail[!idx]);
		idx = !idx;
		if(node == seq->gfx_root)
			break;
	}

	copyVec3(tail[idx], pos);
	subVec3(tip[idx], tail[idx], dir);
	*len = normalVec3(dir);
}

F32 seqCapToBoneCap(Vec3 src_pos, Vec3 src_dir, F32 src_len, F32 src_rad, SeqInst *target_seq,
					Vec3 center, Vec3 surface, F32 *radius)
{
	int i;
	Vec3 cent_buf;
	Vec3 surf_buf;
	F32 dist = -1.f;

	if(!center)
		center = cent_buf;
	if(!surface)
		surface = surf_buf;

	// FIXME: this could have an early out if the entity capsules are very far apart.

	for(i = 0; i < eaSize(&target_seq->type->boneCapsules); i++)
	{
		BoneCapsule *bone = target_seq->type->boneCapsules[i];

		Vec3 target_pos, target_dir;
		Vec3 source_nearest;
		F32 target_len;
		F32 distline, distgap;

		if(!bone->radius)
			continue;

		seqGetBoneLine(target_pos, target_dir, &target_len, target_seq, bone->bone);

		distline = coh_LineLineDist(target_pos,	target_dir,	target_len,	center,
									src_pos,	src_dir,	src_len,	source_nearest );
		distline = sqrtf(distline); // ugh, i know this is slow, but we want to find the nearest surface, right?
		distgap = distline
				- src_rad
				- bone->radius;
		distgap = MAX(distgap, 0.f);

		if(dist < 0.f || distgap < dist)
		{
			subVec3(source_nearest, center, surface);
			scaleAddVec3(surface, bone->radius / distline, center, surface);

			// target_nearest is faster, but doesn't look very good
			// copyVec3(target_nearest, location);

			dist = distgap;
			if(radius)
				*radius = bone->radius;
		}
	}

	return dist;
}
