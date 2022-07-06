#ifndef	NXU_STRING_H

#define	NXU_STRING_H


#ifdef __CELLOS_LV2__
#	define	_stricmp(a,b) strcasecmp((a),(b))
#elif defined(LINUX)
#	define	_stricmp(a,b) strcasecmp((a),(b))
#endif

namespace	NXU
{
const	char *getGlobalString(const	char *str);
void releaseGlobalStrings(void);

void *findInstance(const char	*str); //	find a previously	created	instance of	an entity	by name. null	if not found.
void setInstance(const char	*str,	void *instance); //	set	a	named	association	with this	instance.
void releaseGlobalInstances(void); //	release	the	global instance	table

void reportError(const char *fmt, ... );
void reportWarning(const char *fmt, ... );

extern bool gUseClothActiveState;

};

#endif