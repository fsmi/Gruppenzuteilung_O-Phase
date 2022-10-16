#!/usr/bin/sh

if [ -f "config.ini" ]; then
    # load config
    . ./config.ini
fi

echo "URL=$DATA_ENDPOINT\n"

curl -f --header "authorization: $TOKEN" "$DATA_ENDPOINT" > $1
