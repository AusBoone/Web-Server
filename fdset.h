/* fdset.h
 *
 * Defines a class for managing and selecting from among a set of input 
 * file descripitors when input arrives.
 */

#ifndef _INPUTSET_H
#define _INPUTSET_H


class FDSet {
public:
   /* Creates an empty file descriptor set.  
      maxSize indicates the maximum number of elements in the set. */
  FDSet(int maxSize = 20);
  
   /* Destroys the internal data when the object is destroyed. */;
  ~FDSet();
  
   /* Returns the number of fd's in the set. */
  int
  length() { return Count; }
  
   /* Returns the maximum size of the set. */
  int
  maxSize() { return MaxSize; }
  
   /* Adds a file descriptor to the set. Returns true if the fd was added
      or false if the set is full. */
  bool
  addFD(int fd);
  
   /* Removes the indicated file descriptor from the set. Returns true if 
      the fd was removed, or false if the fd is not in the set. */
  bool
  removeFD(int fd);
  
   /* Clear the set of file descriptors. */
  void
  clear();
  
   /* Stops and waits for input to arrive on at least one of the input
      streams indicated by the file descriptors in the set. When input 
      arrives, the method returns a list of file descriptors, within an 
      array, corresponding to the input streams on which input was detected. 
      The end of the list is indicated by a -1. The pointer returned refers
      to an internal array that should not be deleted. */ 
  int *
  select();
  
private:
  int *ElementList;
  int MaxSize;
  int Count;
  int *ActiveSet;  
};

#endif

