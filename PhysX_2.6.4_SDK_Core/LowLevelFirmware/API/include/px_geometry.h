#ifndef PXD_GEOMETRY_H
#define PXD_GEOMETRY_H

#include "px_config.h"
#include "px_task.h"
#include "px_dynamics.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct PxConvexMeshData_ PxConvexMeshData;
typedef struct PxInternalTriangleMeshData_ PxInternalTriangleMeshData;

/*!
\file
Geometry interface
*/

/************************************************************************/
/* Shapes                                                               */
/************************************************************************/

typedef PxdHandle PxdShape;

/*!
Shape types
*/
typedef enum PxdShapeType_
{
	/*!
	Not a valid shape type.
	*/
	PXD_SHAPE_TYPE_NONE = 0,
	/*!
	Primitive shape. Sphere shape.
	*/
	PXD_SHAPE_TYPE_SPHERE = 1,
	/*!
	Primitive shape. 
	Plane defining a half-space, in this case x > 0
	*/
	PXD_SHAPE_TYPE_PLANE,
	/*!
	Primitive shape. Capsule shape, extension along X-axis
	*/
	PXD_SHAPE_TYPE_CAPSULE,
	/*!
	NOT IMPLEMENTED
	*/
	PXD_SHAPE_TYPE_CYLINDER,
	/*!
	Primitive shape. Box shape.
	*/
	PXD_SHAPE_TYPE_BOX,
	/*!
	Primitive shape. A precooked convex mesh.
	*/
	PXD_SHAPE_TYPE_OPTIMIZED_CONVEX_MESH,
	/*!
	Primitive shape. A precooked mesh.
	*/
	PXD_SHAPE_TYPE_OPTIMIZED_MESH,
	/*!
	Primitive shape. A height-field
	*/
	PXD_SHAPE_TYPE_HEIGHTFIELD,
	/*!
	Primitive shape. NOT IMPLEMENTED
	*/
	PXD_SHAPE_TYPE_TRIANGLES,
	/*!
	Derived shape. The body shape uses a body atom to 
	position a primitive shape.
	*/
	PXD_SHAPE_TYPE_BODY,
	/*!
	Derived shape. A collection of particle atoms.
	NOT IMPLEMENTED
	*/
	PXD_SHAPE_TYPE_PARTICLE_CLOUD,
	/*!
	Derived shape. A collection of line segments connecting 
	particle atoms.
	NOT IMPLEMENTED
	*/
	PXD_SHAPE_TYPE_PARTICLE_WIREFRAME,
	/*!
	Derived shape. A collection of triangles connecting particle atoms.
	NOT IMPLEMENTED
	*/
	PXD_SHAPE_TYPE_PARTICLE_MESH,
	/*!
	Total number of shape-types
	*/
	PXD_SHAPE_TYPE_COUNT
} PxdShapeType;

/*!

*/
typedef enum PxdShapeProperty_
{
	/*!
	For PXD_SHAPE_TYPE_SPHERE, PXD_SHAPE_TYPE_CAPSULE and PXD_SHAPE_TYPE_CYLINDER
	this is the radius.
	*/
	PXD_SHAPE_RADIUS,
	/*!
	For PXD_SHAPE_TYPE_CAPSULE and PXD_SHAPE_TYPE_CYLINDER this represents
	the extent along the X axis, from the center out to the end, not 
	including the hemisphere on capsules (1 PxFloat, not negative).
	For PXD_SHAPE_TYPE_BOX this represents the box extent along x, y and z,
	measured from the center (1 PxdVector, not negative).
	*/
	PXD_SHAPE_EXTENT,
	/*!
	The body shape's atom identifier. This must be an identifier to
	a body atom.
	*/
	PXD_SHAPE_BODY_ATOM,
	/*!
	The body shape's shape identifier. This must be an identifier to
	a primitive shape.
	*/
	PXD_SHAPE_BODY_SHAPE,
	/*!
	The body shape's pose, relative to the body atom it is connected to.
	1 PxdTransform
	*/
	PXD_SHAPE_BODY_POSE,
	/*!
	Precooked mesh data. Either convex, triangle mesh or height-field, depending on
	shape type.
	*/
	PXD_SHAPE_MESH_DATA,

	/*!
	Heightfield number of rows. 1 PxInt
	*/	
	PXD_SHAPE_HF_ROWS,
	/*!
	Heightfield number of columns. 1 PxInt
	*/
	PXD_SHAPE_HF_COLUMNS,
	/*!
	Number of bytes between consecutive samples in the samples array. 1 PxInt
	*/
	PXD_SHAPE_HF_STRIDE, 
	/*!
	Heightfield samples array. 1 void*
	*/
	PXD_SHAPE_HF_SAMPLES,
	/*!
	Heightfield row scale. 1 PxFloat
	*/
	PXD_SHAPE_HF_ROW_SCALE,
	/*!
	Heightfield row scale. 1 PxFloat
	*/
	PXD_SHAPE_HF_COLUMN_SCALE,
	/*!
	Heightfield height scale. 1 PxFloat
	*/
	PXD_SHAPE_HF_HEIGHT_SCALE,
	/*!
	Heightfield vertical extent. 1 PxFloat
	*/
	PXD_SHAPE_HF_VERTICAL_EXTENT,
	/*!
	Heightfield convex edge threshold. 1 PxFloat
	*/
	PXD_SHAPE_HF_CONVEX_EDGE_THRESHOLD,
	/*!
	Heightfield flags. 1 PxInt
	In Novodex there where two sets of flags related to heightfields.
	NxHeightFieldDesc.flags which could have the value NX_HF_NO_BOUNDARY_EDGES (=1)
	and NxHeightFieldShapeDesc.meshFlags which could have the value NX_MESH_SMOOTH_SPHERE_COLLISIONS (=1).
	This parameter is used for both of them.
	NO_BOUNDARY_EDGES = 1
	SMOOTH_SPHERE_COLLISIONS = 2
	Thus, the mapping becomes:
	PxdShapeDesc.heightfield.flags = NxHeightFieldDesc.flags | (NxHeightFieldShapeDesc.meshFlags << 1)
	*/
	PXD_SHAPE_HF_FLAGS,
	/*!
	Heightfield hole material index. 1 PxInt
	*/
	PXD_SHAPE_HF_HOLE_MATERIAL,
	/*!
	Flag for convex and triangle meshes defining if collision normals
	when colliding with spheres should be smoothed or face normals.
	1 PxInt - flag
	*/
	PXD_SHAPE_SMOOTH_SPHERE_NORMALS,
	/*!
	Shape user data (void*). 
	*/
	PXD_SHAPE_USER_DATA
} PxdShapeProperty;


typedef struct PxdShapeDesc_
{
	/*!
	Shape type

	\sa PxdShapeType
	*/
	PxdShapeType type;


	/*!
	Shape user data
	
	\sa PXD_SHAPE_USER_DATA
	*/
	void* userData;

	union
	{
		/*!
		Initial parameters for type PXD_SHAPE_TYPE_BOX
		*/
		struct
		{
			/*!
			Length of half-side of box.

			\sa PXD_SHAPE_EXTENT
			*/
			PxdVector halfSide;
		} box;

		/*!
		Initial parameters for type PXD_SHAPE_TYPE_CAPSULE
		*/
		struct
		{
			/*!
			Capsule radius

			\sa PXD_SHAPE_RADIUS
			*/
			PxFloat radius;
			
			/*!
			Half-extent of the capsule segment along local x-axis

			\sa PXD_SHAPE_EXTENT
			*/
			PxFloat halfHeight;
		} capsule;

		/*!
		Initial parameters for type PXD_SHAPE_TYPE_SPHERE
		*/
		struct
		{
			/*!
			Sphere radius

			\sa PXD_SHAPE_RADIUS
			*/
			PxFloat radius;
		} sphere;

		/*!
		Initial parameters for type PXD_SHAPE_TYPE_OPTIMIZED_CONVEX_MESH
		*/
		struct 
		{
			/*!
			Data-structure shared with user.
			\warning LL assumes this is correct, don't screw up.

			\sa PXD_SHAPE_MESH_DATA
			*/
			PxConvexMeshData* convexMesh;

			/*!
			Use interpolated normals for collision against spheres.
			*/
			PxBool smoothNormals;
		} convex;

		/*!
		Initial parameters for type PXD_SHAPE_TYPE_OPTIMIZED_MESH
		*/
		struct 
		{
			/*!
			Data-structure shared with user.
			\warning LL assumes this is correct, don't screw up.

			\sa PXD_SHAPE_MESH_DATA
			*/
			PxInternalTriangleMeshData* mesh;

			/*!
			Use interpolated normals for collision against spheres.
			*/
			PxBool smoothNormals;
		} mesh;

		/*!
		Initial parameters for type PXD_SHAPE_TYPE_HEIGHTFIELD
		*/
		struct
		{
			PxU32	rows;
			PxU32	columns;
			PxU32	stride;
			void*	samples;
			PxFloat rowScale;
			PxFloat columnScale;
			PxFloat heightScale;
			PxFloat verticalExtent;
			PxU32	flags;
			PxU32	holeMaterial;
			PxFloat convexEdgeThreshold;
		} heightfield;

		/*!
		Initial parameters for type PXD_SHAPE_TYPE_BODY
		*/
		struct
		{
			/*!
			Body atom identifier

			\sa PXD_SHAPE_BODY_ATOM
			*/
			PxdAtom atom;

			/*!
			Primitive shape identifier

			\sa PXD_SHAPE_BODY_SHAPE
			*/
			PxdShape shape;

			/*!
			Shape-to-atom relative transform

			\sa PXD_SHAPE_BODY_POSE
			*/
			PxdTransform relativeTransform;

			/*!
			mesh page addr, to page tri mesh page onto the card.
			*/

			void* meshPageAddr;

			/*!
			mesh page length, to page tri mesh page onto the card.
			*/

			PxU32 meshPageLength;
		} body;

	};
} PxdShapeDesc;

/*!
Initialize shape descriptor
*/
PXD_EXPORT_FUNCTION void PxdShapeInitDesc(PxdShapeDesc* desc);

/*!
Create shape
\param type
Shape type identifier. This cannot be changed afterwards.
\return
Shape identifier. Zero on failure.
*/
PXD_EXPORT_FUNCTION PxdShape PxdShapeCreate(PxdContext context, PxdShapeDesc* desc);

/*!
Destroy shape
\param shape
Shape identifier
*/
PXD_EXPORT_FUNCTION void PxdShapeDestroy(PxdShape shape);

/*!
Return the shape type for a specific shape
\param shape
Shape identifier
\return
Shape type identifier. Zero if shape identifier is invalid.
*/
PXD_EXPORT_FUNCTION PxdShapeType PxdShapeGetType(PxdShape shape);


/*!
\addtogroup Shape property set/get
@{
*/
PXD_EXPORT_FUNCTION void PxdShapeSetInt(PxdShape shape, PxdShapeProperty property, PxInt value);
PXD_EXPORT_FUNCTION PxInt PxdShapeGetInt(PxdShape shape, PxdShapeProperty property);

PXD_EXPORT_FUNCTION void PxdShapeSetFloat(PxdShape shape, PxdShapeProperty property, PxFloat value);
PXD_EXPORT_FUNCTION PxFloat PxdShapeGetFloat(PxdShape shape, PxdShapeProperty property);

PXD_EXPORT_FUNCTION void PxdShapeSetProperty(PxdShape shape, PxdShapeProperty property, void* data, size_t size);
PXD_EXPORT_FUNCTION void PxdShapeGetProperty(PxdShape shape, PxdShapeProperty property, void* data, size_t size);

PXD_EXPORT_FUNCTION void PxdShapeSetVec(PxdShape Shape, PxdShapeProperty property, PxdVector v);
PXD_EXPORT_FUNCTION PxdVector PxdShapeGetVec(PxdShape Shape, PxdShapeProperty property);

PXD_EXPORT_FUNCTION void PxdShapeSetQuat(PxdShape Shape, PxdShapeProperty property, PxdQuaternion q);
PXD_EXPORT_FUNCTION PxdQuaternion PxdShapeGetQuat(PxdShape Shape, PxdShapeProperty property);

PXD_EXPORT_FUNCTION void PxdShapeSetTransform(PxdShape Shape, PxdShapeProperty property, PxdTransform t);
PXD_EXPORT_FUNCTION PxdTransform PxdShapeGetTransform(PxdShape Shape, PxdShapeProperty property);

PXD_EXPORT_FUNCTION void PxdShapeSetAtom(PxdShape shape, PxdShapeProperty property, PxdAtom atom);
PXD_EXPORT_FUNCTION PxdAtom PxdShapeGetAtom(PxdShape shape, PxdShapeProperty property);

PXD_EXPORT_FUNCTION PxdShape PxdShapeGetShape(PxdShape shape, PxdShapeProperty property);	

/*!
@}
*/


/************************************************************************/
/* Queries                                                              */
/************************************************************************/

typedef PxdHandle PxdShapeQuery;

/*!
Shape query types. This is used when creating a new shape query.
*/
typedef enum PxdShapeQueryType_
{
	/*!
	Not a valid query type
	*/
	PXD_SHAPE_QUERY_TYPE_NONE = 0,

	/*!
	Report overlap between two shapes.
	*/
	PXD_SHAPE_QUERY_TYPE_OVERLAP,
	/*!
	Report closest distance between two shapes. This includes
	the closest points, their normals and features on both shapes.
	*/
	PXD_SHAPE_QUERY_TYPE_DISTANCE,
	/*!
	Report maximum length of a linear sweep of first shape before 
	hitting second shape. This query also reports hit point on
	both shapes and their normals as well as contact features.
	*/
	PXD_SHAPE_QUERY_TYPE_SWEEP
} PxdShapeQueryType;

/*!
Shape query status result
*/
typedef enum PxdShapeQueryStatus_
{
	PXD_SHAPE_QUERY_STATUS_SEPARATION,
	PXD_SHAPE_QUERY_STATUS_PENETRATION,
	PXD_SHAPE_QUERY_STATUS_HIT,
	PXD_SHAPE_QUERY_STATUS_MISS
} PxdShapeQueryStatus;

/*!
Shape query properties
*/
typedef enum PxdShapeQueryProperty_
{
	/*!
	The relative transform between the two shapes (7 PxFloats, in 
	order pos.x, pos.y, pos.y, rot.x, rot.y, rot.z, rot.w).
	Default value is unit transform.
	*/
	PXD_SHAPE_QUERY_RELATIVE_TRANSFORM,
	/*!
	The status of a shape query (1 pxShapeQueryStatus, read-only)
	*/
	PXD_SHAPE_QUERY_STATUS,
	/*!
	The direction vector of a sweep query (3 PxFloats, normalized 
	vector) in first shape local space.
	*/
	PXD_SHAPE_QUERY_DIRECTION,
	/*!
	Maximum sweep length (1 PxFloat) for sweep queries.
	*/
	PXD_SHAPE_QUERY_MAX_LENGTH,
	/*!
	Closest point on first shape in local space(3 PxFloats, read-only)
	Undefined if status is not PXD_SHAPE_QUERY_STATUS_HIT.
	*/
	PXD_SHAPE_QUERY_POINT0,
	/*!
	Closest point on second shape in local space(3 PxFloats, read-only)
	Undefined if status is not PXD_SHAPE_QUERY_STATUS_HIT.
	*/
	PXD_SHAPE_QUERY_POINT1,
	/*!
	Closest feature on first shape (1 PxInt, read-only)
	Currently only used for optimized mesh, where feature equals 
	triangle index. Undefined if status is not PXD_SHAPE_QUERY_STATUS_HIT.
	*/
	PXD_SHAPE_QUERY_FEATURE0,
	/*!
	Closest feature on second shape (1 PxInt, read-only)
	Currently only used for optimized mesh, where feature equals 
	triangle index. Undefined if status is not PXD_SHAPE_QUERY_STATUS_HIT.
	*/
	PXD_SHAPE_QUERY_FEATURE1,
	/*!
	Actual sweep length (1 PxFloat, read-only). 
	Undefined if status is not PXD_SHAPE_QUERY_STATUS_HIT.
	*/
	PXD_SHAPE_QUERY_LENGTH,
} PxdShapeQueryProperty;

/*!
Create shape query
\param type
Shape query type identifier. This cannot be changed afterwards.
\param shape0
First shape this query should use
\param shape1
Second shape this query should use
\return
Shape identifier. Zero on failure.
*/
PXD_EXPORT_FUNCTION PxdShapeQuery pxShapeQueryCreate(PxdShapeQueryType type, PxdShape shape0, PxdShape shape1);

/*!
Destroy shape query
\param query
Shape query identifier
*/
PXD_EXPORT_FUNCTION void PxdShapeQueryDestroy(PxdShapeQuery query);

/*!
Return the type for a specific shape query
\param query
Shape query identifier
\return
Shape query type identifier. Zero if shape query identifier is invalid.
*/
PXD_EXPORT_FUNCTION PxdShapeQueryType PxdShapeQueryGetType(PxdShapeQuery query);

/*!
Create shape query
\param query
Shape query identifier.
\param shape0
First shape this query uses is filled in here (1 PxdShape). NULL if query is invalid.
\param shape1
Second shape this query uses is filled in here (1 PxdShape). NULL if query is invalid.
*/
PXD_EXPORT_FUNCTION void PxdShapeQueryGetShapes(PxdShapeQuery query, PxdShape* shape0, PxdShape* shape1);



/*!
Activate a shape query
\param shape quert
Shape query identifier
*/
PXD_EXPORT_FUNCTION void PxdShapeQueryActivate(PxdShapeQuery shapeQuery);

/*!
Deactivate a shape query
\param shapeQuery
Shape query identifier
*/
PXD_EXPORT_FUNCTION void PxdShapeQueryDeactivate(PxdShapeQuery shapeQuery);
/*!
Deactivate a shape query
\param shapeQuery
Shape query identifier
*/
PXD_EXPORT_FUNCTION PxInt PxdShapeQueryIsActive(PxdShapeQuery shapeQuery);


/*!
\addtogroup Shape query control property set/get
@{
*/
PXD_EXPORT_FUNCTION void PxdShapeQuerySetFloat(PxdShapeQuery query, PxdShapeQueryProperty property, PxFloat value);
PXD_EXPORT_FUNCTION PxFloat PxdShapeQueryGetFloat(PxdShapeQuery query, PxdShapeQueryProperty property);

PXD_EXPORT_FUNCTION void PxdShapeQuerySetInt(PxdShapeQuery query, PxdShapeQueryProperty property, PxInt value);
PXD_EXPORT_FUNCTION PxInt PxdShapeQueryGetInt(PxdShapeQuery query, PxdShapeQueryProperty property);

PXD_EXPORT_FUNCTION void PxdShapeQuerySetProperty(PxdShapeQuery query, PxdShapeQueryProperty property, void* data, size_t size);
PXD_EXPORT_FUNCTION void PxdShapeQueryGetProperty(PxdShapeQuery query, PxdShapeQueryProperty property, void* data, size_t size);

PXD_EXPORT_FUNCTION void PxdShapeQuerySetVec(PxdShapeQuery ShapeQuery, PxdShapeQueryProperty property, PxdVector v);
PXD_EXPORT_FUNCTION PxdVector PxdShapeQueryGetVec(PxdShapeQuery ShapeQuery, PxdShapeQueryProperty property);

PXD_EXPORT_FUNCTION void PxdShapeQuerySetQuat(PxdShapeQuery ShapeQuery, PxdShapeQueryProperty property, PxdQuaternion q);
PXD_EXPORT_FUNCTION PxdQuaternion PxdShapeQueryGetQuat(PxdShapeQuery ShapeQuery, PxdShapeQueryProperty property);

PXD_EXPORT_FUNCTION void PxdShapeQuerySetTransform(PxdShapeQuery ShapeQuery, PxdShapeQueryProperty property, PxdTransform t);
PXD_EXPORT_FUNCTION PxdTransform PxdShapeQueryGetTransform(PxdShapeQuery ShapeQuery, PxdShapeQueryProperty property);


/*!
@}
*/


/************************************************************************/
/* Shape query control                                                  */
/************************************************************************/

/*!
Allocate task identifier for running the shape queries solver.
After this task has fully executed, resulting status, points,
normals, etc can be retrieved from the queries.
\param queries
An array of queries to update, pass NULL to update all queries
\param count
Number of queries in array. Ignored if queries equals NULL.
*/
PXD_EXPORT_FUNCTION PxdTask PxdShapeQueryUpdate(PxdShapeQuery* queries, PxUInt count);

#ifdef __cplusplus
}
#endif

#endif

