ifeq ("$(NO_EMULATOR)","")
EMULATOR=1
else
EMULATOR=0
endif

ifeq ("$(EMULATOR_SDK)","")
ifeq ("$(NANO_MODEL)","nanox")
EMULATOR_SDK=1.2
else
EMULATOR_SDK=2.0
endif
endif

ifeq ("$(NO_EMULATOR_TESTS)","")
EMULATOR_TESTS=1
else
EMULATOR_TESTS=0
endif

ifeq ("$(NANO_MODEL)","nanos")
EMULATOR_MODEL=nanos
endif

ifeq ("$(NANO_MODEL)","nanox")
EMULATOR_MODEL=nanox
endif

ifeq ("$(EMULATOR_MODEL)","")
$(error Unknown NANO_MODEL '$(NANO_MODEL)')
endif

ifeq ("$(EMULATOR_MODEL)","")
$(error Environment variable EMULATOR_MODEL is not set (source ./prepare-devenv.sh))
endif

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

ifeq ($(EMULATOR_TESTS),1)
ifeq ("$(EMULATOR_MODEL)","nanox")
$(warning Emulator automation is not supported on the Nano X, reverting to manual unit tests)
EMULATOR_AUTOMATION=
EMULATOR_DISPLAY=
else
EMULATOR_AUTOMATION=--automation file:../emulator_automation.json
EMULATOR_DISPLAY=--display headless
endif
endif

define EMULATOR_TESTS_SCRIPT
echo "Starting $(EMULATOR_MODEL) emulator"
../dev-env/speculos/speculos.py -k $(EMULATOR_SDK) -m $(EMULATOR_MODEL) \
                       $(EMULATOR_DISPLAY) $(EMULATOR_AUTOMATION) \
                       -s "$(TEST_MNEMONIC)" \
                       ../bin/app.elf > emulator_tests.log 2>&1 &
PID1=$$!
echo $$PID1 > ../emulator.pid
sleep 1
LEDGER_PROXY_ADDRESS=127.0.0.1 LEDGER_PROXY_PORT=9999 ./unit_tests.py
RES=$$?
echo "Stopping emulator on pid $$PID1"
kill $${PID1} > /dev/null 2>&1 || true
exit $${RES}
endef

export EMULATOR_TESTS_SCRIPT
emulator_tests: ../bin/app.elf ../dev-env/speculos/build/src/launcher ../emulator_automation.json
ifeq ($(EMULATOR),1)
ifeq ($(EMULATOR_TESTS),1)
	@echo "$$EMULATOR_TESTS_SCRIPT" > $@
	@chmod +x $@
ifneq (,$(wildcard ../emulator.pid))
	@echo "Stopping emulator with PID $(EMULATOR_PID)"
	@kill $(EMULATOR_PID) >/dev/null 2>&1 || true
	@rm -f ../emulator.pid
endif
	@./$@ || (rm -f emulator_tests && exit 211)
endif
endif

../bin/app.elf: $(wildcard ../src/*.h) $(wildcard ../src/*.c)
	$(MAKE) -C .. default

../dev-env/speculos/build/src/launcher: ../dev-env/speculos
ifeq ($(EMULATOR),1)
	cd $<; cmake -Bbuild -H.
	$(MAKE) -C $</build
endif

clean:
	rm -rf *.o *.log utils_tests random_oracle_input_tests emulator_tests
