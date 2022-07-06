#ifdef WIN32 
	#define NOMINMAX 
	#include <windows.h>
#endif
#include <stdio.h>
#undef random
#include "NxPhysics.h"
#include "Terrain.h"
#include "TerrainRender.h"

#include <GL/gl.h>
#include <GL/glut.h>

void RenderTerrain(const TerrainData& terrain)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const NxU32* faces = terrain.faces;
    const NxVec3* colors = terrain.colors;
    const NxVec3* normals = terrain.normals;
    const NxVec3* verts = terrain.verts;

    glBegin(GL_TRIANGLES);
    glColor4f(0.5f, 0.4f, 0.2f, 1.0f);
    for (NxU32 i=0;i<terrain.nbFaces;i++)
    {
        glColor3fv(&colors[faces[i*3+0]].x);
        glNormal3fv(&normals[faces[i*3+0]].x);
        glVertex3fv(&verts[faces[i*3+0]].x);

        glColor3fv(&colors[faces[i*3+1]].x);
        glNormal3fv(&normals[faces[i*3+1]].x);
        glVertex3fv(&verts[faces[i*3+1]].x);

        glColor3fv(&colors[faces[i*3+2]].x);
        glNormal3fv(&normals[faces[i*3+2]].x);
        glVertex3fv(&verts[faces[i*3+2]].x);
    }
    glEnd();

    glShadeModel(GL_SMOOTH);
}

void RenderTerrainTriangle(const TerrainData& terrain, NxU32 i)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const NxU32* faces = terrain.faces;
    const NxVec3* colors = terrain.colors;
    const NxVec3* normals = terrain.normals;
    const NxVec3* verts = terrain.verts;

    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glNormal3fv(&normals[faces[i*3+0]].x);
    glVertex3fv(&verts[faces[i*3+0]].x);

    glNormal3fv(&normals[faces[i*3+1]].x);
    glVertex3fv(&verts[faces[i*3+1]].x);

    glNormal3fv(&normals[faces[i*3+2]].x);
    glVertex3fv(&verts[faces[i*3+2]].x);
    glEnd();
}

void RenderTerrainTriangles(const TerrainData& terrain, NxU32 nbTriangles, const NxU32* indices)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const NxU32* faces = terrain.faces;
    const NxVec3* colors = terrain.colors;
    const NxVec3* normals = terrain.normals;
    const NxVec3* verts = terrain.verts;

    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    while(nbTriangles--)
    {
        NxU32 index = *indices++;
        glNormal3fv(&normals[faces[index*3+0]].x);
        glVertex3fv(&verts[faces[index*3+0]].x);

        glNormal3fv(&normals[faces[index*3+1]].x);
        glVertex3fv(&verts[faces[index*3+1]].x);

        glNormal3fv(&normals[faces[index*3+2]].x);
        glVertex3fv(&verts[faces[index*3+2]].x);
    }
    glEnd();
}
