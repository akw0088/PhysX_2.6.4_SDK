#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#include "NXU_Asc2Bin.h"
#include "NXU_string.h"



namespace	NXU
{

static inline	bool IsWhitespace(char c)
{
	if (c	== ' ' ||	c	== 9 ||	c	== 13	|| c ==	10 ||	c	== ',')
	{
		return true;
	}
	return false;
}



static inline	const	char *SkipWhitespace(const char	*str)
{
	if ( str )
	{
		while	(*str	&& IsWhitespace(*str))
		{
			str++;
		}
	}
	return str;
}

static char	ToLower(char c)
{
	if (c	>= 'A' &&	c	<= 'Z')
	{
		c	+= 32;
	}
	return c;
}

static inline	unsigned int GetHex(unsigned char	c)
{
	unsigned int v = 0;
	c	=	ToLower(c);
	if (c	>= '0' &&	c	<= '9')
	{
		v	=	c	-	'0';
	}
	else
	{
		if (c	>= 'a' &&	c	<= 'f')
		{
			v	=	10+c - 'a';
		}
	}
	return v;
}

static inline	unsigned char	GetHEX1(const	char *foo, const char	**endptr)
{
	unsigned int ret = 0;

	if ( foo )
	{

		ret	=	(GetHex(foo[0])	<< 4)	|	GetHex(foo[1]);

		if (endptr)
		{
			*endptr	=	foo	+	2;
		}
	}

	return (unsigned char)ret;
}


static inline	unsigned short GetHEX2(const char	*foo,	const	char **endptr)
{
	unsigned int ret = 0;

	if ( foo )
	{
		ret	=	(GetHex(foo[0])	<< 12) | (GetHex(foo[1]) <<	8) | (GetHex(foo[2]) <<	4) | GetHex(foo[3]);

		if (endptr)
		{
			*endptr	=	foo	+	4;
		}
	}

	return (unsigned short)ret;
}

static inline	unsigned int GetHEX4(const char	*foo,	const	char **endptr)
{
	unsigned int ret = 0;

	if ( foo )
	{
		for	(int i = 0;	i	<	8; i++)
		{
			ret	=	(ret <<	4) | GetHex(foo[i]);
		}

		if (endptr)
		{
			*endptr	=	foo	+	8;
		}
	}

	return ret;
}

static inline	unsigned int GetHEX(const	char *foo, const char	**endptr)
{
	unsigned int ret = 0;

	if ( foo )
	{
		while	(*foo)
		{
			unsigned char	c	=	ToLower(*foo);
			unsigned int v = 0;
			if (c	>= '0' &&	c	<= '9')
			{
				v	=	c	-	'0';
			}
			else
			{
				if (c	>= 'a' &&	c	<= 'f')
				{
					v	=	10+c - 'a';
				}
				else
				{
					break;
				}
			}
			ret	=	(ret <<	4) | v;
			foo++;
		}

		if (endptr)
		{
			*endptr	=	foo;
		}
	}

	return ret;
}




#define	MAXNUM 128

static inline float	GetFloatValue(const	char *str, const char	**next)
{
	float	ret	=	0;

	if ( str )
	{
		if (next)
		{
			*next	=	0;
		}

		str	=	SkipWhitespace(str);

		char dest[MAXNUM];
		char *dst	=	dest;
		const	char *hex	=	0;

		for	(int i = 0;	i	<	(MAXNUM	-	1);	i++)
		{
			char c =	*str;
			if (c	== 0 ||	IsWhitespace(c))
			{
				if (next)
				{
					*next	=	str;
				}
				break;
			}
			else if	(c ==	'$')
			{
				hex	=	str	+	1;
			}
			*dst++ = ToLower(c);
			str++;
		}

		*dst = 0;

		if (hex)
		{
			unsigned int iv	=	GetHEX(hex,	0);
			float	*v = (float*)	&iv;
			ret	=	 *v;
		}
		else if	(dest[0] ==	'f')
		{
			if (_stricmp(dest,	"fltmax")	== 0 ||	_stricmp(dest,	"fmax")	== 0 ||	_stricmp(dest,	"FLT_MAX") ==	0	|| _stricmp(dest, "INF")	== 0)
			{
				ret	=	FLT_MAX;
			}
			else if	(_stricmp(dest, "fltmin") ==	0	|| _stricmp(dest, "fmin") ==	0	|| _stricmp(dest, "FLT_MIN")	== 0 ||	_stricmp(dest,	"-INF")	== 0)
			{
				ret	=	FLT_MIN;
			}
		}
		else if	(dest[0] ==	't')
		// t or	'true' is	treated	as the value '1'.
		{
			ret	=	1;
		}
		else
		{
			ret	=	(float)atof(dest);
		}
	}
	return ret;
}

static inline	int	GetIntValue(const	char *str, const char	**next)
{
	int	ret	=	0;

	if ( str )
	{
		if (next)
		{
			*next	=	0;
		}

		str	=	SkipWhitespace(str);

		char dest[MAXNUM];
		char *dst	=	dest;

		for	(int i = 0;	i	<	(MAXNUM	-	1);	i++)
		{
			char c =	*str;
			if (c	== 0 ||	IsWhitespace(c))
			{
				if (next)
				{
					*next	=	str;
				}
				break;
			}
			*dst++ = c;
			str++;
		}

		*dst = 0;

		ret	=	atoi(dest);
	}

	return ret;
}



#ifdef PLAYSTATION3
#include <ctype.h> //	for	tolower()
#endif

enum Atype
{
	AT_FLOAT,	AT_INT,	AT_CHAR, AT_BYTE,	AT_SHORT,	AT_STR,	AT_HEX1, AT_HEX2,	AT_HEX4, AT_LAST
};

#define	MAXARG 64

#if	0
void TestAsc2bin(void)
{
	Asc2Bin("1 2 A 3 4 Foo AB	ABCD FFEDFDED",	1, "f	d	c	b	h	p	x1 x2	x4", 0);
}
#endif

void *Asc2Bin(const	char *source,	const	int	count, const char	*spec, void	*dest)
{
  if ( !source || !spec ) 
		return 0;
	int	cnt	=	0;
	int	size = 0;

	Atype	types[MAXARG];

	const	char *ctype	=	spec;

	while	(*ctype)
	{
		switch (ToLower(*ctype))
		{
			case 'f':
				size +=	sizeof(float);
				types[cnt] = AT_FLOAT;
				cnt++;
				break;
			case 'd':
				size +=	sizeof(int);
				types[cnt] = AT_INT;
				cnt++;
				break;
			case 'c':
				size +=	sizeof(char);
				types[cnt] = AT_CHAR;
				cnt++;
				break;
			case 'b':
				size +=	sizeof(char);
				types[cnt] = AT_BYTE;
				cnt++;
				break;
			case 'h':
				size +=	sizeof(short);
				types[cnt] = AT_SHORT;
				cnt++;
				break;
			case 'p':
				size +=	sizeof(const char*);
				types[cnt] = AT_STR;
				cnt++;
				break;
			case 'x':
				if (1)
				{
					Atype	type = AT_HEX4;
					int	sz = 4;
					switch (ctype[1])
					{
					case '1':
						type = AT_HEX1;
						sz = 1;
						ctype++;
						break;
					case '2':
						type = AT_HEX2;
						sz = 2;
						ctype++;
						break;
					case '4':
						type = AT_HEX4;
						sz = 4;
						ctype++;
						break;
					}
					types[cnt] = type;
					size +=	sz;
					cnt++;
				}
				break;
		}
		if (cnt	== MAXARG)
		{
			return 0;
		}
		// over	flowed the maximum specification!
		ctype++;
	}

	bool myalloc = false;

	if (dest ==	0)
	{
		myalloc	=	true;
		dest = (char*)new	char[count *size];
	}

	// ok...ready	to parse lovely	data....
	memset(dest, 0,	count	*size);	// zero	out	memory

	char *dst	=	(char*)dest; //	where	we are storing the results
	for	(int i = 0;	i	<	count && source; i++)
	{
		for	(int j = 0;	j	<	cnt && source; j++)
		{
			source = SkipWhitespace(source); //	skip white spaces.

			if (*source	== 0)
			// we	hit	the	end	of the input data	before we	successfully parsed	all	input!
			{
				if (myalloc)
				{
					delete dest;
				}
				return 0;
			}

			switch (types[j])
			{
				case AT_FLOAT:
					if (1)
					{
						float	*v = (float*)dst;
						*v = GetFloatValue(source, &source);
						dst	+= sizeof(float);
					}
					break;
				case AT_INT:
					if (1)
					{
						int	*v = (int*)dst;
						*v = GetIntValue(source, &source);
						dst	+= sizeof(int);
					}
					break;
				case AT_CHAR:
					if (1)
					{
						*dst++ =	*source++;
					}
					break;
				case AT_BYTE:
					if (1)
					{
						char *v	=	(char*)dst;
						*v = GetIntValue(source, &source);
						dst	+= sizeof(char);
					}
					break;
				case AT_SHORT:
					if (1)
					{
						short	*v = (short*)dst;
						*v = GetIntValue(source, &source);
						dst	+= sizeof(short);
					}
					break;
				case AT_STR:
					if (1)
					{
						const	char **ptr = (const	char **)dst;
						*ptr = source;
						dst	+= sizeof(const	char*);
						while	(*source &&	!IsWhitespace(*source))
						{
							source++;
						}
					}
					break;
				case AT_HEX1:
					if (1)
					{
						unsigned int hex = GetHEX1(source, &source);
						unsigned char	*v = (unsigned char*)dst;
						*v = hex;
						dst	+= sizeof(unsigned char);
					}
					break;
				case AT_HEX2:
					if (1)
					{
						unsigned int hex = GetHEX2(source, &source);
						unsigned short *v	=	(unsigned	short*)dst;
						*v = hex;
						dst	+= sizeof(unsigned short);
					}
					break;
				case AT_HEX4:
					if (1)
					{
						unsigned int hex = GetHEX4(source, &source);
						unsigned int *v	=	(unsigned	int*)dst;
						*v = hex;
						dst	+= sizeof(unsigned int);
					}
					break;
			}
		}
	}

	return dest;
}


void *Asc2Bin(const	char *source,	int	&count,	const	char *spec)
{
	char *dest = 0;

	count	=	0;


	int	cnt	=	0;
	int	size = 0;

	Atype	types[MAXARG];

	const	char *ctype	=	spec;

	while	(*ctype)
	{
		switch (ToLower(*ctype))
		{
			case 'f':
				size +=	sizeof(float);
				types[cnt] = AT_FLOAT;
				cnt++;
				break;
			case 'd':
				size +=	sizeof(int);
				types[cnt] = AT_INT;
				cnt++;
				break;
			case 'c':
				size +=	sizeof(char);
				types[cnt] = AT_CHAR;
				cnt++;
				break;
			case 'b':
				size +=	sizeof(char);
				types[cnt] = AT_BYTE;
				cnt++;
				break;
			case 'h':
				size +=	sizeof(short);
				types[cnt] = AT_SHORT;
				cnt++;
				break;
			case 'p':
				size +=	sizeof(const char*);
				types[cnt] = AT_STR;
				cnt++;
				break;
			case 'x':
				if (1)
				{
					Atype	type = AT_HEX4;
					int	sz = 4;
					switch (ctype[1])
					{
					case '1':
						type = AT_HEX1;
						sz = 1;
						ctype++;
						break;
					case '2':
						type = AT_HEX2;
						sz = 2;
						ctype++;
						break;
					case '4':
						type = AT_HEX4;
						sz = 4;
						ctype++;
						break;
					}
					types[cnt] = type;
					size +=	sz;
					cnt++;
				}
				break;
		}
		if (cnt	== MAXARG)
		{
			return 0;
		}
		// over	flowed the maximum specification!
		ctype++;
	}

	int	reserve_count	=	16;

	dest = new char[reserve_count	*size];
	memset(dest, 0,	reserve_count	*size);	// zero	out	memory

	char *dst	=	(char*)dest; //	where	we are storing the results

	while	(1)
	{

		for	(int j = 0;	j	<	cnt; j++)
		{
			source = SkipWhitespace(source); //	skip white spaces.

			if (*source	== 0)
			// we	hit	the	end	of the input data	before we	successfully parsed	all	input!
			{
				return dest;
			}

			switch (types[j])
			{
				case AT_FLOAT:
					if (1)
					{
						float	*v = (float*)dst;
						*v = GetFloatValue(source, &source);
						dst	+= sizeof(float);
					}
					break;
				case AT_INT:
					if (1)
					{
						int	*v = (int*)dst;
						*v = GetIntValue(source, &source);
						dst	+= sizeof(int);
					}
					break;
				case AT_CHAR:
					if (1)
					{
						*dst++ =	*source++;
					}
					break;
				case AT_BYTE:
					if (1)
					{
						char *v	=	(char*)dst;
						*v = GetIntValue(source, &source);
						dst	+= sizeof(char);
					}
					break;
				case AT_SHORT:
					if (1)
					{
						short	*v = (short*)dst;
						*v = GetIntValue(source, &source);
						dst	+= sizeof(short);
					}
					break;
				case AT_STR:
					if (1)
					{
						const	char **ptr = (const	char **)dst;
						*ptr = source;
						dst	+= sizeof(const	char*);
						while	(*source &&	!IsWhitespace(*source))
						{
							source++;
						}
					}
					break;
				case AT_HEX1:
					if (1)
					{
						unsigned int hex = GetHEX1(source, &source);
						unsigned char	*v = (unsigned char*)dst;
						*v = hex;
						dst	+= sizeof(unsigned char);
					}
					break;
				case AT_HEX2:
					if (1)
					{
						unsigned int hex = GetHEX2(source, &source);
						unsigned short *v	=	(unsigned	short*)dst;
						*v = hex;
						dst	+= sizeof(unsigned short);
					}
					break;
				case AT_HEX4:
					if (1)
					{
						unsigned int hex = GetHEX4(source, &source);
						unsigned int *v	=	(unsigned	int*)dst;
						*v = hex;
						dst	+= sizeof(unsigned int);
					}
					break;
			}
		}
		count++;

		if (count	>= reserve_count)
		{
			char *old_dest = (char*)dest;
			reserve_count	*= 2;
			dest = new char[reserve_count	*size];
			memset(dest, 0,	reserve_count	*size);	// zero	out	memory
			memcpy(dest, old_dest, reserve_count / 2 * size);	// copy	the	old	data.

			int	dist = (int)(dst - old_dest);
			dst	=	&dest[dist]; //	the	new	destination.

			delete old_dest; //	free up	the	old	data.

		}

	}

	return dest;
}

};
