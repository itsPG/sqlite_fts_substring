FLAGS = -DSQLITE_ENABLE_FTS3 -pthread -ldl
ifeq ($(shell uname), Darwin)
FLAGS = -DSQLITE_ENABLE_FTS3
endif

HEADERS = $(wildcard *.h)

all: main sqlite3commandTest

main: test.o sqlite3.o character_tokenizer.o
	g++ $(FLAGS) -o $@ $^

sqlite3commandTest: test_sqlite3command.o sqlite3command.o sqlite3.o character_tokenizer.o
	g++ $(FLAGS) -o $@ $^

clean:
	rm -f main *.o example.db

%.o: %.cpp $(HEADERS)
	g++ $(FLAGS) -c -o $@ $<

%.o: %.c $(HEADERS)
	gcc $(FLAGS) -c -o $@ $<


