#include <xinu.h>

status pipconnect(did32 devpipe , pid32 writer, pid32 reader) {
  intmask mask;
  mask = disable();

  wait(con_sem);

  struct pipe_t * p = &pipe_tables[devpipe-PIPELINE0];

  if (p->state != PIPE_USED || reader == writer) {
    signal(con_sem);
    restore(mask);
    return SYSERR;
  }

  // Set reader and writer
  p->read_proc = reader;
  p->write_proc = writer;

  // Save old in/out
  p->old_read = proctab[reader].prdesc[0];
  p->old_write = proctab[writer].prdesc[1];

  // Connect stdin and stdout of processes to pipe
  proctab[reader].prdesc[0] = devpipe;
  proctab[writer].prdesc[1] = devpipe;

  p->state = PIPE_CONNECTED;

  signal(con_sem);
  restore(mask);
	return OK;
}
