#!/bin/bash

OLD_STRING="<HOST_NAME>"
NEW_STRING="vcm-31263.vm.duke.edu"

DIRECTORY="./test_temp"

OUTPUT_DIRECTORY="./test_reqs"

find "$DIRECTORY" -type f -name "*.txt" | while read file; do
  new_file="$OUTPUT_DIRECTORY/${file#$DIRECTORY/}"
  sed "s/$OLD_STRING/$NEW_STRING/g" "$file" > "$new_file"
done
