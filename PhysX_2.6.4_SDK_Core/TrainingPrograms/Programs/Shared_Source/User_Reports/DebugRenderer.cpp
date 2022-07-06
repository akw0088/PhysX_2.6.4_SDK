
#include "DebugRenderer.h"
#include "NxDebugRenderable.h"
#include <GL/glut.h>

void DebugRenderer::renderBuffer(float* pVertList, float* pColorList, int type, int num)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT, 0, pVertList);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, pColorList);
	glDrawArrays(type, 0, num);
	glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void DebugRenderer::renderData(const NxDebugRenderable& data) const
{
	glLineWidth(1.0f);
	glDisable(GL_LIGHTING);

	unsigned int NbPoints = data.getNbPoints();
	if(NbPoints)
	{
		float* pVertList = new float[NbPoints*3];
    	float* pColorList = new float[NbPoints*4];
    	int vertIndex = 0;
    	int colorIndex = 0;
		const NxDebugPoint* Points = data.getPoints();
		while(NbPoints--)
		{
        	pVertList[vertIndex++] = Points->p.x;
        	pVertList[vertIndex++] = Points->p.y;
        	pVertList[vertIndex++] = Points->p.z;
        	pColorList[colorIndex++] = (float)((Points->color>>16)&0xff)/255.0f;
        	pColorList[colorIndex++] = (float)((Points->color>>8)&0xff)/255.0f;
        	pColorList[colorIndex++] = (float)(Points->color&0xff)/255.0f;
        	pColorList[colorIndex++] = 1.0f;
	      	Points++;
		}
		
		renderBuffer(pVertList, pColorList, GL_POINTS, data.getNbPoints());
    	
    	delete[] pVertList;
    	delete[] pColorList;
	}

	unsigned int NbLines = data.getNbLines();
	if(NbLines)
	{
		float* pVertList = new float[NbLines*3*2];
    	float* pColorList = new float[NbLines*4*2];
    	int vertIndex = 0;
    	int colorIndex = 0;
		const NxDebugLine* Lines = data.getLines();
		while(NbLines--)
		{
        	pVertList[vertIndex++] = Lines->p0.x;
        	pVertList[vertIndex++] = Lines->p0.y;
        	pVertList[vertIndex++] = Lines->p0.z;
        	pColorList[colorIndex++] = (float)((Lines->color>>16)&0xff)/255.0f;
        	pColorList[colorIndex++] = (float)((Lines->color>>8)&0xff)/255.0f;
        	pColorList[colorIndex++] = (float)(Lines->color&0xff)/255.0f;
        	pColorList[colorIndex++] = 1.0f;

        	pVertList[vertIndex++] = Lines->p1.x;
        	pVertList[vertIndex++] = Lines->p1.y;
        	pVertList[vertIndex++] = Lines->p1.z;
        	pColorList[colorIndex++] = (float)((Lines->color>>16)&0xff)/255.0f;
        	pColorList[colorIndex++] = (float)((Lines->color>>8)&0xff)/255.0f;
        	pColorList[colorIndex++] = (float)(Lines->color&0xff)/255.0f;
        	pColorList[colorIndex++] = 1.0f;

	      	Lines++;
		}
		
		renderBuffer(pVertList, pColorList, GL_LINES, data.getNbLines()*2);
    	
    	delete[] pVertList;
    	delete[] pColorList;
	}

	unsigned int NbTris = data.getNbTriangles();
	if(NbTris)
	{
		float* pVertList = new float[NbTris*3*3];
    	float* pColorList = new float[NbTris*4*3];
    	int vertIndex = 0;
    	int colorIndex = 0;
		const NxDebugTriangle* Triangles = data.getTriangles();
		while(NbTris--)
		{
        	pVertList[vertIndex++] = Triangles->p0.x;
        	pVertList[vertIndex++] = Triangles->p0.y;
        	pVertList[vertIndex++] = Triangles->p0.z;

        	pVertList[vertIndex++] = Triangles->p1.x;
        	pVertList[vertIndex++] = Triangles->p1.y;
        	pVertList[vertIndex++] = Triangles->p1.z;

        	pVertList[vertIndex++] = Triangles->p2.x;
        	pVertList[vertIndex++] = Triangles->p2.y;
        	pVertList[vertIndex++] = Triangles->p2.z;

			for(int i=0;i<3;i++)
			{
        		pColorList[colorIndex++] = (float)((Triangles->color>>16)&0xff)/255.0f;
        		pColorList[colorIndex++] = (float)((Triangles->color>>8)&0xff)/255.0f;
        		pColorList[colorIndex++] = (float)(Triangles->color&0xff)/255.0f;
        		pColorList[colorIndex++] = 1.0f;
			}

	      	Triangles++;
		}
		
		renderBuffer(pVertList, pColorList, GL_TRIANGLES, data.getNbTriangles()*3);
  	
    	delete[] pVertList;
    	delete[] pColorList;
	}
	glEnable(GL_LIGHTING);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

