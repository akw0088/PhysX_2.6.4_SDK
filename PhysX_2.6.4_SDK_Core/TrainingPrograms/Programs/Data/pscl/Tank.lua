-- PSCL sample script.
-- PSCL is a procedural-style scripting language that allows you to define
-- in great detail a physical definition of scene while keeping it human-readable.
-- For comments or questions contact John Ratcliff (jratcliff@infiniplex.net)
-- or James Dolan (james@6msoft.com).

-- Gears Demo 2.
-- Created 7/14/04 by Matthias Flierl.
-- Inspired by James Dolan

require "odf.lua"
require "math.lua"

PAUSED = "false"
minimizer = 1

OdfScript("PsReset")
OdfScript("PsNameSpace Gears")
OdfScript("PsGravity -15")

OdfScript("PsOnActorCreate OnActor")
OdfScript("PsOnJointCreate OnJoint")

--OdfScript("PsScript small.psc")
--OdfScript("PsTriangleMesh smallterrain position(0,0,0) euler(0,0,0) mass(1) awake(false)")
OdfScript("PsPlane plane(0,1,0,-30) static(true)")
OdfScript("NxDynFrictScaling 1")
OdfScript("NxStaFrictScaling 2.2")
--OdfScript("NxContinuousCollisionDetection 3")

--OdfScript("PsScript small.psc")


OdfScript("PsPlane name(groundPlane) plane(0,1,0,0) static(true)")

OdfScript("PsPause %s", PAUSED)
OdfScript("PsTickRate 0.004")
OdfScript("OdfLook 30 30 80 -10 -10")

--OdfScript("OdfShowGrid true")
--OdfScript("OdfMirror   true")
--OdfScript("OdfShadows  true")
--OdfScript("OdfShowSkyBox true")

OdfScript("OdfMessageReset")
OdfScript("OdfMessage 6 \"A demonstration of precision collision detection.\"")
OdfScript("OdfMessage 6 \"Drive with I,K,J and L. O stops.\"")
OdfScript("OdfMessage 6 \"Rotate tower with T,G,F and H. V stops.\"")
OdfScript("OdfMessage 6 \"Create a wall with U.\"")

--controls user input
OdfScript("PsOnKey KeyI I")
OdfScript("PsOnKey KeyK K")
OdfScript("PsOnKey KeyJ J")
OdfScript("PsOnKey KeyL L")
OdfScript("PsOnKey KeyO O")
OdfScript("PsOnKey KeyU U")
OdfScript("PsOnKey KeyF F")
OdfScript("PsOnKey KeyH H")
OdfScript("PsOnKey KeyT T")
OdfScript("PsOnKey KeyG G")
OdfScript("PsOnKey KeyV V")
OdfScript("PsOnKey KeyN N")

jointID = 0
jointID2 = 0
jointID3 = 0
jointID4 = 0
cylID = 0

motorstrength = 6
motorspeed = 12
rotspeed = 0.5
cylspeed = 0.1
shootspeed = 230

function KeyI(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID, motorspeed, motorstrength)
  OdfScript("PsSetJoint %d motor %f %f true", jointID2, motorspeed, motorstrength)

end

function KeyK(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID, -motorspeed, motorstrength)
  OdfScript("PsSetJoint %d motor %f %f true", jointID2, -motorspeed, motorstrength)

end

function KeyJ(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID, -0.8*motorspeed, motorstrength)
  OdfScript("PsSetJoint %d motor %f %f true", jointID2, 0.8*motorspeed, motorstrength)

end

function KeyL(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID, 0.8*motorspeed, motorstrength)
  OdfScript("PsSetJoint %d motor %f %f true", jointID2, -0.8*motorspeed, motorstrength)

end

function KeyO(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID, 0, motorstrength)
  OdfScript("PsSetJoint %d motor %f %f true", jointID2, 0, motorstrength)

end

function KeyU(key)

  OdfScript("PsWall sides(0.5,0.5,0.5) wid(10) depth(1) hit(10) localeuler(0,0,90) position(20,0,0)")
end

function KeyF(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID3, rotspeed, 5)

end

function KeyH(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID3, -rotspeed, 5)
  
end

function KeyT(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID4, 4*cylspeed, 2)
  
end

function KeyG(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID4, -cylspeed/2, 2)
  
end

function KeyV(key)

  OdfScript("PsSetJoint %d motor %f %f true", jointID4, 0, 2)
  OdfScript("PsSetJoint %d motor %f %f true", jointID3, 0, 2)
  
end

function KeyN(key)
  local posx = 0
  local posy = 0
  local posz = 0
  --local quat1 = 0
  --local quat2 = 0
  --local quat3 = 0
  --local quat4 = 0
  --local mat11 = 0
  local mat12 = 0
  --local mat13 = 0
  --local mat21 = 0
  local mat22 = 0
  --local mat23 = 0
  --local mat31 = 0
  local mat32 = 0
  --local mat33 = 0
  
  local deltax = -6
  local deltay = -6
  local deltaz = -6
  
  ret = OdfScript("PsGetActor %d getposition", cylID )

  if ret[1] == 1 then
    posx = ret[2]
    posy = ret[3]
    posz = ret[4]
    --OdfLog("Position(%s) = (%f,%f,%f)\n", "cyl", ret[2], ret[3], ret[4] );
  end

  ret = OdfScript("PsGetActor %d getorientation", cylID )

  --if ret[1] == 1 then
  --  quat1 = ret[2]
  --  quat2 = ret[3]
  --  quat3 = ret[4]
  --  quat4 = ret[5]
  --end
  
  --quaternion to 3x3 matrix
  --mat11 = 1 - 2*(ret[3]*ret[3]+ret[4]*ret[4])
  mat12 = 2*(ret[2]*ret[3] - ret[4]*ret[5])
  --mat13 = 2*(ret[2]*ret[4]+ret[3]*ret[5])
  
  --mat21 = 2*(ret[2]*ret[3]+ret[4]*ret[5])
  mat22 = 1 - 2*(ret[2]*ret[2]+ret[4]*ret[4])
  --mat23 = 2*(ret[3]*ret[4]-ret[2]*ret[5])
  
  --mat31 = 2*(ret[2]*ret[4]-ret[3]*ret[5])
  mat32 = 2*(ret[3]*ret[4]+ret[2]*ret[5])
  --mat33 = 1-2*(ret[2]*ret[2]+ret[3]*ret[3])
  
  --OdfLog("Matrix: 11:%f 12:%f 13:%f\n", mat11, mat12, mat13)
  --OdfLog("Matrix: 21:%f 22:%f 23:%f\n", mat21, mat22, mat23)
  --OdfLog("Matrix: 31:%f 32:%f 33:%f\n", mat31, mat32, mat33)
  
  OdfScript("PsSphere position(%f,%f,%f) velocity(%f,%f,%f)",posx+deltax*mat12,posy+deltay*mat22,posz+deltaz*mat32,-shootspeed*mat12,-shootspeed*mat22,-shootspeed*mat32 )
  
end

-- handles joint events
function OnJoint(name,id)
	
	if  name == "Gears.LeftMotorJoint" then
		jointID = id
	end
	if  name == "Gears.RightMotorJoint" then
		jointID2 = id
	end
	if  name == "Gears.TowerJoint" then
		jointID3 = id
	end
	if  name == "Gears.CylinderJoint" then
		jointID4 = id
	end
end

--handles actors
function OnActor(name,id)
	if name == "Gears.cyl1" then
		cylID = id
	end
end

-- Defines a Gear shape. Returns shape name.
-- gearName  CreateBoxGear(minRadius, maxRadius, numTeeth)
function CreateBoxGear(minRadius, maxRadius, height, numTeeth)
	gearName = Sprintf("BoxGear_%f_%f_%f_%d", minRadius, maxRadius, height, numTeeth)
	
	parimeter	= 2 * PI * minRadius
	toothWidth	= (parimeter / (numTeeth*2)) * 0.75
	toothHeight	= height
	toothDepth	= maxRadius - (minRadius/2)
	
	OdfScript("PsShapeBegin %s", gearName)
		
		OdfScript("PsConvexBegin %s_center", gearName)
			for i=0, numTeeth-1 do
				a = 360 * (i/numTeeth)
				c = math.cos(a*(PI/180)) * minRadius
				s = math.sin(a*(PI/180)) * minRadius
				OdfScript("PsVert %f %f %f", s, -(height/2), c)
				OdfScript("PsVert %f %f %f", s, (height/2), c)
			end
		OdfScript("PsConvexEnd")
		
		OdfScript("PsConvexBegin %s_edge", gearName)
			for i=0, numTeeth-1 do
				a = 360 * (i/numTeeth)
				c = math.cos(a*(PI/180)) * maxRadius
				s = math.sin(a*(PI/180)) * maxRadius
				OdfScript("PsVert %f %f %f", s, -0.1/minimizer, c)
				OdfScript("PsVert %f %f %f", s, 0.1/minimizer, c)
			end
		OdfScript("PsConvexEnd")
		
		
		OdfScript("PsConvex %s_center", gearName)
		OdfScript("PsConvex %s_edge localposition(0,%f,0)", gearName, -height/2 - 0.1/minimizer)
		OdfScript("PsConvex %s_edge localposition(0,%f,0)", gearName, height/2 + 0.1/minimizer)
		
		for i=0, numTeeth-1 do
			a = 360 * (i/numTeeth)
			c = math.cos(a*(PI/180)) * ((minRadius/2)+toothDepth/2)
			s = math.sin(a*(PI/180)) * ((minRadius/2)+toothDepth/2)
			OdfScript("PsBox sides(%f, %f, %f) localposition(%f,0,%f) localeuler(0,%f,0)", toothWidth, toothHeight, toothDepth, s, c, a)
		end
	OdfScript("PsShapeEnd")
	
	return gearName;
end


-- Defines a Step
function CreateStep(stepLength, stepWidth, stepHeight,locx,locy,locz)
	stepName = Sprintf("Step_%f_%f_%f", stepLength,stepWidth, stepHeight)
	OdfScript("PsShapeBegin %s", stepName)
		OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", stepLength, stepWidth, stepHeight, locx, locy, locz)
		--OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 1/2*stepLength, stepWidth/8, 0.25/minimizer, locx, locy, locz-0.1/minimizer)
		OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 1/3*stepLength, stepWidth/8, 0.25/minimizer, locx, locy, locz-0.1/minimizer)
		--OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 0.2/minimizer, stepWidth, 0.5/minimizer, stepLength/2, locy, locz+0.201/minimizer)
		--OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 0.2/minimizer, stepWidth, 0.5/minimizer, -stepLength/2, locy, locz+0.201/minimizer)
	OdfScript("PsShapeEnd")
	return stepName;
end

function CreateCyl(maxRadius,length,sides)
	cylName = "cyl"
	OdfScript("PsShapeBegin %s", cylName)
	OdfScript("PsConvexBegin points")
		for i=0, sides-1 do
			a = 360 * (i/sides)
			c = math.cos(a*(PI/180)) * maxRadius
			s = math.sin(a*(PI/180)) * maxRadius
			OdfScript("PsVert %f %f %f", s, -length/2, c)
			OdfScript("PsVert %f %f %f", s, length/2, c)
		end
	OdfScript("PsConvexEnd")
	OdfScript("PsConvex points")
	OdfScript("PsShapeEnd")
	return cylName;
end

cylName = CreateCyl(0.4/minimizer,12/minimizer,15/minimizer)

chassisName = "chassis"
OdfScript("PsShapeBegin %s", chassisName)
OdfScript("PsBox name('middle_down') mass(10) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 17.7/minimizer, 3.8/minimizer, 6.6/minimizer, 1/minimizer, -1.7/minimizer, 0)
OdfScript("PsBox name('middle_deck') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 17.7/minimizer, 0.3/minimizer, 14.6/minimizer, 1/minimizer, 0.35/minimizer, 0)
OdfScript("PsBox name('side_right') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 18.3/minimizer, 0.3/minimizer, 3.5/minimizer, 1.3/minimizer, 0.58/minimizer, 5.5)
OdfScript("PsBox name('side_left') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 18.3/minimizer, 0.3/minimizer, 3.5/minimizer, 1.3/minimizer, 0.58/minimizer, -5.5)
OdfScript("PsBox name('front_right') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,%f)", 3.8/minimizer, 0.3/minimizer, 3.5/minimizer, -9.7/minimizer, 0.25/minimizer, 5.5,10)
OdfScript("PsBox name('front_left') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,%f)", 3.8/minimizer, 0.3/minimizer, 3.5/minimizer, -9.7/minimizer, 0.25/minimizer, -5.5,10)
OdfScript("PsBox name('front_up') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,%f)", 4/minimizer, 0.3/minimizer, 7.6/minimizer, -9.6/minimizer, -0.15/minimizer, 0,15)
OdfScript("PsBox name('front_down') mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,%f)", 4/minimizer, 0.3/minimizer, 7.6/minimizer, -10.3/minimizer, -2.3/minimizer, 0,125)
--OdfScript("PsBox mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 22.5/minimizer, 0.3/minimizer, 16/minimizer, 1.3/minimizer, 0.28/minimizer, 0)
--OdfScript("PsBox mass(5) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 12/minimizer, 0.1/minimizer, 5/minimizer, -0.72/minimizer, 3.3/minimizer, 0)
OdfScript("PsShapeEnd")

towerName = "tower"
OdfScript("PsShapeBegin %s", towerName)
OdfScript("PsBox mass(1) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 9.4/minimizer, 3.3/minimizer, 7/minimizer, -0.5/minimizer, 2.45/minimizer, 0)
OdfScript("PsShapeEnd")

--
--
--Declaration of elements
--
--


OdfScript("PsJointBegin LeftMotorJoint")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
--OdfScript("PsJointLimit twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointLimit motor(false) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

OdfScript("PsJointBegin RightMotorJoint")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
OdfScript("PsJointLimit motor(false) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

OdfScript("PsJointBegin WheelJoint")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
--OdfScript("PsJointLimit twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointLimit motor(false) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

OdfScript("PsJointBegin TowerJoint")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
--OdfScript("PsJointLimit twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointLimit motor(false) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

OdfScript("PsJointBegin CylinderJoint")
OdfScript("PsJointOffset frame(primary)   body(primary)   offset(0,0,0)")
OdfScript("PsJointOffset frame(primary)   body(secondary)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   yaxis(1,0,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) yaxis(1,0,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
--OdfScript("PsJointLimit twist(free) twistspring(none) swing1(limit) swing2(limit) linear(0.1,0.1,0.1)")
OdfScript("PsJointLimit twist(-0.2,0,1,  0.23,0,1) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
--OdfScript("PsJointLimit motor(false) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

--StepsJoint
OdfScript("PsJointBegin HingeJoint")
--minimizer = 5
--OdfScript("PsJointOffset frame(primary)   body(primary)   offset(0,-0.087,0)")
--OdfScript("PsJointOffset frame(secondary)   body(secondary)   offset(0,0.087,0)")
--minimizer = 2
--OdfScript("PsJointOffset frame(primary)   body(primary)   offset(0,-0.229,0)")
--OdfScript("PsJointOffset frame(secondary)   body(secondary)   offset(0,0.229,0)")
--minimizer = 1
OdfScript("PsJointOffset frame(primary)   body(primary)   offset(0,-0.457,0)")
OdfScript("PsJointOffset frame(secondary)   body(secondary)   offset(0,0.457,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(1,0,0) yaxis(0,1,0) zaxis(1,0,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(1,0,0) yaxis(0,1,0) zaxis(1,0,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
OdfScript("PsJointLimit twist(free) motor(false) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

m = 3
stepwidth = 0.84/minimizer
b = 0.1/minimizer
s=-50/minimizer
numTeeth = 15
radius = 2/minimizer
height = 0.1/minimizer

gearName = CreateBoxGear(1.75/minimizer, 2.0/minimizer, 1.8/minimizer, 14)
stepName = CreateStep(3.6/minimizer,stepwidth,0.1/minimizer,0/minimizer,0/minimizer,0/minimizer)


--wheel frames
frameName = "frame"

OdfScript("PsShapeBegin %s", frameName)
OdfScript("PsBox name(frame%d) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 1, 16/minimizer, 1/minimizer, 0.1/minimizer, 0/minimizer,0/minimizer,1.7/minimizer)
OdfScript("PsBox name(frame%d) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 2, 16/minimizer, 1/minimizer, 0.1/minimizer,0/minimizer,0/minimizer,-1.7/minimizer)
OdfScript("PsShapeEnd")


--
--
-- Instantiation
--
--


--chassis
OdfScript("PsShape %s name(chassis%d) mesh(tank.gr2) mass(1) solvercount(30) position(%f,%f,%f) euler(0,0,0)", chassisName, 1, 8/minimizer,(m+2.4)/minimizer, 5.35/minimizer)

--tower
OdfScript("PsShape %s name(tower%d) mesh(tower.gr2) mass(1) solvercount(30) position(%f,%f,%f) euler(0,0,0)", towerName, 1, 8/minimizer,(m+1.9)/minimizer, 5.35/minimizer)
OdfScript("PsShape %s name(cyl%d) mesh(shooter.gr2) solvercount(30) position(%f,%f,%f) euler(90,90,0)",cylName,1, -1.8/minimizer, (m+4.5)/minimizer, 5.35/minimizer)
OdfScript("PsJoint CylinderJoint %s1 %s1", towerName, cylName)

--joint chassis/tower
OdfScript("PsJoint TowerJoint %s1 %s1", chassisName, towerName)

--crawler left
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 1, 0.15/minimizer, m/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 2, 5.0/minimizer, m/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 3, 10.1/minimizer, m/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 4, 15/minimizer, m/minimizer, s+0.29/minimizer)
--OdfScript("PsShape %s name(frame%d) solvercount(30) position(%f,%f,%f) euler(0,0,0)", frameName, 1, 7.5/minimizer, m/minimizer, s-0.4/minimizer)
--OdfScript("PsJoint LeftMotorJoint gear%d  @world", 1)
--OdfScript("PsJoint GearJoint gear%d frame%d", 1, 1)
--OdfScript("PsJoint WheelJoint gear%d frame%d", 2, 1)
OdfScript("PsJoint LeftMotorJoint gear%d chassis%d", 1, 1)
OdfScript("PsJoint WheelJoint gear%d chassis%d", 2, 1)
OdfScript("PsJoint WheelJoint gear%d chassis%d", 3, 1)
OdfScript("PsJoint LeftMotorJoint gear%d chassis%d", 4, 1)

nosteps = 24 --has to be even
for i=1, 17 do
	OdfScript("PsShape %s name(step%d) solvercount(2) euler(-90,90,0) position(%f,%f,%f)", stepName, i, stepwidth*(i-1)+i*b, (m+2.3)/minimizer, s+0.29/minimizer)
end

--steps 18 to 23
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-70,90,0) position(%f,%f,%f)", stepName, 18, 16.1/minimizer, (m+2.1)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-40,90,0) position(%f,%f,%f)", stepName, 19, 16.8/minimizer, (m+1.5)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(0,90,0) position(%f,%f,%f)", stepName, 20, 17.2/minimizer, (m+0.7)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(15,90,0) position(%f,%f,%f)", stepName, 21, 17.1/minimizer, (m-0.3)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(40,90,0) position(%f,%f,%f)", stepName, 22, 16.8/minimizer, (m-1.2)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(60,90,0) position(%f,%f,%f)", stepName, 23, 16.3/minimizer, (m-1.9)/minimizer, s+0.29/minimizer)

-- steps 24 to 40
for i=1, 17 do
	OdfScript("PsShape %s name(step%d) solvercount(2) euler(90,90,0) position(%f,%f,%f)", stepName, i+23, 15.4/minimizer - stepwidth*(i-1)-(i)*b, (m-2.3)/minimizer, s+0.29/minimizer)
end
OdfScript("PsShape %s name(step%d) solvercount(2) euler(90,90,0) position(%f,%f,%f)", stepName, 41, -0.8/minimizer, (m-2.4)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(120,90,0) position(%f,%f,%f)", stepName, 42, -1.2/minimizer, (m-1.9)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(150,90,0) position(%f,%f,%f)", stepName, 43, -1.8/minimizer, (m-1.2)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(170,90,0) position(%f,%f,%f)", stepName, 44, -2.1/minimizer, (m-0.3)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-170,90,0) position(%f,%f,%f)", stepName, 45, -2.1/minimizer, (m+0.6)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-140,90,0) position(%f,%f,%f)", stepName, 46, -1.5/minimizer, (m+1.5)/minimizer, s+0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-110,90,0) position(%f,%f,%f)", stepName, 47, -0.8/minimizer, (m+2.1)/minimizer, s+0.29/minimizer)


for i=1, 46 do
	OdfScript("PsJoint HingeJoint step%d step%d",i,i+1)
end
OdfScript("PsJoint HingeJoint step%d step%d",47,1)


																											  
--crawler right
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 5, 0.15/minimizer, m/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 6, 5.0/minimizer, m/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 7, 10.1/minimizer, m/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) euler(90,0,0)", gearName, 8, 15/minimizer, m/minimizer, s+12/minimizer-0.29/minimizer)
--OdfScript("PsShape %s name(frame%d) solvercount(30) position(%f,%f,%f) euler(0,0,0)", frameName, 2, 7.5/minimizer, m/minimizer, s+11.9/minimizer-0.29/minimizer)
--OdfScript("PsJoint LeftMotorJoint gear%d  @world", 1)
OdfScript("PsJoint RightMotorJoint gear%d chassis%d", 5, 1)
OdfScript("PsJoint WheelJoint gear%d chassis%d", 6, 1)
OdfScript("PsJoint WheelJoint gear%d chassis%d", 7, 1)
OdfScript("PsJoint RightMotorJoint gear%d chassis%d", 8, 1)

nosteps = 24 --has to be even
for i=1, 17 do
	OdfScript("PsShape %s name(step%d) solvercount(2) euler(-90,90,0) position(%f,%f,%f)", stepName, i+47, stepwidth*(i-1)+i*b, (m+2.3)/minimizer, s+12/minimizer-0.29/minimizer)
end

--steps 18 to 23
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-70,90,0) position(%f,%f,%f)", stepName, 18+47, 16.1/minimizer, (m+2.1)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-40,90,0) position(%f,%f,%f)", stepName, 19+47, 16.8/minimizer, (m+1.5)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(0,90,0) position(%f,%f,%f)", stepName, 20+47, 17.2/minimizer, (m+0.7)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(15,90,0) position(%f,%f,%f)", stepName, 21+47, 17.1/minimizer, (m-0.3)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(40,90,0) position(%f,%f,%f)", stepName, 22+47, 16.8/minimizer, (m-1.2)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(60,90,0) position(%f,%f,%f)", stepName, 23+47, 16.3/minimizer, (m-1.9)/minimizer, s+12/minimizer-0.29/minimizer)

-- steps 24 to 40
for i=1, 17 do
	OdfScript("PsShape %s name(step%d) solvercount(2) euler(90,90,0) position(%f,%f,%f)", stepName, i+23+47, 15.4/minimizer - stepwidth*(i-1)-(i)*b, (m-2.3)/minimizer, s+12/minimizer-0.29/minimizer)
end
OdfScript("PsShape %s name(step%d) solvercount(2) euler(90,90,0) position(%f,%f,%f)", stepName, 41+47, -0.8/minimizer, (m-2.4)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(120,90,0) position(%f,%f,%f)", stepName, 42+47, -1.2/minimizer, (m-1.9)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(150,90,0) position(%f,%f,%f)", stepName, 43+47, -1.8/minimizer, (m-1.2)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(170,90,0) position(%f,%f,%f)", stepName, 44+47, -2.1/minimizer, (m-0.3)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-170,90,0) position(%f,%f,%f)", stepName, 45+47, -2.1/minimizer, (m+0.6)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-140,90,0) position(%f,%f,%f)", stepName, 46+47, -1.5/minimizer, (m+1.5)/minimizer, s+12/minimizer-0.29/minimizer)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-110,90,0) position(%f,%f,%f)", stepName, 47+47, -0.8/minimizer, (m+2.1)/minimizer, s+12/minimizer-0.29/minimizer)


for i=1, 46 do
	OdfScript("PsJoint HingeJoint step%d step%d",i+47,i+1+47)
end
OdfScript("PsJoint HingeJoint step%d step%d",94,48)

--OdfScript("PsScript smallterrain.psc")

OdfScript("PsWall sides(0.75,0.75,0.75) wid(1) hit(10) depth(10) position(-50,10,0)")
OdfScript("PsWall sides(0.75,0.75,0.75) wid(1) hit(10) depth(10) position(-70,0,0)")
OdfScript("PsWall sides(0.75,0.75,0.75) wid(1) hit(10) depth(10) position(-100,20,0)")
OdfScript("PsWall sides(0.75,0.75,0.75) wid(1) hit(10) depth(10) position(-80,-10,0)")
OdfScript("PsWall sides(0.75,0.75,0.75) wid(1) hit(10) depth(10) position(-30,-20,0)")
OdfScript("PsWall sides(0.75,0.75,0.75) wid(1) hit(10) depth(10) position(-40,35,0)")
