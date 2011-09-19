#include <stdio.h>

int
main()
{
    long *f;
    long g = f;
    long *h;
    ((long) h ) = (0xC0) + 3;
    printf("%lx\n",g);
    g = h;
    printf("%lx\n",g);
    return 0;
}

