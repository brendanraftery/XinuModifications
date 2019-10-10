/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;
void printReady();

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
  struct procent *mainpr;

  mainpr = &proctab[mainpid];
  if (mainpid == currpid && mainpr->prparent == 0 && (mainpr->prstate == PR_CURR || mainpr->prstate == PR_READY)) {
    lastres = sysclk;
    mainpr->prstate = PR_CURR;
    return;
  }
 
  struct procent *cp = &proctab[currpid];
  if ((cp->prstate == PR_CURR || cp->prstate == PR_READY) && !(cp->prdl < sysclk || cp->prextime < 0 || cp->prslack < 0)) {
    if (isempty(readylist) && (qnum < 5 || isempty(lowprio))) {    
      qnum++;
      lastres = sysclk;
      cp->prstate = PR_CURR;
      return;
    }
  } 

  if ((qnum == 5 || isempty(readylist)) && nonempty(lowprio)) { // Give time to delinq proc
    qnum = 1;
    ptold = &proctab[currpid];
    // Check if curr is a violater
    if (ptold->prslack < 0 || ptold->violate == 1 || ptold->prextime < 0 || ptold->prdl < sysclk) {
      if (currpid != 0) {
        ptold->violate = 1;
//        kprintf("VIOLATOR e: %d\n", currpid);
        enqueue(currpid, lowprio);
      }
    } else if (currpid != 1) {
      insert(currpid, readylist, ptold->prslack);
    }
    pid32 dlq = dequeue(lowprio);
    ptnew = &proctab[dlq];
    
    if (currpid == dlq) {
      qnum = 1;
      return;
    }

    currpid = dlq;
    ptold->prstate = PR_READY;
    //getitem(currpid);
    ptnew->prstate = PR_CURR;
    preempt = QUANTUM;
    lastres = sysclk;
    qnum = 1;
    ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
    return;         
  }

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];

  // Prioritize main
  /*
  if (strcmp(ptold->prname, "Main process") == 0 && (ptold->prstate == PR_CURR)) {
    return;
  } 
  */

  ptold->prextime = ptold->prextime - (sysclk-lastres); 
  ptold->prslack = ptold->prdl - (sysclk + ptold->prextime); 


  if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		ptold->prstate = PR_READY;
    if (ptold->prdl > sysclk && ptold->prextime > 0 && ptold->prslack > 0)
  		insert(currpid, readylist, ptold->prslack);
    else {
  //    if (currpid != 0) kprintf("\n%s VIOLATED\n", ptold->prname);
      if (currpid != 0) enqueue(currpid, lowprio);
      if (currpid != 0) ptold->violate = 1;
    }
	}

  int32 smallest = getSmallestSlack(); 


  if (smallest == currpid) {
    lastres = sysclk;
    qnum++;
    return;
  }
  
  currpid = smallest;
  ptold->prstate = PR_READY;
  getitem(currpid);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
  lastres = sysclk;
  qnum++;
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	return;
}



void printReady() {

  if (firstid(lowprio) == queuetail(lowprio))
    return;

  kprintf("\nLOWPRIO : {");
  int16 curr = firstid(lowprio);
  while (curr != queuetail(lowprio) && curr >= 0) {
    kprintf(" %d", curr);
    curr = queuetab[curr].qnext;
  }
  kprintf(" }\n");
}

/*-----------------------------------------------------------------------
 *  getSmallestSlack  -  Find pid with smallest slack time, update slack
 *  times of all pids in readylist, and remove violating processes from
 *  readylist and place them into the violating list "lowprio"
 *-----------------------------------------------------------------------
 */

pid32 getSmallestSlack() {
//  kprintf("\n-----Finding Smallest Slack-----\n");
  int32 s = -1;
  struct procent *currproc;
  int16 smallestpid = 0;
  int16 curr = firstid(readylist);
  int32 smallestslack = (&proctab[curr])->prslack;
  int32 smallct = -1;  

  while (curr != queuetail(readylist)) {
    currproc = &proctab[curr];
    uint32 curdl = currproc->prdl;
    uint32 crct = currproc->prcrtime;
    uint32 curet = currproc->prextime;
    s = curdl - sysclk - curet;

    if ((sysclk > curdl || curet < 0 || s < 0) && (strcmp("prnull", currproc->prname) != 0 && strcmp("Main process", currproc->prname) != 0)) { // Violating process
      int16 tcurr = queuetab[curr].qnext; // Temp
      getitem(curr); // removes violating process from readylist
//      kprintf("VIOLATOR ! : %s\n", currproc->prname);
      if (curr != 0)
        enqueue((pid32) curr, lowprio);
      curr = tcurr;
      currproc->prslack = s;
      continue;
    }

    if (curr == 0) {
      curr = queuetab[curr].qnext;
      continue;
    }

    if (s < smallestslack || smallestslack == -1) { // New smallest slack found in RL
      smallestslack = s;
      smallestpid = curr;
      smallct = crct;
    } else if (s == smallestslack) { // If tie, give priority to earliest created
      if (crct < smallct) {
        smallestpid = curr;
        smallct = crct;
      }
    }

    curr = queuetab[curr].qnext;
    currproc->prslack = s;
  } // end while
// kprintf("\nSMALLEST SLACK : %d\n------------------------------\n", smallestpid); 
  return smallestpid;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
