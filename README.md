# ledger-app-mina

## Overview
This is the Mina app for the Nano S/X hardware wallet.

## Building and installing
To build and install the app on your Ledger Nano S you must set up the Ledger Nano S build environments. Please follow the Getting Started instructions at [here](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).

If you don't want to setup a global environnment, you can also setup one just for this app by sourcing `prepare-devenv.sh` with the right target (`s` or `x`).

install prerequisite and switch to a Nano X dev-env:

```bash
sudo apt install python3-venv python3-dev libudev-dev libusb-1.0-0-dev

# (x or s, depending on your device)
source prepare-devenv.sh x 
```

Compile and load the app onto the device:
```bash
make load
```

Refresh the repo (required after Makefile edits):
```bash
make clean
```

Remove the app from the device:
```bash
make delete
```

## Unit tests

1. Initialize your Ledger test hardware with the following secret phrase

```
course grief vintage slim tell hospital car maze model style elegant kitchen state purpose matrix gas grid enable frown road goddess glove canyon key
```

2. Build and install the Mina unit tests app

```bash
make clean
make USER_DEFINES=UNIT_TESTS load
```

3. Start the unit tests app on your Ledger device

This app replaces the normal Mina app and will show

```
Mina
unit tests
```

once loaded.

4. Run the unit tests

```bash
./tests/unit_tests.py
```

## Example of Ledger wallet functionality

Test functionality:
```bash
# (x or s, depending on your device)
source prepare-devenv.sh x
python test_example.py --account_number 12345
```

## Documentation
This follows the specification available in the [`api.asc`](https://github.com/LedgerHQ/ledger-app-boilerplate/blob/master/doc/api.asc).
