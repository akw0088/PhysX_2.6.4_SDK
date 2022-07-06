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

PAUSED		= "false"

OdfScript("PsReset")
OdfScript("PsNameSpace Gears")
OdfScript("PsPlaneEmpty")
OdfScript("PsPause %s", PAUSED)
OdfScript("PsTickRate 0.004")
OdfScript("OdfLook 16.8621 26.6236 -31.1342 16.3347 26.1210 -30.4492")

OdfScript("OdfMessageReset")
OdfScript("OdfMessage 6 \"A demonstration of precision collision detection.\"")
OdfScript("OdfMessage 6 \"Use the Z , X and C keys to control the tread.\"")


OdfScript("PsOnKey KeyC C")
OdfScript("PsOnKey KeyX X")
OdfScript("PsOnKey KeyZ Z")

jointID   = 0

--controls user input
function KeyC(key)
  OdfScript("PsSetJoint %d motor -5 5 false", jointID )
end
function KeyX(key)
  OdfScript("PsSetJoint %d motor 0 5 false", jointID )
end
function KeyZ(key)
  OdfScript("PsSetJoint %d motor 5 5 false", jointID )
end


function OnJoint(name,id)
	if  name == "Gears.GearJoint" then
		jointID = id
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
				OdfScript("PsVert %f %f %f", s, -0.1, c)
				OdfScript("PsVert %f %f %f", s, 0.1, c)
			end
		OdfScript("PsConvexEnd")
		
		
		OdfScript("PsConvex %s_center", gearName)
		OdfScript("PsConvex %s_edge localposition(0,%f,0)", gearName, -height/2 - 0.1)
		OdfScript("PsConvex %s_edge localposition(0,%f,0)", gearName, height/2 + 0.1)

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
		OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 1/3*stepLength, stepWidth/8, 0.25, locx, locy, locz-0.1)
		OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 0.2, stepWidth, 0.5, stepLength/2, locy, locz+0.201)
		OdfScript("PsBox sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 0.2, stepWidth, 0.5, -stepLength/2, locy, locz+0.201)
	OdfScript("PsShapeEnd")
	return stepName;
end

OdfScript("PsOnJointCreate OnJoint")

--Gears&GearJoints
--gearName = CreateBoxGear(1.25, 1.4, 0.5, 15)
--gearName = CreateBoxGear(1.75, 2.0, 0.5, 14)
gearName = CreateBoxGear(1.75, 2.0, 1.8, 14)

OdfScript("PsJointBegin GearJoint")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
--OdfScript("PsJointLimit twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointLimit motor(0, 50) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

OdfScript("PsJointBegin GearJoint2")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
OdfScript("PsJointLimit motor(false) twist(free) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

c=-13
s = 0
h = 5
stepwidth = 0.84
b = 0.1
numTeeth = 15
radius = 2
height = 0.1

OdfScript("PsConvexBegin limiter")
	for i=0, numTeeth-1 do
		a = 360 * (i/numTeeth)
		y = math.cos(a*(PI/180)) * radius
		x = math.sin(a*(PI/180)) * radius
		OdfScript("PsVert %f %f %f", x, -(height/2), y)
		OdfScript("PsVert %f %f %f", x, (height/2), y)
	end
OdfScript("PsConvexEnd")


--wheel frames
frameName = "frame"
OdfScript("PsShapeBegin %s", frameName)
OdfScript("PsBox name(frame%d) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 1, 16, 1, 0.1, 0,0,1.7)
OdfScript("PsBox name(frame%d) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 2, 16, 1, 0.1,0,0,-1.7)
--OdfScript("PsBox name(frame%d) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 1, 16, 1, 0.1, -6, -5, 1.7)
--OdfScript("PsBox name(frame%d) sides(%f, %f, %f) localposition(%f,%f,%f) localeuler(0,0,0)", 2, 16, 1, 0.1, -6, -5, -1.7)
OdfScript("PsShapeEnd")

OdfScript("PsShape %s name(gear%d) solvercount(2) position(%f,%f,%f) euler(90,0,0)", gearName, 1, c+0.1, h, s)
OdfScript("PsShape %s name(gear%d) solvercount(2) position(%f,%f,%f) euler(90,0,0)", gearName, 2, c+15, h, s)
OdfScript("PsShape %s name(frame%d) solvercount(2) position(%f,%f,%f) euler(0,0,0)", frameName, 1, -5.5, h, s)
--OdfScript("PsJoint GearJoint gear%d  @world", 1)
OdfScript("PsJoint GearJoint gear%d frame%d", 1, 1)
OdfScript("PsJoint GearJoint2 gear%d frame%d", 2, 1)

--Steps
OdfScript("PsJointBegin HingeJoint")
--OdfScript("PsJointOffset frame(primary)   body(primary)   offset(0,-0.43,0)")
--OdfScript("PsJointOffset frame(secondary)   body(secondary)   offset(0,0.43,0)")
OdfScript("PsJointOffset frame(primary)   body(primary)   offset(0,-0.457,0)")
OdfScript("PsJointOffset frame(secondary)   body(secondary)   offset(0,0.457,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(1,0,0) yaxis(0,1,0) zaxis(1,0,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(1,0,0) yaxis(0,1,0) zaxis(1,0,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced)")
OdfScript("PsJointLimit twist(free) motor(false) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

stepName = CreateStep(5,stepwidth,0.1,0,0,0)

nosteps = 24 --has to be even
for i=1, 17 do
	OdfScript("PsShape %s name(step%d) solvercount(2) euler(-90,90,0) position(%f,%f,%f)", stepName, i, c + stepwidth*(i-1)+i*b, h+2.3, s)
end

--steps 18 to 23
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-70,90,0) position(%f,%f,%f)", stepName, 18, c + 16.1, 7.1, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-40,90,0) position(%f,%f,%f)", stepName, 19, c + 16.8, 6.5, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(0,90,0) position(%f,%f,%f)", stepName, 20, c + 17.2, 5.7, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(15,90,0) position(%f,%f,%f)", stepName, 21, c + 17.1, 4.7, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(40,90,0) position(%f,%f,%f)", stepName, 22, c + 16.8, 3.8, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(60,90,0) position(%f,%f,%f)", stepName, 23, c + 16.3, 3.1, s)

-- steps 24 to 40
for i=1, 17 do
	OdfScript("PsShape %s name(step%d) solvercount(2) euler(90,90,0) position(%f,%f,%f)", stepName, i+23, c + 15.4 - stepwidth*(i-1)-(i)*b, 2.7, s)
end
OdfScript("PsShape %s name(step%d) solvercount(2) euler(90,90,0) position(%f,%f,%f)", stepName, 41, c -0.8, 2.6, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(120,90,0) position(%f,%f,%f)", stepName, 42, c -1.2, 3.1, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(150,90,0) position(%f,%f,%f)", stepName, 43, c -1.8, 3.8, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(170,90,0) position(%f,%f,%f)", stepName, 44, c-2.1, 4.7, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-170,90,0) position(%f,%f,%f)", stepName, 45, c-2.1, 5.6, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-140,90,0) position(%f,%f,%f)", stepName, 46, c-1.5, 6.5, s)
OdfScript("PsShape %s name(step%d) solvercount(2) euler(-110,90,0) position(%f,%f,%f)", stepName, 47, c-0.8, 7.1, s)


for i=1, 46 do
	OdfScript("PsJoint HingeJoint step%d step%d",i,i+1)
end
OdfScript("PsJoint HingeJoint step%d step%d",47,1)

OdfScript("PsOnKey OnKeyPressed U")
OdfScript("PsOnKey OnKeyPressed J")


OdfScript("PsOnJointCreate")

