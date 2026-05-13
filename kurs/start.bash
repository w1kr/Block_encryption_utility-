cd build
cmake ..
cmake --build .

./App encrypt ../files/input.txt ../files/cipher.bin FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF
./App decrypt ../files/cipher.bin ../files/output.txt FFEEDDCCBBAA99887766554433221100F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF

diff ../files/input.txt ../files/output.txt
