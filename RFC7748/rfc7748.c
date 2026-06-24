/*
Code focused on a simple implementation of the RFC 7748
This code is provided "as is" without any warranty of any kind, 
either express or implied, including but not limited to the 
warranties of merchantability, fitness for a particular 
purpose and noninfringement. In no event shall the authors or 
copyright holders be liable for any claim, damages or other 
liability, whether in an action of contract, tort or otherwise, 
arising from, out of or in connection with the software or the 
use or other dealings in the software.

Therefore the code is provided for educational purposes only and 
should not be used in production systems without proper review and testing.

The main purpose of this code is to demonstrate the implementation of the RFC 7748, 
which defines the elliptic curve Diffie-Hellman (ECDH) key exchange protocol 
using Curve25519 and Curve448. The code includes functions for generating 
public and private keys, performing key exchange, and deriving shared secrets.

While the code is more focused on a simple implementation of the RFC778 the main purpose 
focuses on the conditional swap (CSAWP) operation protected and un protected against side-channel attacks. 
The CSAWP operation is a critical component of the ECDH key exchange protocol, and with the
use of ChipWhisperer tool Husky and several targets as SAM4S, STM32F3, STM32F4, STM32L5 to demostrate 
the side-channel attacks and the effectiveness of the CSAWP operation in protecting against them.
*/

#include <stdio.h>
#include <stdint.h>

const int a = 486662;
const int p = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED;
const int u = 9;
const int v = 43114425171068552920764898935933967039370386198203806730763910166200978582548;



int main() {
    printf("This code is a simple implementation of RFC 7748 for educational purposes.\n");
    return 0;
}

