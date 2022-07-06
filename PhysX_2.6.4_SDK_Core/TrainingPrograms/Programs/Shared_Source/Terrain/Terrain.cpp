#ifdef WIN32 
	#define NOMINMAX 
	#include <windows.h>
#endif
#include <stdio.h>
#undef random
#include "NxPhysics.h"
#include "Terrain.h"

#define ONE_OVER_RAND_MAX	(1.0f / NxF32(RAND_MAX))

TerrainData::TerrainData() :
	size	(0),
	nbVerts	(0),
	nbFaces	(0),
	offset	(0.0f),
	width	(0.0f),
	chaos	(0.0f),
	verts	(NULL),
	colors	(NULL),
	normals	(NULL),
	faces	(NULL)
	{
	}

TerrainData::~TerrainData()
	{
	release();
	}

void TerrainData::release()
	{
	NX_FREE(normals);
	NX_FREE(faces);
	NX_FREE(colors);
	NX_FREE(verts);
	}

void TerrainData::init(NxU32 s, NxF32 o, NxF32 w, NxF32 c, bool flat)
	{
	release();

	size	= s;
	offset	= o;
	width	= w;
	chaos	= c;
	nbVerts = size*size;
	nbFaces	= (size-1)*(size-1)*2;

	////////

	// Initialize terrain vertices
	verts = (NxVec3*)NX_ALLOC(sizeof(NxVec3)*nbVerts);
	for(NxU32 y=0;y<size;y++)
		{
		for(NxU32 x=0;x<size;x++)
			{
			verts[x+y*size] = NxVec3(NxF32(x)-(NxF32(size-1)*0.5f), 0.0f, NxF32(y)-(NxF32(size-1)*0.5f)) * width;
			}
		}

	// Initialize terrain colors
		{
		colors = (NxVec3*)NX_ALLOC(sizeof(NxVec3)*nbVerts);
		for(NxU32 y=0;y<size;y++)
			{
			for(NxU32 x=0;x<size;x++)
				{
				colors[x+y*size] = NxVec3(0.5f, 0.4f, 0.2f);
				}
			}
		}

	// Initialize terrain faces
	faces = (NxU32*)NX_ALLOC(sizeof(NxU32)*nbFaces*3);
	NxU32 k = 0;
	for(NxU32 j=0;j<size-1;j++)
		{
		for(NxU32 i=0;i<size-1;i++)
			{
			// Create first triangle
			faces[k++] = i   + j*size;
			faces[k++] = i   + (j+1)*size;
			faces[k++] = i+1 + (j+1)*size;

			// Create second triangle
			faces[k++] = i   + j*size;
			faces[k++] = i+1 + (j+1)*size;
			faces[k++] = i+1 + j*size;
			}
		}

	struct Local
		{
		static void _Compute(bool* done, NxVec3* field, NxU32 x0, NxU32 y0, NxU32 currentSize, NxF32 value, NxU32 initSize)
			{
			// Compute new size
			currentSize>>=1;
			if(!currentSize) return;

			// Compute new heights
			NxF32 v0 = (value * NxF32(rand()-16384) * ONE_OVER_RAND_MAX);
			NxF32 v1 = (value * NxF32(rand()-16384) * ONE_OVER_RAND_MAX);
			NxF32 v2 = (value * NxF32(rand()-16384) * ONE_OVER_RAND_MAX);
			NxF32 v3 = (value * NxF32(rand()-16384) * ONE_OVER_RAND_MAX);
			NxF32 v4 = (value * NxF32(rand()-16384) * ONE_OVER_RAND_MAX);

			NxU32 x1 = (x0+currentSize)				% initSize;
			NxU32 x2 = (x0+currentSize+currentSize)	% initSize;
			NxU32 y1 = (y0+currentSize)				% initSize;
			NxU32 y2 = (y0+currentSize+currentSize)	% initSize;

			if(!done[x1 + y0*initSize])	field[x1 + y0*initSize].y = v0 + 0.5f * (field[x0 + y0*initSize].y + field[x2 + y0*initSize].y);
			if(!done[x0 + y1*initSize])	field[x0 + y1*initSize].y = v1 + 0.5f * (field[x0 + y0*initSize].y + field[x0 + y2*initSize].y);
			if(!done[x2 + y1*initSize])	field[x2 + y1*initSize].y = v2 + 0.5f * (field[x2 + y0*initSize].y + field[x2 + y2*initSize].y);
			if(!done[x1 + y2*initSize])	field[x1 + y2*initSize].y = v3 + 0.5f * (field[x0 + y2*initSize].y + field[x2 + y2*initSize].y);
			if(!done[x1 + y1*initSize])	field[x1 + y1*initSize].y = v4 + 0.5f * (field[x0 + y1*initSize].y + field[x2 + y1*initSize].y);

			done[x1 + y0*initSize] = true;
			done[x0 + y1*initSize] = true;
			done[x2 + y1*initSize] = true;
			done[x1 + y2*initSize] = true;
			done[x1 + y1*initSize] = true;

			// Recurse through 4 corners
			value *= 0.5f;
			_Compute(done, field, x0,	y0,	currentSize, value, initSize);
			_Compute(done, field, x0,	y1,	currentSize, value, initSize);
			_Compute(done, field, x1,	y0,	currentSize, value, initSize);
			_Compute(done, field, x1,	y1,	currentSize, value, initSize);
			}
		};

	// Fractalize
	bool* done = (bool*)NX_ALLOC_TMP(sizeof(bool)*nbVerts);
	memset(done, 0, nbVerts);
	verts[0].y = 10.0f;
	verts[size-1].y = 10.0f;
	verts[size*(size-1)].y = 10.0f;
	verts[nbVerts-1].y = 10.0f;
	Local::_Compute(done, verts, 0, 0, size, chaos, size);
	for(NxU32 i=0;i<nbVerts;i++)	verts[i].y += offset;
	NX_FREE(done);

	// Create a flat area in our terrain
	if(flat)
		{
		NxU32 a = ((size)/2) - ((size)/8);
		NxU32 b = ((size)/2) + ((size)/8);
		for(NxU32 y=a;y<b;y++)
			{
			for(NxU32 x=a;x<b;x++)
				{
				verts[x+y*size].y = 0.0f;
				colors[x+y*size].x = 0.3f;
				colors[x+y*size].y = 0.3f;
				colors[x+y*size].z = 0.3f;
				}
			}
		}

	// Build vertex normals
	normals = (NxVec3*)NX_ALLOC(sizeof(NxVec3)*nbVerts);
//	NxBuildSmoothNormals(nbFaces, nbVerts, verts, faces, NULL, normals, true);
	}
