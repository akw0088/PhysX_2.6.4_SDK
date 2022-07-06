-- PSCL sample script.
-- PSCL is a procedural-style scripting language that allows you to define
-- in great detail a physical definition of scene while keeping it human-readable.
-- For comments or questions contact John Ratcliff (jratcliff@infiniplex.net)
-- or James Dolan (james@6msoft.com).

-- Rigid Body Grass Demo
-- Created 7/15/04 by James Dolan

require "odf.lua"

OdfScript("PsReset")
OdfScript("PsPlaneEmpty")
OdfScript("PsPause false")
OdfScript("OdfShadows true")
OdfScript("OdfLook 14.1145 11.4840 14.0090 13.4890 11.0044 13.3936")
OdfScript("PsTickRate 0.01")

OdfScript("OdfMessage 6 \"Rigid-body grass.\"")


-- Creates a grass blade definition.
-- name CreateBlade(height, numJoints)
-- name - Name of definition
-- height - height of the blade
-- numJoints - number of joints in the blade
function CreateBlade(height, numJoints)
	
	if numJoints < 2 then
		return "error"
	end
	
	_name = Sprintf("GrassBlade_%f_%d", height, numJoints)
	_partHeight = height / numJoints
	_radius = 0.1
	
	_jointName = Sprintf("%s_Joint", _name)
	OdfScript("PsJointBegin %s", _jointName)
		OdfScript("PsJointOffset frame(primary) body(both)		offset(0,%f,0)", -_partHeight/2)
		OdfScript("PsJointAxes   frame(primary) body(primary)   xaxis(1,0,0) yaxis(0,1,0)")
		OdfScript("PsJointAxes   frame(primary) body(secondary) xaxis(1,0,0) yaxis(0,1,0)")
		OdfScript("PsJointLimit twist(-0.1,0,1,0.1,0,1) twistspring(0,1,0) swing1(locked) swing2(locked) swingspring(none) linear(locked,locked,locked)")
		OdfScript("PsDefaultSettings bodycollide(false) breakable(false)")
	OdfScript("PsJointEnd")

	OdfScript("PsCollectionBegin %s", _name)

		for i=0, numJoints do
			_partName = Sprintf("%s_Part_%d", _name, i)
			--OdfScript("PsCapsule group(1) name(%s) height(%f) radius(%f) position(0,%f,0) mass(%f) lineardamping(10)", _partName, _partHeight, _radius, _partHeight/2+_radius+(_partHeight+_radius*2)*i, numJoints-i+1)
			OdfScript("PsBox group(1) name(%s) sides(%f,%f,%f) position(0,%f,0) mass(%f) lineardamping(10)", _partName, _radius, _partHeight+2*_radius, _radius, _partHeight/2+_radius+(_partHeight+_radius*2)*i, numJoints-i+1)
		end

		_partNameRoot = Sprintf("%s_Part_%d", _name, 0)
		OdfScript("PsJoint %s %s @world", _jointName, _partNameRoot)

		for i=1, numJoints do
			_partNameA = Sprintf("%s_Part_%d", _name, i-1)
			_partNameB = Sprintf("%s_Part_%d", _name, i)
			OdfScript("PsJoint %s %s %s", _jointName, _partNameB, _partNameA)
		end

	OdfScript("PsCollectionEnd")

	return _name

end

OdfScript("PsSphere radius(1.5) mass(500) position(0,30,0)")

grassBlade0 = CreateBlade(3.0, 4)
grassBlade1 = CreateBlade(1.8, 2)
grassBlade2 = CreateBlade(2.0, 3)
grassBlade3 = CreateBlade(1.5, 2)
grassBlade4 = CreateBlade(2.1, 3)

scale = 0.45

patchsize = 8

for x=-patchsize, patchsize do
	for z=-patchsize, patchsize do
		fRX = math.random(-45,45)*0.01
		fRZ = math.random(-45,45)*0.01

		r = math.random(0, 4)
		if r == 0 then
			OdfScript("PsLoadCollection %s position(%f,0,%f)", grassBlade0, scale*(x+fRX), scale*(z+fRZ))
		elseif r == 1 then
			OdfScript("PsLoadCollection %s position(%f,0,%f)", grassBlade1, scale*(x+fRX), scale*(z+fRZ))
		elseif r == 1 then
			OdfScript("PsLoadCollection %s position(%f,0,%f)", grassBlade2, scale*(x+fRX), scale*(z+fRZ))
		elseif r == 1 then
			OdfScript("PsLoadCollection %s position(%f,0,%f)", grassBlade3, scale*(x+fRX), scale*(z+fRZ))
		else
			OdfScript("PsLoadCollection %s position(%f,0,%f)", grassBlade4, scale*(x+fRX), scale*(z+fRZ))
		end

	end
end


OdfScript("PsGroupCollision 1 1 false")

