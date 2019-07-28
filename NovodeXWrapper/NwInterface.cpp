// -------------------------------------------------------------------------------------------------------------------
// NOVODEX SDK WRAPPER INTERFACE
// Written by Tom Lassanske, 03-01-05
// -------------------------------------------------------------------------------------------------------------------
#include "NwWrapper.h"
#if 0 && NOVODEX

#include "NwInterface.h"
#include "NwDebugRenderer.h"
#include "NwStream.h"

// Basic SDK includes.
#include "NxPhysicsSDK.h"
#include "NxMaterial.h"
#include "NxScene.h"

// Shape and actor includes.
#include "NxActor.h"
#include "NxTriangleMeshDesc.h"
#include "NxTriangleMeshShapeDesc.h"

#include "NxCooking.h"
// -------------------------------------------------------------------------------------------------------------------
NwInterface::NwInterface() : m_scene(NULL), m_debugRenderer(NULL)
{
    // Create the physics SDK and set relevant parameters.
    {
        m_physicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
        assert(m_physicsSDK);

        m_physicsSDK->setParameter(NX_MIN_SEPARATION_FOR_PENALTY, -0.01f);
        m_physicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
        m_physicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
        m_physicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
    }

    // Create a default material.
    {
        NxMaterial defaultMaterial;
        defaultMaterial.restitution = 0.5f;
        defaultMaterial.staticFriction = 0.5f;
        defaultMaterial.dynamicFriction = 0.5f;
        m_physicsSDK->setMaterialAtIndex(0, &defaultMaterial);
    }
}
// -------------------------------------------------------------------------------------------------------------------
NwInterface::~NwInterface()
{
    while (m_shapes.size())
    {
        delete m_shapes.back();
        m_shapes.popBack();
    }
}
// -------------------------------------------------------------------------------------------------------------------
void NwInterface::createScene()
{
    // Release any old scene, which will release the contents, as well.
    if (m_scene)
    {
        m_physicsSDK->releaseScene(*m_scene);
        m_scene = NULL;
    }

    // Create a new scene.
    {
        NxSceneDesc sceneDesc;
        sceneDesc.gravity = NxVec3(0, -10, 0);
        sceneDesc.broadPhase = NX_BROADPHASE_COHERENT;
        sceneDesc.collisionDetection = true;

        m_scene = m_physicsSDK->createScene(sceneDesc);
        m_scene->setTiming(0.016f, 1, NX_TIMESTEP_FIXED);
    }

    m_elapsedTime = 0.0f;
}
// -------------------------------------------------------------------------------------------------------------------
void NwInterface::stepScene(double stepTime)
{
    m_elapsedTime += stepTime;

    while (m_elapsedTime > 0.016)
    {
        m_elapsedTime -= 0.016;

        // Simulate in new thread and block until finished simulating.  TODO: Make loop asynchronous.
        m_scene->simulate(0.016f);
        while (!m_scene->fetchResults(NX_RIGID_BODY_FINISHED, true));

		if (m_debugRenderer)
		{
			m_scene->visualize();
			m_physicsSDK->visualize(*m_debugRenderer);
		}
    }
}
// -------------------------------------------------------------------------------------------------------------------
void* NwInterface::loadMeshShape(const char* name, const void* vertArray, int vertCount, int vertStride, 
const void* triArray, int triCount, int triStride, bool indexIs16Bit)
{
    char filePath[160];
    sprintf(filePath, "C:/game/data/novodex/%s.mesh", name);

    NwStream stream(filePath, true);
    NxTriangleMesh* mesh;
    
    if (stream.isValid())
    {
        mesh = m_physicsSDK->createTriangleMesh(stream);
    }
    else
    {
        // Create descriptor for concave mesh
        NxTriangleMeshDesc meshDesc;
        meshDesc.numVertices         = vertCount;
        meshDesc.pointStrideBytes    = vertStride;
        meshDesc.points              = vertArray;
        meshDesc.numTriangles        = triCount;
        meshDesc.triangles           = triArray;
        meshDesc.triangleStrideBytes = triStride;
        meshDesc.flags               = 0;

        if (indexIs16Bit)
        {
            meshDesc.flags |= NX_MF_16_BIT_INDICES;
        }

        NxInitCooking();
        bool status = NxCookTriangleMesh(meshDesc, NwStream(filePath, false));
        mesh = m_physicsSDK->createTriangleMesh(NwStream(filePath, true));
    }

    NxTriangleMeshShapeDesc* shapeDesc = new NxTriangleMeshShapeDesc();
    shapeDesc->meshData = mesh;

    m_shapes.pushBack(shapeDesc);

    return reinterpret_cast< void* >(shapeDesc);
}
// -------------------------------------------------------------------------------------------------------------------
void* NwInterface::createBoxActor(const NxVec3& position, NxReal density, const NxVec3& size)
{
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = size;

	NxBodyDesc bodyDesc;

	NxActorDesc actorDesc;
	actorDesc.body    = density ? &bodyDesc   : NULL;
	actorDesc.density = density;
	actorDesc.globalPose.t = position;
	actorDesc.shapes.pushBack(&boxDesc);

	return m_scene->createActor(actorDesc);	
}
// -------------------------------------------------------------------------------------------------------------------
void* NwInterface::createSphereActor(const NxVec3& position, NxReal density, NxReal radius)
{
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;

	NxBodyDesc bodyDesc;
	
	if(density < 0)
	{
		bodyDesc.flags |= NX_BF_KINEMATIC;
		density *= -1;
	}

	NxActorDesc actorDesc;
	actorDesc.body    = density ? &bodyDesc   : NULL;
	actorDesc.density = density;
	actorDesc.globalPose.t = position;
	actorDesc.shapes.pushBack(&sphereDesc);

	return m_scene->createActor(actorDesc);	
}
// -------------------------------------------------------------------------------------------------------------------
void* NwInterface::createMeshActor(const NxVec3& position, NxReal* orientation, NxShapeDesc* shapeDesc)
{
    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack(shapeDesc);
    actorDesc.globalPose.t = position;
    actorDesc.globalPose.M.setColumnMajor(orientation);
 
    return m_scene->createActor(actorDesc);
}
// -------------------------------------------------------------------------------------------------------------------
void NwInterface::getOrientation(NxActor* actor, float* position, float (*rotation)[3])
{
	position[0] = actor->getGlobalPosition()[0];
	position[1] = actor->getGlobalPosition()[1];
	position[2] = actor->getGlobalPosition()[2];
	
	int i;
	
	for(i = 0; i < 3; i++){
		rotation[i][0] = actor->getGlobalOrientation().getColumn(0)[i];
		rotation[i][1] = actor->getGlobalOrientation().getColumn(1)[i];
		rotation[i][2] = actor->getGlobalOrientation().getColumn(2)[i];
	}
}
// -------------------------------------------------------------------------------------------------------------------
void NwInterface::moveActor(NxActor* actor, const NxVec3& position, NxReal* orientation)
{
	if(actor->readBodyFlag(NX_BF_KINEMATIC))
	{
		actor->moveGlobalPosition(position);
	}
	else
	{
		actor->setGlobalPosition(position);
	}
}
// -------------------------------------------------------------------------------------------------------------------
void NwInterface::createDebugger(NwDrawFunc drawFunc)
{
	if (!m_debugRenderer)
	{
		m_debugRenderer = new NwDebugRenderer(drawFunc);
	}
}
// -------------------------------------------------------------------------------------------------------------------
void NwInterface::deleteDebugger()
{
	delete m_debugRenderer;
	m_debugRenderer = NULL;
}
// -------------------------------------------------------------------------------------------------------------------
#endif