#include <stdio.h>
#define nil NULL
#define nul '\0'

int
fib(n)
{
	int t = 0, j = 1, i = 1;
	while(n >= 2)
	{
		t = i;
		i += j;
		j = t;
		n--;
	}
	return i;
}
int 
main()
{
	int i = 0;
	while(i < 21)
	{
		printf("fib(%d) = %d\n",i,fib(i));
		i++;
	}
}
