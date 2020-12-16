#!/usr/bin/env python

import argparse
import sys
import os

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../utils/")
import mina_ledger_wallet as mina

parser = argparse.ArgumentParser()
parser.add_argument('--kind', help="Kind of tests to run (all, get-address, sign-transaction)",
                    choices = ["all", "get-address", "sign-transaction"], default="all")
args = parser.parse_args()

mina.ledger_init()

print("Running unit tests")

def run_get_address_tests():
    # account 0
    # private key 3c041039ac9ac5dea94330115aacf6d4780f08d7299a84a6ee2b62599cebb5e6 (27145950286090989573235160126994188021722699404890955797699008383743072908774)
    # public key 1e32fca120ee917a3ce10d2198300526c5289f276b2020d77329d38538be2be93011e21673a9e1af4240ecac174770f2db76c646cd5609196421ffaefed46d94
    assert(mina.ledger_get_address(0) == "B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx")

    # account 1
    # private key 06270f74f8a6f2529f492a3bf112a3806e91e62b8fc3f247569e9a43ba9e8d6e
    # public key 2e162c0254a378d642a8716fb2a0a45249809b36e69a6146d25a00bc3b0492b625fc3205992bc6b270329bd51787e35f2f79b9311db2888cefb6c04364853988
    assert(mina.ledger_get_address(1) == "B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P")

    # account 2
    # private key 2943fbbbcf63025456cfcebae3f481462b1e720d0c7d2a10d113fb6b1847cb3d
    # public key 27f462c09d9d73da6dba3c15ff26706660ff93c1a62c54220b5298fcafaaf0bb047be08f48a8121779023576d98e3679e6cf9ae3e0869e978468f23f7286f9f7
    assert(mina.ledger_get_address(2) == "B62qpkf1jH9sqtZy4kAJHgdChfuRX7SPqoX4Q2ZjJH2YDNWUUd92bxo")

    # account 3
    # private key 03e97cbf15dba6da23616785886f8cb4ce9ced51f0140261332ee063bb7f17d3
    # public key 28057358624705e5eb20e72c5aed1e31181345f7247bd3cebd41f3ceb1583b8211fe1925f43f6f063eb47bc136e712ca6e8583846d7f5f85a7a442493d095851
    assert(mina.ledger_get_address(3) == "B62qnpUj6EJGNvhJFMEAmM6skJRg1H37hVsHvPHMXhHeCXfKhSWGkGN")

    # account 49370
    # private key 02989a314a65930de289a5578daa03c3410b177e009121574d8730bf8644ab9f
    # public key 2ce3f026cad6d15845b0d99dfd0286fe85c976ad61a31fddda7d11ce0f2f08c733f4fd49c1797b69f16a14f65a78a99d9630faa30cb7c20aa7fe2165168052dc
    assert(mina.ledger_get_address(49370) == "B62qq8DZP9h5cCKr6ecXY3MqVz1oQuEzJMyZLCbEukCJGS9SuVXK33o")

    # account 12586
    # private key 1f2c90f146d1035280364cb1a01a89e7586a340972936abd5d72307a0674549c
    # public key 2969f63c053ea7e49aa985982427822dfe1d09f2e3f43366714c84db3de6e3780bf8cdef3b89c52c47752a58df4b2898705ebfc67434a2dcb035b5e2858ee5b4
    assert(mina.ledger_get_address(0x312a) == "B62qnWKWnUmj3mxUx4UcnQGMMsqwNkHUdgzvhto6Je3LwKSRb7dYqm9")

def run_signature_tests():
    # Sign payment tx (account 0, amount=1729, fee=2, nonce=16, valid_until=271828, mem="Hello Mina!")
    # apdu=e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000
    # sig=2991b0b58663dfc6d11dc14051365d5a866eb7d91c05111d337626a3aa7511ef1e819896060028741949282e52540084f8a05863f5feb68499704619303660d2
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               0,
                               "B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx",
                               "B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P",
                               1729000000000,
                               2000000000,
                               16,
                               271828,
                               "Hello Mina!") == "2991b0b58663dfc6d11dc14051365d5a866eb7d91c05111d337626a3aa7511ef1e819896060028741949282e52540084f8a05863f5feb68499704619303660d2")

    # Sign payment tx (account 12586, amount=314.159265359, fee=1.618033988, nonce=0)
    # apdu=e00300000000015a0000312a423632716e574b576e556d6a336d7855783455636e51474d4d7371774e6b485564677a7668746f364a65334c774b535262376459716d3942363271706b66316a48397371745a79346b414a486764436866755258375350716f583451325a6a4a483259444e57555564393262786f000000492559f64f0000000060713d4400000000ffffffff000000000000000000000000000000000000000000000000000000000000000000
    # sig=3d9d3cbc668092dabf9412b38e42f6cab7c620424bf168f42f5292406a0627733410112f65f5aaafd761f8f9c01926e07546247b55e47e60db58284f12ab27d8
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               12586,
                               "B62qnWKWnUmj3mxUx4UcnQGMMsqwNkHUdgzvhto6Je3LwKSRb7dYqm9",
                               "B62qpkf1jH9sqtZy4kAJHgdChfuRX7SPqoX4Q2ZjJH2YDNWUUd92bxo",
                               314159265359,
                               1618033988,
                               0,
                               4294967295,
                               "") == "3d9d3cbc668092dabf9412b38e42f6cab7c620424bf168f42f5292406a0627733410112f65f5aaafd761f8f9c01926e07546247b55e47e60db58284f12ab27d8")

    # Sign payment tx (account 12586, amount=271828.182845904, fee=0.0001, nonce=5687, memo="01234567890123456789012345678901")
    # apdu=e00300000000015a0000312a423632716e574b576e556d6a336d7855783455636e51474d4d7371774e6b485564677a7668746f364a65334c774b535262376459716d39423632716e70556a36454a474e76684a464d45416d4d36736b4a526731483337685673487650484d586848654358664b685357476b474e0000f739ee4749d000000000000186a000001637ffffffff303132333435363738393031323334353637383930313233343536373839303100
    # sig=1aa66d201959bad1232c37ab2808dd0804a48672e3df82a93641f82c5801be690c99d1b5a1e864822d54c387dd45c8fa562e81ad8d5e61539e9330633c3ca6c4
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               12586,
                               "B62qnWKWnUmj3mxUx4UcnQGMMsqwNkHUdgzvhto6Je3LwKSRb7dYqm9",
                               "B62qnpUj6EJGNvhJFMEAmM6skJRg1H37hVsHvPHMXhHeCXfKhSWGkGN",
                               271828182845904,
                               100000,
                               5687,
                               4294967295,
                               "01234567890123456789012345678901") == "1aa66d201959bad1232c37ab2808dd0804a48672e3df82a93641f82c5801be690c99d1b5a1e864822d54c387dd45c8fa562e81ad8d5e61539e9330633c3ca6c4")

    # Sign payment tx (account 3, amount=0, fee=2, nonce=0, valid_until=1982)
    # apdu=e00300000000015a00000003423632716e70556a36454a474e76684a464d45416d4d36736b4a526731483337685673487650484d586848654358664b685357476b474e4236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676780000000000000000000000007735940000000000000007be000000000000000000000000000000000000000000000000000000000000000000
    # sig=30c2b6e779ad28f2648e35d124fff18af6587570daa18ad310f76a7621cc7e9e2ed2ef12a7e9ade54aaebaa2e44097e8c143dc129f9dd32a8934122d9fb8b172
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               3,
                               "B62qnpUj6EJGNvhJFMEAmM6skJRg1H37hVsHvPHMXhHeCXfKhSWGkGN",
                               "B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx",
                               0,
                               2000000000,
                               0,
                               1982,
                               "") == "30c2b6e779ad28f2648e35d124fff18af6587570daa18ad310f76a7621cc7e9e2ed2ef12a7e9ade54aaebaa2e44097e8c143dc129f9dd32a8934122d9fb8b172")

    # Sign delegation tx (account 0, fee=2, nonce=16, valid_until=1337, mem="Delewho?")
    # apdu=e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000000000000000000000077359400000000100000053944656c6577686f3f00000000000000000000000000000000000000000000000004
    # sig=0d26383a40a93af5e236fb2f0ff24d7337d67030d640c4017662b74626ae7ba716aba2cc21604e3be6b5427b5a70d251d752263714deb7dec724947429a4e187
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               0,
                               "B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx",
                               "B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P",
                               0,
                               2000000000,
                               16,
                               1337,
                               "Delewho?") == "0d26383a40a93af5e236fb2f0ff24d7337d67030d640c4017662b74626ae7ba716aba2cc21604e3be6b5427b5a70d251d752263714deb7dec724947429a4e187")

    # Sign delegation tx (account 49370, fee=2, nonce=0)
    # apdu=e00300000000015a0000c0da423632717138445a5039683563434b723665635859334d71567a316f5175457a4a4d795a4c436245756b434a475339537556584b33336f4236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676780000000000000000000000007735940000000000ffffffff000000000000000000000000000000000000000000000000000000000000000004
    # sig=174f0d86d8624e602220e20f8cb797bc0647917819fd4fcd438ad0fe2499424b01b7327d61cba90c7a22ca5795c676112f3aa6bce9ddd8a1aefff0aa48e83a71
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               49370,
                               "B62qq8DZP9h5cCKr6ecXY3MqVz1oQuEzJMyZLCbEukCJGS9SuVXK33o",
                               "B62qrGaXh9wekfwaA2yzUbhbvFYynkmBkhYLV36dvy5AkRvgeQnY6vx",
                               0,
                               2000000000,
                               0,
                               4294967295,
                               "") == "174f0d86d8624e602220e20f8cb797bc0647917819fd4fcd438ad0fe2499424b01b7327d61cba90c7a22ca5795c676112f3aa6bce9ddd8a1aefff0aa48e83a71")

    # Sign delegation tx (account 12586, fee=42, nonce=1, memo="more delegates, more fun........")
    # apdu=e00300000000015a0000312a423632716e574b576e556d6a336d7855783455636e51474d4d7371774e6b485564677a7668746f364a65334c774b535262376459716d39423632717138445a5039683563434b723665635859334d71567a316f5175457a4a4d795a4c436245756b434a475339537556584b33336f000000000000000000000009c765240000000001ffffffff6d6f72652064656c6567617465732c206d6f72652066756e2e2e2e2e2e2e2e2e04
    # sig=16920bbd7daee6835747b4645356ee982c51de036cf64e00d9339ce48af1be670a2405d197021b86ff9b6b7eefe128f02a63e9e9bbbe45856867cc5eec7cacb5
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               12586,
                               "B62qnWKWnUmj3mxUx4UcnQGMMsqwNkHUdgzvhto6Je3LwKSRb7dYqm9",
                               "B62qq8DZP9h5cCKr6ecXY3MqVz1oQuEzJMyZLCbEukCJGS9SuVXK33o",
                               0,
                               42000000000,
                               1,
                               4294967295,
                               "more delegates, more fun........") == "16920bbd7daee6835747b4645356ee982c51de036cf64e00d9339ce48af1be670a2405d197021b86ff9b6b7eefe128f02a63e9e9bbbe45856867cc5eec7cacb5")

    # Sign delegation tx (account 2, fee=1.2020569, nonce=0, valid_until=577216)
    # apdu=e00300000000015a0000000242363271706b66316a48397371745a79346b414a486764436866755258375350716f583451325a6a4a483259444e57555564393262786f4236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000000000000000000000047a5eec4000000000008cec0000000000000000000000000000000000000000000000000000000000000000004
    # sig=1b3870f89d0208740ab1284c00364f7f34949a9a823fd245e1d3d33e322679561379544c9002726386d11a04c5d58291b41af7cff516d809f77d8dda3e8cc1f9
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               2,
                               "B62qpkf1jH9sqtZy4kAJHgdChfuRX7SPqoX4Q2ZjJH2YDNWUUd92bxo",
                               "B62qpaDc8nfu4a7xghkEni8u2rBjx7EH95MFeZAhTgGofopaxFjdS7P",
                               0,
                               1202056900,
                               0,
                               577216,
                               "") == "1b3870f89d0208740ab1284c00364f7f34949a9a823fd245e1d3d33e322679561379544c9002726386d11a04c5d58291b41af7cff516d809f77d8dda3e8cc1f9")

try:
    if args.kind == "all":
        run_get_address_tests()
        run_signature_tests()
    elif args.kind == "get-address":
        run_get_address_tests()
    elif args.kind == "sign-transaction":
        run_signature_tests()
except AssertionError:
    raise
except Exception as ex:
    print("{}".format(ex))
    sys.exit(233)

print("Unit tests completed successfully")
