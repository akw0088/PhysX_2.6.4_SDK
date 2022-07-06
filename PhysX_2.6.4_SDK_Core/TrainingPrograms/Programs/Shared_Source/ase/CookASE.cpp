#include "CookASE.h"

#include <NxPhysics.h>
#include <NxCooking.h>
#include <windows.h>

#include "Stream.h"

NxVec3 _offset;

char* trimFront(char* c) 
{
	while(*c == ' ' || *c == '\t')
		c++;
	return c;
}

void readVertices(FILE* f, NxArray<NxVec3>& vertices, NxVec3 scale) 
{
	char line[512];

	while(true) {
		fgets(line, 512, f);
		char* l = trimFront(line);
		if (*l == '}') {
			return;
		} else if (strstr(l, "*MESH_VERTEX")) {
			float a,b,c;
			int i;
			sscanf(l, "*MESH_VERTEX %d %f %f %f", &i, &a, &b, &c);
			NxVec3 newV(a,c,-b);
			newV.x *= scale.x;
			newV.y *= scale.y;
			newV.z *= scale.z;
			vertices[i] = newV;
			vertices[i] += _offset;
		}
	}
}

void readFaces(FILE* f, NxArray<NxU32>& faces) 
{
	char line[512];

	while(true) {
		fgets(line, 512, f);
		char* l = trimFront(line);
		if (*l == '}') {
			return;
		} else if (strstr(l, "*MESH_FACE")) {
			int a,b,c;
			int i;
			sscanf(l, "*MESH_FACE %d: A: %d B: %d C: %d ", &i, &a, &b, &c);
			faces[3*i+0] = a;
			faces[3*i+1] = b;
			faces[3*i+2] = c;
		}
	}
}

void readMesh(FILE* f, NxArray<NxVec3>& vertices, NxArray<NxU32>& faces, NxVec3 scale) 
{
	char line[512];
	int nbVertices;
	int nbFaces;
	while(true) {
		fgets(line, 512, f);
		char* l = trimFront(line);
		if (*l == '}') {
			return;
		} else if (strstr(l, "*MESH_NUMVERTEX")) {
			sscanf(l, "*MESH_NUMVERTEX %d", &nbVertices);
			vertices.resize(nbVertices);
		} else if (strstr(l, "*MESH_NUMFACES")) {
			sscanf(l, "*MESH_NUMFACES %d", &nbFaces);
			faces.resize(nbFaces*3);
		} else if (strstr(l, "*MESH_VERTEX_LIST {")) {
			readVertices(f, vertices, scale);
		} else if (strstr(l, "*MESH_FACE_LIST {")) {
			readFaces(f, faces);
		}
	}
}

NxActor* CookASE(const std::string& filename, NxScene* scene, NxVec3 offset, NxVec3 scale) 
{
	_offset = offset;
	FILE* f = fopen(filename.c_str(), "rb");
	if (f == NULL) 
	{
		printf("File not found: %s\n", filename.c_str());
		return NULL;
	}

	NxArray<NxVec3> vertices;
	NxArray<NxU32> faces;

	char line[512];
	int linenbr = 0;
	while (!feof(f)) 
	{
		fgets(line, 512, f);
		char* l = trimFront(line);
		if (!strncmp(l, "*MESH {", 7)) 
		{
			readMesh(f, vertices, faces, scale);
		} 
		else 
		{
			//printf("Line %4d: %s\n", linenbr++, l);
		}
	}

	fclose(f);

	// Build physical model
	NxTriangleMeshDesc meshDesc;
	meshDesc.numVertices					= vertices.size();
	meshDesc.numTriangles					= faces.size()/3;
	meshDesc.pointStrideBytes				= sizeof(NxVec3);
	meshDesc.triangleStrideBytes			= 3*sizeof(NxU32);
	meshDesc.points							= &vertices[0].x;
	meshDesc.triangles						= &faces[0];
	meshDesc.flags							= NX_MF_HARDWARE_MESH;

//	meshDesc.heightFieldVerticalAxis		= NX_Y;
//	meshDesc.heightFieldVerticalExtent		= -1000.0f;

	MemoryWriteBuffer buf;
	bool status = NxCookTriangleMesh(meshDesc, buf);
	NxTriangleMesh* mesh = scene->getPhysicsSDK().createTriangleMesh(MemoryReadBuffer(buf.data));
	// scene->createFluidHardwareTriangleMesh(MemoryReadBuffer(buf.data));

	NxTriangleMeshShapeDesc meshShapeDesc;
	meshShapeDesc.meshData = mesh;

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&meshShapeDesc);
	
	NxActor* a = scene->createActor(actorDesc);

#if 0
	assert(a != NULL);
	assert(a->getNbShapes() == 1);
	NxShape* s = a->getShapes()[0];
	assert(s != NULL);
	NxTriangleMeshShape* ts = s->isTriangleMesh();
	assert(ts != NULL);
	NxTriangleMeshDesc* tmd= new NxTriangleMeshDesc();
	ts->getTriangleMesh().saveToDesc(*tmd);
	ts->userData = tmd;
#endif  // 0

	return a;
}
