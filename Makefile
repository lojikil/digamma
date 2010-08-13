all: 
	@echo Buliding Vesta...
	@cd ./c && make
	@echo Done
	@echo -n Moving ./c/vesta ./bin/vesta...
	@mv ./c/vesta ./bin/vesta
	@echo " Completed."
clean:
	@echo Removing object files...
	@cd ./c && make clean
clobber:
	@echo Removing all files
	@cd ./c && make clobber
	@rm -f ./bin/vesta
	@echo "Done."
