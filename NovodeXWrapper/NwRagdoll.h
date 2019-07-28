#ifndef INCLUDED_NW_RAGDOLL_H
#define INCLUDED_NW_RAGDOLL_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "mathutil.h"
#include "ragdoll.h"





#if RAGDOLL

typedef struct Entity Entity;
typedef struct Ragdoll Ragdoll;






void  nwDeleteRagdoll(Ragdoll* parentRagdoll);
#ifdef CLIENT
Ragdoll* nwCreateRagdollNoPhysics( Entity* e );
#endif
void  nwPushRagdoll( Entity* e );
void nwProcessRagdollQueue( );
void processRagdollDeletionQueues();
void nwCreateRagdollQueues();
void nwDeleteRagdollQueues();




void drawRagdollSkeleton( GfxNode* rootNode, Mat4 parentMat, SeqInst* seq  );



void nwSetRagdollFromQuaternions( Ragdoll* ragdoll, Quat qRoot );




#endif // RAGDOLL

#ifdef __cplusplus
}
#endif // __cplusplus


#endif
