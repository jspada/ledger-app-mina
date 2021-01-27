#!/usr/bin/env python3

import ledgerblue.comm as ledgerblue
import argparse
import sys
import json
import requests
import binascii
import ctypes
import string
import os

COIN = 1000000000

MINA_URL = "http://localhost:3087"
NETWORK = "debug"
VERBOSE = False

TX_TYPE_PAYMENT    = 0x00
TX_TYPE_DELEGATION = 0x04
MAX_ACCOUNT_NUM    = ctypes.c_uint32(-1).value
MAX_AMOUNT         = ctypes.c_uint64(-1).value
MAX_VALID_UNTIL    = ctypes.c_uint32(-1).value
MAX_NONCE          = ctypes.c_uint32(-1).value
MAX_MEMO_LEN       = 32
ADDRESS_LEN        = 55

DONGLE = None

__version__ = "1.0.0"

def valid_account(num):
    try:
        value = int(num)
        if value < 0 or value > MAX_ACCOUNT_NUM:
            raise
    except:
        raise argparse.ArgumentTypeError("Must be in [0,{}]".format(MAX_ACCOUNT_NUM))
    return value

def valid_amount(amount):
    try:
        value = int(amount)
        if value < 0 or value > MAX_AMOUNT:
            raise
    except:
        raise argparse.ArgumentTypeError("Must be in [0,{}]".format(MAX_AMOUNT))
    return value

def valid_nonce(nonce):
    try:
        value = int(nonce)
        if value < 0 or value > MAX_NONCE:
            raise
    except:
        raise argparse.ArgumentTypeError("Must be in [0,{}]".format(MAX_NONCE))
    return value

def valid_address(id):
    def f(address):
        if address is None or len(address) != ADDRESS_LEN:
            raise argparse.ArgumentTypeError("{} address length must be {}".format(id, ADDRESS_LEN))
        else:
            return address
    return f

def valid_memo(memo):
    if memo is None or len(memo) > MAX_MEMO_LEN:
        raise argparse.ArgumentTypeError("Length must be at most {}".format(MAX_MEMO_LEN))
    else:
        return memo

def valid_valid_until(valid_until):
    try:
        value = int(valid_until)
        if value is None or value > MAX_VALID_UNTIL:
            raise argparse.ArgumentTypeError("Must be at most {}".format(MAX_VALID_UNTIL))
    except:
        return value

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--verbose', default=False, action="store_true", help='Verbose mode')
    parser.add_argument('--cstruct', default=False, action="store_true", help='Dump cstruct of ledger hex responses')
    parser.add_argument('--version', action='version', version='%(prog)s {version}'.format(version=__version__))
    subparsers = parser.add_subparsers(dest='operation')
    subparsers.required = True
    get_address_parser = subparsers.add_parser('get-address')
    get_address_parser.add_argument('account_number', type=valid_account, help='BIP44 account to retrieve. e.g. 42.')
    get_balance_parser = subparsers.add_parser('get-balance')
    get_balance_parser.add_argument('address', type=valid_address("Address"), help='Mina address')
    get_balance_parser.add_argument('--mina_url', help='Mina rosetta interface url (default http://localhost:3087)')
    get_balance_parser.add_argument('--network', help='Network override')
    send_payment_parser = subparsers.add_parser('send-payment')
    send_payment_parser.add_argument('sender_bip44_account', type=valid_account, help='BIP44 account to send from (e.g. 42)')
    send_payment_parser.add_argument('sender_address', type=valid_address("Sender"), help='Mina address of sender')
    send_payment_parser.add_argument('receiver', type=valid_address("Receiver"), help='Mina address of recipient')
    send_payment_parser.add_argument('amount', type=valid_amount, help='Payment amount you want to send')
    send_payment_parser.add_argument('--mina_url', help='Mina rosetta interface url (default http://localhost:3087)')
    send_payment_parser.add_argument('--network', help='Network override')
    send_payment_parser.add_argument('--fee', type=valid_amount, help='Fee override')
    send_payment_parser.add_argument('--nonce', type=valid_nonce, help='Nonce override')
    send_payment_parser.add_argument('--valid_until', type=valid_valid_until, help='Valid until')
    send_payment_parser.add_argument('--memo', type=valid_memo, help='Transaction memo (publicly visible)')
    delegate_payment_parser = subparsers.add_parser('delegate')
    delegate_payment_parser.add_argument('delegator_bip44_account', type=valid_account, help='BIP44 account of delegator (e.g. 42)')
    delegate_payment_parser.add_argument('delegator_address', type=valid_address("Delegator"), help='Address of delegator')
    delegate_payment_parser.add_argument('delegate', type=valid_address("Delegate"), help='Address of delegate')
    delegate_payment_parser.add_argument('--mina_url', help='Mina rosetta interface url (default http://localhost:3087)')
    delegate_payment_parser.add_argument('--network', help='Network override')
    delegate_payment_parser.add_argument('--fee', help='Fee override')
    delegate_payment_parser.add_argument('--nonce', help='Nonce override')
    delegate_payment_parser.add_argument('--valid_until', type=valid_valid_until, help='Valid until')
    delegate_payment_parser.add_argument('--memo', type=valid_memo, help='Transaction memo (publicly visible)')
    test_transaction_parser = subparsers.add_parser('test-transaction')
    test_transaction_parser.add_argument('account_number', type=valid_account, help='BIP44 account generate test transaction with. e.g. 42.')
    test_transaction_parser.add_argument('account_address', type=valid_address("Account"), help='Mina address corresponding to BIP44 account')
    test_transaction_parser.add_argument('--interactive', default=False, action="store_true", help='Interactive mode')

    args = parser.parse_args()
    VERBOSE = args.verbose

def rosetta_network_request():
    network_request = json.loads(r"""{
        "metadata": {}
    }""")

    # Query
    print("Getting network identifier... ", end="", flush=True)
    network_resp = requests.post(MINA_URL + '/network/list',
                                data=json.dumps(network_request)).json()

    # Validate response
    if "network_identifiers" not in network_resp:
        print("error")
        if VERBOSE:
            print("\nNETWORK_RESP = {}\n".format(network_resp))
        raise Exception("Failed to get network identifiers")
    network_identifiers = network_resp["network_identifiers"]
    if len(network_identifiers) < 1:
        print("error", flush=True)
        if VERBOSE:
            print("\nNETWORK_RESP = {}\n".format(network_resp))
        raise Exception("Empty network identifiers")
    identifier = network_identifiers[0]
    if identifier["blockchain"] != "coda":
        print("error", flush=True)
        if VERBOSE:
            print("\nNETWORK_RESP = {}\n".format(network_resp))
        raise Exception("Invalid blockchain {}".format(identifier["blockchain"]))
    print("{}".format(NETWORK), flush=True)

    return identifier["network"]

def rosetta_metadata_request(sender_address):
    construction_metadata_request = json.loads(r"""{
        "network_identifier": {
            "blockchain": "coda",
            "network": ""
        },
        "options": {
            "sender": "",
            "token_id": "1"
        },
        "public_keys": []
    }""")

    # Lookup the senders nonce and suggested fee
    construction_metadata_request["network_identifier"]["network"] = NETWORK
    construction_metadata_request["options"]["sender"] = sender_address;

    print("Getting account nonce and suggested fee... ", end="", flush=True)
    metadata_resp = requests.post(MINA_URL + '/construction/metadata',
                                  data=json.dumps(construction_metadata_request)).json()
    if "metadata" not in metadata_resp:
        print("error", flush=True)
        if VERBOSE:
            print("\nMETADATA_RESP = {}\n".format(metadata_resp))
        raise Exception("Failed to get metadata")
    if "suggested_fee" not in metadata_resp:
        print("error", flush=True)
        if VERBOSE:
            print("\nMETADATA_RESP = {}\n".format(metadata_resp))
        raise Exception("Failed to get suggested fee")
    print("done", flush=True)

    nonce = int(metadata_resp["metadata"]["nonce"]);
    fee = int(metadata_resp["suggested_fee"][0]["value"])

    return (nonce, fee)

def rosetta_balance_request(address):
    account_balance_request = json.loads(r"""{
        "network_identifier": {
            "blockchain": "coda",
            "network": ""
        },
        "account_identifier": {
            "address": ""
        },
        "currencies": [
            {
                "symbol": "CODA",
                "decimals": 9,
                "metadata": {}
            }
        ]
    }""")

    account_balance_request["network_identifier"]["network"] = NETWORK
    account_balance_request["account_identifier"]["address"] = address

    print("Getting account balance... ", end="", flush=True)
    balance_resp = requests.post(MINA_URL + '/account/balance',
                                data=json.dumps(account_balance_request)).json()

    if "balances" not in balance_resp:
        print("error")
        if VERBOSE:
            print("\nBALANCE_RESP = {}\n".format(balance_resp))
        raise Exception("Failed to get balance")
    print("done", flush=True)

    return int(balance_resp["balances"][0]["value"])

def rosetta_send_payment_payloads_request(sender, receiver, amount, fee, nonce):
    construction_payloads_request = json.loads(r"""{
        "network_identifier": {
            "blockchain": "coda",
            "network": ""
        },
        "operations": [
            {
                "operation_identifier": {
                    "index": 0
                },
                "related_operations": [],
                "type": "fee_payer_dec",
                "status": "Pending",
                "account": {
                    "address": "",
                    "metadata": {
                        "token_id": "1"
                    }
                },
                "amount": {
                    "value": "",
                    "currency": {
                        "symbol": "CODA",
                        "decimals": 9
                    }
                }
            },
            {
                "operation_identifier": {
                    "index": 1
                },
                "related_operations": [],
                "type": "payment_source_dec",
                "status": "Pending",
                "account": {
                    "address": "",
                    "metadata": {
                        "token_id": "1"
                    }
                },
                "amount": {
                    "value": "",
                    "currency": {
                        "symbol": "CODA",
                        "decimals": 9
                    }
                }
            },
            {
                "operation_identifier": {
                    "index": 2
                },
                "related_operations": [
                    {
                        "index": 1
                    }
                ],
                "type": "payment_receiver_inc",
                "status": "Pending",
                "account": {
                    "address": "",
                    "metadata": {
                        "token_id": "1"
                    }
                },
                "amount": {
                    "value": "",
                    "currency": {
                        "symbol": "CODA",
                        "decimals": 9
                    }
                }
            }
        ],
        "metadata": {
            "sender": "",
            "nonce": "",
            "token_id": "1"
        },
        "public_keys": []
    }""")

    # Network
    construction_payloads_request["network_identifier"]["network"] = NETWORK

    # Fee details
    construction_payloads_request["operations"][0]["account"]["address"] = sender
    construction_payloads_request["operations"][0]["amount"]["value"] = '-{}'.format(fee)

    # Payment source
    construction_payloads_request["operations"][1]["account"]["address"] = sender
    construction_payloads_request["operations"][1]["amount"]["value"] = '-{}'.format(amount)

    # Receiver
    construction_payloads_request["operations"][2]["account"]["address"] = receiver
    construction_payloads_request["operations"][2]["amount"]["value"] = '{}'.format(amount)

    # Nonce
    construction_payloads_request["metadata"]["nonce"] = '{}'.format(nonce)

    print("Constructing unsigned payment transaction... ", end="", flush=True)
    payloads_resp = requests.post(MINA_URL + '/construction/payloads',
                                  data=json.dumps(construction_payloads_request)).json()
    if "unsigned_transaction" not in payloads_resp:
        print("error", flush=True)
        if VERBOSE:
            print("\nPAYLOADS_RESP = {}\n".format(payloads_resp))
        raise Exception("Failed to get unsigned transaction")
    payload = json.loads(payloads_resp["unsigned_transaction"])
    if "payment" not in payload or payload["payment"] == None:
        print("error", flush=True)
        if VERBOSE:
            print("\nPAYLOADS_RESP = {}\n".format(payloads_resp))
        raise Exception("Failed to get payment info")
    print("done", flush=True)

    return payload, payload["payment"]

def rosetta_delegation_payloads_request(delegator, delegate, fee, nonce):
    construction_payloads_request = json.loads(r"""{
        "network_identifier": {
            "blockchain": "coda",
            "network": ""
        },
        "operations": [
            {
                "operation_identifier": {
                    "index": 0
                },
                "related_operations": [],
                "type": "fee_payer_dec",
                "status": "Pending",
                "account": {
                    "address": "",
                    "metadata": {
                        "token_id": "1"
                    }
                },
                "amount": {
                    "value": "",
                    "currency": {
                        "symbol": "CODA",
                        "decimals": 9
                    }
                }
            },
            {
                "operation_identifier": {
                    "index": 1
                },
                "related_operations": [],
                "type": "delegate_change",
                "status": "Pending",
                "account": {
                    "address": ""
                },
                "metadata": {
                    "delegate_change_target": ""
                }
            }
        ],
        "metadata": {
            "sender": "",
            "nonce": "",
            "token_id": "1"
        },
        "public_keys": []
    }""")

    # Network
    construction_payloads_request["network_identifier"]["network"] = NETWORK

    # Fee details
    construction_payloads_request["operations"][0]["account"]["address"] = delegator
    construction_payloads_request["operations"][0]["amount"]["value"] = '-{}'.format(int(fee))

    # Delegate_change
    construction_payloads_request["operations"][1]["account"]["address"] = delegator
    construction_payloads_request["operations"][1]["metadata"]["delegate_change_target"] = delegate

    # Sender
    construction_payloads_request["metadata"]["sender"] = delegator

    # Nonce
    construction_payloads_request["metadata"]["nonce"] = '{}'.format(nonce)

    print("Constructing unsigned delegate transaction... ", end="", flush=True)
    payloads_resp = requests.post(MINA_URL + '/construction/payloads',
                                  data=json.dumps(construction_payloads_request)).json()
    if "unsigned_transaction" not in payloads_resp:
        print("error", flush=True)
        if VERBOSE:
            print("\nPAYLOADS_RESP = {}\n".format(payloads_resp))
        raise Exception("Failed to get unsigned transaction")
    payload = json.loads(payloads_resp["unsigned_transaction"])
    if "stakeDelegation" not in payload or payload["stakeDelegation"] == None:
        print("error", flush=True)
        if VERBOSE:
            print("\nPAYLOADS_RESP = {}\n".format(payloads_resp))
        raise Exception("Failed to get payment info") # TODO update all this
    print("done", flush=True)

    return payload, payload["stakeDelegation"]

def rosetta_combine_request(payload, signature, tx_type):
    construction_combine_request = json.loads(r"""{
        "network_identifier": {
            "blockchain": "coda",
            "network": ""
        },
        "unsigned_transaction": "",
        "signatures": [
            {
                "signing_payload": {
                    "address": "",
                    "account_identifer": "",
                    "hex_bytes": "",
                    "signature_type": ""
                },
                "signature_type": "schnorr_poseidon",
                "public_key": {
                    "hex_bytes": "",
                    "curve_type": ""
                },
                "hex_bytes": ""
            }
        ]
    }""")

    # Construct signed transaction
    construction_combine_request["network_identifier"]["network"] = NETWORK

    # Set unsigned transaction
    construction_combine_request["unsigned_transaction"] = json.dumps(payload)

    # Set signature hex_bytes
    construction_combine_request["signatures"][0]["hex_bytes"] = signature

    print("Constructing signed transaction... ", end="", flush=True)
    combine_resp = requests.post(MINA_URL + '/construction/combine',
                                 data=json.dumps(construction_combine_request)).json()

    if "signed_transaction" not in combine_resp:
        print("error")
        if VERBOSE:
            print("\nCOMBINE_RESP = {}\n".format(combine_resp))
        raise Exception("Failed to construct signed transaction")
    payload = json.loads(combine_resp["signed_transaction"])

    if tx_type == "send-payment":
        if "payment" not in payload or payload["payment"] == None:
            print("error")
            if VERBOSE:
                print("\nCOMBINE_RESP = {}\n".format(combine_resp))
                raise Exception("Failed to get payment info")
        signed_tx = payload["payment"]
    else:
        if "stake_delegation" not in payload or payload["stake_delegation"] == None:
            print("error")
            if VERBOSE:
                print("\nCOMBINE_RESP = {}\n".format(combine_resp))
                raise Exception("Failed to get stakeDelegation info")
        signed_tx = payload["stake_delegation"]
    print("done", flush=True)

    return payload, signed_tx

def rosetta_submit_request(signed_tx):
    construction_submit_request = json.loads(r"""{
        "network_identifier": {
        "blockchain": "coda",
        "network": ""
        },
        "signed_transaction": ""
    }""")

    construction_submit_request["network_identifier"]["network"] = NETWORK
    construction_submit_request["signed_transaction"] = json.dumps(signed_tx)

    print("Sending transaction... ", end="", flush=True)
    submit_resp = requests.post(MINA_URL + '/construction/submit',
                                data=json.dumps(construction_submit_request)).json()

    if "transaction_identifier" not in submit_resp:
        print("error")
        if VERBOSE:
            print("\nSUBMIT_RESP = {}\n".format(submit_resp))
        if "code" in submit_resp and "details" in submit_resp:
            raise Exception(submit_resp["details"]["body"][1])
        else:
            raise Exception("Failed to submit transaction")
    print("done", flush=True)

    return submit_resp["transaction_identifier"]["hash"]

def rosetta_parse_request(tx):
    construction_parse_request = json.loads(r"""{
        "network_identifier": {
            "blockchain": "coda",
            "network": ""
        },
        "transaction": ""
    }""")

    construction_parse_request["network_identifier"]["network"] = NETWORK
    construction_parse_request["transaction"] = json.dumps(tx)
    print(json.dumps(construction_parse_request))
    print("Checking transaction... ", end="", flush=True)
    parse_resp = requests.post(MINA_URL + '/construction/parse',
                                data=json.dumps(construction_parse_request)).json()

    if "operations" not in parse_resp:
        print("error")
        if VERBOSE:
            print("\nPARSE_RESP = {}\n".format(parse_resp))
        raise Exception("Failed to parse transaction")
    print("done", flush=True)

def ledger_init():
    global DONGLE
    try:
        DONGLE = ledgerblue.getDongle(debug=False)
    except ledgerblue.CommException as ex:
        if ex.sw == 26368:
            print("Ledger app not open")
        else:
            print("Failed to communicate with ledger device (error = {})".format(ex.sw))
            sys.exit(233)
    except AssertionError:
        raise
    except Exception as ex:
        print("Error: {}".format(ex))
        sys.exit(233)

def ledger_send_apdu(apdu_hex):
    if not len(apdu_hex):
        return

    apdu = bytearray.fromhex(apdu_hex)
    DONGLE.exchange(apdu)
    return True

def ledger_crypto_tests():
    return ledger_send_apdu("e004000000")

def ledger_get_address(account):
    # Create APDU message.
    # CLA 0xe0 CLA
    # INS 0x02 INS_GET_ADDR
    # P1  0x00 UNUSED
    # P2  0x00 UNUSED
    account = '{:08x}'.format(account)
    apduMessage = 'e0020000' + '{:02x}'.format(int(len(account)/2)) + account
    apdu = bytearray.fromhex(apduMessage)

    if VERBOSE:
        print("\napduMessage hex ({}) = {}\n".format(total_len + 4, apduMessage))

    return DONGLE.exchange(apdu).decode('utf-8').rstrip('\x00')

def ledger_sign_tx(tx_type, sender_account, sender_address, receiver, amount, fee, nonce, valid_until, memo):
    sender_bip44_account = '{:08x}'.format(int(sender_account))
    sender_address = sender_address.encode().hex()
    receiver = receiver.encode().hex()
    amount = '{:016x}'.format(int(amount))
    fee = '{:016x}'.format(int(fee))
    nonce = '{:08x}'.format(nonce)
    valid_until = '{:08x}'.format(valid_until)
    memo = memo.ljust(MAX_MEMO_LEN, '\x00')[:MAX_MEMO_LEN].encode().hex()
    tag = '{:02x}'.format(tx_type)

    total_len = len(sender_bip44_account) \
                + len(sender_address) \
                + len(receiver) \
                + len(amount) \
                + len(fee) \
                + len(nonce) \
                + len(valid_until) \
                + len(memo) \
                + len(tag)

    # Create APDU message.
    #     CLA 0xe0 CLA
    #     INS 0x03 INS_SIGN_TX
    #     P1  0x00 UNUSED
    #     P2  0x00 UNUSED
    apduMessage = 'e0030000' + '{:02x}'.format(int(total_len/2)) \
                  + sender_bip44_account \
                  + sender_address \
                  + receiver \
                  + amount \
                  + fee \
                  + nonce \
                  + valid_until \
                  + memo \
                  + tag

    if VERBOSE:
        print("\napduMessage hex ({}) = {}\n".format(total_len + 4, apduMessage))

    apdu = bytearray.fromhex(apduMessage)
    return DONGLE.exchange(apdu).decode('utf-8').rstrip('\x00')

def print_cstruct(str):
    revbytes = bytes.fromhex(str)[::-1]
    print("uint8_t bytes[{}] = {{".format(len(revbytes)), end="", flush=True)
    i = 0
    for byte in revbytes:
        if i % 8 == 0:
            print("\n    ", end="", flush=True)
        print("0x{:02x}, ".format(byte), end="", flush=True)
        i += 1
    print("\n};", flush=True)

def to_currency(amount):
    return float(amount)/COIN

def from_currency(amount):
    return int(float(amount)*COIN)

def tx_type_name(op):
    if op == "send-payment":
        return "Payment"
    elif op == "delegate":
        return "Delegation"
    elif op == "test-transaction":
        return "Test"

def tx_type_from_op(op):
    if op == "send-payment" or op == "test-transaction":
        return TX_TYPE_PAYMENT
    elif op == "delegate":
        return TX_TYPE_DELEGATION

def check_tx(tx_type, tx, sender, receiver, amount, fee, valid_until, nonce, memo):
    def common_tx_check(tx, fee, valid_until, nonce, memo):
        return tx["fee"] == str(fee) and \
               ((tx["valid_until"] is not None and \
               tx["valid_until"] == str(valid_until)) or \
               (tx["valid_until"] is None and valid_until == MAX_VALID_UNTIL)) and \
               tx["nonce"] == str(nonce) and \
               ((tx["memo"] is not None and tx["memo"] == memo) or \
               (tx["memo"] is None and memo == ""))

    if tx_type == TX_TYPE_PAYMENT:
        return tx["from"] == sender and \
               tx["to"] == receiver and \
               tx["amount"] == str(amount) and \
               common_tx_check(tx, fee, valid_until, nonce, memo)
    else:
        return tx["delegator"] == sender and \
               tx["new_delegate"] == receiver and \
               common_tx_check(tx, fee, valid_until, nonce, memo)

__all__ = [TX_TYPE_PAYMENT, TX_TYPE_DELEGATION, ledger_init, ledger_get_address, ledger_sign_tx]

if __name__ == "__main__":
    try:
        if args.operation == 'get-address':
            ledger_init()

            print("Get address for account {} (path 44'/12586'/\033[4m\033[1m{}\033[0m'/0/0)".format(args.account_number, args.account_number))

            answer = str(input("Continue? (y/N) ")).lower().strip()
            if answer != 'y':
                sys.exit(211)

            print("Generating address (please confirm on Ledger device)... ", end="", flush=True)
            address = ledger_get_address(int(args.account_number))
            print("done")
            print('Received address: {}'.format(address))

            if args.cstruct and all(c in string.hexdigits for c in address):
                print_cstruct(address)

        elif args.operation == "get-balance":
            if args.mina_url is not None:
                # mina url override
                print("Using rosetta override: {}".format(args.mina_url))
                MINA_URL = args.mina_url

            # Lookup the network
            NETWORK = rosetta_network_request()

            if args.network is not None:
                # mina url override
                print("Using network override: {}".format(args.network))
                NETWORK = args.network

            balance = rosetta_balance_request(args.address)

            print()
            print("Address: {}".format(args.address))
            print("Balance: {}".format(to_currency(balance)))
            print()

        elif args.operation == "send-payment" or args.operation == "delegate":
            ledger_init()

            # Set common user supplied parameters
            if args.operation == "send-payment":
                account = args.sender_bip44_account
                sender = args.sender_address
                receiver = args.receiver
                amount = from_currency(args.amount)
            elif args.operation == "delegate":
                account = args.delegator_bip44_account
                sender = args.delegator_address
                receiver = args.delegate
                amount = 0

            # Set optional memo
            memo = args.memo if args.memo is not None else ""
            memo = memo[:MAX_MEMO_LEN] if len(memo) > MAX_MEMO_LEN else memo

            # Optional valid_until override
            if args.valid_until is not None:
                print("Using valid_until override: {}".format(args.valid_until))
            valid_until = args.valid_until if args.valid_until is not None else ""

            if args.mina_url is not None:
                # mina url override
                print("Using rosetta override: {}".format(args.mina_url))
                MINA_URL = args.mina_url

            # Lookup the network
            NETWORK = rosetta_network_request()

            if args.network is not None:
                # mina url override
                print("Using network override: {}".format(args.network))
                NETWORK = args.network

            # Lookup the senders nonce and suggested fee
            nonce, fee = rosetta_metadata_request(sender)

            # Apply optional overrides
            if args.fee is not None:
                print("Using fee override: {}".format(args.fee))
                fee = from_currency(args.fee)
            if args.nonce is not None:
                print("Using nonce override: {}".format(args.nonce))
                nonce = int(args.nonce)

            balance = rosetta_balance_request(sender)

            if amount + fee > balance:
                if args.operation == "send-payment":
                    print("Total amount {} exceeds account balance {}".format(to_currency(amount + fee), to_currency(balance)))
                else:
                    print("Delegation fee {} exceeds account balance {}".format(to_currency(amount + fee), to_currency(balance)))
                sys.exit(211)

            print()
            print("Sign transaction:")
            print("    Type:        {}".format(tx_type_name(args.operation)))
            print("    Account:     {} (path 44'/12586'/\033[4m\033[1m{}\033[0m'/0/0)".format(account, account))

            if args.operation == "send-payment":
                print("    Sender:      {} (balance {})".format(sender, to_currency(balance)))
                print("    Receiver:    {}".format(receiver))
                print("    Amount:      {:.9f}".format(to_currency(amount)))
            else:
                print("    Delegator:   {} (balance {})".format(sender, to_currency(balance)))
                print("    Delegate:    {}".format(receiver))

            print("    Fee:         {:.9f}".format(to_currency(fee)))
            if args.operation == "send-payment":
                print("    Total:       {:.9f}".format(to_currency(amount + fee)))
            print("    Nonce:       {}".format(nonce))

            if valid_until != "":
                print("    Valid until: {}".format(valid_until))
            if memo != "":
                print("    Memo:        {}".format(memo))
            print()
            if args.operation == "delegate":
                print("    The entire balance will be delegated.")
                print()

            answer = str(input("Continue? (y/N) ")).lower().strip()
            if answer != 'y':
                sys.exit(211)
            print()

            # Construct payload and unsigned tx
            #    n.b. unsigned_tx is a conveniece reference into part of unsigned_payload
            if args.operation == "send-payment":
                unsigned_payload, unsigned_tx = rosetta_send_payment_payloads_request(sender, receiver, amount, fee, nonce)
            else:
                unsigned_payload, unsigned_tx = rosetta_delegation_payloads_request(sender, receiver, fee, nonce)

            # Load valid_until
            if valid_until == "":
                valid_until = unsigned_tx["valid_until"]
                if valid_until == None:
                    # None means valid forever
                    valid_until = MAX_VALID_UNTIL
            else:
                # Apply valid_until override
                unsigned_tx["valid_until"] = str(valid_until)

            if memo != "":
                # Apply memo override
                unsigned_tx["memo"] = memo

            if VERBOSE:
                print("\nUNSIGNED_TX = {}\n".format(json.dumps(unsigned_payload)))

            # # Rosetta parse
            # rosetta_parse_request(unsigned_tx)

            # Security check rosetta output 1
            if not check_tx(tx_type_from_op(args.operation),
                            unsigned_tx,
                            sender,
                            receiver,
                            amount,
                            fee,
                            valid_until,
                            nonce,
                            memo):
                raise Exception("Rosetta unsigned tx diverges from user supplied parameters")

            # Tell Ledger to construct, verify and sign the tx
            print("Signing transaction (please confirm on Ledger device)... ", end="", flush=True)
            signature = ledger_sign_tx(tx_type_from_op(args.operation),
                                       account,
                                       sender,
                                       receiver,
                                       amount,
                                       fee,
                                       nonce,
                                       valid_until,
                                       memo)
            print("done", flush=True)

            if VERBOSE:
                print("\nReceived signature: {}\n".format(signature))

            # Combine signature and unsigned_tx to create signed_tx
            #    n.b. signed_tx is a conveniece reference into part of signed_payload
            signed_payload, signed_tx = rosetta_combine_request(unsigned_payload, signature, args.operation)

            if valid_until != "":
                # Apply valid_until override (again)
                signed_tx["valid_until"] = str(valid_until)

            if memo != "":
                # Apply memo override (again)
                signed_tx["memo"] = memo

            if VERBOSE:
                print("\nSIGNED_TX   = {}\n".format(json.dumps(signed_payload)))

            # Rosetta parse
            # rosetta_parse_request(signed_tx)

            # Security check rosetta output 2
            if not check_tx(tx_type_from_op(args.operation),
                            signed_tx,
                            sender,
                            receiver,
                            amount,
                            fee,
                            valid_until,
                            nonce,
                            memo):
                raise Exception("Rosetta signed tx diverges from user supplied parameters")
            if signed_payload["signature"] != signature:
                raise Exception("Rosetta tx signature diverges")

            print()
            print("Send transaction:")
            print("    Type:        {}".format(tx_type_name(args.operation)))
            print("    Account:     {} (path 44'/12586'/\033[4m\033[1m{}\033[0m'/0/0)".format(account, account))

            if args.operation == "send-payment":
                print("    Sender:      {} (balance {})".format(signed_tx["from"], to_currency(balance)))
                print("    Receiver:    {}".format(signed_tx["to"]))
                print("    Amount:      {:.9f}".format(to_currency(signed_tx["amount"])))
            else:
                print("    Delegator:   {} (balance {})".format(signed_tx["delegator"], to_currency(balance)))
                print("    Delegate:    {}".format(signed_tx["new_delegate"]))

            print("    Fee:         {:.9f}".format(to_currency(signed_tx["fee"])))
            if args.operation == "send-payment":
                print("    Total:       {:.9f}".format(to_currency(amount + fee)))
            print("    Nonce:       {}".format(signed_tx["nonce"]))

            if valid_until != MAX_VALID_UNTIL:
                print("    Valid until: {}".format(valid_until))
            if memo != "":
                print("    Memo:        {}".format(memo))

            print("    Signature:   {}...".format(signed_payload["signature"][0:74]))
            print()

            if args.cstruct and all(c in string.hexdigits for c in signature):
                print_cstruct(signature)

            if args.operation == "delegate":
                print("    The entire balance will be delegated.")
                print()

            answer = str(input("Continue? (y/N) ")).lower().strip()
            if answer != 'y':
                sys.exit(211)
            print()

            tx_hash = rosetta_submit_request(signed_payload)

            print()
            print("Transaction id: {}".format(tx_hash))

        elif args.operation == "test-transaction":
            ledger_init()

            account = args.account_number
            address = args.account_address
            memo = os.urandom(16).hex()

            if len(memo) != 32:
                raise Exception("Failed to get memo entropy")

            if args.interactive:
                print()
                print("Generate test transaction:")
                print("    Account:     {} (path 44'/12586'/\033[4m\033[1m{}\033[0m'/0/0)".format(account, account))
                print("    Address:     {}".format(address))
                print()
                answer = str(input("Continue? (y/N) ")).lower().strip()
                if answer != 'y':
                    sys.exit(211)

            # Tell Ledger to construct, verify and sign the tx
            if args.interactive:
                print()
                print("Signing transaction (please confirm on Ledger device)... ", end="", flush=True)
            signature = ledger_sign_tx(tx_type_from_op(args.operation),
                                       account,
                                       address,
                                       address,
                                       0,
                                       0,
                                       0,
                                       0,
                                       memo)
            if args.interactive:
                print("done")

            test_transaction_json = json.loads(r"""{
                "signature": "",
                "payment": {
                    "to": "",
                    "from": "",
                    "fee": "0",
                    "token": "1",
                    "nonce": "0",
                    "memo": "",
                    "amount": "0",
                    "valid_until": "0"
                 },
                 "stake_delegation": null,
                 "create_token": null,
                 "create_token_account": null,
                 "mint_tokens": null
            }""")

            test_transaction_json["signature"] = signature
            test_transaction_json["payment"]["to"] = address
            test_transaction_json["payment"]["from"] = address
            test_transaction_json["payment"]["memo"] = memo

            if args.interactive:
                print("")

            print(json.dumps(test_transaction_json, indent=2))

    except ledgerblue.CommException as ex:
        if ex.sw == 26368:
            print("Ledger app not open")
        elif ex.sw == 27013:
            print("rejected")
        else:
            print("Failed to communicate with ledger device (error = {})".format(ex.sw))
            sys.exit(233)
    except AssertionError:
        raise
    except Exception as ex:
        print("Error: {}".format(ex))
        sys.exit(233)
