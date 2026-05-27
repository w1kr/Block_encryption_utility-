cd build
cmake ..
cmake --build .

KEY=FF89DDCCBBAA998877665544332211F0F0F1F2F311F5F6F7F8F9FAFBFCFDFE4F
   #FF89DDCCBBAA998877665544332211F0F0F1F2F311F5F6F7F8F9FAFBFCFDFE4F

./App encrypt ../files/input.txt ../files/cipher.bin $KEY
./App decrypt ../files/cipher.bin ../files/output.txt $KEY

diff ../files/input.txt ../files/output.txt
