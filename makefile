.PHONY: all compile install remove clean

all: compile install
	
compile:
	gcc main.c -o png2xga -lm
	
install:
	sudo cp png2xga /bin

remove:
	sudo rm -f /bin/png2xga
	
clean:
	rm xga-viewer
