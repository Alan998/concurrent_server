# main file to make all files

all:
	make -C src/cpp
	make -C src/go

.PHONY: clean format
clean:
	make -C src/cpp clean
	make -C src/go clean

format:
	make -C src/cpp format
	make -C src/go format
