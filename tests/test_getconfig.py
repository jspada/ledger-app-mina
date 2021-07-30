#!/usr/bin/env python
from ledgerblue.comm import getDongle
import argparse
from binascii import unhexlify

# Create APDU message.
# CLA 0xE0
# INS 0x01  GET_APP_CONFIGURATION
# P1 0x00   USER CONFIRMATION REQUIRED (0x00 otherwise)
# P2 0x00   UNUSED
# Lc 0x00
# Le 0x40
apduMessage = "E00100000004"
apdu = bytearray.fromhex(apduMessage)

print("~~ Ledger Boilerplate ~~")
print("Check Configuration")

dongle = getDongle(debug=False)
result = dongle.exchange(apdu)

print("N_storage.dummy_setting_1 : " + '{:02x}' .format(result[0]))
print("N_storage.dummy_setting_2 : " + '{:02x}' .format(result[1]))
print("LEDGER_MAJOR_VERSION      : " + '{:02x}' .format(result[2]))
print("LEDGER_MINOR_VERSION      : " + '{:02x}' .format(result[3]))
print("LEDGER_PATCH_VERSION      : " + '{:02x}' .format(result[4]))
