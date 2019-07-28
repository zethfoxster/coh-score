#include "rt_tune.h"
#include "win_init.h"
#include "newfeature.h"


float newFeatureSplit[NEWFEATURE_COUNT][2] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
float newFeatureSplitTarget[NEWFEATURE_COUNT][2] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
float speed = 50.0f;
bool isVertical = true;

void getNewFeatureForCurWindow(Vec4 * newFeature, int featureID)
{
	(*newFeature)[0] = rdr_view_state.width * newFeatureSplit[featureID][0] / 100.0f;
	(*newFeature)[1] = rdr_view_state.height * newFeatureSplit[featureID][1] / 100.0f;
	(*newFeature)[2] = 0.0f;
	(*newFeature)[3] = 0.0f;
}

void animateNewFeaturesOff(int featureID)
{
	if(isVertical) {
		newFeatureSplit[featureID][1] = 0.0f;
		newFeatureSplitTarget[featureID][0] = 100.0f;
	} else {
		newFeatureSplit[featureID][0] = 0.0f;
		newFeatureSplitTarget[featureID][1] = 100.0f;
	}
}

void animateNewFeaturesOn(int featureID)
{
	if(isVertical) {
		newFeatureSplit[featureID][1] = 0.0f;
	} else {
		newFeatureSplit[featureID][0] = 0.0f;
	}

	newFeatureSplitTarget[featureID][0] = 0.0f;
	newFeatureSplitTarget[featureID][1] = 0.0f;
}

void animateNewFeaturesVertical(int featureID)
{
	isVertical = true;
	newFeatureSplitTarget[featureID][0] = 50.0f;
	newFeatureSplitTarget[featureID][1] = 0.0f;
}

void animateNewFeaturesHorizontal(int featureID)
{
	isVertical = false;
	newFeatureSplitTarget[featureID][0] = 0.0f;
	newFeatureSplitTarget[featureID][1] = 50.0f;
}

static int curSplitMode_SSAO = 1;
static int curSplitMode_AllButSSAO = 1;
static int curSplitMode_All = 1;

void animateNewFeatures_AllButSSAO()
{
	switch(curSplitMode_AllButSSAO) {
		case 0: animateNewFeaturesOff(NEWFEATURE_ALLBUTSSAO); break;
		case 1: animateNewFeaturesOn(NEWFEATURE_ALLBUTSSAO); break;
		case 2: animateNewFeaturesVertical(NEWFEATURE_ALLBUTSSAO); break;
		case 3: animateNewFeaturesHorizontal(NEWFEATURE_ALLBUTSSAO); break;
	}
}

void animateNewFeatures_SSAO()
{
	switch(curSplitMode_SSAO) {
		case 0: animateNewFeaturesOff(NEWFEATURE_SSAO); break;
		case 1: animateNewFeaturesOn(NEWFEATURE_SSAO); break;
		case 2: animateNewFeaturesVertical(NEWFEATURE_SSAO); break;
		case 3: animateNewFeaturesHorizontal(NEWFEATURE_SSAO); break;
	}
}

void animateNewFeatures_All()
{
	// set both features to same split mode and start animation in unison
	curSplitMode_SSAO = curSplitMode_All;
	curSplitMode_AllButSSAO = curSplitMode_All;
	animateNewFeatures_AllButSSAO();
	animateNewFeatures_SSAO();
}

void updateDemoStuff()
{
	int i, featureID;
	float step = speed * MIN((1.0/30.0f) * TIMESTEP_NOSCALE, 1.0f);
	bool bIsAnimating = false;
	static bool bWasAnimating = false;
	static int savedCubeFace = 0;

	for(featureID=0; featureID<NEWFEATURE_COUNT; featureID++) {
		for(i=0; i<2; i++) {
			float diff = newFeatureSplitTarget[featureID][i] - newFeatureSplit[featureID][i];
			if(fabs(diff) > fabs(step)) {
				newFeatureSplit[featureID][i] += _copysign(step, diff);
				bIsAnimating = true;
			} else {
				newFeatureSplit[featureID][i] = newFeatureSplitTarget[featureID][i];
			}
		}
	}

	// if wipe is active only update one face at a time, otherwise all
	if(bIsAnimating && !bWasAnimating) {
		// just started animating
		savedCubeFace = game_state.forceCubeFace;
		game_state.forceCubeFace = 0;
		bWasAnimating = true;
	} else if(bWasAnimating && !bIsAnimating) {
		// just finished animating
		game_state.forceCubeFace = savedCubeFace; 
		bWasAnimating = false;
	}

	// check to see if a feature toggle was requested:
	if( game_state.newfeaturetoggle )
	{
		switch(game_state.newfeaturetoggle)
		{
			case 1:
				curSplitMode_SSAO = !curSplitMode_SSAO;
				animateNewFeatures_SSAO();
				break;
			case 2:
				curSplitMode_AllButSSAO = !curSplitMode_AllButSSAO;
				animateNewFeatures_AllButSSAO();
				break;
			case 3:
				curSplitMode_All = !curSplitMode_All;
				animateNewFeatures_All();
				break;
		}
		game_state.newfeaturetoggle = 0; // reset
	}
}

void rdrNewFeature(void)
{
	static bool once = true;
	static const int splitValues[] = {0, 1, 2, 3};
	static const char * splitNames[] = {"Off", "On", NULL}; // "Vertical", "Horiozontal"

	if(once)
	{
		int featureID;
		// start with newfeature on
		//curSplitMode = 1;
		animateNewFeaturesOn(NEWFEATURE_ALLBUTSSAO);
		animateNewFeaturesOn(NEWFEATURE_SSAO);
		for(featureID=0; featureID<NEWFEATURE_COUNT; featureID++) {
			newFeatureSplit[featureID][0] = newFeatureSplitTarget[featureID][0];
			newFeatureSplit[featureID][1] = newFeatureSplitTarget[featureID][1];
		}
		once = false;
	}

	updateDemoStuff();

	tunePushMenu("Demo");

	tuneEnum("New Features - SSAO", &curSplitMode_SSAO, splitValues, splitNames, &animateNewFeatures_SSAO);
	tuneEnum("New Features - All but SSAO", &curSplitMode_AllButSSAO, splitValues, splitNames, &animateNewFeatures_AllButSSAO);
	tuneEnum("New Features - All", &curSplitMode_All, splitValues, splitNames, &animateNewFeatures_All);

	tunePopMenu("Demo");
}

