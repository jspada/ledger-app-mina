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

There are two types of unit tests: those that run as part of the build
and those that can be run on the Ledger device.  This section describes
how to set up and run the on-device unit tests.

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

## Command-line wallet

This package provides a simple command-line wallet that interfaces
with your Ledger device and Mina blockchain to allow you to generate
addresses, sign transaction and submit them to the Mina network.

```bash
$ ./utils/mina-ledger-wallet.py -h
usage: mina-ledger-wallet.py [-h] {get-address,send-payment} ...

positional arguments:
  {get-address,send-payment}

optional arguments:
  -h, --help            show this help message and exit
```

Example of generating a keypair corresponding to hardware wallet account 1 (BIP44 address /44'/49370'/1/0/0)

```bash
$ ./utils/mina-ledger-wallet.py get-address 1
Getting address...
Received: B62qqwtcG43GVR1D1cGEvSuAgkTJwpZHToBqKTMX7oqWmr9Xb5R64tB
```

Example of sending a payment transaction sending 100.5 Mina from hardware wallet account 1 (B62qqwtcG43GVR1D1cGEvSuAgkTJwpZHToBqKTMX7oqWmr9Xb5R64tB) to recipient B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g.

```bash
$ ./utils/mina-ledger-wallet.py send-payment --fee 0.00271828 1 B62qqwtcG43GVR1D1cGEvSuAgkTJwpZHToBqKTMX7oqWmr9Xb5R64tB B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g 100.5
```

## Documentation
This follows the specification available in the [`api.asc`](https://github.com/LedgerHQ/ledger-app-boilerplate/blob/master/doc/api.asc).
