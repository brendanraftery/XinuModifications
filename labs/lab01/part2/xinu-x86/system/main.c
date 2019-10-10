/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

int prch(char);
int convoysem;

process	main(void)
{

  kprintf("Main Start\n");
  convoysem = semcreate(1);
  pid32 prA, prB, prC;
  prA = create_lst(prch, 2000, 20, sysclk+60, "pA", 1, 'A');
  prB = create_lst(prch, 2000, 17, sysclk+197, "pB", 1, 'B');
  prC = create_lst(prch, 2000, 15, sysclk+55, "pC", 1, 'C');
  resume(prA);
  resume(prB);
  resume(prC);

  sleepms(400);
  kill(prA);
  kill(prB);
  kill(prC);

  kprintf("\n\n YEET!\n");
/*
	kprintf("\nMain...creating a shell\n");
	recvclr();
	resume(create_lst(shell, 8192, 0, sysclk + 1000000, "shell", 1, CONSOLE));


	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create_lst(shell, 4096, 0, sysclk + 1000000, "shell", 1, CONSOLE));
	}*/
	return OK;
}


int prch(char c) {
  int i;
  while (1) {
    for (i = 0; i < 1000; i++);
    kprintf("%c", c);
  }
    return 1;
}
