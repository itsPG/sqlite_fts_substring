FLAGS = -DSQLITE_ENABLE_FTS3 -pthread -ldl
ifeq ($(shell uname), Darwin)
FLAGS = -DSQLITE_ENABLE_FTS3
endif

HEADERS = $(wildcard *.h)
CXX = g++
CC = gcc

all: main sqlite3commandTest

main: test.o sqlite3.o character_tokenizer.o
	$(CXX) $(FLAGS) -o $@ $^

sqlite3commandTest: test_sqlite3command.o sqlite3command.o sqlite3.o character_tokenizer.o
	$(CXX) $(FLAGS) -o $@ $^

clean:
	rm -f main *.o example.db sqlite3commandTest

%.o: %.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c -o $@ $<

%.o: %.c $(HEADERS)
	$(CC) $(FLAGS) -c -o $@ $<


