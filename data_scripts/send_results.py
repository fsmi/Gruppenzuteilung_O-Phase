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
transport = AIOHTTPTransport(url=url, headers={"authorization": token})
gql_client = Client(transport=transport)

with open(args.result_file) as json_file:
    parsed = json.load(json_file)
    assert isinstance(parsed, dict)
    print(f"Successfully parsed JSON ({len(parsed)} entries).")

    assignment_list = [f'{{teamId:"{key}",groupId:"{value}"}}' for key, value in parsed.items()]

    print(f"Sending data to: {url}")

    i = 0
    while i < len(assignment_list):
        list_part_as_str = ",".join(assignment_list[i, min(i + 100, len(assignment_list))])

        query = gql(
            f"""
            mutation {{
                assignTeams(data: [{list_part_as_str}])
            }}
            """
        )

        result = gql_client.execute(query)
        assert result
        i += 100
