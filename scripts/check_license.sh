#!/usr/bin/env bash

path_to_vttv=${1}
cd "$path_to_vttv" || exit 1

for sub_dir in "src" "tests/unit" "examples"
do
  "$path_to_vttv/scripts/add-license-perl.pl" "$path_to_vttv/$sub_dir" "$path_to_vttv/scripts/license-template"
done

result=$(git diff --name-only)

if [ -n "$result" ]; then
  echo -e "Following files have incorrect license!\n"
  echo "$result"
  exit 1
fi
