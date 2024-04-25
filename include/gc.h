#ifndef GC_H
#define GC_H

#include <stdlib.h>


int gc_init();

void* gc_malloc(size_t size);

void gc_collect();

#endif /* GC_H */