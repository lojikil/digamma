all: 
	@echo Buliding Vesta...
	@cd ./c && make
	@echo Done
	@echo -n Moving ./c/vesta ./bin/vesta...
	@mv ./c/vesta ./bin/vesta
	@echo " Completed."
rt: 
	@echo Buliding Runtime...
	@cd ./c && make rt
	@echo Done
	@echo " Completed."
clean:
	@echo Removing object files...
	@cd ./c && make clean
clobber:
	@echo Removing all files
	@cd ./c && make clobber
	@rm -f ./bin/vesta
	@echo "Done."
init:
	@echo -n Initalizing Digamma environment in ~
	@[ ! -d ~/.digamma ] && mkdir ~/.digamma
	@[ ! -d ~/.digamma/lib ] && mkdir ~/.digamma/lib
	@[ ! -d ~/.digamma/anaxagoras ] && mkdir -p ~/.digamma/anaxagoras/notes
	@[ ! -d ~/.digamma/bin ] && mkdir ~/.digamma/bin
	@echo "Done."
install:
	@echo -n "Installing files... "
	@[ -d ~/bin ] && cp bin/vesta ~/bin
	@[ -d ~/.digamma ] && cp scheme/prelude.ss ~/.digamma/prelude.ss
	@echo "Done."
