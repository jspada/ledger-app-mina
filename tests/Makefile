all: utils_tests random_oracle_input_tests

utils_tests: utils.o utils_tests.c
	@echo "Running utils tests..."
	$(CC) -Wall -Werror -I ../src utils_tests.c -o utils_tests utils.o -lm
	./utils_tests

random_oracle_input_tests: utils.o random_oracle_input.o transaction.o random_oracle_input_tests.c
		@echo "Running random oracle input tests..."
		@$(CC) -Wall -Werror -I../src -lm -o random_oracle_input_tests \
		                             random_oracle_input_tests.c \
									 utils.o \
		                             random_oracle_input.o \
									 transaction.o
		./random_oracle_input_tests

utils.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/utils.c -c

random_oracle_input.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/random_oracle_input.c -c

transaction.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/transaction.c -c

clean:
	rm -f *.o utils_tests roinput_tests
