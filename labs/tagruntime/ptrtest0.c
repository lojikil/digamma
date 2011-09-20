#include <stdio.h>

int
main()
{
    long *f;
    long g = f;
    long *h;
    ((long) h ) = (0xC0) + 3 + ((long)18l << 8);
    printf("%lx\n",g);
    g = h;
    printf("%lx\n",g);
    printf("%d\n",g >> 8);
    return 0;
}

