#include <stdio.h>

int
fib(int n)
{
	int i = 0, j = 1, zi = 0;
	while(n > 0)
	{
            zi = i;
            i += j;
            j = zi;
            n--;
        }
        return i;
}
int
main()
{
    printf("%d\n",fib(10));
    printf("%d\n",fib(20));
    printf("%d\n",fib(30));
    printf("%d\n",fib(32));
    return 0;
}
