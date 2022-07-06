PsReset
PsNameSpace NxCoreDump

PsConvexBegin Convex1 
PsVert 0.5 0.5 0.75  -0.5 0.5 0.75  -1 -0.5 1.5  1 -0.5 1.5  
PsVert -0.5 0.5 -0.75  -1 -0.5 -1.5  0.5 0.5 -0.75  1 -0.5 -1.5  
PsFace 3 0 1 2 3 0 2 3 3 2 4 5 3 2 1 4 
PsFace 3 0 6 4 3 0 4 1 3 4 7 5 3 4 6 7 
PsFace 3 0 7 6 3 0 3 7 3 3 5 7 3 3 2 5 
PsConvexEnd

PsNameSpace NxCoreDump
PsPhysicsModelBegin Scene1  mesh()
PsPlane plane(0,1,0,0) localposition(0,0,0) localorientation(0,0,0,1) material(defaultmaterial)  name(Actor1) position(0,0,0) orientation(0,0,0,1) density(1) inertia(0,0,0) com(0,0,0) comrot(0,0,0,1) lineardamping(0) angulardamping(0) wakeupcounter(0.400000006) velocity(0,0,0) angularvelocity(0,0,0) maxangularvelocity(7) awake(false) collision(false) kinematic(false) static(true) locked(false) 
PsBox sides(1,2,1) localposition(0,1,0) localorientation(0,0,0,1) material(defaultmaterial)  name(Actor2) position(5,0,0) orientation(0,0,0,1) mass(40) inertia(16.666669846,6.666666985,16.666669846) com(0,1,0) comrot(0,0,0,1) lineardamping(0) angulardamping(0.050000001) wakeupcounter(0.399999976) velocity(0,0,0) angularvelocity(0,0,0) maxangularvelocity(7) awake(true) collision(false) kinematic(false) static(false) locked(false) 
PsSphere radius(1) localposition(0,1,0) localorientation(0,0,0,1) material(defaultmaterial)  name(Actor3) position(0,0,5) orientation(0,0,0,1) mass(41.887901306) inertia(16.755161285,16.755161285,16.755161285) com(0,1,0) comrot(0,0,0,1) lineardamping(0) angulardamping(0.050000001) wakeupcounter(0.399999976) velocity(0,0,0) angularvelocity(0,0,0) maxangularvelocity(7) awake(true) collision(false) kinematic(false) static(false) locked(false) 
PsCapsule radius(0.5) height(2) localposition(0,1.5,0) localorientation(0,0,0,1) material(defaultmaterial)  name(Actor4) position(-5,0,0) orientation(0,0,0,1) mass(23.561944962) inertia(19.144081116,2.94524312,19.144081116) com(0,1.5,0) comrot(0,0,0,1) lineardamping(0) angulardamping(0.050000001) wakeupcounter(0.399999976) velocity(0,0,0) angularvelocity(0,0,0) maxangularvelocity(7) awake(true) collision(false) kinematic(false) static(false) locked(false) 
PsConvex Convex1 localposition(0,0,0) localorientation(0,0,0,1) material(defaultmaterial)  name(Actor5) position(0,0,0) orientation(0,0,0,1) mass(35) inertia(20.035713196,25.1875,10.348213196) com(-0.000000008,0.392857194,-0) comrot(0,0,0,1) lineardamping(0) angulardamping(0.050000001) wakeupcounter(0.399999976) velocity(0,0,0) angularvelocity(0,0,0) maxangularvelocity(7) awake(true) collision(false) kinematic(false) static(false) locked(false) 
PsSetConstant NX_PENALTY_FORCE 0.800000012
PsSetConstant NX_SKIN_WIDTH 0.005
PsSetConstant NX_DEFAULT_SLEEP_LIN_VEL_SQUARED 0.022500001
PsSetConstant NX_DEFAULT_SLEEP_ANG_VEL_SQUARED 0.0196
PsSetConstant NX_BOUNCE_TRESHOLD -2
PsSetConstant NX_DYN_FRICT_SCALING 1
PsSetConstant NX_STA_FRICT_SCALING 1
PsSetConstant NX_MAX_ANGULAR_VELOCITY 7
PsSetConstant NX_CONTINUOUS_CD 0
PsSetConstant NX_VISUALIZATION_SCALE 1
PsSetConstant NX_VISUALIZE_WORLD_AXES 0
PsSetConstant NX_VISUALIZE_BODY_AXES 0
PsSetConstant NX_VISUALIZE_BODY_MASS_AXES 0
PsSetConstant NX_VISUALIZE_BODY_LIN_VELOCITY 0
PsSetConstant NX_VISUALIZE_BODY_ANG_VELOCITY 0
PsSetConstant NX_VISUALIZE_BODY_LIN_MOMENTUM 0
PsSetConstant NX_VISUALIZE_BODY_ANG_MOMENTUM 0
PsSetConstant NX_VISUALIZE_BODY_LIN_ACCEL 0
PsSetConstant NX_VISUALIZE_BODY_ANG_ACCEL 0
PsSetConstant NX_VISUALIZE_BODY_LIN_FORCE 0
PsSetConstant NX_VISUALIZE_BODY_ANG_FORCE 0
PsSetConstant NX_VISUALIZE_BODY_REDUCED 0
PsSetConstant NX_VISUALIZE_BODY_JOINT_GROUPS 0
PsSetConstant NX_VISUALIZE_BODY_CONTACT_LIST 0
PsSetConstant NX_VISUALIZE_BODY_JOINT_LIST 0
PsSetConstant NX_VISUALIZE_BODY_DAMPING 0
PsSetConstant NX_VISUALIZE_BODY_SLEEP 0
PsSetConstant NX_VISUALIZE_JOINT_LOCAL_AXES 0
PsSetConstant NX_VISUALIZE_JOINT_WORLD_AXES 0
PsSetConstant NX_VISUALIZE_JOINT_LIMITS 0
PsSetConstant NX_VISUALIZE_JOINT_ERROR 0
PsSetConstant NX_VISUALIZE_JOINT_FORCE 0
PsSetConstant NX_VISUALIZE_JOINT_REDUCED 0
PsSetConstant NX_VISUALIZE_CONTACT_POINT 0
PsSetConstant NX_VISUALIZE_CONTACT_NORMAL 0
PsSetConstant NX_VISUALIZE_CONTACT_ERROR 0
PsSetConstant NX_VISUALIZE_CONTACT_FORCE 0
PsSetConstant NX_VISUALIZE_ACTOR_AXES 1
PsSetConstant NX_VISUALIZE_COLLISION_AABBS 0
PsSetConstant NX_VISUALIZE_COLLISION_SHAPES 1
PsSetConstant NX_VISUALIZE_COLLISION_AXES 0
PsSetConstant NX_VISUALIZE_COLLISION_COMPOUNDS 0
PsSetConstant NX_VISUALIZE_COLLISION_VNORMALS 0
PsSetConstant NX_VISUALIZE_COLLISION_FNORMALS 1
PsSetConstant NX_VISUALIZE_COLLISION_EDGES 0
PsSetConstant NX_VISUALIZE_COLLISION_SPHERES 0
PsSetConstant NX_VISUALIZE_COLLISION_SAP 0
PsSetConstant NX_VISUALIZE_COLLISION_STATIC 0
PsSetConstant NX_VISUALIZE_COLLISION_DYNAMIC 0
PsSetConstant NX_VISUALIZE_COLLISION_FREE 0
PsSetConstant NX_VISUALIZE_FLUID_EMITTERS 0
PsSetConstant NX_VISUALIZE_FLUID_POSITION 0
PsSetConstant NX_VISUALIZE_FLUID_VELOCITY 0
PsSetConstant NX_VISUALIZE_FLUID_KERNEL_RADIUS 0
PsSetConstant NX_VISUALIZE_FLUID_BOUNDS 0
PsSetConstant (null) 0
PsSetConstant (null) 0
PsSetConstant (null) 1
PsSetConstant (null) 1
PsSetConstant (null) 1
PsGravity 0 -9.800000191 0
PsTickRate 0.016666668
PsPhysicsModelEnd

PsSetScene 0
PsPhysicsModel Scene1 position(0,0,0) orientation(0,0,0,1)
PsSetScene 0

