#include <NxVersionNumber.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "CoreContainer.h"
#include "CoreDump.h"
#include "NxCooking.h"
#include "NxStream.h"

static char *strlwrPortable(char *s)
{
	if (!s) return NULL;
	while ( *s ) { 
		*s = (char) tolower( *s ); 
		++s;
	}
	return s;
}

NX_INLINE NxU16 coredump_flip(const NxU16* v)
{
	const NxU8* b = (const NxU8*)v;
	NxU16 f;
	NxU8* bf = (NxU8*)&f;
	bf[0] = b[1];
	bf[1] = b[0];
	return f;
}

NX_INLINE NxU32 coredump_flip(const NxU32* v)
{
	const NxU8* b = (const NxU8*)v;
	NxU32 f;
	NxU8* bf = (NxU8*)&f;
	bf[0] = b[3];
	bf[1] = b[2];
	bf[2] = b[1];
	bf[3] = b[0];
	return f;
}

NX_INLINE NxF32 coredump_flip(const NxF32* v)
{
	NxU32 d = coredump_flip((const NxU32*)v);
	return NX_FR(d);
}

NX_INLINE NxF64 coredump_flip(const NxF64* v)
{
	assert(0);

	//-- BZ_TODO: implement this

	return 0;
}

namespace CONTAINCORESTREAM
{

class UserStream : public NxStream
	{
	public:
								UserStream(const char* filename, bool load);
	virtual						~UserStream();

	virtual		NxU8			readByte()								const;
	virtual		NxU16			readWord()								const;
	virtual		NxU32			readDword()								const;
	virtual		float			readFloat()								const;
	virtual		double			readDouble()							const;
	virtual		void			readBuffer(void* buffer, NxU32 size)	const;

	virtual		NxStream&		storeByte(NxU8 b);
	virtual		NxStream&		storeWord(NxU16 w);
	virtual		NxStream&		storeDword(NxU32 d);
	virtual		NxStream&		storeFloat(NxReal f);
	virtual		NxStream&		storeDouble(NxF64 f);
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size);

				FILE*			fp;
	};

class MemoryWriteBuffer : public NxStream
	{
	public:
								MemoryWriteBuffer();
	virtual						~MemoryWriteBuffer();

	virtual		NxU8			readByte()								const	{ NX_ASSERT(0);	return 0;	}
	virtual		NxU16			readWord()								const	{ NX_ASSERT(0);	return 0;	}
	virtual		NxU32			readDword()								const	{ NX_ASSERT(0);	return 0;	}
	virtual		float			readFloat()								const	{ NX_ASSERT(0);	return 0.0f;}
	virtual		double			readDouble()							const	{ NX_ASSERT(0);	return 0.0;	}
	virtual		void			readBuffer(void* buffer, NxU32 size)	const	{ NX_ASSERT(0);				}

	virtual		NxStream&		storeByte(NxU8 b);
	virtual		NxStream&		storeWord(NxU16 w);
	virtual		NxStream&		storeDword(NxU32 d);
	virtual		NxStream&		storeFloat(NxReal f);
	virtual		NxStream&		storeDouble(NxF64 f);
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size);

				NxU32			currentSize;
				NxU32			maxSize;
				NxU8*			data;
	};

class MemoryReadBuffer : public NxStream
	{
	public:
								MemoryReadBuffer(const NxU8* data);
	virtual						~MemoryReadBuffer();

	virtual		NxU8			readByte()								const;
	virtual		NxU16			readWord()								const;
	virtual		NxU32			readDword()								const;
	virtual		float			readFloat()								const;
	virtual		double			readDouble()							const;
	virtual		void			readBuffer(void* buffer, NxU32 size)	const;

	virtual		NxStream&		storeByte(NxU8 b)							{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeWord(NxU16 w)							{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeDword(NxU32 d)							{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeFloat(NxReal f)						{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeDouble(NxF64 f)						{ NX_ASSERT(0);	return *this;	}
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size)	{ NX_ASSERT(0);	return *this;	}

	mutable		const NxU8*		buffer;
	};


UserStream::UserStream(const char* filename, bool load) : fp(NULL)
	{
	fp = fopen(filename, load ? "rb" : "wb");
	}

UserStream::~UserStream()
	{
	if(fp)	fclose(fp);
	}

// Loading API
NxU8 UserStream::readByte() const
	{
	NxU8 b;
	size_t r = fread(&b, sizeof(NxU8), 1, fp);
	NX_ASSERT(r);
	return b;
	}

NxU16 UserStream::readWord() const
	{
	NxU16 w;
	size_t r = fread(&w, sizeof(NxU16), 1, fp);
	NX_ASSERT(r);
	return w;
	}

NxU32 UserStream::readDword() const
	{
	NxU32 d;
	size_t r = fread(&d, sizeof(NxU32), 1, fp);
	NX_ASSERT(r);
	return d;
	}

float UserStream::readFloat() const
	{
	NxReal f;
	size_t r = fread(&f, sizeof(NxReal), 1, fp);
	NX_ASSERT(r);
	return f;
	}

double UserStream::readDouble() const
	{
	NxF64 f;
	size_t r = fread(&f, sizeof(NxF64), 1, fp);
	NX_ASSERT(r);
	return f;
	}

void UserStream::readBuffer(void* buffer, NxU32 size)	const
	{
	size_t w = fread(buffer, size, 1, fp);
	NX_ASSERT(w);
	}

// Saving API
NxStream& UserStream::storeByte(NxU8 b)
	{
	size_t w = fwrite(&b, sizeof(NxU8), 1, fp);
	NX_ASSERT(w);
	return *this;
	}

NxStream& UserStream::storeWord(NxU16 w)
	{
	size_t ww = fwrite(&w, sizeof(NxU16), 1, fp);
	NX_ASSERT(ww);
	return *this;
	}

NxStream& UserStream::storeDword(NxU32 d)
	{
	size_t w = fwrite(&d, sizeof(NxU32), 1, fp);
	NX_ASSERT(w);
	return *this;
	}

NxStream& UserStream::storeFloat(NxReal f)
	{
	size_t w = fwrite(&f, sizeof(NxReal), 1, fp);
	NX_ASSERT(w);
	return *this;
	}

NxStream& UserStream::storeDouble(NxF64 f)
	{
	size_t w = fwrite(&f, sizeof(NxF64), 1, fp);
	NX_ASSERT(w);
	return *this;
	}

NxStream& UserStream::storeBuffer(const void* buffer, NxU32 size)
	{
	size_t w = fwrite(buffer, size, 1, fp);
	NX_ASSERT(w);
	return *this;
	}




MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
	{
	}

MemoryWriteBuffer::~MemoryWriteBuffer()
	{
	free(data);
	}

NxStream& MemoryWriteBuffer::storeByte(NxU8 b)
	{
	storeBuffer(&b, sizeof(NxU8));
	return *this;
	}
NxStream& MemoryWriteBuffer::storeWord(NxU16 w)
	{
	storeBuffer(&w, sizeof(NxU16));
	return *this;
	}
NxStream& MemoryWriteBuffer::storeDword(NxU32 d)
	{
	storeBuffer(&d, sizeof(NxU32));
	return *this;
	}
NxStream& MemoryWriteBuffer::storeFloat(NxReal f)
	{
	storeBuffer(&f, sizeof(NxReal));
	return *this;
	}
NxStream& MemoryWriteBuffer::storeDouble(NxF64 f)
	{
	storeBuffer(&f, sizeof(NxF64));
	return *this;
	}
NxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, NxU32 size)
	{
	NxU32 expectedSize = currentSize + size;
	if(expectedSize > maxSize)
		{
		maxSize = expectedSize + 4096;

		NxU8* newData = (NxU8*)malloc(maxSize);
		if(data)
			{
			memcpy(newData, data, currentSize);
			free(data);
			}
		data = newData;
		}
	memcpy(data+currentSize, buffer, size);
	currentSize += size;
	return *this;
	}


MemoryReadBuffer::MemoryReadBuffer(const NxU8* data) : buffer(data)
	{
	}

MemoryReadBuffer::~MemoryReadBuffer()
	{
	// We don't own the data => no delete
	}

NxU8 MemoryReadBuffer::readByte() const
	{
	NxU8 b;
	memcpy(&b, buffer, sizeof(NxU8));
	buffer += sizeof(NxU8);
	return b;
	}

NxU16 MemoryReadBuffer::readWord() const
	{
	NxU16 w;
	memcpy(&w, buffer, sizeof(NxU16));
	buffer += sizeof(NxU16);
	return w;
	}

NxU32 MemoryReadBuffer::readDword() const
	{
	NxU32 d;
	memcpy(&d, buffer, sizeof(NxU32));
	buffer += sizeof(NxU32);
	return d;
	}

float MemoryReadBuffer::readFloat() const
	{
	float f;
	memcpy(&f, buffer, sizeof(float));
	buffer += sizeof(float);
	return f;
	}

double MemoryReadBuffer::readDouble() const
	{
	double f;
	memcpy(&f, buffer, sizeof(double));
	buffer += sizeof(double);
	return f;
	}

void MemoryReadBuffer::readBuffer(void* dest, NxU32 size) const
	{
	memcpy(dest, buffer, size);
	buffer += size;
	}


};

using namespace CONTAINCORESTREAM;

#if SMOKETEST
#include "NxTest.h"
using namespace NxTest;
#endif

// have a #define here which indicates where logging information goes
// in the event that something isn't right (for ASCII core dump reading), and simply
// output any "errors" it encounters to that file
static const char * sTrue				 = "true";
static const char * sOne				 = "1";
static const char * sFalse				 = "false";
static const char * sZero				 = "0";
static const char * sAsciiStatusFileName = "AsciiStatus.txt";

const int nNumHardSymbols		 = 8;
static const char * sHardSymbols = "()[],={}"; 

// static member variable
CharacterArray CoreContainer::mConsistentStrings;

// "main" parser keys
static const char * sPhysicsSDKVersion	= "physics";
static const char * sNumParams			= "num_params";
static const char * sNumTriangleMeshes	= "nbtrianglemeshes";
static const char * sNumConvexMeshes	= "nbconvexmeshes";
static const char * sNumCCDSkeletons	= "nbccdskeletons";
static const char * sNumHeightFields	= "nbheightfields";
static const char * sNumScenes			= "nbscenes";
static const char * sNumShapes			= "nbshapes";

// "sub" parser keys
static const char * sParamKey			= "nx_";
static const char * sTriangleMeshKey	= "trianglemesh";
static const char * sConvexMeshKey		= "convexmesh";
static const char * sCCDSkeletonMeshKey = "ccdskeleton";
static const char * sHeightFieldKey		= "heightfield";
static const char * sSceneKey			= "scene";

// TriangleMesh "sub" parser keys
static const char * sTriangleMesh_MaterialIndexStride		= "materialindexstride";
static const char * sTriangleMesh_MaterialIndices			= "materialindices";
static const char * sTriangleMesh_HeightFieldVerticalAxis	= "heightfieldverticalaxis";
static const char * sTriangleMesh_HeightFieldVerticalExtent = "heightfieldverticalextent";
static const char * sTriangleMesh_ConvexEdgeThreshold		= "convexedgethreshold";
static const char * sTriangleMesh_Flag_FlipNormals			= "nx_mf_flipnormals";
static const char * sTriangleMesh_Flag_16BitIndices			= "nx_mf_16_bit_indices";

// HeightField "sub" parser keys
static const char *sHeightField_Flag_BoundaryEdges			= "nx_hf_no_boundary_edges";
static const char *sHeightField_NumRows						= "heightfield_numrows";
static const char *sHeightField_NumColumns					= "heightfield_numcolumns";
static const char *sHeightField_Format						= "heightfield_format";
static const char *sHeightField_VerticalExtent				= "heightfield_verticalextent";
static const char *sHeightField_ConvexEdgeThreshold			= "heightfield_convexedgethreshold";
static const char *sHeightField_SampleStride				= "heightfield_samplestride";
static const char *sHeightField_Information					= "heightfield_information";
static const char *sHeightField_HeightFieldValue			= "heightfield_heightfieldvalue";
static const char *sHeightField_MaterialIndexOne			= "heightfield_materialindexone";
static const char *sHeightField_MaterialIndexTwo			= "heightfield_materialindextwo";
static const char *sHeightField_TessFlagOne					= "heightfield_tessflagone";
static const char *sHeightField_TessFlagTwo					= "heightfield_tessflagtwo";
// HeightFieldShape keys
//static const char *sHeightFieldShapeDesc					= "heightfieldshapedesc";
static const char *sHeightField_MeshFlags					= "heightfield_meshflags";
static const char *sHeightField_Scale						= "heightfield_scale";
static const char *sHeightField_HoleMaterial				= "heightfield_holematerial";
static const char *sHeightField_MaterialIndexHighBits		= "heightfield_materialindexhighbits";
static const char *sHeightField_ColumnScale					= "heightfield_columnscale";
static const char *sHeightField_RowScale					= "heightfield_rowscale";
static const char *sHeightField_HeightScale					= "heightfield_heightscale";
static const char *sHeightField_HeightFieldIndex			= "heightfield_heightfieldindex";

// common stuff
static const char * sNull					= "null";
static const char * sX						= "_x";
static const char * sY						= "_y";
static const char * sZ						= "_z";
static const char * sNumVertices			= "numvertices";
static const char * sPointStrideBytes		= "pointstridebytes";
static const char * sVertex					= "v";
static const char * sNumTriangles			= "numtriangles";
static const char * sTriangleStrideBytes	= "trianglestridebytes";
static const char * sTriangle				= "tri";
static const char * sPmap					= "pmap";
static const char * sNxMaterial				= "nxmaterial";
static const char * sActor					= "actor";
static const char * sNxActor				= "nxactor";
static const char * sName					= "name";
static const char * sDensity				= "density";
static const char * sGroup					= "group";
static const char * sMaterialIndex			= "materialindex";
static const char * sPlane					= "plane";
static const char * sMass					= "mass";
static const char * sType					= "type";
static const char * sMaxForce				= "maxforce";
static const char * sMaxTorque				= "maxtorque";
static const char * sValues					= "values";
static const char * sRestitution			= "restitution";
static const char * sSwing					= "swing";
static const char * sSpring					= "spring";
static const char * sDamping				= "damping";
static const char * sLow					= "low";
static const char * sHigh					= "high";
static const char * sBody					= "body";
static const char * sXMotion				= "xmotion";
static const char * sYMotion				= "ymotion";
static const char * sZMotion				= "zmotion";
static const char * sSwing1Motion			= "swing1motion";
static const char * sSwing2Motion			= "swing2motion";
static const char * sTwistMotion			= "twistmotion";
static const char * sXDrive					= "xdrive";
static const char * sYDrive					= "ydrive";
static const char * sZDrive					= "zdrive";
static const char * sSwingDrive				= "swingdrive";
static const char * sTwistDrive				= "twistdrive";
static const char * sSlerpDrive				= "slerpdrive";
static const char * sLinearVelocity			= "linearvelocity";
static const char * sAngularVelocity		= "angularvelocity";
static const char * sWakeUpCounter			= "wakeupcounter";
static const char * sLinearDamping			= "lineardamping";
static const char * sAngularDamping			= "angulardamping";
static const char * sMaxAngularVelocity		= "maxangularvelocity";
static const char * sScale					= "scale";
static const char * sRadius					= "radius";
static const char * sBoxDimensions			= "boxdimensions";
static const char * sHeight					= "height";
static const char * sFlags					= "flags";
static const char * sDesc					= "desc";
static const char * sMeshFlags				= "meshflags";
static const char * sTriangleMesh			= "trianglemesh";
static const char * sHeightField			= "heightfield";
static const char * sNxPairFlag				= "nxpairflag";
static const char * sPairFlag				= "pairflag";
static const char * sActorIndex				= "actorindex";
static const char * sShapeIndex				= "shapeindex";

// ConvexMesh "sub" parser keys
static const char * sConvexMesh_Flag_FlipNormals	= "nx_cf_flipnormals";
static const char * sConvexMesh_Flag_16BitIndices	= "nx_cf_16_bit_indices";
static const char * sConvexMesh_Flag_ComputeConvex	= "nx_cf_compute_convex";
static const char * sConvexMesh_Flag_InflateConvex	= "nx_cf_inflate_convex";

// CCDSkeleton "sub" parser keys
//static const char * sCCDSkeleton_ = "";

// Scene "sub" parser keys
static const char * sScene_SceneDesc	= "scenedesc";
static const char * sScene_NumMaterials = "nbmaterials";
static const char * sScene_NumActors	= "nbactors";
static const char * sScene_NumPairs		= "nbpairs";
static const char * sScene_NumJoints	= "nbjoints";
static const char * sScene_NumEffectors = "nbeffectors";

static const char * sSceneNum			= "scene";

// Scene Desc parser keys
static const char * sSceneDesc_CollisionDetection	= "collision_detection";
static const char * sSceneDesc_GroundPlane			= "groundplane";
static const char * sSceneDesc_BoundsPlanes			= "boundsplanes";
static const char * sSceneDesc_Gravity				= "gravity";
static const char * sSceneDesc_BroadPhase			= "broadphase";
static const char * sSceneDesc_TimeStepMethod		= "timestepmethod";
static const char * sSceneDesc_MaxTimestep			= "maxtimestep";
static const char * sSceneDesc_MaxIter				= "maxiter";
static const char * sSceneDesc_SimType				= "simtype";
static const char * sSceneDesc_HwSceneType			= "hwscenetype";
static const char * sSceneDesc_PipelineSpec			= "pipelinespec";
static const char * sSceneDesc_Limits				= "limits";
static const char * sSceneDesc_MaxBounds			= "maxbounds";
static const char * sSceneDesc_NxUserNotify			= "nxusernotify";
static const char * sSceneDesc_NxUserTriggerReport	= "nxusertriggerreport";
static const char * sSceneDesc_NxUserContactReport	= "nxusercontactreport";
static const char * sSceneDesc_NxUserData			= "userdata";

// material parser keys

static const char * sMaterial_DynamicFriction			= "dynamicfriction";
static const char * sMaterial_StaticFriction			= "staticfriction";
static const char * sMaterial_Restitution				= "restitution";
static const char * sMaterial_DynamicFrictionV			= "dynamicfrictionv";
static const char * sMaterial_StaticFrictionV			= "staticfrictionv";
static const char * sMaterial_FrictionCombineMode		= "frictioncombinemode";
static const char * sMaterial_RestitutionCombineMode	= "restitutioncombinemode";
static const char * sMaterial_DirOfAnisotropy			= "dirofanis";
static const char * sMaterial_Flag_Anisotropic			= "nx_mf_anisotropic";
static const char * sMaterial_Flag_SpringContact		= "nx_mf_spring_contact";

// actor parser keys
static const char * sActor_GlobalPose					= "globalpose";
static const char * sActor_Flag_DisableCollision		= "nx_af_disable_collision";
static const char * sActor_Flag_DisableResponse			= "nx_af_disable_response";
static const char * sActor_Flag_FluidDisableCollision	= "nx_af_fluid_disable_collision";
static const char * sActor_Flag_FluidActorReaction		= "nx_af_fluid_actor_rea";

// actor desc keys
static const char * sActorDesc_MassLocalPose			= "masslocalpose";
static const char * sActorDesc_MassSpaceInertia			= "massspaceinertia";
static const char * sActorDesc_CCDMotionThreshold		= "ccdmotionthreshold";
static const char * sActorDesc_SleepLinearVelocity		= "sleeplinearvelocity";
static const char * sActorDesc_SleepAngularVelocity		= "sleepangularvelocity";
static const char * sActorDesc_SolverIterationCount		= "solveriterationcount";
static const char * sActorDesc_Flag_DisableGravity		= "nx_bf_disable_gravity";
static const char * sActorDesc_Flag_FrozenPos			= "nx_bf_frozen_pos";
static const char * sActorDesc_Flag_FrozenRot			= "nx_bf_frozen_rot";
static const char * sActorDesc_Flag_Kinematic			= "nx_bf_kinematic";
static const char * sActorDesc_Flag_Visualization		= "nx_bf_visualization";

// shape parser keys
static const char * sShape_ShapePresent					= "shape";
// note we will bypass the ascii for NxShape=NX_SHAPE_xxx as it is just a printout for the actual type, which follows
static const char * sShape_LocalPose					= "localpose";
static const char * sShape_SkinWidth					= "skinwidth";
static const char * sShape_Flag_TriggerOnEnter			= "trigger_on_enter";
static const char * sShape_Flag_TriggerOnLeave			= "trigger_on_leave";
static const char * sShape_Flag_TriggerOnStay			= "trigger_on_stay";
static const char * sShape_Flag_TriggerEnable			= "trigger_enable";
static const char * sShape_Flag_Visualization			= "nx_sf_visualization";
static const char * sShape_Flag_DisableCollision		= "nx_sf_disable_collision";
static const char * sShape_Flag_FeatureIndices			= "nx_sf_feature_indices";
static const char * sShape_Flag_DisableRaycasting		= "nx_sf_disable_raycasting";
static const char * sShape_Flag_PointContactForce		= "nx_sf_point_contact_force";
static const char * sShape_Flag_DisableResponse			= "nx_sf_disable_response";
static const char * sShape_Flag_FluidDrain				= "nx_sf_fluid_drain";
static const char * sShape_Flag_FluidDrainInvert		= "nx_sf_fluid_drain_invert";
static const char * sShape_Flag_FluidDisableCollision	= "nx_sf_fluid_disable_collision";
static const char * sShape_Flag_ActorReaction			= "nx_sf_fluid_actor_reaction";
static const char * sShape_CCDSkeleton					= "ccdskeleton";
static const char * sShape_GroupsMask					= "groupsmask";
static const char * sShape_MeshData						= "meshdata";
static const char * sShape_NxShape						= "nxshape";

// joint parser keys
static const char * sJoint_NxJointDesc				= "nxjointdesc";
static const char * sJoint_NxD6JointDesc			= "nxd6jointdesc";
static const char * sJoint_Flag_CollisionEnabled	= "nx_jf_collision_enabled";
static const char * sJoint_Flag_Visualization		= "nx_jf_visualization";
static const char * sD6Joint						= "nx_joint_d6";
static const char * sPrismatic						= "nx_joint_prismatic";
static const char * sRevolute						= "nx_joint_revolute";
static const char * sCylindrical					= "nx_joint_cylindrical";
static const char * sSpherical						= "nx_joint_spherical";
static const char * sPointOnLine					= "nx_joint_point_on_line";
static const char * sPointInPlane					= "nx_joint_point_in_plane";
static const char * sDistanceJoint					= "nx_joint_distance";
static const char * sPulleyJoint					= "nx_joint_pulley";
static const char * sFixedJoint						= "nx_joint_fixed";

// d6 only joints info
static const char * sJoint_D6_Flag_SlerpDrive	= "nx_d6joint_slerp_drive";
static const char * sJoint_D6_Flag_GearEnabled	= "nx_d6joint_gear_enabled";

// any joint stuff
static const char * sLocalNormal				= "localnormal";
static const char * sLocalAxis					= "localaxis";
static const char * sLocalAnchor				= "localanchor";
static const char * sJointFlag_CollisionEnabled	= "nx_jf_collision_enabled";
static const char * sJointFlag_Visualization	= "nx_jf_visualization";
static const char * sNumLimitPlanes				= "numlimitplanes";
static const char * sPlaneLimitPointOnActor2	= "planelimitpointonactor2";
static const char * sPlaneLimitPoint			= "planelimitpoint";
static const char * sLimitPlane					= "limitplane";
static const char * sPlaneNormal				= "planenormal";
static const char * sPlaneD						= "planed";
static const char * sWorldLimitPt				= "worldlimitpt";
static const char * sTwistSpring_Spring			= "twistspring_spring";
static const char * sTwistSpring_Damper			= "twistspring_damper";
static const char * sTwistSpring_TargetValue	= "twistspring_targetvalue";
static const char * sSwingSpring_Spring			= "swingspring_spring";
static const char * sSwingSpring_Damper			= "swingspring_damper";
static const char * sSwingSpring_TargetValue	= "swingspring_targetvalue";
static const char * sJointSpring_Spring			= "jointspring_spring";
static const char * sJointSpring_Damper			= "jointspring_damper";
static const char * sJointSpring_TargetValue	= "jointspring_targetvalue";
static const char * sProjectionDist				= "projectiondist";
static const char * sLimit_Low_Hardness			= "limit_low_hardness";
static const char * sLimit_Low_Restitution		= "limit_low_restitution";
static const char * sLimit_Low_Value			= "limit_low_value";
static const char * sLimit_High_Hardness		= "limit_high_hardness";
static const char * sLimit_High_Restitution		= "limit_high_restitution";
static const char * sLimit_High_Value			= "limit_high_value";
static const char * sSwingLimit_Hardness		= "swinglimit_hardness";
static const char * sSwingLimit_Restitution		= "swinglimit_restitution";
static const char * sSwingLimit_Value			= "swinglimit_value";
static const char * sSwingAxis					= "swingaxis";
static const char * sProjectionMode				= "projectionmode";
static const char * sMotor_FreeSpin				= "motor_freespin";
static const char * sMotor_MaxForce				= "motor_maxforce";
static const char * sMotor_VelTarget			= "motor_veltarget";
static const char * sSpring_Spring				= "spring_spring";
static const char * sSpring_Damper				= "spring_damper";
static const char * sSpring_TargetValue			= "spring_targetvalue";
static const char * sProjectionDistance			= "projectiondistance";
static const char * sProjectionAngle			= "projectionangle";
static const char * sMinDistance				= "mindistance";
static const char * sMaxDistance				= "maxdistance";
static const char * sDistance					= "distance";
static const char * sStiffness					= "stiffness";
static const char * sRatio						= "ratio";
static const char * sDescPulley					= "desc->pulley";
static const char * sLinearLimit				= "linearlimit";
static const char * sSwing1Limit				= "swing1limit";
static const char * sSwing2Limit				= "swing2limit";
static const char * sTwistLimit					= "twistlimit";
static const char * sDrivePosition				= "driveposition";
static const char * sDriveOrientation			= "driveorientation";
static const char * sDriveLinearVelocity		= "drivelinearvelocity";
static const char * sDriveAngularVelocity		= "driveangularvelocity";
static const char * sGearRatio					= "gearratio";
static const char * sD6_Flag_SlerpDrive			= "nx_d6joint_slerp_drive";
static const char * sD6_Flag_GearEnabled		= "nx_d6joint_gear_enabled";
static const char * sForceLimit					= "forcelimit";
static const char * sNxD6Joint_Flag_Drive_Position = "nx_d6joint_drive_position";
static const char * sNxD6Joint_Flag_Drive_Velocity = "nx_d6joint_drive_velocity";

// spring and damper strings
static const char * sNxEffector				= "nxeffector";
static const char * sSpringAndDamperEffector= "isspringanddampereffector";
static const char * sJointConnectionPos		= "jointconnectionpos";
static const char * sLinearSpringInfo		= "linearspringinfo";
static const char * sLinearDamperInfo		= "lineardamperinfo";
static const char * sDistCompressSaturate	= "distcomp";
static const char * sDistRelaxed			= "distrelaxed";
static const char * sDistStretchSaturate	= "diststretchsaturate";
static const char * sMaxCompressForce		= "maxcompressforce";
static const char * sMaxStretchForce		= "maxstretchforce";
static const char * sVelCompressSaturate	= "velcompresssaturate";
static const char * sVelStretchSaturate		= "velstretchsaturate";

// NX PARAMETER STRINGS
static const char * sNxParam_BoxNoiseLevel				= "NX_BBOX_NOISE_LEVEL";
static const char * sNxParam_SolverConvergenceThreshold = "NX_SOLVER_CONVERGENCE_THRESHOLD";
static const char * sNxParam_VisualizeFluidDrains		= "NX_VISUALIZE_FLUID_DRAINS";
static const char * sNxParam_VisualizeFluidPackets		= "NX_VISUALIZE_FLUID_PACKETS";
static const char * sNxParam_VisualizeFluidMotionLimit	= "NX_VISUALIZE_FLUID_MOTION_LIMIT";
static const char * sNxParam_VisualizeFluidDynCollision = "NX_VISUALIZE_FLUID_DYN_COLLISION";
static const char * sNxParam_CCDEpsilon					= "NX_CCD_EPSILON";
static const char * sNxParam_PenaltyForce				= "NX_PENALTY_FORCE";
static const char * sNxParam_SkinWidth					= "NX_SKIN_WIDTH";
static const char * sNxParam_DefaultSleepLinVelSquared	= "NX_DEFAULT_SLEEP_LIN_VEL_SQUARED";
static const char * sNxParam_DefaultSleepAngVelSquared	= "NX_DEFAULT_SLEEP_ANG_VEL_SQUARED";
static const char * sNxParam_BounceThreshold			= "NX_BOUNCE_THRESHOLD";
static const char * sNxParam_DynFrictScaling			= "NX_DYN_FRICT_SCALING";
static const char * sNxParam_StaFrictScaling			= "NX_STA_FRICT_SCALING";
static const char * sNxParam_MaxAngularVelocity			= "NX_MAX_ANGULAR_VELOCITY";
static const char * sNxParam_ContinuousCD				= "NX_CONTINUOUS_CD";
static const char * sNxParam_VisualizationScale			= "NX_VISUALIZATION_SCALE";
static const char * sNxParam_VisualizeWorldAxes			= "NX_VISUALIZE_WORLD_AXES";
static const char * sNxParam_VisualizeBodyAxes			= "NX_VISUALIZE_BODY_AXES";
static const char * sNxParam_VisualizeBodyMassAxes		= "NX_VISUALIZE_BODY_MASS_AXES";
static const char * sNxParam_VisualizeBodyLinVelocity	= "NX_VISUALIZE_BODY_LIN_VELOCITY";
static const char * sNxParam_VisualizeBodyAngVelocity	= "NX_VISUALIZE_BODY_ANG_VELOCITY";
static const char * sNxParam_VisualizeBodyLinMomentum	= "NX_VISUALIZE_BODY_LIN_MOMENTUM";
static const char * sNxParam_VisualizeBodyAngMomentum	= "NX_VISUALIZE_BODY_ANG_MOMENTUM";
static const char * sNxParam_VisualizeBodyLinAccel		= "NX_VISUALIZE_BODY_LIN_ACCEL";
static const char * sNxParam_VisualizeBodyAngAccel		= "NX_VISUALIZE_BODY_ANG_ACCEL";
static const char * sNxParam_VisualizeBodyLinForce		= "NX_VISUALIZE_BODY_LIN_FORCE";
static const char * sNxParam_VisualizeBodyAngForce		= "NX_VISUALIZE_BODY_ANG_FORCE";
static const char * sNxParam_VisualizeBodyReduced		= "NX_VISUALIZE_BODY_REDUCED";
static const char * sNxParam_VisualizeBodyJointGroups	= "NX_VISUALIZE_BODY_JOINT_GROUPS";
static const char * sNxParam_VisualizeBodyContactList	= "NX_VISUALIZE_BODY_CONTACT_LIST";
static const char * sNxParam_VisualizeBodyJointList		= "NX_VISUALIZE_BODY_JOINT_LIST";
static const char * sNxParam_VisualizeBodyDamping		= "NX_VISUALIZE_BODY_DAMPING";
static const char * sNxParam_VisualizeBodySleep			= "NX_VISUALIZE_BODY_SLEEP";
static const char * sNxParam_VisualizeJointLocalAxes	= "NX_VISUALIZE_JOINT_LOCAL_AXES";
static const char * sNxParam_VisualizeJointWorldAxes	= "NX_VISUALIZE_JOINT_WORLD_AXES";
static const char * sNxParam_VisualizeJointLimits		= "NX_VISUALIZE_JOINT_LIMITS";
static const char * sNxParam_VisualizeJointError		= "NX_VISUALIZE_JOINT_ERROR";
static const char * sNxParam_VisualizeJointForce		= "NX_VISUALIZE_JOINT_FORCE";
static const char * sNxParam_VisualizeJointReduced		= "NX_VISUALIZE_JOINT_REDUCED";
static const char * sNxParam_VisualizeContactPoint		= "NX_VISUALIZE_CONTACT_POINT";
static const char * sNxParam_VisualizeContactNormal		= "NX_VISUALIZE_CONTACT_NORMAL";
static const char * sNxParam_VisualizeContactError		= "NX_VISUALIZE_CONTACT_ERROR";
static const char * sNxParam_VisualizeContactForce		= "NX_VISUALIZE_CONTACT_FORCE";
static const char * sNxParamVisualizeActorAxes			= "NX_VISUALIZE_ACTOR_AXES";
static const char * sNxParam_VisualizeCollisionAABBs	= "NX_VISUALIZE_COLLISION_AABBS";
static const char * sNxParam_VisualizeCollisionShapes	= "NX_VISUALIZE_COLLISION_SHAPES";
static const char * sNxParam_VisualizeCollisionAxes		= "NX_VISUALIZE_COLLISION_AXES";
static const char * sNxParam_VisualizeCollisionCompounds = "NX_VISUALIZE_COLLISION_COMPOUNDS";
static const char * sNxParam_VisualizeCollisionVNormals = "NX_VISUALIZE_COLLISION_VNORMALS";
static const char * sNxParam_VisualizeCollisionFNormals = "NX_VISUALIZE_COLLISION_FNORMALS";
static const char * sNxParam_VisualizeCollisioEdges		= "NX_VISUALIZE_COLLISION_EDGES";
static const char * sNxParam_VisualizeCollisionSpheres	= "NX_VISUALIZE_COLLISION_SPHERES";
static const char * sNxParam_VisualizeCollisionSAP		= "NX_VISUALIZE_COLLISION_SAP";
static const char * sNxParam_VisualizeCollisionStatic	= "NX_VISUALIZE_COLLISION_STATIC";
static const char * sNxParam_VisualizeCollisionDynamic	= "NX_VISUALIZE_COLLISION_DYNAMIC";
static const char * sNxParam_VisualizeCollisionFree		= "NX_VISUALIZE_COLLISION_FREE";
static const char * sNxParam_VisualizeCollisionCCD		= "NX_VISUALIZE_COLLISION_CCD";
static const char * sNxParam_VisualizeCollisionSkeletons = "NX_VISUALIZE_COLLISION_SKELETONS";
static const char * sNxParam_VisualizeFluidEmitters		= "NX_VISUALIZE_FLUID_EMITTERS";
static const char * sNxParam_VisualizeFluidPosition		= "NX_VISUALIZE_FLUID_POSITION";
static const char * sNxParam_VisualizeFluidVelocity		= "NX_VISUALIZE_FLUID_VELOCITY";
static const char * sNxParam_VisualizeFluidKernelRadius = "NX_VISUALIZE_FLUID_KERNEL_RADIUS";
static const char * sNxParam_VisualizeFluidBounds		= "NX_VISUALIZE_FLUID_BOUNDS";
static const char * sNxParam_AdaptiveForce				= "NX_ADAPTIVE_FORCE";
static const char * sNxParam_CollVetoJoined				= "NX_COLL_VETO_JOINTED";
static const char * sNxParam_TriggerTriggerCallback		= "NX_TRIGGER_TRIGGER_CALLBACK";
static const char * sNxParam_SelectHWAlgo				= "NX_SELECT_HW_ALGO";
static const char * sNxParam_VisualizeActiveVertices	= "NX_VISUALIZE_ACTIVE_VERTICES";
static const char * sNxParam_ParamsNumValues			= "NX_PARAMS_NUM_VALUES";
static const char * sNxParam_MinSeparationForPenalty	= "NX_MIN_SEPARATION_FOR_PENALTY";

static const char * sNxParam_VisualizeClothCollisions	= "NX_VISUALIZE_CLOTH_COLLISIONS";
static const char * sNxParam_VisualizeClothSelfCollisions = "NX_VISUALIZE_CLOTH_SELFCOLLISIONS";
static const char * sNxParam_VisualizeClothWorkPackets	= "NX_VISUALIZE_CLOTH_WORKPACKETS";

// wheel values
static const char * sWheelParam_SuspensionTravel			= "suspensiontravel";
static const char * sWheelParam_MotorTorque					= "motortorque";
static const char * sWheelParam_BrakeTorque					= "braketorque";
static const char * sWheelParam_SteerAngle					= "steerangle";
static const char * sWheelParam_InverseWheelMass			= "inversewheelmass";
static const char * sWheelParam_WheelFlags					= "wheelflags";
static const char * sWheelParam_LateralFunc					= "lateraltireforcefunc";
static const char * sWheelParam_LongitudinalFunc			= "longitudinaltireforcefunc";
static const char * sWheelParam_TireFunc_ExtremumSlip		= "extremumslip";
static const char * sWheelParam_TireFunc_ExtremumValue		= "extremumvalue";
static const char * sWheelParam_TireFunc_AsumptoteSlip		= "asymptoteslip";
static const char * sWheelParam_TireFunc_AsymptoteValue		= "asymptotevalue";
static const char * sWheelParam_TireFunc_StiffnessFactor	= "stiffnessfactor";

// extra flag items I needed to add
static const char * sSweptShapeFlag						= "nx_swept_shape";
static const char * sMeshSmoothSphereCollisionsFlag		= "nx_mesh_smooth_sphere_collisions";
static const char * sWfWheelAxisContactNormalFlag		= "nx_wf_wheel_axis_contact_normal";
static const char * sWfInputLatSlipVelocityFlag			= "nx_wf_input_lat_slipvelocity";
static const char * sWfInputLngSlipVelocityFlag			= "nx_wf_input_lng_slipvelocity";
static const char * sWfUnscaledSpringBehaviorFlag		= "nx_wf_unscaled_spring_behavior";
static const char * sWfAxleSpeedOverrideFlag			= "nx_wf_axle_speed_override";
static const char * sPairIsActorPairFlag				= "pair_flag_is_actor_pair";
static const char * sPairIgnorePairFlag					= "pair_flag_nx_ignore_pair";
static const char * sPairNotifyOnTouchFlag				= "pair_flag_nx_notify_on_touch";
static const char * sSjfTwistLimitEnabledFlag			= "nx_sjf_twist_limit_enabled";
static const char * sSjfSwingLimitEnabledFlag			= "nx_sjf_swing_limit_enabled";
static const char * sSjfTwistSpringEnabledFlag			= "nx_sjf_twist_spring_enabled";
static const char * sSjfSwingSpringEnabledFlag			= "nx_sjf_swing_spring_enabled";
static const char * sSjfSpringEnabledFlag				= "nx_sjf_joint_spring_enabled";
static const char * sRjfLimitEnabledFlag				= "nx_rjf_limit_enabled";
static const char * sRjfMotorEnabledFlag				= "nx_rjf_motor_enabled";
static const char * sRjfSpringEnabledFlag				= "nx_rjf_spring_enabled";
static const char * sDjfMaxDistanceEnabledFlag			= "nx_djf_max_distance_enabled";
static const char * sDjfMinDistanceEnabledFlag			= "nx_djf_min_distance_enabled";
static const char * sDjfSpringEnabledFlag				= "nx_djf_spring_enabled";
static const char * sPfjIsRigidFlag						= "nx_pfj_is_rigid";
static const char * sPfjMotorEnabledFlag				= "nx_pfj_motor_enabled";

static const char * sPrismaticJointDesc		= "nxprismaticjointdesc";
static const char * sRevoluteJointDesc		= "nxrevolutejointdesc";
static const char * sCylindricalJointDesc	= "nxcylindricaljointdesc";
static const char * sSphericalJointDesc		= "nxsphericaljointdesc";
static const char * sPointOnLineJointDesc	= "nxpointonlinejointdesc";
static const char * sPointInPlaneJointDesc	= "nxpointinplanejointdesc";
static const char * sDistanceJointDesc		= "nxdistancejointdesc";
static const char * sPulleyJointDesc		= "nxpulleyjointdesc";
static const char * sFixedJointDesc			= "nxfixedjointdesc";
static const char * sNxD6JointDesc			= "nxd6jointdesc";
static const char * sNxJoint				= "nxjoint";

static const char * sWfSteerableInputFlag		= "nx_wf_steerable_input";
static const char * sWfSteerableAutoFlag		= "nx_wf_steerable_auto";
static const char * sWfAffectedByHandbrakeFlag	= "nx_wf_affected_by_handbrake";
static const char * sWfAcceleratedFlag			= "nx_wf_accelerated";
static const char * sWfBuildLowerHalfFlag		= "nx_wf_build_lower_half";
static const char * sWfUseWheelshapeFlag		= "nx_wf_use_wheelshape";

static const char * sCommand_GiveActorsNames	= "NX_GiveActorsNames";

//==================================================================================
// creation of a namespace for this file's in-place parser
//==================================================================================
namespace PARSER
{
	#if defined(__APPLE__) || defined(__CELLOS_LV2__)
	#define stricmp(a, b) strcasecmp((a), (b))
	#endif

	//==================================================================================
	//==================================================================================
	class InPlaceParserInterface
	{
	public:
		virtual int ParseLine(int lineno,int argc,const char **argv) =0;  // return TRUE to continue parsing, return FALSE to abort parsing process
	};

	enum SeparatorType
	{
		ST_DATA,        // is data
		ST_HARD,        // is a hard separator
		ST_SOFT,        // is a soft separator
		ST_EOS          // is a comment symbol, and everything past this character should be ignored
	};

	//==================================================================================
	//==================================================================================
	class InPlaceParser
	{
	public:
		InPlaceParser(void)
		{
			Init();
		}

		~InPlaceParser(void);

		void Init(void)
		{
			mQuoteChar	= 34;
			mData		= 0;
			mLen		= 0;
			mMyAlloc	= false;
			for (int i=0; i<256; i++)
			{
				mHard[i]			= ST_DATA;
				mHardString[i*2]	= i;
				mHardString[i*2+1]	= 0;
			}
			mHard[0]  = ST_EOS;
			mHard[32] = ST_SOFT;
			mHard[9]  = ST_SOFT;
			mHard[13] = ST_SOFT;
			mHard[10] = ST_SOFT;
		}

		bool SetFile(const char *fname); // use this file as source data to parse.

		void SetSourceData(char *data,int len)
		{
			mData		= data;
			mLen		= len;
			mMyAlloc	= false;
		};

		int  Parse(InPlaceParserInterface *callback); // returns true if entire file was parsed, false if it aborted for some reason

		int ProcessLine(int lineno,char *line,InPlaceParserInterface *callback);

		const char ** GetArglist(char *source,int &count); // convert source string into an arg list, this is a destructive parse.

		void SetHardSeparator(char c) // add a hard separator
		{
			mHard[c] = ST_HARD;
		}

		void SetHard(char c) // add a hard separator
		{
			mHard[c] = ST_HARD;
		}

		void SetCommentSymbol(char c) // comment character, treated as 'end of string'
		{
			mHard[c] = ST_EOS;
		}

		void ClearHardSeparator(char c)
		{
			mHard[c] = ST_DATA;
		}

		void DefaultSymbols(void); // set up default symbols for hard seperator and comment symbol of the '#' character.

		bool EOS(char c)
		{
			bool ret = false;
			if ( mHard[c] == ST_EOS )
			{
				ret = true;
			}
			return ret;
		}

		void SetQuoteChar(char c)
		{
			mQuoteChar = c;
		}

	private:


		inline char *	AddHard(int &argc,const char **argv,char *foo);
		inline bool		IsHard(char c);
		inline char *	SkipSpaces(char *foo);
		inline bool		IsWhiteSpace(char c);
		inline bool		IsNonSeparator(char c); // non seperator,neither hard nor soft

		bool			mMyAlloc; // whether or not *I* allocated the buffer and am responsible for deleting it.
		char *			mData;  // ascii data to parse.
		int				mLen;   // length of data
		SeparatorType	mHard[256];
		char			mHardString[256*2];
		char			mQuoteChar;
	};

	//==================================================================================
	bool InPlaceParser::SetFile(const char *fname)
	{
		bool foundFile = false;

		if ( mMyAlloc )
		{
			free(mData);
		}
		mData		= 0;
		mLen		= 0;
		mMyAlloc	= false;

		FILE *fph = fopen(fname,"rb");
		if ( fph )
		{
			fseek(fph,0L,SEEK_END);
			mLen = ftell(fph);
			fseek(fph,0L,SEEK_SET);
			if ( mLen )
			{
				mData = (char *) malloc(sizeof(char)*(mLen+1));
				int ok = fread(mData, mLen, 1, fph);
				if ( !ok )
				{
					free(mData);
					mData = 0;
				}
				else
				{
					mData[mLen] = 0; // zero byte terminate end of file marker.
					mMyAlloc	= true;
				}
			}
			fclose(fph);
			foundFile = true;
		}

		return foundFile;
	}

	//==================================================================================
	InPlaceParser::~InPlaceParser(void)
	{
		if ( mMyAlloc )
		{
			free(mData);
		}
	}

	#define MAXARGS 512
	//==================================================================================
	bool InPlaceParser::IsHard(char c)
	{
		return mHard[c] == ST_HARD;
	}

	//==================================================================================
	char * InPlaceParser::AddHard(int &argc,const char **argv,char *foo)
	{
		while ( IsHard(*foo) )
		{
			const char *hard = &mHardString[*foo*2];
			if ( argc < MAXARGS )
			{
				argv[argc++] = hard;
			}
			++foo;
		}
		return foo;
	}

	//==================================================================================
	bool   InPlaceParser::IsWhiteSpace(char c)
	{
		return mHard[c] == ST_SOFT;
	}

	//==================================================================================
	char * InPlaceParser::SkipSpaces(char *foo)
	{
		while ( !EOS(*foo) && IsWhiteSpace(*foo) ) foo++;
		return foo;
	}

	//==================================================================================
	bool InPlaceParser::IsNonSeparator(char c)
	{
		if ( !IsHard(c) && !IsWhiteSpace(c) && c != 0 ) return true;
		return false;
	}

	//==================================================================================
	int InPlaceParser::ProcessLine(int lineno,char *line,InPlaceParserInterface *callback)
	{
		int ret = 0;

		const char *argv[MAXARGS];
		int argc = 0;

		char *foo = line;

		while ( !EOS(*foo) && argc < MAXARGS )
		{
			foo = SkipSpaces(foo); // skip any leading spaces

			if ( EOS(*foo) ) break;

			if ( *foo == mQuoteChar ) // if it is an open quote
			{
				++foo;
				if ( argc < MAXARGS )
				{
					argv[argc++] = foo;
				}
				while ( !EOS(*foo) && ( *foo != mQuoteChar ) )
					++foo;

				if ( !EOS(*foo) )
				{
					*foo = 0; // replace close quote with zero byte EOS
					++foo;
				}
			}
			else
			{
				foo = AddHard(argc,argv,foo); // add any hard separators, skip any spaces

				if ( IsNonSeparator(*foo) )  // add non-hard argument.
				{
					bool quote  = false;
					if ( *foo == mQuoteChar )
					{
						++foo;
						quote = true;
					}

					if ( argc < MAXARGS )
					{
						argv[argc++] = foo;
					}

					if ( quote )
					{
						while (*foo && *foo != mQuoteChar ) 
							++foo;
						if ( *foo ) 
							*foo = 32;
					}

					// continue..until we hit an eos ..
					while ( !EOS(*foo) ) // until we hit EOS
					{
						if ( IsWhiteSpace(*foo) ) // if we hit a space, stomp a zero byte, and exit
						{
							*foo = 0;
							++foo;
							break;
						}
						else if ( IsHard(*foo) ) // if we hit a hard separator, stomp a zero byte and store the hard separator argument
						{
							const char *hard = &mHardString[*foo*2];
							*foo = 0;
							if ( argc < MAXARGS )
							{
								argv[argc++] = hard;
							}
							++foo;
							break;
						}
						++foo;
					} // end of while loop...
				}
			}
		}

		if ( argc )
		{
			ret = callback->ParseLine(lineno, argc, argv );
		}

		return ret;
	}

	//==================================================================================
	// returns true if entire file was parsed, false if it aborted for some reason
	//==================================================================================
	int  InPlaceParser::Parse(InPlaceParserInterface *callback)
	{
		assert( callback );
		if ( !mData ) return 0;

		int ret		= 0;
		int lineno	= 0;
		char *foo   = mData;
		char *begin = foo;

		while ( *foo )
		{
			if ( *foo == 10 || *foo == 13 )
			{
				++lineno;
				*foo = 0;

				if ( *begin ) // if there is any data to parse at all...
				{
					int v = ProcessLine(lineno,begin,callback);
					if ( v ) ret = v;
				}

				++foo;
				if ( *foo == 10 ) 
					++foo; // skip line feed, if it is in the carraige-return line-feed format...
				begin = foo;
			}
			else
			{
				++foo;
			}
		}

		lineno++; // last line.

		int v = ProcessLine(lineno,begin,callback);
		if ( v ) 
			ret = v;
		return ret;
	}

	//==================================================================================
	void InPlaceParser::DefaultSymbols(void)
	{
		SetHardSeparator(',');
		SetHardSeparator('(');
		SetHardSeparator(')');
		SetHardSeparator('=');
		SetHardSeparator('[');
		SetHardSeparator(']');
		SetHardSeparator('{');
		SetHardSeparator('}');
		SetCommentSymbol('#');
	}

	//==================================================================================
	// convert source string into an arg list, this is a destructive parse.
	//==================================================================================
	const char ** InPlaceParser::GetArglist(char *line,int &count)
	{
		const char **ret = 0;

		static const char *argv[MAXARGS];
		int argc = 0;

		char *foo = line;

		while ( !EOS(*foo) && argc < MAXARGS )
		{
			foo = SkipSpaces(foo); // skip any leading spaces

			if ( EOS(*foo) ) break;

			if ( *foo == mQuoteChar ) // if it is an open quote
			{
				++foo;
				if ( argc < MAXARGS )
				{
					argv[argc++] = foo;
				}
				while ( !EOS(*foo) && *foo != mQuoteChar ) foo++;
				if ( !EOS(*foo) )
				{
					*foo = 0; // replace close quote with zero byte EOS
					++foo;
				}
			}
			else
			{

				foo = AddHard(argc,argv,foo); // add any hard separators, skip any spaces

				if ( IsNonSeparator(*foo) )  // add non-hard argument.
				{
					bool quote  = false;
					if ( *foo == mQuoteChar )
					{
						++foo;
						quote = true;
					}

					if ( argc < MAXARGS )
					{
						argv[argc++] = foo;
					}

					if ( quote )
					{
						while (*foo && *foo != mQuoteChar ) 
							++foo;
						if ( *foo ) 
							*foo = 32;
					}

					// continue..until we hit an eos ..
					while ( !EOS(*foo) ) // until we hit EOS
					{
						if ( IsWhiteSpace(*foo) ) // if we hit a space, stomp a zero byte, and exit
						{
							*foo = 0;
							++foo;
							break;
						}
						else if ( IsHard(*foo) ) // if we hit a hard separator, stomp a zero byte and store the hard separator argument
						{
							const char *hard = &mHardString[*foo*2];
							*foo = 0;
							if ( argc < MAXARGS )
							{
								argv[argc++] = hard;
							}
							++foo;
							break;
						}
						foo++;
					} // end of while loop...
				}
			}
		}

		count = argc;
		if ( argc )
		{
			ret = argv;
		}

		return ret;
	}
}; // end namespace PARSER

//==================================================================================
//==================================================================================
class AsciiCoreDump : public PARSER::InPlaceParserInterface
{
public:

	enum ParseType
	{
		// "main" parse strings
		PT_SDK_VERSION,
		PT_NUM_PARAMS,
		PT_NUM_TRIANGLE_MESHES,
		PT_NUM_CONVEX_MESHES,
		PT_NUM_CCD_SKELETONS,
		PT_NUM_HEIGHT_FIELDS,
		PT_NUM_SCENES,
		PT_NUM_MAIN_HEADINGS,

		// "sub-main" parse strings
		PT_PARAM_VALUE			= PT_NUM_MAIN_HEADINGS,
		PT_TRIANGLE_MESH_INFO,
		PT_CONVEX_MESH_INFO,
		PT_CCD_SKELETON_INFO,
		PT_HEIGHT_FIELD_INFO,
		PT_SCENE_INFO,

		PT_LAST,
	};

	enum SceneParseType
	{
		SPT_SCENE_DESC,
		SPT_MATERIALS,
		SPT_ACTORS,
		SPT_PAIR_FLAGS,
		SPT_JOINTS,
		SPT_EFFECTORS,
		SPT_LAST,
	};

	enum ActorParseType
	{
		APT_ACTOR_PROPS,
		APT_BODY_INFO,
		APT_SHAPE_INFO,

		APT_LAST,
	};

	enum D6JointParseType
	{
		D6JPT_LINEAR,
		D6JPT_SWING1,
		D6JPT_SWING2,
		D6JPT_TWIST_LOW,
		D6JPT_TWIST_HIGH,
		D6JPT_XDRIVE,
		D6JPT_YDRIVE,
		D6JPT_ZDRIVE,
		D6JPT_SWINGDRIVE,
		D6JPT_TWISTDRIVE,
		D6JPT_SLERPDRIVE,

		D6JPT_LAST,
	};

	AsciiCoreDump( NxPhysicsSDK *sdk, CoreContainer *cc )
	{
		mSDK				= sdk;
  		mContainer			= cc;
		mFp					= 0;
		mParamNum			= 0;
		mNumTriangleMeshes	= 0;
		mNumConvexMeshes	= 0;
		mNumSkeletons		= 0;
		mNumScenes			= 0;
		mNumHeightFields	= 0;
		mTriangleMeshData	= 0;
		mConvexMeshData		= 0;
		mHeightFieldData	= 0;
		mCurrTriMesh		= 0;
		mCurrConvexMesh		= 0;
		mCurrSkeleton		= 0;
		mCurrHeightField	= 0;
		mCurrScene			= 0;
		mCoreScene			= 0;
		mCurrMat			= 0;
		mCurrActor			= 0;
		mBodyDesc			= 0;
		mCurrShape			= 0;
		mWhichPairFlag		= 0;
		mCurrSpringDamper	= 0;
		mCurrJoint			= 0;
		mVoidTriPntr		= 0;
		mVoidVrtPntr		= 0;
		memset( mParsedMainHeading, 0, sizeof(bool)*PT_NUM_MAIN_HEADINGS );
	}

	
	AsciiCoreDump::~AsciiCoreDump()
	{
		if(mFp)
		{
			fclose(mFp);
			mFp = 0;
		}
	}
	bool Load( const char *fname )
	{
		bool ret = false;

		if ( mContainer )
		{
			PARSER::InPlaceParser ipp;
			if ( ipp.SetFile( fname ) )
			{
				ret = true;

				// add the appropriate hard symbols
				for ( int i = 0; i < nNumHardSymbols; ++i )
				{
					ipp.SetHard( sHardSymbols[i] );
				}
			
				ipp.SetCommentSymbol('#');
				ipp.Parse( this );
			}
		}

		return ret;
	}

	void OutputStatusMsg( const char *msg )
	{
		if ( !mFp )
		{
			mFp = fopen( sAsciiStatusFileName, "w+" );
		}
		if ( mFp && msg )
		{
			fprintf( mFp, "%s", msg );
		}
	}

	const char *GetFirstString( int offset, int argc, const char **argv )
	{
		const char *ret = 0;
		bool done = false;
		for ( int i = offset; !done && (i < argc); ++i )
		{
			if ( !IsHardSymbol( argv[i] ) )
			{
				ret  = CoreContainer::GetStringPntr( argv[i] );
				done = true;
			}
		}				
		return ret;
	}

	bool GetIndices( int num, int *indices, int offset, int argc, const char **argv )
	{
		bool gotEmAll = false;
		if ( indices )
		{
			int which = 0;
			for ( int i = offset; !gotEmAll && (i < argc); ++i )
			{
				if ( HasDigitValue( argv[i] ) )
				{
					indices[which++] = i;
				}
				gotEmAll = ( which == num );
			}
		}
		return gotEmAll;
	}

	bool GetFloatVector( float *vals, int size, int offset, int argc, const char **argv )
	{
		bool gotEmAll = false;
		if ( vals )
		{
			int which = 0;
			for ( int i = offset; !gotEmAll && (i < argc); ++i )
			{
				if ( HasDigitValue( argv[i] ) )
				{
					vals[which++] = (float)atof(argv[i]);
				}
				gotEmAll = ( which == size );
			}
		}
		return gotEmAll;
	}

	bool GetIntVector( int *vals, int size, int offset, int argc, const char **argv )
	{
		bool gotEmAll = false;
		if ( vals )
		{
			int which = 0;
			for ( int i = offset; !gotEmAll && (i < argc); ++i )
			{
				if ( HasDigitValue( argv[i] ) )
				{
					vals[which++] = atoi(argv[i]);
				}
				gotEmAll = ( which == size );
			}
		}
		return gotEmAll;
	}

	bool HasDigitValue( const char *c )
	{
		bool hasIt = false;
		if ( c )
		{
			int len = strlen( c );
			int i	= 0;
			while ( ( i < len ) && !hasIt )
			{
				if ( ( c[i] >= '0' ) && ( c[i] <= '9' ) )
				{
					hasIt = true;
				}
				++i;
			}
		}
		return hasIt;
	}

	float GetDigitValue( const char *c )
	{
		float val = 0;
		if ( c )
		{
			char buff[128];
			memset( buff, 0, sizeof(char)*128 );
			int len		= strlen( c );
			int i		= 0;
			int which	= 0;
			while ( i < len )
			{
				if ( ( ( c[i] >= '0' ) && ( c[i] <= '9' ) ) ||
					 ( c[i] == '+' ) || ( c[i] == '-' ) || ( c[i] == '.' ) )
				{
					buff[which++] = c[i];
				}
				++i;
			}
			val = (float)atof( buff );
		}
		return val;
	}

	double GetDigitValue_Dbl( const char *c )
	{
		double val = 0;
		if ( c )
		{
			char buff[128];
			memset( buff, 0, sizeof(char)*128 );
			int len		= strlen( c );
			int i		= 0;
			int which	= 0;
			while ( i < len )
			{
				if ( ( ( c[i] >= '0' ) && ( c[i] <= '9' ) ) ||
					 ( c[i] == '+' ) || ( c[i] == '-' ) || ( c[i] == '.' ) )
				{
					buff[which++] = c[i];
				}
				++i;
			}
			val = atof( buff );
		}
		return val;
	}

	int GetDigitValue_Int( const char *c )
	{
		int val = 0;
		if ( c )
		{
			char buff[128];
			memset( buff, 0, sizeof(char)*128 );
			int len		= strlen( c );
			int i		= 0;
			int which	= 0;
			while ( i < len )
			{
				if ( ( ( c[i] >= '0' ) && ( c[i] <= '9' ) ) ||
					 ( c[i] == '+' ) || ( c[i] == '-' ) || ( c[i] == '.' ) )
				{
					buff[which++] = c[i];
				}
				else if ( which > 0 )
				{
					// k, we got all the numbers in a row thus far, so stop
					i = len;
				}
				++i;
			}
			val = atoi( buff );
		}
		return val;
	}

	int GetFirstHex( int offset, int argc, const char **argv )
	{
		int ret = 0;
		for ( int i = offset; i < argc; ++i )
		{
			const char *found = strstr( argv[i], "0x" );

			if ( found )
			{
				char buff[32];

				// k, we gots it
				found += 2;
				strcpy( buff, found );
				strlwrPortable( buff );
				int len = strlen( buff );

				int amt = 0;
				for ( int j = len-1; j >= 0; --j )
				{
					char tmp = buff[j];
					int value = 0;
					if ( tmp >= '0' && tmp <= '9' )
					{
						char ch[2] = { tmp, '\0' };
						value = atoi( ch );
					}
					else
					{
						value = int(tmp) - 97 + 10;
					}
					for ( int k = 0; k < 4; ++k )
					{
						if ( value & (int)pow((float)2,k) )
						{
							ret += (int)pow((float)2, amt );
						}
						++amt;
					}
				}
				break;
			}
		}
		return ret;
	}

	int GetFirstInt( int offset, int argc, const char **argv )
	{
		int which = 0;
		for ( int i = offset; i < argc; ++i )
		{
			if ( HasDigitValue( argv[i] ) )
			{
				which = i;
				break;
			}
		}
		return (int)GetDigitValue( argv[which] );
	}

	int GetFirstUnsigned( int offset, int argc, const char **argv )
	{
		int which = 0;
		for ( int i = offset; i < argc; ++i )
		{
			if ( HasDigitValue( argv[i] ) )
			{
				which = i;
				break;
			}
		}
		return (unsigned)GetDigitValue( argv[which] );
	}

	float GetFirstFloat( int offset, int argc, const char **argv )
	{
		int which = 0;
		int i;
		for ( i = offset; i < argc; ++i )
		{
			if ( HasDigitValue( argv[i] ) )
			{
				which = i;
				break;
			}
		}

		if ( i == argc )
		{
			// hmm, no float found
			const char *buff = GetFirstString( offset, argc, argv );
			char val[128];
			sprintf( val, buff );

			strlwrPortable( val );
			if ( !stricmp( val, "fltmax" ) )
			{
				return FLT_MAX;
			}
			else if ( !stricmp( val, "fltmin" ) )
			{
				return FLT_MIN;
			}
		}

		return GetDigitValue( argv[which] );
	}
	double GetFirstDouble( int offset, int argc, const char **argv )
	{
		int which = 0;
		int i;
		for ( i = offset; i < argc; ++i )
		{
			if ( HasDigitValue( argv[i] ) )
			{
				which = i;
				break;
			}
		}

		if ( i == argc )
		{
			// hmm, no float found
			const char *buff = GetFirstString( offset, argc, argv );
			char val[128];
			sprintf( val, buff );
			strlwrPortable( val );

			if ( !stricmp( val, "dblmax" ) )
			{
				return DBL_MAX;
			}
			else if ( !stricmp( val, "dblmin" ) )
			{
				return DBL_MIN;
			}
		}

		return GetDigitValue_Dbl( argv[which] );
	}

	bool GetFirstBool( int offset, int argc, const char **argv )
	{
		bool value = false;
		for ( int i = offset; i < argc; ++i )
		{
			if ( !stricmp( argv[i], sTrue ) || !stricmp( argv[i], sOne ) )
			{
				value = true;
				break;
			}
			else if ( !stricmp( argv[i], sFalse ) || !stricmp( argv[i], sZero ) )
			{
				value = false;
				break;
			}
		}
		return value;
	}

	bool GetFirstMatrix( NxMat34 &mat, int offset, int argc, const char **argv )
	{
		bool gotEmAll1 = false, gotEmAll2 = false;
		float m[9] = {0,0,0,0,0,0,0,0,0};
		float x = 0, y = 0, z = 0;

		int which = 0;
		int i;
		for ( i = offset; !gotEmAll1 && (i < argc); ++i )
		{
			if ( HasDigitValue( argv[i] ) )
			{
				m[which++] = (float)atof(argv[i]);
			}
			gotEmAll1 = ( which == 9 );
		}
		which = 0;
		for ( ; !gotEmAll2 && (i < argc); ++i )
		{
			if ( HasDigitValue( argv[i] ) )
			{
				if ( which == 0 )
				{
					x = (float)atof(argv[i]);
				}
				else if ( which == 1 )
				{
					y = (float)atof(argv[i]);
				}
				else
				{
					z = (float)atof(argv[i]);
				}
				++which;
			}
			gotEmAll2 = ( which == 3 );
		}

		mat.M.setRowMajor( m );
		mat.t.x = x;
		mat.t.y = y;
		mat.t.z = z;
		return gotEmAll1 && gotEmAll2;
	}

	bool ObtainTriangleMeshInfo( int argc, const char **argv )
	{
		int which  = 0;
		bool found = false;

		while ( !found && ( which < argc ) )
		{
			char buff[256];
			sprintf( buff, argv[which] );
			strlwrPortable( buff );

			if ( !IsHardSymbol( buff ) )
			{
				if ( strstr( buff, sTriangleMeshKey ) )
				{
					found = true;

					// k, time to allocate a new triangle mesh desc
					mTriangleMeshData = new TriangleMeshData;
					if ( mTriangleMeshData )
					{
						mTriangleMeshData->setToDefault();
						mContainer->AddTMD( mTriangleMeshData );
					}
					++mCurrTriMesh;
				}
				else if ( mTriangleMeshData )
				{
					// k, get individual thingies
					if ( strstr( buff, sTriangleMesh_MaterialIndexStride ) )
					{
						mTriangleMeshData->mTmd.materialIndexStride = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangleMesh_MaterialIndices ) )
					{
						// hmm, what do we do here?
						mTriangleMeshData->mTmd.materialIndices = 0;
						found = true;
					}
					else if ( strstr( buff, sTriangleMesh_HeightFieldVerticalAxis ) )
					{
						mTriangleMeshData->mTmd.heightFieldVerticalAxis = (NxHeightFieldAxis)GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangleMesh_HeightFieldVerticalExtent ) )
					{
						mTriangleMeshData->mTmd.heightFieldVerticalExtent = GetFirstFloat( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangleMesh_ConvexEdgeThreshold ) )
					{
						mTriangleMeshData->mTmd.convexEdgeThreshold = GetFirstFloat( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangleMesh_Flag_FlipNormals ) )
					{
						bool val = GetFirstBool( which+1, argc, argv );
						if ( val )
						{
							mTriangleMeshData->mTmd.flags |= NX_MF_FLIPNORMALS;
						}
						else
						{
							mTriangleMeshData->mTmd.flags &= ~NX_MF_FLIPNORMALS;
						}
						found = true;
					}
					else if ( strstr( buff, sTriangleMesh_Flag_16BitIndices ) )
					{
						bool val = GetFirstBool( which+1, argc, argv );
						if ( val )
						{
							mTriangleMeshData->mTmd.flags |= NX_MF_16_BIT_INDICES;
						}
						else
						{
							mTriangleMeshData->mTmd.flags &= ~NX_MF_16_BIT_INDICES;
						}
						found = true;
					}
					else if ( strstr( buff, sNumVertices ) )
					{
						mTriangleMeshData->mTmd.numVertices = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sPointStrideBytes ) )
					{
						mTriangleMeshData->mTmd.pointStrideBytes = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sVertex ) && HasDigitValue( argv[which] ) )
					{
						found = true;
						if ( !mTriangleMeshData->mTmd.points )
						{
							if ( mTriangleMeshData->mTmd.pointStrideBytes == sizeof(float)*3 )
							{
								mVoidVrtPntr = new float[mTriangleMeshData->mTmd.numVertices*3];
								if ( mVoidVrtPntr )
								{
									memset( (void *)mVoidVrtPntr, 0, sizeof(float)*3*mTriangleMeshData->mTmd.numVertices );
									mTriangleMeshData->mTmd.points = (const float *)mVoidVrtPntr;
								}
							}
							else if ( mTriangleMeshData->mTmd.pointStrideBytes == sizeof(double)*3 )
							{
								mVoidVrtPntr = new double[mTriangleMeshData->mTmd.numVertices*3];
								if ( mVoidVrtPntr )
								{
									memset( (void *)mVoidVrtPntr, 0, sizeof(double)*3*mTriangleMeshData->mTmd.numVertices );
									mTriangleMeshData->mTmd.points = (const double *)mVoidVrtPntr;
								}
							}
							else
							{
								mVoidVrtPntr = 0;
							}
						}
						if ( mTriangleMeshData->mTmd.points )
						{
							int index[3] = {-1, -1, -1};

							// get which vert this is
							unsigned vertNum = (unsigned)GetDigitValue( argv[which] ) - 1;
							if ( GetIndices( 3, index, which+1, argc, argv ) )
							{
								if ( vertNum < mTriangleMeshData->mTmd.numVertices )
								{
									if ( mTriangleMeshData->mTmd.pointStrideBytes == sizeof(float)*3 )
									{
										char *ctmp	= (char *)mVoidVrtPntr;
										float *tmp	= (float *)&ctmp[vertNum*3*sizeof(float)];

										// now set the vert value
										tmp[0] = GetFirstFloat( index[0], argc, argv );
										tmp[1] = GetFirstFloat( index[1], argc, argv );
										tmp[2] = GetFirstFloat( index[2], argc, argv );
									}
									else if ( mTriangleMeshData->mTmd.pointStrideBytes == sizeof(double)*3 )
									{
										char *ctmp	= (char *)mVoidVrtPntr;
										double *tmp	= (double *)&ctmp[vertNum*3*sizeof(double)];

										// now set the vert value
										tmp[0] = GetFirstDouble( index[0], argc, argv );
										tmp[1] = GetFirstDouble( index[1], argc, argv );
										tmp[2] = GetFirstDouble( index[2], argc, argv );
									}
								}
								else
								{
									// spit out err msg, but continue on
									char tmp[128];
									sprintf( tmp, "TriMesh %d - Have a vertex num of %d - indicated max num of vertices to be %d - continuing anyways",
										mCurrTriMesh, vertNum, mTriangleMeshData->mTmd.numVertices );
									OutputStatusMsg( tmp );
								}
							}
							else
							{
								char tmp[128];
								sprintf( tmp, "TriMesh %d - Could not determine vertices for vertex num %d (will be set to all 0s) - continuing anyways",
									mCurrTriMesh, vertNum );
								OutputStatusMsg( tmp );
							}
						}
					}
					else if ( strstr( buff, sNumTriangles ) )
					{
						mTriangleMeshData->mTmd.numTriangles = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangleStrideBytes ) )
					{
						mTriangleMeshData->mTmd.triangleStrideBytes = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangle ) && HasDigitValue( argv[which] ) )
					{
						if ( !mTriangleMeshData->mTmd.triangles )
						{
							if ( mTriangleMeshData->mTmd.triangleStrideBytes == 3*sizeof(int) )
							{
								mVoidTriPntr = new int[mTriangleMeshData->mTmd.numTriangles*3];
								if ( mVoidTriPntr )
								{
									memset( mVoidTriPntr, 0, sizeof(int)*3*mTriangleMeshData->mTmd.numTriangles );
									mTriangleMeshData->mTmd.triangles = (const int *)mVoidTriPntr;
								}
							}
							else if ( mTriangleMeshData->mTmd.triangleStrideBytes == 3*sizeof(short) )
							{
								mVoidTriPntr = new short[mTriangleMeshData->mTmd.numTriangles*3];
								if ( mVoidTriPntr )
								{
									memset( mVoidTriPntr, 0, sizeof(short)*3*mTriangleMeshData->mTmd.numTriangles );
									mTriangleMeshData->mTmd.triangles = (const short *)mVoidTriPntr;
								}
							}
							else if ( mTriangleMeshData->mTmd.triangleStrideBytes == 3*sizeof(char) )
							{
								mVoidTriPntr = new char[mTriangleMeshData->mTmd.numTriangles*3];
								if ( mVoidTriPntr )
								{
									memset( mVoidTriPntr, 0, sizeof(char)*3*mTriangleMeshData->mTmd.numTriangles );
									mTriangleMeshData->mTmd.triangles = (const char *)mVoidTriPntr;
								}
							}
							else
							{
								mVoidTriPntr = 0;
							}
						}

						if ( mTriangleMeshData->mTmd.triangles )
						{
							int index[3] = {-1, -1, -1};

							// get which vert this is
							unsigned triNum = (unsigned)GetDigitValue( argv[which] ) - 1;
							if ( GetIndices( 3, index, which+1, argc, argv ) )
							{
								if ( triNum < mTriangleMeshData->mTmd.numTriangles )
								{
									if ( mTriangleMeshData->mTmd.triangleStrideBytes == 3*sizeof(int) )
									{
										char *ctmp	= (char *)mVoidTriPntr;
										int *tmp	= (int *)&ctmp[triNum*3*sizeof(int)];

										// now set the tri value
										tmp[0]	= atoi( argv[index[0]] );
										tmp[1]	= atoi( argv[index[1]] );
										tmp[2]	= atoi( argv[index[2]] );
									}
									else if ( mTriangleMeshData->mTmd.triangleStrideBytes == 3*sizeof(short) )
									{
										char *ctmp	= (char *)mVoidTriPntr;
										short *tmp	= (short *)&ctmp[triNum*3*sizeof(short)];

										// now set the tri value
										tmp[0]	= (short)atoi( argv[index[0]] );
										tmp[1]	= (short)atoi( argv[index[1]] );
										tmp[2]	= (short)atoi( argv[index[2]] );
									}
									else if ( mTriangleMeshData->mTmd.triangleStrideBytes == 3*sizeof(char) )
									{
										char *ctmp	= (char *)mVoidTriPntr;
										char *tmp	= (char *)&ctmp[triNum*3*sizeof(char)];

										// now set the tri value
										tmp[0]	= (char)atoi( argv[index[0]] );
										tmp[1]	= (char)atoi( argv[index[1]] );
										tmp[2]	= (char)atoi( argv[index[2]] );
									}
								}
								else
								{
									// spit out err msg, but continue on
									char tmp[128];
									sprintf( tmp, "TriMesh %d - Have a triangle num of %d - indicated max num of triangles to be %d - continuing anyways",
										mCurrTriMesh, triNum, mTriangleMeshData->mTmd.numTriangles );
									OutputStatusMsg( tmp );
								}
							}
							else
							{
								char tmp[128];
								sprintf( tmp, "TriMesh %d - Could not determine vertex indices for triangle num %d (will be set to all 0s) - continuing anyways",
									mCurrTriMesh, triNum );
								OutputStatusMsg( tmp );
							}
						}

						found = true;
					}
					else if ( strstr( buff, sPmap ) )
					{
						// hmm, what do we do here?
						found = true;
					}
				}
			}
			++which;
		}

		return found;
	}

	bool ObtainConvexMeshInfo( int argc, const char **argv )
	{
		int which  = 0;
		bool found = false;

		while ( !found && ( which < argc ) )
		{
			char buff[256];
			sprintf( buff, argv[which] );
			strlwrPortable( buff );

			if ( !IsHardSymbol( buff ) )
			{
				if ( strstr( buff, sConvexMeshKey ) )
				{
					found = true;

					// k, time to allocate a new convex mesh desc
					mConvexMeshData = new ConvexMeshData;
					if ( mConvexMeshData )
					{
						mConvexMeshData->setToDefault();
						mContainer->AddCMD( mConvexMeshData );
					}
					++mCurrConvexMesh;
				}
				else if ( mConvexMeshData )
				{
					if ( strstr( buff, sConvexMesh_Flag_FlipNormals ) )
					{
						bool val = GetFirstBool( which+1, argc, argv );
						if ( val )
						{
							mConvexMeshData->mCmd.flags |= NX_CF_FLIPNORMALS;
						}
						else
						{
							mConvexMeshData->mCmd.flags &= ~NX_CF_FLIPNORMALS;
						}
						found = true;					
					}
					else if ( strstr( buff, sConvexMesh_Flag_16BitIndices ) )
					{
						bool val = GetFirstBool( which+1, argc, argv );
						if ( val )
						{
							mConvexMeshData->mCmd.flags |= NX_CF_16_BIT_INDICES;
						}
						else
						{
							mConvexMeshData->mCmd.flags &= ~NX_CF_16_BIT_INDICES;
						}
						found = true;					
					}
					else if ( strstr( buff, sConvexMesh_Flag_ComputeConvex ) )
					{
						bool val = GetFirstBool( which+1, argc, argv );
						if ( val )
						{
							mConvexMeshData->mCmd.flags |= NX_CF_COMPUTE_CONVEX;
						}
						else
						{
							mConvexMeshData->mCmd.flags &= ~NX_CF_COMPUTE_CONVEX;
						}
						found = true;					
					}
					else if ( strstr( buff, sConvexMesh_Flag_InflateConvex ) )
					{
						bool val = GetFirstBool( which+1, argc, argv );
						if ( val )
						{
#if NX_SDK_VERSION_NUMBER >= 230

#if NX_SDK_VERSION_NUMBER <= 230
							mConvexMeshData->mCmd.flags |= NX_CF_INFLATE_CONVEX;
#else
							mConvexMeshData->mCmd.flags &= ~NX_CF_USE_LEGACY_COOKER;
#endif

#endif
						}
						else
						{
#if NX_SDK_VERSION_NUMBER >= 230

#if NX_SDK_VERSION_NUMBER <= 230
							mConvexMeshData->mCmd.flags &= ~NX_CF_INFLATE_CONVEX;
#else
							mConvexMeshData->mCmd.flags |= NX_CF_USE_LEGACY_COOKER;
#endif

#endif
						}
						found = true;					
					}
					else if ( strstr( buff, sNumVertices ) )
					{
						mConvexMeshData->mCmd.numVertices = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sPointStrideBytes ) )
					{
						mConvexMeshData->mCmd.pointStrideBytes = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sVertex ) )
					{
						found = true;
						if ( !mConvexMeshData->mCmd.points )
						{
							if ( mConvexMeshData->mCmd.pointStrideBytes == sizeof(float)*3 )
							{
								mVoidVrtPntr = new float[mConvexMeshData->mCmd.numVertices*3];
								if ( mVoidVrtPntr )
								{
									memset( (void *)mVoidVrtPntr, 0, sizeof(float)*3*mConvexMeshData->mCmd.numVertices );
									mConvexMeshData->mCmd.points = (const float *)mVoidVrtPntr;
								}
							}
							else if ( mConvexMeshData->mCmd.pointStrideBytes == sizeof(double)*3 )
							{
								mVoidVrtPntr = new double[mConvexMeshData->mCmd.numVertices*3];
								if ( mVoidVrtPntr )
								{
									memset( (void *)mVoidVrtPntr, 0, sizeof(double)*3*mConvexMeshData->mCmd.numVertices );
									mConvexMeshData->mCmd.points = (const double *)mVoidVrtPntr;
								}
							}
							else
							{
								mVoidVrtPntr = 0;
							}
						}
						if ( mConvexMeshData->mCmd.points )
						{
							int index[3] = {-1, -1, -1};

							// get which vert this is
							unsigned vertNum = (unsigned)GetDigitValue( argv[which] ) - 1;
							if ( GetIndices( 3, index, which+1, argc, argv ) )
							{
								if ( vertNum < mConvexMeshData->mCmd.numVertices )
								{
									if ( mConvexMeshData->mCmd.pointStrideBytes == sizeof(float)*3 )
									{
										char *ctmp	= (char *)mVoidVrtPntr;
										float *tmp	= (float *)&ctmp[vertNum*3*sizeof(float)];

										// now set the vert value
										tmp[0]	= GetFirstFloat( index[0], argc, argv );
										tmp[1]	= GetFirstFloat( index[1], argc, argv );
										tmp[2]	= GetFirstFloat( index[2], argc, argv );
									}
									else if ( mConvexMeshData->mCmd.pointStrideBytes == sizeof(double)*3 )
									{
										char *ctmp	= (char *)mVoidVrtPntr;
										double *tmp	= (double *)&ctmp[vertNum*3*sizeof(double)];

										// now set the vert value
										tmp[0]	= GetFirstDouble( index[0], argc, argv );
										tmp[1]	= GetFirstDouble( index[1], argc, argv );
										tmp[2]	= GetFirstDouble( index[2], argc, argv );
									}
								}
								else
								{
									// spit out err msg, but continue on
									char tmp[128];
									sprintf( tmp, "ConvexMesh %d - Have a vertex num of %d - indicated max num of vertices to be %d - continuing anyways",
										mCurrConvexMesh, vertNum, mConvexMeshData->mCmd.numVertices );
									OutputStatusMsg( tmp );
								}
							}
							else
							{
								char tmp[128];
								sprintf( tmp, "ConvexMesh %d - Could not determine vertices for vertex num %d (will be set to all 0s) - continuing anyways",
									mCurrConvexMesh, vertNum );
								OutputStatusMsg( tmp );
							}
						}
					}
					else if ( strstr( buff, sNumTriangles ) )
					{
						mConvexMeshData->mCmd.numTriangles = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangleStrideBytes ) )
					{
						mConvexMeshData->mCmd.triangleStrideBytes = GetFirstInt( which+1, argc, argv );
						found = true;
					}
					else if ( strstr( buff, sTriangle ) && HasDigitValue( argv[which] ) )
					{
						if ( !mConvexMeshData->mCmd.triangles )
						{
							if ( mConvexMeshData->mCmd.triangleStrideBytes == 3*sizeof(int) )
							{
								mVoidTriPntr = new int[mConvexMeshData->mCmd.numTriangles*3];
								if ( mVoidTriPntr )
								{
									memset( mVoidTriPntr, 0, sizeof(int)*3*mConvexMeshData->mCmd.numTriangles );
									mConvexMeshData->mCmd.triangles = (const int *)mVoidTriPntr;
								}
							}
							else if ( mConvexMeshData->mCmd.triangleStrideBytes == 3*sizeof(short) )
							{
								mVoidTriPntr = new short[mConvexMeshData->mCmd.numTriangles*3];
								if ( mVoidTriPntr )
								{
									memset( mVoidTriPntr, 0, sizeof(short)*3*mConvexMeshData->mCmd.numTriangles );
									mConvexMeshData->mCmd.triangles = (const short *)mVoidTriPntr;
								}
							}
							else if ( mConvexMeshData->mCmd.triangleStrideBytes == 3*sizeof(char) )
							{
								mVoidTriPntr = new char[mConvexMeshData->mCmd.numTriangles*3];
								if ( mVoidTriPntr )
								{
									memset( mVoidTriPntr, 0, sizeof(char)*3*mConvexMeshData->mCmd.numTriangles );
									mConvexMeshData->mCmd.triangles = (const char *)mVoidTriPntr;
								}
							}
							else
							{
								mVoidTriPntr = 0;
							}
						}

						if ( mConvexMeshData->mCmd.triangles )
						{
							int index[3] = {-1, -1, -1};

							// get which vert this is
							unsigned triNum = (unsigned)GetDigitValue( argv[which] ) - 1;
							if ( GetIndices( 3, index, which+1, argc, argv ) )
							{
								if ( triNum < mConvexMeshData->mCmd.numTriangles )
								{
									if ( mConvexMeshData->mCmd.triangleStrideBytes == 3*sizeof(int) )
									{
										char *ctmp	= (char *)mVoidTriPntr;
										int *tmp	= (int *)&ctmp[triNum*3*sizeof(int)];

										// now set the tri value
										tmp[0]	= atoi( argv[index[0]] );
										tmp[1]	= atoi( argv[index[1]] );
										tmp[2]	= atoi( argv[index[2]] );
									}
									else if ( mConvexMeshData->mCmd.triangleStrideBytes == 3*sizeof(short) )
									{
										char *ctmp	= (char *)mVoidTriPntr;
										short *tmp	= (short *)&ctmp[triNum*3*sizeof(short)];

										// now set the tri value
										tmp[0]	= (short)atoi( argv[index[0]] );
										tmp[1]	= (short)atoi( argv[index[1]] );
										tmp[2]	= (short)atoi( argv[index[2]] );
									}
									else if ( mConvexMeshData->mCmd.triangleStrideBytes == 3*sizeof(char) )
									{
										char *ctmp	= (char *)mVoidTriPntr;
										char *tmp	= (char *)&ctmp[triNum*3*sizeof(char)];

										// now set the tri value
										tmp[0]	= (char)atoi( argv[index[0]] );
										tmp[1]	= (char)atoi( argv[index[1]] );
										tmp[2]	= (char)atoi( argv[index[2]] );
									}
								}
								else
								{
									// spit out err msg, but continue on
									char tmp[128];
									sprintf( tmp, "ConvexMesh %d - Have a triangle num of %d - indicated max num of triangles to be %d - continuing anyways",
										mCurrConvexMesh, triNum, mConvexMeshData->mCmd.numTriangles );
									OutputStatusMsg( tmp );
								}
							}
							else
							{
								char tmp[128];
								sprintf( tmp, "ConvexMesh %d - Could not determine vertex indices for triangle num %d (will be set to all 0s) - continuing anyways",
									mCurrConvexMesh, triNum );
								OutputStatusMsg( tmp );
							}
						}

						found = true;
					}
					else if ( strstr( buff, sPmap ) )
					{
						// hmm, what do we do here?
						found = true;
					}				
				}
			}
			++which;
		}

		return found;
	}

	bool ObtainSkeletonInfo( int argc, const char **argv )
	{
		int which  = 0;
		bool found = false;

		while ( !found && ( which < argc ) )
		{
			char buff[256];
			sprintf( buff, argv[which] );
			strlwrPortable( buff );

			if ( !IsHardSymbol( buff ) )
			{
				if ( strstr( buff, sCCDSkeletonMeshKey ) )
				{
					found = true;
				}
			}
			++which;
		}

		return found;
	}

	bool ObtainHeightFieldInfo( int argc, const char **argv )
	{
		int which  = 0;
		bool found = false;

		while ( !found && ( which < argc ) )
		{
			char buff[256];
			sprintf( buff, argv[which] );
			strlwrPortable( buff );

			if ( !IsHardSymbol( buff ) )
			{
				if ( strstr( buff, sHeightFieldKey ) )
				{
					found = true;

					// k, time to allocate a new triangle mesh desc
					mHeightFieldData = new HeightFieldData;
					if ( mHeightFieldData )
					{
						mHeightFieldData->setToDefault();
#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.samples = 0;
#endif
						mContainer->AddHFD( mHeightFieldData );
					}
					++mCurrHeightField;
				}
				else if ( mHeightFieldData )
				{
					if ( strstr( buff, sHeightField_Flag_BoundaryEdges ) )
					{
						found = true;

						bool val = GetFirstBool( which+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 240
						if ( val )
						{
							mHeightFieldData->mHfd.flags |= NX_HF_NO_BOUNDARY_EDGES;
						}
						else
						{
							mHeightFieldData->mHfd.flags &= ~NX_HF_NO_BOUNDARY_EDGES;
						}
#endif
					}
					else if ( strstr( buff, sHeightField_NumRows ) )
					{
						found = true;

#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.nbRows = GetFirstUnsigned( which+1, argc, argv );
#endif
					}
					else if ( strstr( buff, sHeightField_NumColumns ) )
					{
						found = true;

#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.nbColumns = GetFirstUnsigned( which+1, argc, argv );
#endif
					}
					else if ( strstr( buff, sHeightField_Format ) )
					{
						found = true;

#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.format = (NxHeightFieldFormat)GetFirstUnsigned( which+1, argc, argv );
#endif
					}
					else if ( strstr( buff, sHeightField_VerticalExtent ) )
					{
						found = true;

#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.verticalExtent = GetFirstFloat( which+1, argc, argv );
#endif
					}
					else if ( strstr( buff, sHeightField_ConvexEdgeThreshold ) )
					{
						found = true;

#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.convexEdgeThreshold = GetFirstFloat( which+1, argc, argv );
#endif
					}
					else if ( strstr( buff, sHeightField_SampleStride ) )
					{
						found = true;

						int value = GetFirstInt( which+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 240
						mHeightFieldData->mHfd.sampleStride = value;
						// k, now we allocate the buffer!
						if ( value )
						{
							unsigned totalSize	= mHeightFieldData->mHfd.nbRows * mHeightFieldData->mHfd.nbColumns;
							unsigned *data		= new unsigned[totalSize];
							if ( data )
							{
								memset( data, 0, sizeof(unsigned)*totalSize );
							}
							mHeightFieldData->mHfd.samples = data;
						}
#endif
					}
#if NX_SDK_VERSION_NUMBER >= 240
					else if ( mHeightFieldData->mHfd.samples )
					{
						int totalSize = mHeightFieldData->mHfd.nbRows * mHeightFieldData->mHfd.nbColumns;

						if ( strstr( buff, sHeightField_Information ) )
						{
							// just indicates info is following ... ignore but indicate found
							found = true;
						}
						else if ( strstr( buff, sHeightField_HeightFieldValue ) )
						{
							int index = GetFirstInt( which, argc, argv );
							assert( ( index >= 0 ) && ( index < totalSize ) );
							found = true;

							int value		= GetFirstInt( which+1, argc, argv );
							unsigned *tmp	= (unsigned *)&mHeightFieldData->mHfd.samples;
							tmp[index]		|= value;
						}
						else if ( strstr( buff, sHeightField_MaterialIndexOne ) )
						{
							int index = GetFirstInt( which, argc, argv );
							assert( ( index >= 0 ) && ( index < totalSize ) );
							found = true;

							int value		= GetFirstInt( which+1, argc, argv );
							unsigned *tmp	= (unsigned *)&mHeightFieldData->mHfd.samples;
							tmp[index]		|= ( value << 16 );
						}
						else if ( strstr( buff, sHeightField_MaterialIndexTwo ) )
						{
							int index = GetFirstInt( which, argc, argv );
							assert( ( index >= 0 ) && ( index < totalSize ) );
							found = true;

							int value		= GetFirstInt( which+1, argc, argv );
							unsigned *tmp	= (unsigned *)&mHeightFieldData->mHfd.samples;
							tmp[index]		|= ( value << 23 );
						}
						else if ( strstr( buff, sHeightField_TessFlagOne ) )
						{
							int index = GetFirstInt( which, argc, argv );
							assert( ( index >= 0 ) && ( index < totalSize ) );
							found = true;

							int value		= GetFirstInt( which+1, argc, argv );
							unsigned *tmp	= (unsigned *)&mHeightFieldData->mHfd.samples;
							tmp[index]		|= ( value << 24 );
						}
						else if ( strstr( buff, sHeightField_TessFlagTwo ) )
						{
							int index = GetFirstInt( which, argc, argv );
							assert( ( index >= 0 ) && ( index < totalSize ) );
							found = true;

							int value		= GetFirstInt( which+1, argc, argv );
							unsigned *tmp	= (unsigned *)&mHeightFieldData->mHfd.samples;
							tmp[index]		|= ( value << 31 );
						}
					}
#endif // #if NX_SDK_VERSION_NUMBER >= 240
					else
					{
						if ( strstr( buff, sHeightField_Information ) ||	
							 strstr( buff, sHeightField_HeightFieldValue ) ||
							 strstr( buff, sHeightField_MaterialIndexOne ) ||
							 strstr( buff, sHeightField_MaterialIndexTwo ) ||
							 strstr( buff, sHeightField_TessFlagOne ) ||
							 strstr( buff, sHeightField_TessFlagTwo ) 
						   )
						{
							found = true;
						}
					}
				}
			}

			++which;
		}

		return found;
	}

	bool ObtainSceneDesc( const char *buff, int which, int argc, const char **argv )
	{
		bool found = true;

		// k, make sure of the buff pntrs
		if ( buff && mCoreScene )
		{
			// k, now get the appropriate param
//			if ( strstr( buff, sSceneDesc_CollisionDetection ) )
//			{
//				mCoreScene->mDesc.collisionDetection = GetFirstBool( which+1, argc, argv );
//			}
//			else 
			if ( strstr( buff, sSceneDesc_GroundPlane ) )
			{
				mCoreScene->mDesc.groundPlane = GetFirstBool( which+1, argc, argv );
			}
			else if ( strstr( buff, sSceneDesc_BoundsPlanes ) )
			{
				mCoreScene->mDesc.boundsPlanes = GetFirstBool( which+1, argc, argv );
			}
			else if ( strstr( buff, sSceneDesc_Gravity ) )
			{
				float vals[3];
				GetFloatVector( vals, 3, which+1, argc, argv );
				mCoreScene->mDesc.gravity.set( vals[0], vals[1], vals[2] );
			}
//			else if ( strstr( buff, sSceneDesc_BroadPhase ) )
//			{
//				mCoreScene->mDesc.broadPhase = (NxBroadPhaseType)GetFirstInt( which+1, argc, argv );
//			}
			else if ( strstr( buff, sSceneDesc_TimeStepMethod ) )
			{
				mCoreScene->mDesc.timeStepMethod = (NxTimeStepMethod)GetFirstInt( which+1, argc, argv );
			}
			else if ( strstr( buff, sSceneDesc_MaxTimestep ) )
			{
				mCoreScene->mDesc.maxTimestep = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sSceneDesc_MaxIter ) )
			{
				mCoreScene->mDesc.maxIter = GetFirstInt( which+1, argc, argv );
			}
			else if ( strstr( buff, sSceneDesc_SimType ) )
			{
#if NX_SDK_VERSION_NUMBER >= 230
				mCoreScene->mDesc.simType = static_cast<NxSimulationType>(GetFirstInt( which+1, argc, argv ));
#endif
			}
			else if ( strstr( buff, sSceneDesc_HwSceneType ) )
			{
#if NX_SDK_VERSION_NUMBER >= 230

#endif
			}
			else if ( strstr( buff, sSceneDesc_PipelineSpec ) )
			{
#if NX_SDK_VERSION_NUMBER >= 231

#endif
			}
			else if ( strstr( buff, sSceneDesc_Limits ) )
			{
				bool hasLimits = GetFirstBool( which+1, argc, argv );
				//mCoreScene->mDesc.limits
			}
			else if ( strstr( buff, sSceneDesc_MaxBounds ) )
			{
				bool hasMaxBounds = GetFirstBool( which+1, argc, argv );
				//mCoreScene->mDesc.maxBounds
			}
			else if ( strstr( buff, sSceneDesc_NxUserNotify ) )
			{
				bool hadUserNotify = GetFirstBool( which+1, argc, argv );
				// don't do anything - cannot here!
				// mCoreScene->mDesc.userNotify
			}
			else if ( strstr( buff, sSceneDesc_NxUserTriggerReport ) )
			{
				bool hadUserTrigger = GetFirstBool( which+1, argc, argv );
				// don't do anything - cannot here!
				// mCoreScene->mDesc.userTriggerReport
			}
			else if ( strstr( buff, sSceneDesc_NxUserContactReport ) )
			{
				bool hadUserContactReport = GetFirstBool( which+1, argc, argv );
				// don't do anything - cannot here!
				// mCoreScene->mDesc.userContactReport
			}
			else if ( strstr( buff, sSceneDesc_NxUserData ) )
			{
				bool hadUserData = GetFirstBool( which+1, argc, argv );
				// don't do anything - cannot here!
				// mCoreScene->mDesc.userData
			}
			else
			{
				found = false;
			}
		}
		return found;
	}

	bool ObtainMaterialDesc( const char *buff, int which, int argc, const char **argv )
	{
		bool found = true;

		// k, make sure of the buff pntrs
		if ( buff )
		{
			if ( strstr( buff, sNxMaterial ) )
			{
				if ( mCoreScene )
				{
					if ( mCurrMat )
					{
						mCurrMat = 0;
					}
					if ( !mCurrMat )
					{
						mCurrMat = new NxMaterialDesc;
						assert( mCurrMat );
						if ( mCurrMat )
						{
							mCurrMat->setToDefault();
							mCoreScene->mMaterials.push_back( mCurrMat );
						}
					}
				}
			}
			else if ( mCurrMat )
			{
				if ( strstr( buff, sMaterial_DynamicFrictionV ) )
				{
					mCurrMat->dynamicFrictionV = GetFirstFloat( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_StaticFrictionV ) )
				{
					mCurrMat->staticFrictionV = GetFirstFloat( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_DynamicFriction ) )
				{
					mCurrMat->dynamicFriction = GetFirstFloat( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_StaticFriction ) )
				{
					mCurrMat->staticFriction = GetFirstFloat( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_RestitutionCombineMode ) )
				{
					mCurrMat->restitutionCombineMode = (NxCombineMode)GetFirstInt( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_Restitution ) )
				{
					mCurrMat->restitution = GetFirstFloat( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_FrictionCombineMode ) )
				{
					mCurrMat->frictionCombineMode = (NxCombineMode)GetFirstInt( which+1, argc, argv );
				}
				else if ( strstr( buff, sMaterial_DirOfAnisotropy ) )
				{
					float vals[3];
					GetFloatVector( vals, 3, which+1, argc, argv );
					mCurrMat->dirOfAnisotropy.set( vals[0], vals[1], vals[2] );
				}
				else if ( strstr( buff, sMaterial_Flag_Anisotropic ) )
				{
					bool val = GetFirstBool( which+1, argc, argv );
					if ( val )
					{
						mCurrMat->flags |= NX_MF_ANISOTROPIC;
					}
					else
					{
						mCurrMat->flags &= ~NX_MF_ANISOTROPIC;
					}
				}
				else if ( strstr( buff, sMaterial_Flag_SpringContact ) )
				{
					bool val = GetFirstBool( which+1, argc, argv );
/*
					if ( val )
					{
						mCurrMat->flags |= NX_MF_SPRING_CONTACT;
					}
					else
					{
						mCurrMat->flags &= ~NX_MF_SPRING_CONTACT;
					}
*/
				}
				else if ( strstr( buff, sSpring ) )
				{
					// k, note that this is done 2X - one time it will say either
					// spring=NULL or spring=TRUE, and the other time it will say
					// Spring Desc Present = blah - we ignore this one!
					const char *buff = GetFirstString( which+1, argc, argv );
					char str[128];
					sprintf( str, buff );
					strlwrPortable( str );

					if ( stricmp( str, "present" ) )
					{
						if ( GetFirstBool( which+1, argc, argv ) )
						{
#if NX_SDK_VERSION_NUMBER >= 230
							mCurrMat->spring = new NxSpringDesc;
							if ( mCurrMat->spring )
							{
								mCurrMat->spring->setToDefault();
							}
#endif
						}
					}
				}
#if NX_SDK_VERSION_NUMBER >= 230
				else if ( mCurrMat->spring && strstr( buff, sSpring_Spring ) )
				{
					mCurrMat->spring->spring = GetFirstFloat( which+1, argc, argv );
				}
				else if ( mCurrMat->spring && strstr( buff, sSpring_Damper ) )
				{
					mCurrMat->spring->damper = GetFirstFloat( which+1, argc, argv );
				}
				else if ( mCurrMat->spring && strstr( buff, sSpring_TargetValue ) )
				{
					mCurrMat->spring->targetValue = GetFirstFloat( which+1, argc, argv );
				}
#endif
				else
				{
					found = false;
				}
			}
		}

		return found;
	}

	void ObtainActorDesc_Props( const char *buff, int which, int argc, const char **argv )
	{
		if ( strstr( buff, sActor_GlobalPose ) )
		{
			GetFirstMatrix( mCurrActor->globalPose, which+1, argc, argv );
		}
		else if ( strstr( buff, sDensity ) )
		{
			mCurrActor->density = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sGroup ) )
		{
			mCurrActor->group = GetFirstInt( which+1, argc, argv );
		}
		else if ( strstr( buff, sActor_Flag_DisableCollision ) )
		{
			bool flag = GetFirstBool( which+1, argc, argv );
			if ( flag )
			{
				mCurrActor->flags |= NX_AF_DISABLE_COLLISION;
			}
			else
			{
				mCurrActor->flags &= ~NX_AF_DISABLE_COLLISION;
			}
		}
		else if ( strstr( buff, sActor_Flag_DisableResponse ) )
		{
			bool flag = GetFirstBool( which+1, argc, argv );
			if ( flag )
			{
				mCurrActor->flags |= NX_AF_DISABLE_RESPONSE;
			}
			else
			{
				mCurrActor->flags &= ~NX_AF_DISABLE_RESPONSE;
			}
		}
#if NX_USE_FLUID_API	
		else if ( strstr( buff, sActor_Flag_FluidDisableCollision ) )
		{
			bool flag = GetFirstBool( which+1, argc, argv );
			if ( flag )
			{
				mCurrActor->flags |= NX_AF_FLUID_DISABLE_COLLISION;
			}
			else
			{
				mCurrActor->flags &= ~NX_AF_FLUID_DISABLE_COLLISION;
			}
		}
		else if ( strstr( buff, sActor_Flag_FluidActorReaction ) )
		{
/* OLD_FLUID_API
			bool flag = GetFirstBool( which+1, argc, argv );
			if ( flag )
			{
				mCurrActor->flags |= NX_AF_FLUID_ACTOR_REACTION;
			}
			else
			{
				mCurrActor->flags &= ~NX_AF_FLUID_ACTOR_REACTION;
			}
*/
		}
#endif
		else if ( strstr( buff, sName ) )
		{
			mCurrActor->name = GetFirstString( which+1, argc, argv );
		}
	}

	void ObtainActorDesc_Body( const char *buff, int which, int argc, const char **argv )
	{
		if ( strstr( buff, sBody ) )
		{
			// k, if this says true in it, we got a new body desc!
			bool present = GetFirstBool( which+1, argc, argv );
			if ( present )
			{
				mBodyDesc = new NxBodyDesc;
				if ( mBodyDesc )
				{
					mBodyDesc->setToDefault();
					mCurrActor->body = mBodyDesc;
				}
			}
			else
			{
				// if the word dynamic is present, then this is just another indicator that we
				// are still on the same body
				const char *buff = GetFirstString( which+1, argc, argv );
				char str[128];
				sprintf( str, buff );

				if ( str )
				{
					strlwrPortable( str );
					if ( strstr( str, sNull ) && mBodyDesc )
					{
						mBodyDesc = 0;
					}
				}
				else if ( mBodyDesc )
				{
					mBodyDesc = 0;
				}
			}
		}
		else if ( mBodyDesc )
		{
			if ( strstr( buff, sActorDesc_MassLocalPose ) )
			{		
				GetFirstMatrix( mBodyDesc->massLocalPose, which+1, argc, argv );
			}
			else if ( strstr( buff, sActorDesc_MassSpaceInertia ) )
			{
				float vals[3];
				GetFloatVector( vals, 3, which+1, argc, argv );
				mBodyDesc->massSpaceInertia.set( vals[0], vals[1], vals[2] );
			}
			else if ( strstr( buff, sMass ) )
			{
				mBodyDesc->mass = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sActorDesc_SleepLinearVelocity ) ) 
			{
				mBodyDesc->sleepLinearVelocity = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sActorDesc_SleepAngularVelocity ) )
			{
				mBodyDesc->sleepAngularVelocity = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sMaxAngularVelocity ) )
			{
				mBodyDesc->maxAngularVelocity = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sLinearVelocity ) )
			{
				float vals[3];
				GetFloatVector( vals, 3, which+1, argc, argv );
				mBodyDesc->linearVelocity.set( vals[0], vals[1], vals[2] );
			}
			else if ( strstr( buff, sAngularVelocity ) )
			{
				float vals[3];
				GetFloatVector( vals, 3, which+1, argc, argv );
				mBodyDesc->angularVelocity.set( vals[0], vals[1], vals[2] );
			}
			else if ( strstr( buff, sWakeUpCounter ) )
			{
				mBodyDesc->wakeUpCounter = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sLinearDamping ) )
			{
				mBodyDesc->linearDamping = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sAngularDamping ) )
			{
				mBodyDesc->angularDamping = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sActorDesc_CCDMotionThreshold ) )
			{
#if NX_SDK_VERSION_NUMBER >= 230
				mBodyDesc->CCDMotionThreshold = GetFirstFloat( which+1, argc, argv );
#endif
			}
			else if ( strstr( buff, sActorDesc_SolverIterationCount ) )
			{
				mBodyDesc->solverIterationCount = GetFirstInt( which+1, argc, argv );
			}
			else if ( strstr( buff, sActorDesc_Flag_DisableGravity ) )
			{
				if ( GetFirstBool( which+1, argc, argv ) )
				{
					mBodyDesc->flags |= NX_BF_DISABLE_GRAVITY;
				}
				else
				{
					mBodyDesc->flags &= ~NX_BF_DISABLE_GRAVITY;
				}
			}
			else if ( strstr( buff, sActorDesc_Flag_FrozenPos ) )
			{
				if ( strstr( buff, sX ) )
				{
					if ( GetFirstBool( which+1, argc, argv ) )
					{
						mBodyDesc->flags |= NX_BF_FROZEN_POS_X;
					}
					else
					{
						mBodyDesc->flags &= ~NX_BF_FROZEN_POS_X;
					}
				}
				else if ( strstr( buff, sY ) )
				{
					if ( GetFirstBool( which+1, argc, argv ) )
					{
						mBodyDesc->flags |= NX_BF_FROZEN_POS_Y;
					}
					else
					{
						mBodyDesc->flags &= ~NX_BF_FROZEN_POS_Y;
					}
				}
				else if ( strstr( buff, sZ ) )
				{
					if ( GetFirstBool( which+1, argc, argv ) )
					{
						mBodyDesc->flags |= NX_BF_FROZEN_POS_Z;
					}
					else
					{
						mBodyDesc->flags &= ~NX_BF_FROZEN_POS_Z;
					}
				}
			}
			else if ( strstr( buff, sActorDesc_Flag_FrozenRot ) )
			{
				if ( strstr( buff, sX ) )
				{
					if ( GetFirstBool( which+1, argc, argv ) )
					{
						mBodyDesc->flags |= NX_BF_FROZEN_ROT_X;
					}
					else
					{
						mBodyDesc->flags &= ~NX_BF_FROZEN_ROT_X;
					}
				}
				else if ( strstr( buff, sY ) )
				{
					if ( GetFirstBool( which+1, argc, argv ) )
					{
						mBodyDesc->flags |= NX_BF_FROZEN_ROT_Y;
					}
					else
					{
						mBodyDesc->flags &= ~NX_BF_FROZEN_ROT_Y;
					}
				}
				else if ( strstr( buff, sZ ) )
				{
					if ( GetFirstBool( which+1, argc, argv ) )
					{
						mBodyDesc->flags |= NX_BF_FROZEN_ROT_Z;
					}
					else
					{
						mBodyDesc->flags &= ~NX_BF_FROZEN_ROT_Z;
					}
				}
			}
			else if ( strstr( buff, sActorDesc_Flag_Kinematic ) )
			{
				if ( GetFirstBool( which+1, argc, argv ) )
				{
					mBodyDesc->flags |= NX_BF_KINEMATIC;
				}
				else
				{
					mBodyDesc->flags &= ~NX_BF_KINEMATIC;
				}
			}
			else if ( strstr( buff, sActorDesc_Flag_Visualization ) )
			{
				if ( GetFirstBool( which+1, argc, argv ) )
				{
					mBodyDesc->flags |= NX_BF_VISUALIZATION;
				}
				else
				{
					mBodyDesc->flags &= ~NX_BF_VISUALIZATION;
				}
			}
		}
	}
	
	void ObtainActorDesc_Shape_Default( const char *buff, int which, int argc, const char **argv )
	{
		if ( strstr( buff, sShape_LocalPose ) )
		{
			GetFirstMatrix( mCurrShape->localPose, which+1, argc, argv );			
		}
		else if ( strstr( buff, sShape_GroupsMask ) )
		{
#ifdef NX_SUPPORT_NEW_FILTERING
			int vals[4];
			GetIntVector( vals, 4, which+1, argc, argv );
			mCurrShape->groupsMask.bits0 = vals[0];
			mCurrShape->groupsMask.bits1 = vals[1];
			mCurrShape->groupsMask.bits2 = vals[2];
			mCurrShape->groupsMask.bits3 = vals[3];
#endif
		}
		else if ( strstr( buff, sGroup ) )
		{
			mCurrShape->group = GetFirstInt( which+1, argc, argv );
		}
		else if ( strstr( buff, sMaterialIndex ) )
		{
			mCurrShape->materialIndex = GetFirstInt( which+1, argc, argv );
		}
		else if ( strstr( buff, sMass ) )
		{
#if NX_SDK_VERSION_NUMBER >= 230
			mCurrShape->mass = GetFirstFloat( which+1, argc, argv );
#endif
		}
		else if ( strstr( buff, sDensity ) )
		{
#if NX_SDK_VERSION_NUMBER >= 230
			mCurrShape->density = GetFirstFloat( which+1, argc, argv );
#endif
		}
		else if ( strstr( buff, sShape_SkinWidth ) )
		{
			mCurrShape->skinWidth = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sShape_Flag_TriggerOnEnter ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_TRIGGER_ON_ENTER;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_TRIGGER_ON_ENTER;
			}
		}
		else if ( strstr( buff, sShape_Flag_TriggerOnLeave ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_TRIGGER_ON_LEAVE;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_TRIGGER_ON_LEAVE;
			}
		}
		else if ( strstr( buff, sShape_Flag_TriggerOnStay ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_TRIGGER_ON_STAY;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_TRIGGER_ON_STAY;
			}
		}
		else if ( strstr( buff, sShape_Flag_TriggerEnable ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_TRIGGER_ENABLE;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_TRIGGER_ENABLE;
			}
		}
		else if ( strstr( buff, sShape_Flag_Visualization ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_VISUALIZATION;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_VISUALIZATION;
			}
		}
		else if ( strstr( buff, sShape_Flag_DisableCollision ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_DISABLE_COLLISION;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_DISABLE_COLLISION;
			}
		}
		else if ( strstr( buff, sShape_Flag_FeatureIndices ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_FEATURE_INDICES;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_FEATURE_INDICES;
			}
		}
		else if ( strstr( buff, sShape_Flag_DisableRaycasting ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_DISABLE_RAYCASTING;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_DISABLE_RAYCASTING;
			}
		}
		else if ( strstr( buff, sShape_Flag_PointContactForce ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_POINT_CONTACT_FORCE;
			}
		}
		else if ( strstr( buff, sShape_Flag_DisableResponse ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_DISABLE_RESPONSE;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_DISABLE_RESPONSE;
			}
		}
		else if ( strstr( buff, sShape_Flag_FluidDrainInvert ) )
		{
/* OLD_FLUID_API
#if NX_USE_FLUID_API
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_FLUID_DRAIN_INVERT;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_FLUID_DRAIN_INVERT;
			}
#endif
*/
		}
		else if ( strstr( buff, sShape_Flag_FluidDrain ) )
		{
#if NX_USE_FLUID_API
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_FLUID_DRAIN;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_FLUID_DRAIN;
			}
#endif
		}
		else if ( strstr( buff, sShape_Flag_FluidDisableCollision ) )
		{
#if NX_USE_FLUID_API
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_FLUID_DISABLE_COLLISION;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_FLUID_DISABLE_COLLISION;
			}
#endif
		}
		else if ( strstr( buff, sShape_Flag_ActorReaction ) )
		{
/* OLD_FLUID_API
#if NX_USE_FLUID_API
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrShape->shapeFlags |= NX_SF_FLUID_ACTOR_REACTION;
			}
			else
			{
				mCurrShape->shapeFlags &= ~NX_SF_FLUID_ACTOR_REACTION;
			}
#endif
*/
		}
		else if ( strstr( buff, sShape_CCDSkeleton ) )
		{
			// hmm, what to do here...?
		}
		else if ( strstr( buff, sName ) )
		{
			mCurrShape->name = GetFirstString( which+1, argc, argv );
		}
	}

	void ObtainActorDesc_Shape_Plane( const char *buff, int which, int argc, const char **argv )
	{
		NxPlaneShapeDesc *desc = (NxPlaneShapeDesc *)mCurrShape;

		// here we just get its plane stuff
		if ( strstr( buff, sPlane ) )
		{
			float vals[4];
			GetFloatVector( vals, 4, which+1, argc, argv );
			desc->normal.set( vals[0], vals[1], vals[2] );
			desc->d = vals[3];
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
	}
	
	void ObtainActorDesc_Shape_Sphere( const char *buff, int which, int argc, const char **argv )
	{
		NxSphereShapeDesc *desc	= (NxSphereShapeDesc *)mCurrShape;
		if ( strstr( buff, sRadius ) )
		{
			desc->radius = GetFirstFloat( which+1, argc, argv );
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
	}
	
	void ObtainActorDesc_Shape_Box( const char *buff, int which, int argc, const char **argv )
	{
		NxBoxShapeDesc *desc = (NxBoxShapeDesc *)mCurrShape;

		if ( strstr( buff, sBoxDimensions ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			desc->dimensions.set( vals[0], vals[1], vals[2] );
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
	}

	void ObtainActorDesc_Shape_Capsule( const char *buff, int which, int argc, const char **argv )
	{
		NxCapsuleShapeDesc *desc = (NxCapsuleShapeDesc *)mCurrShape;

		if ( strstr( buff, sRadius ) )
		{
			desc->radius = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sHeight ) )
		{
			desc->height = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sFlags ) )
		{
			desc->flags = GetFirstHex( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sSweptShapeFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_SWEPT_SHAPE;
			}
			else
			{
				desc->flags &= ~NX_SWEPT_SHAPE;
			}
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
	}

	void ObtainActorDesc_Shape_Convex( const char *buff, int which, int argc, const char **argv )
	{
		NxConvexShapeDesc *desc	= (NxConvexShapeDesc *)mCurrShape;
		if ( strstr( buff, sScale ) )
		{
#ifdef NX_SUPPORT_CONVEX_SCALE
			desc->scale = GetFirstFloat( which+1, argc, argv );
#endif
		}
		else if ( strstr( buff, sMeshFlags ) )
		{
			desc->meshFlags = GetFirstInt( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sMeshSmoothSphereCollisionsFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->meshFlags |= NX_MESH_SMOOTH_SPHERE_COLLISIONS;
			}
			else
			{
				desc->meshFlags &= ~NX_MESH_SMOOTH_SPHERE_COLLISIONS;
			}
		}
		else if ( strstr( buff, sShape_MeshData ) )
		{
			// hmm, what do we do here - k, we get the name of its mesh!
			const char *meshName = GetFirstString( which+1, argc, argv );

			// now, based upon that, we determine its index!
			if ( HasDigitValue( meshName ) )
			{
				// k, get it!
				int index		= GetDigitValue_Int( meshName ) - 1;
				desc->meshData	= (NxConvexMesh *)index;
			}
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
	}

	void ObtainActorDesc_Shape_TriMesh( const char *buff, int which, int argc, const char **argv )
	{
		NxTriangleMeshShapeDesc	*desc =	(NxTriangleMeshShapeDesc *)mCurrShape;

		if ( strstr( buff, sMeshFlags ) )
		{
			desc->meshFlags = GetFirstInt( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sMeshSmoothSphereCollisionsFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->meshFlags |= NX_MESH_SMOOTH_SPHERE_COLLISIONS;
			}
			else
			{
				desc->meshFlags &= ~NX_MESH_SMOOTH_SPHERE_COLLISIONS;
			}
		}
		else if ( strstr( buff, sScale ) )
		{
#ifdef NX_SUPPORT_CONVEX_SCALE
			desc->scale = GetFirstFloat( which+1, argc, argv );
#endif
		}
		else if ( strstr( buff, sShape_MeshData ) )
		{
			// hmm, what do we do here - k, we get the name of its mesh!
			const char *meshName = GetFirstString( which+1, argc, argv );

			// now, based upon that, we determine its index!
			if ( HasDigitValue( meshName ) )
			{
				// k, get it!
				int index		= GetDigitValue_Int( meshName ) - 1;
				desc->meshData	= (NxTriangleMesh *)index;
			}
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
	}

	void ObtainActorDesc_Shape_WheelShape( const char *buff, int which, int argc, const char **argv )
	{
#if NX_SDK_VERSION_NUMBER >= 230
		NxWheelShapeDesc *desc = (NxWheelShapeDesc *) mCurrShape;
		static int func = 0;

		if ( strstr( buff, sRadius ) )
		{
			desc->radius = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sWheelParam_SuspensionTravel ) )
		{
			desc->suspensionTravel = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sWheelParam_MotorTorque ) )
		{
			desc->motorTorque = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sWheelParam_BrakeTorque ) )
		{
			desc->brakeTorque = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sWheelParam_SteerAngle ) )
		{
			desc->steerAngle = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sWheelParam_InverseWheelMass ) )
		{
			desc->inverseWheelMass = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sWheelParam_WheelFlags ) )
		{
			desc->wheelFlags = GetFirstInt( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sWfWheelAxisContactNormalFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->wheelFlags |= NX_WF_WHEEL_AXIS_CONTACT_NORMAL;
			}
			else
			{
				desc->wheelFlags &= ~NX_WF_WHEEL_AXIS_CONTACT_NORMAL;
			}
		}
		else if ( strstr( buff, sWfInputLatSlipVelocityFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->wheelFlags |= NX_WF_INPUT_LAT_SLIPVELOCITY;
			}
			else
			{
				desc->wheelFlags &= ~NX_WF_INPUT_LAT_SLIPVELOCITY;
			}
		}
		else if ( strstr( buff, sWfInputLngSlipVelocityFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->wheelFlags |= NX_WF_INPUT_LNG_SLIPVELOCITY;
			}
			else
			{
				desc->wheelFlags &= ~NX_WF_INPUT_LNG_SLIPVELOCITY;
			}
		}
		else if ( strstr( buff, sWfUnscaledSpringBehaviorFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->wheelFlags |= NX_WF_UNSCALED_SPRING_BEHAVIOR;
			}
			else
			{
				desc->wheelFlags &= ~NX_WF_UNSCALED_SPRING_BEHAVIOR;
			}
		}
		else if ( strstr( buff, sWfAxleSpeedOverrideFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->wheelFlags |= NX_WF_AXLE_SPEED_OVERRIDE;
			}
			else
			{
				desc->wheelFlags &= ~NX_WF_AXLE_SPEED_OVERRIDE;
			}
		}
		else if ( strstr( buff, sSpring_Spring ) )
		{
			desc->suspension.spring = GetFirstFloat( which+1, argc, argv );				
		}
		else if ( strstr( buff, sSpring_Damper ) )
		{
			desc->suspension.damper = GetFirstFloat( which+1, argc, argv );				
		}
		else if ( strstr( buff, sSpring_TargetValue ) )
		{
			desc->suspension.targetValue = GetFirstFloat( which+1, argc, argv );		
		}
		else if ( strstr( buff, sSpring ) )
		{
			// do nothing
		}
		else if ( strstr( buff, sWheelParam_LateralFunc ) )
		{
			func = 0;
		}
		else if ( strstr( buff, sWheelParam_LongitudinalFunc ) )
		{
			func = 1;		
		}
		else if ( strstr( buff, sWheelParam_TireFunc_ExtremumSlip ) )
		{
			float value = GetFirstFloat( which+1, argc, argv );
			if ( func == 0 )
			{
				desc->lateralTireForceFunction.extremumSlip = value;
			}
			else
			{
				desc->longitudalTireForceFunction.extremumSlip = value;
			}
		}
		else if ( strstr( buff, sWheelParam_TireFunc_ExtremumValue ) )
		{
			float value = GetFirstFloat( which+1, argc, argv );
			if ( func == 0 )
			{
				desc->lateralTireForceFunction.extremumValue = value;
			}
			else
			{
				desc->longitudalTireForceFunction.extremumValue = value;
			}		
		}
		else if ( strstr( buff, sWheelParam_TireFunc_AsumptoteSlip ) )
		{
			float value = GetFirstFloat( which+1, argc, argv );
			if ( func == 0 )
			{
				desc->lateralTireForceFunction.asymptoteSlip = value;
			}
			else
			{
				desc->longitudalTireForceFunction.asymptoteSlip = value;
			}		
		}
		else if ( strstr( buff, sWheelParam_TireFunc_AsymptoteValue ) )
		{
			float value = GetFirstFloat( which+1, argc, argv );
			if ( func == 0 )
			{
				desc->lateralTireForceFunction.asymptoteValue = value;
			}
			else
			{
				desc->longitudalTireForceFunction.asymptoteValue = value;
			}		
		}
		else if ( strstr( buff, sWheelParam_TireFunc_StiffnessFactor ) )
		{
			float value = GetFirstFloat( which+1, argc, argv );
			if ( func == 0 )
			{
				desc->lateralTireForceFunction.stiffnessFactor = value;
			}
			else
			{
				desc->longitudalTireForceFunction.stiffnessFactor = value;
			}		
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
#endif
	}

	//==================================================================================
	void ObtainActorDesc_Shape_HeightField( const char *buff, int which, int argc, const char **argv )
	{
#if NX_SDK_VERSION_NUMBER >= 240
		NxHeightFieldShapeDesc *desc = (NxHeightFieldShapeDesc *) mCurrShape;
		static int func = 0;

		if ( strstr( buff, sMeshSmoothSphereCollisionsFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->meshFlags |= NX_MESH_SMOOTH_SPHERE_COLLISIONS;
			}
			else
			{
				desc->meshFlags &= ~NX_MESH_SMOOTH_SPHERE_COLLISIONS;
			}
		}
		else if ( strstr( buff, sHeightField_Scale ) )
		{
#ifdef NX_SUPPORT_CONVEX_SCALE
			desc->scale = GetFirstFloat( which+1, argc, argv );
#endif
		}
		else if ( strstr( buff, sHeightField_HoleMaterial ) )
		{
			desc->holeMaterial = GetFirstUnsigned( which+1, argc, argv );
		}
		else if ( strstr( buff, sHeightField_MaterialIndexHighBits ) )
		{
			desc->materialIndexHighBits = GetFirstUnsigned( which+1, argc, argv );
		}
		else if ( strstr( buff, sHeightField_ColumnScale ) )
		{
			desc->columnScale = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sHeightField_RowScale ) )
		{
			desc->rowScale = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sHeightField_HeightScale ) )
		{
			desc->heightScale = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sHeightField_HeightFieldIndex ) )
		{
			desc->heightField = (NxHeightField *)GetFirstInt( which+1, argc, argv );
		}
		else
		{
			ObtainActorDesc_Shape_Default( buff, which, argc, argv );
		}
#endif
	}

	//==================================================================================
	const char *GetShapeType_Ascii( NxShapeType type )
	{
		const char *ret = "null";

		if ( type == NX_SHAPE_PLANE )
		{
			ret  = "NX_SHAPE_PLANE";
		}
		else if ( type == NX_SHAPE_SPHERE )
		{
			ret  = "NX_SHAPE_SPHERE";
		}
		else if ( type == NX_SHAPE_BOX )
		{
			ret  = "NX_SHAPE_BOX";
		}
		else if ( type == NX_SHAPE_CAPSULE )
		{
			ret  = "NX_SHAPE_CAPSULE";
		}
		else if ( type == NX_SHAPE_CONVEX )
		{
			ret  = "NX_SHAPE_CONVEX";
		}
#if NX_SDK_VERSION_NUMBER >= 230
		else if ( type == NX_SHAPE_WHEEL )
		{
			ret  = "NX_SHAPE_WHEEL";
		}
#endif
		else if ( type == NX_SHAPE_MESH )
		{
			ret  = "NX_SHAPE_MESH";
		}
#if NX_SDK_VERSION_NUMBER >= 240
		else if ( type == NX_SHAPE_HEIGHTFIELD )
		{
			ret = "NX_SHAPE_HEIGHTFIELD";
		}						
		else if ( type == NX_SHAPE_RAW_MESH )
		{
			ret = "NX_SHAPE_RAW_MESH";
		}
#endif
		else if ( type == NX_SHAPE_COMPOUND )
		{
			ret  = "NX_SHAPE_COMPOUND";
		}
		return ret;
	}

	//==================================================================================
	int GetShapeType( const char *val )
	{
		int ret = -1;

		if ( !stricmp( val, "NX_SHAPE_PLANE" ) )
		{
			ret  = (int)NX_SHAPE_PLANE;
		}
		else if ( !stricmp( val, "NX_SHAPE_SPHERE" ) )
		{
			ret  = (int)NX_SHAPE_SPHERE;
		}
		else if ( !stricmp( val, "NX_SHAPE_BOX" ) )
		{
			ret  = (int)NX_SHAPE_BOX;
		}
		else if ( !stricmp( val, "NX_SHAPE_CAPSULE" ) )
		{
			ret  = (int)NX_SHAPE_CAPSULE;
		}
		else if ( !stricmp( val, "NX_SHAPE_CONVEX" ) )
		{
			ret  = (int)NX_SHAPE_CONVEX;
		}
#if NX_SDK_VERSION_NUMBER >= 230
		else if ( !stricmp( val, "NX_SHAPE_WHEEL" ) )
		{
			ret  = (int)NX_SHAPE_WHEEL;
		}
#endif
		else if ( !stricmp( val, "NX_SHAPE_MESH" ) )
		{
			ret  = (int)NX_SHAPE_MESH;
		}
#if NX_SDK_VERSION_NUMBER >= 240
		else if ( !stricmp( val, "NX_SHAPE_HEIGHTFIELD" ) )
		{
			ret = (int)NX_SHAPE_HEIGHTFIELD;
		}						
		else if ( !stricmp( val, "NX_SHAPE_RAW_MESH" ) )
		{
			ret = (int)NX_SHAPE_RAW_MESH;
		}
#endif
		else if ( !stricmp( val, "NX_SHAPE_COMPOUND" ) )
		{
			ret  = (int)NX_SHAPE_COMPOUND;
		}
		return ret;
	}

	//==================================================================================
	int GetShapeType_WithArgs( int offset, int argc, const char **argv )
	{
		int ret = -1;

		bool done = false;
		for ( int i = offset; !done && (i < argc); ++i )
		{
			ret = GetShapeType( argv[i] );
			if ( ret != -1 )
			{
				done = true;
			}
		}				

		return ret;
	}

	void ObtainActorDesc_Shape( const char *buff, int which, int argc, const char **argv )
	{
		if ( strstr( buff, sType ) )
		{
			NxShapeType type = (NxShapeType)GetFirstInt( which+1, argc, argv );

			if ( mCurrShape && ( type != mCurrShape->getType() ) )
			{
				char tmp[256];
				sprintf( tmp, "Note that 'type = ' specified %s, while 'NxShape=' specified %s ... continuing anyways with type of %s",
					GetShapeType_Ascii(mCurrShape->getType()), GetShapeType_Ascii(type), GetShapeType_Ascii(mCurrShape->getType()) );
				OutputStatusMsg( tmp );
			}
		}
		else if ( strstr( buff, sShape_NxShape ) )
		{
			// k, what type is this - will get us to determine what we are doing?
			int ret = (NxShapeType)GetShapeType_WithArgs( which+1, argc, argv );
			if ( ret != -1 )
			{
				NxShapeType type = (NxShapeType)ret;
				mCurrShape		 = mContainer->CreateShapeFromType( type );

				if ( mCurrShape )
				{
					mCurrShape->setToDefault();
					mCurrActor->shapes.push_back( mCurrShape );
				}
			}
		}
		else if ( mCurrShape )
		{
			// k, based upon its type, get its info
			switch( mCurrShape->getType() )
			{
				case NX_SHAPE_PLANE:
				{
					ObtainActorDesc_Shape_Plane( buff, which, argc, argv );
				}
				break;

				case NX_SHAPE_SPHERE:
				{
					ObtainActorDesc_Shape_Sphere( buff, which, argc, argv );
				}
				break;

				case NX_SHAPE_BOX:
				{
					ObtainActorDesc_Shape_Box( buff, which, argc, argv );
				}
				break;

				case NX_SHAPE_CAPSULE:
				{
					ObtainActorDesc_Shape_Capsule( buff, which, argc, argv );
				}
				break;

				case NX_SHAPE_CONVEX:
				{
					ObtainActorDesc_Shape_Convex( buff, which, argc, argv );
				}
				break;

				case NX_SHAPE_MESH:
				{
					ObtainActorDesc_Shape_TriMesh( buff, which, argc, argv );
				}
				break;

#if NX_SDK_VERSION_NUMBER >= 230
				case NX_SHAPE_WHEEL:
				{
					ObtainActorDesc_Shape_WheelShape( buff, which, argc, argv );
				}
				break;
#endif

#if NX_SDK_VERSION_NUMBER >= 240
				case NX_SHAPE_HEIGHTFIELD:
				{
					ObtainActorDesc_Shape_HeightField( buff, which, argc, argv );
				}
				break;
			
				case NX_SHAPE_RAW_MESH:
					assert(0); // this should not happen
				break;
#endif

				case NX_SHAPE_COMPOUND:
					assert(	0 );
				break;

				default:
					assert(0); //!!???
				break;
			}

		}
	}

	bool ObtainActorDesc( const char *buff, int which, int argc, const char **argv )
	{
		bool found = true;

		// k, make sure of the buff pntrs
		if ( buff )
		{
			if ( strstr( buff, sNxActor ) )
			{
				mActorParseType = APT_ACTOR_PROPS;

				if ( mCoreScene )
				{
					if ( mCurrActor )
					{
						mCurrActor = 0;
					}
					if ( !mCurrActor )
					{
						mCurrActor = new NxActorDesc;
						assert( mCurrActor );
						if ( mCurrActor )
						{
							mCurrActor->setToDefault();
							mCoreScene->mActors.push_back( mCurrActor );
						}
					}
				}
			}
			else if ( mCurrActor )
			{
				if ( strstr( buff, sDensity ) && ( mActorParseType == APT_BODY_INFO ) )
				{
					mActorParseType = APT_ACTOR_PROPS;
				}
				else if ( strstr( buff, sBody ) )
				{
					mActorParseType = APT_BODY_INFO;
				}
				else if ( strstr( buff, sNumShapes ) )
				{
					if ( GetFirstInt( which+1, argc, argv ) )
					{
						mActorParseType = APT_SHAPE_INFO;
					}
				}

				switch( mActorParseType )
				{
					case APT_ACTOR_PROPS:
					{
						ObtainActorDesc_Props( buff, which+1, argc, argv );
					}
					break;

					case APT_BODY_INFO:
					{
						ObtainActorDesc_Body( buff, which+1, argc, argv );
					}
					break;

					case APT_SHAPE_INFO:
					{
						ObtainActorDesc_Shape( buff, which+1, argc, argv );
					}
					break;
				}
			}
		}

		return found;
	}

	bool ObtainPairFlagDesc( const char *buff, int which, int argc, const char **argv )
	{
		bool found = true;

		// k, make sure of the buff pntrs
		if ( buff && mCoreScene )
		{
			if ( !mCoreScene->mPairFlagArray && mCoreScene->mPairCount )
			{
				mCoreScene->mPairFlagArray = new PairFlagCapsule[mCoreScene->mPairCount];
				if ( mCoreScene->mPairFlagArray )
				{
					memset( mCoreScene->mPairFlagArray, 0, sizeof( PairFlagCapsule ) * mCoreScene->mPairCount );
				}
				mWhichPairFlag = 0;
			}
			
			if ( mCoreScene->mPairFlagArray )
			{
				// k, check them
				if ( strstr( buff, sNxPairFlag ) )
				{
					mWhichPairFlag = GetDigitValue_Int( buff ) - 1;
				}
				// shoot me
				else if ( strstr( buff, sPairFlag ) )
				{
					mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag = GetFirstHex( which+1, argc, argv );
				}
				else if ( strstr( buff, sPairIsActorPairFlag ) )
				{
					bool ok = GetFirstBool( which+1, argc, argv );
					if ( ok )
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag |= 0x80000000;
					}
					else
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag &= ~0x80000000;
					}
				}
				else if ( strstr( buff, sPairIgnorePairFlag ) )
				{
					bool ok = GetFirstBool( which+1, argc, argv );
					if ( ok )
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag |= NX_IGNORE_PAIR;
					}
					else
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag &= ~NX_IGNORE_PAIR;
					}
				}
				else if ( strstr( buff, sPairNotifyOnTouchFlag ) )
				{
					bool ok = GetFirstBool( which+1, argc, argv );
					if ( ok )
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag |= NX_NOTIFY_ON_TOUCH;
					}
					else
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mFlag &= ~NX_NOTIFY_ON_TOUCH;
					}
				}
				else if ( strstr( buff, sActorIndex ) )
				{
					int index = GetDigitValue_Int( buff );
					if ( index == 1 )
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mActorIndex1 = GetFirstInt( which+1, argc, argv );
					}
					else
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mActorIndex2 = GetFirstInt( which+1, argc, argv );
					}
				}
				else if ( strstr( buff, sShapeIndex ) )
				{
					int index = GetDigitValue_Int( buff );
					if ( index == 1 )
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mShapeIndex1 = GetFirstInt( which+1, argc, argv );
					}
					else
					{
						mCoreScene->mPairFlagArray[mWhichPairFlag].mShapeIndex2 = GetFirstInt( which+1, argc, argv );
					}
				}
				else
				{
					found = false;
				}
			}
		}

		return found;
	}

	void ObtainNormalJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		static int snNumLimitPlanes = 0;
		static PlaneInfo *spiPlaneInfo = 0;

		if ( strstr( buff, sPlaneLimitPointOnActor2 ) )
		{
			mCurrJoint->mOnActor2 = GetFirstBool( which+1, argc, argv );
		}
		else if ( strstr( buff, sActor ) )
		{
			// reset this guy!
			snNumLimitPlanes = 0;
			spiPlaneInfo = 0;

			if ( GetDigitValue_Int( buff ) == 1 )
			{
				mCurrJoint->mDesc->actor[0] = (NxActor *)(GetFirstInt( which+1, argc, argv ) - 1);
			}
			else
			{
				mCurrJoint->mDesc->actor[1] = (NxActor *)(GetFirstInt( which+1, argc, argv ) - 1);
			}
		}
		else if ( strstr( buff, sLocalNormal ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			if ( GetDigitValue_Int( buff ) == 1 )
			{
				mCurrJoint->mDesc->localNormal[0].set( vals[0], vals[1], vals[2] );
			}
			else
			{
				mCurrJoint->mDesc->localNormal[1].set( vals[0], vals[1], vals[2] );
			}
		}
		else if ( strstr( buff, sLocalAxis ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			if ( GetDigitValue_Int( buff ) == 1 )
			{
				mCurrJoint->mDesc->localAxis[0].set( vals[0], vals[1], vals[2] );
			}
			else
			{
				mCurrJoint->mDesc->localAxis[1].set( vals[0], vals[1], vals[2] );
			}
		}
		else if ( strstr( buff, sLocalAnchor ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			if ( GetDigitValue_Int( buff ) == 1 )
			{
				mCurrJoint->mDesc->localAnchor[0].set( vals[0], vals[1], vals[2] );
			}
			else
			{
				mCurrJoint->mDesc->localAnchor[1].set( vals[0], vals[1], vals[2] );
			}
		}
		else if ( strstr( buff, sMaxForce ) )
		{
			mCurrJoint->mDesc->maxForce = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sMaxTorque ) )
		{
			mCurrJoint->mDesc->maxTorque = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sName ) )
		{
			mCurrJoint->mDesc->name = GetFirstString( which+1, argc, argv );
		}
		else if ( strstr( buff, sJointFlag_CollisionEnabled ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrJoint->mDesc->jointFlags |= NX_JF_COLLISION_ENABLED;
			}
			else
			{
				mCurrJoint->mDesc->jointFlags &= ~NX_JF_COLLISION_ENABLED;
			}
		}
		else if ( strstr( buff, sJointFlag_Visualization ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				mCurrJoint->mDesc->jointFlags |= NX_JF_VISUALIZATION;
			}
			else
			{
				mCurrJoint->mDesc->jointFlags &= ~NX_JF_VISUALIZATION;
			}
		}
		else if ( strstr( buff, sNumLimitPlanes ) )
		{
			snNumLimitPlanes = GetFirstInt( which+1, argc, argv );
		}
		else if ( strstr( buff, sPlaneLimitPoint ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			mCurrJoint->mPlaneLimitPoint.set( vals[0], vals[1], vals[2] );
		}
		else if ( strstr( buff, sLimitPlane ) )
		{
			// k, time to allocate a limit plane!
			spiPlaneInfo = new PlaneInfo;
			if ( spiPlaneInfo )
			{
				mCurrJoint->mPlaneInfo.push_back( spiPlaneInfo );
			}
		}
		else if ( spiPlaneInfo )
		{
			if ( strstr( buff, sPlaneNormal ) )
			{
				float vals[3];
				GetFloatVector( vals, 3, which+1, argc, argv );
				spiPlaneInfo->mPlaneNormal.set( vals[0], vals[1], vals[2] );
			}
			else if ( strstr( buff, sPlaneD ) )
			{
				spiPlaneInfo->mPlaneD = GetFirstFloat( which+1, argc, argv );
			}
			else if ( strstr( buff, sWorldLimitPt ) )
			{
				float vals[3];
				GetFloatVector( vals, 3, which+1, argc, argv );
				spiPlaneInfo->mWorldLimitPt.set( vals[0], vals[1], vals[2] );
			}
		}
	}

	void ObtainPrismaticJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		ObtainNormalJointDesc( buff, which, argc, argv );
	}

	void ObtainSphericalJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		NxSphericalJointDesc *desc = (NxSphericalJointDesc *)mCurrJoint->mDesc;

		if ( strstr( buff, sTwistSpring_Spring ) )
		{
			desc->twistSpring.spring = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sTwistSpring_Damper ) )
		{
			desc->twistSpring.damper = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sTwistSpring_TargetValue ) )
		{
			desc->twistSpring.targetValue = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSwingSpring_Spring ) )
		{
			desc->swingSpring.spring = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSwingSpring_Damper ) )
		{
			desc->swingSpring.damper = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSwingSpring_TargetValue ) )
		{
			desc->swingSpring.targetValue = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sJointSpring_Spring ) )
		{
			desc->jointSpring.spring = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sJointSpring_Damper ) )
		{
			desc->jointSpring.damper = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sJointSpring_TargetValue ) )
		{
			desc->jointSpring.targetValue = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sProjectionDist ) )
		{
			desc->projectionDistance = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_Low_Hardness ) )
		{
			desc->twistLimit.low.hardness = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_Low_Restitution ) )
		{
			desc->twistLimit.low.restitution = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_Low_Value ) )
		{
			desc->twistLimit.low.value = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_High_Hardness ) )
		{
			desc->twistLimit.high.hardness = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_High_Restitution ) )
		{
			desc->twistLimit.high.restitution = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_High_Value ) )
		{
			desc->twistLimit.high.value = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSwingLimit_Hardness ) )
		{
			desc->swingLimit.hardness = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSwingLimit_Restitution ) )
		{
			desc->swingLimit.restitution = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSwingLimit_Value ) )
		{
			desc->swingLimit.value = GetFirstFloat( which+1, argc, argv );
		}
		// SRM : had a bug where I mistakenly said "desc" instead of flags, so that is why this is in here!
		else if ( strstr( buff, sFlags ) || strstr( buff, sDesc ) )
		{
			desc->flags = GetFirstHex( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sSjfTwistLimitEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_SJF_TWIST_LIMIT_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_SJF_TWIST_LIMIT_ENABLED;
			}
		}
		else if ( strstr( buff, sSjfSwingLimitEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_SJF_SWING_LIMIT_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_SJF_SWING_LIMIT_ENABLED;
			}
		}
		else if ( strstr( buff, sSjfTwistSpringEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_SJF_TWIST_SPRING_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_SJF_TWIST_SPRING_ENABLED;
			}
		}
		else if ( strstr( buff, sSjfSwingSpringEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_SJF_SWING_SPRING_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_SJF_SWING_SPRING_ENABLED;
			}
		}
		else if ( strstr( buff, sSjfSpringEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_SJF_JOINT_SPRING_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_SJF_JOINT_SPRING_ENABLED;
			}
		}
		else if ( strstr( buff, sSwingAxis ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			desc->swingAxis.set( vals[0], vals[1], vals[2] );
		}
		else if ( strstr( buff, sProjectionMode ) )
		{
			desc->projectionMode = NX_JPM_NONE;
			const char *type = GetFirstString( which+1, argc, argv );
			if ( !stricmp( type, "NX_JPM_POINT_MINDIST" ) )
			{
				desc->projectionMode = NX_JPM_POINT_MINDIST;
			}
		}
		else
		{
			ObtainNormalJointDesc( buff, which, argc, argv );
		}
	}

	void ObtainCylindricalJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		ObtainNormalJointDesc( buff, which, argc, argv );
	}

	void ObtainRevoluteJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		NxRevoluteJointDesc *desc = (NxRevoluteJointDesc *)mCurrJoint->mDesc;

		if ( strstr( buff, sLimit_Low_Hardness ) )
		{
			desc->limit.low.hardness = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_Low_Restitution ) )
		{
			desc->limit.low.restitution = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_Low_Value ) )
		{
			desc->limit.low.value = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_High_Hardness ) )
		{
			desc->limit.high.hardness = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_High_Restitution ) )
		{
			desc->limit.high.restitution = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sLimit_High_Value ) )
		{
			desc->limit.high.value = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sMotor_FreeSpin ) )
		{
			desc->motor.freeSpin = GetFirstBool( which+1, argc, argv );
		}
		else if ( strstr( buff, sMotor_MaxForce ) )
		{
			desc->motor.maxForce = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sMotor_VelTarget ) )
		{
			desc->motor.velTarget = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSpring_Spring ) )
		{
			desc->spring.spring = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSpring_Damper ) )
		{
			desc->spring.damper = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSpring_TargetValue ) )
		{
			desc->spring.targetValue = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sProjectionDistance ) )
		{
			desc->projectionDistance = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sProjectionAngle ) )
		{
			desc->projectionAngle = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sFlags ) )
		{
			desc->flags = GetFirstHex( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sRjfLimitEnabledFlag ) ) 
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_RJF_LIMIT_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_RJF_LIMIT_ENABLED;
			}
		}
		else if ( strstr( buff, sRjfMotorEnabledFlag ) ) 
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_RJF_MOTOR_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_RJF_MOTOR_ENABLED;
			}	
		}
		else if ( strstr( buff, sRjfSpringEnabledFlag ) ) 
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_RJF_SPRING_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_RJF_SPRING_ENABLED;
			}
		}
		else if ( strstr( buff, sProjectionMode ) )
		{
			desc->projectionMode = NX_JPM_NONE;
			const char *type = GetFirstString( which+1, argc, argv );
			if ( !stricmp( type, "NX_JPM_POINT_MINDIST" ) )
			{
				desc->projectionMode = NX_JPM_POINT_MINDIST;
			}
		}
		else
		{
			ObtainNormalJointDesc( buff, which, argc, argv );
		}
	}

	void ObtainPointOnLineJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		ObtainNormalJointDesc( buff, which, argc, argv );
	}

	void ObtainPointInPlaneJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		ObtainNormalJointDesc( buff, which, argc, argv );
	}

	void ObtainDistanceJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		NxDistanceJointDesc *desc = (NxDistanceJointDesc *)mCurrJoint->mDesc;

		if ( strstr( buff, sMinDistance ) )
		{
			desc->minDistance = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sMaxDistance ) )
		{
			desc->maxDistance = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sFlags ) )
		{
			desc->flags = GetFirstHex( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sDjfMaxDistanceEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_DJF_MAX_DISTANCE_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_DJF_MAX_DISTANCE_ENABLED;
			}
		}
		else if ( strstr( buff, sDjfMinDistanceEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_DJF_MIN_DISTANCE_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_DJF_MIN_DISTANCE_ENABLED;
			}
		}
		else if ( strstr( buff, sDjfSpringEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_DJF_SPRING_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_DJF_SPRING_ENABLED;
			}
		}
		else if ( strstr( buff, sSpring_Spring ) )
		{
			desc->spring.spring = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSpring_Damper ) )
		{
			desc->spring.damper = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sSpring_TargetValue ) )
		{
			desc->spring.targetValue = GetFirstFloat( which+1, argc, argv );
		}
		else
		{
			ObtainNormalJointDesc( buff, which, argc, argv );
		}
	}

	void ObtainPulleyJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		NxPulleyJointDesc *desc = (NxPulleyJointDesc *)mCurrJoint->mDesc;

		if ( strstr( buff, sDistance ) )
		{
			desc->distance = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sStiffness ) )
		{
			desc->stiffness = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sRatio ) )
		{
			desc->ratio = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sFlags ) )
		{
			desc->flags = GetFirstHex( which+1, argc, argv );
		}
		// shoot me
		else if ( strstr( buff, sPfjIsRigidFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_PJF_IS_RIGID;
			}
			else
			{
				desc->flags &= ~NX_PJF_IS_RIGID;
			}
		}
		else if ( strstr( buff, sPfjMotorEnabledFlag ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_PJF_MOTOR_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_PJF_MOTOR_ENABLED;
			}
		}
		else if ( strstr( buff, sMotor_FreeSpin ) )
		{
			desc->motor.freeSpin = GetFirstBool( which+1, argc, argv );
		}
		else if ( strstr( buff, sMotor_MaxForce ) )
		{
			desc->motor.maxForce = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sMotor_VelTarget ) )
		{
			desc->motor.velTarget = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sDescPulley ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );

			if ( GetDigitValue_Int( buff ) == 1 )
			{
				desc->pulley[0].set( vals[0], vals[1], vals[2] );
			}
			else
			{
				desc->pulley[1].set( vals[0], vals[1], vals[2] );
			}
		}
		else
		{
			ObtainNormalJointDesc( buff, which, argc, argv );
		}
	}

	void ObtainFixedJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		ObtainNormalJointDesc( buff, which, argc, argv );
	}

	void ObtainD6JointDesc( const char *buff, int which, int argc, const char **argv )
	{
		static D6JointParseType sjptParseType = D6JPT_LINEAR;

		NxD6JointDesc *desc = (NxD6JointDesc *)mCurrJoint->mDesc;

		if ( strstr( buff, sXMotion ) )
		{
			desc->xMotion = (NxD6JointMotion)GetFirstInt( which+1, argc, argv );				
		}
		else if ( strstr( buff, sYMotion ) )
		{
			desc->yMotion = (NxD6JointMotion)GetFirstInt( which+1, argc, argv );				
		}
		else if ( strstr( buff, sZMotion ) )
		{
			desc->zMotion = (NxD6JointMotion)GetFirstInt( which+1, argc, argv );				
		}
		else if ( strstr( buff, sSwing1Motion ) )
		{
			desc->swing1Motion = (NxD6JointMotion)GetFirstInt( which+1, argc, argv );				
		}
		else if ( strstr( buff, sSwing2Motion ) )
		{
			desc->swing2Motion = (NxD6JointMotion)GetFirstInt( which+1, argc, argv );				
		}
		else if ( strstr( buff, sTwistMotion ) )
		{
			desc->twistMotion = (NxD6JointMotion)GetFirstInt( which+1, argc, argv );				
		}
		else if ( strstr( buff, sLinearLimit ) )
		{
			sjptParseType = D6JPT_LINEAR;
		}
		else if ( strstr( buff, sSwing1Limit ) )
		{
			sjptParseType = D6JPT_SWING1;
		}
		else if ( strstr( buff, sSwing2Limit ) )
		{
			sjptParseType = D6JPT_SWING2;
		}
		else if ( strstr( buff, sTwistLimit ) )
		{
		}
		else if ( strstr( buff, sLow ) )
		{
			sjptParseType = D6JPT_TWIST_LOW;
		}
		else if ( strstr( buff, sHigh ) )
		{
			sjptParseType = D6JPT_TWIST_HIGH;
		}
		else if ( strstr( buff, sXDrive ) )
		{
			sjptParseType = D6JPT_XDRIVE;
		}
		else if ( strstr( buff, sYDrive ) )
		{
			sjptParseType = D6JPT_YDRIVE;
		}
		else if ( strstr( buff, sZDrive ) )
		{
			sjptParseType = D6JPT_ZDRIVE;
		}
		else if ( strstr( buff, sSwingDrive ) )
		{
			sjptParseType = D6JPT_SWINGDRIVE;
		}
		else if ( strstr( buff, sTwistDrive ) )
		{
			sjptParseType = D6JPT_TWISTDRIVE;
		}
		else if ( strstr( buff, sSlerpDrive ) )
		{
			sjptParseType = D6JPT_SLERPDRIVE;
		}
		else if ( strstr( buff, sValues ) )
		{
			float val = GetFirstFloat( which+1, argc, argv );

			switch( sjptParseType )
			{
				case D6JPT_LINEAR:
					desc->linearLimit.value = val;
				break;
				case D6JPT_SWING1:
					desc->swing1Limit.value = val;
				break;
				case D6JPT_SWING2:
					desc->swing2Limit.value = val;
				break;
				case D6JPT_TWIST_LOW:
					desc->twistLimit.low.value = val;
				break;
				case D6JPT_TWIST_HIGH:
					desc->twistLimit.high.value = val;
				break;
			}
		}
		else if ( strstr( buff, sRestitution ) )
		{
			float val = GetFirstFloat( which+1, argc, argv );

			switch( sjptParseType )
			{
				case D6JPT_LINEAR:
					desc->linearLimit.restitution = val;
				break;
				case D6JPT_SWING1:
					desc->swing1Limit.restitution = val;
				break;
				case D6JPT_SWING2:
					desc->swing2Limit.restitution = val;
				break;
				case D6JPT_TWIST_LOW:
					desc->twistLimit.low.restitution = val;
				break;
				case D6JPT_TWIST_HIGH:
					desc->twistLimit.high.restitution = val;
				break;
			}
		}
		else if ( strstr( buff, sSpring ) )
		{
			float val = GetFirstFloat( which+1, argc, argv );

			switch( sjptParseType )
			{
				case D6JPT_LINEAR:
					desc->linearLimit.spring = val;
				break;
				case D6JPT_SWING1:
					desc->swing1Limit.spring = val;
				break;
				case D6JPT_SWING2:
					desc->swing2Limit.spring = val;
				break;
				case D6JPT_TWIST_LOW:
					desc->twistLimit.low.spring = val;
				break;
				case D6JPT_TWIST_HIGH:
					desc->twistLimit.high.spring = val;
				break;
				case D6JPT_XDRIVE:
					desc->xDrive.spring = val;
				break;
				case D6JPT_YDRIVE:
					desc->yDrive.spring = val;
				break;
				case D6JPT_ZDRIVE:
					desc->zDrive.spring = val;
				break;
				case D6JPT_SWINGDRIVE:
					desc->swingDrive.spring = val;
				break;
				case D6JPT_TWISTDRIVE:
					desc->twistDrive.spring = val;
				break;
				case D6JPT_SLERPDRIVE:
					desc->slerpDrive.spring = val;
				break;
			}
		}
		else if ( strstr( buff, sDamping ) )
		{
			float val = GetFirstFloat( which+1, argc, argv );

			switch( sjptParseType )
			{
				case D6JPT_LINEAR:
					desc->linearLimit.damping = val;
				break;
				case D6JPT_SWING1:
					desc->swing1Limit.damping = val;
				break;
				case D6JPT_SWING2:
					desc->swing2Limit.damping = val;
				break;
				case D6JPT_TWIST_LOW:
					desc->twistLimit.low.damping = val;
				break;
				case D6JPT_TWIST_HIGH:
					desc->twistLimit.high.damping = val;
				break;
				case D6JPT_XDRIVE:
					desc->xDrive.damping = val;
				break;
				case D6JPT_YDRIVE:
					desc->yDrive.damping = val;
				break;
				case D6JPT_ZDRIVE:
					desc->zDrive.damping = val;
				break;
				case D6JPT_SWINGDRIVE:
					desc->swingDrive.damping = val;
				break;
				case D6JPT_TWISTDRIVE:
					desc->twistDrive.damping = val;
				break;
				case D6JPT_SLERPDRIVE:
					desc->slerpDrive.damping = val;
				break;
			}
		}
		else if ( strstr( buff, sForceLimit ) )
		{
			float val = GetFirstFloat( which+1, argc, argv );

			switch( sjptParseType )
			{
				case D6JPT_XDRIVE:
					desc->xDrive.forceLimit = val;
				break;
				case D6JPT_YDRIVE:
					desc->yDrive.forceLimit = val;
				break;
				case D6JPT_ZDRIVE:
					desc->zDrive.forceLimit = val;
				break;
				case D6JPT_SWINGDRIVE:
					desc->swingDrive.forceLimit = val;
				break;
				case D6JPT_TWISTDRIVE:
					desc->twistDrive.forceLimit = val;
				break;
				case D6JPT_SLERPDRIVE:
					desc->slerpDrive.forceLimit = val;
				break;
			}
		}
		else if ( strstr( buff, sNxD6Joint_Flag_Drive_Position ) )
		{
			// k, is flag set?
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				switch( sjptParseType )
				{
					case D6JPT_XDRIVE:
						desc->xDrive.driveType.bitField |= NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_YDRIVE:
						desc->yDrive.driveType.bitField |= NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_ZDRIVE:
						desc->zDrive.driveType.bitField |= NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_SWINGDRIVE:
						desc->swingDrive.driveType.bitField |= NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_TWISTDRIVE:
						desc->twistDrive.driveType.bitField |= NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_SLERPDRIVE:
						desc->slerpDrive.driveType.bitField |= NX_D6JOINT_DRIVE_POSITION;
					break;
				}
			}
			else
			{
				switch( sjptParseType )
				{
					case D6JPT_XDRIVE:
						desc->xDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_YDRIVE:
						desc->yDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_ZDRIVE:
						desc->zDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_SWINGDRIVE:
						desc->swingDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_TWISTDRIVE:
						desc->twistDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_POSITION;
					break;
					case D6JPT_SLERPDRIVE:
						desc->slerpDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_POSITION;
					break;
				}
			}
		}
		else if ( strstr( buff, sNxD6Joint_Flag_Drive_Velocity ) )
		{
			// k, is flag set?
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				switch( sjptParseType )
				{
					case D6JPT_XDRIVE:
						desc->xDrive.driveType.bitField |= NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_YDRIVE:
						desc->yDrive.driveType.bitField |= NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_ZDRIVE:
						desc->zDrive.driveType.bitField |= NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_SWINGDRIVE:
						desc->swingDrive.driveType.bitField |= NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_TWISTDRIVE:
						desc->twistDrive.driveType.bitField |= NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_SLERPDRIVE:
						desc->slerpDrive.driveType.bitField |= NX_D6JOINT_DRIVE_VELOCITY;
					break;
				}
			}
			else
			{
				switch( sjptParseType )
				{
					case D6JPT_XDRIVE:
						desc->xDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_YDRIVE:
						desc->yDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_ZDRIVE:
						desc->zDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_SWINGDRIVE:
						desc->swingDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_TWISTDRIVE:
						desc->twistDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_VELOCITY;
					break;
					case D6JPT_SLERPDRIVE:
						desc->slerpDrive.driveType.bitField &= ~NX_D6JOINT_DRIVE_VELOCITY;
					break;
				}
			}
		}
		else if ( strstr( buff, sDrivePosition ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			desc->drivePosition.set( vals[0], vals[1], vals[2] );
		}
		else if ( strstr( buff, sDriveOrientation ) )
		{
			float vals[4];
			GetFloatVector( vals, 4, which+1, argc, argv );
			desc->driveOrientation.setXYZW( vals[0], vals[1], vals[2], vals[3] );
		}
		else if ( strstr( buff, sDriveLinearVelocity ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			desc->driveLinearVelocity.set( vals[0], vals[1], vals[2] );
		}
		else if ( strstr( buff, sDriveAngularVelocity ) )
		{
			float vals[3];
			GetFloatVector( vals, 3, which+1, argc, argv );
			desc->driveAngularVelocity.set( vals[0], vals[1], vals[2] );
		}
		else if ( strstr( buff, sProjectionMode ) )
		{
			desc->projectionMode = (NxJointProjectionMode)GetFirstInt( which+1, argc, argv );
		}
		else if ( strstr( buff, sProjectionDistance ) )
		{
			desc->projectionDistance = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sProjectionAngle ) )
		{
			desc->projectionAngle = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sGearRatio ) )
		{
			desc->gearRatio = GetFirstFloat( which+1, argc, argv );
		}
		else if ( strstr( buff, sD6_Flag_SlerpDrive ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_D6JOINT_SLERP_DRIVE;
			}
			else
			{
				desc->flags &= ~NX_D6JOINT_SLERP_DRIVE;
			}
		}
		else if ( strstr( buff, sD6_Flag_GearEnabled ) )
		{
			if ( GetFirstBool( which+1, argc, argv ) )
			{
				desc->flags |= NX_D6JOINT_GEAR_ENABLED;
			}
			else
			{
				desc->flags &= ~NX_D6JOINT_GEAR_ENABLED;
			}
		}
		else
		{
			ObtainNormalJointDesc( buff, which, argc, argv );
		}
	}

	bool ObtainJointDesc( const char *buff, int which, int argc, const char **argv )
	{
		bool found = true;

		// k, make sure of the buff pntrs
		if ( buff && mCoreScene )
		{
			// k, this should be fun...
			if ( strstr( buff, sJoint_NxJointDesc ) ||
				 strstr( buff, sPrismaticJointDesc ) ||
				 strstr( buff, sRevoluteJointDesc ) ||
				 strstr( buff, sCylindricalJointDesc ) ||
				 strstr( buff, sSphericalJointDesc ) ||
				 strstr( buff, sPointOnLineJointDesc ) ||
				 strstr( buff, sPointInPlaneJointDesc ) ||			 
				 strstr( buff, sDistanceJointDesc ) ||			 
				 strstr( buff, sPulleyJointDesc ) ||			 
				 strstr( buff, sFixedJointDesc ) ||			 
				 strstr( buff, sNxD6JointDesc ) ||			 
				 strstr( buff, sNxJoint ) )
			{
				mCurrJoint = 0;
			}
			else if ( strstr( buff, sType ) )
			{
				mCurrJoint = new JointDescription;
				assert( mCurrJoint );

				if ( mCurrJoint )
				{
					mCoreScene->mJointArray.push_back( mCurrJoint );

					// cool, we know what type of joint we are!
					const char *buff = GetFirstString( which+1, argc, argv );
					char type[128];
					sprintf( type, buff );
					strlwrPortable( type );

					if ( strstr( type, sD6Joint ) )
					{
						mCurrJoint->mDesc = new NxD6JointDesc;
					}
					else if ( strstr( type, sPrismatic ) )
					{
						mCurrJoint->mDesc = new NxPrismaticJointDesc;
					}
					else if ( strstr( type, sRevolute ) )
					{
						mCurrJoint->mDesc = new NxRevoluteJointDesc;
					}
					else if ( strstr( type, sCylindrical ) )
					{
						mCurrJoint->mDesc = new NxCylindricalJointDesc;
					}
					else if ( strstr( type, sSpherical ) )
					{
						mCurrJoint->mDesc = new NxSphericalJointDesc;
					}
					else if ( strstr( type, sPointOnLine ) )
					{
						mCurrJoint->mDesc = new NxPointOnLineJointDesc;
					}
					else if ( strstr( type, sPointInPlane ) )
					{
						mCurrJoint->mDesc = new NxPointInPlaneJointDesc;
					}
					else if ( strstr( type, sDistanceJoint ) )
					{
						mCurrJoint->mDesc = new NxDistanceJointDesc;
					}
					else if ( strstr( type, sPulleyJoint ) )
					{
						mCurrJoint->mDesc = new NxPulleyJointDesc;
					}
					else if ( strstr( type, sFixedJoint ) )
					{
						mCurrJoint->mDesc = new NxFixedJointDesc;
					}

					if ( mCurrJoint->mDesc )
					{
						mCurrJoint->mDesc->setToDefault();
					}
				}
			}
			else if ( mCurrJoint && mCurrJoint->mDesc )
			{
				int type = mCurrJoint->mDesc->getType();

				switch( type )
				{
					case NX_JOINT_PRISMATIC:
						ObtainPrismaticJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_REVOLUTE:
						ObtainRevoluteJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_CYLINDRICAL:
						ObtainCylindricalJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_SPHERICAL:
						ObtainSphericalJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_POINT_ON_LINE:
						ObtainPointOnLineJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_POINT_IN_PLANE:
						ObtainPointInPlaneJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_DISTANCE:
						ObtainDistanceJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_PULLEY:
						ObtainPulleyJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_FIXED:
						ObtainFixedJointDesc( buff, which, argc, argv );
					break;

					case NX_JOINT_D6:
						ObtainD6JointDesc( buff, which, argc, argv );
					break;
				}
			}
		}

		return found;
	}

	bool ObtainEffectorDesc( const char *buff, int which, int argc, const char **argv )
	{
		bool found = true;
		static int snWhichType = 0;

		// k, make sure of the buff pntrs
		if ( buff && mCoreScene )
		{
			if ( strstr( buff, sNxEffector ) )
			{
				// k, this means we are on a new effector for the following info!
				// so, we can zero out the current one!
				mCurrSpringDamper = 0;
			}
			else if ( strstr( buff, sSpringAndDamperEffector ) )
			{
				if ( GetFirstBool( which+1, argc, argv ) )
				{
					mCurrSpringDamper = new NxSpringAndDamperEffectorDesc;
					if ( mCurrSpringDamper )
					{
						mCurrSpringDamper->setToDefault();
						mCoreScene->mEffectorArray.push_back( mCurrSpringDamper );
					}
				}
			}
			else if ( mCurrSpringDamper )
			{
				if ( strstr( buff, sActorIndex ) )
				{
					if ( GetDigitValue_Int( buff ) == 1 )
					{
						mCurrSpringDamper->body1 = (NxActor *)GetFirstInt( which+1, argc, argv );
					}
					else
					{
						mCurrSpringDamper->body2 = (NxActor *)GetFirstInt( which+1, argc, argv );
					}
				}
				else if ( strstr( buff, sJointConnectionPos ) )
				{
					if ( GetDigitValue_Int( buff ) == 1 )
					{
						float vals[3];
						GetFloatVector( vals, 3, which+1, argc, argv );
						mCurrSpringDamper->pos1.set( vals[0], vals[1], vals[2] );
					}
					else
					{
						float vals[3];
						GetFloatVector( vals, 3, which+1, argc, argv );
						mCurrSpringDamper->pos2.set( vals[0], vals[1], vals[2] );
					}
				}
				else if ( strstr( buff, sLinearSpringInfo ) )
				{
					snWhichType = 0;
				}
				else if ( strstr( buff, sLinearDamperInfo ) )
				{
					snWhichType = 1;
				}
				else if ( snWhichType == 0 )
				{
					if ( strstr( buff, sDistCompressSaturate ) )
					{
						mCurrSpringDamper->springDistCompressSaturate = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sDistRelaxed ) )
					{
						mCurrSpringDamper->springDistRelaxed = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sDistStretchSaturate ) )
					{
						mCurrSpringDamper->springDistStretchSaturate = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sMaxCompressForce ) )
					{
						mCurrSpringDamper->springMaxCompressForce = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sMaxStretchForce ) )
					{
						mCurrSpringDamper->springMaxStretchForce = GetFirstFloat( which+1, argc, argv );
					}
				}
				else
				{
					if ( strstr( buff, sVelCompressSaturate ) )
					{
						mCurrSpringDamper->damperVelCompressSaturate = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sVelStretchSaturate ) )
					{
						mCurrSpringDamper->damperVelStretchSaturate = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sMaxCompressForce ) )
					{
						mCurrSpringDamper->damperMaxCompressForce = GetFirstFloat( which+1, argc, argv );
					}
					else if ( strstr( buff, sMaxStretchForce ) )
					{
						mCurrSpringDamper->damperMaxStretchForce = GetFirstFloat( which+1, argc, argv );
					}
					else
					{
						found = false;
					}
				}
			}
			else
			{
				found = false;
			}
		}

		return found;
	}
	
	bool ObtainSceneInfo( int argc, const char **argv )
	{
		int which  = 0;
		bool found = false;

		while ( !found && ( which < argc ) )
		{
			char buff[256];
			sprintf( buff, argv[which] );
			strlwrPortable( buff );

			if ( !IsHardSymbol( buff ) )
			{
				if ( strstr( buff, sScene_SceneDesc ) )
				{
					// k, we need to allocate a core scene
					if ( mCoreScene )
					{
						mCoreScene = 0;
					}
					if ( !mCoreScene )
					{
						mCoreScene = new CoreScene;
						assert( mCoreScene );
						if ( mCoreScene )
						{
							mCoreScene->mDesc.setToDefault();
							mContainer->AddCoreScene( mCoreScene );
						}
					}

					// k, we have a new scene desc - but we can bypass the val that says
					mSceneParseType = SPT_SCENE_DESC;
					found = true;
				}
				else if ( strstr( buff, sSceneNum ) && GetDigitValue( buff ) )
				{
					found = true;
				}
				else if ( strstr( buff, sScene_NumMaterials ) )
				{
					// k, here are the number of materials
					mSceneParseType = SPT_MATERIALS;
					found = true;
				}
				else if ( strstr( buff, sScene_NumActors ) )
				{
					// k, here are number of actors
					mSceneParseType = SPT_ACTORS;
					found = true;
				}
				else if ( strstr( buff, sScene_NumPairs ) )
				{
					mSceneParseType	= SPT_PAIR_FLAGS;
					if ( mCoreScene )
					{
						mCoreScene->mPairCount = GetFirstInt( which+1, argc, argv );
					}
					found = true;
				}
				else if ( strstr( buff, sScene_NumJoints ) )
				{
					mSceneParseType = SPT_JOINTS;
					found = true;
				}
				else if ( strstr( buff, sScene_NumEffectors ) )
				{
					mSceneParseType = SPT_EFFECTORS;
					found = true;
				}
				else if ( mCoreScene )
				{
					// k, now what were we doing last?
					switch( mSceneParseType )
					{
						case SPT_SCENE_DESC:
						{
							found = ObtainSceneDesc( buff, which, argc, argv );
						}
						break;

						case SPT_MATERIALS:
						{
							found = ObtainMaterialDesc( buff, which, argc, argv );
						}
						break;

						case SPT_ACTORS:
						{
							found = ObtainActorDesc( buff, which, argc, argv );
						}
						break;

						case SPT_PAIR_FLAGS:
						{
							found = ObtainPairFlagDesc( buff, which, argc, argv );
						}
						break;

						case SPT_JOINTS:
						{
							found = ObtainJointDesc( buff, which, argc, argv );
						}
						break;

						case SPT_EFFECTORS:
						{
							found = ObtainEffectorDesc( buff, which, argc, argv );
						}
						break;
					}

					if ( !found )
					{
						// back to square one
						mLastParseType  = PT_LAST;
						mSceneParseType = SPT_LAST;
					}

				}
			}
			++which;
		}

		return found;
	}

	bool IsHardSymbol( const char *test )
	{
		bool isHard = false;
		if ( test )
		{
			for ( int i = 0; !isHard && (i < nNumHardSymbols); ++i )
			{
				isHard = ( test[0] == sHardSymbols[i] );
			}
		}
		return isHard;
	}

	bool SetParam( int which, int argc, const char **argv )
	{
		bool found = false;

		for ( int i = 0; !found && (i < argc); ++i )
		{
			char buff[128];
			sprintf( buff, argv[i] );
			//strlwrPortable( buff );

			if ( !stricmp( buff, sNxParam_BoxNoiseLevel ) )
			{
				found = true;
#if NX_SDK_VERSION_NUMBER > 230
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_BBOX_NOISE_LEVEL, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_SolverConvergenceThreshold ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER > 230
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_SOLVER_CONVERGENCE_THRESHOLD, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidDrains ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_DRAINS, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidPackets ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_PACKETS, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if (!stricmp( buff, sNxParam_VisualizeFluidMotionLimit ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_MOTION_LIMIT, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidDynCollision ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_DYN_COLLISION, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_CCDEpsilon ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_CCD_EPSILON, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_PenaltyForce ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_PENALTY_FORCE, value );
			}
			else if ( !stricmp( buff, sNxParam_SkinWidth ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_SKIN_WIDTH, value );
			}
			else if ( !stricmp( buff, sNxParam_DefaultSleepLinVelSquared ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_DEFAULT_SLEEP_LIN_VEL_SQUARED, value );
			}
			else if ( !stricmp( buff, sNxParam_DefaultSleepAngVelSquared ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_DEFAULT_SLEEP_ANG_VEL_SQUARED, value );
			}
			else if ( !stricmp( buff, sNxParam_BounceThreshold ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_BOUNCE_THRESHOLD, value );
#else
				mContainer->SetParam( NX_BOUNCE_TRESHOLD, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_DynFrictScaling ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_DYN_FRICT_SCALING, value );
			}
			else if ( !stricmp( buff, sNxParam_StaFrictScaling ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_STA_FRICT_SCALING, value );
			}
			else if ( !stricmp( buff, sNxParam_MaxAngularVelocity ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_MAX_ANGULAR_VELOCITY, value );
			}
			else if ( !stricmp( buff, sNxParam_ContinuousCD	) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_CONTINUOUS_CD, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizationScale ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZATION_SCALE , value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeWorldAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_WORLD_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeBodyAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_BODY_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeBodyMassAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_BODY_MASS_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeBodyLinVelocity	) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_BODY_LIN_VELOCITY, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeBodyAngVelocity ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_BODY_ANG_VELOCITY, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeBodyJointGroups	) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_BODY_JOINT_GROUPS, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeJointLocalAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_JOINT_LOCAL_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeJointWorldAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_JOINT_WORLD_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeJointLimits	) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_JOINT_LIMITS, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeContactPoint ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CONTACT_POINT, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeContactNormal ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CONTACT_NORMAL, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeContactError ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CONTACT_ERROR, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeContactForce ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CONTACT_FORCE, value );
			}
			else if ( !stricmp( buff, sNxParamVisualizeActorAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_ACTOR_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionAABBs ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_AABBS, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionShapes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_SHAPES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionAxes ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_AXES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionCompounds ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_COMPOUNDS, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionVNormals ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_VNORMALS, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionFNormals ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_FNORMALS, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisioEdges ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_VISUALIZE_COLLISION_EDGES, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionSpheres ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_SPHERES, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionSAP ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_SAP, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionStatic ) )
			{
				found = true;
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_STATIC, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionDynamic ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_DYNAMIC, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionFree ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_COLLISION_FREE, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionCCD ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_VISUALIZE_COLLISION_CCD, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeCollisionSkeletons ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_VISUALIZE_COLLISION_SKELETONS, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidEmitters ) )
			{
				found = true;				
#if	NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_EMITTERS, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidPosition ) )
			{
				found = true;				
#if	NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_POSITION, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidVelocity ) )
			{
				found = true;				
#if	NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_VELOCITY, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidKernelRadius ) )
			{
				found = true;				
#if	NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_KERNEL_RADIUS, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeFluidBounds	) )
			{
				found = true;				
#if	NX_USE_FLUID_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_FLUID_BOUNDS, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_AdaptiveForce ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_ADAPTIVE_FORCE, value );
			}
			else if ( !stricmp( buff, sNxParam_CollVetoJoined ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_COLL_VETO_JOINTED, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_TriggerTriggerCallback ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_TRIGGER_TRIGGER_CALLBACK, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_SelectHWAlgo ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_SELECT_HW_ALGO, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeActiveVertices ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
#if NX_SDK_VERSION_NUMBER >= 230
				mContainer->SetParam( NX_VISUALIZE_ACTIVE_VERTICES, value );
#endif
			}
			else if ( !stricmp( buff, sNxParam_ParamsNumValues ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_PARAMS_NUM_VALUES, value );
			}
			else if ( !stricmp( buff, sNxParam_MinSeparationForPenalty ) )
			{
				found = true;				
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_MIN_SEPARATION_FOR_PENALTY, value );
			}
			else if ( !stricmp( buff, sNxParam_VisualizeClothCollisions ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_CLOTH_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CLOTH_COLLISIONS, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeClothSelfCollisions ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_CLOTH_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CLOTH_SELFCOLLISIONS, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}
			else if ( !stricmp( buff, sNxParam_VisualizeClothWorkPackets ) )
			{
				found = true;				
#if NX_SDK_VERSION_NUMBER >= 231 && NX_USE_CLOTH_API
				float value = GetFirstFloat( i+1, argc, argv );
				mContainer->SetParam( NX_VISUALIZE_CLOTH_WORKPACKETS, value );
#else 
				char tmp[128];
				sprintf( tmp, "NOTE  Parameter '%s' is not enabled in this build - continuing anyways",
					buff );
				OutputStatusMsg( tmp );
#endif
			}

		}
		return found;
	}

	ParseType GetWhatLineMeans( int argc, const char **argv, int &which )
	{
		bool found		= false;
		ParseType ret	= PT_LAST;
		which = 0;

		// k, we get to figure out what the hell a line means!
		while ( !found && ( which < argc ) )
		{
			char buff[256];
			sprintf( buff, argv[which] );
			strlwrPortable( buff );

			if ( !IsHardSymbol( buff ) )
			{
				// k, check the first arg - at first, we check "main" strings, to determine
				// if we are switching what we are doing - then we check "sub-main" args
				if ( !mParsedMainHeading[PT_SDK_VERSION] && 
					 strstr( buff, sPhysicsSDKVersion ) )
				{
					ret = PT_SDK_VERSION;
				}
				else if ( !mParsedMainHeading[PT_NUM_PARAMS] && 
						strstr( buff, sNumParams ) )
				{
					ret = PT_NUM_PARAMS;
				}
				else if ( !mParsedMainHeading[PT_NUM_TRIANGLE_MESHES] && 
						strstr( buff, sNumTriangleMeshes ) )
				{
					ret = PT_NUM_TRIANGLE_MESHES;
				}
				else if ( !mParsedMainHeading[PT_NUM_CONVEX_MESHES] && 
						strstr( buff, sNumConvexMeshes ) )
				{
					ret = PT_NUM_CONVEX_MESHES;
				}
				else if ( !mParsedMainHeading[PT_NUM_CCD_SKELETONS] && 
						strstr( buff, sNumCCDSkeletons ) )
				{
					ret = PT_NUM_CCD_SKELETONS;
				}
				else if ( !mParsedMainHeading[PT_NUM_HEIGHT_FIELDS] &&
						strstr( buff, sNumHeightFields ) )
				{
					ret = PT_NUM_HEIGHT_FIELDS;
				}
				else if ( !mParsedMainHeading[PT_NUM_SCENES] && 
						strstr( buff, sNumScenes ) )
				{
					ret = PT_NUM_SCENES;
				}
				else
				{
					if ( ( mLastParseType == PT_NUM_PARAMS ) ||
						( mLastParseType == PT_PARAM_VALUE ) )
					{
						ret = PT_PARAM_VALUE;
					}
					else if ( ( mLastParseType == PT_NUM_TRIANGLE_MESHES ) ||
							( mLastParseType == PT_TRIANGLE_MESH_INFO ) )
					{
						ret = PT_TRIANGLE_MESH_INFO;
					}
					else if ( ( mLastParseType == PT_NUM_CONVEX_MESHES ) ||
							( mLastParseType == PT_CONVEX_MESH_INFO ) )
					{
						ret = PT_CONVEX_MESH_INFO;
					}
					else if ( ( mLastParseType == PT_NUM_CCD_SKELETONS ) ||
							( mLastParseType == PT_CCD_SKELETON_INFO ) )
					{
						ret = PT_CCD_SKELETON_INFO;
					}
					else if ( ( mLastParseType == PT_NUM_HEIGHT_FIELDS ) ||
							  ( mLastParseType == PT_HEIGHT_FIELD_INFO ) )
					{
						ret = PT_HEIGHT_FIELD_INFO;
					}
					else if ( ( mLastParseType == PT_NUM_SCENES ) || 
							  ( mLastParseType == PT_SCENE_INFO ) )
					{
						ret = PT_SCENE_INFO;
					}
				}

				// are we about to parse a main heading?
				if ( ret < PT_NUM_MAIN_HEADINGS )
				{
					mParsedMainHeading[ret] = true;
				}
			}

			++which;
			found = ( ret != PT_LAST );
		}

		return ret;
	}

	virtual void GiveAllActorsThatDontHaveNamesNames( void )
	{
		if ( mCoreScene )
		{
			unsigned size = mCoreScene->mActors.size();
			for ( unsigned i = 0; i < size; ++i )
			{
				NxActorDesc *a = mCoreScene->mActors[i];

				if ( a )
				{
					char buff[128];
					int len = strlen( a->name );
					if ( len )
					{
						strcpy( buff, a->name );
						strlwrPortable( buff );
					}
					else
					{
						memset( buff, 0, sizeof(char) * 128 );
					}

					if ( ( len == 0 ) || 
					   ( ( len != 0) && strstr( buff, sNull ) ) )
					{
						sprintf( buff, "Actor_%d", i+1 );
						a->name = CoreContainer::GetStringPntr( buff );
					}
				}
			}
		}		
	}

	// return TRUE to continue parsing, return FALSE to abort parsing process
	virtual int ParseLine( int lineno, int argc, const char **argv )
	{
		int which	= 0;
		bool found	= false;
		ParseType pt = GetWhatLineMeans( argc, argv, which );

		// k, is this indicating our version number? - [Physics SDK Version]
		switch( pt )
		{
			case PT_SDK_VERSION:
			{
				// k, this is our version - check versus current, and if it differs,
				// we spit out a msg
	  			int	version					= GetFirstInt( which+1, argc, argv );
  				unsigned int coreVersion	= NxGetCoreVersion( mSDK );			
				if ( version != coreVersion )
				{
					char tmp[128];
					sprintf( tmp, "Version of this dump is %d - current SDK version is %d - continuing anyways",
						version, coreVersion );
					OutputStatusMsg( tmp );
				}
				found = true;
			}
			break;

			case PT_NUM_PARAMS:
			{
				// k, get the number of parameters
				int numParams = GetFirstInt( which+1, argc, argv );
				if ( numParams != NX_PARAMS_NUM_VALUES )
				{
					char tmp[128];
					sprintf( tmp, "NumParams for this dump is %d - current SDK numParams is %d - continuing anyways",
						numParams, NX_PARAMS_NUM_VALUES );
					OutputStatusMsg( tmp );
				}
				found = true;
			}
			break;

			case PT_PARAM_VALUE:
			{
				// k, get a parameter value
				found = SetParam( which+1, argc, argv );
			}
			break;

			case PT_NUM_TRIANGLE_MESHES:
			{
				mNumTriangleMeshes = GetFirstInt( which+1, argc, argv );
				found = true;
			}
			break;

			case PT_TRIANGLE_MESH_INFO:
			{
				// k, here we get triangle mesh information
				found = ObtainTriangleMeshInfo( argc, argv );
			}
			break;

			case PT_NUM_CONVEX_MESHES:
			{
				mNumConvexMeshes = GetFirstInt( which+1, argc, argv );
				found = true;
			}
			break;

			case PT_CONVEX_MESH_INFO:
			{
				found = ObtainConvexMeshInfo( argc, argv );
			}
			break;

			case PT_NUM_CCD_SKELETONS:
			{
				mNumSkeletons = GetFirstInt( which+1, argc, argv );
				found = true;
			}
			break;

			case PT_CCD_SKELETON_INFO:
			{
				found = ObtainSkeletonInfo( argc, argv );
			}
			break;

			case PT_NUM_HEIGHT_FIELDS:
			{
				mNumHeightFields = GetFirstInt( which+1, argc, argv );
				found = true;
			}
			break;

			case PT_HEIGHT_FIELD_INFO:
			{
				found = ObtainHeightFieldInfo( argc, argv );
			}
			break;

			case PT_NUM_SCENES:
			{
				mNumScenes = GetFirstInt( which+1, argc, argv );
				found = true;
			}
			break;

			case PT_SCENE_INFO:
			{
				found = ObtainSceneInfo( argc, argv );
			}
			break;
		}

		if ( !found )
		{
			bool save = true;

			// one extra check
			if ( ( argc == 1 ) && !strcmp( argv[0], sCommand_GiveActorsNames ) )
			{
				save = false;
				
				// now give all actors that do not have names names
				GiveAllActorsThatDontHaveNamesNames();
			}

			if ( save )
			{
				// k, here we need to push this back!
				mContainer->SaveAsPsCommand( argc, argv, this );
			}
		}

		mLastParseType = pt;
		return 0;
	}

private:

	NxPhysicsSDK *		mSDK;
	CoreContainer	*	mContainer;
	bool				mParsedMainHeading[PT_NUM_MAIN_HEADINGS];
	ParseType			mLastParseType;
	FILE *				mFp;
	unsigned			mParamNum;
	int					mNumTriangleMeshes;
	int					mNumConvexMeshes;
	int					mNumSkeletons;
	int					mNumHeightFields;
	int					mNumScenes;
	TriangleMeshData *  mTriangleMeshData;
	ConvexMeshData *	mConvexMeshData;
	HeightFieldData *   mHeightFieldData;
	void *				mVoidVrtPntr;
	void *				mVoidTriPntr;
	int					mCurrTriMesh;
	int					mCurrConvexMesh;
	int					mCurrSkeleton;
	int					mCurrHeightField;
	int					mCurrScene;
	CoreScene *			mCoreScene;
	SceneParseType		mSceneParseType;
	NxMaterialDesc *	mCurrMat;
	NxActorDesc *		mCurrActor;
	NxBodyDesc *		mBodyDesc;
	ActorParseType		mActorParseType;
	NxShapeDesc	*		mCurrShape;
	int					mWhichPairFlag;
	JointDescription *	mCurrJoint;
	NxSpringAndDamperEffectorDesc * mCurrSpringDamper;
};

//==================================================================================
// CoreContainer Code Starts Here
//==================================================================================

//==================================================================================
bool CoreContainer::GetBool(void)
{
	bool ret = false;
	if ( mFp )
	{
		char tmp;
		fread( &tmp, sizeof(char), 1, mFp );
		if ( tmp ==	1 )
		{
			ret	= true;
		}
	}
	return ret;
}

//==================================================================================
char CoreContainer::GetChar(void)
{
	char ret = 0;
	if ( mFp )
	{
		fread( &ret, sizeof(char), 1, mFp );
	}
	return ret;
}

//==================================================================================
int CoreContainer::GetInt(void)
{
	int	ret	= 0;
	if ( mFp )
	{
		fread( &ret, sizeof(int), 1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxU32*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
long CoreContainer::GetLong(void)
{
	long ret = 0;
	if ( mFp )
	{
		fread( &ret, sizeof(long), 1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxU32*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
short CoreContainer::GetShort(void)
{
	short ret =	0;
	if ( mFp )
	{
		fread( &ret, sizeof(short),	1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxU16*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
unsigned CoreContainer::GetUnsigned(void)
{
	unsigned ret = 0;
	if ( mFp )
	{
		fread( &ret, sizeof(unsigned), 1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxU32*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
unsigned long CoreContainer::GetUnsignedLong(void)
{
	unsigned long ret =	0;
	if ( mFp )
	{
		fread( &ret, sizeof(unsigned long),	1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxU32*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
unsigned short CoreContainer::GetUnsignedShort( void )
{
	unsigned short ret = 0;
	if ( mFp )
	{
		fread( &ret, sizeof(unsigned short), 1,	mFp	);
		#ifdef _XBOX
		ret=coredump_flip((const NxU16*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
float CoreContainer::GetFloat(void)
{
	float ret =	0;
	if ( mFp )
	{
		fread( &ret, sizeof(float),	1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxF32*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
double CoreContainer::GetDouble(void)
{
	double ret = 0;
	if ( mFp )
	{
		fread( &ret, sizeof(double), 1,	mFp	);
		#ifdef _XBOX
		ret=coredump_flip((const NxF64*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
int CoreContainer::GetString( char *buff, int buffSize )
{
	int	len	= 0;
	if ( mFp )
	{
		len	= GetInt();
		assert(	len	< buffSize );
		if ( len )
		{
			fread( buff, sizeof(char), sizeof(char)*len, mFp );
			buff[len] =	'\0';
		}
	}
	return len;
}

//==================================================================================
unsigned CoreContainer::GetFlag( void )
{
	unsigned ret = 0;
	if ( mFp )
	{
		fread( &ret, sizeof(unsigned), 1, mFp );
		#ifdef _XBOX
		ret=coredump_flip((const NxU32*)&ret);
		#endif
	}
	return ret;
}

//==================================================================================
void CoreContainer::GetMatrix( NxMat34 &matrix )
{
	NxReal m[9];
	for	( int i	= 0; i < 9;	++i	)
	{
		m[i] = GetFloat();
	}
	matrix.M.setRowMajor( m	);

	matrix.t.x = GetFloat();
	matrix.t.y = GetFloat();
	matrix.t.z = GetFloat();
}

//==================================================================================
void CoreContainer::GetVec3( NxVec3 &v )
{
	if ( mFp )
	{
		fread( &v.x, sizeof(float),	1, mFp );	 
		fread( &v.y, sizeof(float),	1, mFp );	 
		fread( &v.z, sizeof(float),	1, mFp );	 
		#ifdef _XBOX
		v.x=coredump_flip((const NxF32*)&v.x);
		v.y=coredump_flip((const NxF32*)&v.y);
		v.z=coredump_flip((const NxF32*)&v.z);
		#endif
	}
}

//==================================================================================
void CoreContainer::GetQuat( NxQuat &q )
{
	NxReal quat[4] = {0, 0,	0, 0};

	if ( mFp )
	{
		fread( &quat[0], sizeof(float),	1, mFp );
		fread( &quat[1], sizeof(float),	1, mFp );
		fread( &quat[2], sizeof(float),	1, mFp );
		fread( &quat[3], sizeof(float),	1, mFp );
		#ifdef _XBOX
		quat[0]=coredump_flip((const NxF32*)&quat[0]);
		quat[1]=coredump_flip((const NxF32*)&quat[1]);
		quat[2]=coredump_flip((const NxF32*)&quat[2]);
		quat[3]=coredump_flip((const NxF32*)&quat[3]);
		#endif
	}
	q.setXYZW( quat	);
}

//==================================================================================
bool CoreContainer::GetBodyDesc( NxBodyDesc &b )
{
	bool ret = false;
	if ( mFp &&	GetBool() )
	{
		GetMatrix( b.massLocalPose	);
		GetVec3( b.massSpaceInertia );
		b.mass				   = GetFloat();
		GetVec3( b.linearVelocity );
		GetVec3( b.angularVelocity	);
		b.wakeUpCounter		   = GetFloat();
		b.linearDamping		   = GetFloat();
		b.angularDamping	   = GetFloat();
		b.maxAngularVelocity   = GetFloat();
#if NX_SDK_VERSION_NUMBER >= 230
		b.CCDMotionThreshold   = GetFloat();
#endif
		b.sleepLinearVelocity  = GetFloat();
		b.sleepAngularVelocity = GetFloat();
		b.solverIterationCount = GetUnsigned();
		b.flags	               = GetFlag();

		ret	= true;
	}
	return ret;
}

//==================================================================================
NxShapeType CoreContainer::GetShapeType( void )
{
	NxShapeType	ret	= NX_SHAPE_COUNT;

	// k, do we	even have a	shape?
	if ( GetBool() )
	{
		// k, get the type
		ret	= static_cast<NxShapeType>(	GetInt() );
	}
	return ret;
}

//==================================================================================
void CoreContainer::GetShapeDesc( NxShapeDesc &desc )
{
	// k, do we	need its name -	for	now	assuming yes?
	if ( mFp )
	{
		// Get local pose
		GetMatrix( desc.localPose );

		// get other information
		desc.group		   = GetInt();
		desc.materialIndex = GetInt();
#if NX_SDK_VERSION_NUMBER >= 230
		desc.mass		   = GetFloat();
		desc.density	   = GetFloat();
#endif
		desc.skinWidth	   = GetFloat();

		// k, here we don't	care about the description - just the flags
		desc.shapeFlags	   = GetFlag();
#if NX_SDK_VERSION_NUMBER >= 230
		// k, this is bad, as it reads a pntr data,	which is not good
		desc.ccdSkeleton   = reinterpret_cast<NxCCDSkeleton	*>(	GetInt() );
#endif

#ifdef NX_SUPPORT_NEW_FILTERING
		//!< Groups	bitmask	for	collision filtering
		desc.groupsMask.bits0 =	GetUnsigned();
		desc.groupsMask.bits1 =	GetUnsigned();
		desc.groupsMask.bits2 =	GetUnsigned();
		desc.groupsMask.bits3 =	GetUnsigned();
#endif

		char buff[128];
		desc.name	= 0;
		int	nameLen	= GetString( buff,	128	);
		if ( nameLen )
		{
			char *tmp = new	char[nameLen+1];
			assert(	tmp );
			strcpy( tmp, buff );
			desc.name = tmp;
		}
	}
}

//==================================================================================
void CoreContainer::GetSphereShapeDesc(	NxSphereShapeDesc &desc )
{
	// k, get normal shape desc
	GetShapeDesc( desc );

	// now get the rest
	desc.radius	= GetFloat();
}

//==================================================================================
void CoreContainer::GetBoxShapeDesc( NxBoxShapeDesc &desc )
{
	// k, get normal shape desc
	GetShapeDesc( desc );

	// now get the rest
	GetVec3( desc.dimensions );
}

//==================================================================================
void CoreContainer::GetCapsuleShapeDesc( NxCapsuleShapeDesc &desc )
{
	// k, get normal shape desc
	GetShapeDesc( desc	);

	// now get the rest
	desc.radius	= GetFloat(); 
	desc.height	= GetFloat();
	desc.flags	= GetFlag();
}

//==================================================================================
void CoreContainer::GetConvexShapeDesc(	NxConvexShapeDesc &desc )
{
	// k, get normal shape desc
	GetShapeDesc( desc );

	desc.meshFlags = GetInt();
#ifdef NX_SUPPORT_CONVEX_SCALE
	desc.scale	   = GetFloat();
#endif

	// k, get the index of its mesh
	unsigned index = GetInt();
	desc.meshData  = (NxConvexMesh *)index;
}

//==================================================================================
void CoreContainer::GetTriangleMeshShapeDesc( NxTriangleMeshShapeDesc &desc )
{
	// k, get normal shape desc
	GetShapeDesc( desc );

	desc.meshFlags = GetFlag();
#ifdef NX_SUPPORT_CONVEX_SCALE
	desc.scale	   = GetFloat();
#endif

	// k, get the index of its mesh
	unsigned index = GetUnsigned();
	desc.meshData  = (NxTriangleMesh *)index;
}

#if NX_SDK_VERSION_NUMBER >= 240
//==================================================================================
void CoreContainer::GetHeightFieldShapeDesc( NxHeightFieldShapeDesc &desc )
{
	// k, get normal shape desc
	GetShapeDesc( desc );

	// k, get rest of info
	desc.meshFlags = GetFlag();

#ifdef NX_SUPPORT_CONVEX_SCALE
	desc.scale	   = GetFloat();
#endif

	// now get other info
	desc.holeMaterial			= GetUnsigned();
	desc.materialIndexHighBits	= GetUnsigned();
	desc.columnScale			= GetFloat();
	desc.rowScale				= GetFloat();
	desc.heightScale			= GetFloat();
	desc.heightField			= (NxHeightField *)GetInt();
}
#endif

//==================================================================================
void CoreContainer::GetTriangleMeshDesc( TriangleMeshData *tmd, int indexToCheck )
{
	if ( mFp && tmd )
	{
		int	i;

		// get its index
		int	index = GetInt();
		assert(	index == indexToCheck );

		// get its stride index
		tmd->mTmd.materialIndexStride	= GetInt();
		unsigned numIndices				= GetUnsigned();
		if ( numIndices )
		{
			if ( tmd->mTmd.materialIndexStride == 4 )
			{
				int *ptr = new int[numIndices];
				assert( ptr );
				if ( ptr )
				{
					memset( ptr, 0, sizeof(int)*numIndices );
				}

				for ( unsigned j = 0; j < numIndices; ++j )
				{
					int val = GetInt();
					if ( ptr )
					{
						ptr[j] = val;
					}
				}
				tmd->mTmd.materialIndices = (void *)ptr;
			}
			else if ( tmd->mTmd.materialIndexStride == 2 )
			{
				short *ptr = new short[numIndices];
				assert( ptr );
				if ( ptr )
				{
					memset( ptr, 0, sizeof(short)*numIndices );
				}

				for ( unsigned j = 0; j < numIndices; ++j )
				{
					short val = GetShort();
					if ( ptr )
					{
						ptr[j] = val;
					}
				}			
				tmd->mTmd.materialIndices = (void *)ptr;
			}
			else if ( tmd->mTmd.materialIndexStride == 1 )
			{
				char *ptr = new char[numIndices];
				assert( ptr );
				if ( ptr )
				{
					memset( ptr, 0, sizeof(char)*numIndices );
				}

				for ( unsigned j = 0; j < numIndices; ++j )
				{
					char val = GetChar();
					if ( ptr )
					{
						ptr[j] = val;
					}
				}
				tmd->mTmd.materialIndices = (void *)ptr;
			}
		}
		else
		{
			tmd->mTmd.materialIndices = 0;
		}

		// get its height field vertical axis type, etc.
		tmd->mTmd.heightFieldVerticalAxis	= (NxHeightFieldAxis)GetInt();
		tmd->mTmd.heightFieldVerticalExtent = GetFloat();
		tmd->mTmd.convexEdgeThreshold		= GetFloat();
		tmd->mTmd.flags						= GetFlag();

		// get num vertices and point stride bytes
		tmd->mTmd.numVertices		= GetInt();
		tmd->mTmd.pointStrideBytes	= GetInt();
		int	totalToRead				= tmd->mTmd.numVertices*3;
		float *newPts				= new float[totalToRead];
		assert( newPts );
		if ( newPts )
		{
			memset( newPts, 0, sizeof(float)*totalToRead );
		}

		tmd->mTmd.points = (const void *)newPts;
		for	( i	= 0; i < totalToRead; ++i )
		{
			float val = GetFloat();
			if ( newPts )
			{
				newPts[i] =	val;
			}
		}

		// get num triangles and triangle stride bytes
		tmd->mTmd.numTriangles			= GetInt();
		tmd->mTmd.triangleStrideBytes	= GetInt();

		int	size = tmd->mTmd.numTriangles*3;
		if ( tmd->mTmd.triangleStrideBytes/3 == 1 )
		{
			tmd->mTmd.triangles = (void *)( new char[size] );
			assert( tmd->mTmd.triangles );
			char *tmp = (char *)tmd->mTmd.triangles;
			if ( tmp )
			{
				memset( tmp, 0, sizeof(char)*size );
			}

			for	( i	= 0; i < size; ++i )
			{
				char val1 = GetChar();
				char val2 = GetChar();
				char val3 = GetChar();
				if ( tmp )
				{
					tmp[i] = val1;
					++i;
					tmp[i] = val2;
					++i;
					tmp[i] = val3;
				}
			}
		}
		else if	( tmd->mTmd.triangleStrideBytes/3 == 2 )
		{
			tmd->mTmd.triangles = (void *)( new short[size] );
			assert( tmd->mTmd.triangles );
			short *tmp = (short	*)tmd->mTmd.triangles;
			if ( tmp )
			{
				memset( tmp, 0, sizeof(short)*size );
			}

			for	( i	= 0; i < size; ++i )
			{
				short val1 = GetShort();
				short val2 = GetShort();
				short val3 = GetShort();
				if ( tmp )
				{
					tmp[i] = val1;
					++i;
					tmp[i] = val2;
					++i;
					tmp[i] = val3;
				}
			}
		}
		else if	( tmd->mTmd.triangleStrideBytes/3 == 4 )
		{
			tmd->mTmd.triangles = (void *)( new int[size] );
			assert( tmd->mTmd.triangles );
			int	*tmp = (int *)tmd->mTmd.triangles;
			if ( tmp )
			{
				memset( tmp, 0, sizeof(int)*size );
			}

			for	( i	= 0; i < size; ++i )
			{
				int val1 = GetInt();
				int val2 = GetInt();
				int val3 = GetInt();
				if ( tmp )
				{
					tmp[i] = val1;	
					++i;
					tmp[i] = val2;
					++i;
					tmp[i] = val3;
				}
			}
		}
		else
		{
			assert(	false );
		}

		// SRM : new - did we store the cooked mesh data in this guy?
		tmd->mCookedDataSize = GetUnsigned();
		if ( tmd->mCookedDataSize )
		{
			// k, we got some cooked data!
			char *cookedData = new char[tmd->mCookedDataSize];
			if ( cookedData )
			{
				fread( cookedData, sizeof(char), sizeof(char)*tmd->mCookedDataSize, mFp );
				tmd->mCookedData = cookedData;
			}
			else
			{
				// hmm, don't got it, so just read in the info, but throw it away
				for ( int i = 0; i < tmd->mCookedDataSize; ++i )
				{
					GetChar();
				}
			}
		}

		// k, get the pmap data	(init to 0)
		unsigned pmapSize = GetUnsigned();
		if ( pmapSize )
		{
			NxPMap *pmap = new NxPMap;

			if ( pmap )
			{
				// init	info
				pmap->data	   = 0;
				pmap->dataSize = pmapSize;
			}

			char *pmapData = new char[pmapSize];
			assert( pmapData );
			if ( pmapData )
			{
				memset( pmapData, 0, sizeof(char)*pmapSize );

				// k, setup the pntr
				pmap->data = pmapData;
			}

			if ( pmap->data	)
			{
				// get the pmap data
				fread( pmap->data, sizeof(char), sizeof(char)*pmapSize,	mFp	);
			}
			else
			{
				// k, NOT good,	as we now just read	the	information	in,	but	throw it away!
				for	( unsigned i = 0; i	< pmapSize;	++i	)
				{
					GetChar();
				}

				// delete the pmap
				delete pmap;
				pmap = 0;
			}

			// assign the pmap
			tmd->mTmd.pmap = pmap;
		}
	}
}

#if NX_SDK_VERSION_NUMBER >= 240
//==================================================================================
void CoreContainer::GetHeightFieldDesc( HeightFieldData *hfd, int indexToCheck )
{
	if ( mFp && hfd )
	{
		// get its index
		int	index = GetInt();
		assert(	index == indexToCheck );

		// get flags, # rows, columns, etc.
		hfd->mHfd.flags					= GetFlag();
		hfd->mHfd.nbRows				= GetUnsigned();
		hfd->mHfd.nbColumns				= GetUnsigned();
		hfd->mHfd.format				= (NxHeightFieldFormat)GetUnsigned();
		hfd->mHfd.verticalExtent		= GetFloat();
		hfd->mHfd.convexEdgeThreshold	= GetFloat();
		hfd->mHfd.sampleStride			= GetInt();

		// do we have samples to get?
		if ( hfd->mHfd.sampleStride )
		{
			// k, here we go
			unsigned totalAmt = hfd->mHfd.nbColumns * hfd->mHfd.nbRows;
			if ( hfd->mHfd.format == NX_HF_S16_TM )
			{
				// k, get the info
				unsigned *data = new unsigned[totalAmt];
				if ( data )
				{
					fread( data, sizeof(unsigned), sizeof(unsigned)*totalAmt, mFp );
					hfd->mHfd.samples = data;
				}
				else
				{
					// hmm, allocation failed, so just read in the info, but throw it away
					for ( unsigned i = 0; i < totalAmt; ++i )
					{
						GetUnsigned();
					}
				}
			}
		}
	}
}
#endif // #if NX_SDK_VERSION_NUMBER >= 240

//==================================================================================
void CoreContainer::GetConvexMeshDesc( ConvexMeshData *cmd, int indexToCheck )
{
	if ( cmd )
	{
		unsigned i;

		// write its index
		int	index = GetInt();
		assert(	index == indexToCheck );

		// get its info
		cmd->mCmd.flags				= GetFlag();
		cmd->mCmd.numVertices		= GetInt();
		cmd->mCmd.pointStrideBytes	= GetInt();

		// now get all the vertices
		int	size	= cmd->mCmd.pointStrideBytes / 3;
		char *verts	= new char[cmd->mCmd.numVertices * cmd->mCmd.pointStrideBytes];
		assert( verts );
		if ( verts )
		{
			memset( verts, 0, sizeof(char)*cmd->mCmd.numVertices * cmd->mCmd.pointStrideBytes );
		}

		for	( i=0; i<cmd->mCmd.numVertices; ++i )
		{
			int	indexToUse = i * cmd->mCmd.pointStrideBytes;

			// read	the	vertices
			int	which =	GetInt();
			assert(	which == i );

			float val1 = GetFloat();
			float val2 = GetFloat();
			float val3 = GetFloat();

			if ( verts )
			{
				float *fpntr = (float *)&verts[indexToUse];
				fpntr[0]	 = val1;
				fpntr[1]	 = val2;
				fpntr[2]	 = val3;
			}
		}
		// set pntr
		cmd->mCmd.points = verts;

		// now get number of triangles and their stride	bytes
		cmd->mCmd.numTriangles			= GetInt();
		cmd->mCmd.triangleStrideBytes	= GetInt();

		// only try and get triangle indices if there are any
		char *indices = 0;
		if ( cmd->mCmd.numTriangles )
		{
			// k, now get the triangle indices
			size = cmd->mCmd.triangleStrideBytes / 3;
			indices = new char[cmd->mCmd.numTriangles * cmd->mCmd.triangleStrideBytes];
			for( i=0; i<cmd->mCmd.numTriangles; ++i )
			{
				int	indexToUse = i * cmd->mCmd.triangleStrideBytes;
				int	*ipntr	   = (int *)&indices[indexToUse];
				ipntr[0]	   = GetInt();
				ipntr[1]	   = GetInt();
				ipntr[2]	   = GetInt();
			}
		}
		cmd->mCmd.triangles = indices;

		// SRM : new - did we store the cooked mesh data in this guy?
		cmd->mCookedDataSize = GetUnsigned();
		if ( cmd->mCookedDataSize )
		{
			// k, we got some cooked data!
			char *cookedData = new char[cmd->mCookedDataSize];
			if ( cookedData )
			{
				fread( cookedData, sizeof(char), sizeof(char)*cmd->mCookedDataSize, mFp );
				cmd->mCookedData = cookedData;
			}
			else
			{
				// hmm, don't got it, so just read in the info, but throw it away
				for ( int i = 0; i < cmd->mCookedDataSize; ++i )
				{
					GetChar();
				}
			}
		}
	}
}

//==================================================================================
void CoreContainer::GetPlaneShapeDesc( NxPlaneShapeDesc &desc )
{
	if ( mFp )
	{
		GetShapeDesc( desc );

		// now write plane shape desc specific params
		GetVec3( desc.normal );
		desc.d = GetFloat();
	}
}

//==================================================================================
void CoreContainer::GetActorDesc( NxActorDesc *nad )
{
	if ( mFp &&	nad	)
	{
		char buff[128];

		nad->setToDefault();

		// get its global pose
		GetMatrix( nad->globalPose	);

		// get body	description	if it exists (start	out	with pntr at 0)
		nad->body =	0;
		
		NxBodyDesc *bodyDesc = new NxBodyDesc;
		assert(	bodyDesc );
		if ( bodyDesc )
		{
			if ( !GetBodyDesc( *bodyDesc ) )
			{
				delete bodyDesc;
				bodyDesc = 0;
			}
		}
		nad->body		= bodyDesc;
		nad->density	= GetFloat();
		nad->group		= GetInt();
		nad->flags		= GetFlag();
		nad->name		= 0;

		int	readLen	= GetString( buff, 128 );
		if ( readLen )
		{
			char *tmp = new char[readLen+1];
			if ( tmp )
			{
				memset( tmp, 0, sizeof(char)*(readLen+1) );
				strcpy(	tmp, buff );
			}
			nad->name =	tmp;
		}
	}
}

//==================================================================================
NxShapeDesc *CoreContainer::GetActorShape( void )
{
	NxShapeDesc	*shape = 0;
	if ( mFp )
	{
		// k, does this	actor have a shape?
		NxShapeType type = GetShapeType();

		if ( type != NX_SHAPE_COUNT )
		{
			shape = CreateShapeFromType( type );

			switch(	type )
			{
				case NX_SHAPE_PLANE:
				{
					NxPlaneShapeDesc *desc = (NxPlaneShapeDesc *)shape;

					if ( desc )
					{
						GetPlaneShapeDesc( *desc );
					}
					else
					{
						NxPlaneShapeDesc tmp;
						GetPlaneShapeDesc( tmp );
					}
				}
				break;

				case NX_SHAPE_SPHERE:
				{
					NxSphereShapeDesc *desc	= (NxSphereShapeDesc *)shape;

					if ( desc )
					{
						GetSphereShapeDesc(	*desc );
					}
					else
					{
						NxSphereShapeDesc tmp;
						GetSphereShapeDesc( tmp );
					}
				}
				break;

				case NX_SHAPE_BOX:
				{
					NxBoxShapeDesc *desc = (NxBoxShapeDesc *)shape;

					if ( desc )
					{
						GetBoxShapeDesc( *desc );
					}
					else
					{
						NxBoxShapeDesc tmp;
						GetBoxShapeDesc( tmp );
					}
				}
				break;

				case NX_SHAPE_CAPSULE:
				{
					NxCapsuleShapeDesc *desc = (NxCapsuleShapeDesc *)shape;

					if ( desc )
					{
						GetCapsuleShapeDesc( *desc );
					}
					else
					{
						NxCapsuleShapeDesc tmp;
						GetCapsuleShapeDesc( tmp );
					}
				}
				break;

				case NX_SHAPE_CONVEX:
				{
					NxConvexShapeDesc *desc	= (NxConvexShapeDesc *)shape;

					if ( desc )
					{
						GetConvexShapeDesc(	*desc );
					}
					else
					{
						NxConvexShapeDesc tmp;
						GetConvexShapeDesc( tmp );
					}
				}
				break;
				case NX_SHAPE_MESH:
				{
					NxTriangleMeshShapeDesc	*desc =	(NxTriangleMeshShapeDesc *) shape;

					if ( desc )
					{
						GetTriangleMeshShapeDesc( *desc );
					}
					else
					{
						NxTriangleMeshShapeDesc tmp;
						GetTriangleMeshShapeDesc( tmp );
					}
				}
				break;

#if NX_SDK_VERSION_NUMBER >= 240
				case NX_SHAPE_HEIGHTFIELD:
				{
					NxHeightFieldShapeDesc *desc = (NxHeightFieldShapeDesc *)shape;
					if ( desc )
					{
						GetHeightFieldShapeDesc( *desc );
					}
					else
					{
						NxHeightFieldShapeDesc tmp;
						GetHeightFieldShapeDesc( tmp );
					}
				}
				break;
			
				case NX_SHAPE_RAW_MESH:
					assert(0); // this should not happen
				break;
#endif

				case NX_SHAPE_COMPOUND:
					assert(	0 );
				break;

				default:
					assert(0); //!!???
				break;
			}
		}
	}
	return shape;
}

//==================================================================================
NxShapeDesc *CoreContainer::CreateShapeFromType( NxShapeType type )
{
	NxShapeDesc *ret = 0;

	switch(	type )
	{
		case NX_SHAPE_PLANE:
		{
			NxPlaneShapeDesc *desc = new NxPlaneShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;

		case NX_SHAPE_SPHERE:
		{
			NxSphereShapeDesc *desc	= new NxSphereShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;

		case NX_SHAPE_BOX:
		{
			NxBoxShapeDesc *desc = new NxBoxShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;

		case NX_SHAPE_CAPSULE:
		{
			NxCapsuleShapeDesc *desc = new NxCapsuleShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;

		case NX_SHAPE_CONVEX:
		{
			NxConvexShapeDesc *desc	= new NxConvexShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;

		case NX_SHAPE_MESH:
		{
			NxTriangleMeshShapeDesc	*desc =	new	NxTriangleMeshShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;

#if NX_SDK_VERSION_NUMBER >= 230
		case NX_SHAPE_WHEEL:
		{
			NxWheelShapeDesc *desc = new NxWheelShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;
#endif

#if NX_SDK_VERSION_NUMBER >= 240
		case NX_SHAPE_HEIGHTFIELD:
		{
			NxHeightFieldShapeDesc *desc = new NxHeightFieldShapeDesc;
			assert( desc );
			ret = desc;
		}
		break;
	
		case NX_SHAPE_RAW_MESH:
			assert(0); // this should not happen
		break;
#endif

		case NX_SHAPE_COMPOUND:
			assert(	0 );
		break;

		default:
			assert(0); //!!???
		break;
	}
	return ret;
}

//==================================================================================
void CoreContainer::GetJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// IMPORTANT!  At this point, the joint	type has already been read in!!
		// get actor indices - Please note that	these are actually pointers, but we haven't
		// created them yet, so right now they are REFERENCES into the actor that will be created!!!!!!!
		d.mDesc->actor[0] = (NxActor *)GetInt();
		d.mDesc->actor[1] = (NxActor *)GetInt();
			
		// get normal, axis, anchor	(both 1	and	2)
		GetVec3( d.mDesc->localNormal[0] );
		GetVec3( d.mDesc->localAxis[0] );
		GetVec3( d.mDesc->localAnchor[0] );
		GetVec3( d.mDesc->localNormal[1] );
		GetVec3( d.mDesc->localAxis[1] );
		GetVec3( d.mDesc->localAnchor[1] );

		// write force and torque
		d.mDesc->maxForce	= GetFloat();
		d.mDesc->maxTorque	= GetFloat();

		// get joint name
		char name[128];
		int	nameLen	= GetString( name, 128 );
		if ( nameLen )
		{
			char *tmpName = new char[nameLen+1];
			assert(	tmpName );
			if ( tmpName )
			{
				memset( tmpName, 0, sizeof(char)*(nameLen+1) );
				strcpy( tmpName, name );
			}
			d.mDesc->name = tmpName;
		}

		// get flags
		d.mDesc->jointFlags = GetFlag();

		// get plane limits (if any)
		int numLimits = GetInt();
		if ( numLimits )
		{
			// get the limit point!
			GetVec3( d.mPlaneLimitPoint );
			d.mOnActor2 = GetBool();
		}
		// now, get each plane normal for limits!
		for ( int i = 0; i < numLimits; ++i )
		{
			NxVec3 planeNormal;
			GetVec3( planeNormal );
			float planeD = GetFloat();
			NxVec3 worldLimit;
			GetVec3( worldLimit );

			// allocate a joint limit plane
			PlaneInfo *ptr = new PlaneInfo;
			if ( ptr )
			{
				memset( ptr, 0, sizeof(PlaneInfo) );
				ptr->mPlaneNormal.set( planeNormal );
				ptr->mPlaneD		= planeD;
				ptr->mWorldLimitPt	= worldLimit;
				d.mPlaneInfo.push_back( ptr );
			}
		}
	}
}

//==================================================================================
void CoreContainer::GetPrismaticJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description - note	that is	all	we have	to do with this	joint type!
		GetJointDesc( d );
	}
}

//==================================================================================
void CoreContainer::GetRevoluteJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description
		GetJointDesc( d );

		NxRevoluteJointDesc *ptr = static_cast<NxRevoluteJointDesc * >( d.mDesc );

		// k, fill this	in - start with	low/high limits
		ptr->limit.low.hardness	 = GetFloat();
		ptr->limit.low.restitution	 = GetFloat();
		ptr->limit.low.value		 = GetFloat();
		ptr->limit.high.hardness	 = GetFloat();
		ptr->limit.high.restitution = GetFloat();
		ptr->limit.high.value		 = GetFloat();

		// get motor description
		ptr->motor.freeSpin  =	GetBool();
		ptr->motor.maxForce  =	GetFloat();
		ptr->motor.velTarget =	GetFloat();

		// get spring desc
		ptr->spring.spring		 = GetFloat();
		ptr->spring.damper		 = GetFloat();
		ptr->spring.targetValue = GetFloat();

		// get rest
		ptr->projectionDistance = GetFloat();
		ptr->projectionAngle	 = GetFloat();
		ptr->flags				 = GetFlag();

		ptr->projectionMode	 = (NxJointProjectionMode)GetUnsigned();
	}
}

//==================================================================================
void CoreContainer::GetCylindricalJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description - note	that is	all	we have	to do with this	joint type!
		GetJointDesc( d );
	}
}

//==================================================================================
void CoreContainer::GetSphericalJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description
		GetJointDesc( d );

		NxSphericalJointDesc *ptr = static_cast<NxSphericalJointDesc * >( d.mDesc );

		// now fill	the	rest of	it in
		// get spring descs
		ptr->twistSpring.spring	  =	GetFloat();
		ptr->twistSpring.damper	  =	GetFloat();
		ptr->twistSpring.targetValue =	GetFloat();
		ptr->swingSpring.spring	  =	GetFloat();
		ptr->swingSpring.damper	  =	GetFloat();
		ptr->swingSpring.targetValue =	GetFloat();
		ptr->jointSpring.spring	  =	GetFloat();
		ptr->jointSpring.damper	  =	GetFloat();
		ptr->jointSpring.targetValue =	GetFloat();

		// projection dist
		ptr->projectionDistance = GetFloat();

		// other limits	(twist and swing)
		ptr->twistLimit.low.hardness	  =	GetFloat();
		ptr->twistLimit.low.restitution  =	GetFloat();
		ptr->twistLimit.low.value		  =	GetFloat();
		ptr->twistLimit.high.hardness	  =	GetFloat();
		ptr->twistLimit.high.restitution =	GetFloat();
		ptr->twistLimit.high.value		  =	GetFloat();

		ptr->swingLimit.hardness		  =	GetFloat();
		ptr->swingLimit.restitution	  =	GetFloat();
		ptr->swingLimit.value			  =	GetFloat();

		// flags
		ptr->flags	= GetFlag();

		// axis
		ptr->swingAxis.x =	GetFloat();
		ptr->swingAxis.y =	GetFloat();
		ptr->swingAxis.z =	GetFloat();

		// projection mode
		ptr->projectionMode = (NxJointProjectionMode)GetUnsigned();
	}
}

//==================================================================================
void CoreContainer::GetPointOnLineJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description - note	that is	all	we have	to do with this	joint type!
		GetJointDesc( d );
	}
}

//==================================================================================
void CoreContainer::GetPointInPlaneJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description - note	that is	all	we have	to do with this	joint type!
		GetJointDesc( d );
	}
}

//==================================================================================
void CoreContainer::GetDistanceJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description
		GetJointDesc( d );

		NxDistanceJointDesc *ptr = static_cast<NxDistanceJointDesc * >( d.mDesc );

		// now fill	the	rest of	it in
		// min/max dist
		ptr->minDistance =	GetFloat();
		ptr->maxDistance =	GetFloat();

		// flags
		ptr->flags	= GetFlag();

		// spring desc
		ptr->spring.spring		 = GetFloat();
		ptr->spring.damper		 = GetFloat();
		ptr->spring.targetValue = GetFloat();
	}
}

//==================================================================================
void CoreContainer::GetPulleyJointDesc(	JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description
		GetJointDesc( d );

		NxPulleyJointDesc *ptr = static_cast<NxPulleyJointDesc * >( d.mDesc );

		// now fill	the	rest of	it in
		ptr->distance	= GetFloat();
		ptr->stiffness	= GetFloat();
		ptr->ratio		= GetFloat();
		ptr->flags		= GetFlag();

		// motor desc
		ptr->motor.freeSpin  =	GetBool();
		ptr->motor.maxForce  =	GetFloat();
		ptr->motor.velTarget =	GetFloat();

		// and pulley info
		ptr->pulley[0].x =	GetFloat();
		ptr->pulley[0].y =	GetFloat();
		ptr->pulley[0].z =	GetFloat();
		ptr->pulley[1].x =	GetFloat();
		ptr->pulley[1].y =	GetFloat();
		ptr->pulley[1].z =	GetFloat();
	}
}

//==================================================================================
void CoreContainer::GetFixedJointDesc( JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description - note	that is	all	we have	to do with this	joint type!
		GetJointDesc( d );
	}
}

//==================================================================================
void CoreContainer::GetJointLimitSoftDesc( NxJointLimitSoftDesc &l )
{
	if ( mFp )
	{
		l.value		  =	GetFloat();
		l.restitution =	GetFloat();
		l.spring	  =	GetFloat();
		l.damping	  =	GetFloat();
	}
}

//==================================================================================
void CoreContainer::GetJointDriveDesc( NxJointDriveDesc &d )
{
	if ( mFp )
	{
		d.spring			 = GetFloat();
		d.damping			 = GetFloat();
		d.forceLimit		 = GetFloat();
		d.driveType.bitField = GetFlag();
	}
}

//==================================================================================
void CoreContainer::GetD6JointDesc(	JointDescription &d )
{
	if ( mFp && d.mDesc )
	{
		// first, get normal description
		GetJointDesc( d );

		NxD6JointDesc *ptr = static_cast<NxD6JointDesc * >( d.mDesc );

		// now get joint specific info
		ptr->xMotion	   = (NxD6JointMotion)GetUnsigned();
		ptr->yMotion	   = (NxD6JointMotion)GetUnsigned();
		ptr->zMotion	   = (NxD6JointMotion)GetUnsigned();
		ptr->swing1Motion = (NxD6JointMotion)GetUnsigned();
		ptr->swing2Motion = (NxD6JointMotion)GetUnsigned();
		ptr->twistMotion  = (NxD6JointMotion)GetUnsigned();

		GetJointLimitSoftDesc( ptr->linearLimit );
		GetJointLimitSoftDesc( ptr->swing1Limit );
		GetJointLimitSoftDesc( ptr->swing2Limit );
		GetJointLimitSoftDesc( ptr->twistLimit.low );
		GetJointLimitSoftDesc( ptr->twistLimit.high );

		GetJointDriveDesc( ptr->xDrive );
		GetJointDriveDesc( ptr->yDrive );
		GetJointDriveDesc( ptr->zDrive );
		GetJointDriveDesc( ptr->swingDrive );
		GetJointDriveDesc( ptr->twistDrive );
		GetJointDriveDesc( ptr->slerpDrive );

		GetVec3( ptr->drivePosition );
		GetQuat( ptr->driveOrientation );
		GetVec3( ptr->driveLinearVelocity	);
		GetVec3( ptr->driveAngularVelocity );

		ptr->projectionMode	 = (NxJointProjectionMode)GetUnsigned();
		ptr->projectionDistance = GetFloat();
		ptr->projectionAngle	 = GetFloat();
		ptr->gearRatio			 = GetFloat();
		ptr->flags				 = GetFlag();
	}
}

//==================================================================================
JointDescription *CoreContainer::GetJoint( int whichJoint )
{
	JointDescription *retVal = 0;

	if ( mFp )
	{
		// k, get its type
		int	jointType =	GetInt();

		if ( ( jointType >= NX_JOINT_PRISMATIC ) && ( jointType < NX_JOINT_COUNT ) )
		{
			retVal = new JointDescription;
			if ( retVal )
			{
				// k, get other	info
				switch(	jointType )
				{
					case NX_JOINT_PRISMATIC:
					{				 
						NxPrismaticJointDesc *pjd = new NxPrismaticJointDesc;
						assert( pjd );
						if ( pjd )
						{
							retVal->mDesc = pjd;
							GetPrismaticJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_REVOLUTE:
					{
						NxRevoluteJointDesc	*rjd = new NxRevoluteJointDesc;
						assert( rjd );
						if ( rjd )
						{
							retVal->mDesc = rjd;
							GetRevoluteJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_CYLINDRICAL:
					{
						NxCylindricalJointDesc *cjd	= new NxCylindricalJointDesc;
						assert( cjd );
						if ( cjd )
						{
							retVal->mDesc = cjd;
							GetCylindricalJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_SPHERICAL:
					{
						NxSphericalJointDesc *sjd = new NxSphericalJointDesc;
						assert( sjd );
						if ( sjd )
						{
							retVal->mDesc = sjd;
							GetSphericalJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_POINT_ON_LINE:
					{
						NxPointOnLineJointDesc *poljd = new NxPointOnLineJointDesc;
						assert( poljd );
						if ( poljd )
						{
							retVal->mDesc = poljd;
							GetPointOnLineJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_POINT_IN_PLANE:
					{
						NxPointInPlaneJointDesc	*pipjd = new NxPointInPlaneJointDesc;
						assert( pipjd );
						if ( pipjd )
						{
							retVal->mDesc = pipjd;
							GetPointInPlaneJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_DISTANCE:
					{
						NxDistanceJointDesc	*djd = new NxDistanceJointDesc;
						assert( djd );
						if ( djd )
						{
							retVal->mDesc = djd;
							GetDistanceJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_PULLEY:
					{
						NxPulleyJointDesc *pjd = new NxPulleyJointDesc;
						assert( pjd );
						if ( pjd )
						{
							retVal->mDesc = pjd;
							GetPulleyJointDesc(	*retVal );
						}
					}
					break;

					case NX_JOINT_FIXED:
					{
						NxFixedJointDesc *fjd = new NxFixedJointDesc;
						assert( fjd );
						if ( fjd )
						{
							retVal->mDesc = fjd;
							GetFixedJointDesc( *retVal );
						}
					}
					break;

					case NX_JOINT_D6:
					{
						NxD6JointDesc *d = new NxD6JointDesc;
						assert( d );
						if ( d )
						{
							retVal->mDesc = d;
							GetD6JointDesc(	*retVal );
						}
					}
					break;
				}

				if ( !retVal->mDesc )
				{
					delete retVal;
					retVal = 0;
				}
			}
		}
	}

	return retVal;
}

//==================================================================================
void CoreContainer::GetScene( CoreScene *scene, int whichScene )
{
	if ( mFp &&	scene )
	{
		int	i ;

		// k, first	get	scene description
//		scene->mDesc.collisionDetection   = GetBool();
		scene->mDesc.groundPlane          = GetBool();
		scene->mDesc.boundsPlanes         = GetBool();
		GetVec3( scene->mDesc.gravity );
//		scene->mDesc.broadPhase			  =	(NxBroadPhaseType)GetInt();
		scene->mDesc.timeStepMethod		  =	(NxTimeStepMethod)GetInt();
		scene->mDesc.maxTimestep		  =	GetFloat();
		scene->mDesc.maxIter			  =	GetUnsigned();
#if NX_SDK_VERSION_NUMBER >= 230
		scene->mDesc.simType              = static_cast<NxSimulationType>(GetUnsigned());

#endif

#if NX_SDK_VERSION_NUMBER >= 231

#endif

		// k, did it have limits?
		scene->mDesc.limits = 0;
		if ( GetBool() )
		{
			scene->mDesc.limits = new NxSceneLimits;
			assert( scene->mDesc.limits );

			unsigned act	= GetUnsigned();
			unsigned bodies = GetUnsigned();
			unsigned stat	= GetUnsigned();
			unsigned dyn	= GetUnsigned();
			unsigned joints = GetUnsigned();

			if ( scene->mDesc.limits )
			{
				scene->mDesc.limits->maxNbActors		= act;
				scene->mDesc.limits->maxNbBodies		= bodies;
				scene->mDesc.limits->maxNbStaticShapes	= stat;
				scene->mDesc.limits->maxNbDynamicShapes	= dyn;
				scene->mDesc.limits->maxNbJoints		= joints;
			}
		}
		// did it have maxBounds?
		scene->mDesc.maxBounds = 0;
		if ( GetBool() )
		{
			scene->mDesc.maxBounds = new NxBounds3;
			assert( scene->mDesc.maxBounds );

			float minx = GetFloat();
			float miny = GetFloat();
			float minz = GetFloat();
			float maxx = GetFloat();
			float maxy = GetFloat();
			float maxz = GetFloat();

			if ( scene->mDesc.maxBounds )
			{
				scene->mDesc.maxBounds->min.x = minx;
				scene->mDesc.maxBounds->min.y = miny;
				scene->mDesc.maxBounds->min.z = minx;
				scene->mDesc.maxBounds->max.x = maxx;
				scene->mDesc.maxBounds->max.y = maxy;
				scene->mDesc.maxBounds->max.z = maxz;
			}
		}

		// k, read in number of	materials
		int	numMaterials = GetInt();
		for	( i	= 0; i < numMaterials; ++i )
		{
			NxMaterialDesc *nmd	= new NxMaterialDesc;
			assert(	nmd	);
			float dyn		= GetFloat();
			float stat		= GetFloat();
			float rest		= GetFloat();
			float dynV		= GetFloat();
			float statV		= GetFloat();
			int fcm			= GetInt();
			int rcm			= GetInt();
			float anisx		= GetFloat();
			float anisy		= GetFloat();
			float anisz		= GetFloat();
			unsigned flag	= GetFlag();

			// fill	in the material	description
			if ( nmd )
			{
				nmd->dynamicFriction  =	dyn;
				nmd->staticFriction	  =	stat;
				nmd->restitution	  =	rest;
				nmd->dynamicFrictionV =	dynV;
				nmd->staticFrictionV  =	statV;

				nmd->frictionCombineMode	= (NxCombineMode)fcm;
				nmd->restitutionCombineMode	= (NxCombineMode)rcm;

				// get rest of info
				nmd->dirOfAnisotropy.x = anisx;
				nmd->dirOfAnisotropy.y = anisy;
				nmd->dirOfAnisotropy.z = anisz;
				nmd->flags			   = flag;
			}
#if NX_SDK_VERSION_NUMBER >= 230
			// by default, consider	it as not having a spring description
			nmd->spring					= 0;
#endif
			// does	it have	a spring description?
			if ( GetBool() )
			{
				NxSpringDesc *desc = new NxSpringDesc;
				assert(	desc );
				float spring = GetFloat();
				float damper = GetFloat();
				float target = GetFloat();
				if ( desc )
				{
                    desc->spring		= spring;
					desc->damper		= damper;
					desc->targetValue	= target;
				}
			}

			// save	off	this material in this scene
			scene->mMaterials.push_back( nmd );
		}

		// now on towards actors
		int	numActors =	(int)GetUnsigned();
		for	( i	= 0; i < numActors;	++i	)
		{
			NxActorDesc	*nad = new NxActorDesc;
			assert(	nad );
			if ( nad )
			{
				// get the actor description
				GetActorDesc( nad );
			}
			else
			{
				NxActorDesc tmp;
				GetActorDesc( &tmp );
			}

			// now read	actor shapes
			int	numActorShapes = GetInt();
			for	( int j	= 0; j < numActorShapes; ++j )
			{
				NxShapeDesc	*shape = GetActorShape();
				nad->shapes.push_back( shape );
			}

			scene->mActors.push_back( nad );
		}

		scene->mPairCount =	GetInt();
		if ( scene->mPairCount )
		{
			assert(	scene->mPairFlagArray == 0 );
			scene->mPairFlagArray =	new	PairFlagCapsule[scene->mPairCount];
			assert(	scene->mPairFlagArray );
			// now get the pair	flags
			for	( i	= 0; i < scene->mPairCount;	++i	)
			{
				scene->mPairFlagArray[i].mFlag = GetInt();

				// note	that these are indices to the actors!!
				scene->mPairFlagArray[i].mActorIndex1 =	GetInt();
				scene->mPairFlagArray[i].mShapeIndex1 =	GetInt();
				scene->mPairFlagArray[i].mActorIndex2 =	GetInt();
				scene->mPairFlagArray[i].mShapeIndex2 =	GetInt();
			}
		}

		// now get # of	joints
		int	jointCount = GetInt();
		if ( jointCount	)
		{
			for	( i	= 0; i < jointCount; ++i )
			{
				scene->mJointArray.push_back( GetJoint(	i ) );
			}
		}

		int effectorCount = GetInt();
		if ( effectorCount )
		{
			// k, we have some effectors - but please note that we cannot do anything
			// with them yet, as the API for saveToDesc() for them is broken, and thus
			// we do not know what 2 actors to associate them with - but get the info anyways
			for ( i = 0; i < effectorCount; ++i )
			{
				NxSpringAndDamperEffectorDesc *desc = 0;
				if ( GetBool() )
				{
					NxVec3 pos1, pos2;
					float fVals[9];

					desc = new NxSpringAndDamperEffectorDesc;
					assert( desc );

					// get the info
					int bodyIndex1 = GetInt();
					int bodyIndex2 = GetInt();
					GetVec3( pos1 );
					GetVec3( pos2 );
					for ( int j = 0; j < 9; ++j )
					{
						fVals[j] = GetFloat();
					}

					if ( desc )
					{
						// k, this is a spring and linear damper effector - first set actor indices
						desc->body1 = (NxActor *)bodyIndex1;
						desc->body2 = (NxActor *)bodyIndex2;

						// set position of first and second connection points
						desc->pos1 = pos1;
						desc->pos2 = pos2;

						// k, now set information about the linear spring
						desc->springDistCompressSaturate = fVals[0];  // springDistCompressSaturate
						desc->springDistRelaxed          = fVals[1];  // springDistRelaxed
						desc->springDistStretchSaturate  = fVals[2];  // springDistStretchSaturate
						desc->springMaxCompressForce     = fVals[3];  // springMaxCompressForce
						desc->springMaxStretchForce      = fVals[4];  // springMaxStretchForce

						// k, now set information about the linear damper
						desc->damperVelCompressSaturate  = fVals[5];  // damperVelCompressSaturate
						desc->damperVelStretchSaturate   = fVals[6];  // damperVelStretchSaturate
						desc->damperMaxCompressForce     = fVals[7];  // damperMaxCompressForce
						desc->damperMaxStretchForce      = fVals[8];  // damperMaxStretchForce
					}

					scene->mEffectorArray.push_back( desc );
				}
			}
		}

		if ( 0 )
		{
//#pragma message("TODO: Implement binary core dump	load for 'Fluids'")
		//count	= scene->getNbFluids();
		}

	}
}

//==================================================================================
CoreContainer::CoreContainer( void ) :
	mPhysicsSDK( 0 ),
	mFp( 0 ),
	mCallback( 0 )
{
}

//==================================================================================
CoreContainer::~CoreContainer( void	)
{
	// now delete any items	we have	allocated
	ClearAllData();
}

//==================================================================================
const char * CoreContainer::GetStringPntr( const char *str )
{
	const char *allocatedStr = 0;

	if ( str )
	{
		bool found = false;

		// for now don't sweat it not being sorted alphabetically, as strings can come
		// in in any order
		for ( unsigned i = 0; !found && (i < mConsistentStrings.size()); ++i )
		{
			char *current = mConsistentStrings[i];

			if ( !stricmp( str, current ) )
			{
				found		 = true;
				allocatedStr = current;
			}
		}

		if ( !found )
		{
			int len      = strlen( str ) + 1;
			char *newStr = new char[len];
			if ( newStr )
			{
				strcpy( newStr, str );
				allocatedStr = newStr;
				mConsistentStrings.push_back( newStr );
			}
		}
	}

	return allocatedStr;
}

//==================================================================================
void CoreContainer::ClearAllStringPntrs( void )
{
	for ( unsigned i = 0; i < mConsistentStrings.size(); ++i )
	{
		char *current = mConsistentStrings[i];
		if ( current )
		{
			delete current;
		}
	}
	mConsistentStrings.clear();
}

//==================================================================================
void CoreContainer::SaveAsPsCommand( int argc, const char **argv, AsciiCoreDump *helper )
{
	char *charArray = 0;
	for ( int i = 0; i < argc; ++i )
	{
		bool addSpace = false;
		int len = strlen( argv[i] );

		if ( !helper->IsHardSymbol( argv[i] ) )
		{
			++len;	
			addSpace = true;
		}

		// now add 1 for the null terminator
		++len;

		if ( charArray )
		{
			int currLen = strlen( charArray );
			char *tmp = new char[len + currLen];
			if ( tmp )
			{
				memset( tmp, 0, sizeof(char)*(len+currLen) );
				strcpy( tmp, charArray );
				delete [] charArray;
				charArray = tmp;
				strcat( charArray+currLen, argv[i] );
				if ( addSpace )
				{
					charArray[len+currLen-2] = ' ';
				}
			}
		}
		else
		{
			charArray = new char[len];
			memset( charArray, 0, sizeof(char)*(len) );
			strcpy( charArray, argv[i] );
			if ( addSpace )
			{
				charArray[len-2] = ' ';
			}
		}
	}

	if ( charArray )
	{
		mCommands.push_back( charArray );
	}
}

//==================================================================================
void CoreContainer::ClearAllData( void )
{
	unsigned i;

	// delete triangle meshes
	for	( i	= 0; i < mTriangleMeshes.size(); ++i )
	{
		TriangleMeshData *tmd = mTriangleMeshes[i];
		if ( tmd )
		{
			delete [] tmd->mTmd.pmap;
			delete [] tmd->mTmd.points;
			delete [] tmd->mTmd.triangles;
			delete [] tmd->mTmd.materialIndices;
			delete tmd;
		}
	}
	mTriangleMeshes.clear();

	// delete convex meshes
	for	( i	= 0; i < mConvexMeshes.size(); ++i )
	{
		ConvexMeshData *cmd = mConvexMeshes[i];
		if ( cmd )
		{
			delete [] cmd->mCmd.points;
			delete [] cmd->mCmd.triangles;
			delete cmd;
		}
	}
	mConvexMeshes.clear();

	// delete any height field stuff
	for ( i = 0; i < mHeightFields.size(); ++i )
	{
		HeightFieldData *hfd = mHeightFields[i];
		if ( hfd )
		{
#if NX_SDK_VERSION_NUMBER >= 240
			delete [] hfd->mHfd.samples;
#endif
		}
	}
	mHeightFields.clear();

	// delete core scenes
	for	( i	= 0; i < mScenes.size(); ++i )
	{
		CoreScene *cs = mScenes[i];
		if ( cs )
		{
			delete cs;
		}
	}
	mScenes.clear();

	for ( i = 0; i < mCommands.size(); ++i )
	{
		delete [] mCommands[i];
	}
	mCommands.clear();

	mFp = 0;
}


//==================================================================================
// load	a core dump...
//==================================================================================
bool CoreContainer::LoadCoreDump( const char *fname )
{
	return LoadCoreDump( 0, fname );
}

//==================================================================================
// load	a core dump...
//==================================================================================
bool CoreContainer::LoadCoreDump( NxPhysicsSDK *sdk, const char *fname )	
{
	bool ret = false;

	char scratch[512];
	strcpy(scratch,fname);
	strlwrPortable(scratch);

	if ( 1 ) // initialize all of the parameters to zero!
	{
		memset(mParameters,0,sizeof(mParameters));
	}

	if ( strstr( scratch, ".cda" ) != 0 )
	{
		AsciiCoreDump acd( sdk, this );
		ret = acd.Load( fname );
	}
	else
	{
  		mFp = fopen( fname, "rb" );

  		if ( mFp )
  		{
  			int	version	= GetInt();

  			unsigned int coreVersion = NxGetCoreVersion(sdk);

  			if ( version ==	coreVersion ) // if	the	same version of	core dump we support...
  			{
  				ret	= true;

  				// k, so far so	good - versions	match
  				// read	in the # of	parameters (better match NX_PARAMS_NUM_VALUES)
  				int	numParams =	GetInt();
  				assert(	numParams == NX_PARAMS_NUM_VALUES );

  				// now read	in the parameters (note	this is	more for validating	here)
  				int	j;
  				unsigned i;
  				for	( j	= 0; j < numParams;	++j	)
  				{
  					NxReal fileVal = GetFloat();

  					mParameters[j] = fileVal;
  					// here	we would compare the values...
  					//NxParameter p	= (NxParameter)	j;
  					//NxReal v		= sdk->getParameter( p );
  				}

  				// k, read triangle	mesh info
  				unsigned numTriangleMeshes = GetInt();
  				for	( i	= 0; i < numTriangleMeshes;	++i	)
  				{
  					// create a	new	triangle mesh desc
					TriangleMeshData *tmd = new TriangleMeshData;
  					assert(	tmd	);

  					if ( tmd )
  					{
  						// get its information
  						GetTriangleMeshDesc( tmd, i );
  					}
  					else
  					{
						TriangleMeshData t;
  						GetTriangleMeshDesc( &t, i );
  					}

  					// push	it back
  					mTriangleMeshes.push_back( tmd );
  				}

  				// k, read the convex mesh info
  				unsigned numConvexMeshes = GetInt();
  				for	( i	= 0; i < numConvexMeshes; ++i )
  				{
  					// create a	new	convex mesh	desc
					ConvexMeshData *cmd = new ConvexMeshData;
  					assert(	cmd	);

  					if ( cmd )
  					{
  						// get its information
  						GetConvexMeshDesc( cmd, i );
  					}
  					else
  					{
						ConvexMeshData c;

  						// get its information
  						GetConvexMeshDesc( &c, i );
  					}

  					// push	it back
  					mConvexMeshes.push_back( cmd );
  				}

  				unsigned numSkeletons =	GetInt();
  				/**	TODO : fill	this in	- right	now	does nothing
  				for	( i	= 0; i < numSkeletons; ++i )
  				{
  					WriteSkeleton_b( mSkeletons[i] );
  				}
  				**/

#if NX_SDK_VERSION_NUMBER >= 240
				// get number of height fields
				unsigned numHeightFields = GetInt();
  				for	( i	= 0; i < numHeightFields;	++i	)
  				{
  					// create a	new	triangle mesh desc
					HeightFieldData *hfd = new HeightFieldData;
  					assert(	hfd	);

  					if ( hfd )
  					{
  						// get its information
  						GetHeightFieldDesc( hfd, i );
  					}
  					else
  					{
						HeightFieldData h;
  						GetHeightFieldDesc( &h, i );
  					}

  					// push	it back
  					mHeightFields.push_back( hfd );
  				}

#endif
  				// now we need to read in the number of	scenes
  				int	numScenes =	GetInt();

  				// now read	in each	scene
  				for	(int i = 0;	i <	numScenes; ++i )
  				{
  					// create a	new	scene
  					CoreScene *scene = new CoreScene;
  					assert(	scene );

  					if ( scene )
  					{
  						// obtain the scene's specifics
  						GetScene( scene, i );
  					}
  					else
  					{
  						CoreScene cs;
  						GetScene( &cs, i );
  					}

  					// "save" it
  					mScenes.push_back( scene );
  				}
  			}
  			fclose( mFp );
  			mFp = 0;
  		}
	}

	return ret;
}

//==================================================================================
CoreScene::CoreScene(void) : mPairFlagArray( 0 )
{
}

//==================================================================================
CoreScene::~CoreScene(void)
{
	unsigned i;

	// delete all of our materials,	etc.
	for	( i	= 0; i < mMaterials.size();	++i	)
	{
		NxMaterialDesc *md = mMaterials[i];
		if ( md )
		{
#if NX_SDK_VERSION_NUMBER >= 230
			// delete any spring desc
			if ( md->spring )
			{
				delete md->spring;
				md->spring = 0;
			}
#endif
			// delete material desc
			delete md;
		}
	}
	mMaterials.clear();

	// no delete all of	our	actor descs
	for	( i	= 0; i < mActors.size(); ++i )
	{
		NxActorDesc *ad = mActors[i];
		if ( ad )
		{
			// don't have to delete their name - got it from a StringRef
			/**
			if ( ad->name )
			{
				delete [] ad->name;
				ad->name = 0;
			}
			**/

			// delete any body desc
			if ( ad->body )
			{
				delete ad->body;
				ad->body = 0;
			}

			// delete the shape desc's
			int	shapeSize = ad->shapes.size();
			for	( int j	= 0; j < shapeSize;	++j	)
			{
				NxShapeDesc	*shaped	= ad->shapes[j];
				if ( shaped	)
				{
					delete shaped;
				}
			}
			ad->shapes.clear();
			
			// delete actor desc
			delete ad;
		}
	}
	mActors.clear();

	// delete joint descs
	for ( i = 0; i < mJointArray.size(); ++i )
	{
		JointDescription *jd = mJointArray[i];
		if ( jd )
		{
			// don't have to delete their name - got it from a StringRef
			/**
			if ( jd->mDesc->name )
			{
				delete [] jd->mDesc->name;
				jd->mDesc->name = 0;
			}
			**/

			if ( jd->mPlaneInfo.size() )
			{
				for ( unsigned z = 0; z < jd->mPlaneInfo.size(); ++z )
				{
					PlaneInfo *info = jd->mPlaneInfo[z];
					if ( info )
					{
						delete info;
					}
				}
				jd->mPlaneInfo = 0;
			}

			// delete NxJointDesc
			delete jd->mDesc;

			// delete joint description
			delete jd;
		}
	}
	mJointArray.clear();

	// delete the pair flag array
	delete [] mPairFlagArray;
	mPairFlagArray = 0;
	mPairCount = 0;

	// delete spring and damper effectors descs
	for ( i = 0; i < mEffectorArray.size(); ++i )
	{
		NxSpringAndDamperEffectorDesc *ed = mEffectorArray[i];
		if ( ed )
		{
			delete ed;
		}
	}
	mEffectorArray.clear();
}

//==================================================================================
TriangleMeshData::TriangleMeshData(void) : 
	mCookedData( 0 ),
	mCookedDataSize( 0 )
{
}

//==================================================================================
TriangleMeshData::~TriangleMeshData(void)
{
	if ( mCookedData )
	{
		delete [] mCookedData;
		mCookedData = 0;
	}
	mCookedDataSize = 0;
}

//==================================================================================
void TriangleMeshData::setToDefault( void )
{
	mTmd.setToDefault();
}

//==================================================================================
ConvexMeshData::ConvexMeshData(void) : 
	mCookedData( 0 ),
	mCookedDataSize( 0 )
{
}

//==================================================================================
ConvexMeshData::~ConvexMeshData(void)
{
	if ( mCookedData )
	{
		delete [] mCookedData;
		mCookedData = 0;
	}
	mCookedDataSize = 0;
}

//==================================================================================
void ConvexMeshData::setToDefault( void )
{
	mCmd.setToDefault();
}

//==================================================================================
HeightFieldData::HeightFieldData(void)
{
}

//==================================================================================
HeightFieldData::~HeightFieldData(void)
{
}

//==================================================================================
void HeightFieldData::setToDefault(void)
{
#if NX_SDK_VERSION_NUMBER >= 240
	mHfd.setToDefault();
#endif
}

//==================================================================================
CoreScene *CoreContainer::GetCoreSceneInfo( unsigned which )
{
	CoreScene *ret = 0;
	if ( which < mScenes.size() )
	{
		ret = mScenes[which];
	}
	return ret;	
}

//==================================================================================
NxScene	* CoreContainer::InstantiateCoreDump( NxPhysicsSDK *sdk, CoreUserNotify	*callback,CoreUserScene *userScene,NxVec3 *offset,SceneType stype)
{
	NxScene	*ret = 0;

	NxArray< NxTriangleMesh	* >	triMeshes;
	NxArray< NxConvexMesh *	>	conMeshes;
#if NX_SDK_VERSION_NUMBER >= 240
	NxArray< NxHeightField * >  heightFields;
#endif
	NxArray< NxScene * >		scenes;

	// k, now we need to instantiate a new scene (this will	only return	the	first
	// one btw,	since we can actually load multiple	scenes)
	mCallback =	callback;
	if ( !mPhysicsSDK )
	{
		mPhysicsSDK	= sdk;
	}

	// first set params
	unsigned i;
	for	( i	= 0; i < NX_PARAMS_NUM_VALUES; ++i )
	{
		#if SMOKETEST
		NxPhysicsSDK_setParameter(sdk, (NxParameter)i, mParameters[i] );
		#else
		sdk->setParameter( (NxParameter)i, mParameters[i] );
		#endif
	}

	// now load	up any tri meshes
	unsigned triSize = mTriangleMeshes.size();
	for	( i	= 0; i < triSize; ++i )
	{
		NxTriangleMesh *tm = 0;

		TriangleMeshData *tmd = mTriangleMeshes[i];
		if ( tmd )
		{
			// k, do we need to cook the triangle mesh?
			if ( !tmd->mCookedData )
			{
				// SRM : if	this tmd has a pmap, we	need to	zero it	out	here, then put it back
				NxPMap *pmap = tmd->mTmd.pmap;
				tmd->mTmd.pmap = 0;

				bool status	= true;
				NxInitCooking();

				//on some platforms we cannot write to disk so it is better to use a memory buffer for cooking.
				MemoryWriteBuffer writeBuffer;
				status = NxCookTriangleMesh( tmd->mTmd, writeBuffer );
				
				tmd->mTmd.pmap = pmap;

				if ( status	)
				{
					#if SMOKETEST
					tm = NxPhysicsSDK_createTriangleMesh(sdk, MemoryReadBuffer(writeBuffer.data));
					#else
					tm = sdk->createTriangleMesh(MemoryReadBuffer(writeBuffer.data));
					#endif

					if ( tmd->mTmd.pmap )
					{
						bool ok = tm->loadPMap( *tmd->mTmd.pmap );
						if ( ok )
						{
							// k, do anything?
							int s = 0;
						}
					}
				}
			}
			else
			{
				#if SMOKETEST
				tm = NxPhysicsSDK_createTriangleMesh(sdk, MemoryReadBuffer((NxU8 *)tmd->mCookedData));
				#else
				tm = sdk->createTriangleMesh(MemoryReadBuffer((NxU8 *)tmd->mCookedData));
				#endif
			}
			triMeshes.push_back( tm	);
		}
	}

	// now load	up any convex meshes
	unsigned conSize = mConvexMeshes.size();
	for	( i	= 0; i < conSize; ++i )
	{
		NxConvexMesh *cm = 0;

		ConvexMeshData *cmd = mConvexMeshes[i];
		if ( cmd )
		{
			if ( !cmd->mCookedData )
			{
				bool status	= true;
				NxInitCooking();

				//on some platforms we cannot write to disk so it is better to use a memory buffer for cooking.
				MemoryWriteBuffer writeBuffer;
				status = NxCookConvexMesh( cmd->mCmd, writeBuffer);

				if ( status	)
				{
					#if SMOKETEST
					cm = NxPhysicsSDK_createConvexMesh(sdk,MemoryReadBuffer(writeBuffer.data));
					#else
					cm = sdk->createConvexMesh(MemoryReadBuffer(writeBuffer.data));
					#endif
				}
			}
			else
			{
				#if SMOKETEST
				cm = NxPhysicsSDK_createConvexMesh(sdk, MemoryReadBuffer((NxU8 *)cmd->mCookedData));
				#else
				cm = sdk->createConvexMesh(MemoryReadBuffer((NxU8 *)cmd->mCookedData));
				#endif
			}

			conMeshes.push_back( cm	);
		}
	}

	// now create any height fields
	unsigned hfSize = mHeightFields.size();
	for	( i	= 0; i < hfSize; ++i )
	{
		HeightFieldData *hfd = mHeightFields[i];
		if ( hfd )
		{
#if NX_SDK_VERSION_NUMBER >= 240
			// create height field from its description
			NxHeightField *hf = 0;

#if SMOKETEST
			hf = NxPhysicsSDK_createHeightField( sdk, hfd->mHfd );
#else
			hf = sdk->createHeightField( hfd->mHfd );
#endif // SMOKETEST

			// save it off
			heightFields.push_back( hf );
#endif
		}
	}

	// now for scenes
	for (unsigned int sno=0; sno<mScenes.size(); sno++)
	{
		// k, get the first	core scene
		CoreScene *coreScene = mScenes[sno];

		switch ( stype )
		{
			case ST_FORCE_SOFTWARE:
#if NX_SDK_VERSION_NUMBER >= 230
				coreScene->mDesc.simType = NX_SIMULATION_SW;
#endif
				break;
			case ST_FORCE_HARDWARE:
#if NX_SDK_VERSION_NUMBER >= 230
				coreScene->mDesc.simType = NX_SIMULATION_HW;
#endif
				break;
		}

		// try to create the scene
		NxScene *newScene = 0;
		if ( userScene )
		{
			newScene = userScene->CoreCreateScene(sdk,coreScene->mDesc);
		}
		else
		{
  		#if SMOKETEST
  		newScene =	NxPhysicsSDK_createScene(sdk, coreScene->mDesc );
  		#else
  		newScene =	sdk->createScene( coreScene->mDesc );
  		#endif
  	}

		assert(	newScene );

		if ( newScene )
		{
			if ( mCallback )
			{
				mCallback->CoreNotifyScene(newScene);
			}

			// fill	in the mesh	data for all actors	now	that we	have our convex	meshes!
			for	(i = 0;	i <	coreScene->mActors.size(); ++i )
			{
				NxActorDesc	*ad	= coreScene->mActors[i];
				int	shapeSize	= ad->shapes.size();
				for	( int j	= 0; j < shapeSize;	++j	)
				{
					NxShapeDesc	*shaped	= ad->shapes[j];
					if ( shaped	)
					{
						if ( shaped->getType() == NX_SHAPE_CONVEX )
						{
							NxConvexShapeDesc *altDesc = (NxConvexShapeDesc	*)shaped;
							altDesc->meshData =	conMeshes[(unsigned)altDesc->meshData];
						}
						else if	( shaped->getType()	== NX_SHAPE_MESH )
						{
							NxTriangleMeshShapeDesc	*altDesc = (NxTriangleMeshShapeDesc	*)shaped;
							altDesc->meshData =	triMeshes[(unsigned)altDesc->meshData];
						}
#if NX_SDK_VERSION_NUMBER >= 240
						else if ( shaped->getType() == NX_SHAPE_HEIGHTFIELD ) 
						{
							NxHeightFieldShapeDesc *altDesc = (NxHeightFieldShapeDesc *)shaped;
							altDesc->heightField = heightFields[(unsigned)altDesc->heightField];
						}
#endif
					}
				}
			}

			// k, we created (or rather	we cooked) all the tri meshes and convex meshes, so
			// now let us fill in the scene	- start	with materials
			for	( i	= 0; i < coreScene->mMaterials.size(); ++i )
			{
				NxMaterialDesc *md = coreScene->mMaterials[i];

				// k, if we	are	material 0,	then we	need to	set	information, as	we always
				// have	a default material present (which is material 0)
				if ( i == 0	)
				{
					#if SMOKETEST
					NxMaterial *mat = NxScene_getMaterialFromIndex(newScene,0);
					#else
					NxMaterial *mat	= newScene->getMaterialFromIndex( 0	);
					#endif
					assert(	mat	);
					mat->loadFromDesc( *md );
				}
				else
				{
					#if SMOKETEST
					NxScene_createMaterial( newScene, *md );
					#else
					newScene->createMaterial( *md );
					#endif
				}
			}

			// now handle creation of all actors
			NxArray< NxActor * > actors;

			for	( i	= 0; i < coreScene->mActors.size();	++i	)
			{
				NxActorDesc	*ad = coreScene->mActors[i];
				if ( ad )
				{
					if ( offset )
					{
						ad->globalPose.t += *offset;
					}

					bool valid = ad->isValid();
					assert(	valid );
					#if SMOKETEST
					NxActor	*act = NxScene_createActor(newScene, *ad );
					#else
					NxActor	*act = newScene->createActor( *ad );
					#endif
					assert(	act	);
					if ( act && mCallback )
					{
						mCallback->CoreNotifyActor(act);
					}
					actors.push_back( act );
				}
			}

			// now create pair flags
			int	actorSize  = (int)actors.size();
			for	( i	= 0; i < (unsigned)coreScene->mPairCount; ++i )
			{
				int	actorIndex1	= coreScene->mPairFlagArray[i].mActorIndex1;
				int	actorIndex2	= coreScene->mPairFlagArray[i].mActorIndex2;
				int	shapeIndex1	= coreScene->mPairFlagArray[i].mShapeIndex1;
				int	shapeIndex2	= coreScene->mPairFlagArray[i].mShapeIndex2;

				if ( ( actorIndex1 >= 0	) && ( actorIndex1 < actorSize ) &&
						( actorIndex2 >= 0 ) &&	( actorIndex2 <	actorSize )	)
				{
					NxActor	*actor1	= actors[actorIndex1];
					NxActor	*actor2	= actors[actorIndex2];					  

					if ( ( shapeIndex1 == -1 ) && (	shapeIndex2	== -1 )	)
					{
#if SMOKETEST
						NxScene_setActorPairFlags(newScene, *actor1, *actor2, coreScene->mPairFlagArray[i].mFlag );
#else
						newScene->setActorPairFlags( *actor1, *actor2, coreScene->mPairFlagArray[i].mFlag );
#endif
					}
					else
					{
						NxShape	*shape1	= 0;
						NxShape	*shape2	= 0;

						// k, get shape	pntrs
						NxShape	*const *s1 = actor1->getShapes();
						NxShape	*const *s2 = actor2->getShapes();
						int	num1 = actor1->getNbShapes();
						int	num2 = actor2->getNbShapes();
						if ( ( shapeIndex1 < num1 )	&& ( shapeIndex2 < num2	) )
						{
							shape1 = s1[shapeIndex1];
							shape2 = s2[shapeIndex2];
						}

						if ( shape1	&& shape2 )
						{
							unsigned flag = coreScene->mPairFlagArray[i].mFlag;
							unsigned flag1 = flag & NX_IGNORE_PAIR;
							unsigned flag2 = flag & ~NX_IGNORE_PAIR;
							
							if ( flag1 )
							{
#if SMOKETEST
								NxScene_setShapePairFlags(newScene, *shape1, *shape2, flag1 );
#else
								newScene->setShapePairFlags( *shape1, *shape2, flag1 );
#endif
							}

							// k, now set the other flags (if necessary)
							if ( flag2 )
							{
#if SMOKETEST
								NxScene_setActorPairFlags( newScene, *actor1, *actor2, flag2 );
#else
								newScene->setActorPairFlags( *actor1, *actor2, flag2 );
#endif
							}
						}
					}
				}
			}

			// now create the joints
			unsigned jointSize = coreScene->mJointArray.size();
			for	( i	= 0; i < jointSize;	++i	)
			{
				JointDescription *d = coreScene->mJointArray[i];
				if ( d && d->mDesc )
				{
					// k, we have a	joint description -	does it	have valid actor refs (when
					// we get the actor	info, it saves out indices as to what actor
					// they	belong to, so here we convert the index	into the actual	actor)?
					int	actorRef1 =	(int)d->mDesc->actor[0];
					int	actorRef2 =	(int)d->mDesc->actor[1];

					if ( ( actorRef1 ==	-1 ) ||
							( (unsigned int)actorRef1 >= actors.size()	) )
					{
						// I do	believe	this means that	joint is connected to the world
						d->mDesc->actor[0]	= (NxActor *)0;
					}
					else
					{
						d->mDesc->actor[0]	= actors[actorRef1];						
					}

					if ( ( actorRef2 ==	-1 ) ||
							( (unsigned int)actorRef2 >= actors.size()	) )
					{
						// I do	believe	this means that	joint is connected to the world
						d->mDesc->actor[1]	= (NxActor *)0;
					}
					else
					{
						d->mDesc->actor[1]	= actors[actorRef2];
					}

					// k, now try to create	the	joint
					#if SMOKETEST
					NxJoint	*newJoint =	NxScene_createJoint(newScene, *d->mDesc );
					#else
					NxJoint	*newJoint =	newScene->createJoint( *d->mDesc );
					#endif

					if ( newJoint )
					{
						newJoint->setName( d->mDesc->name );
						if ( d->mPlaneInfo.size() )
						{
							newJoint->setLimitPoint( d->mPlaneLimitPoint, d->mOnActor2 );
						}

						// do we want to do	anything with it - yes, if it has limits!
						for ( unsigned z = 0; z < d->mPlaneInfo.size(); ++z )
						{
							PlaneInfo *info = d->mPlaneInfo[z];
							if ( info )
							{
								NxVec3 ptOnPlane;
								ptOnPlane.x = info->mPlaneNormal.x * info->mPlaneD;
								ptOnPlane.y = info->mPlaneNormal.y * info->mPlaneD;
								ptOnPlane.z = info->mPlaneNormal.z * info->mPlaneD;
								newJoint->addLimitPlane( info->mPlaneNormal, info->mWorldLimitPt );
							}
						}
					}
				}
			}

			// and now create the effectors
			unsigned effSize = coreScene->mEffectorArray.size();
			for ( i = 0; i < effSize; ++i )
			{
				NxSpringAndDamperEffectorDesc *d = coreScene->mEffectorArray[i];
				if ( d )
				{
					// k, got a spring and damper effector desc - get the actor ref's
					int actorRef1 = (int)d->body1;
					int actorRef2 = (int)d->body2;
					if ( ( actorRef1 == -1 ) ||
							( (unsigned int)actorRef1 >= actors.size() ) )
					{
						d->body1 = (NxActor *)0;
					}
					else
					{
						d->body1 = actors[actorRef1];
					}

					if ( ( actorRef2 == -1 ) ||
							( (unsigned int)actorRef2 >= actors.size() ) )
					{
						d->body2 = (NxActor *)0;
					}
					else
					{
						d->body2 = actors[actorRef2];
					}

					// now, try to create the effector
					#if SMOKETEST
					NxSpringAndDamperEffector *eff = NxScene_createSpringAndDamperEffector(newScene, *d );
					#else
					NxSpringAndDamperEffector *eff = newScene->createSpringAndDamperEffector( *d );
					#endif

					if ( eff )
					{
						// do we want to do anything with it?
					}
				}
			}
		}

		// return the first	scene (if it exists)
		if ( sno == 0 )
		{
  		ret	= newScene;
  	}

	}

	mCallback =	0;

	// return the correct scene	(may not exist btw)
	return ret;
}

//==================================================================================
bool CoreContainer::Compare( const CoreContainer &diff,	CoreCompareNotify *callback	, float tolerance)
{
	int	diffcount =	0;

	//*****************
	// 1) k, first	go through the parameters and see if they are different
	unsigned i;
	for	( i=0; i<NX_PARAMS_NUM_VALUES; ++i)
	{
		// for floating	point values, we do	a difference check,	as sometimes
		// floating	point direct comparisons fail
		if ( FloatDiff( mParameters[i], diff.mParameters[i], tolerance ) )
		{
			// do what we got to do
			++diffcount;
			if ( callback && callback->CoreDiff( (NxParameter) i, mParameters[i], diff.mParameters[i] ) )
			{
				return false;
			}
		}
	}

	//*****************
	// 2) now do triangle meshes
	if ( mTriangleMeshes.size() != diff.mTriangleMeshes.size() )
	{
		// do what we got to do
		++diffcount;
		if ( callback && callback->CoreDiff( CD_NUM_TRIANGLE_MESHES, mTriangleMeshes.size(), diff.mTriangleMeshes.size() ) )
		{
			return false;
		}
	}
	else
	{
		// check the triangle meshes
		unsigned triSize = mTriangleMeshes.size();
		for ( i = 0; i < triSize; ++i )
		{
			// k, get each triangle mesh desc
			TriangleMeshData *ta = mTriangleMeshes[i];
			TriangleMeshData *tb = diff.mTriangleMeshes[i];
			if ( ta && tb )
			{
				NxTriangleMeshDesc *t1 = &ta->mTmd;
				NxTriangleMeshDesc *t2 = &tb->mTmd;

				if ( CompareTriMeshDescriptions( t1, t2, callback, diffcount, tolerance ) )
				{
					return false;
				}
			}
		}
	}

	//*****************
	// 3) now do convex meshes
	if ( mConvexMeshes.size() != diff.mConvexMeshes.size() )
	{
		// do what we got to do
		++diffcount;
		if ( callback && callback->CoreDiff( CD_NUM_CONVEX_MESHES, mConvexMeshes.size(), diff.mConvexMeshes.size() ) )
		{
			return false;	
		}
	}
	else
	{
		unsigned conSize = mConvexMeshes.size();
		for ( i = 0; i < conSize; ++i )
		{
			ConvexMeshData *ca = mConvexMeshes[i];
			ConvexMeshData *cb = diff.mConvexMeshes[i];
			if ( ca && cb )
			{
				NxConvexMeshDesc *c1 = &ca->mCmd;
				NxConvexMeshDesc *c2 = &cb->mCmd;

				if ( CompareConvexMeshDescriptions( c1, c2, callback, diffcount, tolerance ) )
				{
					return false;
				}
			}
		}
	}

	if ( mHeightFields.size() != diff.mHeightFields.size() )
	{
		// indicate difference
		++diffcount;
		if ( callback && callback->CoreDiff( CD_HEIGHT_FIELD_NUM, mHeightFields.size(), diff.mHeightFields.size() ) )
		{
			return false;
		}
	}
	else
	{
		unsigned size = mHeightFields.size();
		for ( i = 0; i < size; ++i )
		{
			HeightFieldData *ha = mHeightFields[i];
			HeightFieldData *hb = diff.mHeightFields[i];
			if ( ha && hb )
			{
#if NX_SDK_VERSION_NUMBER >= 240
				NxHeightFieldDesc *h1 = &ha->mHfd;
				NxHeightFieldDesc *h2 = &hb->mHfd;

				if ( CompareHeightFieldDescriptions( h1, h2, callback, diffcount, tolerance ) )
				{
					return false;
				}
#endif
			}
		}
	}

	//*****************
	// 4) now do core scenes
	if ( mScenes.size() != diff.mScenes.size() )
	{
		// increment the difference	counter
		++diffcount;

		// check if	we should continue or not
		if ( callback && callback->CoreDiff( CD_NUM_CORE_SCENES, mScenes.size(), diff.mScenes.size() ) )
		{
			return false;
		}
	}
	else
	{
		// k, we got the same number of scenes, so see if they differ in any way
		unsigned sceneSize = mScenes.size();

		for ( i = 0; i < sceneSize; ++i )
		{
			// k, check if they differ or not
			CoreScene *c1 = mScenes[i];
			CoreScene *c2 = diff.mScenes[i];
			if ( CompareSceneDescriptions( c1, c2, callback, diffcount, tolerance ) )
			{
				return false;
			}
		}
	}

	return true;
}

//==================================================================================
bool CoreContainer::CompareTriMeshDescriptions( NxTriangleMeshDesc *t1, 
											    NxTriangleMeshDesc *t2, 
											    CoreCompareNotify *callback,
												int &diffcount,
												float tolerance)
{
	bool bail = false;

	// k, I hate to do returns in the middle of a func, but will make it cleaner!
	if ( t1 && t2 )
	{
		// should we check material indices?
		/**
		if ( t1->materialIndices != t2->materialIndices )
		{
			++numDiffs;
		}
		**/

		// does convex edge threshold differ?
		if ( FloatDiff( t1->convexEdgeThreshold, t2->convexEdgeThreshold, tolerance ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( TMD_CET, *t1, *t2 );
			}
		}

		// do flags differ?
		if ( !bail && ( t1->flags != t2->flags ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( TMD_FLAGS, *t1, *t2 );
			}
		}

		// does height field vertical axis differ?
		if ( !bail && ( t1->heightFieldVerticalAxis != t2->heightFieldVerticalAxis ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( TMD_HFVA, *t1, *t2 );
			}
		}

		// does height field extent differ
		if ( !bail && FloatDiff( t1->heightFieldVerticalExtent, t2->heightFieldVerticalExtent, tolerance ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( TMD_HFVE, *t1, *t2 );
			}
		}

		// does material index stride differ
		if ( !bail && ( t1->materialIndexStride != t2->materialIndexStride ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( TMD_MIS, *t1, *t2 );
			}
		}

		// does pmap info differ?
		if ( !bail )
		{
			TriangleMeshDifferences type;
			bool different = false;

			if ( ( t1->pmap && !t2->pmap ) || ( !t1->pmap && t2->pmap ) )
			{
				different = true;
				++diffcount;
				type = TMD_PMAP_MISSING;
			}
			else if ( t1->pmap && t2->pmap && ( t1->pmap->dataSize != t2->pmap->dataSize ) )
			{
				different = true;
				++diffcount;
				type = TMD_PMAP_NUMVERTS;
			}

			if ( different && callback && callback->CoreDiff( type, *t1, *t2 ) )
			{
				bail = true;
			}
		}

		// k, does any of the vert info differ?
		if ( !bail )
		{
			TriangleMeshDifferences type;
			bool different = false;

			if ( t1->numVertices != t2->numVertices )
			{
				++diffcount;
				type = TMD_VERT_NUM;
				different = true;
			}
			else if ( t1->pointStrideBytes != t2->pointStrideBytes )
			{
				++diffcount;
				type = TMD_VERT_STRIDE;
				different = true;
			}
			else if ( memcmp( t1->points, t2->points, t1->numVertices*t1->pointStrideBytes/3 ) != 0 )
			{
				++diffcount;
				type = TMD_VERT_PTS;
				different = true;
			}

			if ( different && callback && callback->CoreDiff( type, *t1, *t2 ) )
			{
				bail = true;
			}			
		}

		// k, does any of the tri info differ?
		if ( !bail )
		{
			TriangleMeshDifferences type;
			bool different = false;
			int triSize = t1->numTriangles*t1->triangleStrideBytes;

			if ( t1->numTriangles != t2->numTriangles )
			{
				++diffcount;
				type = TMD_TRI_NUMVERTS;
				different = true;
			}
			else if ( t1->triangleStrideBytes != t2->triangleStrideBytes )
			{
				++diffcount;
				type = TMD_TRI_STRIDE;
				different = true;
			}
			else if ( memcmp( t1->triangles, t2->triangles, triSize ) != 0 )
			{
				++diffcount;
				type = TMD_TRI_TRIS;
				different = true;
			}

			if ( different && callback && callback->CoreDiff( type, *t1, *t2 ) )
			{
				bail = true;
			}			
		}
	}

	return bail;
}

//==================================================================================
bool CoreContainer::CompareConvexMeshDescriptions( NxConvexMeshDesc *c1, 
												   NxConvexMeshDesc *c2, 
												   CoreCompareNotify *callback,
												   int &diffcount,
												   float tolerance)
{
	bool bail = false;

	if ( c1 && c2 )
	{
		// do flags differ?
		if ( c1->flags != c2->flags )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CMD_FLAGS, *c1, *c2 );
			}
		}

		// k, does any of the vert info differ?
		if ( !bail )
		{
			ConvexMeshDifferences type;
			bool different = false;

			if ( c1->numVertices != c2->numVertices )
			{
				++diffcount;
				type = CMD_VERT_NUM;
				different = true;
			}
			else if ( c1->pointStrideBytes != c2->pointStrideBytes )
			{
				++diffcount;
				type = CMD_VERT_STRIDE;
				different = true;
			}
			else if ( memcmp( c1->points, c2->points, c1->numVertices*c1->pointStrideBytes/3 ) != 0 )
			{
				++diffcount;
				type = CMD_VERT_PTS;
				different = true;
			}

			if ( different && callback && callback->CoreDiff( type, *c1, *c2 ) )
			{
				bail = true;
			}			
		}

		// k, does any of the tri info differ?
		if ( !bail )
		{
			bool different = false;
			ConvexMeshDifferences type;
			int triSize = c1->numTriangles*c1->triangleStrideBytes;

			if ( c1->numTriangles != c2->numTriangles )
			{
				++diffcount;
				type = CMD_TRI_NUMVERTS;
				different = true;
			}
			else if ( c1->triangleStrideBytes != c2->triangleStrideBytes )
			{
				++diffcount;
				type = CMD_TRI_STRIDE;
				different = true;
			}
			else if ( c1->triangles && c2->triangles && 
				    ( memcmp( c1->triangles, c2->triangles, triSize ) != 0 ) )
			{
				++diffcount;
				type = CMD_TRI_TRIS;
				different = true;
			}

			if ( different && callback && callback->CoreDiff( type, *c1, *c2 ) )
			{
				bail = true;
			}			
		}
	}

	return bail;
}

#if NX_SDK_VERSION_NUMBER >= 240
//==================================================================================
bool CoreContainer::CompareHeightFieldDescriptions( NxHeightFieldDesc *h1, NxHeightFieldDesc *h2, CoreCompareNotify *callback, int &diffcount, float tolerance )
{
	bool bail = false;

	if ( h1 && h2 )
	{
		if ( h1->convexEdgeThreshold != h2->convexEdgeThreshold )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_CET_DIFF, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->flags != h2->flags ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_FLAGS, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->format != h2->format ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_FORMAT, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->nbColumns != h2->nbColumns ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_NUM_COLUMNS, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->nbRows != h2->nbRows ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_NUM_ROWS, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->sampleStride != h2->sampleStride ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_SAMPLE_STRIDE, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->verticalExtent != h2->verticalExtent ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_VERTICAL_EXTENT, *h1, *h2 );
			}
		}

		if ( !bail && ( ( !h1->samples && h2->samples ) || ( h1->samples && !h2->samples ) ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_HEIGHT_FIELD_MISSING_SAMPLES, *h1, *h2 );
			}
		}

		if ( !bail && ( h1->samples && h2->samples ) )
		{
			// do a mem compare
			if ( memcmp( h1->samples, h2->samples, sizeof(unsigned) * h1->nbColumns*h1->nbRows ) )
			{
				++diffcount;
				if ( callback )
				{
					bail = callback->CoreDiff( CD_HEIGHT_FIELD_SAMPLE_MEM_DIFF, *h1, *h2 );
				}
			}
		}
	}

	return bail;
}
#endif

//==================================================================================
bool CoreContainer::CompareSceneDescriptions( CoreScene *c1, CoreScene *c2, CoreCompareNotify *callback, int &diffcount, float tolerance )
{
	bool bail = false;

	if ( c1 && c2 )
	{
		// k, now go through materials
		if ( c1->mMaterials.size() != c2->mMaterials.size() )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_NUM_MATERIALS, c1->mMaterials.size(), c2->mMaterials.size() );
			}
		}

		if ( !bail )
		{
			// k, should we go through each material - sure what the hell.  But, we do NOT
			// want to make up a core difference type for each material - if we find a difference,
			// it is just a CD_MATERIAL_TYPE diff
			unsigned matSize = c1->mMaterials.size();
			for ( unsigned i = 0; !bail && (i < matSize); ++i )
			{
				NxMaterialDesc *m1 = c1->mMaterials[i];
				NxMaterialDesc *m2 = c2->mMaterials[i];

				if ( m1 && m2 )
				{
					if ( Vec3Diff( m1->dirOfAnisotropy, m2->dirOfAnisotropy, tolerance ) ||
						 FloatDiff( m1->dynamicFriction,m2->dynamicFriction, tolerance ) || 
						 FloatDiff( m1->dynamicFrictionV, m2->dynamicFrictionV, tolerance ) ||
						 FloatDiff( m1->restitution, m2->restitution, tolerance ) ||
						 FloatDiff( m1->staticFriction, m2->staticFriction, tolerance ) ||
						 FloatDiff( m1->staticFrictionV, m2->staticFrictionV, tolerance ) ||
						 ( m1->frictionCombineMode != m2->frictionCombineMode ) ||
						 ( m1->flags != m2->flags ) ||
						 ( m1->restitutionCombineMode != m2->restitutionCombineMode )
					  )
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_MATERIAL_DESC, i, *m1, *m2 );
						}
					}
#if NX_SDK_VERSION_NUMBER >= 230
					if ( !bail && (( m1->spring && !m2->spring ) || ( !m1->spring && m2->spring )) )
					{
						if ( callback )
						{
							bail = callback->CoreDiff( CD_MATERIAL_DESC, i, *m1, *m2 );				
						}
					}
					if ( !bail && m1->spring && m2->spring && 
							( FloatDiff( m1->spring->damper, m2->spring->damper, tolerance ) || 
							  FloatDiff( m1->spring->spring, m2->spring->spring, tolerance ) ||
							  FloatDiff( m1->spring->targetValue, m2->spring->targetValue, tolerance ) ) )
					{
						if ( callback )
						{
							bail = callback->CoreDiff( CD_MATERIAL_DESC, i, *m1, *m2 );
						}
					}
#endif
				}
				else
				{
					++diffcount;
				}
			}
		}

		// do scene descriptions
		int numDiffs = ( !bail ? SceneDescDiff( c1->mDesc, c2->mDesc ) : 0 );
		if ( numDiffs )
		{
			diffcount += numDiffs;
			if ( callback )
			{
				bail = callback->CoreDiff( c1->mDesc, c2->mDesc );
			}
		}

		// now do actors
		if ( !bail && ( c1->mActors.size() != c2->mActors.size() ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_NUM_ACTORS, c1->mActors.size(), c2->mActors.size() );
			}
		}

		if ( !bail )
		{
			// check each actor
			unsigned actorSize = c1->mActors.size();
			for ( unsigned i = 0; !bail && (i < actorSize); ++i )
			{
				NxActorDesc *a1 = c1->mActors[i];
				NxActorDesc *a2 = c2->mActors[i];

				if ( a1 && a2 )
				{
					// check individual items
					if ( FloatDiff( a1->density, a2->density, tolerance ) ||
						 ( a1->flags != a2->flags ) ||
						 ( a1->group != a2->group ) ||
						 ( a1->name && !a2->name ) ||
						 ( !a1->name && a2->name ) ||
						 ( a1->name && a2->name && strcmp( a1->name, a2->name ) ) )
					{
						// actor diff occurred
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_ACTOR_DESC, i, *a1, *a2 );
						}
					}

					// now check actor shape sizes
					if ( !bail && (a1->shapes.size() != a2->shapes.size()) )
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_NUM_ACTOR_SCENES, a1->shapes.size(), a2->shapes.size() );
						}
					}

					// are both body desc's present/missing?
					if ( !bail && (( a1->body && !a2->body ) || ( !a1->body && a2->body )) )
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_ACTOR_BODY_DESC_DIFF, *a1, *a2 );
						}
					}
					
					// check rotation, position, etc.
					if ( !bail && (memcmp( &a1->globalPose, &a2->globalPose, sizeof(NxMat34) ) != 0 ))
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_ACTOR_STATE_POSROT_INFO, a1->globalPose, a2->globalPose );						
						}
					}
					
					if ( !bail && a1->body && a2->body && 
						    ( Vec3Diff( a1->body->linearVelocity, a2->body->linearVelocity, tolerance ) ||
							  Vec3Diff( a1->body->angularVelocity, a2->body->angularVelocity, tolerance ) ) )
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_ACTOR_STATE_VEL_INFO, *a1->body, *a2->body );
						}
					}
					
					// now check other individual NxBodyDesc member variables
					if ( !bail && a1->body && a2->body && 
						    ( FloatDiff( a1->body->angularDamping, a2->body->angularDamping, tolerance ) ||
							  FloatDiff( a1->body->linearDamping, a2->body->linearDamping, tolerance ) ||
							  FloatDiff( a1->body->mass, a2->body->mass, tolerance ) ||
#if NX_SDK_VERSION_NUMBER >= 230
							  FloatDiff( a1->body->CCDMotionThreshold, a2->body->CCDMotionThreshold, tolerance ) ||
#endif
							  FloatDiff( a1->body->wakeUpCounter, a2->body->wakeUpCounter, tolerance ) ||
							  FloatDiff( a1->body->maxAngularVelocity, a2->body->maxAngularVelocity, tolerance ) ||
							  FloatDiff( a1->body->sleepAngularVelocity, a2->body->sleepAngularVelocity, tolerance ) ||
							  FloatDiff( a1->body->sleepLinearVelocity, a2->body->sleepLinearVelocity, tolerance ) ||
							  ( a1->body->solverIterationCount != a2->body->solverIterationCount ) ||
							  ( a1->body->flags != a2->body->flags ) ||
							  ( Vec3Diff( a1->body->massSpaceInertia, a2->body->massSpaceInertia, tolerance ) ) ||
							  ( memcmp( &a1->body->massLocalPose, &a2->body->massLocalPose, sizeof(NxMat34) ) ) ) ) 
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_ACTOR_BODY_DESC_MEMBER_INFO, *a1->body, *a2->body );					
						}
					}

					if ( !bail )
					{
						// k, # of shapes are same - compare them
						int	shapeSize = a1->shapes.size();
						for	( int j	= 0; !bail && (j < shapeSize); ++j )
						{
							NxShapeDesc	*s1= a1->shapes[j];
							NxShapeDesc	*s2	= a2->shapes[j];

							// validate they both exist
							if ( !bail && s1 && s2 )
							{
								// k, check the shape desc items
								if ( ( s1->shapeFlags != s2->shapeFlags ) ||
									 ( s1->group != s2->group ) ||
									 ( s1->materialIndex != s2->materialIndex ) ||
#ifdef NX_SUPPORT_NEW_FILTERING
									 ( s1->groupsMask.bits0 != s2->groupsMask.bits0 ) ||
									 ( s1->groupsMask.bits1 != s2->groupsMask.bits1 ) ||
									 ( s1->groupsMask.bits2 != s2->groupsMask.bits2 ) ||
									 ( s1->groupsMask.bits3 != s2->groupsMask.bits3 ) ||
#endif
#if NX_SDK_VERSION_NUMBER >= 230
									 FloatDiff( s1->mass, s2->mass, tolerance ) ||
									 FloatDiff( s1->density, s2->density, tolerance ) ||
#endif
									 FloatDiff( s1->skinWidth, s2->skinWidth, tolerance ) )
								{
									++diffcount;
									if ( callback )
									{
										bail = callback->CoreDiff( CD_ACTOR_SHAPE_DESC, *s1, *s2 );
									}
								}
								// check the names
								if ( !bail && ( ( s1->name && !s2->name ) || 
									      ( !s1->name && s2->name ) ||
									      ( s1->name && s2->name && strcmp( s1->name, s2->name ) ) ) )
								{
									++diffcount;
									if ( callback )
									{
										bail = callback->CoreDiff( CD_ACTOR_SHAPE_NAME_DESC, *s1, *s2 );								
									}
								}
								else
								{
									// should I do anything about the ccd skeleton?
								}
							}
							else 
							{
								// shape desc is missing
								++diffcount;
								if ( callback )
								{
									bail = callback->CoreDiff( CD_ACTOR_SHAPE_DESC_MISSING, j, *a1, *a2 );
								}
							}
						}
					}
				}
				else
				{
					++diffcount;
				}
			}
		}

		// do pair counts
		if ( !bail && ( c1->mPairCount != c2->mPairCount ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_NUM_PAIR_COUNTS, c1->mPairCount, c2->mPairCount );
			}
		}
		else
		{
			// now check the pair flags
			for	( int i = 0; !bail && (i < c1->mPairCount); ++i )
			{
				const PairFlagCapsule &p1 = c1->mPairFlagArray[i];
				const PairFlagCapsule &p2 = c2->mPairFlagArray[i];

				// check if anything differs
				if ( ( p1.mFlag != p2.mFlag ) ||
					 ( p1.mShapeIndex1 != p2.mShapeIndex1 ) ||
					 ( p1.mShapeIndex2 != p2.mShapeIndex2 ) ||
					 ( p1.mActorIndex1 != p2.mActorIndex1 ) ||
					 ( p1.mActorIndex2 != p2.mActorIndex2 ) )
				{
					// k, pair capsule's differ
					++diffcount;
					if ( callback )
					{
						bail = callback->CoreDiff( CD_PAIR_FLAG_DESC, p1, p2 );
					}
				}
			}
		}

		// now do joints
		if ( !bail && ( c1->mJointArray.size() != c2->mJointArray.size() ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_NUM_JOINTS, c1->mJointArray.size(), c2->mJointArray.size() );
			}
		}

		if ( !bail )
		{
			unsigned actorSize1 = c1->mActors.size();
			unsigned actorSize2 = c2->mActors.size();

			// check the joints
			unsigned jointSize = c1->mJointArray.size();
			for ( unsigned i = 0; !bail && (i < jointSize); ++i )
			{
				JointDescription *jd1 = c1->mJointArray[i];
				JointDescription *jd2 = c2->mJointArray[i];

				if ( jd1 && jd2 && jd1->mDesc && jd2->mDesc )
				{
					NxJointDesc *j1 = jd1->mDesc;
					NxJointDesc *j2 = jd2->mDesc;

					if ( j1 && j2 )
					{
						if ( ( j1->getType() != j2->getType() ) ||											
							( j1->jointFlags != j2->jointFlags ) ||
							FloatDiff( j1->maxForce, j2->maxForce, tolerance ) ||
							FloatDiff( j1->maxTorque, j2->maxTorque, tolerance ) )
						{
							++diffcount;
							if ( callback )
							{
								bail = callback->CoreDiff( CD_JOINT_DESC_DIFF, *j1, *j2 );
							}
						}

						// do an actor test - but only do it if the actor pntrs haven't been setup yet
						// -- that is, initially, the joint actor pntrs get set to a reference into
						// which actor they connect to before they are actually connected.
						if ( !bail &&
							 ( (unsigned)j1->actor[0] < actorSize1 ) &&
							 ( (unsigned)j1->actor[1] < actorSize1 ) &&
							 ( (unsigned)j2->actor[0] < actorSize2 ) &&
							 ( (unsigned)j2->actor[1] < actorSize2 ) )
						{
							if ( ( j1->actor[0] != j2->actor[0] ) ||
							     ( j1->actor[1] != j2->actor[1] ) )
							{
								++diffcount;
								if ( callback )
								{
									bail = callback->CoreDiff( CD_JOINT_DESC_ACTOR_DIFF, *j1, *j2 );
								}
							}
						}

						// now a name test
						if ( !bail && (( j1->name && !j2->name ) ||
								( !j1->name && j2->name ) ||
								( j1->name && j2->name && strcmp( j1->name, j2->name ) ) ))
						{
							++diffcount;
							if ( callback )
							{
								bail = callback->CoreDiff( CD_JOINT_DESC_NAME_DIFF, *j1, *j2 );					
							}
						}

						if ( !bail && (Vec3Diff( j1->localNormal[0], j2->localNormal[0], tolerance ) ||
								Vec3Diff( j1->localNormal[1], j2->localNormal[1], tolerance ) ||
								Vec3Diff( j1->localAxis[0], j2->localAxis[0], tolerance ) ||
								Vec3Diff( j1->localAxis[1], j2->localAxis[1], tolerance ) ||
								Vec3Diff( j1->localAnchor[0], j2->localAnchor[0], tolerance ) ||
								Vec3Diff( j1->localAnchor[1], j2->localAnchor[1], tolerance ) ))
						{
							++diffcount;
							if ( callback )
							{
								bail = callback->CoreDiff( CD_JOINT_DESC_VECTOR_DIFF, *j1, *j2 );										
							}
						}

						if ( !bail )
						{
							NxJointType type = j1->getType();

							// k, do joint type specific comparison, except for NxPrismaticJointDesc,
							// NxCylindricalJointDesc, NxPointOnLineJointDesc, NxPointInPlaneJointDesc,
							// and NxFixedJointDesc, as they are basic and don't have any extra params!
							switch(	type )
							{
								case NX_JOINT_REVOLUTE:
								{
									NxRevoluteJointDesc *p1 = static_cast<NxRevoluteJointDesc *>( j1 );
									NxRevoluteJointDesc *p2 = static_cast<NxRevoluteJointDesc *>( j2 );
									if ( ( p1->flags != p2->flags ) ||
										( p1->projectionMode != p2->projectionMode ) ||
										FloatDiff( p1->projectionAngle, p2->projectionAngle, tolerance ) ||
										FloatDiff( p1->projectionDistance, p2->projectionDistance, tolerance ) )
									{
										// "normal" rev joint diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_REV_JOINT_DESC_DIFF, *j1, *j2 );
										}
									}
									if ( !bail && JointLimitDescDiff( p1->limit.low, p2->limit.low, tolerance ) )
									{
										// low limit diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_REV_JOINT_DESC_LOW_LIMIT_DIFF, *j1, *j2 );
										}
									}
									
									if ( !bail && JointLimitDescDiff( p1->limit.high, p2->limit.high, tolerance ) )
									{
										// high limit diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_REV_JOINT_DESC_HIGH_LIMIT_DIFF, *j1, *j2 );
										}
									}
									if ( !bail && MotorDescDiff( p1->motor, p2->motor, tolerance ) )
									{
										// motor diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_REV_JOINT_DESC_MOTOR_DIFF, *j1, *j2 );
										}
									}

									if ( !bail &&SpringDescDiff( p1->spring, p2->spring, tolerance ) )
									{
										// spring diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_REV_JOINT_DESC_SPRING_DIFF, *j1, *j2 );
										}
									}
								}
								break;

								case NX_JOINT_SPHERICAL:
								{
									NxSphericalJointDesc *p1 = static_cast<NxSphericalJointDesc *>( j1 );
									NxSphericalJointDesc *p2 = static_cast<NxSphericalJointDesc *>( j2 );
									
									if ( SpringDescDiff( p1->twistSpring, p2->twistSpring, tolerance ) ||
										SpringDescDiff( p1->swingSpring, p2->swingSpring, tolerance ) ||
										SpringDescDiff( p1->jointSpring, p2->jointSpring, tolerance ) )
									{
										// spring diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_SPHERE_JOINT_DESC_SPRING_DIFF, *j1, *j2 );
										}
									}
									if ( !bail && (JointLimitDescDiff( p1->twistLimit.low, p2->twistLimit.low, tolerance ) ||
											JointLimitDescDiff( p1->twistLimit.high, p2->twistLimit.high, tolerance ) ||
											JointLimitDescDiff( p1->swingLimit, p2->swingLimit, tolerance ) ))
									{
										// LIMIT diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_SPHERE_JOINT_DESC_LIMIT_DIFF, *j1, *j2 );
										}
									}
									if ( !bail && Vec3Diff( p1->swingAxis, p2->swingAxis, tolerance ) )
									{
										// axis diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_SPHERE_JOINT_DESC_SWINGAXIS_DIFF, *j1, *j2 );								
										}
									}
									if ( !bail && (FloatDiff( p1->projectionDistance, p2->projectionDistance, tolerance ) ||
											( p1->flags != p2->flags ) ||
											( p1->projectionMode != p2->projectionMode ) ))
									{
										// "normal" diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_SPHERE_JOINT_DESC_DIFF, *j1, *j2 );
										}
									}
								}
								break;

								case NX_JOINT_DISTANCE:
								{
									NxDistanceJointDesc *p1 = static_cast<NxDistanceJointDesc *>( j1 );
									NxDistanceJointDesc *p2 = static_cast<NxDistanceJointDesc *>( j2 );

									if ( FloatDiff( p1->minDistance, p2->minDistance, tolerance ) ||
										FloatDiff( p1->maxDistance, p2->maxDistance, tolerance ) ||
										( p1->flags != p2->flags ) )
									{
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_DIST_JOINT_DESC_DIFF, *j1, *j2 );								
										}
									}
									if ( !bail && SpringDescDiff( p1->spring, p2->spring, tolerance ) )
									{
										// spring diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_DIST_JOINT_DESC_SPRING_DIFF, *j1, *j2 );								
										}
									}
								}
								break;

								case NX_JOINT_PULLEY:
								{
									NxPulleyJointDesc *p1 = static_cast<NxPulleyJointDesc *>( j1 );
									NxPulleyJointDesc *p2 = static_cast<NxPulleyJointDesc *>( j2 );

									if ( FloatDiff( p1->distance, p2->distance, tolerance ) ||
										FloatDiff( p1->stiffness, p2->stiffness, tolerance ) ||
										FloatDiff( p1->ratio, p2->ratio, tolerance ) ||
										( p1->flags != p2->flags ) )
									{
										// normal pully diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_PULLEY_JOINT_DESC_DIFF, *j1, *j2 );								
										}

									}
									if ( !bail && MotorDescDiff( p1->motor, p2->motor, tolerance ) )
									{
										// motor diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_PULLEY_JOINT_DESC_MOTOR_DIFF, *j1, *j2 );								
										}

									}
									if ( !bail && (Vec3Diff( p1->pulley[0], p2->pulley[0], tolerance ) ||
											Vec3Diff( p1->pulley[1], p2->pulley[1], tolerance ) ))
									{
										// pulley diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_PULLEY_JOINT_DESC_PULLEY_DIFF, *j1, *j2 );
										}
									}
								}
								break;

								case NX_JOINT_D6:
								{
									NxD6JointDesc *p1 = static_cast<NxD6JointDesc *>( j1 );
									NxD6JointDesc *p2 = static_cast<NxD6JointDesc *>( j2 );

									if ( ( p1->xMotion != p2->xMotion ) ||
										( p1->yMotion != p2->yMotion ) ||
										( p1->zMotion != p2->zMotion ) ||
										( p1->swing1Motion != p2->swing1Motion ) ||
										( p1->swing2Motion != p2->swing2Motion ) ||
										( p1->twistMotion != p2->twistMotion ) )
									{
										// motion diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_D6_JOINT_DESC_MOTION_DIFF, *j1, *j2 );								
										}
									}
									if ( !bail && (( p1->projectionMode != p2->projectionMode ) ||
											( p1->flags != p2->flags ) ||
											FloatDiff( p1->projectionAngle, p2->projectionAngle, tolerance ) ||
											FloatDiff( p1->projectionDistance, p2->projectionDistance, tolerance ) ||
											FloatDiff( p1->gearRatio, p2->gearRatio, tolerance ) ))
									{
										// "normal" diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_D6_JOINT_DESC_DIFF, *j1, *j2 );								
										}
									}
									if ( !bail && (JointLimitSoftDescDiff( p1->linearLimit, p2->linearLimit, tolerance ) ||
											JointLimitSoftDescDiff( p1->swing1Limit, p2->swing1Limit, tolerance ) ||
											JointLimitSoftDescDiff( p1->swing2Limit, p2->swing2Limit, tolerance ) ||
											JointLimitSoftDescDiff( p1->twistLimit.low, p2->twistLimit.low, tolerance ) ||
											JointLimitSoftDescDiff( p1->twistLimit.high, p2->twistLimit.high, tolerance ) ))
									{
										// soft limit diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_D6_JOINT_SOFT_DESC_DIFF, *j1, *j2 );								
										}
									}
									if ( !bail && (JointDriveDescDiff( p1->xDrive, p2->xDrive, tolerance ) ||
											JointDriveDescDiff( p1->yDrive, p2->yDrive, tolerance ) ||
											JointDriveDescDiff( p1->zDrive, p2->zDrive, tolerance ) ||
											JointDriveDescDiff( p1->swingDrive, p2->swingDrive, tolerance ) ||
											JointDriveDescDiff( p1->twistDrive, p2->twistDrive, tolerance ) ||
											JointDriveDescDiff( p1->slerpDrive, p2->slerpDrive, tolerance ) ))
									{
										// joint drive diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_D6_JOINT_DRIVE_DESC_DIFF, *j1, *j2 );								
										}
									}
									if ( !bail && (Vec3Diff( p1->drivePosition, p2->drivePosition, tolerance ) ||
											Vec3Diff( p1->driveLinearVelocity, p2->driveLinearVelocity, tolerance ) ||
											Vec3Diff( p1->driveAngularVelocity, p2->driveAngularVelocity, tolerance ) ))
									{
										// vec diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_D6_JOINT_VEC_DESC_DIFF, *j1, *j2 );
										}
									}
									if ( !bail && QuatDiff( p1->driveOrientation, p2->driveOrientation, tolerance ) )
									{
										// quat diff
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_D6_JOINT_QUAT_DESC_DIFF, *j1, *j2 );
										}
									}
								}
								break;
							}

						}
					}
					else
					{
						++diffcount;
					}
				}
				else
				{
					++diffcount;
				}

				if ( jd1 && jd2 )
				{
					if ( jd1->mPlaneInfo.size() != jd2->mPlaneInfo.size() )
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_NUM_JOINT_LIMIT_INFO, jd1->mPlaneInfo.size(), jd2->mPlaneInfo.size() ); 
						}
					}
					// only check plane limit pts if plane information exists!
					if ( !bail && jd1->mPlaneInfo.size() )
					{
						if ( Vec3Diff( jd1->mPlaneLimitPoint, jd2->mPlaneLimitPoint, tolerance ) )
						{
							++diffcount;
							if ( callback )
							{
								bail = callback->CoreDiff( CD_JOINT_LIMIT_PLANE_POINT, jd1->mPlaneLimitPoint, jd2->mPlaneLimitPoint );
							}
						}

						if ( !bail )
						{
							for ( unsigned j = 0; !bail && (i < jd1->mPlaneInfo.size()); ++j )
							{
								PlaneInfo *p1 = jd1->mPlaneInfo[j];
								PlaneInfo *p2 = jd2->mPlaneInfo[j];

								if ( p1 && p2 )
								{
									if ( Vec3Diff( p1->mPlaneNormal, p2->mPlaneNormal, tolerance ) )
									{
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_JOINT_LIMIT_PLANE_NORMAL, *p1, *p2 );
										}
									}
									if ( !bail && FloatDiff( p1->mPlaneD, p2->mPlaneD, tolerance ) )
									{
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_JOINT_LIMIT_PLANE_D, *p1, *p2 );
										}
									}
									if ( !bail && Vec3Diff( p1->mWorldLimitPt, p2->mWorldLimitPt, tolerance ) )
									{
										++diffcount;
										if ( callback )
										{
											bail = callback->CoreDiff( CD_JOINT_LIMIT_PLANE_WORLD_PT, *p1, *p2 );
										}
									}
								}
								else
								{
									++diffcount;
								}
							}
						}
					}
				}
			}
		}

		// now we do the effectors
		if ( !bail && ( c1->mEffectorArray.size() != c2->mEffectorArray.size() ) )
		{
			++diffcount;
			if ( callback )
			{
				bail = callback->CoreDiff( CD_NUM_SADE, c1->mEffectorArray.size(), c2->mEffectorArray.size() );
			}
		}
		else
		{
			// k, compare the spring and damper effectors
			unsigned effSize = c1->mEffectorArray.size();
			for ( unsigned i = 0; !bail && (i < effSize); ++i )
			{
				NxSpringAndDamperEffectorDesc *s1 = c1->mEffectorArray[i];
				NxSpringAndDamperEffectorDesc *s2 = c2->mEffectorArray[i];

				if ( s1 && s2 )
				{
					if ( ( s1->body1 != s2->body1 ) || ( s2->body2 != s2->body2 ) )
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_SADE_BODY_DIFF, *s1, *s2 );					
						}
					}
					if ( !bail && (FloatDiff( s1->damperMaxCompressForce, s2->damperMaxCompressForce, tolerance ) ||
						      FloatDiff( s1->damperMaxStretchForce, s2->damperMaxStretchForce, tolerance ) ||
							  FloatDiff( s1->damperVelCompressSaturate, s2->damperVelCompressSaturate, tolerance ) ||
							  FloatDiff( s1->damperVelStretchSaturate, s2->damperVelStretchSaturate, tolerance ) ))
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_SADE_DAMPER_PARAM, *s1, *s2 );
						}
					}
					if ( !bail && (FloatDiff( s1->springDistCompressSaturate, s2->springDistCompressSaturate, tolerance ) ||
						      FloatDiff( s1->springDistRelaxed, s2->springDistRelaxed, tolerance ) ||
							  FloatDiff( s1->springDistStretchSaturate, s2->springDistStretchSaturate, tolerance ) ||
							  FloatDiff( s1->springMaxCompressForce, s2->springMaxCompressForce, tolerance ) ||
							  FloatDiff( s1->springMaxStretchForce, s2->springMaxStretchForce, tolerance ) ))
					{
						++diffcount;
						if ( callback )
						{
							bail = callback->CoreDiff( CD_SADE_SPRING_PARAM, *s1, *s2 );	
						}
					}
				}
			}
		}
	}

	return bail;
}

//==================================================================================
bool CoreContainer::FloatDiff( const float &a, const float &b, float tolerance ) const
{
	return ( fabs( a - b ) > tolerance );
}

//==================================================================================
bool CoreContainer::Vec3Diff( const NxVec3 &a, const NxVec3 &b, float tolerance ) const
{
	return ( ( fabs( a.x - b.x ) > tolerance ) ||
		     ( fabs( a.y - b.y ) > tolerance ) ||
			 ( fabs( a.z - b.z ) > tolerance ) );
}

//==================================================================================
bool CoreContainer::QuatDiff( const NxQuat &a, const NxQuat &b, float tolerance ) const
{
	return ( ( fabs( a.x - b.x ) > tolerance ) ||
		     ( fabs( a.y - b.y ) > tolerance ) ||
			 ( fabs( a.z - b.z ) > tolerance ) ||
			 ( fabs( a.w - b.w ) > tolerance ) );
}

//==================================================================================
bool CoreContainer::JointLimitSoftDescDiff( const NxJointLimitSoftDesc &a, 
										    const NxJointLimitSoftDesc &b,
											float tolerance ) const
{
	return ( ( fabs( a.value - b.value ) > tolerance ) ||
		     ( fabs( a.restitution - b.restitution ) > tolerance ) ||
			 ( fabs( a.spring - b.spring ) > tolerance ) ||
			 ( fabs( a.damping - b.damping ) > tolerance ) );
}

//==================================================================================
bool CoreContainer::JointDriveDescDiff( const NxJointDriveDesc &a, const NxJointDriveDesc &b,
									    float tolerance ) const
{
	return ( ( a.driveType.bitField != b.driveType.bitField ) |\
		     ( fabs( a.spring - b.spring ) > tolerance ) ||
		     ( fabs( a.damping - b.damping ) > tolerance ) ||
		     ( fabs( a.forceLimit - b.forceLimit ) > tolerance ) );
}

//==================================================================================
bool CoreContainer::SpringDescDiff( const NxSpringDesc &a, const NxSpringDesc &b, float tolerance ) const
{
	return ( ( fabs( a.damper - b.damper ) > tolerance ) ||
		     ( fabs( a.spring - b.spring ) > tolerance ) ||
			 ( fabs( a.targetValue - b.targetValue ) > tolerance ) );
}

//==================================================================================
bool CoreContainer::MotorDescDiff( const NxMotorDesc &a, const NxMotorDesc &b, float tolerance ) const
{
	return ( ( a.freeSpin != b.freeSpin ) ||
		     ( fabs( a.maxForce - b.maxForce ) > tolerance ) ||
			 ( fabs( a.velTarget - b.velTarget ) > tolerance ) );
}

//==================================================================================
bool CoreContainer::JointLimitDescDiff( const NxJointLimitDesc &a, const NxJointLimitDesc &b, float tolerance ) const
{
	return ( ( fabs( a.hardness - b.hardness ) > tolerance ) ||
		     ( fabs( a.restitution - b.restitution ) > tolerance ) ||
			 ( fabs( a.value - b.value ) > tolerance ) );
		    
}

//==================================================================================
int CoreContainer::SceneDescDiff( const NxSceneDesc &d1, const NxSceneDesc &d2, float tolerance ) const
{
	int numDiffs = 0;
//	if ( d1.broadPhase != d2.broadPhase )
//	{
//		++numDiffs;
//	}
	
	if ( d1.groundPlane != d2.groundPlane )
	{
		++numDiffs;
	}
	
	if ( d1.boundsPlanes != d2.boundsPlanes )
	{
		++numDiffs;
	}
	
//	if ( d1.collisionDetection != d2.collisionDetection )
//	{
//		++numDiffs;
//	}
#if NX_SDK_VERSION_NUMBER >= 230	
	if ( d1.simType != d2.simType )
	{
		++numDiffs;
	}
	

	{
		++numDiffs;
	}
#endif

#if NX_SDK_VERSION_NUMBER >= 231

	{
		++numDiffs;
	}
#endif	

	if ( d1.maxIter != d2.maxIter )
	{
		++numDiffs;
	}
	
	if ( d1.timeStepMethod != d2.timeStepMethod )
	{
		++numDiffs;
	}
	
	if ( Vec3Diff( d1.gravity, d2.gravity, tolerance ) )
	{
		++numDiffs;
	}
	
	if ( FloatDiff( d1.maxTimestep, d2.maxTimestep, tolerance ) )
	{
		++numDiffs;		
	}
	
	// these are user pointers, so we can only test if they are present on one, and not
	// present on the other
	if ( ( d1.userNotify && !d2.userNotify ) || ( !d1.userNotify && d2.userNotify ) )
	{
		++numDiffs;			
	}

	if ( ( d1.userTriggerReport && !d2.userTriggerReport ) || ( !d1.userTriggerReport && d2.userTriggerReport ) )
	{
		++numDiffs;			
	}

	if ( ( d1.userContactReport && !d2.userContactReport ) || ( !d1.userContactReport && d2.userContactReport ) )
	{
		++numDiffs;			
	}

	if ( ( d1.maxBounds && !d2.maxBounds ) || ( !d1.maxBounds && d2.maxBounds ) )
	{
		++numDiffs;	
	}
	else if ( d1.maxBounds && d2.maxBounds &&
		    ( Vec3Diff( d1.maxBounds->min, d2.maxBounds->min, tolerance ) ||
		      Vec3Diff( d1.maxBounds->max, d2.maxBounds->max, tolerance ) ) )
	{
		++numDiffs;
	}
	
	if ( ( d1.limits && !d2.limits ) || ( !d1.limits && d2.limits ) )
	{
		++numDiffs;
	}
	else if ( d1.limits && d2.limits &&
		    ( ( d1.limits->maxNbActors        != d2.limits->maxNbActors ) ||
		      ( d1.limits->maxNbBodies        != d2.limits->maxNbBodies ) ||
			  ( d1.limits->maxNbDynamicShapes != d2.limits->maxNbDynamicShapes ) ||
			  ( d1.limits->maxNbJoints        != d2.limits->maxNbJoints ) ||
			  ( d1.limits->maxNbStaticShapes  != d2.limits->maxNbStaticShapes ) ) )
	{
		++numDiffs;
	}

	if ( ( d1.userData && !d2.userData ) || ( !d1.userData && d2.userData ) )
	{
		++numDiffs;
	}

	return numDiffs;
}

//==================================================================================
const char * CoreCompareNotify::GetCoreDifferenceTypeString( const CoreDifference type )
{
	// k, get it
	const char *ret = 0;

	switch( type )
	{
		case CD_NUM_ACTORS:						// # of 'NxActorDesc's differ
			ret = "CD_NUM_ACTOR";
		break;

		case CD_NUM_JOINTS:						// # of 'NxJointDesc's differ
			ret = "CD_NUM_JOINTS";
		break;

		case CD_NUM_CORE_SCENES:					// # of 'CoreScene's differ
			ret = "CD_NUM_CORE_SCENES";
		break;

		case CD_NUM_TRIANGLE_MESHES:				// # of 'NxTriangleMeshDesc's differ
			ret = "CD_NUM_TRIANGLE_MESHES";
		break;

		case CD_NUM_CONVEX_MESHES:				// # of 'NxConvexMeshDesc's differ
			ret = "CD_NUM_CONVEX_MESHES";
		break;

		case CD_NUM_MATERIALS:					// # of 'NxMaterialDesc's differ
			ret = "CD_NUM_MATERIALS";
		break;

		case CD_NUM_PAIR_COUNTS:					// # of 'PairFlagCapsule's differ
			ret = "CD_NUM_PAIR_COUNTS";
		break;

		case CD_NUM_SADE:						// # of NxSpringAndDamperEffectorDesc's differ
			ret = "CD_NUM_SADE";
		break;

		case CD_NUM_JOINT_LIMIT_INFO:			// # of a joint's Limit plane info's differ
			ret = "CD_NUM_JOINT_LIMIT_INFO";
		break;

		case CD_MATERIAL_DESC:					// an item in an NxMaterialDesc differs
			ret = "CD_MATERIAL_DESC";
		break;

		case CD_ACTOR_DESC:						// an item in an NxActorDesc differs
			ret = "CD_ACTOR_DESC";
		break;

		case CD_ACTOR_SHAPE_DESC_MISSING:		// an NxActorShapeDesc is missing
			ret = "CD_ACTOR_SHAPE_DESC_MISSING";
		break;

		case CD_ACTOR_SHAPE_DESC:				// an item in an 'NxActorDesc's NxShapeDesc differs 
			ret = "CD_ACTOR_SHAPE_DESC";
		break;

		case CD_ACTOR_BODY_DESC_DIFF:            // an NxBodyDesc is missing or is now present for an NxActorDesc
			ret = "CD_ACTOR_BODY_DESC_DIFF";
		break;

		case CD_ACTOR_SHAPE_NAME_DESC:			// an NxActorDesc name item is missing or differs
			ret = "CD_ACTOR_SHAPE_NAME_DESC";
		break;

		case CD_ACTOR_STATE_POSROT_INFO:         // an NxActor's position or rotation differs
			ret = "CD_ACTOR_STATE_POSROT_INFO";
		break;

		case CD_ACTOR_STATE_VEL_INFO:            // an NxActor's linear or angular velocity differs
			ret = "CD_ACTOR_STATE_VEL_INFO";
		break;

		case CD_ACTOR_BODY_DESC_MEMBER_INFO:     // an NxActor's NxBodyDesc has a difference
			ret = "CD_ACTOR_BODY_DESC_MEMBER_INFO";
		break;

		case CD_NUM_ACTOR_SCENES:				// # of 'NxSceneDesc's for an NxActor differs
			ret = "CD_NUM_ACTOR_SCENES";
		break;

		case CD_PAIR_FLAG_DESC:					// an item in a 'PairFlagCapsule' differs
			ret = "CD_PAIR_FLAG_DESC";
		break;
	
		case CD_JOINT_DESC_DIFF:					// an item in an NxJointDesc differs
			ret = "CD_JOINT_DESC_DIFF";
		break;

		case CD_JOINT_DESC_VECTOR_DIFF:			// one of the NxVec3 items in an NxJointDesc differs
			ret = "CD_JOINT_DESC_VECTOR_DIFF";
		break;

		case CD_JOINT_DESC_NAME_DIFF:			// an NxJointDesc name item is missing or differs
			ret = "CD_JOINT_DESC_NAME_DIFF";
		break;

		case CD_REV_JOINT_DESC_DIFF:				// a "normal" revolution joint diff
			ret = "CD_REV_JOINT_DESC_DIFF";
		break;

		case CD_REV_JOINT_DESC_LOW_LIMIT_DIFF:	// a NxJointLimitDesc (low limit) revolution joint diff
			ret = "CD_REV_JOINT_DESC_LOW_LIMIT_DIFF";
		break;

		case CD_REV_JOINT_DESC_HIGH_LIMIT_DIFF:	// a NxJointLimitDesc (high limit) revolution joint diff
			ret = "CD_REV_JOINT_DESC_HIGH_LIMIT_DIFF";
		break;

		case CD_REV_JOINT_DESC_MOTOR_DIFF:		// a NxMotorDesc revolution joint diff
			ret = "CD_REV_JOINT_DESC_MOTOR_DIFF";
		break;

		case CD_REV_JOINT_DESC_SPRING_DIFF:		// a NxSpringDesc revolution joint diff
			ret = "CD_REV_JOINT_DESC_SPRING_DIFF";
		break;

		case CD_PULLEY_JOINT_DESC_DIFF:			// a "normal" pully joint diff
			ret = "CD_PULLEY_JOINT_DESC_DIFF";
		break;

		case CD_PULLEY_JOINT_DESC_MOTOR_DIFF:	// a pully's NxMotorDesc diff
			ret = "CD_PULLEY_JOINT_DESC_MOTOR_DIFF";
		break;

		case CD_PULLEY_JOINT_DESC_PULLEY_DIFF:	// a NxVec3 of pully's pully joint has a diff
			ret = "CD_PULLEY_JOINT_DESC_PULLEY_DIFF";
		break;

		case CD_SPHERE_JOINT_DESC_DIFF:			// a "normal" spherical joint diff
			ret = "CD_SPHERE_JOINT_DESC_DIFF";
		break;

		case CD_SPHERE_JOINT_DESC_SWINGAXIS_DIFF:// a swing-axis (NxVec3) joint diff for a spherical joint
			ret = "CD_SPHERE_JOINT_DESC_SWINGAXIS_DIFF";
		break;

		case CD_SPHERE_JOINT_DESC_LIMIT_DIFF:	// a NxJointLimitDesc diff for a spherical joint
			ret = "CD_SPHERE_JOINT_DESC_LIMIT_DIFF";
		break;

		case CD_SPHERE_JOINT_DESC_SPRING_DIFF:	// a NxSpringDesc diff for a spherical joint
			ret = "CD_SPHERE_JOINT_DESC_SPRING_DIFF";
		break;

		case CD_DIST_JOINT_DESC_DIFF:			// a "normal" distance joint desc diff
			ret = "CD_DIST_JOINT_DESC_DIFF";
		break;

		case CD_DIST_JOINT_DESC_SPRING_DIFF:		// a NxSpringDesc diff for a distance joint
			ret = "CD_DIST_JOINT_DESC_SPRING_DIFF";
		break;

		case CD_D6_JOINT_DESC_DIFF:				// a "normal" D6 joint desc diff
			ret = "CD_D6_JOINT_DESC_DIFF";
		break;

		case CD_D6_JOINT_DESC_MOTION_DIFF:		// a motion item of the D6 joint has a diff
			ret = "CD_D6_JOINT_DESC_MOTION_DIFF";
		break;

		case CD_D6_JOINT_SOFT_DESC_DIFF:			// a NxJointLimitSoftDesc diff for a D6 joint desc
			ret = "CD_D6_JOINT_SOFT_DESC_DIFF";
		break;

		case CD_D6_JOINT_DRIVE_DESC_DIFF:		// a NxJointDriveDesc diff for a D6 joint desc
			ret = "CD_D6_JOINT_DRIVE_DESC_DIFF";
		break;

		case CD_D6_JOINT_VEC_DESC_DIFF:			// one of the NxVec3 has a diff for a D6 joint desc
			ret = "CD_D6_JOINT_VEC_DESC_DIFF";
		break;

		case CD_D6_JOINT_QUAT_DESC_DIFF:			// the NxQuat has a diff for a D6 joint desc
			ret = "CD_D6_JOINT_QUAT_DESC_DIFF";
		break;

		case CD_SADE_BODY_DIFF:					// the body indices do not match for an NxSpringAndDamperEffectorDesc
			ret = "CD_SADE_BODY_DIFF";
		break;

		case CD_SADE_SPRING_PARAM:               // one of the spring parameters do not match for an NxSpringAndDamperEffectorDesc
			ret = "CD_SADE_SPRING_PARAM";
		break;

		case CD_SADE_DAMPER_PARAM:				// one of the damper parameters do not match for an NxSpringAndDamperEffectorDesc
			ret = "CD_SADE_DAMPER_PARAM";
		break;

		case CD_JOINT_LIMIT_PLANE_NORMAL:        // a joint's limit plane's normal differs in either x,y, and/or z
			ret = "CD_JOINT_LIMIT_PLANE_NORMAL";
		break;

		case CD_JOINT_LIMIT_PLANE_POINT:			// a joint's limit plane's point differs
			ret = "CD_JOINT_LIMIT_PLANE_POINT";
		break;

		case CD_JOINT_LIMIT_PLANE_D:  			// a joint's limit plane's planeD differs
			ret = "CD_JOINT_LIMIT_PLANE_D";
		break;

		case CD_JOINT_LIMIT_PLANE_WORLD_PT:		// a joint's limit plane's world pt differs
			ret = "CD_JOINT_LIMIT_PLANE_WORLD_PT";
		break;

		default:
			ret = "Error";
		break;
	}

	return ret;
}

//==================================================================================
const char * CoreCompareNotify::GetCoreDifferenceDescriptionString( const CoreDifference type )
{
	// k, get it
	const char *ret = 0;

	switch( type )
	{
		case CD_NUM_ACTORS:
			ret = "Number of 'NxActorDesc's differ";
		break;

		case CD_NUM_JOINTS:
			ret = "Number of 'NxJointDesc's differ";
		break;

		case CD_NUM_CORE_SCENES:
			ret = "Number of 'CoreScene's differ";
		break;

		case CD_NUM_TRIANGLE_MESHES:
			ret = "Number of 'NxTriangleMeshDesc's differ";
		break;

		case CD_NUM_CONVEX_MESHES:
			ret = "Number of 'NxConvexMeshDesc's differ";
		break;

		case CD_NUM_MATERIALS:
			ret = "Number of 'NxMaterialDesc's differ";
		break;

		case CD_NUM_PAIR_COUNTS:
			ret = "Number of 'PairFlagCapsule's differ";
		break;

		case CD_NUM_SADE:
			ret = "Number of 'SpringAndDamperEffectorDesc's differ";
		break;

		case CD_NUM_JOINT_LIMIT_INFO:
			ret = "Number of a joint's Limit plane info's differ";
		break;

		case CD_MATERIAL_DESC:
			ret = "An item in an NxMaterialDesc differs";
		break;

		case CD_ACTOR_DESC:
			ret = "An item in an NxActorDesc differs";
		break;

		case CD_ACTOR_SHAPE_DESC_MISSING:
			ret = "An NxActorShapeDesc is missing";
		break;

		case CD_ACTOR_SHAPE_DESC:
			ret = "An item in an 'NxActorDesc's NxShapeDesc differs ";
		break;

		case CD_ACTOR_BODY_DESC_DIFF:
			ret = "An NxBodyDesc is missing or is now present for an NxActorDesc";
		break;

		case CD_ACTOR_SHAPE_NAME_DESC:
			ret = "An NxActorDesc name item is missing or differs";
		break;

		case CD_ACTOR_STATE_POSROT_INFO:
			ret = "An NxActor's position or rotation differs";
		break;

		case CD_ACTOR_STATE_VEL_INFO:
			ret = "An NxActor's linear or angular velocity differs";
		break;

		case CD_ACTOR_BODY_DESC_MEMBER_INFO:
			ret = "An NxActor's NxBodyDesc has a difference";
		break;

		case CD_NUM_ACTOR_SCENES:
			ret = "Number of 'NxSceneDesc's for an NxActor differs";
		break;

		case CD_PAIR_FLAG_DESC:
			ret = "An item in a 'PairFlagCapsule' differs";
		break;
	
		case CD_JOINT_DESC_DIFF:
			ret = "An item in an NxJointDesc differs";
		break;

		case CD_JOINT_DESC_VECTOR_DIFF:
			ret = "One of the NxVec3 items in an NxJointDesc differs";
		break;

		case CD_JOINT_DESC_NAME_DIFF:
			ret = "An NxJointDesc name item is missing or differs";
		break;

		case CD_REV_JOINT_DESC_DIFF:
			ret = "A 'normal' revolution joint diff";
		break;

		case CD_REV_JOINT_DESC_LOW_LIMIT_DIFF:
			ret = "A NxJointLimitDesc (low limit) revolution joint diff";
		break;

		case CD_REV_JOINT_DESC_HIGH_LIMIT_DIFF:
			ret = "A NxJointLimitDesc (high limit) revolution joint diff";
		break;

		case CD_REV_JOINT_DESC_MOTOR_DIFF:
			ret = "A NxMotorDesc revolution joint diff";
		break;

		case CD_REV_JOINT_DESC_SPRING_DIFF:
			ret = "A NxSpringDesc revolution joint diff";
		break;

		case CD_PULLEY_JOINT_DESC_DIFF:
			ret = "A 'normal' pully joint diff";
		break;

		case CD_PULLEY_JOINT_DESC_MOTOR_DIFF:
			ret = "A pully's NxMotorDesc diff";
		break;

		case CD_PULLEY_JOINT_DESC_PULLEY_DIFF:
			ret = "A NxVec3 of pully's pully joint has a diff";
		break;

		case CD_SPHERE_JOINT_DESC_DIFF:
			ret = "A 'normal' spherical joint diff";
		break;

		case CD_SPHERE_JOINT_DESC_SWINGAXIS_DIFF:
			ret = "A swing-axis (NxVec3) joint difference for a spherical joint";
		break;

		case CD_SPHERE_JOINT_DESC_LIMIT_DIFF:
			ret = "A NxJointLimitDesc difference for a spherical joint";
		break;

		case CD_SPHERE_JOINT_DESC_SPRING_DIFF:
			ret = "A NxSpringDesc difference for a spherical joint";
		break;

		case CD_DIST_JOINT_DESC_DIFF:
			ret = "A 'normal' distance joint desc diff";
		break;

		case CD_DIST_JOINT_DESC_SPRING_DIFF:
			ret = "A NxSpringDesc difference for a distance joint";
		break;

		case CD_D6_JOINT_DESC_DIFF:
			ret = "A 'normal' D6 joint desc diff";
		break;

		case CD_D6_JOINT_DESC_MOTION_DIFF:
			ret = "A motion item of the D6 joint has a diff";
		break;

		case CD_D6_JOINT_SOFT_DESC_DIFF:
			ret = "A NxJointLimitSoftDesc difference for a D6 joint desc";
		break;

		case CD_D6_JOINT_DRIVE_DESC_DIFF:
			ret = "A NxJointDriveDesc difference for a D6 joint desc";
		break;

		case CD_D6_JOINT_VEC_DESC_DIFF:
			ret = "One of the NxVec3 has a difference for a D6 joint desc";
		break;

		case CD_D6_JOINT_QUAT_DESC_DIFF:
			ret = "The NxQuat has a difference for a D6 joint desc";
		break;

		case CD_SADE_BODY_DIFF:
			ret = "The body indices do not match for an NxSpringAndDamperEffectorDesc";
		break;

		case CD_SADE_SPRING_PARAM:
			ret = "One or more of the spring parameters do not match for an NxSpringAndDamperEffectorDesc";
		break;

		case CD_SADE_DAMPER_PARAM:
			ret = "One or more of the damper parameters do not match for an NxSpringAndDamperEffectorDesc";
		break;

		case CD_JOINT_LIMIT_PLANE_NORMAL:
			ret = "A joint's limit plane's normal differs in either x,y, and/or z";
		break;

		case CD_JOINT_LIMIT_PLANE_POINT:		
			ret = "A joint's limit plane's point differs";
		break;

		case CD_JOINT_LIMIT_PLANE_D: 
			ret = "A joint's limit plane's planeD differs";
		break;

		case CD_JOINT_LIMIT_PLANE_WORLD_PT:
			ret = "A joint's limit plane's world pt differs";
		break;

		case CD_HEIGHT_FIELD_NUM:
			ret = "The Number of Height Fields differ";
		break;

		case CD_HEIGHT_FIELD_CET_DIFF:
			ret = "A Height Field's convex edge threshold value differs";
		break;
		
		case CD_HEIGHT_FIELD_FLAGS:
			ret = "A Height Field's flags differs";
		break;
		
		case CD_HEIGHT_FIELD_FORMAT:
			ret = "A Height Field's format differs";
		break;
		
		case CD_HEIGHT_FIELD_NUM_COLUMNS:
			ret = "A Height Field's number of columns differs";
		break;
		
		case CD_HEIGHT_FIELD_NUM_ROWS:
			ret = "A Height Field's number of rows differs";
		break;
		
		case CD_HEIGHT_FIELD_SAMPLE_STRIDE:
			ret = "A Height Field's sample stride value differs";
		break;
		
		case CD_HEIGHT_FIELD_VERTICAL_EXTENT:
			ret = "A Height Field's vertical extent differs";
		break;
		
		case CD_HEIGHT_FIELD_MISSING_SAMPLES:
			ret = "A Height Field is missing some samples, and the other is not";
		break;
		
		case CD_HEIGHT_FIELD_SAMPLE_MEM_DIFF:
			ret = "A memory compare of the 2 height field samples differs";
		break;

		default:
			ret = "Error";
		break;
	}

	return ret;
}

//==================================================================================
const char * CoreCompareNotify::GetTriangleMeshDifferenceTypeString( const TriangleMeshDifferences &type )
{
	const char *ret = 0;

	switch( type )
	{
		case TMD_CET:
			ret = "TMD_CET";
		break;

		case TMD_FLAGS:
			ret = "TMD_FLAGS";
		break;

		case TMD_HFVA:
			ret = "TMD_HFVA";
		break;

		case TMD_HFVE:
			ret = "TMD_HFVE";
		break;
	
		case TMD_MIS:
			ret = "TMD_MIS";
		break;

		case TMD_PMAP_MISSING:
			ret = "TMD_PMAP_MISSING";
		break;

		case TMD_PMAP_NUMVERTS:
			ret = "TMD_PMAP_NUMVERTS";
		break;

		case TMD_VERT_NUM:
			ret = "TMD_VERT_NUM";
		break;

		case TMD_VERT_STRIDE:
			ret = "TMD_VERT_STRIDE";
		break;

		case TMD_VERT_PTS:
			ret = "TMD_VERT_PTS";
		break;

		case TMD_TRI_NUMVERTS:
			ret = "TMD_TRI_NUMVERTS";
		break;

		case TMD_TRI_STRIDE:
			ret = "TMD_TRI_STRIDE";
		break;

		case TMD_TRI_TRIS:
			ret = "TMD_TRI_TRIS";
		break;

		default:
			ret = "Error";
		break;
	}

	return ret;
}

//==================================================================================
const char * CoreCompareNotify::GetTriangleMeshDifferenceDescriptionString( const TriangleMeshDifferences &type )
{
	const char *ret = 0;

	switch( type )
	{
		case TMD_CET:
			ret = "The convex edge threshold for the triangle mesh has a difference";
		break;

		case TMD_FLAGS:
			ret = "The flags for the triangle meshes differ";
		break;

		case TMD_HFVA:
			ret = "The height field vertical axis has a difference in the triangle meshes";
		break;

		case TMD_HFVE:
			ret = "The height field vertical extents differ in the triangle meshes";
		break;
	
		case TMD_MIS:
			ret = "The material index stride has a difference in the triangle meshes";
		break;

		case TMD_PMAP_MISSING:
			ret = "One of the triangle meshes is missing a pmap";
		break;

		case TMD_PMAP_NUMVERTS:
			ret = "The number of vertex information's (actual pmap data size) is different between the 2 triangle meshes";
		break;

		case TMD_VERT_NUM:
			ret = "The number of vertices in the two triangle meshes differ";
		break;

		case TMD_VERT_STRIDE:
			ret = "The vertex stride length ('length' of one set of vertex (x,y,z) information) is different)";
		break;

		case TMD_VERT_PTS:
			ret = "A memory comparison of the vertex position information is different";
		break;

		case TMD_TRI_NUMVERTS:
			ret = "The number of triangles in the two triangle meshes differ";
		break;

		case TMD_TRI_STRIDE:
			ret = "The triangle stride length ('length' of one set of triangle (v1Index,v2Index,v3Index) information) is different)";
		break;

		case TMD_TRI_TRIS:
			ret = "A memory comparison of the triangle index information is different";
		break;

		default:
			ret = "Error";
		break;
	}

	return ret;
}

//==================================================================================
const char * CoreCompareNotify::GetConvexMeshDifferenceTypeString( const ConvexMeshDifferences &type )
{
	const char *ret = 0;

	switch( type )
	{
		case CMD_FLAGS:
			ret = "CMD_FLAGS";
		break;

		case CMD_VERT_NUM:
			ret = "CMD_VERT_NUM";
		break;

		case CMD_VERT_STRIDE:
			ret = "CMD_VERT_STRIDE";
		break;

		case CMD_VERT_PTS:
			ret = "CMD_VERT_PTS";
		break;

		case CMD_TRI_NUMVERTS:
			ret = "CMD_TRI_NUMVERTS";
		break;

		case CMD_TRI_STRIDE:
			ret = "CMD_TRI_STRIDE";
		break;

		case CMD_TRI_TRIS:
			ret = "CMD_TRI_TRIS";
		break;

		default:
			ret = "Error";
		break;
	}

	return ret;
}

//==================================================================================
const char * CoreCompareNotify::GetConvexMeshDifferenceDescriptionString( const ConvexMeshDifferences &type )
{
	const char *ret = 0;

	switch( type )
	{
		case CMD_FLAGS:
			ret = "The flags for a convex mesh description differs";
		break;

		case CMD_VERT_NUM:
			ret = "The number of vertices for a convex mesh differs";
		break;

		case CMD_VERT_STRIDE:
			ret = "The vertex stride length ('length' of one set of vertex (x,y,z) information) is different)";
		break;

		case CMD_VERT_PTS:
			ret = "A memory comparison of vertex positions does not match for the convex mesh";
		break;

		case CMD_TRI_NUMVERTS:
			ret = "The number of triangles for the convex mesh differs";
		break;

		case CMD_TRI_STRIDE:
			ret = "The triangle stride length ('length' of one set of triangle (v1Index,v2Index,v3Index) information) is different)";
		break;

		case CMD_TRI_TRIS:
			ret = "A memory comparison of triangle indices does not match for the convex mesh";
		break;

		default:
			ret = "Error";
		break;
	}

	return ret;
}

//==================================================================================
bool CoreContainer::ValidateEnergy(float maxVel,float maxAngularVelocity,int &lowEnergy,int &highEnergy) const
{
	bool ret = true;

	lowEnergy = 0;
	highEnergy = 0;

	int scount = mScenes.size();
	for (int i=0; i<scount; i++)
	{
		CoreScene *cs = mScenes[i];
		if ( !cs->ValidateEnergy(maxVel,maxAngularVelocity,lowEnergy,highEnergy) )
		{
			ret = false;
		}
	}

	return ret;
}

//==================================================================================
bool CoreContainer::ValidateSleep(int &awake,int &asleep) const
{
	bool ret = true;

	awake = 0;
	asleep = 0;

	int scount = mScenes.size();
	for (int i=0; i<scount; i++)
	{
		CoreScene *cs = mScenes[i];
		if ( ! cs->ValidateSleep(awake,asleep) )
		{
			ret = false;
		}
	}

	return ret;
}

//==================================================================================
bool CoreScene::ValidateSleep(int &awake,int &asleep) const
{
	bool ret = true;

  int count = mActors.size();
  for (int i=0; i<count; i++)
  {
  	NxActorDesc *d = mActors[i];
  	if ( d->body )
  	{
  		if ( d->body->wakeUpCounter <= 0 )
  			asleep++;
  		else
  		{
  			awake++;
  			ret = false;
  		}
  	}
  	else
  	{
  		asleep++;
  	}
  }
  return ret;
}

//==================================================================================
bool CoreScene::ValidateEnergy(float maxVel,float maxAngularVelocity,int &lowEnergy,int &highEnergy) const
{
	bool ret = true;

  int count = mActors.size();
  for (int i=0; i<count; i++)
  {
  	NxActorDesc *d = mActors[i];
  	if ( d->body )
  	{
  		if ( d->body->linearVelocity.x > maxVel ||
  			   d->body->linearVelocity.y > maxVel ||
  			   d->body->linearVelocity.z > maxVel ||
  			   d->body->angularVelocity.x > maxAngularVelocity ||
  			   d->body->angularVelocity.y > maxAngularVelocity ||
  			   d->body->angularVelocity.z > maxAngularVelocity )
  		{
  			highEnergy++;
  			ret = false;
  		}
  		else
  		{
  			lowEnergy++;
  		}
  	}
  	else
  	{
  		lowEnergy++;
  	}
  }
  return ret;
}

