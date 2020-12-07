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
    apduMessage = 'E0020000' + '{:08x}'.format(len(account) + 1) + account
    apdu = bytearray.fromhex(apduMessage)
    address = dongle.exchange(apdu)
    return address.decode('utf-8').rstrip('\x00')

print("Running unit tests")

def run_get_address_tests():

    # account 0
    # private key 3c041039ac9ac5dea94330115aacf6d4780f08d7299a84a6ee2b62599cebb5e6 (27145950286090989573235160126994188021722699404890955797699008383743072908774)
    # public key 1e32fca120ee917a3ce10d2198300526c5289f276b2020d77329d38538be2be93011e21673a9e1af4240ecac174770f2db76c646cd5609196421ffaefed46d94
    assert(get_address(0) == "B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx")

    # account 1
    # private key 06270f74f8a6f2529f492a3bf112a3806e91e62b8fc3f247569e9a43ba9e8d6e
    # public key 2e162c0254a378d642a8716fb2a0a45249809b36e69a6146d25a00bc3b0492b625fc3205992bc6b270329bd51787e35f2f79b9311db2888cefb6c04364853988
    assert(get_address(1) == "B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P")

    # account 2
    # private key 2943fbbbcf63025456cfcebae3f481462b1e720d0c7d2a10d113fb6b1847cb3d
    # public key 27f462c09d9d73da6dba3c15ff26706660ff93c1a62c54220b5298fcafaaf0bb047be08f48a8121779023576d98e3679e6cf9ae3e0869e978468f23f7286f9f7
    assert(get_address(2) == "B62qpkf1jH9sqtZy4kAJHgdChfuRX7SPqoX4Q2ZjJH2YDNWUUd92bxo")

    # account 3
    # private key 03e97cbf15dba6da23616785886f8cb4ce9ced51f0140261332ee063bb7f17d3
    # public key 28057358624705e5eb20e72c5aed1e31181345f7247bd3cebd41f3ceb1583b8211fe1925f43f6f063eb47bc136e712ca6e8583846d7f5f85a7a442493d095851
    assert(get_address(3) == "B62qnpUj6EJGNvhJFMEAmM6skJRg1H37hVsHvPHMXhHeCXfKhSWGkGN")

    # account 49370
    # private key 02989a314a65930de289a5578daa03c3410b177e009121574d8730bf8644ab9f
    # public key 2ce3f026cad6d15845b0d99dfd0286fe85c976ad61a31fddda7d11ce0f2f08c733f4fd49c1797b69f16a14f65a78a99d9630faa30cb7c20aa7fe2165168052dc
    assert(get_address(49370) == "B62qq8DZP9h5cCKr6ecXY3MqVz1oQuEzJMyZLCbEukCJGS9SuVXK33o")

    # account 12586
    # private key 1f2c90f146d1035280364cb1a01a89e7586a340972936abd5d72307a0674549c
    # public key 2969f63c053ea7e49aa985982427822dfe1d09f2e3f43366714c84db3de6e3780bf8cdef3b89c52c47752a58df4b2898705ebfc67434a2dcb035b5e2858ee5b4
    assert(get_address(0x312a) == "B62qnWKWnUmj3mxUx4UcnQGMMsqwNkHUdgzvhto6Je3LwKSRb7dYqm9")

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
