/* A hacked version of ctak, that uses tagged pointers;
 * this is a basic test of tagged pointers as a replacement
 * runtime mechanism for Vesta's current descriminated unions.
 * I wouldn't replace all of Vesta, but I would make a minimal 
 * runtime that could be used for E'/Enyo & the like. It would
 * also be useful with Ceres: compile Ceres with Enyo, run it
 * against this minimal runtime.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <gc.h>

#define nil NULL

typedef long long SExp;

#define MK_F_TAG(num) ((num) + 0xC0)

#define STRUE MK_F_TAG(0)
#define SFALSE MK_F_TAG(1)
#define SSUCC MK_F_TAG(2)
#define SUNSUCC MK_F_TAG(3)
#define SNIL MK_F_TAG(4)
#define SSVOID MK_F_TAG(5)

#define hmalloc GC_MALLOC

#define makeinteger(x) (SExp)((intptr_t)(x) << 32) + 0xC7
#define AINT(x) ((SExp)(x) >> 32)

SExp *tak(SExp *, SExp *, SExp *);
SExp *scheme_main();

SExp *
fprimsub(SExp *a, SExp *b)
{
    /* there really should be an INTP check here
     * but for this test this should suffice
     */
    int p = AINT(a) - AINT(b);
    return (SExp *)makeinteger(p);
}

SExp *
fprimgt(SExp *a, SExp *b)
{
    if(AINT(a) > AINT(b))
        return STRUE;
    return SFALSE;
}

SExp *
tak(SExp *x,SExp *y,SExp *z)
{
	SExp *ret = nil, *x2 = nil,*y3 = nil,*z4 = nil;
    SExp *co119 = makeinteger(1);
    printf("In tak; x == %d, y == %d, z == %d\n",AINT(x), AINT(y), AINT(z));
	int s1 = 1;
	while(s1)
	{
        ret = SNIL;
        SExp *it5 = fprimgt(x,y);
		if(it5 == nil || it5 == STRUE)
        {
            x2 = tak(fprimsub(x,co119),y,z);
            y3 = tak(fprimsub(y,co119),z,x);
            z4 = tak(fprimsub(z,co119),x,y);
            x = x2;
            y = y3;
            z = z4;
        }
        else
        {
                s1 = 0;
                ret = y;
        }

	}
	return ret;
}
SExp *
fprimprinc(SExp *s)
{
    printf("%d",AINT(s));
}
SExp *
scheme_main()
{
	SExp *ret = nil;
	ret = fprimprinc(tak(makeinteger(18),makeinteger(12),makeinteger(6)));
    printf("\n");
	return ret;
}

int
main()
{
    GC_INIT();
    SExp *f = makeinteger(18);
    printf("%d\n",AINT(f));
    scheme_main();
    return 0;
}
