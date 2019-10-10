/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

void prch(char);

process	main(void)
{
  
  pid32 prA, prB, prC;
  prA = create(prch, 2000, 1, "pA", 1, 'A');
  prB = create(prch, 2000, 5, "pB", 1, 'B');
  prC = create(prch, 2000, 200, "pC", 1, 'C');

  resume(prA);
  resume(prB);
  resume(prC);

  sleep(1);
  kill(prA);
  kill(prB); 
  kill(prC);

  kprintf("\n\n YEET!\n"); 
	/* Wait for shell to exit and recreate it */
  /*
	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}*/
	return OK;
}

void prch(char c) {
  while (1) {
    int i;
    for (i = 0; i < 2000; i++) {
      kprintf("%c", c);
    }
  }
}
