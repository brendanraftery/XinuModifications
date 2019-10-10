#include <xinu.h>

did32 pipcreate() {
  int i = 0;
  intmask mask;
  mask = disable();

  wait(create_sem);
  for (i = 0; i < MAXPIPES; i++) {
//  kprintf("\nChecking pipe %d\n", i);
    if (pipe_tables[i].state == PIPE_FREE) {
      struct pipe_t * p = &pipe_tables[i];
      p->pipid = i;
      p->owner = getpid();
      p->device = i + PIPELINE0;
      p->state = PIPE_USED;
      p->start = 0;
      p->end = 0;
      p->cnt = 0;
      // Init semaphores (prod/con)
      p->count_sem = semcreate(0);
      p->space_sem = semcreate(PIPE_SIZE);
      restore(mask);
      signal(create_sem);
      return p->device;
    }  
  }
  signal(create_sem);
  restore(mask);
  return SYSERR;
}


