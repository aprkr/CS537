#include "spinlock.h"
typedef struct {
  // Lock state, ownership, etc.
  uint locked;
  struct spinlock lk;
  int holderPriority;
  int requesterPriority;
  struct proc *p;
} mutex;
