#!/usr/bin/sh

if [ -f "config.ini" ]; then
    # load config
    . ./config.ini
fi

echo "URL=$DATA_ENDPOINT\n"

curl --header "AUTH_TOKEN: $TOKEN" "$DATA_ENDPOINT" > $1
