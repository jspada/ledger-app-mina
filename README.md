# ledger-app-mina

## Overview
This is the Mina app for the Ledger Nano S and Nano X hardware wallet.

## Building and installing
To build and install the app on your Ledger Nano you must set up the Ledger Nano build environments. Please follow the Getting Started instructions at [here](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).

If you don't want to setup a global environnment, you can also setup one just for this app by sourcing `prepare-devenv.sh` with the right target (`s` or `x`).

Install prerequisite and switch to the Nano dev-env:
```bash
sudo apt install gcc-multilib g++-multilib
sudo apt install python3-venv python3-dev libudev-dev libusb-1.0-0-dev

# install pyelftools if using emulator
pip3 install --upgrade pip
pip3 install pytest pyelftools mnemonic jsonschema construct pyqt5 flask flask-restful

# (x or s, depending on your device)
source prepare-devenv.sh s
```

Clone the source
```bash
git clone --recurse-submodules https://github.com/jspada/ledger-app-mina.git
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

Run the app with the emulator:
```bash
make run
```

To engage the emulator use the `LEDGER_PROXY_ADDRESS` and `LEDGER_PROXY_PORT` environmental variables.

For example

```bash
LEDGER_PROXY_ADDRESS=127.0.0.1 LEDGER_PROXY_PORT=9999 ./utils/mina_ledger_wallet get-address 0
```

You can also run the app with emulator automation that will automatically approve each operation.  This is done by setting the `AUTOMATION` environmental variable and can be useful when testing.

Run the app with the emulator and automation:
```bash
AUTOMATION=1 make run
```

## Unit tests

There are two types of unit tests: those that run off-device as part of the build
and those that can be run on the Ledger device.

### Off-device unit tests

The off-device unit tests run automatically as part of the build, some using the Ledger [Speculos](https://github.com/LedgerHQ/speculos) emulator.  Speculos is included as a submodule of the ledger-app-mina repository (see cloning instructions above).

You can skip running the off-device emulator tests by using the `NO_EMULATOR` environmental variable.

```bash
NO_EMULATOR=1 make
```

### On-device unit tests

1. Initialize your Ledger test hardware with the following secret phrase

```
course grief vintage slim tell hospital car maze model style elegant kitchen state purpose matrix gas grid enable frown road goddess glove canyon key
```

2. Build and install the Mina unit tests app

```bash
make clean
ON_DEVICE_UNIT_TESTS=1 make load
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
There is additional help available for each subcommand.

**Get address**

```bash
$ ./utils/mina_ledger_wallet.py get-address 1
Get address for account 1 (path 44'/12586'/1'/0/0)
Continue? (y/N) y
Generating address (please confirm on Ledger device)... done
Received address: B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt
```
This generates the keypair corresponding to hardware wallet account 1 (BIP44 account `44'/12586'/1'/0/0`) and returns the corresponding Mina address.

**Get balance**

```bash
$ ./utils/mina_ledger_wallet.py get-balance B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt
Getting network identifier... debug
Getting account balance... done

Address: B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt
Balance: 9792.0
```
This queries the Mina blockchain for the balance of address `B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt`.

**Send payment**

```bash
$ ./utils/mina_ledger_wallet.py send-payment 1 B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g 2.71821
```

This sends a payment of 2.71821 Mina from hardware wallet account 1 (`B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt`) to recipient `B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g`.

**Delegate**

```bash
$ ./utils/mina_ledger_wallet.py delegate 1 B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g --memo "Delegation is fun!"
```

This delegates the entire balance of hardware wallet account 1 (`B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt`) to delegate `B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g`.

## Documentation
This follows the specification available in the [`api.asc`](https://github.com/LedgerHQ/ledger-app-boilerplate/blob/master/doc/api.asc).
