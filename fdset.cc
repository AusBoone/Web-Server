#include "fdset.h"
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

/* An internal data structure used by the select function. */ 
fd_set readSet;


FDSet::FDSet(int maxSize) 
{
  MaxSize = maxSize;
  ElementList = new int[MaxSize];
  Count = 0;
  ActiveSet = new int[MaxSize]; 
}

FDSet::~FDSet()
{
  delete ElementList;
  delete ActiveSet;
}

bool
FDSet::addFD(int fd)
{
   /* Make sure there is room in the set for the fd. */
  if( Count == MaxSize ) 
    return false;
  
   /* Make sure the fd is not already in the set. */
  for( int i = 0; i < Count; i++ )
    if(fd == ElementList[i]) 
      return false;
    
   /* Add the fd. */
  ElementList[Count] = fd;
  Count++;
  return true;
}

bool
FDSet::removeFD(int fd)
{
  for(int i = 0; i < Count; i++)
    if(fd == ElementList[i]) {
      ElementList[i] = ElementList[Count - 1];
      Count--;
      return true;
    }
    
  return false;
}

void
FDSet::clear()
{
  Count = 0;
}

int *
FDSet::select()
{
  timeval tv;
  
  int result;
  while(1) {
    tv.tv_sec = 25;
    tv.tv_usec = 0;
   
     /* Add the fd's to the internal fd_set. */
    FD_ZERO(&readSet);
    int maxFD = 0;
    for(int i = 0; i < Count; i++) {
      FD_SET(ElementList[i], &readSet);
      if(ElementList[i] > maxFD) 
        maxFD = ElementList[i];
    }
      
    result = ::select(maxFD + 1, &readSet, 0, 0, &tv);
    if(result == -1) 
      perror("FDSet::select(): invalid fd set.");
    else if(result) {
      ActiveSet[0] = -1;
      int pos = 0;
      for(int i = 0; i < Count; i++) {
        int fd = ElementList[i];
        if(FD_ISSET(fd, &readSet)) {
          ActiveSet[pos] = fd;
          pos++;
        }
      }
      ActiveSet[pos] = -1;
      return ActiveSet;
    }
  }
}

