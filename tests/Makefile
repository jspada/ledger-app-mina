all: utils_tests random_oracle_input_tests emulator_tests

utils_tests: utils.o utils_tests.c
	@echo "Running utils tests..."
	$(CC) -Wall -Werror -I ../src utils_tests.c -o $@ utils.o -lm
	./$@

random_oracle_input_tests: utils.o random_oracle_input.o transaction.o random_oracle_input_tests.c
	@echo "Running random oracle input tests..."
	@$(CC) -Wall -Werror -I../src -lm -o $@ \
	                            random_oracle_input_tests.c \
	                            utils.o \
	                            random_oracle_input.o \
	                            transaction.o
	./$@

utils.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/utils.c -c

random_oracle_input.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/random_oracle_input.c -c

transaction.o: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(CC) -Wall -Werror -I ../src ../src/transaction.c -c

define EMULATOR_TESTS_SCRIPT
echo "Starting emulator"
./speculos/speculos.py --display headless \
                       --automation file:./emulator_automation.json \
                       -s 'course grief vintage slim tell hospital car maze model style elegant kitchen state purpose matrix gas grid enable frown road goddess glove canyon key' \
                       ../bin/app.elf > emulator_tests.log 2>&1 &
PID1=$$!
sleep 1
LEDGER_PROXY_ADDRESS=127.0.0.1 LEDGER_PROXY_PORT=9999 ./unit_tests.py
echo "Stopping emulator"
kill $${PID1}
endef

export EMULATOR_TESTS_SCRIPT
emulator_tests: speculos/build/src/launcher emulator_automation.json $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	@echo "$$EMULATOR_TESTS_SCRIPT" > $@
	@chmod +x $@
ifeq ("$(SKIP_EMULATOR_TESTS)","")
	@./$@
endif

speculos/build/src/launcher: speculos
ifeq ("$(SKIP_EMULATOR_TESTS)","")
	cd $<; cmake -Bbuild -H.
	$(MAKE) -C $</build
endif

clean:
	rm -rf *.o *.log utils_tests random_oracle_input_tests emulator_tests speculos/build
