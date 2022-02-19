
#include "y_stdlib.h"

void* calloc(size_t nelem, size_t elsize)
{
  void* ptr;  
  if (nelem == 0 || elsize == 0)
    nelem = elsize = 1;
  
  ptr = malloc (nelem * elsize);
  if (ptr) bzero (ptr, nelem * elsize);
  
  return ptr;
}