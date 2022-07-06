-- PSCL sample script.
-- PSCL is a procedural-style scripting language that allows you to define
-- in great detail a physical definition of scene while keeping it human-readable.
-- For comments or questions contact John Ratcliff (jratcliff@infiniplex.net)
-- or James Dolan (james@6msoft.com).

-- Gears Demo.
-- Created 6/28/04 by James Dolan

require("odf.lua")
require("math.lua")

PAUSED		= "false"

OdfScript("PsReset")
OdfScript("PsNameSpace Gears")
OdfScript("PsPlane name(groundPlane) plane(0,1,0,0) static(true)")
OdfScript("PsPause %s", PAUSED)
OdfScript("PsTickRate 0.005")
OdfScript("OdfLook -13.1936 11.8687 -13.4168 -12.5533 11.5123 -12.7364")

OdfScript("OdfMessageReset")
OdfScript("OdfMessage 6 \"A demonstration of precision collision detection.\"")
OdfScript("OdfMessage 6 \"Press B to toggle between art and collision geometry.\"")

-- Defines a Gear shape. Returns shape name.
-- gearName  CreateBoxGear(minRadius, maxRadius, numTeeth)
function CreateBoxGear(minRadius, maxRadius, height, numTeeth)
	gearName = Sprintf("BoxGear_%f_%f_%f_%d", minRadius, maxRadius, height, numTeeth)
	
	parimeter	= 2 * PI * minRadius
	toothWidth	= (parimeter / (numTeeth*2)) * 0.75
	toothHeight	= height
	toothDepth	= maxRadius - minRadius
	
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
		OdfScript("PsConvex %s_center", gearName)
		
		for i=0, numTeeth-1 do
			a = 360 * (i/numTeeth)
			c = math.cos(a*(PI/180)) * (minRadius+toothDepth/2)
			s = math.sin(a*(PI/180)) * (minRadius+toothDepth/2)
			OdfScript("PsBox sides(%f, %f, %f) localposition(%f,0,%f) localeuler(0,%f,0)", toothWidth, toothHeight, toothDepth, s, c, a)
		end
	OdfScript("PsShapeEnd")
	
	return gearName;
end


gearName = CreateBoxGear(1.25, 1.4, 0.5, 30)

OdfScript("PsJointBegin GearJoint")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(0,1,0) yaxis(0,0,1)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(0,1,0) yaxis(0,0,1)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced) breakable(false) ")
OdfScript("PsJointLimit twist(free) motor(false) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

iNumGears	= 10
radius		= 4.35
for i=0, iNumGears-1 do
	a = 360 * (i/iNumGears)

	c = math.cos(a*(PI/180)) * radius
	s = math.sin(a*(PI/180)) * radius

	OdfScript("PsShape %s name(gear%d) solvercount(30) position(%f,%f,%f) mesh(Gear_large_stacker.gr2) euler(0,%f,0)", gearName, i, c, 0.3, s, 5*(math.mod(i,2)))
	OdfScript("PsJoint GearJoint gear%d @world", i)
end

OdfScript("PsJointBegin GearJoint2")
OdfScript("PsJointOffset frame(primary)   body(both)   offset(0,0,0)")
OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(1,0,0) yaxis(0,1,0)")
OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(1,0,0) yaxis(0,1,0)")
OdfScript("PsDefaultSettings bodycollide(false) jointmethod(reduced) breakable(false) ")
OdfScript("PsJointLimit twist(free) motor(false) twistspring(none) swing1(locked) swing2(locked) linear(locked,locked,locked)")
OdfScript("PsJointEnd")

for i=0, iNumGears-1 do
	a = 360 * (i/iNumGears)
	
	c = math.cos(a*(PI/180)) * radius
	s = math.sin(a*(PI/180)) * radius
	
	OdfScript("PsShape %s name(gearb%d) solvercount(30) position(%f,%f,%f) euler(0,%f,90)", gearName, i, 5.88, c + 6.2, s, 5*(math.mod(i,2)))
	OdfScript("PsJoint GearJoint gearb%d @world", i)
end