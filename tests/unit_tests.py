#!/usr/bin/env python

import ledgerblue.comm as ledgerblue
from ledgerblue.comm import getDongle
import argparse
from binascii import unhexlify
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--kind', help="Kind of tests to run (all, get_address, sign)", default="all")
args = parser.parse_args()

try:
    dongle = getDongle(True)
except:
    print("Failed to connect to Ledger device")
    sys.exit(233);

def get_address(account_number):
    # Create APDU message.
    # CLA 0xE0  CLA
    # INS 0x02  GET_ADDRESS
    # P1 0x00   UNUSED
    # P2 0x00   UNUSED
    account = '{:08x}'.format(account_number)
    apduMessage = 'E0020000' + '{:02x}'.format(len(account) + 1) + account
    apdu = bytearray.fromhex(apduMessage)
    address = dongle.exchange(apdu)
    return address.decode('utf-8').rstrip('\x00')

print("Running unit tests")

def run_get_address_tests():

    # private key 01dbc383e924532164a9c8518f781b02c4762aa183e167ecd157d0b251d138c0
    # public key 2c9761f871a49b5956bfb39dd46b0365aed1af4e2a0a085412b4164e349ffdc4381eeda14f3d4743c9f1a6f712b0bc37bc2ee0cf1e1ebb3dd67e0fbbb10c8be3
    assert(get_address(0) == "B62qq4FREq1q1DuTSBDEzMbpkXMjcp6p1XogmCVzZL21dawjZa2SS1d")

    # private key 2b07a97d8ecc8f7da2cab847aa7b465fa46880661642df802078c29f8236785e
    # public key 1c42e1ce09937b3c9a6463d11a70dda150377330fdf865cea44717b736918edf19aabedc4cd9475cf43b1057c78790665beda1e30d6548ef2c63975265596878
    assert(get_address(1) == "B62qqwtcG43GVR1D1cGEvSuAgkTJwpZHToBqKTMX7oqWmr9Xb5R64tB")

    # private key 184f5234bfa662076f56cf5faf4a482361dbb7fe8647e100711cac3b7ce0d05a
    # public key 0bee7a1aabb40cbd84518dbfdaa09b85845b29902d8d77a48bd343682c900c5d2485f565358686d6b01f2cdcaf0b3e09206d37f38a15bc8920de3e375b0729ad
    assert(get_address(2) == "B62qmaCaYi4bvfVsibkeY4P4cNaGGYc6xU5s1UWvMT26q9ichdCTtkK")

    # private key 231c7f2478a36f51609d1743124d0a15b2d917c989e5fa9b2c4dc4997c807f57
    # public key 07b96c07a29ea0fd597cf23b62b2870ff2105a7c5e7b266e65a5509eb9cbcf2c31ca66751ee409cdf9f8ebdefad6fd78975d918b63f2c8ebaedeb6da8779627c
    assert(get_address(3) == "B62qjxS5rkpdtASZzEBeo32yyeVPqjB1RwxNrhmpNmnyGqStzv2FMj9")

    # private key 35e02474e94ec3d790684104988ec5bd9c7329c3bf3d630fe28efdc2a49227eb
    # public key 39eac1c1e50c6324e65b093384763363eae2c52c1512f463777003afbaa21c3318a91f6c7be05943b86cbc9803685f8a893b0cc7f640d9f392c4d3799da7217f
    assert(get_address(0xc0da) == "B62qkAgPgsUUxMPgVggMYgH6VBLsjwH6adYgZA5bpkKHrRRsjN1EH34")

try:
    if args.kind == "all":
        run_get_address_tests()
    elif args.kind == "get_address":
        run_get_address_tests()
except ledgerblue.CommException as ex:
    if ex.sw == 26368:
        print("Ledger app not open")
    else:
        print("Comm {}".format(ex.sw))
    sys.exit(233)
except AssertionError:
    raise
except Exception as ex:
    print("{}".format(ex))
    sys.exit(233)

print("Unit tests completed successfully")
