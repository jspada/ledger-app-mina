#!/usr/bin/env python

from ledgerblue.comm import getDongle
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--account_number', help="BIP32 account to retrieve. e.g. \"12345\".")
args = parser.parse_args()

if args.account_number == None:
	args.account_number = "12345"

dongle = getDongle(True)

# Get address example
print("Get address")
# Create APDU message.
# CLA 0xE0  CLA
# INS 0x02  GET_ADDRESS
# P1 0x00   UNUSED
# P2 0x00   UNUSED
account = '{:08x}'.format(int(args.account_number))
apduMessage = 'E0020000' + '{:08x}'.format(len(account) + 1) + account
apdu = bytearray.fromhex(apduMessage)
address = dongle.exchange(apdu).decode('utf-8').rstrip('\x00')
print('Received: {}'.format(address))
