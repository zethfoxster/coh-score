#include "Position.h"
#include <stdlib.h>
#include <assert.h>
#include "entity.h"


/* Function posFindNearest
 *	Searches through the given array of Position pointers and returns
 *	the one that is closest to the given reference Position.
 */
Beacon* posFindNearest(Array* searchlist, const Mat4 ref){
	int i;
	int target = -1;				// index of currenlty selected target
	float targetDistance = -1;		// distance of currently selected target
	float curDistance;				// distance of entity currently under examination
	
	// no Positions in the search list?
	if(!searchlist || !searchlist->size)
		return NULL;

	// assume that the first Position is to be returned
	target = 0;
	targetDistance = posGetDistSquared(searchlist->storage[0], ref);
	
	
	// process subsequent Positions
	for(i = 1; i < searchlist->size; i++){
		curDistance = posGetDistSquared(searchlist->storage[i], ref);

		// if the distance to current Position is shorter, set current
		// Position as the one to be returned
		if(curDistance < targetDistance){			
			target = i;
			targetDistance = curDistance;
		}
	}
	
	return searchlist->storage[target];
}

/* Function posCondFindNearest
 *	Same as FindNearest with one exception.  Each element considered as the
 *	returned position will be passed to the condition function.  Only Positions
 *	that are "accepted" can be returned.
 *
 *	Condition function:
 *		1st parameter - Position under consideration
 *		2nd parameter - reference Position
 *
 */
Beacon* posCondFindNearest(Array* searchlist, Entity* e, Condition accept){
	int i;
	int target = -1;				// index of currenlty selected target
	float targetDistance = -1;		// distance of currently selected target
	float curDistance;				// distance of entity currently under

	// find first acceptable Position
	for(i = 0; i < searchlist->size; i++){
		if(accept(searchlist->storage[i], e)){
			target = i;
			targetDistance = posGetDistSquared(searchlist->storage[i], ENTMAT(e));
			break;
		}
	}

	// no acceptable Positions?
	if(-1 == target)
		return NULL;

	for(i++; i < searchlist->size; i++){
		Position* pos = searchlist->storage[i];
		curDistance = posGetDistSquared(searchlist->storage[i], ENTMAT(e));
		
		//printf("pos: %5.2f, %5.2f, %5.2f: ", posX(pos), posY(pos), posZ(pos));

		// if the distance to current Position is shorter AND the current Position
		// is "acceptable", set current Position as the one to be returned
		if(curDistance < targetDistance && accept(searchlist->storage[i], e)){
			target = i;
			targetDistance = curDistance;
			//printf("good\n");
		}//else
			//printf("bad\n");
	}

	return searchlist->storage[target];
}

/* Function posCondFindBest
 *	Given an array of Positions to search, the function extracts the "best" position
 *	as dicated by the compare function.  The function uses the compare function to
 *	continually compare elements until it determines which one the compare function
 *	prefers the most.  Expect the compare function to be called N - 1 times where N 
 *	is the size of the searchlist.
 *
 *	Condition function:
 *		1st parameter - Position 1
 *		2nd parameter - Position 2
 *		Return values:
 *			-1 - Position 1 is better
 *			 0 - The two positions are equally perferred
 *			 1 - Position 2 is better
 *
 */
//Position* posCondFindBest(Array* searchlist, Condition compare){
//	int i;
//	int target;				// index of currenlty selected target
//	int compareResult;
//	
//	assert(searchlist && compare);
//
//	// Can't find the best element if there are elements in the searchlist
//	if(0 == searchlist->size)
//		return NULL;
//	
//	// Start by choosing the first element in the searchlist
//	target = 0;
//	for(i = 1; i < searchlist->size; i++){
//		// Compare the currently chosen target to the element currently under consideration.
//		compareResult = compare(searchlist->storage[target], searchlist->storage[i]);
//		switch(compareResult){
//		case(1):
//			// The 2nd parameter given to the compare function is preferred.  Update the
//			// current selection to the index of the 2nd parameter.
//			target = i;
//			break;
//
//		case(0):
//			// The two elements are equally preferred.  Nothing needs to be done.
//			break;
//
//		case(-1):
//			// The first parameter given to the compare function is preferred.  Since the
//			// first parameter is already the selected target, nothing needs to be done.
//			break;
//
//		default:
//			// The compare function returned an expected value.
//			assert(0);
//		}
//	}
//	
//	return searchlist->storage[target];
//}

/* Function posFindNearby
 *	Searches through the given array of Position pointers and an array
 *	of Position structures in the searchlist that are considered "nearby".
 *
 *	The function will iterate through the searchlist and send the "nearby"
 *	function the Position under examination and the Position used
 *	as the reference.  Only those elements that the condition function
 *	considers nearby will be added to the results list.
 *
 *	Condition function:
 *		1st parameter - Position under consideration
 *		2nd parameter - reference Position
 *
 *
 *	Parameters:
 *		searchlist - the array of Positions to compare against the reference Position
 *		ref - the reference Position
 *		nearby - the function used to tell if an element in the searchlist is
 *				 near the reference object
 *	Returns:
 *		NULL - no nearby elements found
 *		Array* - all Position pointers in the searchlist that are near the reference Position
 */
//Array* posFindNearby(Array* searchlist, Position* ref, Condition nearby){
//	Array* result;
//	int i;
//
//	// create the array to hold the elements to be returned.
//	result = createArray();
//	initArray(result, 4);
//
//	// iterate through the entire searchlist
//	for(i = 0; i < searchlist->size; i++){
//		// do not return the reference Position as a nearby position
//		if(searchlist->storage[i] == ref)
//			continue;
//
//		// if the current Position is near the reference position
//		if(nearby(searchlist->storage[i], ref))
//			// add the current Position to the results list
//			arrayPushBack(result, searchlist->storage[i]);
//	}
//
//	// if there are no nearby elements, return NULL
//	if(0 == result->size){
//		destroyArray(result);
//		return NULL;
//	}
//
//	return result;
//}

//void posFindNearbyEx(Array* searchlist, Position* ref, Condition nearby, Array* result){
//	int i;
//	
//	// iterate through the entire searchlist
//	for(i = 0; i < searchlist->size; i++){
//		// do not return the reference Position as a nearby position
//		if(searchlist->storage[i] == ref)
//			continue;
//		
//		// if the current Position is near the reference position
//		if(nearby(searchlist->storage[i], ref))
//			// add the current Position to the results list
//			arrayPushBack(result, searchlist->storage[i]);
//	}
//}

/* Function posWithinDistance
 *	Checks to see if the target is within the given distance to the source.
 *
 *	Returns:
 *		0 - source not within distance of target
 *		1 - source within distance of target
 */
int posWithinDistance(const Mat4 source, const Mat4 target, float distance){
	return posGetDistSquared(source, target) <= distance * distance;
}
//
//int posWithinDistance2D(Position* source, Position* target, float distance){
//	return posGetDistSquared2D(source, target) <= distance * distance;
//}
//
//void posGetUnitVector(Position* source, Position* target, Vec3 result){
//	subVec3(target->mat[3], source->mat[3], result);
//	normalVec3(result);
//}
//
//void posGetVector(Position* source, Position* target, Vec3 result){
//	subVec3(target->mat[3], source->mat[3], result);
//}
//
//float posGetHeading(Position* source){
//	Vec3 PYR;
//	getMat3YPR(source->mat, PYR);
//	return -PYR[1];
//}
//
//float posGetHeading2(Position* source, Position* target){
//	Vec3 direction;
//	float yaw, pitch;
//	
//	subVec3(source->mat[3], target->mat[3], direction);
//	getVec3YP(direction, &yaw, &pitch);
//
//	return yaw;
//}
//
//float posGetPitch(Position* source){
//	Vec3 PYR;
//	getMat3YPR(source->mat, PYR);
//	return -PYR[0];
//}
//
float posGetDistSquared(const Mat4 source, const Mat4 target){
	Vec3 diff;
	subVec3(source[3], target[3], diff);
	return diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2];
}
//
//float posGetDistSquared2D(Position* source, Position* target){
//	Vec3 diff;
//	diff[0] = posX(source) - posX(target);
//	diff[2] = posZ(source) - posZ(target);
//	return diff[0]*diff[0] + diff[2]*diff[2];
//}
//
//float posGetDist(Position* source, Position* target){
//	Vec3 diff;
//	subVec3(source->mat[3], target->mat[3], diff);
//	return sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
//}
//
//float posGetDist2D(Position* source, Position* target){
//	Vec3 diff;
//	diff[0] = posX(source) - posX(target);
//	diff[2] = posZ(source) - posZ(target);
//	return sqrt(diff[0]*diff[0] + diff[2]*diff[2]);
//}

//void posApplyRandomOffset(Position* source, Position* destination, int maxXOffset, int maxYOffset, int maxZOffset){
//	int xoffset, yoffset, zoffset;
//
//	copyMat4(source->mat, destination->mat);
//	xoffset = randInt(maxXOffset);
//	yoffset = randInt(maxYOffset);
//	zoffset = randInt(maxZOffset);
//	
//	xoffset = (randInt(2) ? xoffset : -xoffset);
//	yoffset = (randInt(2) ? yoffset : -yoffset);
//	zoffset = (randInt(2) ? zoffset : -zoffset);
//	posX(destination) = posX(source) + xoffset;
//	posY(destination) = posY(source) + yoffset;
//	posZ(destination) = posZ(source) + zoffset;
//}
//
//void posApplyOffset(Position* source, Position* destination, RelativePosition* offset){
//	if(OFFSET_CARTESIAN == offset->type)
//		posApplyCOffset(source, destination, posX(offset), posY(offset), posZ(offset));
//
//	posApplyPOffset(source, destination, posX(offset), posY(offset));
//}
//
//void posApplyCOffset(Position* source, Position* destination, float xOffset, float yOffset, float zOffset){
//	copyMat4(source->mat, destination->mat);
//	posX(destination) = posX(source) + xOffset;
//	posY(destination) = posY(source) + yOffset;
//	posZ(destination) = posZ(source) + zOffset;
//}
//
//void posApplyPOffset(Position* source, Position* destination, float yaw, float distance){
//	copyMat4(source->mat, destination->mat);
//
//	// rotate a unit vector along the xz plane				
//	posX(destination) = -sin(yaw);
//	posY(destination) = 0;
//	posZ(destination) = -cos(yaw);
//
//	// scale rotated vector
//	scaleVec3(destination->mat[3], distance, destination->mat[3]);
//
//	posX(destination) += posX(source);
//	posY(destination) += posY(source);
//	posZ(destination) += posZ(source);
//}
//
//void posConvertRelative(RelativePosition* pos, OffsetType newType){
//	if(newType == pos->type)
//		return;
//
//	if(OFFSET_CARTESIAN == newType){
//		float yaw, pitch;
//		getVec3YP(pos->mat[3], &yaw, &pitch);
//		
//		// ack... rotate yaw vector so that it is aligned with one of the axis and then
//		// find out the distance bla bla bla...
//		assert(0);	// don't use this function yet... not fully implemented...
//		return;
//	}
//
//	if(OFFSET_PARAMETRIC== newType){
//		Vec3 newOffset;
//
//		// rotate a unit vector along the xz plane				
//		newOffset[0] = -sin(posX(pos));
//		newOffset[1] = 0;
//		newOffset[2] = -cos(posX(pos));
//
//		// scale rotated vector
//		scaleVec3(newOffset, pos->parametric.distance, newOffset);
//		copyVec3(pos->mat[3], newOffset);
//
//		return;
//	}
//}
//
//
//RelativePosition* createRelativePosition(){
//	return calloc(1, sizeof(RelativePosition*));
//}
//
//void destroyRelativePosition(RelativePosition* pos){
//	free(pos);
//}