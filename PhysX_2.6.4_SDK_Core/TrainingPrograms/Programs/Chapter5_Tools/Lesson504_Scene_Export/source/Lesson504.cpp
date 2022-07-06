// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//			                  LESSON 504: SCENE EXPORT
//
//   Scene Export: This program shows how to export and import a scene to
//	 and from a special file format
//
//   Written by Stephen Hatcher, based on work by Pierre Terdiman (01.01.04) 
//   and others
//
//						    Edited by Bob Schade, 9-15-06
// ===============================================================================

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/glut.h>
#include <stdio.h>

#include "Lesson504.h"
#include "UpdateTime.h"
#include "MediaPath.h"

// include exporter here
#include "NXU_helper.h"  // NxuStream helper functions.

// Physics SDK globals
NxPhysicsSDK*     gPhysicsSDK = NULL;

const NxU32       gMaxScenes = 3;
NxScene*          gScenes[gMaxScenes] = { NULL, NULL, NULL };
NxU32             gCurrentScene = 0;

NxVec3            gDefaultGravity(0,-9.8,0);

// User report globals
DebugRenderer    gDebugRenderer;
UserAllocator*	 gAllocator;
ErrorStream      gErrorStream;

// HUD globals
HUD hud;
 
// Display globals
int gMainHandle;
int mx = 0;
int my = 0;

// Camera globals
float gCameraAspectRatio = 1;
NxVec3 gCameraPos(0,5,-15);
NxVec3 gCameraForward(0,0,1);
NxVec3 gCameraRight(-1,0,0);
const NxReal gCameraSpeed = 10;

// Force globals
NxVec3 gForceVec(0,0,0);
NxReal gForceStrength = 50000;
bool bForceMode = true;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Simulation globals
NxReal gDeltaTime = 1.0/60.0;
bool bHardwareScene = false;
bool bPause = false;
bool bShadows = true;
bool bDebugWireframeMode = false;

// Data Directory Paths (solution, binary, install)
char fname[] = "../../../../Data/bin";
char fname1[] = "../../../Data/bin";
char fname2[] = "../../TrainingPrograms/Programs/Data/bin";
char fname3[] = "TrainingPrograms/Programs/Data/bin";

// Actor globals
NxActor* groundPlane = NULL;

// Focus actor
NxActor* gSelectedActor = NULL;

// Export Flags
bool gSave = false;
int  gLoad = 0;
bool gClear = false;
bool gCreateCube = false;
bool gCreateStack10 = false;
bool gCreateStack30 = false;
bool gCreateTower30 = false;
bool gCreateCubeCam = false;


// BEGIN NEW STUFF

#if NX_USE_FLUID_API

class FluidMem
{
public:

  FluidMem(NxFluidDesc &desc)
  {

  	NxParticleData &data = desc.particlesWriteData;

  	data.maxParticles = desc.maxParticles;

  	NxParticleData &initial = desc.initialParticleData;
		NxU32 pcount = 0;
		if ( initial.numParticlesPtr )
  	 pcount            = *initial.numParticlesPtr; // number of particles in the initial data field.

  	if (initial.bufferPosByteStride && pcount )
  	{
  		data.bufferPosByteStride = initial.bufferPosByteStride;
  		data.bufferPos = (NxF32*) new NxU8[data.maxParticles*data.bufferPosByteStride];
  		memcpy(data.bufferPos, initial.bufferPos, sizeof(data.bufferPosByteStride*pcount) );
  	}
  	else
  	{
  		data.bufferPos = (NxF32*) new NxU8[data.maxParticles*sizeof(NxVec3)];
  		data.bufferPosByteStride = sizeof(NxVec3);
  	}

  	if (initial.bufferVelByteStride && pcount )
  	{
  		data.bufferVelByteStride = initial.bufferVelByteStride;
  		data.bufferVel = (NxF32*) new NxU8[data.maxParticles*data.bufferVelByteStride];
  		memcpy(data.bufferVel, initial.bufferVel, sizeof(data.bufferVelByteStride*pcount) );
  	}
  	else
  	{
  		data.bufferVel = (NxF32 *) new NxU8[data.maxParticles*sizeof(NxVec3)];
  		data.bufferVelByteStride = sizeof(NxVec3);
  	}

  	if (initial.bufferLifeByteStride && pcount)
  	{
  		data.bufferLifeByteStride = initial.bufferLifeByteStride;
  		data.bufferLife = (NxF32*) new NxU8[data.maxParticles*data.bufferLifeByteStride];
  		memcpy(data.bufferLife, initial.bufferLife, sizeof(data.bufferLifeByteStride*pcount) );
  	}
  	else
  	{
  		data.bufferLife = new NxF32[data.maxParticles];
  		data.bufferLifeByteStride = sizeof(NxF32);
  	}

  	if (initial.bufferDensityByteStride && pcount )
  	{
  		data.bufferDensityByteStride = initial.bufferDensityByteStride;
  		data.bufferDensity = (NxF32 *)new NxU8[data.maxParticles*data.bufferDensityByteStride];
  		memcpy(data.bufferDensity, initial.bufferDensity, sizeof(data.bufferDensityByteStride*pcount) );
  	}
  	else
  	{
  		data.bufferDensity = new NxF32[data.maxParticles];
  		data.bufferDensityByteStride = sizeof(NxF32);
  	}

    #if NX_SDK_VERSION_NUMBER >= 260
    if ( initial.bufferFlagByteStride && pcount )
  	{
  		data.bufferFlagByteStride = initial.bufferFlagByteStride;
  		data.bufferFlag = (NxU32*) new NxU8[data.maxParticles*data.bufferFlagByteStride];
  		memcpy(data.bufferFlag, initial.bufferFlag, sizeof(data.bufferFlagByteStride*pcount) );
  	}
  	else
  	{
  		data.bufferFlag = new NxU32[data.maxParticles];
  		data.bufferFlagByteStride = sizeof(NxU32);
  	}
    #endif

    #if NX_SDK_VERSION_NUMBER >= 260
    if ( initial.bufferIdByteStride && pcount )
  	{
  		data.bufferIdByteStride = initial.bufferIdByteStride;
  		data.bufferId = (NxU32*) new NxU8[data.maxParticles*data.bufferIdByteStride];
  		memcpy(data.bufferId, initial.bufferId, sizeof(data.bufferIdByteStride*pcount) );
  	}
  	else
  	{
  		data.bufferId = new NxU32[data.maxParticles];
  		data.bufferIdByteStride = sizeof(NxU32);
  	}
    #endif

  	data.numParticlesPtr = &mNumParticles;
  	*data.numParticlesPtr = pcount;


  	desc.userData = this; // assign the user data pointer to this allocated helper class so we can delete it later.

		mData = data;

  }

  ~FluidMem(void)
  {
  	delete mData.bufferPos;
  	delete mData.bufferVel;
  	delete mData.bufferLife;
  	delete mData.bufferDensity;
#if NX_SDK_VERSION_NUMBER >= 260
  	delete mData.bufferFlag;
  	delete mData.bufferId;
#endif
  }

  NxU32  mNumParticles;
  NxParticleData	mData;

};


#endif

class ClothMem
{
public:
  ClothMem(NxClothDesc &desc)
  {
  	desc.userData = this;
  	mClothIndices = 0;
  	mClothVertices = 0;

	  NxClothMesh *clothMesh = desc.clothMesh;
	  NxClothMeshDesc cdesc;
	  clothMesh->saveToDesc(cdesc);

		NxU32 vcount = cdesc.numVertices;
		NxU32 tcount = cdesc.numTriangles;

		if ( desc.flags & NX_CLF_TEARABLE )
		{
			vcount*=2;
			tcount*=2;
		}

		desc.flags|=NX_CLF_VISUALIZATION;

		if ( vcount && tcount )
		{

  		mClothVertices = new NxF32[vcount*3];
  		mClothIndices  = new NxU32[tcount*3];

      NxMeshData &mesh = desc.meshData;

      mesh.verticesPosBegin = mClothVertices;
      mesh.verticesNormalBegin = 0;
      mesh.verticesPosByteStride = sizeof(NxF32)*3;
      mesh.verticesNormalByteStride = 0;
      mesh.maxVertices = vcount;
      mesh.numVerticesPtr = &mNumVertices;
      mesh.indicesBegin = mClothIndices;
			mesh.indicesByteStride = sizeof(NxU32);
      mesh.maxIndices   = tcount*3;
      mesh.numIndicesPtr = &mNumIndices;
      mesh.parentIndicesBegin = 0;
      mesh.parentIndicesByteStride = 0;
      mesh.maxParentIndices = 0;
      mesh.numParentIndicesPtr = 0;
      mesh.flags = 0;
    }

  }

  ~ClothMem(void)
  {
  	delete mClothVertices;
  	delete mClothIndices;
  }

	NxU32  mNumIndices;
	NxU32  mNumVertices;

  NxU32 *mClothIndices;
  NxF32 *mClothVertices;
};

#if NX_USE_FLUID_API
void releaseFluids(NxScene *scene)
{
	if ( scene )
	{
  	NxU32 nb = scene->getNbFluids();
  	if ( nb )
  	{
  		NxFluid **fluids = scene->getFluids();
  		for (NxU32 i=0; i<nb; i++)
  		{
  			NxFluid *f = fluids[i];
  			FluidMem *fmem = (FluidMem *) f->userData;
  			delete fmem;
  		}
  	}
  }
}
#else
//No fluids
void releaseFluids(NxScene *scene)
{
}
#endif

void releaseCloths(NxScene *scene)
{
	if ( scene )
	{
  	NxU32 nb = scene->getNbCloths();
  	if ( nb )
  	{
  		NxCloth **cloths = scene->getCloths();
  		for (NxU32 i=0; i<nb; i++)
  		{
  			NxCloth *c = cloths[i];
  			ClothMem *cmem = (ClothMem *) c->userData;
  			delete cmem;
  		}
  	}
  }
}

void releaseScene(NxScene &scene)
{
	releaseFluids(&scene);
	releaseCloths(&scene);
	gPhysicsSDK->releaseScene(scene);
}


class MyUserNotify: public NXU_userNotify, public NXU_errorReport
{
public:
		virtual void NXU_errorMessage(bool isError, const char *str)
		{
			if (isError)
			{
				printf("NxuStream ERROR: %s\r\n", str);
			}
			else
			{
				printf("NxuStream WARNING: %s\r\n", str);
			}
		}

	virtual void	NXU_notifyScene(NxU32 sno,	NxScene	*scene,	const	char *userProperties)
  {
		gScenes[gCurrentScene] = scene;
		gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1.0f);
		gPhysicsSDK->setParameter(NX_VISUALIZE_FLUID_POSITION, 0.2f );
		gPhysicsSDK->setParameter(NX_VISUALIZE_CLOTH_MESH, 1.0f );
  };

	virtual void	NXU_notifyJoint(NxJoint	*joint,	const	char *userProperties){};

	virtual void	NXU_notifyActor(NxActor	*actor,	const	char *userProperties){};

	virtual void	NXU_notifyCloth(NxCloth	*cloth,	const	char *userProperties){};

	virtual void	NXU_notifyFluid(NxFluid	*fluid,	const	char *userProperties){};

 	virtual void 	NXU_notifyTriangleMesh(NxTriangleMesh *t,const char *userProperties){};

 	virtual void 	NXU_notifyConvexMesh(NxConvexMesh *c,const char *userProperties){};

 	virtual void 	NXU_notifyClothMesh(NxClothMesh *t,const char *userProperties){};

 	virtual void 	NXU_notifyCCDSkeleton(NxCCDSkeleton *t,const char *userProperties){};

	virtual void 	NXU_notifyHeightField(NxHeightField *t,const char *userProperties){};

	virtual NxScene *NXU_preNotifyScene(unsigned	int	sno, NxSceneDesc &scene, const char	*userProperties)
  {
		assert( gScenes[gCurrentScene] == 0 );
		if ( gScenes[gCurrentScene] )
		{
			releaseScene(*gScenes[gCurrentScene]);
			gScenes[gCurrentScene] = 0;
		}
		return 0;
  };

	virtual bool	NXU_preNotifyJoint(NxJointDesc &joint, const char	*userProperties) { return	true; };

	virtual bool	NXU_preNotifyActor(NxActorDesc &actor, const char	*userProperties)
  {
		for (NxU32 i=0; i<actor.shapes.size(); i++)
		{
			NxShapeDesc *s = actor.shapes[i];
			s->shapeFlags|=NX_SF_VISUALIZATION; // make sure the shape visualization flags are on so we can see things!
		}
		return true;
  };

	virtual bool	NXU_preNotifyCloth(NxClothDesc &cloth, const char	*userProperties)
  {
  	ClothMem *mem = new ClothMem(cloth);
    return	true;
  };

#if NX_USE_FLUID_API
	virtual bool	NXU_preNotifyFluid(NxFluidDesc &fluid, const char	*userProperties)
  {
  	FluidMem *mem = new FluidMem(fluid);
  	return true;
  };
#endif


	virtual bool 	NXU_preNotifyTriangleMesh(NxTriangleMeshDesc &t,const char *userProperties) { return true;	};

	virtual bool 	NXU_preNotifyConvexMesh(NxConvexMeshDesc &t,const char *userProperties)	{	return true; };

	virtual bool 	NXU_preNotifyClothMesh(NxClothMeshDesc &t,const char *userProperties) { return true; };

	virtual bool 	NXU_preNotifyCCDSkeleton(NxSimpleTriangleMesh &t,const char *userProperties)	{	return true; };

	virtual bool 	NXU_preNotifyHeightField(NxHeightFieldDesc &t,const char *userProperties) {	return true; }

	virtual bool 	NXU_preNotifySceneInstance(const char *name,const char *sceneName,const char *userProperties,NxMat34 &rootNode,NxU32 depth) { return true; };


	virtual void NXU_notifySceneFailed(unsigned	int	sno, NxSceneDesc &scene, const char	*userProperties) { };

	virtual void	NXU_notifyJointFailed(NxJointDesc &joint, const char	*userProperties) {  };

	virtual void	NXU_notifyActorFailed(NxActorDesc &actor, const char	*userProperties) { };

	virtual void	NXU_notifyClothFailed(NxClothDesc &cloth, const char	*userProperties)
  {
  	ClothMem *mem = (ClothMem *) cloth.userData;
  	delete mem;
  };

#if NX_USE_FLUID_API
	virtual void	NXU_notifyFluidFailed(NxFluidDesc &fluid, const char	*userProperties)
  {
  	FluidMem *mem = (FluidMem *) fluid.userData;
  	delete mem;
  };
#endif

	virtual void 	NXU_notifyTriangleMeshFailed(NxTriangleMeshDesc &t,const char *userProperties) {	};

	virtual void 	NXU_notifyConvexMeshFailed(NxConvexMeshDesc &t,const char *userProperties)	{	 };

	virtual void 	NXU_notifyClothMeshFailed(NxClothMeshDesc &t,const char *userProperties) { };

	virtual void 	NXU_notifyCCDSkeletonFailed(NxSimpleTriangleMesh &t,const char *userProperties)	{	 };

	virtual void 	NXU_notifyHeightFieldFailed(NxHeightFieldDesc &t,const char *userProperties) {	}


};

MyUserNotify gUserNotify;


//==================================================================================
//This is the function that calls the importer to load the state of the currently selected scene from a file
NX_BOOL LoadScene(const char *pFilename,NXU::NXU_FileType type)
{
	NX_BOOL success = false;

	if (gPhysicsSDK)
	{
		char LoadFilename[512];
		FindMediaFile(pFilename, LoadFilename);

		NXU::NxuPhysicsCollection *c = NXU::loadCollection(LoadFilename, type );

		if ( c )
		{
			if (gScenes[gCurrentScene])
			{
				// Release old scene
				releaseScene(*gScenes[gCurrentScene]);
				gScenes[gCurrentScene] = 0;
			}

			if (gPhysicsSDK)
			{
				success = NXU::instantiateCollection( c, *gPhysicsSDK, 0, 0, &gUserNotify );
			}
			NXU::releaseCollection(c);
		}
		else
		{
		}
	}

	gLoad = 0;

	if ( success )
	  printf("Scene %d loaded from file %s.\n", gCurrentScene, pFilename);

	return success;
}

//==================================================================================
//This is the function that calls the exporter to save out the state of the currently selected scene to a file
void SaveScene(const char *pFilename)
{
	char SaveFilename[512];
	GetTempFilePath(SaveFilename);
	strcat(SaveFilename, pFilename);

	NXU::NxuPhysicsCollection *c = NXU::extractCollectionScene(gScenes[gCurrentScene]);

	if (c)
	{
		char scratch[512];
		sprintf(scratch, "%s.xml", SaveFilename);
		printf("Saving NxuStream XML file to '%s'\r\n", scratch);
		NXU::saveCollection(c, scratch, NXU::FT_XML, false, false);

		sprintf(scratch, "%s.nxb", SaveFilename);
		printf("Saving NxuStream BINARY file to '%s'\r\n", scratch);
		NXU::saveCollection(c, scratch, NXU::FT_BINARY, true, true);

		sprintf(scratch, "%s.dae", SaveFilename);
		printf("Saving COLLADA Physics 1.4.1 file to '%s'\r\n", scratch);
		NXU::saveCollection(c, scratch, NXU::FT_COLLADA, false, false);

		NXU::releaseCollection(c);

	}

}



// END NEW STUFF




#if 0

//Set the file format to be used, only one of these three options may be used

//This format will generally be the fastest to read/write on the local operating system
static NxuStreamFactory::FormatType gFileFormat = NxuStreamFactory::BINARY;

//This format will generally be the fastest to read/write on Windows based OS
//static NxuStreamFactory::FormatType gFileFormat = NxuStreamFactory::BINARY_LITTLE_END;

//This format will generally be the fastest to read/write on XBOX 360/MAC/*nix etc.
//static NxuStreamFactory::FormatType gFileFormat = NxuStreamFactory::BINARY_BIG_END;

//This format is human readable, so it is good for debugging and understanding the structure of the data, 
// but it can't be read back in so it is not intended as a persistance mechanism
//static NxuStreamFactory::FormatType gFileFormat = NxuStreamFactory::ASCII;

//This format is XML so it is somewhat human readable, but also compatible with the Collada standard so
// that it can be used to transfer to other tools that support the standand. Note that tools without
// PhysX directly integrated will lose information as not all fields are currently standardized.
//static NxuStreamFactory::FormatType gFileFormat = NxuStreamFactory::COLLADA;


//This is a callback class that will inform us of things being created during import
//This provides the opportunity to fill in user data or otherwise link them to application objects
class UserNotify : public NxuUserNotify
{
public:

    virtual	void NotifyScene(NxScene *scene) 
    {
        //Here we associate the new scene pointer with our local application variable
        //We stored a reference to that variable during scene creation, so we can assign it directly here
        if (scene->userData)
        {
            *((NxScene**)(scene->userData)) = scene;
        }
        else
        {
            //No variable was assigned, so use some other means of storing the pointer;
            gScenes[gCurrentScene] = scene;
        }
    }

	/**
	* Notify application of	a joint	creation.
	*/
    virtual	void NotifyJoint(NxJoint *joint) {}

	/**
	* Notify application of	a actor	creation.
	*/
    virtual	void NotifyActor(NxActor *actor) {}

#if NXU_SDK_FEATURES & NXU_SDK_HAS_FLUID_API
	/**
	* Notify application of	a fluid	creation. Application should call setParticlesWriteData() to specify destination of fluid updates
	*/
    virtual void NotifyFluid(NxFluid *fluid) 
    {	
     //   NxParticleData particles;
	    //particles.maxParticles			= gParticleBufferCap;
	    //particles.numParticlesPtr		= &gParticleBufferNum;
	    //particles.bufferPos				= &gParticleBuffer[0].x;
	    //particles.bufferPosByteStride	= sizeof(NxVec3);
     //   fluid->setParticlesWriteData(particles);
    }
#endif

};
UserNotify gUserNotify;



//This is a callback class that allows the application to control the instantiation of objects within the SDK
//This can be used to modify the descriptors or to filter out things that aren't desired
//It is optional. If not provided the importer will 
class UserCreate: public NxuUserCreate
{
public:
  virtual NxScene * CreateScene(NxPhysicsSDK *sdk, NxuSceneDesc &desc)
  {
      //In this example with assign the variable we will use to store the scene pointer
      // See MyUserNotify::NotifyScene to see how this is used
       desc.userData = (void*)&gScenes[gCurrentScene];

       //This would be the proper place to add a custom scheduler for managing the scenes tasks.
       // Schedulers can't be saved to a stream as they are user classes. They can't be added to an
       //  existing scene, so this is the only time when they can be associated with a scene.
       // See SampleThreading.h for an example of how to create a custom scheduler.

       //Create and return the scene now that we are done modifying the description
       return sdk->createScene(desc);
      
  }
};
UserCreate gUserCreate;

//==================================================================================
//This is the function that calls the exporter to save out the state of the currently selected scene to a file
void SaveScene(const char* pFilename)
{
    //Open our file directly so we can write out application specific data first.
	char SaveFilename[512];
	GetTempFilePath(SaveFilename);
	strcat(SaveFilename, pFilename);
    FILE* file = fopen(SaveFilename, "wb");
    if (!file) return;

    NxU32 actorCount = gScenes[gCurrentScene]->getNbActors();
    NxActor** actorList = gScenes[gCurrentScene]->getActors();


    //Write out a custom header. For this example, we will just write out a count of the number of actors
    //Note that to make the file be portable across platforms, endian issues would need to be handled here
    fwrite(&actorCount, 1, sizeof(actorCount), file);

    //Create a stream to write Physx data to our file
    NxuOutputStream* stream = NxuStreamFactory::getOutputStream(gFileFormat, file);
    if (stream) 
    {
        //Create an exporter to write out the scene.
        NxuPhysicsExport exporter;
        exporter.setStream(stream);  //Assign it the stream

        for (NxU32 i=0;i<actorCount;++i)
        {
            //For each actor, we first right out our application specific data for that actor
            //In this case we will store the user data, which is the half-extent of the cube.
            int size = (int)(actorList[i]->userData);
            fwrite(&size, 1, sizeof(size), file);

            //Now stream the actor
            exporter.Write(actorList[i]);
            //Force everything into the stream so that we can continue writing to the file
            exporter.flush();
            //Clear out the previous actor to prevent it from being streamed again
            //Otherwise it would be cached to allow subsequent joints and effectors to reference it
            exporter.clear();
        }
    }
    //We are done streaming Physx objects
    NxuStreamFactory::releaseStream(stream);

    //Write out any other application data needed. We will write out a string to be echoed on load
    // so that we can easily see if any file corruption has occured.
    char buffer[256];
    sprintf(buffer, "File successfully loaded!\n");
    int length = strlen(buffer);
    fwrite(&length,1,sizeof(length), file);
    fwrite(buffer,length, sizeof(char), file);

    fclose(file);
    gSave = false;
    printf("Scene %d saved to file %s.\n", gCurrentScene, pFilename);
}

//==================================================================================
//This is the function that calls the importer to load the state of the currently selected scene from a file
NX_BOOL LoadScene(const char* pFilename)
{
	NX_BOOL success = false;

	if ( gScenes[gCurrentScene] )
	{
        // Load our actor file into the current scene

		if ( gPhysicsSDK )
		{
			char LoadFilename[512];
			FindMediaFile(pFilename, LoadFilename);
            FILE* file = fopen(LoadFilename, "rb");
			if (!file) {
				printf("%s\n", "Before loading, please first save a scene by pressing '1'");
				gLoad = false;
				return false;
			}

			//Release the current scene
			if (gScenes[gCurrentScene] != NULL) {
				gPhysicsSDK->releaseScene(*gScenes[gCurrentScene]);
				gScenes[gCurrentScene] = NULL;
			}

			InitBlankScene();

            //Read in our application header. This was the number of actors in the file.
            NxU32 count;
            fread(&count, 1, sizeof(count), file);

            //Get a stream to read the PhysX actors
            NxuInputStream* stream = NxuStreamFactory::getInputStream(gFileFormat, file);
            if (stream) 
            {
                NxuPhysicsImport importer; //Create the importer

                for (NxU32 i=0;i<count;++i)
                {
                    //Read in our application data for the actor
                    int size;
                    fread(&size, 1, sizeof(size), file);

                    NxuSectionTypes type;
                    //Read in the PhysX actor
                    type = importer.parse(*stream); //Read in the stream
                    assert(type == NxuTypeActor); //Should be an actor!

                    //We could use the following call to have the importer creat the object for us
                    //importer.instantiate(*gPhysicsSDK, &gUserNotify, &gUserCreate); //Add the imported data to the SDK

                    //However, for this example we will show how to simple grab the descriptor directly
                    const NxuPhysicsCollection* collection = importer.getCollection();

                    //Since we just imported an actor directly, we extract it from the collection's default model
                    //If we had imported a whole scene, we would need to traverse the scene to examine all the descriptors
                    NxActorDesc* actor = collection->defaultModel.mActors[0];
                    assert(actor); //There should be one there!

                    actor->userData = (void*)size; //Assign our custom user data
                    NxActor* instance = gScenes[gCurrentScene]->createActor(*actor); //Add the actor to the scene

                    //Clear the collection
                    importer.reset();
                }
                success = true;
            }
            //Realease the stream as we are done reading in PhysX data
            NxuStreamFactory::releaseStream(stream);

            //Read in the validation string we put at the end
            int length;
            fread(&length, 1, sizeof(length), file);
            char buffer[256];
            memset(buffer, 0, 256); //Clear it out
            fread(buffer, length, sizeof(char), file);
            printf(buffer); //Echo the confirmation so we can see file was read in correctly.
            fclose(file);
		}
	}

    gLoad = false;
    printf("Scene %d loaded from file %s.\n", gCurrentScene, pFilename);

	return success;
}

#endif  // 0




void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n x = Toggle Shadows\n t = Move Focus Actor to (0,5,0)\n");
	printf("\n Special:\n --------\n t = Switch Scene, 1 = Save Scene, 3 = Load Scene, c = Clear Scene\n");
	printf(" <space> = Create Random Sized Cube, 8 = Create Stack(10), 9 = Create Stack(30)\n");
	printf(" 0 = Create Tower(30), v = Fire Cube\n");
}

bool IsSelectable(NxActor* actor)
{
   NxShape*const* shapes = gSelectedActor->getShapes();
   NxU32 nShapes = gSelectedActor->getNbShapes();
   while (nShapes--)
   {
       if (shapes[nShapes]->getFlag(NX_TRIGGER_ENABLE)) 
       {           
           return false;
       }
   }

   if (actor == groundPlane)
       return false;

   return true;
}

void SelectNextActor()
{
	NxScene* scene = gScenes[gCurrentScene];
	if (scene)
	{
        NxU32 nbActors = scene->getNbActors();
        NxActor** actors = scene->getActors();
        for (NxU32 i = 0; i < nbActors; i++)
        {
            if (actors[i] == gSelectedActor)
            {
                NxU32 j = 1;
                gSelectedActor = actors[(i+j)%nbActors];
                while (!IsSelectable(gSelectedActor))
                {
                    j++;
                    gSelectedActor = actors[(i+j)%nbActors];
                }
                break;
            }
        }
	}
}

void ProcessCameraKeys()
{
	NxReal deltaTime;

    if (bPause) deltaTime = 0.0005; else deltaTime = gDeltaTime;  

	// Process camera keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Camera controls
			case 'w':{ gCameraPos += gCameraForward*gCameraSpeed*deltaTime; break; }
			case 's':{ gCameraPos -= gCameraForward*gCameraSpeed*deltaTime; break; }
			case 'a':{ gCameraPos -= gCameraRight*gCameraSpeed*deltaTime; break; }
			case 'd':{ gCameraPos += gCameraRight*gCameraSpeed*deltaTime; break; }
			case 'z':{ gCameraPos -= NxVec3(0,1,0)*gCameraSpeed*deltaTime; break; }
			case 'q':{ gCameraPos += NxVec3(0,1,0)*gCameraSpeed*deltaTime; break; }
		}
	}
}

void SetupCamera()
{
	// Setup camera
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x,gCameraPos.y,gCameraPos.z,gCameraPos.x + gCameraForward.x, gCameraPos.y + gCameraForward.y, gCameraPos.z + gCameraForward.z, 0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderActors(bool shadows)
{
	NxScene* scene = gScenes[gCurrentScene];
	if (scene)
	{
		// Render all the actors in the scene
        NxU32 nbActors = scene->getNbActors();
        NxActor** actors = scene->getActors();
        while (nbActors--)
        {
            NxActor* actor = *actors++;
            DrawActor(actor, gSelectedActor, true);

            // Handle shadows
            if (shadows)
            {
			    DrawActorShadow(actor, true);
            }
        }
	}
}

void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color)
{
	// Draw only if the force is large enough
	NxReal force = forceVec.magnitude();
	if (force < 0.1)  return;

	forceVec = 3*forceVec/force;

	NxVec3 pos = actor->getCMassGlobalPosition();
	DrawArrow(pos, pos + forceVec, color);
}

NxVec3 ApplyForceToActor(NxActor* actor, const NxVec3& forceDir, const NxReal forceStrength, bool forceMode)
{
	NxVec3 forceVec = forceStrength*forceDir*gDeltaTime;

	if (forceMode)
		actor->addForce(forceVec);
	else 
		actor->addTorque(forceVec);

	return forceVec;
}

void ProcessForceKeys()
{
	if (!gSelectedActor)  return;

	// Process force keys
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }

		switch (i)
		{
			// Force controls
			case 'i': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,1),gForceStrength,bForceMode); break; }
			case 'k': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,0,-1),gForceStrength,bForceMode); break; }
			case 'j': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(1,0,0),gForceStrength,bForceMode); break; }
			case 'l': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(-1,0,0),gForceStrength,bForceMode); break; }
			case 'u': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,1,0),gForceStrength,bForceMode); break; }
			case 'm': { gForceVec = ApplyForceToActor(gSelectedActor,NxVec3(0,-1,0),gForceStrength,bForceMode); break; }
		}
	}
}

void ProcessInputs()
{
    ProcessForceKeys();

	//Check if a request was made to clear, save or load a scene
	if (gClear)
	{
		if (gScenes[gCurrentScene])
		{
			gPhysicsSDK->releaseScene(*gScenes[gCurrentScene]);
		}
		InitBlankScene();
		gClear = false;
	}
	else if (gSave)
	{
		char scratch[512];
		sprintf(scratch,"SceneSave%d", gCurrentScene );
		SaveScene(scratch);
		gSave = false;
	}
	else if (gLoad)
	{
		switch ( gLoad )
		{
			case 1:
				if ( 1 )
				{
					char scratch[512];
					sprintf(scratch,"SceneSave%d.nxb", gCurrentScene ); // load the binary saved version.
					LoadScene(scratch, NXU::FT_BINARY);
				}
				break;
			case 2:
				LoadScene("test.xml", NXU::FT_XML);
				break;
			case 3:
				LoadScene("test.dae", NXU::FT_COLLADA);
				break;
			case 4:
				LoadScene("test.nxb", NXU::FT_BINARY);
				break;
		}
	}
    else if (gCreateCube)
    {
		CreateCube(NxVec3(0.0f, 20.0f, 0.0f), 1+(rand()&3)); 
		gCreateCube = false;
	}
    else if (gCreateStack10)
    {
		CreateStack(10); 
		gCreateStack10 = false;
	}
    else if (gCreateStack30)
    {
		CreateStack(30); 
		gCreateStack30 = false;
	}
    else if (gCreateTower30)
    {
		CreateTower(30);
		gCreateTower30 = false;
	}
    else if (gCreateCubeCam)
    {
		NxVec3 t = gCameraPos;
		NxVec3 Vel = gCameraForward;
		Vel.normalize();
		Vel *= 200.0f;
		CreateCube(t, 8, &Vel);
		gCreateCubeCam = false;
	}

	// ~Physics code

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
	    NxScene* scene = gScenes[gCurrentScene];
	    if (scene)  gDebugRenderer.renderData(*scene->getDebugRenderable());
	}
}

void RenderCallback()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ProcessCameraKeys();
	SetupCamera();

    for (NxU32 i=0;i<gMaxScenes;++i)
    {
	    if (gScenes[i] && !bPause)
	    {
		    GetPhysicsResults(i);
		}
	}

    ProcessInputs();

	// Update the time step
	gDeltaTime = UpdateTime();

    for (NxU32 i=0;i<gMaxScenes;++i)
    {
	    if (gScenes[i] && !bPause)
	    {
		    StartPhysics(i);
		}
	}

    // Display scene
 	RenderActors(bShadows);

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}

void ReshapeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
    gCameraAspectRatio = float(width)/float(height);
}

void IdleCallback()
{
    glutPostRedisplay();
}

void KeyboardCallback(unsigned char key, int x, int y)
{
	gKeys[key] = true;

	switch (key)
	{
		case ' ':			
		{
			gCreateCube = true;
		}
		break;

		case '8':			
		{
			gCreateStack10 = true;
		}
		break;

		case '9':			
		{
			gCreateStack30 = true;
		}
		break;

		case '0':			
		{
			gCreateTower30 = true;
		}
		break;

		case 'v':
		{
			gCreateCubeCam = true;
		}
		break;
        
		case '1':			
		{
            gSave = true;
		}
		break;

		case 'x':
			gLoad = 2;
		break;
		case 'y':
			gLoad = 3;
		break;
		case 'z':
			gLoad = 4;
		break;

		case '3':
		{
            gLoad = 1;
		}
		break;

        case 'c':
        {
            gClear = true;
        }
        break;

        case 't':
        {
            // Switch to the next scene
            ++gCurrentScene;
            if (gCurrentScene == gMaxScenes)
            {
                gCurrentScene = 0;
            }
			// Add scene number to HUD
			char ds[512];
			sprintf(ds, "Scene #: %d\n", gCurrentScene);
			hud.SetDisplayString(2, ds, 0.015f, 0.92f);
        }

		default: { break; }
	}
}

void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

	switch (key)
	{
		case 'p': { bPause = !bPause; 
					if (bPause)
						hud.SetDisplayString(1, "Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
					else
						hud.SetDisplayString(1, "", 0.0f, 0.0f);	
					UpdateTime(); 
					break; }
		case 'x': { bShadows = !bShadows; break; }
		case 'b': { bDebugWireframeMode = !bDebugWireframeMode; break; }		
		case 'f': { bForceMode = !bForceMode; break; }
		case 27 : { exit(0); break; }
		default : { break; }
	}
}

void SpecialCallback(int key, int x, int y)
{
	switch (key)
    {
		// Reset PhysX
		case GLUT_KEY_F10: ResetNx(); return; 
	}
}

void MouseCallback(int button, int state, int x, int y)
{
    mx = x;
    my = y;
}

void MotionCallback(int x, int y)
{
    int dx = mx - x;
    int dy = my - y;
    
    gCameraForward.normalize();
    gCameraRight.cross(gCameraForward,NxVec3(0,1,0));

    NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));
    qx.rotate(gCameraForward);
    NxQuat qy(NxPiF32 * dy * 20 / 180.0f, gCameraRight);
    qy.rotate(gCameraForward);

    mx = x;
    my = y;
}

void ExitCallback()
{
	ReleaseNx();
}

void InitGlut(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	gMainHandle = glutCreateWindow("Lesson 504: Scene Export");
    glutSetWindow(gMainHandle);
    glutDisplayFunc(RenderCallback);
    glutReshapeFunc(ReshapeCallback);
    glutIdleFunc(IdleCallback);
    glutKeyboardFunc(KeyboardCallback);
    glutKeyboardUpFunc(KeyboardUpCallback);
	glutSpecialFunc(SpecialCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MotionCallback);
	MotionCallback(0,0);
	atexit(ExitCallback);

    // Setup default render states
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);

    // Setup lighting
    glEnable(GL_LIGHTING);
    float AmbientColor[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
    float DiffuseColor[]    = { 0.2f, 0.2f, 0.2f, 0.0f };         glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
    float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };         glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);
    float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };  glLightfv(GL_LIGHT0, GL_POSITION, Position);
    glEnable(GL_LIGHT0);
}

NxActor* CreateGroundPlane()
{
	if (gScenes[gCurrentScene])
	{
        // Create a plane with default descriptor
        NxPlaneShapeDesc planeDesc;
        NxActorDesc actorDesc;
        actorDesc.shapes.pushBack(&planeDesc);
        return gScenes[gCurrentScene]->createActor(actorDesc);
	}

	return NULL;
}

//==================================================================================
void CreateCube(const NxVec3& pos, int size, const NxVec3* initial_velocity)
{
	if ( gScenes[gCurrentScene] )
	{
		// Create body
		NxBodyDesc bodyDesc;
		bodyDesc.angularDamping	= 0.5;
//	    bodyDesc.maxAngularVelocity	= 10;
		if ( initial_velocity )	
			bodyDesc.linearVelocity = *initial_velocity;

		NxBoxShapeDesc boxDesc;
		boxDesc.dimensions		= NxVec3(float(size), float(size), float(size));

		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&boxDesc);
		actorDesc.body			= &bodyDesc;
		actorDesc.density		= 10;
		actorDesc.globalPose.t  = pos;
//		gScenes[gCurrentScene]->createActor(actorDesc)->userData = (void*)size;
		gScenes[gCurrentScene]->createActor(actorDesc);
	}
}

//==================================================================================
void CreateStack(int size)
{
	float CubeSize = 2;
//	float Spacing = 0.01;
	float Spacing = 0.0001;
	NxVec3 Pos(0, CubeSize, 0);
	float Offset = -size * (CubeSize * 2 + Spacing) * 0.5;
	while(size)
	{
		for(int i=0;i<size;i++)
		{
			Pos.x = Offset + float(i) * (CubeSize * 2 + Spacing);
			CreateCube(Pos, CubeSize);
		}
		Offset += CubeSize;
		Pos.y += (CubeSize * 2 + Spacing);
		size--;
	}
}

//==================================================================================
void CreateTower(int size)
{
	float CubeSize = 2;
	float Spacing = 0.01;
	NxVec3 Pos(0, CubeSize, 0);
	while(size)
	{
		CreateCube(Pos, CubeSize);
		Pos.y += (CubeSize * 2 + Spacing);
		size--;
	}
}

void InitBlankScene()
{
	if (gPhysicsSDK)
	{
		// Create a blank scene
		NxSceneDesc sceneDesc;
		sceneDesc.gravity				= gDefaultGravity;
        sceneDesc.simType				= NX_SIMULATION_HW;
		gScenes[gCurrentScene] = gPhysicsSDK->createScene(sceneDesc);
		if(!gScenes[gCurrentScene])
		{
			sceneDesc.simType	   = NX_SIMULATION_SW;
			gScenes[gCurrentScene] = gPhysicsSDK->createScene(sceneDesc);
		}

		NxMaterial* defaultMaterial = gScenes[gCurrentScene]->getMaterialFromIndex(0); 
		defaultMaterial->setRestitution(0);
		defaultMaterial->setStaticFriction(0.5);
		defaultMaterial->setDynamicFriction(0.5);
			
	    // Create the objects in the scene
	    CreateGroundPlane();	
	}
}

void InitializeHUD()
{
	bHardwareScene = (gScenes[gCurrentScene]->getSimType() == NX_SIMULATION_HW);

	// Add hardware/software to HUD
	if (bHardwareScene)
	    hud.AddDisplayString("Hardware Scene", 0.74f, 0.92f);
	else
		hud.AddDisplayString("Software Scene", 0.74f, 0.92f);

	// Add pause to HUD
	if (bPause)  
		hud.AddDisplayString("Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
	else
		hud.AddDisplayString("", 0.0f, 0.0f);
}

void InitializeSpecialHUD()
{
	// Add scene number to HUD
	char ds[512];
	sprintf(ds, "Scene #: %d", gCurrentScene);
	hud.AddDisplayString(ds, 0.015f, 0.92f);
}

void InitNx()
{
    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);

    // Create the scenes
    for (NxU32 i=0;i<gMaxScenes;++i)
    {
        gCurrentScene = i;
	    InitBlankScene();
    }
    gCurrentScene = 0;

    // Set export directory
	int set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname1[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	// Get the current time
	UpdateTime();

	// Start the first frame of the simulation
    // Create the scenes
    for (NxU32 i=0;i<gMaxScenes;++i)
    {
        gCurrentScene = i;
	    StartPhysics(i);
	}
	gCurrentScene = 0;

	// Initialize HUD
	InitializeHUD();
	InitializeSpecialHUD();
}

void ReleaseNx()
{
	if (gPhysicsSDK)
	{
        for (NxU32 i = 0; i < gMaxScenes; ++i)
        {
            if (gScenes[i])
            {
				GetPhysicsResults(i);  // Make sure to fetchResults() before shutting down
			    gPhysicsSDK->releaseScene(*gScenes[i]);
            }
        }
		gPhysicsSDK->release();
	}
	if (gAllocator != NULL)
	{
		delete gAllocator;
		gAllocator=NULL;
	}
}

void ResetNx()
{
	ReleaseNx();
	InitNx();
}

void StartPhysics(NxU32 i)
{
	// Start collision and dynamics for delta time since the last frame
    gScenes[i]->setTiming(1.0/60.0, 1, NX_TIMESTEP_FIXED);
    gScenes[i]->simulate(1.0/60.0);
	gScenes[i]->flushStream();
}

void GetPhysicsResults(NxU32 i)
{
    // Get results from gScene->simulate(gDeltaTime)
	while (!gScenes[i]->fetchResults(NX_RIGID_BODY_FINISHED, false));
}

int main(int argc, char** argv)
{
	PrintControls();
    InitGlut(argc, argv);
    InitNx();
    glutMainLoop();
	ReleaseNx();
	return 0;
}
