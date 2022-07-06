#ifndef PXD_CONTEXT_H
#define PXD_CONTEXT_H

#include "px_config.h"
#include "px_task.h"

#ifdef __cplusplus

extern "C"
{
#endif

/*!
\file
Context handling
*/

/************************************************************************/
/* Context handling, types                                              */
/************************************************************************/

typedef PxdHandle PxdContext;

/*!
Context type. Used when creating a new context.
*/
typedef enum PxdContextType_
{
	/*!
	This context should run on CPU only
	*/
	PXD_CPU,
	
#ifdef PXD_SUPPORT_PPU	
	/*!
	This context should utilize the PPU where possible
	*/
	PXD_PPU
#endif
}PxdContextType;


/*!
Get the context type
\param context
The context to get the type of.
\return
Context type. 
*/
PXD_EXPORT_FUNCTION PxdContextType PxdContextGetType(PxdContext context);

/*!
Create a new context

\param type
Context type. This cannot be changed afterwards.
\param agPPUConnection
Pointer to an AgPPUConnection. Backdoor for sharing one PPU connection between
different types of scenes (fluid, cloth and RB)
\return
Context identifier. Zero on failure.
*/
PXD_EXPORT_FUNCTION PxdContext PxdContextCreate(PxdContextType type, void* agPPUConnection);

/*!
Destroy context

\param context
Context identifier
*/
PXD_EXPORT_FUNCTION void PxdContextDestroy(PxdContext context);

/*!
Associate a sub task callback with a context.

\param contexts
Context identifier
\param userSubTaskCallback
User callback function for subtask queuing and execution
\param userParam
User parameter that is passed along on every call to the userSubTaskCallback
function
*/
PXD_EXPORT_FUNCTION void PxdContextSetSubTaskCallback(PxdContext context, PxdUserSubTaskCallback userSubTaskCallback, void* userParam);

#ifdef __cplusplus
}
#endif

#endif
