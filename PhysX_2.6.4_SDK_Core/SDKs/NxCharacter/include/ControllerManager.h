#ifndef NX_COLLISION_CONTROLLERMANAGER
#define NX_COLLISION_CONTROLLERMANAGER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxCharacter.h"

#include "Nxp.h"
class NxScene;
class Controller;
class NxController;
class NxControllerDesc;

/**
\brief Manages a list of character controllers.

@see NxController NxBoxController NxCapsuleController
*/
class NXCHARACTER_API ControllerManager
	{
	public:
	/**
	\brief Constructor for the controller manager.
	*/
								ControllerManager();

	/**
	\brief Destructor
	*/
								~ControllerManager();

	/**
	\brief Returns the number of controllers that are being managed.

	\return The number of controllers.
	*/
	NX_INLINE	NxU32			getNbControllers()			const	{ return nbControllers;		}

	/**
	\brief Returns the internal list of controllers.

	\return The first controller in the list
	*/
	NX_INLINE	Controller*		getControllers()					{ return controllerList;	}

	/**
	\brief Creates a new character controller.

	\param[in] scene The scene that the controller will belong to.
	\param[in] desc The controllers descriptor
	\return The new controller

	@see NxController NxControllerDesc
	*/
				NxController*	createController(NxScene* scene, const NxControllerDesc& desc);

	/**
	\brief Deletes all the controllers that are being managed.
	*/
				void			purgeControllers();

	/**
	\brief Releases a controller.

	\param[in] controller The controller to release.

	@see NxController
	*/
				void			releaseController(NxController& controller);
	/**
	\brief Removes a controller from the list of controllers.

	\param[in] controller The controller to remove.
	*/
				void			removeController(Controller* controller);

	/**
	\brief Updates the exposed position from the filtered position of all controllers.
	*/
				void			updateControllers();

				void			printStats();
	protected:
				NxU32			nbControllers;				//
				Controller*		controllerList;				//linked list of scene controllers.
	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
