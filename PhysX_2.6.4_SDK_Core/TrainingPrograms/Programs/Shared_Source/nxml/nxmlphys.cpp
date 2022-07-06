
#include <stdio.h>
#include <NxPhysics.h>
#include <NxUtilities.h>
#include <NxCooking.h>
#include "Stream.h"
#include "nxmltypes.h"
#define PI (3.1415926535897932384626433832795f)
#define DEG2RAD (PI / 180.0f)

using namespace nxml;

NxPhysicsSDK* nxsdk=NULL;
NxScene *nxscene=NULL;
extern void NxOut(const char* message,...);
extern Array<const char *> nxContext;


static inline float  Maxf(const float &a,const float &b) {return (a>b)?a:b;}
static inline float  Minf(const float &a,const float &b) {return (a<b)?a:b;}
static inline NxVec3 VectorMax(const NxVec3 &a,const NxVec3 &b){return NxVec3(Maxf(a.x,b.x),Maxf(a.y,b.y),Maxf(a.z,b.z));}
static inline NxVec3 VectorMin(const NxVec3 &a,const NxVec3 &b){return NxVec3(Minf(a.x,b.x),Minf(a.y,b.y),Minf(a.z,b.z));}


static int SubmitMaterial(nxml::Material *pmat)
{
	if(pmat->index>=0) return pmat->index; // already in sdk!!!!
	pmat->nxmaterial = nxscene->createMaterial(*pmat);
	return pmat->nxmaterial->getMaterialIndex();
}

NxShapeDesc *nxml::Convex::BuildNX()  
{
	if(!nxmesh)
	{
		NxConvexMeshDesc cmd;
		cmd.points    = vertices.element;
		cmd.numVertices  = vertices.count;
		cmd.pointStrideBytes = sizeof(NxVec3);
		cmd.flags  =NX_CF_COMPUTE_CONVEX;
		NxInitCooking(NULL,NxGetFoundationSDK()->getErrorStream());
		MemoryWriteBuffer cookresult;
//		cmd.flags |= NX_CF_INFLATE_CONVEX;
		bool status = NxCookConvexMesh(cmd,cookresult);
		assert(status);
		nxmesh = nxsdk->createConvexMesh(MemoryReadBuffer(cookresult.data));
	}
	NxConvexShapeDesc *cs = NULL;
	cs = new NxConvexShapeDesc;
	cs->meshData = nxmesh;
	//cs->meshFlags |= NX_CF_COMPUTE_CONVEX;
	return cs;
}

NxShapeDesc *nxml::TriMesh::BuildNX() 
{
	if(!nxmesh)
	{
		NxTriangleMeshDesc tmd;
		tmd.points    = vertices.element;
		tmd.numVertices  = vertices.count;
		tmd.pointStrideBytes = sizeof(NxVec3);
		tmd.triangleStrideBytes = sizeof(int3);
		tmd.triangles = triangles.element;
		tmd.numTriangles = triangles.count;
		NxInitCooking();
		MemoryWriteBuffer cookresult;
		bool status = NxCookTriangleMesh(tmd,cookresult);
		nxmesh = nxsdk->createTriangleMesh(MemoryReadBuffer(cookresult.data));

	}
	NxTriangleMeshShapeDesc *ts = new NxTriangleMeshShapeDesc;
	ts->meshData = nxmesh;
	return ts;

}
NxShapeDesc *SubmitShape(nxml::Shape *ps)
{
	nxContext.Add(ps->geometry->id);
	NxShapeDesc *ns = ps->geometry->BuildNX();
	ns->localPose.t =  ps->position;
	ns->localPose.M.fromQuat((NxQuat&)ps->orientation); // =  ps->orientation.getmatrix();
	if(ps->material) ns->materialIndex = SubmitMaterial(ps->material);
	ns->mass = ps->mass;
	ns->name = strdup(ps->geometry->id);
	nxContext.Pop();
	return ns;
}

NxActor *SubmitRigidBody(nxml::RigidBody *prb)
{
	int i;
	nxContext.Add(prb->id);
	NxActorDesc actor;
	NxBodyDesc  body;
	actor.name = strdup(prb->id);
	if(prb->dynamic) 
	{
		actor.body = &body;
		if(prb->mass<=0.0f)
		{
			actor.density = 1.0f;
		}
	}
	body.mass = prb->mass;
	body.massLocalPose.t = prb->com;
	if(prb->inertia(0,0)>0) NxDiagonalizeInertiaTensor(prb->inertia, body.massSpaceInertia, body.massLocalPose.M);
	actor.globalPose.t = prb->position;
	actor.globalPose.M.fromQuat((NxQuat&)prb->orientation); // .getmatrix();
	for(i=0;i<prb->shape.count;i++)
	{
		actor.shapes.pushBack(SubmitShape(prb->shape[i]));
	}
	actor.isValid();
	prb->nxactor = nxscene->createActor(actor);  // need to store it so i can make joints.
	nxContext.Pop();
	return prb->nxactor;
}

NxJoint *SubmitJoint(nxml::Joint *pj)
{
	nxContext.Add(pj->id);
	pj->rotlimitmin = VectorMax(NxVec3(-180,-180,-180),pj->rotlimitmin);
	pj->rotlimitmax = VectorMin(NxVec3( 180, 180, 180),pj->rotlimitmax);
	pj->twistLimit.low.value  = pj->rotlimitmin.z*DEG2RAD;   // assume z is the twist axis  
	pj->twistLimit.high.value = pj->rotlimitmax.z*DEG2RAD;  // preprocessing should set z to be the one with highest limit.
	pj->swing1Limit.value = pj->rotlimitmax.x*DEG2RAD;  // assuming: rotlimitmin.x == -rotlimitmax.x  
	pj->swing2Limit.value = pj->rotlimitmax.y*DEG2RAD;
	NxVec3 r = pj->rotlimitmax-pj->rotlimitmin; // range of angular motion
	pj->twistMotion  = (r.z>=360) ? NX_D6JOINT_MOTION_FREE : (r.z<=0) ?NX_D6JOINT_MOTION_LOCKED : NX_D6JOINT_MOTION_LIMITED;
	pj->swing1Motion = (r.x>=180) ? NX_D6JOINT_MOTION_FREE : (r.x<=0) ?NX_D6JOINT_MOTION_LOCKED : NX_D6JOINT_MOTION_LIMITED;
	pj->swing2Motion = (r.y>=180) ? NX_D6JOINT_MOTION_FREE : (r.y<=0) ?NX_D6JOINT_MOTION_LOCKED : NX_D6JOINT_MOTION_LIMITED;
	pj->xMotion     = NX_D6JOINT_MOTION_LOCKED;
	pj->yMotion     = NX_D6JOINT_MOTION_LOCKED;
	pj->zMotion     = NX_D6JOINT_MOTION_LOCKED;
	if(pj->child) pj->actor[1] = pj->child->nxactor;
	pj->localAxis[1]  = pj->childplacement.z;
	pj->localNormal[1]= pj->childplacement.x;
	pj->localAnchor[1]= pj->childplacement.position;
	if(pj->parent)pj->actor[0] = pj->parent->nxactor;
	pj->localAxis[0]  = pj->parentplacement.z;
	pj->localNormal[0]= pj->parentplacement.x;
	pj->localAnchor[0]= pj->parentplacement.position;
	assert(pj->childplacement.specified ); 
	assert(pj->parentplacement.specified); 


	//pj->flags |= NX_D6JOINT_SLERP_DRIVE;
	//pj->slerpDrive.driveType = NX_D6JOINT_DRIVE_POSITION;
	//pj->slerpDrive.spring = 200;

	pj->nxjoint = nxscene->createJoint(*pj);
	if(!pj->nxjoint) 
	{
		NxOut("Joint Creation Failed\n");
	}
	nxContext.Pop();
	return pj->nxjoint;
}

/* 
// The following code was used to diagnose an issue we had with the slerpdrive
char *updatejoint(char *name)
{
	extern Hash<String,nxml::Joint*> joints;
	if(!joints.Exists(name)) return "no such joint";
	nxml::Joint *j=joints[name];
	j->nxjoint->isD6Joint()->setDriveOrientation(j->driveOrientation);
	return "jointupdated";
}
EXPORTFUNC(updatejoint);


NxQuat SetDriveOrientationActorPoseDelta(NxD6Joint *nxjoint,const NxQuat &q)
{
	NxD6JointDesc d6Desc;
	nxjoint->saveToDesc(d6Desc);
	NxMat33   mat0J(d6Desc.localAxis[0] , d6Desc.localNormal[0] , d6Desc.localAxis[0].cross( d6Desc.localNormal[0] ) );
	NxMat33   mat1J(d6Desc.localAxis[1] , d6Desc.localNormal[1] , d6Desc.localAxis[1].cross( d6Desc.localNormal[1] ) );
	mat1J.setTransposed();  // mat1J = Transpose(mat1J);
	NxMat33   matasd((NxQuat&)q);
	NxQuat sd(mat0J * NxMat33((NxQuat&)q) * mat1J);
	nxjoint->setDriveOrientation(sd);  // nx 2.2 may not have this function if not use loadfromdesc????
	return sd;
}

char *alljoints(char *p)
{
	float3 v = String(p).Asfloat3();  // since this is a console function its easier to type in yaw pitch roll
	Quaternion q = YawPitchRoll(v.x,v.y,v.z);
	extern Hash<String,nxml::Joint*> joints;
	for(int i=0;i<joints.slots_count;i++) if(joints.slots[i].used)
	{
		nxml::Joint *j=joints.slots[i].value;
		NxD6Joint *nxjoint = j->nxjoint->isD6Joint();
		if(!nxjoint) continue; //??
		NxQuat sd = SetDriveOrientationActorPoseDelta(nxjoint,(NxQuat&)q);
		j->driveOrientation = sd;
	}
	return "joints updated";
}
EXPORTFUNC(alljoints);
*/


void SubmitModel(nxml::Model *pm)
{
	int i;
	for(i=0;i<pm->rigidbody.count;i++)
	{
		NxActor *actor = SubmitRigidBody(pm->rigidbody[i]);
	}
	for(i=0;i<pm->joint.count;i++)
	{
		NxJoint *nxj   = SubmitJoint(pm->joint[i]);
	}
}

void nxmltosdk(nxml::Library *plib)
{
	int i;
	for(i=0;i<plib->material.count;i++)
	{
		SubmitMaterial(plib->material[i]);
	}
	for(i=0;i<plib->model.count;i++)
	{
		SubmitModel(plib->model[i]);
	}
}




// nxmlLoadScene:
// API entry point for using this code to load NXML into 
// their application.
NxScene *nxmlLoadScene(char *filename,NxPhysicsSDK* sdk,NxScene *scene)
{
	Library *library = nxmlLoadScene(filename);

	NxSceneDesc sd;
	sd.gravity = library->scenedesc.gravity;
	assert(sdk);
	nxsdk   = sdk;
	nxscene = (scene)?scene:nxsdk->createScene(sd);
	nxmltosdk(library);
	return nxscene;
}

