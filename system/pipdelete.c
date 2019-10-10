#include <xinu.h>

status pipdelete(did32 devpipe) {
  intmask mask;
  mask = disable();

  wait(del_sem);
  struct pipe_t * p = &pipe_tables[devpipe-PIPELINE0];
  if (p->device == devpipe && p->owner == getpid()) {
      p->owner = -1;
      p->state = PIPE_FREE;
      p->start = 0;
      p->end = 0;
      p->cnt = 0;
      sleepms(1);
      semdelete(p->count_sem);
      semdelete(p->space_sem);

      restore(mask);
      signal(del_sem);
      return OK;
  }

  signal(del_sem);
  restore(mask);
  return SYSERR;
}
