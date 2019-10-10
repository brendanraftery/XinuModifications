#include <xinu.h>

uint32 pipwrite(struct dentry *devptr, char* buf, uint32 len) {
  did32 devid = devptr->dvnum;
//  struct pipe_t * p = &pipe_tables[devid-PIPELINE0];
  int i;
  for ( i = 0 ; i < len; i++) {
    pipputc(devptr, buf[i]);
  }

  return OK;
}
