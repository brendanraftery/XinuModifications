#include <xinu.h>

devcall pipgetc(struct dentry *devptr) {
    intmask mask;
    mask = disable();

    struct pipe_t * p = &pipe_tables[devptr->dvnum-PIPELINE0];

//  wait(get_sem);
    if (p->read_proc != getpid()) {
      restore(mask);
      signal(get_sem);
      return SYSERR;
    }
    
    wait(p->count_sem);
    char c = p->buffer[p->start];
//  kprintf("Recv [%c] on %d\n", c, devptr->dvnum);
    p->cnt = p->cnt - 1;
    p->start = (p->start + 1) % PIPE_SIZE; 
    signal(p->space_sem);

    if (p->cnt == 0 && p->state == PIPE_WDC) {
      restore(mask);
      signal(p->space_sem);
      return SYSERR;
    }

/*
    if (p->cnt == 0 && p->write_proc == -1) {
      kprintf("cnt = 0, proc = -1\n");
      restore(mask);
      return SYSERR;
    }
*/
//  signal(get_sem);
    restore(mask);
    return c;
}

