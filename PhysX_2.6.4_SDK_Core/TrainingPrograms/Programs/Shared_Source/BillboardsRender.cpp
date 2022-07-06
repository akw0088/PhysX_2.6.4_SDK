#include "BillboardsRender.h"

TGAHeader tgaheader;									// TGA header
TGA tga;												// TGA image data

GLubyte uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
GLubyte cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	// Compressed TGA Header


bool LoadTGA(TGATexture * texture, char * filename)				// Load a TGA file
{
	FILE * fTGA;												// File pointer to texture file
	fTGA = fopen(filename, "rb");								// Open file for reading

	if(fTGA == NULL)											// If it didn't open....
	{
		MessageBox(NULL, "Could not open texture file", "ERROR", MB_OK);	// Display an error message
		return false;														// Exit function
	}

	if(fread(&tgaheader, sizeof(TGAHeader), 1, fTGA) == 0)					// Attempt to read 12 byte header from file
	{
		MessageBox(NULL, "Could not read file header", "ERROR", MB_OK);		// If it fails, display an error message 
		if(fTGA != NULL)													// Check to seeiffile is still open
		{
			fclose(fTGA);													// If it is, close it
		}
		return false;														// Exit function
	}

	if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)				// See if header matches the predefined header of 
	{																		// an Uncompressed TGA image
		LoadUncompressedTGA(texture, filename, fTGA);						// If so, jump to Uncompressed TGA loading code
	}
	else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)		// See if header matches the predefined header of
	{																		// an RLE compressed TGA image
		LoadCompressedTGA(texture, filename, fTGA);							// If so, jump to Compressed TGA loading code
	}
	else																	// If header matches neither type
	{
		MessageBox(NULL, "TGA file be type 2 or type 10 ", "Invalid Image", MB_OK);	// Display an error
		fclose(fTGA);
		return false;																// Exit function
	}
	return true;															// All went well, continue on
}

bool LoadUncompressedTGA(TGATexture * texture, char * filename, FILE * fTGA)	// Load an uncompressed TGA (note, much of this code is based on NeHe's 
{																			// TGA Loading code nehe.gamedev.net)
	if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Read TGA header
	{										
		MessageBox(NULL, "Could not read info header", "ERROR", MB_OK);		// Display error
		if(fTGA != NULL)													// if file is still open
		{
			fclose(fTGA);													// Close it
		}
		return false;														// Return failular
	}	

	texture->width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
	texture->bpp	= tga.header[4];										// Determine the bits per pixel
	tga.Width		= texture->width;										// Copy width into local structure						
	tga.Height		= texture->height;										// Copy height into local structure
	tga.Bpp			= texture->bpp;											// Copy BPP into local structure

	if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))	// Make sure all information is valid
	{
		MessageBox(NULL, "Invalid texture information", "ERROR", MB_OK);	// Display Error
		if(fTGA != NULL)													// Check if file is still open
		{
			fclose(fTGA);													// If so, close it
		}
		return false;														// Return failed
	}

	if(texture->bpp == 24)													// If the BPP of the image is 24...
		texture->type	= GL_RGB;											// Set Image type to GL_RGB
	else																	// Else if its 32 BPP
		texture->type	= GL_RGBA;											// Set image type to GL_RGBA

	tga.bytesPerPixel	= (tga.Bpp / 8);									// Compute the number of BYTES per pixel
	tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);		// Compute the total amout ofmemory needed to store data
	texture->imageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory

	if(texture->imageData == NULL)											// If no space was allocated
	{
		MessageBox(NULL, "Could not allocate memory for image", "ERROR", MB_OK);	// Display Error
		fclose(fTGA);														// Close the file
		return false;														// Return failed
	}

	if(fread(texture->imageData, 1, tga.imageSize, fTGA) != tga.imageSize)	// Attempt to read image data
	{
		MessageBox(NULL, "Could not read image data", "ERROR", MB_OK);		// Display Error
		if(texture->imageData != NULL)										// If imagedata has data in it
		{
			free(texture->imageData);										// Delete data from memory
		}
		fclose(fTGA);														// Close file
		return false;														// Return failed
	}

	// Byte Swapping Optimized By Steve Thomas
	for(GLuint cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel)
	{
		texture->imageData[cswap] ^= texture->imageData[cswap+2] ^=
		texture->imageData[cswap] ^= texture->imageData[cswap+2];
	}

	fclose(fTGA);															// Close file
	return true;															// Return success
}

bool LoadCompressedTGA(TGATexture * texture, char * filename, FILE * fTGA)		// Load COMPRESSED TGAs
{ 
	if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Attempt to read header
	{
		MessageBox(NULL, "Could not read info header", "ERROR", MB_OK);		// Display Error
		if(fTGA != NULL)													// If file is open
		{
			fclose(fTGA);													// Close it
		}
		return false;														// Return failed
	}

	texture->width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
	texture->bpp	= tga.header[4];										// Determine Bits Per Pixel
	tga.Width		= texture->width;										// Copy width to local structure
	tga.Height		= texture->height;										// Copy width to local structure
	tga.Bpp			= texture->bpp;											// Copy width to local structure

	if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))	//Make sure all texture info is ok
	{
		MessageBox(NULL, "Invalid texture information", "ERROR", MB_OK);	// If it isnt...Display error
		if(fTGA != NULL)													// Check if file is open
		{
			fclose(fTGA);													// Ifit is, close it
		}
		return false;														// Return failed
	}

	if(texture->bpp == 24)													// If the BPP of the image is 24...
		texture->type	= GL_RGB;											// Set Image type to GL_RGB
	else																	// Else if its 32 BPP
		texture->type	= GL_RGBA;											// Set image type to GL_RGBA

	tga.bytesPerPixel	= (tga.Bpp / 8);									// Compute BYTES per pixel
	tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);		// Compute amout of memory needed to store image
	texture->imageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory

	if(texture->imageData == NULL)											// If it wasnt allocated correctly..
	{
		MessageBox(NULL, "Could not allocate memory for image", "ERROR", MB_OK);	// Display Error
		fclose(fTGA);														// Close file
		return false;														// Return failed
	}

	GLuint pixelcount	= tga.Height * tga.Width;							// Nuber of pixels in the image
	GLuint currentpixel	= 0;												// Current pixel being read
	GLuint currentbyte	= 0;												// Current byte 
	GLubyte * colorbuffer = (GLubyte *)malloc(tga.bytesPerPixel);			// Storage for 1 pixel

	do
	{
		GLubyte chunkheader = 0;											// Storage for "chunk" header

		if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)				// Read in the 1 byte header
		{
			MessageBox(NULL, "Could not read RLE header", "ERROR", MB_OK);	// Display Error
			if(fTGA != NULL)												// If file is open
			{
				fclose(fTGA);												// Close file
			}
			if(texture->imageData != NULL)									// If there is stored image data
			{
				free(texture->imageData);									// Delete image data
			}
			return false;													// Return failed
		}

		if(chunkheader < 128)												// If the ehader is < 128, it means the that is the number of RAW color packets minus 1
		{																	// that follow the header
			chunkheader++;													// add 1 to get number of following color values
			for(short counter = 0; counter < chunkheader; counter++)		// Read RAW color values
			{
				if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel) // Try to read 1 pixel
				{
					MessageBox(NULL, "Could not read image data", "ERROR", MB_OK);		// IF we cant, display an error

					if(fTGA != NULL)													// See if file is open
					{
						fclose(fTGA);													// If so, close file
					}

					if(colorbuffer != NULL)												// See if colorbuffer has data in it
					{
						free(colorbuffer);												// If so, delete it
					}

					if(texture->imageData != NULL)										// See if there is stored Image data
					{
						free(texture->imageData);										// If so, delete it too
					}

					return false;														// Return failed
				}
																						// write to memory
				texture->imageData[currentbyte		] = colorbuffer[2];				    // Flip R and B vcolor values around in the process 
				texture->imageData[currentbyte + 1	] = colorbuffer[1];
				texture->imageData[currentbyte + 2	] = colorbuffer[0];

				if(tga.bytesPerPixel == 4)												// if its a 32 bpp image
				{
					texture->imageData[currentbyte + 3] = colorbuffer[3];				// copy the 4th byte
				}

				currentbyte += tga.bytesPerPixel;										// Increase thecurrent byte by the number of bytes per pixel
				currentpixel++;															// Increase current pixel by 1

				if(currentpixel > pixelcount)											// Make sure we havent read too many pixels
				{
					MessageBox(NULL, "Too many pixels read", "ERROR", NULL);			// if there is too many... Display an error!

					if(fTGA != NULL)													// If there is a file open
					{
						fclose(fTGA);													// Close file
					}	

					if(colorbuffer != NULL)												// If there is data in colorbuffer
					{
						free(colorbuffer);												// Delete it
					}

					if(texture->imageData != NULL)										// If there is Image data
					{
						free(texture->imageData);										// delete it
					}

					return false;														// Return failed
				}
			}
		}
		else																			// chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
		{
			chunkheader -= 127;															// Subteact 127 to get rid of the ID bit
			if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel)		// Attempt to read following color values
			{	
				MessageBox(NULL, "Could not read from file", "ERROR", MB_OK);			// If attempt fails.. Display error (again)

				if(fTGA != NULL)														// If thereis a file open
				{
					fclose(fTGA);														// Close it
				}

				if(colorbuffer != NULL)													// If there is data in the colorbuffer
				{
					free(colorbuffer);													// delete it
				}

				if(texture->imageData != NULL)											// If thereis image data
				{
					free(texture->imageData);											// delete it
				}

				return false;															// return failed
			}

			for(short counter = 0; counter < chunkheader; counter++)					// copy the color into the image data as many times as dictated 
			{																			// by the header
				texture->imageData[currentbyte		] = colorbuffer[2];					// switch R and B bytes areound while copying
				texture->imageData[currentbyte + 1	] = colorbuffer[1];
				texture->imageData[currentbyte + 2	] = colorbuffer[0];

				if(tga.bytesPerPixel == 4)												// If TGA images is 32 bpp
				{
					texture->imageData[currentbyte + 3] = colorbuffer[3];				// Copy 4th byte
				}

				currentbyte += tga.bytesPerPixel;										// Increase current byte by the number of bytes per pixel
				currentpixel++;															// Increase pixel count by 1

				if(currentpixel > pixelcount)											// Make sure we havent written too many pixels
				{
					MessageBox(NULL, "Too many pixels read", "ERROR", NULL);			// if there is too many... Display an error!

					if(fTGA != NULL)													// If there is a file open
					{
						fclose(fTGA);													// Close file
					}	

					if(colorbuffer != NULL)												// If there is data in colorbuffer
					{
						free(colorbuffer);												// Delete it
					}

					if(texture->imageData != NULL)										// If there is Image data
					{
						free(texture->imageData);										// delete it
					}

					return false;														// Return failed
				}
			}
		}
	}

	while(currentpixel < pixelcount);													// Loop while there are still pixels left
	fclose(fTGA);																		// Close the file
	return true;																		// return success
}

BillboardsRender::BillboardsRender(int maxParticles) : maxParticleVertices(maxParticles) 
{
	Init();
};

BillboardsRender::~BillboardsRender() 
{
	if (ParticleVertices)
		delete[] ParticleVertices;

	if (BillboardsVertices)
		delete[] BillboardsVertices;

	if (BillboardsTexelCoords)
		delete[] BillboardsTexelCoords;

	glDeleteTextures(1, texturePtr);
};

void BillboardsRender::Init() 
{
	numParticles = 0;
	scale = 0.5f;
	
	//Alloc memory for vertex arrays
	ParticleVertices = (float *)malloc(sizeof(float)*3*maxParticleVertices);
	BillboardsVertices = (float *)malloc(sizeof(float)*3*6*maxParticleVertices);	//six times because one particle is rendered as 2 triangles
	BillboardsTexelCoords = (float *)malloc(sizeof(float)*2*6*maxParticleVertices);

	texsize = 256;
	tgatexture = (TGATexture *)malloc(sizeof(TGATexture));
	//tgatexture.imageData = texturePtr;
	//LoadTGA(tgatexture, "../../../../Data/splash.tga");
	//LoadTGA(tgatexture, "../../../../Data/billboard_texture_alpha_yellow.tga");
	//LoadTGA(tgatexture, "../../../../Data/billboard_texture_opaque_pep.tga");

    // Data Directory Paths (solution, binary, install)
    char fname[] = "../../../../Data/Fluids";
    char fname2[] = "../../TrainingPrograms/Programs/Data/Fluids";
    char fname3[] = "TrainingPrograms/Programs/Data/Fluids";

	int set = 0;

#ifdef WIN32
	set = SetCurrentDirectory(&fname[0]);
	if (!set) set = SetCurrentDirectory(&fname2[0]);
	if (!set) set = SetCurrentDirectory(&fname3[0]);
#elif LINUX
	set = chdir(&fname[0]);
	//if (set != 0) set = chdir(&fname[1]);
	if (set != 0) set = chdir(&fname2[0]);
	if (set != 0) set = chdir(&fname3[0]);
#endif

	LoadTGA(tgatexture, "splash.tga");
	
	if (tgatexture) {
		printf("Loaded Billboards Texture\n");

	//Create Texture and Load Texture File
	glGenTextures(1, &textureID);
	  
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, tgatexture->width, tgatexture->height,
                      GL_RGBA, GL_UNSIGNED_BYTE, tgatexture->imageData);
  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	}
	else {
		return;
		printf("Could not load Billboards Texture\n");
	}

};

void BillboardsRender::CreateBillboardsQuads(NxVec3 eye, NxVec3 facing) {
//Creates Billboard vertices from particle positions
//facing is the viewing direction (away from eyepos)
	NxVec3 up, right, curcorner, tdir;
	NxVec3 *pos;
	
	float * billboardpos, * billboardtexelpos;

	up = NxVec3(0,1,0);
	right = facing.cross(up);
	up = right.cross(facing);
	right.normalize();
	up.normalize();
	facing.normalize();

	right *= scale;
	up *= scale;
	
	tdir = NxVec3(0, 0, -1.0f);	//normal

if (BillboardsVertices && BillboardsTexelCoords) {
	
		//sort particles according to projected distance from camera
		DepthposVec depthposarray(numParticles);
		float * fpos = ParticleVertices;

		//fill array with depth and pointer to pos
		for(int i=0;i<numParticles; i++)
		{
			depthposarray[i].depth = facing.dot(NxVec3(fpos)-eye);
			depthposarray[i].ptrpos = fpos;
			fpos += 3;
		}
		
		//sort the array
		std::sort(depthposarray.begin(), depthposarray.end(), DepthposSort());

		pos = (NxVec3 *)ParticleVertices;
		billboardpos = BillboardsVertices;
		billboardtexelpos = BillboardsTexelCoords;
		for (int i=0; i< numParticles; i++) {
			//create billboard vertices in pr.mBillBoards
			//store 6 vertices, two of them twice so that we can render triangles
			
			pos = (NxVec3*)depthposarray[i].ptrpos;
			//printf("Render pos: (%.3f, %.3f, %.3f) dist: %.3f\n",pos->x, pos->y, pos->z, depthposarray[i].depth);
			
			//upperleft corner
			curcorner = *pos - right + up;
			billboardpos[0] = curcorner.x;
			billboardpos[1] = curcorner.y;
			billboardpos[2] = curcorner.z;

			billboardtexelpos[0] = 0.0f;
			billboardtexelpos[1] = 0.0f;

			billboardpos += 3;
			billboardtexelpos +=2;

			//lowerleft corner
			curcorner = *pos - right - up;
			billboardpos[0] = curcorner.x;
			billboardpos[1] = curcorner.y;
			billboardpos[2] = curcorner.z;

			billboardtexelpos[0] = 0.0f;
			billboardtexelpos[1] = 1.0f;

			billboardpos += 3;
			billboardtexelpos +=2;
			
			//upperright corner
			curcorner = *pos + right + up;
			billboardpos[0] = curcorner.x;
			billboardpos[1] = curcorner.y;
			billboardpos[2] = curcorner.z;

			billboardtexelpos[0] = 1.0f;
			billboardtexelpos[1] = 0.0f;

			billboardpos += 3;
			billboardtexelpos +=2;

			//upperright corner
			curcorner = *pos + right + up;
			billboardpos[0] = curcorner.x;
			billboardpos[1] = curcorner.y;
			billboardpos[2] = curcorner.z;

			billboardtexelpos[0] = 1.0f;
			billboardtexelpos[1] = 0.0f;

			billboardpos += 3;
			billboardtexelpos +=2;
			
			//lowerleft corner
			curcorner = *pos - right - up;
			billboardpos[0] = curcorner.x;
			billboardpos[1] = curcorner.y;
			billboardpos[2] = curcorner.z;

			billboardtexelpos[0] = 0.0f;
			billboardtexelpos[1] = 1.0f;

			billboardpos += 3;
			billboardtexelpos +=2;
			
			//lowerright corner
			curcorner = *pos + right - up;
			billboardpos[0] = curcorner.x;
			billboardpos[1] = curcorner.y;
			billboardpos[2] = curcorner.z;

			billboardtexelpos[0] = 1.0f;
			billboardtexelpos[1] = 1.0f;

			billboardpos += 3;
			billboardtexelpos +=2;

	}
	}
};


void BillboardsRender::Render() {
	
	if (BillboardsVertices && BillboardsTexelCoords) {
		
		glVertexPointer(3, GL_FLOAT, 0, BillboardsVertices);
		//glNormalPointer(GL_FLOAT, sizeof(float), &testVertNormalList);

		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, BillboardsTexelCoords);

		glEnableClientState(GL_VERTEX_ARRAY );
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		//glEnableClientState(GL_NORMAL_ARRAY );

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		
		glBindTexture(GL_TEXTURE_2D, textureID);	
		glNormal3f(0,0,1.0f);

		//if (numParticles > 100)	glDrawArrays(GL_TRIANGLES, 0, 6*100); else 
			glDrawArrays(GL_TRIANGLES, 0, numParticles*6);
		
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
	}
};

void BillboardsRender::PrintoutBillboardsVertices(int count) {
	float * pos = BillboardsVertices;
	for (int i =0; i<count; i++)
		printf("%i : %.3f %.3f %.3f\n", i, *pos++, *pos++, *pos++);

}
