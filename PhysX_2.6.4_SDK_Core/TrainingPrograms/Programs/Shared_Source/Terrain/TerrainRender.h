#ifndef TERRAINRENDER_H
#define TERRAINRENDER_H

	void RenderTerrain(const TerrainData& terrain);
	void RenderTerrainTriangle(const TerrainData& terrain, NxU32 i);
	void RenderTerrainTriangles(const TerrainData& terrain, NxU32 nbTriangles, const NxU32* indices);

#endif
