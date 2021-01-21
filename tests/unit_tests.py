#!/usr/bin/env python

import argparse
import sys
import os
import time

sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../utils/")
import mina_ledger_wallet as mina

parser = argparse.ArgumentParser()
parser.add_argument('--kind', help="Kind of tests to run (all, fuzz, get-address, sign-transaction)",
                    choices = ["all", "fuzz", "get-address", "sign-transaction"], default="all")
args = parser.parse_args()

mina.ledger_init()


def run_fuzz_tests():
    # Invalid message1
    assert(not mina.ledger_send_apdu("a5a501a6"))

    # Invalid message2
    assert(not mina.ledger_send_apdu("b08fdaeeb08fdaee6e8f58de53c7f54e3b86ef06d646e0c28173ab524cf21297eed41c870346760ecee46558de53c7f5b08fdaee6e8f58de53c7f54e3b86e119a24cf21f06d646e0c28173ab5465b08fdaee6e8f58de53c7f54e3b86e119a24cf21297eed41c8703467652279a3e7ec598ef6f06d646e0c28173ab57f897719eb5db73b16043bc7cc0c94cf21297eed41c870346760ecee465"))

    # Invalid message4
    assert(not mina.ledger_send_apdu("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"))

    # Invalid message5
    assert(not mina.ledger_send_apdu("0"))

    # Invalid message6
    assert(not mina.ledger_send_apdu(""))

    # Invalid command
    assert(not mina.ledger_send_apdu("01f600000000000c00000000"))

    # Invalid instruction
    assert(not mina.ledger_send_apdu("e00000000000000c00000000"))

    # Invalid get address (nonhex1)
    assert(not mina.ledger_send_apdu("e00200000000000d100000000"))

    # Invalid instruction
    assert(not mina.ledger_send_apdu("e00300000000000c00000000"))

    # Invalid instruction
    assert(not mina.ledger_send_apdu("e0ff00000000000c00000000"))

    # Invalid get address (nonhex2)
    assert(not mina.ledger_send_apdu("e00200000000000d10deadbeats"))

    # Invalid get address 4294967296 (account number range)
    assert(not mina.ledger_send_apdu("e00200000000000e0100000000"))

    # Invalid get address 18446744073709551615 (account number range, message too big)
    assert(not mina.ledger_send_apdu("e002000000000014ffffffffffffffff"))

    # Invalid get address (message to small)
    assert(not mina.ledger_send_apdu("e00200000000000e010000"))

    # Valid get address 0
    assert(mina.ledger_send_apdu("e00200000000000c00000000"))

    # Valid get address 0 (ignored P1 and P2)
    assert(mina.ledger_send_apdu("e002a5a50000000c00000000"))

    # Valid get address 0 (ignored P1)
    assert(mina.ledger_send_apdu("e002ff000000000c00000000"))

    # Valid get address 0 (ignored P2)
    assert(mina.ledger_send_apdu("e00200110000000c00000000"))

    # Valid get max address 4294967295
    assert(mina.ledger_send_apdu("e00200000000000cffffffff"))

    # Invalid sign tx (sender address does not match account number)
    assert(not mina.ledger_send_apdu("e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

    # Invalid sign tx (message too big)
    assert(not mina.ledger_send_apdu("e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e6121000000000000000000000000000000000000000000001"))

    # Invalid sign tx (message too big)
    assert(not mina.ledger_send_apdu("e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448600000e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e6121000000000000000000000000000000000000000007454839354d46655a411"))

    # Invalid sign tx (message too small)
    assert(not mina.ledger_send_apdu("e00300000000015a000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e6121000000000000000000000000000000000000000000"))

    # Invalid sign tx (invalid tx type)
    assert(not mina.ledger_send_apdu("e00300000000015a00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000003"))

def run_get_address_tests():
    t0 = time.time()

    # These tests were automatically generated from the Mina c-reference-signer
    #
    #     Details: https://github.com/MinaProtocol/c-reference-signer/README.markdown
    #     Command: ./unit_tests ledger_gen

    # account 0
    # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
    assert(mina.ledger_get_address(0) == "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV")

    # account 1
    # private key 3ca187a58f09da346844964310c7e0dd948a9105702b716f4d732e042e0c172e
    assert(mina.ledger_get_address(1) == "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt")

    # account 2
    # private key 336eb4a19b3d8905824b0f2254fb495573be302c17582748bf7e101965aa4774
    assert(mina.ledger_get_address(2) == "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi")

    # account 3
    # private key 1dee867358d4000f1dafa5978341fb515f89eeddbe450bd57df091f1e63d4444
    assert(mina.ledger_get_address(3) == "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N")

    # account 49370
    # private key 20f84123a26e58dd32b0ea3c80381f35cd01bc22a20346cc65b0a67ae48532ba
    assert(mina.ledger_get_address(49370) == "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M")

    # account 0x312a
    # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
    assert(mina.ledger_get_address(0x312a) == "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4")

    # Performance report
    duration = time.time() - t0
    print("Performed 6 get-address operations in {:0.03f} seconds ({:0.03f} sec per operation)".format(duration, duration/6.0))

def run_signature_tests():
    t0 = time.time()

    # These tests were automatically generated from the Mina c-reference-signer
    #
    #     Details: https://github.com/MinaProtocol/c-reference-signer/README.markdown
    #     Command: ./unit_tests ledger_gen

    # account 0
    # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
    # sig=0a68fc40b470abedd14cd8b830effa4fa6225e76cbc67fa46dfb0f825c0d1a7d1a8685817e449150070456b5628eeb9af954040e023d3a1b4211c818d210ee56
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               0,
                               "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
                               "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
                               1729000000000,
                               2000000000,
                               16,
                               271828,
                               "Hello Mina!") == "0a68fc40b470abedd14cd8b830effa4fa6225e76cbc67fa46dfb0f825c0d1a7d1a8685817e449150070456b5628eeb9af954040e023d3a1b4211c818d210ee56")

    # account 12586
    # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
    # sig=32d7ea2ae54df316e7baa4bebf8a62ea1cfb321debc75e27fc0ba302beba383a398ec6e103e0101a20179955bb11a1956bf0b470d7782344aec4d8d0fc73ed92
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               12586,
                               "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
                               "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi",
                               314159265359,
                               1618033988,
                               0,
                               4294967295,
                               "") == "32d7ea2ae54df316e7baa4bebf8a62ea1cfb321debc75e27fc0ba302beba383a398ec6e103e0101a20179955bb11a1956bf0b470d7782344aec4d8d0fc73ed92")

    # account 12586
    # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
    # sig=063a7b5b5b78090760eb93cbfacf5672155e1c0bcfd5629d75b06bbb079694922f1394b7eb2f929b5a97f229e988523223e4b7fee531d8d85caafd1c702b1673
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               12586,
                               "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
                               "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N",
                               271828182845904,
                               100000,
                               5687,
                               4294967295,
                               "01234567890123456789012345678901") == "063a7b5b5b78090760eb93cbfacf5672155e1c0bcfd5629d75b06bbb079694922f1394b7eb2f929b5a97f229e988523223e4b7fee531d8d85caafd1c702b1673")

    # account 3
    # private key 1dee867358d4000f1dafa5978341fb515f89eeddbe450bd57df091f1e63d4444
    # sig=09c5712632f6281a43c64dbb936ce6002a0c2e004b375037a05ec7e266f9f1be3f8e5bdd506c35c6546cfc4edbeaff816a38096c0bdb408341eb0e25adbf4d83
    assert(mina.ledger_sign_tx(mina.TX_TYPE_PAYMENT,
                               3,
                               "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N",
                               "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
                               0,
                               2000000000,
                               0,
                               1982,
                               "") == "09c5712632f6281a43c64dbb936ce6002a0c2e004b375037a05ec7e266f9f1be3f8e5bdd506c35c6546cfc4edbeaff816a38096c0bdb408341eb0e25adbf4d83")

    # account 0
    # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
    # sig=376cd8a00b4ce495b3b23187b94a688a1c36837d2eb911c0085b3e37ba96dea02a3573e6a6471b068e14a03fe0b7d6399119ea52e4a310c3f98d7af5d988c676
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               0,
                               "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
                               "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
                               0,
                               2000000000,
                               16,
                               1337,
                               "Delewho?") == "376cd8a00b4ce495b3b23187b94a688a1c36837d2eb911c0085b3e37ba96dea02a3573e6a6471b068e14a03fe0b7d6399119ea52e4a310c3f98d7af5d988c676")

    # account 49370
    # private key 20f84123a26e58dd32b0ea3c80381f35cd01bc22a20346cc65b0a67ae48532ba
    # sig=05a1f5f50c6fe5616023251653e5be099d0ad942323498fb23bcfcd21c5fab6a3a641fce6d51e05566b0ce1244da30b0014cb7580f760f84e58eb654190bc607
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               49370,
                               "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M",
                               "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
                               0,
                               2000000000,
                               0,
                               4294967295,
                               "") == "05a1f5f50c6fe5616023251653e5be099d0ad942323498fb23bcfcd21c5fab6a3a641fce6d51e05566b0ce1244da30b0014cb7580f760f84e58eb654190bc607")

    # account 12586
    # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
    # sig=29febace385dfad1bcc97f1297d5f8c5bdadb57faf1c20a9c9f6c7516f80c6af05b0a0a186332f544b70c8e8717355bd7ebde310dee31b351f333219443ac798
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               12586,
                               "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
                               "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M",
                               0,
                               42000000000,
                               1,
                               4294967295,
                               "more delegates, more fun........") == "29febace385dfad1bcc97f1297d5f8c5bdadb57faf1c20a9c9f6c7516f80c6af05b0a0a186332f544b70c8e8717355bd7ebde310dee31b351f333219443ac798")

    # account 2
    # private key 336eb4a19b3d8905824b0f2254fb495573be302c17582748bf7e101965aa4774
    # sig=08a668739ec0bd4149e51a85ea9f05887232f91accb884c312dbca8ef7de0c9b341178cfb969c69bb9fc87df110276880cf09bcdf6b899ea3d1d1b4aa59e7c33
    assert(mina.ledger_sign_tx(mina.TX_TYPE_DELEGATION,
                               2,
                               "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi",
                               "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
                               0,
                               1202056900,
                               0,
                               577216,
                               "") == "08a668739ec0bd4149e51a85ea9f05887232f91accb884c312dbca8ef7de0c9b341178cfb969c69bb9fc87df110276880cf09bcdf6b899ea3d1d1b4aa59e7c33")

    # Performance report
    duration = time.time() - t0
    print("Performed 8 sign-tx operations in {:0.03f} seconds ({:0.03f} sec per operation)".format(duration, duration/8.0))

print("Running unit tests...")

try:
    if args.kind == "all":
        run_fuzz_tests()
        run_get_address_tests()
        run_signature_tests()
    elif args.kind == "fuzz":
        run_fuzz_tests()
    elif args.kind == "get-address":
        run_get_address_tests()
    elif args.kind == "sign-transaction":
        run_signature_tests()
except AssertionError:
    raise
except Exception as ex:
    print("{}".format(ex))
    sys.exit(233)

print("Completed unit tests successfully!")
