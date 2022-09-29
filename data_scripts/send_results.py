#!/usr/bin/python3

import argparse
import json

from gql import gql, Client
from gql.transport.aiohttp import AIOHTTPTransport

parser = argparse.ArgumentParser()
parser.add_argument("result_file", type=str)
parser.add_argument("--config", type=str, default="config.ini")

args = parser.parse_args()

url = None
token = None
with open(args.config) as config:
    for line in config.readlines():
        if "ASSIGNMENT_ENDPOINT" in line:
            url = line.split("ASSIGNMENT_ENDPOINT=")[1].strip()
        if "TOKEN" in line:
            token = line.split("TOKEN=")[1].strip()

assert url, "Missing 'ASSIGNMENT_ENDPOINT' in config!"
assert token, "Missing 'TOKEN' in config!"

# select transport with a defined url endpoint
transport = AIOHTTPTransport(url=url)
gql_client = Client(transport=transport)

with open(args.result_file) as json_file:
    parsed = json.load(json_file)
    assert isinstance(parsed, dict)
    print(f"Successfully parsed JSON ({len(parsed)} entries).")

    assignment_list = [f'{{teamId:"{key}",groupId:"{value}"}}' for key, value in parsed.items()]
    list_as_str = ",".join(assignment_list)

    query = gql(
        f"""
        mutation {{
            assignTeams(data: [{list_as_str}])
        }}
        """
    )

    print(f"Sending data to: {url}")

    result = gql_client.execute(query)
    assert result
