// -------------------------------------------------------------------------------------------------------------------
// NOVODEX SDK WRAPPER INTERFACE
// Written by Tom Lassanske, 03-01-05
// -------------------------------------------------------------------------------------------------------------------
#ifndef INCLUDED_NW_INTERFACE_H
#define INCLUDED_NW_INTERFACE_H

#define SCALE_TO_NX  0.0254f
#define SCALE_TO_COD 39.370f

#include "Nxf.h"
#include "NxVec3.h"
#include "NxArray.h"
#include "NxActor.h"
#include "NxScene.h"

#include "NwTypes.h"

class NwDebugRenderer;

class NxPhysicsSDK;
class NxScene;
class NxShapeDesc;
// -------------------------------------------------------------------------------------------------------------------
class NwInterface 
{
public:
    NwInterface();
    ~NwInterface();

    void createScene();              // Clears any current NxScene and creates a new one. 
    void deleteScene();              // Destroys all dynamic memory allocated by NwInterface.

    void stepScene(double timeStep); // Batches up small steps to pass Novodex 1/60 second steps.

    // Loads precomputed physics data from disk, or creates it on the fly and caches it to disk.
    void* loadMeshShape(const char* name, const void* vertArray, int vertCount, int vertStride, const void* triArray, 
        int triCount, int triStride, bool indexIs16Bit = false);

	void* createBoxActor(const NxVec3& position, NxReal density, const NxVec3& size);
	void* createSphereActor(const NxVec3& position, NxReal density, NxReal radius);
    void* createMeshActor(const NxVec3& position, NxReal* orientation, NxShapeDesc* shapeDesc);
	void  deleteActor(NxActor* actor);
	void  getOrientation(NxActor* actor, float* position, float (*rotation)[3]);
	void  moveActor(NxActor* actor, const NxVec3& position, NxReal* orientation);

	void createDebugger(NwDrawFunc drawFunc);
	void deleteDebugger();

private:
    // NovodeX-specific parameters.
    NxPhysicsSDK*    m_physicsSDK; 
    NxScene*         m_scene;
	NwDebugRenderer* m_debugRenderer;

    // Store shape descriptors for later deletion.
    NxArray< NxShapeDesc* > m_shapes;

    double m_elapsedTime;
};
// -------------------------------------------------------------------------------------------------------------------
// INLINE FUNCTION DEFINITIONS	  														   INLINE FUNCTION DEFINITIONS
// -------------------------------------------------------------------------------------------------------------------
inline void NwInterface::deleteActor(NxActor* actor)
{
	m_scene->releaseActor(*actor);
}
// -------------------------------------------------------------------------------------------------------------------
#endif // INCLUDED_NW_INTERFACE_H
