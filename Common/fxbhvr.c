#include "fxbhvr.h"
#include <string.h>
#include "genericlist.h"
#include "utils.h"
#include "earray.h"
#include "font.h"
#include "time.h"
#include "fileutil.h"
#include "error.h"
#include "tricks.h"
#include "textparser.h"
#include "StashTable.h"
#include "strings_opt.h"

#ifdef CLIENT 
#include "fx.h"
#include "cmdgame.h"


extern FxEngine fx_engine;
#endif

// global lists of parsed objects
typedef struct FxBhvrList {
	FxBhvr **behaviors;
} FxBhvrList;

static StashTable fx_bhvrhash = 0;

StaticDefineInt splatFlags[] = {
	DEFINE_INT
	{ "ADDITIVE", SPLAT_ADDITIVE },
	{ "ADDBASE",  SPLAT_ADDBASE  },
	{ "ROUNDTEXTURE",  SPLAT_ROUNDTEXTURE  },
	{ "SUBTRACTIVE", SPLAT_SUBTRACTIVE },
	DEFINE_END
};

StaticDefineInt splatFalloffType[] = {
	DEFINE_INT
	{ "NONE",	SPLAT_FALLOFF_NONE },
	{ "UP",		SPLAT_FALLOFF_UP   },
	{ "DOWN",	SPLAT_FALLOFF_DOWN },
	{ "BOTH",	SPLAT_FALLOFF_UP },
	{ "SHADOW",	SPLAT_FALLOFF_SHADOW },
	DEFINE_END
};

#if NOVODEX
StaticDefineInt physForceTypeTable[] = {
	DEFINE_INT
	{ "None",		eNxForceType_None },
	{ "Out",		eNxForceType_Out },
	{ "In",			eNxForceType_In },
	{ "CWSwirl",	eNxForceType_CWSwirl },
	{ "CCWSwirl",	eNxForceType_CCWSwirl },
	{ "Up",			eNxForceType_Up },
	{ "Forward",	eNxForceType_Forward },
	{ "Side",		eNxForceType_Side },
	{ "Drag",		eNxForceType_Drag },
	DEFINE_END
};

StaticDefineInt physJointDOFTable[] = {
	DEFINE_INT
	{ "RotateX", 			eNxJointDOF_RotateX },
	{ "RotateY", 			eNxJointDOF_RotateY },
	{ "RotateZ", 			eNxJointDOF_RotateZ },
	{ "TranslateX", 		eNxJointDOF_TranslateX },
	{ "TranslateY", 		eNxJointDOF_TranslateY },
	{ "TranslateZ", 		eNxJointDOF_TranslateZ },
	DEFINE_END
};

StaticDefineInt physDebrisTable[] =
{
	DEFINE_INT
	{ "None", 				eNxPhysDebrisType_None },
	{ "Small", 				eNxPhysDebrisType_Small },
	{ "Large", 				eNxPhysDebrisType_Large },
	{ "LargeIfHardware",	eNxPhysDebrisType_LargeIfHardware },
	DEFINE_END
};
#endif

//########################################################################################
//## Get a .bhvr file
static StaticDefineInt stanim_flags[] = {
    DEFINE_INT
	{ "FRAMESNAP",		STANIM_FRAMESNAP },
	{ "LOCAL_TIMER",	STANIM_LOCAL_TIMER },
	{ "GLOBAL_TIMER",	STANIM_GLOBAL_TIMER },
    DEFINE_END
};

//Only used by splats
TokenizerParseInfo parseStAnim[] = {
	{ "",				TOK_STRUCTPARAM | TOK_F32(StAnim,speed_scale, 0)}, 
	{ "",				TOK_STRUCTPARAM | TOK_F32(StAnim,st_scale, 0)}, 
	{ "",				TOK_STRUCTPARAM | TOK_STRING(StAnim,name, 0)}, 
	{ "",				TOK_STRUCTPARAM | TOK_FLAGS(StAnim,flags,0),stanim_flags}, 
	{ "\n",				TOK_END,			0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseFxBhvr[] = {
	{ "Behavior",				TOK_IGNORE }, // hack, so we can use the old list system for a bit
	{ "ParamBitfield",			TOK_USEDFIELD(FxBhvr,paramBitfield,paramBitSize)},
	{ "Name",					TOK_CURRENTFILE(FxBhvr,name)					},
	{ "FileAge",				TOK_TIMESTAMP(FxBhvr,fileAge)					},
	{ "StartJitter",			TOK_VEC3(FxBhvr,startjitter)					},
	{ "InitialVelocity",		TOK_VEC3(FxBhvr,initialvelocity)				},
	{ "InitialVelocityJitter",	TOK_VEC3(FxBhvr,initialvelocityjitter)			},
	{ "InitialVelocityAngle",	TOK_F32(FxBhvr,initialvelocityangle, 0)			},
	{ "Gravity",				TOK_F32(FxBhvr,gravity, 0)						},
	{ "Physics",				TOK_U8(FxBhvr,physicsEnabled, 0)				},
	{ "PhysRadius",				TOK_F32(FxBhvr,physRadius, 0)					}, // 0.7f) },
	{ "PhysGravity",			TOK_F32(FxBhvr,physGravity, 1.0f)				},
	{ "PhysRestitution",		TOK_F32(FxBhvr,physRestitution, 0)				}, // 0.5f) },
	{ "PhysSFriction",			TOK_F32(FxBhvr,physStaticFriction, 0)			}, // 0.5f) },
	{ "PhysDFriction",			TOK_F32(FxBhvr,physDynamicFriction, 0)			}, // 0.3f) },
	{ "PhysKillBelowSpeed",		TOK_F32(FxBhvr,physKillBelowSpeed, 0.0f)		},
	{ "PhysDensity",			TOK_F32(FxBhvr,physDensity, 1.0f)				},
	{ "PhysForceRadius",		TOK_F32(FxBhvr,physForceRadius, 0.0f)			},
	{ "PhysForcePower",			TOK_F32(FxBhvr,physForcePower, 0.0f)			},
	{ "PhysForcePowerJitter",	TOK_F32(FxBhvr,physForcePowerJitter, 0.0f)		},
	{ "PhysForceCentripetal",	TOK_F32(FxBhvr,physForceCentripetal, 2.0f)		},
	{ "PhysForceSpeedScaleMax",	TOK_F32(FxBhvr,physForceSpeedScaleMax, 0.0f)	},
	{ "PhysScale",				TOK_VEC3(FxBhvr,physScale)						},
#if NOVODEX
	{ "PhysJointDOFs",			TOK_FLAGS(FxBhvr,physJointDOFs,0), physJointDOFTable	},
	{ "PhysJointAnchor",		TOK_VEC3(FxBhvr,physJointAnchor)		},
	{ "PhysJointAngLimit",		TOK_F32(FxBhvr,physJointAngLimit, 0) },
	{ "PhysJointLinLimit",		TOK_F32(FxBhvr,physJointLinLimit, 0) },
	{ "PhysJointBreakTorque",	TOK_F32(FxBhvr,physJointBreakTorque, 0.0f)	},
	{ "PhysJointBreakForce",	TOK_F32(FxBhvr,physJointBreakForce, 0.0f)	},
	{ "PhysJointLinSpring",		TOK_F32(FxBhvr,physJointLinSpring, 0) },
	{ "PhysJointLinSpringDamp",	TOK_F32(FxBhvr,physJointLinSpringDamp, 0) },
	{ "PhysJointAngSpring",		TOK_F32(FxBhvr,physJointAngSpring, 0.0f)	},
	{ "PhysJointAngSpringDamp",	TOK_F32(FxBhvr,physJointAngSpringDamp, 0.0f)	},
	{ "PhysJointDrag",			TOK_F32(FxBhvr,physJointDrag, 0)				},
	{ "PhysJointCollidesWorld", TOK_BOOLFLAG(FxBhvr,physJointCollidesWorld, false)			},
	{ "PhysForceType",			TOK_INT(FxBhvr,physForceType, eNxForceType_None), physForceTypeTable},
	{ "PhysDebris",				TOK_INT(FxBhvr,physDebris, eNxPhysDebrisType_None), physDebrisTable	},
#else
	{ "PhysJointDOFs",			TOK_IGNORE	},
	{ "PhysJointAnchor",		TOK_IGNORE	},
	{ "PhysJointAngLimit",		TOK_IGNORE	},
	{ "PhysJointLinLimit",		TOK_IGNORE	},
	{ "PhysJointBreakTorque",	TOK_IGNORE	},
	{ "PhysJointBreakForce",	TOK_IGNORE	},
	{ "PhysJointLinSpring",		TOK_IGNORE	},
	{ "PhysJointLinSpringDamp",	TOK_IGNORE	},
	{ "PhysJointAngSpring",		TOK_IGNORE	},
	{ "PhysJointAngSpringDamp",	TOK_IGNORE	},
	{ "PhysJointDrag",			TOK_IGNORE	},
	{ "PhysJointCollidesWorld",	TOK_IGNORE	},
	{ "PhysForceType",			TOK_IGNORE	},
	{ "PhysDebris",				TOK_IGNORE	},
#endif
	{ "Spin",					TOK_VEC3(FxBhvr,initspin)				},
	{ "SpinJitter",				TOK_VEC3(FxBhvr,spinjitter)				},
	{ "FadeInLength",			TOK_F32(FxBhvr,fadeinlength, 0)			},
	{ "FadeOutLength",			TOK_F32(FxBhvr,fadeoutlength, 0)		},
	{ "Shake",					TOK_F32(FxBhvr,shake, 0)				},
	{ "ShakeFallOff",			TOK_F32(FxBhvr,shakeFallOff, 0)			},
	{ "ShakeRadius",			TOK_F32(FxBhvr,shakeRadius, 0)			},
	{ "Blur",					TOK_F32(FxBhvr,blur, 0)					},
	{ "BlurFallOff",			TOK_F32(FxBhvr,blurFallOff, 0)			},
	{ "BlurRadius",				TOK_F32(FxBhvr,blurRadius, 0)			},
	{ "Scale",					TOK_VEC3(FxBhvr,scale)					},
	{ "ScaleRate",				TOK_VEC3(FxBhvr,scalerate)				},
	{ "ScaleTime",				TOK_VEC3(FxBhvr,scaleTime)				},
	{ "EndScale",				TOK_VEC3(FxBhvr,endscale)				},
	{ "Stretch",				TOK_U8(FxBhvr,stretch, 0)				},
	{ "Drag",					TOK_F32(FxBhvr,drag, 0)					},
	{ "PyrRotate",				TOK_VEC3(FxBhvr,pyrRotate)				},
	{ "PyrRotateJitter",		TOK_VEC3(FxBhvr,pyrRotateJitter)		},
	{ "PositionOffset",			TOK_VEC3(FxBhvr,positionOffset)			},
	{ "UseShieldOffset",		TOK_BOOLFLAG(FxBhvr, useShieldOffset, 0)	},
	{ "TrackRate",				TOK_F32(FxBhvr,trackrate, 0)			},
	{ "TrackMethod",			TOK_U8(FxBhvr,trackmethod, 0)			},
	{ "Collides",				TOK_U8(FxBhvr,collides, 0)				},
	{ "LifeSpan",				TOK_INT(FxBhvr,lifespan, 0)				},
	{ "AnimScale",				TOK_F32(FxBhvr,animscale, 0)			},
	{ "Alpha",					TOK_U8(FxBhvr,alpha, 0)					},
	{ "PulsePeakTime",			TOK_F32(FxBhvr,pulsePeakTime, 0)		},
	{ "PulseBrightness",		TOK_F32(FxBhvr,pulseBrightness, 0)		},
	{ "PulseClamp",				TOK_F32(FxBhvr,pulseClamp, 0)			},
	{ "SplatFlags",				TOK_FLAGS(FxBhvr,splatFlags,		0), splatFlags			},
	{ "SplatFalloffType",		TOK_FLAGS(FxBhvr,splatFalloffType,	0), splatFalloffType		},
	{ "SplatNormalFade",		TOK_F32(FxBhvr,splatNormalFade, 0)		},
	{ "SplatFadeCenter",		TOK_F32(FxBhvr,splatFadeCenter, 0)		},
	{ "SplatSetBack",			TOK_F32(FxBhvr,splatSetBack, 0)			},
	{ "StAnim",					TOK_STRUCT(FxBhvr,stAnim,parseStAnim) },
	{ "HueShift",				TOK_F32(FxBhvr,hueShift, 0)				},
	{ "HueShiftJitter",			TOK_F32(FxBhvr,hueShiftJitter, 0)			},
	{ "StartColor",				TOK_RGB(FxBhvr,colornavpoint[0].rgb)	},
	{ "BeColor1",				TOK_RGB(FxBhvr,colornavpoint[1].rgb)	},
	{ "BeColor2",				TOK_RGB(FxBhvr,colornavpoint[2].rgb)	},
	{ "BeColor3",				TOK_RGB(FxBhvr,colornavpoint[3].rgb)	},
	{ "BeColor4",				TOK_RGB(FxBhvr,colornavpoint[4].rgb)	},
	{ "ByTime1",				TOK_INT(FxBhvr,colornavpoint[1].time, 0)	},
	{ "ByTime2",				TOK_INT(FxBhvr,colornavpoint[2].time, 0)	},
	{ "ByTime3",				TOK_INT(FxBhvr,colornavpoint[3].time, 0)	},
	{ "ByTime4",				TOK_INT(FxBhvr,colornavpoint[4].time, 0)	},
	{ "PrimaryTint",			TOK_F32(FxBhvr, colornavpoint[0].primaryTint, 0.0f) },
	{ "PrimaryTint1",			TOK_F32(FxBhvr, colornavpoint[1].primaryTint, 0.0f) },
	{ "PrimaryTint2",			TOK_F32(FxBhvr, colornavpoint[2].primaryTint, 0.0f) },
	{ "PrimaryTint3",			TOK_F32(FxBhvr, colornavpoint[3].primaryTint, 0.0f) },
	{ "PrimaryTint4",			TOK_F32(FxBhvr, colornavpoint[4].primaryTint, 0.0f) },
	{ "SecondaryTint",			TOK_F32(FxBhvr, colornavpoint[0].secondaryTint, 0.0f) },
	{ "SecondaryTint1",			TOK_F32(FxBhvr, colornavpoint[1].secondaryTint, 0.0f) },
	{ "SecondaryTint2",			TOK_F32(FxBhvr, colornavpoint[2].secondaryTint, 0.0f) },
	{ "SecondaryTint3",			TOK_F32(FxBhvr, colornavpoint[3].secondaryTint, 0.0f) },
	{ "SecondaryTint4",			TOK_F32(FxBhvr, colornavpoint[4].secondaryTint, 0.0f) },
	{ "InheritGroupTint",		TOK_BOOLFLAG(FxBhvr,bInheritGroupTint, 0)	},
	{ "Rgb0",					TOK_RGB(FxBhvr,colornavpoint[0].rgb)	},
	{ "Rgb1",					TOK_RGB(FxBhvr,colornavpoint[1].rgb)	},
	{ "Rgb2",					TOK_RGB(FxBhvr,colornavpoint[2].rgb)	},
	{ "Rgb3",					TOK_RGB(FxBhvr,colornavpoint[3].rgb)	},
	{ "Rgb4",					TOK_RGB(FxBhvr,colornavpoint[4].rgb)	},
	{ "Rgb0Time",				TOK_INT(FxBhvr,colornavpoint[1].time, 0)	},
	{ "Rgb1Time",				TOK_INT(FxBhvr,colornavpoint[2].time, 0)	},
	{ "Rgb2Time",				TOK_INT(FxBhvr,colornavpoint[3].time, 0)	},
	{ "Rgb3Time",				TOK_INT(FxBhvr,colornavpoint[4].time, 0)	},
	{ "Rgb4Time",				TOK_INT(FxBhvr,colornavpoint[0].time, 0)	}, //junk
	{ "TintGeom",				TOK_BOOL(FxBhvr,bTintGeom, 0) },
	{ "End",					TOK_END,		0										},
	{ "", 0, 0 }
};

TokenizerParseInfo ParseFxBhvrList[] = {
	{ "Behavior",				TOK_STRUCT(FxBhvrList,behaviors, ParseFxBhvr) },
	{ "", 0, 0 }
};



#ifdef CLIENT
int fxInitFxBhvr(FxBhvr* fxbhvr)
{
	ColorNavPoint * cnp0;
	assert( fxbhvr->name );
	assert( !fxbhvr->initialized );

	//Set default values
	if( fxbhvr->scale[0] == 0 )
		fxbhvr->scale[0] = 1.0;
	if( fxbhvr->scale[1] == 0 )
		fxbhvr->scale[1] = 1.0;
	if( fxbhvr->scale[2] == 0 )
		fxbhvr->scale[2] = 1.0;

	//end scale of zero means don't do any scaling
	if( fxbhvr->endscale[0] == 0 )
		fxbhvr->endscale[0] = fxbhvr->scale[0];
	if( fxbhvr->endscale[1] == 0 )
		fxbhvr->endscale[1] = fxbhvr->scale[1];
	if( fxbhvr->endscale[2] == 0 )
		fxbhvr->endscale[2] = fxbhvr->scale[2];
		
	//Old ScaleRate Stuff
	if( fxbhvr->scalerate[0] == 0 && fxbhvr->scalerate[1] == 0 &&  fxbhvr->scalerate[2] == 0 )
		fxbhvr->scalerate[0] = fxbhvr->scalerate[1] = fxbhvr->scalerate[2] = 1.0;
	//Convert old scaleRate to the new scaleTime
	{
		int i;
		F32 distanceToGo, scaleRate, scaleTime;
		for( i = 0 ; i < 3 ; i++ )
		{
			scaleRate = fxbhvr->scalerate[i];
			if( scaleRate != 1.0 )
			{
				distanceToGo = fxbhvr->endscale[i] - fxbhvr->scale[i];
				scaleRate = fxbhvr->scalerate[i];

				//TO DO some math 
				scaleTime = distanceToGo / ( scaleRate - 1 );
	
				fxbhvr->scaleTime[i] = ABS( scaleTime );
			}
			//else
			//{
			//	fxbhvr->scaleTime[i] = 0.0;
			//}
		}
	}
	//End conversion

	if( fxbhvr->animscale == 0.0 )
		fxbhvr->animscale = 1.0;

	if(fxbhvr->fadeinlength < 1)
		fxbhvr->fadeinlength = 1;

	if(fxbhvr->fadeoutlength < 1)
		fxbhvr->fadeoutlength = 1;

	if(!fxbhvr->alpha)
		fxbhvr->alpha = 255;

	if(fxbhvr->shake)
	{
		if(!fxbhvr->shakeRadius)
			fxbhvr->shakeRadius = 50;
		if(!fxbhvr->shakeFallOff)
			fxbhvr->shakeFallOff = 0.01;
	}

	if(fxbhvr->blur)
	{
		if(!fxbhvr->blurRadius)
			fxbhvr->blurRadius = 50;
		if(!fxbhvr->blurFallOff)
			fxbhvr->blurFallOff = 0.03;
	}

	//ARtists give this in degrees, but I want Radians
	fxbhvr->pyrRotate[0] = RAD(fxbhvr->pyrRotate[0]); 
	fxbhvr->pyrRotate[1] = RAD(fxbhvr->pyrRotate[1]);
	fxbhvr->pyrRotate[2] = RAD(fxbhvr->pyrRotate[2]);
	fxbhvr->pyrRotateJitter[0] = RAD(fxbhvr->pyrRotateJitter[0]); 
	fxbhvr->pyrRotateJitter[1] = RAD(fxbhvr->pyrRotateJitter[1]);
	fxbhvr->pyrRotateJitter[2] = RAD(fxbhvr->pyrRotateJitter[2]);

	cnp0 = &fxbhvr->colornavpoint[0];  //not a perfect check, but pretty good
	if( !cnp0->rgb[0] && !cnp0->rgb[1] && !cnp0->rgb[2] && !fxbhvr->colornavpoint[1].time == 0 )
		cnp0->rgb[0] = cnp0->rgb[1] = cnp0->rgb[2] = 255;

	//TO DO fix this so it does a better job
	if(	!fxbhvr->colornavpoint[0].rgb[0] &&
		!fxbhvr->colornavpoint[0].rgb[1] && 
		!fxbhvr->colornavpoint[0].rgb[2] &&
		!fxbhvr->colornavpoint[1].rgb[0] &&
		!fxbhvr->colornavpoint[1].rgb[1] && 
		!fxbhvr->colornavpoint[1].rgb[2])
	{
		fxbhvr->colornavpoint[0].rgb[0] = 255;
		fxbhvr->colornavpoint[0].rgb[1] = 255;
		fxbhvr->colornavpoint[0].rgb[2] = 255;
	}

	//Build color path 
	partBuildColorPath(&fxbhvr->colorpath, fxbhvr->colornavpoint, 0, false);

	//Calculate Fade in and fade out
	fxbhvr->fadeinrate = (F32)fxbhvr->alpha / fxbhvr->fadeinlength; 
	fxbhvr->fadeoutrate = (F32)fxbhvr->alpha / fxbhvr->fadeoutlength; 
	assert( fxbhvr->fadeinrate > 0 ); 
	assert( fxbhvr->fadeoutrate > 0 ); //important, cuz death is triggered off 0 alpha

	//Texture scrolling
	if ( fxbhvr->stAnim ) //only ever one st_anim.  Since it's a struct, though, it needs this (TO DO do a Mark's get count thing)
	{
		if( !setStAnim(*fxbhvr->stAnim) )
		{
			printToScreenLog( 1, "Bad stanim in fxbhvr %s", fxbhvr->name );
			fxbhvr->stAnim = 0;
		}
	}

	return 1;
}

static FxBhvr * fxLoadFxBhvr( char *fname )
{
	FxBhvr * fxbhvr = 0;
	int		 fileisgood = 0;
	TokenizerHandle tok;
	char buf[1000];

	errorLogFileIsBeingReloaded(fname);

	tok = TokenizerCreate(fname);
	if (tok) 
	{
		fxbhvr = (FxBhvr*)listAddNewMember(&fx_engine.fxbhvrs, sizeof(FxBhvr));
		assert(fxbhvr);
		listScrubMember(fxbhvr, sizeof(FxBhvr));
		ParserInitStruct(fxbhvr, 0, ParseFxBhvr);
		fileisgood = TokenizerParseList(tok, ParseFxBhvr, fxbhvr, TokenizerErrorCallback);
		if( !fileisgood )
		{
			listFreeMember( fxbhvr, &fx_engine.fxbhvrs );
			fxbhvr = 0;
		}
		else
		{
			fxCleanFileName(buf, fxbhvr->name); //(buf2 will always be shorter than info->name)
			strcpy(fxbhvr->name, buf);
			fx_engine.fxbhvrcount++;
		}
	}
	TokenizerDestroy(tok);

	return fxbhvr;
}


FxBhvr * fxGetFxBhvr( char *bhvr_name )
{
	FxBhvr * fxbhvr = 0;
	char fx_name_cleaned_up[FX_MAX_PATH];

	fxCleanFileName(fx_name_cleaned_up, bhvr_name);
	stashFindPointer( fx_bhvrhash, fx_name_cleaned_up, &fxbhvr );

	if( fxbhvr )
	{
		//Development only
		if (!global_state.no_file_change_check && isDevelopmentMode())
			fxbhvr->fileAge = fileLastChanged( "bin/behaviors.bin" );
		//End developmetn only
	}

	//Failure in production mode == very bad data.

	// ## Development only 
	if (!global_state.no_file_change_check && isDevelopmentMode()) 
	{
		// always prefer the debug version to the version in the hash table
		{
			FxBhvr * dev_fxbhvr;
			for( dev_fxbhvr = fx_engine.fxbhvrs ; dev_fxbhvr ; dev_fxbhvr = dev_fxbhvr->next )
			{
				if( !_stricmp(fx_name_cleaned_up, dev_fxbhvr->name ) )
				{
					fxbhvr = dev_fxbhvr;
					break;	
				}
			}
		}

		{
			char buf[500];
			sprintf(buf, "%s%s", "fx/", fx_name_cleaned_up);

			//Reload the sysinfo from the file under these conditions
			if( !fxbhvr || //it's brand new fxbhvr
				(fxbhvr && fileHasBeenUpdated( buf, &fxbhvr->fileAge )) )  //the fxbhvr file itself has changed
			{
				fxbhvr = fxLoadFxBhvr(buf);
			}
		}
	}
	// ## End development only

	if( fxbhvr )
	{
		if (!fxbhvr->initialized) 
		{
			if( !fxInitFxBhvr( fxbhvr ) )
			{
				Errorf( "Error in file %s", fx_name_cleaned_up );
				return 0;
			}
			fxbhvr->initialized = 1;
		}
	}
	else
	{
		printToScreenLog( 1, "INFO: Failed to get bhvr %s", fx_name_cleaned_up ); 
	}

	// ## Developnment only: if fxbhvr fails for any reason, try to remove it from the run time load list
	if( !fxbhvr )
	{
		FxBhvr * checkerbhvr;
		FxBhvr * checkerbhvr_next;
		for( checkerbhvr = fx_engine.fxbhvrs ; checkerbhvr ; checkerbhvr = checkerbhvr_next )
		{
			checkerbhvr_next = checkerbhvr->next;
			if( !checkerbhvr->name || !_stricmp( fx_name_cleaned_up, checkerbhvr->name ) )
			{
				listFreeMember( checkerbhvr, &fx_engine.fxbhvrs );
				fx_engine.fxbhvrcount--;
				assert( fx_engine.fxbhvrcount >= 0 );
			}
		}
	}
	//## end development only

	return fxbhvr;
}

void fxPreloadBhvrInfo()
{
	char *dir=game_state.nofx?NULL:"fx";
	char *filetype=game_state.nofx?"fx/behaviors/null.bhvr":".bhvr";
	int flags=0;
	char *binFilename = game_state.nofx?0:"behaviors.bin";
	FxBhvrList fx_bhvrlist = {0};
	ParserLoadFiles(dir, filetype, binFilename, flags, ParseFxBhvrList, &fx_bhvrlist, NULL, NULL, NULL, NULL);

	{
		char buf2[FX_MAX_PATH];
		int num_structs, i;
		FxBhvr * bhvr;

		if (!fx_bhvrhash)
			fx_bhvrhash = stashTableCreateWithStringKeys(4096, StashDeepCopyKeys | StashCaseSensitive );

		//Clean Up Bhvr names, and put them in the hash table
		num_structs = eaSize(&fx_bhvrlist.behaviors);
		for (i = 0; i < num_structs; i++)
		{
			bhvr = fx_bhvrlist.behaviors[i];
			fxCleanFileName(buf2, bhvr->name); //(buf2 will always be shorter than info->name)
			strcpy(bhvr->name, buf2);
			stashAddPointer(fx_bhvrhash, bhvr->name, bhvr, false);
		}
	}

	eaDestroy(&fx_bhvrlist.behaviors);
}

#endif
