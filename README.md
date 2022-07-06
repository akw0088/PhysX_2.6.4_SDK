# PhysX_2.6.4_SDK
Old AGEIA Physics SDK and runtime

Later was declared Free to use by AGEIA, unsure of the license, but should be open source (consult with your lawyer)

Despite being a hardware company, this is mostly a rigid body dynamics simulator that runs on the CPU

Runtime will need:	PhysX_2.6.4_SDK_Core\Bin\win32

	PhysXLoader.dll

	NxExtensions.dll
	
	NxCooking.dll
	
	NxCharacter.dll

Link Time will need	PhysX_2.6.4_SDK_Core\SDKs\lib\win32

	NxCharacter.lib
	
	NxCooking.lib
	
	NxExtensions.lib
	
	PhysXLoader.lib

Include will need (depending on features used)

	PhysX_2.6.4_SDK_Core\SDKs\Cooking\include
	
	PhysX_2.6.4_SDK_Core\SDKs\Foundation\include
	
	PhysX_2.6.4_SDK_Core\SDKs\Framework\include
	
	PhysX_2.6.4_SDK_Core\SDKs\NxCharacter\include
	
	PhysX_2.6.4_SDK_Core\SDKs\NxExtensions\include
	
	PhysX_2.6.4_SDK_Core\SDKs\Physics\include
	
	PhysX_2.6.4_SDK_Core\SDKs\PhysXLoader\include
	

Think you will be stuck with 32 bit windows unless you update to a newer version.
But this is simple to use if you just need a physics simulation done quick.
