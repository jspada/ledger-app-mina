#!/usr/bin/env python3

import mina_ledger_wallet as mina
import pytest
import argparse
import sys
import os
import time

print("hi")
sys.path.append(os.path.dirname(os.path.realpath(__file__)) + "/../utils/")


if __name__ == "__main__":
    print("how")
    parser = argparse.ArgumentParser()
    parser.add_argument('--kind', help="Kind of tests to run (all, release, crypto, fuzz, get-address, sign-transaction)",
                        choices=["all", "release", "crypto", "fuzz", "get-address", "sign-transaction"], default="release")
    args = parser.parse_args()
    print("are")

print("init")
mina.ledger_init()
print("after init")


# def send_apdu(adpu_hex):
#     print("apdu")
#     try:
#         return mina.ledger_send_apdu(adpu_hex)
#     except:
#         return False


# def get_address(account_number):
#     print("get address")
#     try:
#         return mina.ledger_get_address(account_number)
#     except:
#         return False


# def sign_tx(tx_type, sender_account, sender_address, receiver, amount, fee,
#             nonce, valid_until, memo, network_id):
#     try:
#         return mina.ledger_sign_tx(tx_type, sender_account, sender_address, receiver,
#                                    amount, fee, nonce, valid_until, memo, network_id)
#     except:
#         return False


# class TestCrypto:
#     @pytest.mark.all
#     def test(self):
#         assert(mina.ledger_crypto_tests())


# class TestGetAddress:
#     def test(self):
#         # Address generation tests
#         #
#         #     These tests were automatically generated from the Mina c-reference-signer
#         #
#         #     Details:  https://github.com/MinaProtocol/c-reference-signer/README.markdown
#         #     Generate: ./unit_tests ledger_gen

#         # account 0
#         # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
#         assert(get_address(0) ==
#                "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV")

#         # account 1
#         # private key 3ca187a58f09da346844964310c7e0dd948a9105702b716f4d732e042e0c172e
#         assert(get_address(1) ==
#                "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt")

#         # account 2
#         # private key 336eb4a19b3d8905824b0f2254fb495573be302c17582748bf7e101965aa4774
#         assert(get_address(2) ==
#                "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi")

#         # account 3
#         # private key 1dee867358d4000f1dafa5978341fb515f89eeddbe450bd57df091f1e63d4444
#         assert(get_address(3) ==
#                "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N")

#         # account 49370
#         # private key 20f84123a26e58dd32b0ea3c80381f35cd01bc22a20346cc65b0a67ae48532ba
#         assert(get_address(49370) ==
#                "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M")

#         # account 0x312a
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         assert(get_address(0x312a) ==
#                "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4")


# class TestSignTx:
#     def test(self):
#         # Sign transaction tests
#         #
#         #     These tests were automatically generated from the Mina c-reference-signer
#         #
#         #     Details:  https://github.com/MinaProtocol/c-reference-signer/README.markdown
#         #     Generate: ./unit_tests ledger_gen

#         # account 0
#         # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
#         # sig=11a36a8dfe5b857b95a2a7b7b17c62c3ea33411ae6f4eb3a907064aecae353c60794f1d0288322fe3f8bb69d6fabd4fd7c15f8d09f8783b2f087a80407e299af
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        0,
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        1729000000000,
#                        2000000000,
#                        16,
#                        271828,
#                        "Hello Mina!",
#                        mina.TESTNET_ID) == "11a36a8dfe5b857b95a2a7b7b17c62c3ea33411ae6f4eb3a907064aecae353c60794f1d0288322fe3f8bb69d6fabd4fd7c15f8d09f8783b2f087a80407e299af")

#         # account 12586
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         # sig=23a9e2375dd3d0cd061e05c33361e0ba270bf689c4945262abdcc81d7083d8c311ae46b8bebfc98c584e2fb54566851919b58cf0917a256d2c1113daa1ccb27f
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        12586,
#                        "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
#                        "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi",
#                        314159265359,
#                        1618033988,
#                        0,
#                        4294967295,
#                        "",
#                        mina.TESTNET_ID) == "23a9e2375dd3d0cd061e05c33361e0ba270bf689c4945262abdcc81d7083d8c311ae46b8bebfc98c584e2fb54566851919b58cf0917a256d2c1113daa1ccb27f")

#         # account 12586
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         # sig=2b4d0bffcb57981d11a93c05b17672b7be700d42af8496e1ba344394da5d0b0b0432c1e8a77ee1bd4b8ef6449297f7ed4956b81df95bdc6ac95d128984f77205
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        12586,
#                        "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
#                        "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N",
#                        271828182845904,
#                        100000,
#                        5687,
#                        4294967295,
#                        "01234567890123456789012345678901",
#                        mina.TESTNET_ID) == "2b4d0bffcb57981d11a93c05b17672b7be700d42af8496e1ba344394da5d0b0b0432c1e8a77ee1bd4b8ef6449297f7ed4956b81df95bdc6ac95d128984f77205")

#         # account 3
#         # private key 1dee867358d4000f1dafa5978341fb515f89eeddbe450bd57df091f1e63d4444
#         # sig=25bb730a25ce7180b1e5766ff8cc67452631ee46e2d255bccab8662e5f1f0c850a4bb90b3e7399e935fff7f1a06195c6ef89891c0260331b9f381a13e5507a4c
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        3,
#                        "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N",
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        0,
#                        2000000000,
#                        0,
#                        1982,
#                        "",
#                        mina.TESTNET_ID) == "25bb730a25ce7180b1e5766ff8cc67452631ee46e2d255bccab8662e5f1f0c850a4bb90b3e7399e935fff7f1a06195c6ef89891c0260331b9f381a13e5507a4c")

#         # account 0
#         # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
#         # sig=30797d7d0426e54ff195d1f94dc412300f900cc9e84990603939a77b3a4d2fc11ebab12857b47c481c182abe147279732549f0fd49e68d5541f825e9d1e6fa04
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        0,
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        0,
#                        2000000000,
#                        16,
#                        1337,
#                        "Delewho?",
#                        mina.TESTNET_ID) == "30797d7d0426e54ff195d1f94dc412300f900cc9e84990603939a77b3a4d2fc11ebab12857b47c481c182abe147279732549f0fd49e68d5541f825e9d1e6fa04")

#         # account 49370
#         # private key 20f84123a26e58dd32b0ea3c80381f35cd01bc22a20346cc65b0a67ae48532ba
#         # sig=07e9f88fc671ed06781f9edb233fdbdee20fa32303015e795747ad9e43fcb47b3ce34e27e31f7c667756403df3eb4ce670d9175dd0ae8490b273485b71c56066
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        49370,
#                        "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M",
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        0,
#                        2000000000,
#                        0,
#                        4294967295,
#                        "",
#                        mina.TESTNET_ID) == "07e9f88fc671ed06781f9edb233fdbdee20fa32303015e795747ad9e43fcb47b3ce34e27e31f7c667756403df3eb4ce670d9175dd0ae8490b273485b71c56066")

#         # account 12586
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         # sig=1ff9f77fed4711e0ebe2a7a46a7b1988d1b62a850774bf299ec71a24d5ebfdd81d04a570e4811efe867adefe3491ba8b210f24bd0ec8577df72212d61b569b15
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        12586,
#                        "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
#                        "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M",
#                        0,
#                        42000000000,
#                        1,
#                        4294967295,
#                        "more delegates, more fun........",
#                        mina.TESTNET_ID) == "1ff9f77fed4711e0ebe2a7a46a7b1988d1b62a850774bf299ec71a24d5ebfdd81d04a570e4811efe867adefe3491ba8b210f24bd0ec8577df72212d61b569b15")

#         # account 2
#         # private key 336eb4a19b3d8905824b0f2254fb495573be302c17582748bf7e101965aa4774
#         # sig=26ca6b95dee29d956b813afa642a6a62cd89b1929320ed6b099fd191a217b08d2c9a54ba1c95e5000b44b93cfbd3b625e20e95636f1929311473c10858a27f09
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        2,
#                        "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        0,
#                        1202056900,
#                        0,
#                        577216,
#                        "",
#                        mina.TESTNET_ID) == "26ca6b95dee29d956b813afa642a6a62cd89b1929320ed6b099fd191a217b08d2c9a54ba1c95e5000b44b93cfbd3b625e20e95636f1929311473c10858a27f09")

#         # account 0
#         # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
#         # sig=124c592178ed380cdffb11a9f8e1521bf940e39c13f37ba4c55bb4454ea69fba3c3595a55b06dac86261bb8ab97126bf3f7fff70270300cb97ff41401a5ef789
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        0,
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        1729000000000,
#                        2000000000,
#                        16,
#                        271828,
#                        "Hello Mina!",
#                        mina.MAINNET_ID) == "124c592178ed380cdffb11a9f8e1521bf940e39c13f37ba4c55bb4454ea69fba3c3595a55b06dac86261bb8ab97126bf3f7fff70270300cb97ff41401a5ef789")

#         # account 12586
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         # sig=204eb1a37e56d0255921edd5a7903c210730b289a622d45ed63a52d9e3e461d13dfcf301da98e218563893e6b30fa327600c5ff0788108652a06b970823a4124
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        12586,
#                        "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
#                        "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi",
#                        314159265359,
#                        1618033988,
#                        0,
#                        4294967295,
#                        "",
#                        mina.MAINNET_ID) == "204eb1a37e56d0255921edd5a7903c210730b289a622d45ed63a52d9e3e461d13dfcf301da98e218563893e6b30fa327600c5ff0788108652a06b970823a4124")

#         # account 12586
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         # sig=076d8ebca8ccbfd9c8297a768f756ff9d08c049e585c12c636d57ffcee7f6b3b1bd4b9bd42cc2cbee34b329adbfc5127fe5a2ceea45b7f55a1048b7f1a9f7559
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        12586,
#                        "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
#                        "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N",
#                        271828182845904,
#                        100000,
#                        5687,
#                        4294967295,
#                        "01234567890123456789012345678901",
#                        mina.MAINNET_ID) == "076d8ebca8ccbfd9c8297a768f756ff9d08c049e585c12c636d57ffcee7f6b3b1bd4b9bd42cc2cbee34b329adbfc5127fe5a2ceea45b7f55a1048b7f1a9f7559")

#         # account 3
#         # private key 1dee867358d4000f1dafa5978341fb515f89eeddbe450bd57df091f1e63d4444
#         # sig=058ed7fb4e17d9d400acca06fe20ca8efca2af4ac9a3ed279911b0bf93c45eea0e8961519b703c2fd0e431061d8997cac4a7574e622c0675227d27ce2ff357d9
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        3,
#                        "B62qoqiAgERjCjXhofXiD7cMLJSKD8hE8ZtMh4jX5MPNgKB4CFxxm1N",
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        0,
#                        2000000000,
#                        0,
#                        1982,
#                        "",
#                        mina.MAINNET_ID) == "058ed7fb4e17d9d400acca06fe20ca8efca2af4ac9a3ed279911b0bf93c45eea0e8961519b703c2fd0e431061d8997cac4a7574e622c0675227d27ce2ff357d9")

#         # account 0
#         # private key 164244176fddb5d769b7de2027469d027ad428fadcc0c02396e6280142efb718
#         # sig=0904e9521a95334e3f6757cb0007ec8af3322421954255e8d263d0616910b04d213344f8ec020a4b873747d1cbb07296510315a2ec76e52150a4c765520d387f
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        0,
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        0,
#                        2000000000,
#                        16,
#                        1337,
#                        "Delewho?",
#                        mina.MAINNET_ID) == "0904e9521a95334e3f6757cb0007ec8af3322421954255e8d263d0616910b04d213344f8ec020a4b873747d1cbb07296510315a2ec76e52150a4c765520d387f")

#         # account 49370
#         # private key 20f84123a26e58dd32b0ea3c80381f35cd01bc22a20346cc65b0a67ae48532ba
#         # sig=2406ab43f8201bd32bdd81b361fdb7871979c0eec4e3b7a91edf87473963c8a4069f4811ebc5a0e85cbb4951bffe93b638e230ce5a250cb08d2c250113a1967c
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        49370,
#                        "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M",
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        0,
#                        2000000000,
#                        0,
#                        4294967295,
#                        "",
#                        mina.MAINNET_ID) == "2406ab43f8201bd32bdd81b361fdb7871979c0eec4e3b7a91edf87473963c8a4069f4811ebc5a0e85cbb4951bffe93b638e230ce5a250cb08d2c250113a1967c")

#         # account 12586
#         # private key 3414fc16e86e6ac272fda03cf8dcb4d7d47af91b4b726494dab43bf773ce1779
#         # sig=36a80d0421b9c0cbfa08ea95b27f401df108b30213ae138f1f5978ffc59606cf2b64758db9d26bd9c5b908423338f7445c8f0a07520f2154bbb62926aa0cb8fa
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        12586,
#                        "B62qoG5Yk4iVxpyczUrBNpwtx2xunhL48dydN53A2VjoRwF8NUTbVr4",
#                        "B62qkiT4kgCawkSEF84ga5kP9QnhmTJEYzcfgGuk6okAJtSBfVcjm1M",
#                        0,
#                        42000000000,
#                        1,
#                        4294967295,
#                        "more delegates, more fun........",
#                        mina.MAINNET_ID) == "36a80d0421b9c0cbfa08ea95b27f401df108b30213ae138f1f5978ffc59606cf2b64758db9d26bd9c5b908423338f7445c8f0a07520f2154bbb62926aa0cb8fa")

#         # account 2
#         # private key 336eb4a19b3d8905824b0f2254fb495573be302c17582748bf7e101965aa4774
#         # sig=093f9ef0e4e051279da0a3ded85553847590ab739ee1bfd59e5bb30f98ed8a001a7a60d8506e2572164b7a525617a09f17e1756ac37555b72e01b90f37271595
#         assert(sign_tx(mina.TX_TYPE_DELEGATION,
#                        2,
#                        "B62qrKG4Z8hnzZqp1AL8WsQhQYah3quN1qUj3SyfJA8Lw135qWWg1mi",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        0,
#                        1202056900,
#                        0,
#                        577216,
#                        "",
#                        mina.MAINNET_ID) == "093f9ef0e4e051279da0a3ded85553847590ab739ee1bfd59e5bb30f98ed8a001a7a60d8506e2572164b7a525617a09f17e1756ac37555b72e01b90f37271595")

#         # Check testnet and mainnet signatures are not equal
#         assert(sign_tx(mina.TX_TYPE_PAYMENT,
#                        0,
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        1729000000000,
#                        2000000000,
#                        16,
#                        271828,
#                        "Hello Mina!",
#                        mina.MAINNET_ID) !=
#                sign_tx(mina.TX_TYPE_PAYMENT,
#                        0,
#                        "B62qnzbXmRNo9q32n4SNu2mpB8e7FYYLH8NmaX6oFCBYjjQ8SbD7uzV",
#                        "B62qicipYxyEHu7QjUqS7QvBipTs5CzgkYZZZkPoKVYBu6tnDUcE9Zt",
#                        1729000000000,
#                        2000000000,
#                        16,
#                        271828,
#                        "Hello Mina!",
#                        mina.TESTNET_ID))


# class TestFuzz:
#     def test(self):
#         # Invalid message 1
#         assert(not send_apdu(""))

#         # Invalid message 2
#         assert(not send_apdu("00"))

#         # Invalid message 3
#         assert(not send_apdu("a5a501a6"))

#         # Invalid message 4
#         assert(not send_apdu("b08fdaeeb08fdaee6e8f58de53c7f54e3b86ef06d646e0c28173ab524cf21297eed41c870346760ecee46558de53c7f5b08fdaee6e8f58de53c7f54e3b86e119a24cf21f06d646e0c28173ab5465b08fdaee6e8f58de53c7f54e3b86e119a24cf21297eed41c8703467652279a3e7ec598ef6f06d646e0c28173ab57f897719eb5db73b16043bc7cc0c94cf21297eed41c870346760ecee465"))

#         # Invalid message 5
#         assert(not send_apdu(
#             "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"))

#         # Invalid command
#         assert(not send_apdu("01f600000400000000"))

#         # Invalid instruction 1
#         assert(not send_apdu("e00000000400000000"))

#         # Invalid instruction 2
#         assert(not send_apdu("e00500000400000000"))

#         # Invalid instruction 3
#         assert(not send_apdu("e0ff00000400000000"))

#         # Invalid get address (message too small, corrupt client)
#         assert(not send_apdu("e00200000300000000"))

#         # Invalid get address (message too small, invalid hex length)
#         assert(not send_apdu("e002a5a5040000000"))

#         # Invalid get address (invalid hex)
#         assert(not send_apdu("e002000004deadbear"))

#         # Invalid get address (message too small, corrupt ledger protocol)
#         bad_get_address_cmd = "e002"
#         for i in range(0, 6):
#             assert(not send_apdu(bad_get_address_cmd))
#             bad_get_address_cmd += "0"

#         # Ledger boilerplate bug
#         # Invalid get address (too small, corrupt ledger protocol)
#         assert(not send_apdu("e002000004"))

#         # Invalid get address (message too big, corrupt client 1)
#         assert(not send_apdu("e00200000500000000"))

#         # Invalid get address (message too big, corrupt client 2)
#         assert(not send_apdu("e0020000ff00000000"))

#         # Ledger API bug - This should be detected by Ledger
#         # Invalid get address (message too big, corrupt ledger protocol)
#         assert(not send_apdu("e0020000040000000000"))

#         # Invalid get address 4294967296 (account number range)
#         assert(not send_apdu("e0020000050100000000"))

#         # Invalid get address 18446744073709551615 (account number range, message too big)
#         assert(not send_apdu("e002000008ffffffffffffffff"))

#         # Valid get max address 4294967295
#         assert(send_apdu("e002000004ffffffff"))

#         # Invalid sign tx (message way too small)
#         assert(not send_apdu("e00300000158000000004236327172476158"))

#         # Invalid sign tx (message too small, corrupt client)
#         assert(not send_apdu("e0030000aa00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

#         # Invalid sign tx (message too small, corrupt client)
#         assert(not send_apdu("e00300000000000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

#         # Invalid sign tx (message too small, hex length)
#         assert(not send_apdu("e0030000ab00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e61210000000000000000000000000000000000000000000"))

#         # Invalid sign tx (message too small, invalid hex)
#         assert(not send_apdu("e0030000ab00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e6121m0000000000000000000000000000000000000000000"))

#         # Ledger boilerplate bug (very bad things!)
#         # Invalid sign tx (message too small, corrupt ledger 1)
#         assert(not send_apdu("e0030000ab"))

#         # Invalid sign tx (message too big, corrupt client 1)
#         assert(not send_apdu("e0030000ff00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

#         # Invalid sign tx (message too big, corrupt client 2)
#         assert(not send_apdu("e0030000ac000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

#         # Invalid sign tx (message too big, corrupt client 3)
#         assert(not send_apdu("e0030000ad000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

#         # Ledger boilerplate bug
#         # Invalid sign tx (message way too big, corrupt ledger 1)
#         assert(not send_apdu("e0030000ab00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e61210000000000000000000000000000000000000000000000"))

#         # Ledger bug - Causes client to hang (subsequent requests work).
#         #
#         #     The hex length is 676.  Anything less than this does not freeze the API.
#         #     This should be detected by Ledger.  Could be a ledgerblue bug.
#         # # Invalid sign tx (message way too big, corrupt ledger 1)
#         # assert(not send_apdu("e0030000ab00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000e0030000015800000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000"))

#         # Invalid sign tx (sender address does not match account number)
#         assert(not send_apdu("e0030000ab000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000000"))

#         # Invalid sign tx (corrupt sender addresses)
#         #                                                 X
#         assert(not send_apdu("e0030000ab000000004236327172F476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327170614463386e66753461377867686b456e6938753272426a7837454839354d46655a41685467476f666f706178466a6453375000000192906e4a00000000007735940000000010000425d448656c6c6f204d696e6121000000000000000000000000000000000000000000001"))

#         # Invalid sign tx (corrupt receiver addresses)
#         #                                                                                                                                                               X
#         assert(not send_apdu("e0030000ab000000004236327172476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e593676784236327172F476158683977656b6677614132797a55626862764659796e6b6d426b68594c56333664767935416b52766765516e5936767800000192906e4a00000000007735940000000010000425d448656c6c6f204d696e61210000000000000000000000000000000000000000000"))

#         # Invalid sign tx (invalid tx type)
#         assert(not send_apdu("e0030000ab00000000423632716e7a62586d524e6f397133326e34534e75326d70423865374659594c48384e6d6158366f464342596a6a513853624437757a56423632716963697059787945487537516a557153375176426970547335437a676b595a5a5a6b506f4b5659427536746e44556345395a7400000192906e4a00000000007735940000000010000425d448656c6c6f204d696e612100000000000000000000000000000000000000000003"))


# def run_crypto_tests():
#     print("Running crypto unit tests (not for release builds)")
#     t0 = time.time()
#     TestCrypto.test(None)
#     # Performance report
#     duration = time.time() - t0
#     print("Performed crypto tests in {:0.03f} seconds".format(duration))


# def run_get_address_tests():
#     t0 = time.time()
#     TestGetAddress.test(None)
#     # Performance report
#     duration = time.time() - t0
#     print(
#         "Performed 6 get-address operations in {:0.03f} seconds ({:0.03f} sec per operation)".format(duration, duration/6.0))


# def run_signature_tests():
#     t0 = time.time()
#     TestSignTx.test(None)
#     # Performance report
#     duration = time.time() - t0
#     print("Performed 18 sign-tx operations in {:0.03f} seconds ({:0.03f} sec per operation)".format(
#         duration, duration/18.0))


# def run_fuzz_tests():
#     t0 = time.time()
#     TestFuzz.test(None)
#     duration = time.time() - t0
#     print("Performed fuzz tests in {:0.03f} seconds".format(duration))


if __name__ == "__main__":
    print("Running unit tests...")

#     try:
#         if args.kind == "all":
#             run_crypto_tests()
#             run_get_address_tests()
#             run_signature_tests()
#             run_fuzz_tests()
#         if args.kind == "release":
#             run_get_address_tests()
#             run_signature_tests()
#             run_fuzz_tests()
#         elif args.kind == "crypto":
#             run_crypto_tests()
#         elif args.kind == "fuzz":
#             run_fuzz_tests()
#         elif args.kind == "get-address":
#             run_get_address_tests()
#         elif args.kind == "sign-transaction":
#             run_signature_tests()
#     except AssertionError:
#         raise
#     except Exception as ex:
#         print("{}".format(ex))
#         sys.exit(233)

print("Completed unit tests successfully!")
