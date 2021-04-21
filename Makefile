
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence

correctness: kvstore.o correctness.o SkipList.o SSTable.o DiskManager.o global.o LevelStorage.o MergeBuffer.o

persistence: kvstore.o persistence.o SkipList.o SSTable.o DiskManager.o global.o LevelStorage.o MergeBuffer.o

mytest: kvstore.o main.o SkipList.o SSTable.o DiskManager.o global.o LevelStorage.o MergeBuffer.o

clean:
	-rm -f correctness persistence *.o
