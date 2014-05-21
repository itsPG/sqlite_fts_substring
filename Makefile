FLAGS = -DSQLITE_ENABLE_FTS3
HEADERS = $(wildcard *.h)
all: main

main: test.o sqlite3.o character_tokenizer.o
	g++ $(FLAGS) -o $@ $^

clean:
	rm main *.o example.db

%.o: %.cpp $(HEADERS)
	g++ $(FLAGS) -c -o $@ $<

%.o: %.c $(HEADERS)
	gcc $(FLAGS) -c -o $@ $<


