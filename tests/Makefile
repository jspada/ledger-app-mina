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

ifneq (,$(wildcard ../emulator.pid))
EMULATOR_PID=`cat ../emulator.pid`
endif
# stop_emulator:
# ifneq (,$(wildcard ../emulator.pid))
# 	@echo "Stopping emulator with PID $(EMULATOR_PID)"
# 	@kill $(EMULATOR_PID) >/dev/null 2>&1 || true
# 	@rm -f ../emulator.pid
# endif

TEST_MNEMONIC=course grief vintage slim tell hospital car maze model style \
              elegant kitchen state purpose matrix gas grid enable frown road \
			  goddess glove canyon key

define EMULATOR_TESTS_SCRIPT
echo "Starting emulator"
../dev-env/speculos/speculos.py --display headless \
                       --automation file:../emulator_automation.json \
                       -s "$(TEST_MNEMONIC)" \
                       ../bin/app.elf > emulator_tests.log 2>&1 &
PID1=$$!
echo $$PID1 > ../emulator.pid
sleep 1
LEDGER_PROXY_ADDRESS=127.0.0.1 LEDGER_PROXY_PORT=9999 ./unit_tests.py || (rm -f ./emulator_tests && exit 211)
echo "Stopping emulator on pid $$PID1"
kill $${PID1} > /dev/null 2>&1 || true
endef

export EMULATOR_TESTS_SCRIPT
emulator_tests: ../bin/app.elf ../dev-env/speculos/build/src/launcher ../emulator_automation.json
	@echo "$$EMULATOR_TESTS_SCRIPT" > $@
	@chmod +x $@
ifeq ("$(NO_EMULATOR)","")
ifneq (,$(wildcard ../emulator.pid))
	@echo "Stopping emulator with PID $(EMULATOR_PID)"
	@kill $(EMULATOR_PID) >/dev/null 2>&1 || true
	@rm -f ../emulator.pid
endif
	@./$@
endif

../bin/app.elf: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(MAKE) -C .. default

../dev-env/speculos/build/src/launcher: ../dev-env/speculos
ifeq ("$(NO_EMULATOR)","")
	cd $<; cmake -Bbuild -H.
	$(MAKE) -C $</build
endif

clean:
	rm -rf *.o *.log utils_tests random_oracle_input_tests emulator_tests
