#pragma once

#define MAX_VALUE_12BIT 4084 // nice range!
#define MIN_VALUE_12BIT 8

#define VECTORDEFAULTINITSIZE 4
#ifdef POLYRENDER
#define VOICESPERCHIP 4
#endif

#ifdef POLYCONTROL
#define VOICESPERCHIP 8
#endif
