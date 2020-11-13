all: utils_tests

utils_tests: utils.o utils_tests.c
	@echo "Running unit tests..."
	gcc -I ../src utils_tests.c -lm -o utils_tests utils.o
	./utils_tests

utils.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	gcc -I ../src ../src/utils.c -c

clean:
	rm -f utils_tests
