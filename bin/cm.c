#include <stdio.h>

int
main()
{
	int op = 0, cp = 0, oc = 0, cc = 0, ob = 0, cb = 0, line = 0, c = 0, dq = 0, aq = 0;
	while(1)
	{
		c = getc(stdin);
		if(feof(stdin))
			break;
		switch(c)
		{
			case '{':
				oc++;
				break;
			case '}':
				cc++;
				break;
			case '[':
				ob++;
				break;
			case ']':
				cb++;
				break;
			case '(':
				op++;
				break;
			case ')':
				cp++;
				break;
			case '\n':
				line++;
				break;
			case '\'':
				aq++;
				break;
			case '"':
				dq++;
			default:
				break;
		}
	}
	printf("{:%d\n}:%d\n[:%d\n]:%d\n(:%d\n):%d\nlines:%d\n",oc,cc,ob,cb,op,cp,line);
	printf("apostrophe parity: %d\nquote parity: %d\n", (aq % 2), (dq % 2));
	return 0;
}
