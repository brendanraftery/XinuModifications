#include <xinu.h>

devcall pipputc(struct dentry *devptr, char ch) {
    intmask mask;
    mask = disable();
    
    if (devptr == NULL) {
      restore(mask);
      return SYSERR;
    }

    struct pipe_t * p = &pipe_tables[devptr->dvnum-PIPELINE0];
    
    wait(put_sem);
    // Ensure calling process is writer
    if (p->write_proc != getpid()) {
      restore(mask);
      signal(put_sem);
      return SYSERR;
    }

    // Perform send
    wait(p->space_sem);
    p->buffer[p->end] = ch;
//  kprintf("Sent [%c] on %d\n", ch, devptr->dvnum); 
    p->end = (p->end + 1) % PIPE_SIZE;
    p->cnt = p->cnt + 1;
    signal(p->count_sem);

    restore(mask);
    signal(put_sem);
    return 1;
}

