/* create_lst.c */

#include <xinu.h>

local int newpid();

#define roundew(x) ( (x+3)& ~0x3)


pid32 create_lst(
    void *funcaddr,
    uint32 ssize,
    uint32 exec_time,
    uint32 deadline,
    char * name,
    uint32 nargs,
    ...
  )
{
	uint32		savsp, *pushsp;
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/

  kprintf("Creating new process : %s\n", name);

	mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundew(ssize);
	if (((saddr = (uint32 *)getstk(ssize)) ==
	    (uint32 *)SYSERR ) || (pid=newpid()) == SYSERR) {
		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];

  // Check for issues with times

  if (strcmp(name, "prnull") == 0) {
    kprintf("Creating NULL PROC\n");
  }

	/* initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* initial state is suspended	*/
	prptr->prprio = 1;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

  // NEW DATA
  prptr->prdl = deadline;
  prptr->prextime = exec_time;
  prptr->prcrtime = sysclk;
  prptr->prslack = deadline - sysclk - exec_time;
  prptr->prprio = 1;
  prptr->violate = 0;

  /* Init stack as if the process was called */
  *saddr = STACKMAGIC;
  savsp = (uint32)saddr;

  kprintf("-------PROCESS CREATED--------\nName : %s (%d)\n", prptr->prname, pid);

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
		*--saddr = *a--;	/*   onto created process' stack*/
	*--saddr = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registerss, and an old SP	*/

	*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	*--saddr = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	*--saddr = 0;			/* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
	restore(mask);
  kprintf("PID RETURNED : %d\n", pid);
	return pid;
}

local pid32 newpid(void)
{
  uint32 i; /* Iterate through all processes */
  static pid32 nextpid = 1;

  for (i = 0; i < NPROC; i++) {
    nextpid %= NPROC; // Wrap pids
    if (proctab[nextpid].prstate == PR_FREE) {
      return nextpid++;
    } else {
      nextpid++;
    }
  }
  return (pid32) SYSERR;
}
