BINARY=hello_world

# compile the binary
all: 
	gcc hello.c -Wall -g -ggdb -static -o $(BINARY) 
	file $(BINARY)

# Clean the binary and the temporarilly objects
clean:
	rm -f hello_world.o
	rm -f hello_world

# run the binary and display the syscall
strace: 
	@echo "-----------------------------------"
	strace ./$(BINARY) | more
	@echo "-----------------------------------"
	strace -c ./$(BINARY) 
	@echo "-----------------------------------"

# dump the symbol
nm:
	nm ./$(BINARY) 

# display the ELF
readelf:
	readelf --syms ./$(BINARY)  

# dump the object ELF binary
objdump: 
	objdump -d ./$(BINARY) | less
