cd build
cmake ..
cmake --build .

# KEY=FAEEDDCCBBAA99887766554433771190F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF
#   FF89DDCCBBAA998877665544332211F0F0F1F2F311F5F6F7F8F9FAFBFCFDFE4F

# ./App encrypt ../test_files/picture.jpg ../test_files/cipher_picture.bin $KEY
# ./App decrypt ../test_files/cipher_picture.bin ../test_files/picture_dec.jpg $KEY

# diff ../test_files/picture.jpg ../test_files/picture_dec.jpg

# in this place will be more the tests


# Correctness test
# ./App test

# Benchmark
# ./App benchmark

# Crypto resistance
./App resistance
