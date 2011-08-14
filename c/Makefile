CC=cc
#LIBPATH=-L/home/jis/resdev/sedwards/opt/lib
#INCPATH=-I/home/jis/resdev/sedwards/opt/include
OBJS=vesta.o ffi.o posix.o main.o eval.o
LIBPATH=
INCPATH=
LIBS=-lgc -lm
CFLAGS=-I/usr/local/include -L/usr/local/lib -DNEED_LOG2
IMAGE=vesta
all: $(OBJS)
	@echo -n "Linking image..."
	@cc -o vesta -L/usr/local/lib $(OBJS) -lgc -lm
	@echo "Done."
rt: $(OBJS)
	@cc $(CLFAGS) -DRT_ONLY -c vesta.c -o vesta.o
%.o: %.c
	@echo -n Compiling $<
	@cc $(CFLAGS) -c $< -o $@
	@echo " Done."
clean:
	@echo -n "Removing object & core files..."
	@rm -f $(OBJS) *.core
	@echo " Done."
clobber: clean
	@echo -n "Removing IMAGE..."
	@rm -f $(IMAGE)
	@echo " Done."
