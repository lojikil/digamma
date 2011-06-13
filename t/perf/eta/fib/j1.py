def fib(n):
    i,j = 0,1
    while n > 0:
        zi = i 
        i += j 
        j = zi
        n -= 1
    return i
print fib(10)
print fib(20)
print fib(30)
print fib(32)
