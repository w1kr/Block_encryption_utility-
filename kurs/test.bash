cd build
cmake ..
cmake --build .

KEY=FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF

./App encrypt ../test_files/picture.jpg ../test_files/cipher_picture.bin $KEY
./App decrypt ../test_files/cipher_picture.bin ../test_files/picture_dec.jpg $KEY

diff ../files/input.txt ../files/output.txt


# in this place will be more the tests
