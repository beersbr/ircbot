CC = g++
LL = -O1

BOOST_INCLUDES = -I /Users/bbeers/projects/boost_1_48_0/
BOOST_REGEX = -L./boost/
BOOST_REGEX_A = ./boost/libboost_regex-gcc-1_47.a

all: main.o colors.h
	$(CC) -o bot main.o colors.h $(LL) $(BOOST_REGEX) $(BOOST_REGEX_A) $(BOOST_INCLUDES)

main.o: main.cpp
	$(CC) -c main.cpp $(LL) $(BOOST_INCLUDES)

clean:
	rm *.o
	rm *.gc*