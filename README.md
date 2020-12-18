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
source prepare-devenv.sh s
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
$ ./utils/mina_ledger_wallet.py -h
usage: mina_ledger_wallet.py [-h] [--verbose]
                             {get-address,get-balance,send-payment,delegate}
                             ...

positional arguments:
  {get-address,get-balance,send-payment,delegate}

optional arguments:
  -h, --help            show this help message and exit
  --verbose             Verbose mode
```

**Get address**

```bash
$ ./utils/mina_ledger_wallet.py get-address 1
Get address for account 1 (path 44'/12586'/1/0/0)
Continue? (y/N) y
Generating address (please confirm on Ledger device)... done
Received address: B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P
```
This generates the keypair corresponding to hardware wallet account 1 (BIP44 account 44'/12586'/1/0/0) and returns the corresponding Mina address.

**Get balance**

```bash
$ ./utils/mina_ledger_wallet.py get-balance B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx
Getting network identifier... debug
Getting account balance... done

Address: B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx
Balance: 9792.0
```
This queries the Mina blockchain for the balance of address B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx.

**Send payment**

```bash
$ ./utils/mina_ledger_wallet.py send-payment 1 B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g 2.71821
```

This sends a payment of 2.71821 Mina from hardware wallet account 1 (B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P) to recipient B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g.

**Delegate**

```bash
$ ./utils/mina_ledger_wallet.py delegate 1 B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g --memo "Delegation is fun!"
```

This delegates the entire balance of hardware wallet account 1 (B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P) to delegate B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g.

## Documentation
This follows the specification available in the [`api.asc`](https://github.com/LedgerHQ/ledger-app-boilerplate/blob/master/doc/api.asc).
