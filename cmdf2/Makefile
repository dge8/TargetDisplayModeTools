CXXFLAGS = -Ilib/Karabiner-VirtualHIDDevice/dist/include -std=c++14 -Wall -Werror -O2

all: main.o
	c++ -framework IOKit main.o -o cmdf2

install:
	cp cmdf2 /usr/local/bin/cmdf2

clean:
	rm -f *.o
	rm -f cmdf2

run: all
	sudo ./cmdf2
