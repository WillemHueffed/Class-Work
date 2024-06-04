#!/bin/bash

# Check if all required arguments are provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 BOOK_ID EDITION_NUMBER PUBLICATION_DATE"
    exit 1
fi

# Extract command-line arguments
BOOK_ID="$1"
ED_NUM="$2"
PUB_DATE="$3"
# Send POST request using curl
curl -X POST -d "ed_num=${ED_NUM}" -d "pub_date=${PUB_DATE}" localhost:3000/books/${BOOK_ID}/edtions
