#ifndef __NEW_FEATURE_H__
#define __NEW_FEATURE_H__

enum {
	NEWFEATURE_SSAO,
	NEWFEATURE_ALLBUTSSAO,
	NEWFEATURE_COUNT = 2,
	NEWFEATURE_ALL
};

void rdrNewFeature(void);
void getNewFeatureForCurWindow(Vec4 * newFeature, int featureID);

#endif // __NEW_FEATURE_H__