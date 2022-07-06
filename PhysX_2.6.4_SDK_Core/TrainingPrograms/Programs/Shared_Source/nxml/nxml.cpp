/*----------------------------------------------------------------------------*\
|
|						     Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

//
//         NXML
//
//  see nxml.h for more information on usage of
//  nxmlLoadScene()
//
// This is a layer above the physics engine/middleware classes.
// 
// 
// 

#include <stdio.h>
#include <NxPhysics.h>
#ifdef _WIN32
#include <Windows.h>  // needed for varargs - what header do i need here
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#else
#include <stdarg.h>
#endif

#include "xmlparse.h"
#include "nxml.h"
#include "nxmltypes.h"

using namespace nxml;
static Hash<String,char*> synonyms;

static NxQuat Inverse(const NxQuat &q){	NxQuat inv(q);inv.conjugate();return inv;}
static float dot(const NxVec3 &a,const NxVec3 &b){ return a.dot(b);}


Array<const char *> nxContext;
char *nxmlContext()
{
	static String s;
	s="";
	for(int i=0;i<nxContext.count;i++){
		if(i) s+="::";
		s+= nxContext[i];
	}
	return s.buf;
}

void NxOut(const char *format,...) 
{
	va_list vl;
	va_start(vl, format);
	int  n;
	char *tmp = NULL;
    int tmpsize = 128;
	while( (n = vsnprintf((tmp=new char[tmpsize]), tmpsize-1, format, vl))<0) {
		delete []tmp;
		tmpsize *=2;
	}
	va_end(vl);
	//Change to avoid creating the physics SDK just for this one error function
	fprintf(stderr,tmp);
	//NxFoundationSDK* fSDK = NxGetFoundationSDK()->getErrorStream()->print(tmp);
	delete []tmp;
}

// To support intra file referencing and instancing we maintian a hash of assets
Hash<String,Joint*>     joints;
Hash<String,Geometry*>  geometries;
Hash<String,Material*>  materials;
Hash<String,RigidBody*> rigidbodies;
Hash<String,Shape*>     shapes;
Hash<String,NxMat34*>	transforms;

//Advance declaration
int colladatrans(XMLElement *e,NxVec3 &position, NxQuat &orientation);

class XMap
{
public:
	Hash<String,XMLElement *> map;
	XMLElement *env;
	XMLElement *root;
	void MakeMap(XMLElement *e);
	XMap(XMLElement *e);
	~XMap();
};
static XMap *colladamap;
XMap::XMap(XMLElement *r):env(NULL),root(NULL)
{
	assert(!colladamap);
	colladamap=this;
	root = r;
	int i;
	assert(r);
	Array<XMLElement *> a;
	Array<NxVec3*> sceneOffsets;//Current global offset, used for scene graph
	Array<NxQuat*> sceneOrientations;//Current global orientation, used for scene graph
	a.Add(r);
	while(a.count)
	{
		XMLElement *e = a.Pop();
		e->tag = ToLower(e->tag);
		if(e->tag=="param" && e->attributes.Exists("name")) e->tag = ToLower(e->attributes["name"]);
		if(synonyms.Exists(e->tag)) e->tag = synonyms[e->tag];
		if(e->tag=="physicsenvironment" && !e->attributes.Exists("url"))
		{
			colladamap->env = e;
		}
		if(e->attributes.Exists("id"))
		{
			colladamap->map[e->attributes["id"]]=e;
		}
		if (e->tag=="__fake_end_node") 
		{
			//Finished processing a node
			//Store the current transform and pop it off the stacks
			NxMat34* transform = new NxMat34();
			NxQuat* orientation = sceneOrientations.Pop();
			transform->M.fromQuat(*orientation);
			delete orientation;
			NxVec3* position = sceneOffsets.Pop();
			transform->t = *(position);
			delete position;
			if (e->body != "") transforms[e->body] = transform;
			delete e;
			continue;
		}
		if(e->tag=="node") {
			//Begin a new transform
			if (sceneOffsets.count > 0) {
				NxVec3* offset = new NxVec3(*(sceneOffsets[sceneOffsets.count -1]));
				sceneOffsets.Add(offset);
				NxQuat* orient = new NxQuat(*(sceneOrientations[sceneOrientations.count -1]));
				sceneOrientations.Add(orient);
			} else {
				sceneOffsets.Add(new NxVec3(0,0,0));
				sceneOrientations.Add(new NxQuat(NxVec3(0,0,0),1));
			}
			//Stick a placeholder node on the stack so we can detect when we have processed
			// all the decendants of this node
			XMLElement* fakeEndNode = new XMLElement("__fake_end_node");
			fakeEndNode->body = e->attributes.Exists("id") ? e->attributes["id"] : "";
			a.Add(fakeEndNode);
		}
		//Update the transform
		if (sceneOffsets.count) {
			colladatrans(e, *(sceneOffsets[sceneOffsets.count -1]), *(sceneOrientations[sceneOrientations.count -1]));
		}
		int placement=0;
		int count = a.count;
		for(i=0;i<e->children.count;i++) 
		{
			if(e->children[i]->tag=="attachment") e->children[i]->tag=(placement++)?"childplacement":"parentplacement";
			//Use insert to maintain order so transforms are build correctly
			a.Insert(e->children[i], count);
		}
	}
}

XMap::~XMap()
{
	colladamap=NULL;
	//XML Elements are deleted separately, so just mark the hash tables as empty
	for (int i=0;i<joints.slots_count;++i) {
		joints.slots[i].used = 0;
		joints.keys_count = 0;
	}
	joints.Rehash(0);
	for (int i=0;i<geometries.slots_count;++i) {
		geometries.slots[i].used = 0;
		geometries.keys_count = 0;
	}
	geometries.Rehash(0);
	for (int i=0;i<materials.slots_count;++i) {
		materials.slots[i].used = 0;
		materials.keys_count = 0;
	}
	materials.Rehash(0);
	for (int i=0;i<rigidbodies.slots_count;++i) {
		rigidbodies.slots[i].used = 0;
		rigidbodies.keys_count = 0;
	}
	rigidbodies.Rehash(0);
	for (int i=0;i<shapes.slots_count;++i) {
		shapes.slots[i].used = 0;
		shapes.keys_count = 0;
	}
	shapes.Rehash(0);
	//Delete any accumulated matrix transforms
	for (int i=0;i<transforms.slots_count;++i) {
		if (transforms.slots[i].used) {
			if (transforms.slots[i].value) {
				delete transforms.slots[i].value;
				transforms.slots[i].value = 0;
			}
			transforms.slots[i].used = 0;
		}
		transforms.keys_count = 0;
	}
	transforms.Rehash(0);

	delete root;
	root = NULL;
	for (int i=0;i<map.slots_count;++i) {
		map.slots[i].used = 0;
		map.keys_count = 0;
	}
	map.Rehash(0);
}

void nxml::releaseMemory()
{
	delete colladamap;
	colladamap = 0;
}

// The following function nxml::Publish() is for the parser, it describes how to
// convert ascii in an xml tree of nodes into the nxml C++ classes.

ClassDesc SphereDef("Sphere");
ClassDesc BoxDef("Box");
ClassDesc PlaneDef("Plane");
ClassDesc CapsuleDef("Capsule");
ClassDesc ConvexDef("Convex");
ClassDesc TriMeshDef("TriMesh");
ClassDesc MaterialDef("Material");
ClassDesc ShapeDef("Shape");
ClassDesc RigidBodyDef("RigidBody");
ClassDesc PlacementDef("Placement");
ClassDesc JointDef("Joint");
ClassDesc SceneDescDef("SceneDesc");

inline char* ctype(const NxVec3&){return "fff";}
inline char* ctype(const NxQuat&){return "ffff";}
inline char* ctype(const NxMat33&){return "fffffffff";}



static char * syns[][2] = 
{	// synonym                standard term
	{"physics_material"		, "physicsmaterial"		},
	{"static_friction"		, "staticfriction"		},
	{"dynamic_friction"		, "dynamicfriction"		},
	{"constraint"			, "joint"				},
	{"physics_environment"	, "physicsenvironment"	},
	{"elasticity"			, "restitution"			},
	{"offset"   			, "position"			},
	{"rigid_constraint"		, "joint"				},
	{"bodycollide"			, "interpenetrate"		},
	{"limitmin"				, "rotlimitmin"			},
	{"limitmax"				, "rotlimitmax"			},
	{"rot_limit_min"		, "rotlimitmin"			},
	{"rot_limit_max"		, "rotlimitmax"			},
	{"rigid_body"			, "rigidbody"			},
	{"convex_mesh"			, "convexmesh"			},
	{"initial_linear_velocity", "linearvelocity"	},
	{"linear_velocity"		, "linearvelocity"		},
};

namespace nxml
{
	void Publish()
	{
		static int initialized=0;  
		if(initialized) return;  // only need to call this routine once
		initialized=1;

		for(int i=0;i<sizeof(syns)/sizeof(char*[2]);i++) synonyms[syns[i][0]]=syns[i][1];

		ADDMEMBER(Box   ,size);
		ADDMEMBER(Sphere,radius);
		ADDMEMBER(Capsule,radius);
		ADDMEMBER(Capsule,p0);
		ADDMEMBER(Capsule,p1);
		ADDMEMBER(Plane ,normal);
		ADDMEMBER(Plane ,d);
		ADDMARRAY(Convex,vertices,"fff");
		ADDMARRAY(Convex,indices,"d");
		ADDMARRAY(Convex,faces,"d");
		ADDMARRAY(TriMesh,vertices,"fff");
		ADDMARRAY(TriMesh,triangles,"ddd");

		ADDMEMBER(SceneDesc,eye);
		ADDMEMBER(SceneDesc,lookat);
		ADDMEMBER(SceneDesc,up);
		ADDMEMBER(SceneDesc,gravity);

		//ADDMEMBER(NxSceneDesc,collisionDetection);  // bool != int

		//Material data now defined in multi-param form which is not consecutive
		ADDMEMBER(Material,restitution);
		ADDMEMBER(Material,staticFriction);
		ADDMEMBER(Material,dynamicFriction);
		ADDMEMBER(Material,index);

		ADDMEMBER(Shape,position);
		ADDMEMBER(Shape,orientation);
		ADDMEMBER(Shape,mass);
		ADDMEMBER(Shape,density);

		ADDMEMBER(RigidBody,mass);
		ADDMEMBER(RigidBody,dynamic);
		ADDMEMBER(RigidBody,position);
		ADDMEMBER(RigidBody,orientation);
		ADDMEMBER(RigidBody,inertia);
		ADDMEMBER(RigidBody,com);
		ADDMEMBER(RigidBody,density);

		ADDMEMBER(Placement,x);
		ADDMEMBER(Placement,y);
		ADDMEMBER(Placement,z);
		ADDMEMBER(Placement,position);
		ADDMEMBER(Placement,orientation);

		ADDMEMBER(Joint,rotlimitmin);
		ADDMEMBER(Joint,rotlimitmax);
		ADDMEMBER(Joint,interpenetrate);
		//ADDMEMBER(Joint,spring);
	}
}; // namespace



	

int colladatrans(XMLElement *e,NxVec3 &position, NxQuat &orientation)
{
	if(!strcasecmp(e->tag,"translate"))
	{
		NxVec3 p(0,0,0);
		sscanf(e->body,"%f%f%f",&p.x,&p.y,&p.z);
		position += p; // or q*p ??
		return 1;
	}
	if(!strcasecmp(e->tag,"rotate"))
	{
		NxVec3 p(1,0,0);
		float  a(0);
		sscanf(e->body,"%f%f%f%f",&p.x,&p.y,&p.z,&a);
		if(p==NxVec3(0,0,0)) return 1;
		NxQuat q(a,p);//DEG2RAD*a);
		orientation = orientation *q;  // order????
		return 1;
	}
	return 0;
}

ClassDesc *GeoDef(Geometry *g)
{
	if(dynamic_cast<nxml::Box*>    (g)) return &BoxDef;     
	if(dynamic_cast<nxml::Sphere*> (g)) return &SphereDef;  
	if(dynamic_cast<nxml::Plane*>  (g)) return &PlaneDef;   
	if(dynamic_cast<nxml::Capsule*>(g)) return &CapsuleDef; 
	if(dynamic_cast<nxml::Convex*> (g)) return &ConvexDef;  
	if(dynamic_cast<nxml::TriMesh*>(g)) return &TriMeshDef; 
	assert(0);
	NxOut("ERROR: unknown type of geometry\n");
	return NULL;
}
Geometry *GeoFactory(const char *gtype)
{
	if(!strcasecmp(gtype,"box"    )) return new nxml::Box();
	if(!strcasecmp(gtype,"sphere" )) return new nxml::Sphere();
	if(!strcasecmp(gtype,"plane"  )) return new nxml::Plane();
	if(!strcasecmp(gtype,"capsule")) return new nxml::Capsule();
	if(!strcasecmp(gtype,"convex" )) return new nxml::Convex();
	if(!strcasecmp(gtype,"trimesh")) return new nxml::TriMesh();
	NxOut("ERROR: unknown geometry type <%s> \n",gtype);
	return NULL;
}

bool LoadParam(XMLElement *e, void* out, String& paramType)
{
	if (!e) return false;
	String type;
	String value;
	if (!e->attributes.Exists("type")) {
		//Custom parameter
		assert(e->children.count > 0);
		XMLElement* c;
		c = e->Child(paramType);
		if (!c) 
		{
			fprintf(stderr,"WARNING: requsted parameter format %s not found for %s. \n", paramType, e->tag );
			return false; //Requested parameter representation  not available
		}
		type = c->tag;
		value = c->body;
	} else {
		type = e->attributes["type"];
		value = e->body;
	}

	if (type == "float") {
		*((NxReal*)out) = value.Asfloat();
	} else if (type == "float3") {
		NxVec3* vec = (NxVec3*)out;
		sscanf(value, "%f %f %f", &(vec->x), &(vec->y), &(vec->z));
	} else {
		assert(!"Unsupported parameter type in LoadParam");
		return false;
	}

	return true;
}

Geometry *ColladaLoadMesh(XMLElement *e)
{
	Convex* convex = new nxml::Convex();
	XMLElement *b = colladamap->map[SkipChars(e->Child("vertices")->Child("input")->attributes["source"],"#")]->Child("float_array");
	convex->vertices.count = convex->vertices.array_size = b->attributes["count"].Asint()/3;
	convex->vertices.element = (NxVec3*) AscToBin(b->body,convex->vertices.count,"fff");
	assert(convex->vertices.element);
	return convex;
}

Geometry *LoadGeometry(XMLElement *e)
{
	assert(!strcasecmp(e->tag,"geometry"));
	if(e->attributes.Exists("url"))
	{
		String name(e->attributes["url"]);
		if(name[0] != '#') NxOut("Warn: intra file geometry references must begin with '#' in %s\n",(const char*)name);
		else name.DropFirstChars("#");
		Geometry *g=geometries(name);
		if(!g && colladamap && colladamap->map.Exists(name)) g=LoadGeometry(colladamap->map[name]);
		if(!g) NxOut("ERROR: unable to find geometry %s\n",(const char*)name);
		return g; // find 
	}
	if(e->children.count!=1) 
	{ 
		NxOut("ERROR: <geometry> expects exactly one child \n");
		return NULL;
	}
	XMLElement *c = e->children[0];
	Geometry *g = NULL;
	if(colladamap && (!strcasecmp(c->tag,"convexmesh") || !strcasecmp(c->tag,"mesh")))
	{
		g = ColladaLoadMesh(c);
	}
	else 
	{
		g = GeoFactory(c->tag);
		LoadClass(c,g,classdesc[c->tag]);
	}
	assert(g);
	if(e->attributes.Exists("id")) 
	{
		g->id = e->attributes["id"];
	}
	else
	{
		static int num=0; //make up a unique name
		g->id = String("__geometry") + String(num);
		num++;
	}
	geometries[g->id] = g;
	return g;
}
Material *LoadMaterial(XMLElement *e)
{
	Material *m=NULL;
	if(e->attributes.Exists("url")) 
	{
		String mname(SkipChars(e->attributes["url"],"#"));
		m=materials[mname];
		if(!m && colladamap->map.Exists(mname)) m= LoadMaterial(colladamap->map[mname]);
		if(!m) NxOut("ERROR: unable to find material %s\n",(char*)e->attributes["url"]);
		return m; // find 
	}
	//Properties are now under a technique node, but keep looking for them under the current node if technique not found
	XMLElement* technique = e->Child("technique");
	m = new Material();
	if (technique == NULL) {
		LoadClass(e,m,&MaterialDef);
	} else {
		for (int i=0;i<technique->children.count;++i) {
			XMLElement* c = technique->children[i];
			if (!strcasecmp(c->tag, "staticfriction")) 
			{
				NxReal fric[2];
				String float2Str("float2");
				if (LoadParam(c, fric, float2Str)) {
					//FIXME: semantic might have defined different ordering...
					m->staticFriction = fric[0];
					m->staticFrictionV = fric[1];
				} else {
					String floatStr("float");
					LoadParam(c, &(m->staticFriction), floatStr);
				}
				continue;
			}
			if (!strcasecmp(c->tag, "dynamicfriction")) 
			{
				NxReal fric[2];
				String float2Str("float2");
				if (LoadParam(c, fric, float2Str)) {
					//FIXME: semantic might have defined different ordering...
					m->dynamicFriction = fric[0];
					m->dynamicFrictionV = fric[1];
				} else {
					String floatStr("float");
					LoadParam(c, &(m->dynamicFriction), floatStr);
				}
				continue;
			}
			if (!strcasecmp(c->tag, "restitution")) {String floatStr("float"); LoadParam(c, &(m->restitution), floatStr); continue;}
		}
	}
	static int mid=0; // unique id if none supplied
	m->id = (e->attributes.Exists("id")) ? e->attributes["id"] : String("__material")+String(mid++) ;
	return m;
}


Shape *LoadShape(XMLElement *e)
{
	static int sid=0; // unique id if none supplied
	Shape *shape = new Shape((e->attributes.Exists("id")) ? e->attributes["id"] : String("shape_")+String(sid++));
	shapes[shape->id] = shape;
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if(LoadMember(c,shape,&ShapeDef)) continue;
		if(!strcasecmp(c->tag,"geometry")) {assert(!shape->geometry);shape->geometry= LoadGeometry(c); continue;}
		if(c->tag=="physicsmaterial") {assert(!shape->material);shape->material= LoadMaterial(c); continue;}
		if(colladatrans(c,shape->position,shape->orientation)) continue;
		if(!strcasecmp(c->tag,"shape"   )) {shape->shape.Add(LoadShape(c)); continue;}
		NxOut("WARNING: within <%s> node found unexpected xml node <%s>\n",e->tag,c->tag);
	}
	assert(shape->geometry);
	return shape;
}

RigidBody *LoadRigidBody(XMLElement *e)
{
	assert(e);
	RigidBody *rb = new RigidBody();
	static int rbid=0; // unique id if none supplied
	rb->id = (e->attributes.Exists("id")) ? e->attributes["id"] : String("__rigidbody")+String(rbid++) ;
	nxContext.Add(rb->id);
	rigidbodies[rb->id]=rb;
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if (LoadMember(c,rb,&RigidBodyDef)) continue;
		if(!strcasecmp(c->tag,"geometry")) {Shape *s=new Shape("");s->geometry=LoadGeometry(c);rb->shape.Add(s); continue;}
		if(!strcasecmp(c->tag,"shape"   )) {rb->shape.Add(LoadShape(c)); continue;}
		if(!strcasecmp(c->tag,"linearvelocity")) {String float3Str("float3"); LoadParam(c,&(rb->linearVelocity), float3Str); continue;}
		if(!strcasecmp(c->tag,"angularvelocity"))  {String float3Str("float3"); LoadParam(c,&(rb->angularVelocity), float3Str); continue;}
		if(colladatrans(c,rb->position,rb->orientation)) continue;
		NxOut("WARNING: within <%s> node found unexpected xml node <%s>\n",e->tag,c->tag);
	}
	nxContext.Pop();
	return rb;
}

void LoadPlacement(XMLElement *e,Placement &placement)
{
	assert(!placement.specified);
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if(colladatrans(c,(NxVec3&)placement.position,placement.orientation)) continue;
	}
	LoadClass(e,&placement ,&PlacementDef);
	if(placement.orientation.w!=1.0f)
	{
		NxMat33 m(placement.orientation);
		placement.x = m.getRow(0);
		placement.y = m.getRow(1);
		placement.z = m.getRow(2);
	}
	placement.frame = e->attributes("frame");
	placement.specified = 1;
}

Joint *LoadJoint(XMLElement *e)
{
	assert(e); 
	static int jid=0; // unique id if none supplied
	Joint *joint = new Joint((e->attributes.Exists("id")) ? e->attributes["id"] : String("__joint")+String(jid++) ); 
	joints[joint->id] = joint;
	nxContext.Add(joint->id);
	String childbody,parentbody;
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if (LoadMember(c,joint,&JointDef)) continue;
		if(c->tag=="childplacement" ) {LoadPlacement(c,joint->childplacement ); childbody =c->attributes["body"]; continue;}
		if(c->tag=="parentplacement") {LoadPlacement(c,joint->parentplacement); parentbody=c->attributes["body"]; continue;}
		if(c->tag=="spring") continue;  // just dont know what to do with this yet.
		if(c->tag=="enabled") continue;  // just dont know what to do with this yet.
		NxOut("WARNING: within <%s> node found unexpected xml node <%s>\n",e->tag,c->tag);
	}
	if(e->attributes.Exists("child" )) childbody =e->attributes["child" ]; // will be deprecated
	if(e->attributes.Exists("parent")) parentbody=e->attributes["parent"]; // will be deprecated
	if(childbody!="")
	{
		joint->child = rigidbodies[SkipChars(childbody,"#")];
		if(!joint->child ) NxOut("WARNING: cant find child  rigidbody %s for joint %s \n",(char*)childbody, (char*)joint->id);
	}
	if(parentbody!="")
	{
		joint->parent = rigidbodies[SkipChars(parentbody,"#")];
		if(!joint->parent) NxOut("WARNING: cant find parent rigidbody %s for joint %s \n",(char*)parentbody,(char*)joint->id);
	}
	nxContext.Pop();
	return joint;
}


static int mid=0; // unique id if none supplied

Model *LoadModel(XMLElement *e)
{
	assert(e); 
	Model *model = new Model();

	model->id = (e->attributes.Exists("id")) ? e->attributes["id"] : String("__model")+String(mid++) ;
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if(c->tag=="rigidbody") {model->rigidbody.Add(LoadRigidBody(c));      continue;}
		if(c->tag=="joint")     {model->joint.Add(    LoadJoint(c)    );      continue;}
		else NxOut("WARNING: within <%s> node found unexpected xml node <%s>\n",e->tag,c->tag);
	}
	return model;
}

RigidBody* LoadRigidInstance(XMLElement *e)
{
	assert(e);
	RigidBody* rb;
	static int rbids = 0;
	if(e->attributes.Exists("url")) {
		String name(e->attributes["url"]);
		if(name[0] != '#') NxOut("Warn: intra file rigidbody references must begin with '#' in %s\n",(const char*)name);
		else name.DropFirstChars("#");
		RigidBody *g=rigidbodies(name);
		if(!g && colladamap && colladamap->map.Exists(name)) g=LoadRigidBody(colladamap->map[name]);
		if(!g) NxOut("ERROR: unable to find geometry %s\n",(const char*)name);
		rb = new RigidBody(*g);// Make a copy of the template we found
		String temp = rb->id;
		rb->id.sprintf("%s%s%d", temp, "_instance", rbids++);
	} else {
		rb = LoadRigidBody(e);  //No template, must be inline instantiation
	}

	if (e->attributes.Exists("target")) {
		String name(e->attributes["target"]);
		if(name[0] != '#') NxOut("Warn: intra file target references must begin with '#' in %s\n",(const char*)name);
		else name.DropFirstChars("#");
		if (transforms.Exists(name)) {
			//Extract pose from scene node
			NxMat34* pose = transforms[name];
			rb->position = pose->t;  
			pose->M.toQuat(rb->orientation);
			rb->globalPose = *pose;
		}
	}

	if (e->attributes.Exists("id")) {
		rigidbodies[e->attributes["id"]] = rb;
	}

	for(int i=0;i<e->children.count;i++)
	{
		XMLElement* c = e->children[i];

		//SPH: These might be replaced with PARAM representation at some point
		if(strcasecmp(c->tag,"linearvelocity") == 0) {String float3Str("float3"); LoadParam(c, (void*)&(rb->linearVelocity), float3Str); continue;}
		if(strcasecmp(c->tag,"angularvelocity") == 0) {String float3Str("float3"); LoadParam(c, (void*)&(rb->angularVelocity), float3Str); continue;}
		//SPH: What else can instances have?
	}

	return rb;
}

void LoadAttachment(XMLElement *e, Joint* j, int isChild)
{
	RigidBody* rb = NULL;
	if (e->attributes.Exists("body")) {
		String name(e->attributes["body"]);
		if(name[0] != '#') NxOut("Warn: intra file attachment body references must begin with '#' in %s\n",(const char*)name);
		else name.DropFirstChars("#");
		rb = rigidbodies(name);
	}

	NxVec3 pos(0,0,0);
	NxQuat orient(NxVec3(0,0,0),1);
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement* c = e->children[i];
		colladatrans(c,pos, orient);
	}

	Placement& place = isChild ? j->childplacement : j->parentplacement;
	if (isChild) {
		j->child = rb;
		place.frame = "child";
	} else {
		j->parent = rb;
		place.frame = "parent";
	}

	place.orientation = orient;
	place.position = pos;
	place.specified = 1;
	NxMat33 m(place.orientation);
	place.x = m.getRow(0);
	place.y = m.getRow(1);
	place.z = m.getRow(2);

}

Joint* LoadJointInstance(XMLElement *e)
{
	static int jids = 0;
	assert(e);
	Joint* j;
	if(e->attributes.Exists("url")) {
		String name(e->attributes["url"]);
		if(name[0] != '#') NxOut("Warn: intra file rigidbody references must begin with '#' in %s\n",(const char*)name);
		else name.DropFirstChars("#");
		Joint *g=joints(name);
		if(!g && colladamap && colladamap->map.Exists(name)) g=LoadJoint(colladamap->map[name]);
		if(!g) NxOut("ERROR: unable to find joint %s\n",(const char*)name);
		j = new Joint(*g); // make a copy of the template we found
		String temp = j->id;
		j->id.sprintf("%s%s%d", temp, "_instance", jids++);
	} else {
		j = LoadJoint(e); // no template, must be inline instantiation
	}

	if (e->attributes.Exists("id")) {
		joints[e->attributes["id"]] = j;
	}

	for(int i=0;i<e->children.count;i++)
	{
		XMLElement* c = e->children[i];
		if(c->tag=="parentplacement") {LoadAttachment(c,j, false); continue;}
		if(c->tag=="childplacement") {LoadAttachment(c,j, true); continue;}
	}

	return j;
}

void LoadEnvironmentInstance(XMLElement *e, Model* model)
{
	assert(e);
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement* c = e->children[i];
		if(c->tag=="rigidbody") { model->rigidbody.Add(LoadRigidInstance(c)); continue;}
		if(c->tag=="joint") { model->joint.Add(LoadJointInstance(c)); continue;}
		//SPH: What else can instances have?
	}
}

void LoadEnvironmentNode(XMLElement *e, Model* model)
{
	assert(e);
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement* c = e->children[i];
		if(c->tag=="instance") { LoadEnvironmentInstance(c, model); continue;}
		//SPH: What else can nodes have?
	}
}

//Load environment specifying mapping between renderable and physical geometry
Model* LoadEnvironment(XMLElement *e, nxml::Library* library)
{
	assert(e);
	Model *model = new Model();
	model->id = (e->attributes.Exists("id")) ? e->attributes["id"] : String("__model")+String(mid++) ;
	nxContext.Add(model->id);
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if(c->tag=="node"   ) {LoadEnvironmentNode(c,model); continue;}
		if(c->tag=="gravity") {String float3Str("float3"); LoadParam(c,&(library->scenedesc.gravity), float3Str);continue;}
		if(c->tag=="timestep"){continue;} //FIXME: Add timestep to scene description or model?
		//SPH: Where is proper place to store parameters?
	}
	nxContext.Pop();
	return model;
}

String LoadExtraTechnique(XMLElement *e)
{
	if (e->attributes.Exists("profile"))
	{
		if (strncmp(e->attributes["profile"],"NXFLUID",7) == 0) {
			return e->body;
		} else NxOut("WARNING: technique missing profile<%s>\n",e->tag);
	}
	else NxOut("WARNING: technique missing profile<%s>\n",e->tag);

	return "";
}

String LoadExtra(XMLElement *e)
{
	String result;
	String str;
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if(c->tag=="technique") {str = result; result.sprintf("%s\n\n%s", str, LoadExtraTechnique(c)); continue;}

	}

	return result;
}

Library *LoadLibrary(XMLElement *e,Library *library)
{
	assert(e);
	if(!library) library= new Library();
	assert(library);
	Model *dm=new Model("__default");
	if(strcasecmp("library",e->tag) || !e->attributes.Exists("type") || strcasecmp(e->attributes["type"],"physics") ) 
	{
		NxOut("WARNING: expecting root node <library type=\"physics\">\n");
	}
	for(int i=0;i<e->children.count;i++)
	{
		XMLElement *c=e->children[i];
		if(c->tag=="physicsmodel"   ) {library->model.Add(    LoadModel(c)    );      continue;}
		if(c->tag=="geometry"       ) {library->geometry.Add( LoadGeometry(c) );      continue;}
		if(c->tag=="physicsmaterial") {library->material.Add( LoadMaterial(c) );      continue;}
		if(c->tag=="scenedesc"      ) {LoadClass(c,&library->scenedesc,&SceneDescDef);continue;}
		if(c->tag=="rigidbody"      ) {dm->rigidbody.Add(LoadRigidBody(c));           continue;}
		if(c->tag=="joint"          ) {dm->joint.Add(    LoadJoint(c)    );           continue;}
		if(c->tag=="physicsenvironment"          ) {library->model.Add(LoadEnvironment(c, library)); continue;}
		if(c->tag=="extra") {library->extradata.extras.Add(LoadExtra(c)); continue;}
		else NxOut("WARNING: within <%s> node found unexpected xml node <%s>\n",e->tag,c->tag);
	}
	if(dm->rigidbody.count || dm->joint.count) library->model.Add(dm); else delete dm;
	return library;
}

static int isphysicslibrary(XMLElement *e)
{
	return (!strcasecmp(e->tag,"library") && e->attributes.Exists("type") && !strcasecmp(e->attributes["type"],"physics"));
}

//--------------- nxml cooking section -------------
//  the following routines do minor modifications to the data to make it better fit the PhysX API.

static void transformplacement(Placement &placement,const NxQuat &q,const NxVec3& p)
{
	placement.position = q.rot(placement.position) + p;
	placement.x = q.rot( placement.x);
	placement.y = q.rot( placement.y);
	placement.z = q.rot( placement.z);
}


static void computeotherplacment(nxml::Joint *j)
{
	if(!j->childplacement.specified && !j->parentplacement.specified) 
	{
		fprintf(stderr,"WARNING: found joint %s without placement specified\n",(char*)j->id);
		return;  // probably a template that will be instantiated later
	}
	if(j->childplacement.specified==0)
	{
		j->childplacement = j->parentplacement;
		if(j->parent) transformplacement(j->childplacement,j->parent->orientation,j->parent->position);
		if(j->child)
		{
			transformplacement(j->childplacement,NxQuat(NxVec3(0,0,0),1),-j->child->position);
			transformplacement(j->childplacement,!(j->child->orientation),NxVec3(0,0,0));
		}
		j->childplacement.frame = "child";
	}
	if(j->parentplacement.specified==0)
	{
		j->parentplacement = j->childplacement;
		if(j->child) 
		{
			transformplacement(j->parentplacement,j->child->orientation,j->child->position);
		}
		if(j->parent)
		{
			transformplacement(j->parentplacement,NxQuat(NxVec3(0,0,0),1),-j->parent->position);
			transformplacement(j->parentplacement,!(j->parent->orientation),NxVec3(0,0,0));
		}
		j->parentplacement.frame = "parent";
	}
	if(j->parent && !j->parent->dynamic)
	{
		transformplacement(j->parentplacement,j->parent->orientation,j->parent->position);
		j->parent=NULL;
	}
	if(j->child && !j->child->dynamic)
	{
		transformplacement(j->childplacement,j->child->orientation,j->child->position);
		j->child=NULL;
	}
}


static NxQuat RotationArc(NxVec3 v0,NxVec3 v1)
{
	NxQuat q;
	v0.normalize(); 
	v1.normalize(); 
	NxVec3 c ; c.cross(v0,v1);
	float   d = dot(v0,v1);
	if(d<=-1.0f) { return NxQuat(NxVec3(1,0,0),0);} // 180 about x axis
	float   s = sqrtf((1+d)*2);
	q.x = c.x / s;
	q.y = c.y / s;
	q.z = c.z / s;
	q.w = s /2.0f;
	return q;
}

static void reorientcapsules(nxml::RigidBody *rb)
{
	int i;
	for(i=0;i<rb->shape.count;i++)
	{
		nxml::Capsule* capsule = dynamic_cast<nxml::Capsule*>(rb->shape[i]->geometry);
		if(!capsule) continue;
		NxVec3 p = (capsule->p0+capsule->p1)/2;
		rb->shape[i]->position += rb->shape[i]->orientation.rot(p);
		capsule->p0 -= p;
		capsule->p1 -= p;
		NxVec3 dir = capsule->p1 - capsule->p0;
		if(dir == NxVec3(0,0,0)) continue; // its just a sphere.
		NxQuat q = RotationArc(dir,NxVec3(0,1,0));
		capsule->p0 = q.rot(capsule->p0);
		capsule->p1 = q.rot(capsule->p1);
		rb->shape[i]->orientation = rb->shape[i]->orientation * Inverse(q);
		capsule->height = capsule->p1.y-capsule->p0.y; // assumeing i already aligned it to y axis
	}
}



static void promoteshapes(RigidBody *rp)
{
	int i;
	for(i=0;i<rp->shape.count;i++)
	{
		Shape *s = rp->shape[i];
		while(s->shape.count)
		{
			Shape *c = s->shape.Pop();
			c->position = s->position + s->orientation.rot(c->position);
			c->orientation = s->orientation * c->orientation;
			rp->shape.Add(c);
		}
	}
}



static void gathermaterials(Library *library,RigidBody *rb)
{
	int i;
	for(i=0;i<rb->shape.count;i++)
	{
		if(rb->shape[i]->material) library->material.AddUnique(rb->shape[i]->material);
	}
}

void nxmlcook(Library *library)
{
	int i,j;
	for(i=0;i< library->model.count;i++)
	{
		nxml::Model *m = library->model[i];
		for(j=0;j<m->rigidbody.count;j++)
		{
			promoteshapes(m->rigidbody[j]);
		}
		for(j=0;j<m->joint.count;j++)
		{
			computeotherplacment(m->joint[j]);
		}
		for(j=0;j<m->rigidbody.count;j++)
		{
			reorientcapsules(m->rigidbody[j]);
		}
		for(j=0;j<m->rigidbody.count;j++)
		{
			gathermaterials(library,m->rigidbody[j]);
		}
	}
}




Library* nxmlLoadScene(const char *filename)
{
	nxContext.Add(filename);
	XMLElement * xnode = XMLParseFile(filename);  // gets a tree of xml nodes
	assert(xnode);
	nxml::Publish(); 
	XMap thecolladamap(xnode);
	        
	XMLElement *xlibrary = xnode;
	if(xnode->tag=="collada")
	{
		int i;
		for(i=0;i<xnode->children.count;i++)
		{
			if(isphysicslibrary(xnode->children[i])) 
			{
				xlibrary = xnode->children[i];
			}
		}
	}	
	Library *library = LoadLibrary(xlibrary,NULL); 
	nxmlcook(library);
	nxContext.Pop();
	return library;
}

/*
// this function was put here for the sole purpose of diagnosing a problem with convex hulls that JG was sending us.
// this function loads the vertex data in the file format they gave us and generated .pml files that we could load.
char *txt(char *filename)
{
	FILE *fp=fopen(filename,"r");
	assert(fp);
	char buf[1024];
	fgets(buf,1023,fp);
	fgets(buf,1023,fp);
	fgets(buf,1023,fp);
	int n=0;
	while(fgets(buf,1023,fp))
	{
		float a,b,c;
		if(3== sscanf(buf,"%f%f%f",&a,&b,&c)) n++;
	}
	fp=fopen(filename,"r");
	assert(fp);
	fgets(buf,1023,fp);
	fgets(buf,1023,fp);
	fgets(buf,1023,fp);
	int len=strlen(filename);
	strcpy(filename+len-3,"pml");
	FILE *fout=fopen(filename,"w");
	assert(fout);
	fprintf(fout,"<?xml?> \n<Library type=\"physics\">\n <rigidbody> <shape> <geometry> <convex> \n");
	fprintf(fout,"	<vertices count=\"%d\">\n",n);
	while(fgets(buf,1023,fp))
	{
		fputs(buf,fout);
	}
	fprintf(fout,"  </vertices> \n </convex>    </geometry>   </shape>  </rigidbody>\n</Library>\n");
	fclose(fp);
	fclose(fout);
	return "ok";
}
EXPORTFUNC(txt);
*/

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright © 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
