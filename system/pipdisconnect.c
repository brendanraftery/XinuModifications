#include <xinu.h>

status pipdisconnect(did32 devpipe) {

  intmask mask;
  mask = disable();

  wait(dc_sem);  
  struct pipe_t * p = &pipe_tables[devpipe-PIPELINE0];

  if (p->state != PIPE_CONNECTED) {
    restore(mask);
    signal(dc_sem);
    return SYSERR;
  }
 
  // Connect to old in/out
  if (getpid() == p->read_proc) {
    proctab[p->read_proc].prdesc[0] = p->old_read;
    p->read_proc = -1;
    if (p->state != PIPE_WDC) {
      while (pipgetc(devpipe) != SYSERR);

      p->state = PIPE_RDC;
    }
    else
      p->state = PIPE_OTHER;
  }
  if (getpid() == p->write_proc) {
    proctab[p->write_proc].prdesc[1] = p->old_write;
    p->write_proc = -1;
    if (p->state != PIPE_RDC)
      p->state = PIPE_WDC;
    else
      p->state = PIPE_OTHER;
  } 

  signal(dc_sem);
  restore(mask);
	return OK;
}
