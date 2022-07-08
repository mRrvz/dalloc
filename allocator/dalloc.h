#ifndef __DALLOC_H__
#define __DALLOC_H__

#include <stdlib.h>

void *dalloc(size_t cnt);

void dfree(void *mem);

#endif