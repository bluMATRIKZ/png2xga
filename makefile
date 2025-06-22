.PHONY: all compile install remove clean

all: compile install
	
compile:
	gcc main.c -o xga-viewer -lm -DSTB_IMAGE_IMPLEMENTATION
	
install:
	sudo cp png2xga /bin

remove:
	sudo rm -f /bin/png2xga
	
clean:
	rm xga-viewer
