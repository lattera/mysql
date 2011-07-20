#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *xmalloc(size_t sz)
{
	void *p;

	p = malloc(sz);
	if ((p))
		memset(p, 0x00, sz);

	return p;
}
