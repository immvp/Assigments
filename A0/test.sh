#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make


echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*


echo "Generating test files.."
printf "Hello, World!\n" > test_files/ascii.input
printf "Hello, World!" > test_files/ascii2.input
printf "Hello,\x00World!\n" > test_files/data.input
printf "" > test_files/empty.input
### TODO: Generate more test files ###
# uses 2 header bytes after each other
printf "\xf0\xf0" > test_files/iso_1.input
# could be utf, could be iso (same pattern) - we go with utf
printf "\xd7\xa2" > test_files/iso_2.input
printf "\x20\x20\xd7\xa2\x20\x20" > test_files/iso_3.input
# double continous, must be ascii
printf "\x6f\x6f" > test_files/ascii3.input
#only contains ascii .. must be ascii
printf "abc" > test_files/ascii4.input
# ȁ - uses unused bytes, between 128-159 decimal values
printf "\x20\x20\xc4\x9f\x20\x20" > test_files/utf_1.input
#Contains byte 2 utf pattern. We assume its utf.
printf "\x20\x20\xc6\xa3\x20\x20" > test_files/utf_2.input
#Contains byte 3 utf pattern. We assume its utf.
printf "\x20\x20\xe6\xa3\xa3\x20\x20" > test_files/utf_3.input
#Contains byte 4 utf pattern. We assume its utf.
printf "\x20\x20\xf0\xa3\xa3\xa3\x20\x20" > test_files/utf_4.input



echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    ${f} | sed -e 's/ASCII text.*/ASCII text/' \
                         -e 's/UTF-8 Unicode text.*/UTF-8 Unicode text/' \
                         -e 's/ISO-8859 text.*/ISO-8859 text/' \
                         -e 's/writable, regular file, no read permission/cannot determine (Permission denied)/' \
                         > "${f}.expected"
  ./file  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done
exit $exitcode
