all: utils_tests

utils_tests: utils.o utils_tests.c
	@echo "Running unit tests..."
	$(CC) -Wall -Werror -I ../src utils_tests.c -o utils_tests utils.o -lm
	./utils_tests

utils.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/utils.c -c

clean:
	rm -f utils.o utils_tests
