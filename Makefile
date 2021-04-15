
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence

correctness: kvstore.o correctness.o SkipList.o

persistence: kvstore.o persistence.o SkipList.o

clean:
	-rm -f correctness persistence *.o
