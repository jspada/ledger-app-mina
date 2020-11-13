#!/usr/bin/env python

import ledgerblue.comm as ledgerblue
import argparse
import sys
import json
import requests
import binascii

COIN = 1000000000

construction_payloads_request = json.loads(r"""{"network_identifier":{"blockchain":"coda","network":"debug"},"operations":[{"operation_identifier":{"index":0},"related_operations":[],"type":"fee_payer_dec","status":"Pending","account":{"address":"B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g","metadata":{"token_id":"1"}},"amount":{"value":"-2000000000","currency":{"symbol":"CODA","decimals":9}}},{"operation_identifier":{"index":1},"related_operations":[],"type":"payment_source_dec","status":"Pending","account":{"address":"B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g","metadata":{"token_id":"1"}},"amount":{"value":"-5000000000","currency":{"symbol":"CODA","decimals":9}}},{"operation_identifier":{"index":2},"related_operations":[{"index":1}],"type":"payment_receiver_inc","status":"Pending","account":{"address":"B62qoDWfBZUxKpaoQCoFqr12wkaY84FrhxXNXzgBkMUi2Tz4K8kBDiv","metadata":{"token_id":"1"}},"amount":{"value":"5000000000","currency":{"symbol":"CODA","decimals":9}}}],"metadata":{"sender":"B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g","nonce":"0","token_id":"1"},"public_keys":[]}""")

construction_metadata_request = json.loads(r"""{"network_identifier":{"blockchain":"coda","network":"debug"},"options":{"sender":"B62qrPN5Y5yq8kGE3FbVKbGTdTAJNdtNtB5sNVpxyRwWGcDEhpMzc8g","token_id":"1"},"public_keys":[]}""")

parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers(dest='operation')
subparsers.required = True
get_address_parser = subparsers.add_parser('get-address')
get_address_parser.add_argument('account_number', help='BIP32 account to retrieve. e.g. 12.')
send_payment_parser = subparsers.add_parser('send-payment')
send_payment_parser.add_argument('sender_bip44_account', help='BIP32 account to send from (e.g. 12)')
send_payment_parser.add_argument('sender_address', help='Coda address of sender')
send_payment_parser.add_argument('receiver', help='Coda address of recipient')
send_payment_parser.add_argument('amount', help='Payment amount you want to send')
send_payment_parser.add_argument('--rosetta_server', help='Rosetta server (default http://localhost:3087)')
send_payment_parser.add_argument('--fee', help='Max fee')

args = parser.parse_args()

try:
        dongle = ledgerblue.getDongle(True)

        if args.operation == 'get-address':
                # Create APDU message.
                # CLA 0xE0  CLA
                # INS 0x02  INS_GET_ADDR
                # P1 0x00   UNUSED
                # P2 0x00   UNUSED
                account = '{:08x}'.format(int(args.account_number))
                apduMessage = 'E0020000' + '{:08x}'.format(len(account) + 4) + account
                apdu = bytearray.fromhex(apduMessage)
                print("Getting address...")
                address = dongle.exchange(apdu).decode('utf-8').rstrip('\x00')
                print('Received: {}'.format(address))

        elif args.operation == "send-payment":
                if args.rosetta_server is None:
                        args.rosetta_server = "http://localhost:3087"

                # Lookup the senders nonce and suggested fee
                construction_metadata_request["options"]["sender"] = args.sender_address;

                resp = requests.post(args.rosetta_server + '/construction/metadata',
                                     data=json.dumps(construction_metadata_request)).json()
                if "metadata" not in resp:
                        print("Failed to get metadata")
                        raise
                nonce = resp["metadata"]["nonce"];
                fee = float(resp["suggested_fee"][0]["value"])/COIN
                if args.fee is not None:
                    fee = float(args.fee)

                amount = float(args.amount)

                # Fee details
                construction_payloads_request["operations"][0]["account"]["address"] = args.sender_address;
                construction_payloads_request["operations"][0]["amount"]["value"] = '-{}'.format(int(fee*COIN))

                # Payment source
                construction_payloads_request["operations"][1]["account"]["address"] = args.sender_address;
                construction_payloads_request["operations"][1]["amount"]["value"] = '-{}'.format(int(amount*COIN));

                # Receiver
                construction_payloads_request["operations"][2]["account"]["address"] = args.receiver;
                construction_payloads_request["operations"][2]["amount"]["value"] = '{}'.format(int(amount*COIN));

                print()
                print("Payment details:")
                print("    Sender:   {}".format(construction_payloads_request["operations"][0]["account"]["address"]))
                print("    Amount:   {:.9f}".format(amount))
                print("    Nonce:    {}".format(nonce))
                print("    Fee:      {:.9f}".format(fee))
                print("    Receiver: {}".format(construction_payloads_request["operations"][2]["account"]["address"]))
                print()

                while True:
                        answer = str(input("Continue? (y/N) ")).lower().strip()
                        if answer == 'y':
                                break
                        else:
                                sys.exit(211)

                # print(json.dumps(construction_payloads_request))
                resp = requests.post(args.rosetta_server + '/construction/payloads',
                                     data=json.dumps(construction_payloads_request)).json()

                if "unsigned_transaction" not in resp:
                        print("Failed to get unsigned transaction")
                        raise
                unsigned_tx = json.loads(resp["unsigned_transaction"])

                if "payment" not in unsigned_tx or unsigned_tx["payment"] == None:
                        print("Failed to get payment info")
                        raise
                payment = unsigned_tx["payment"]
                # Create APDU message.
                # CLA 0xE0  CLA
                # INS 0x03  INS_SIGN_PAYMENT_TX
                # P1 0x00   UNUSED
                # P2 0x00   UNUSED
                sender_bip44_account = '{:08x}'.format(int(args.sender_bip44_account))
                sender_address = payment["from"].encode().hex()
                receiver = payment["to"].encode().hex()
                amount = '{:016x}'.format(int(payment["amount"]))
                fee = '{:016x}'.format(int(payment["fee"]))
                nonce = '{:016x}'.format(int(payment["nonce"]))
                random_oracle_input = unsigned_tx["randomOracleInput"]

                total_len = len(sender_bip44_account) \
                            + len(sender_address) \
                            + len(receiver) \
                            + len(amount) \
                            + len(fee) \
                            + len(nonce) \
                            + len(random_oracle_input)

                apduMessage = 'E0030000' + '{:08x}'.format(total_len + 4) \
                              + sender_bip44_account \
                              + sender_address \
                              + receiver \
                              + amount \
                              + fee \
                              + nonce \
                              + random_oracle_input

                apdu = bytearray.fromhex(apduMessage)
                print("Signing payment transaction...")
                address = dongle.exchange(apdu).decode('utf-8').rstrip('\x00')
                print('Received: {}'.format(address))

                print("Sending payment...")

except ledgerblue.CommException as ex:
        if ex.sw == 26368:
                print("Ledger app not open")
        else:
                print("Failed to communicate with ledger device (error = {})".format(ex.sw))
        sys.exit(233)
except AssertionError:
        raise
except Exception as ex:
        raise
        print("Error {}".format(ex))
        sys.exit(233)
