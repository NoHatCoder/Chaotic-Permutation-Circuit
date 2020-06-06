# Chaotic Permutation Circuit
*WARNING: The code in this repository is intended solely for evaluation by hardware designers and cryptographers. It has not been studied sufficiently for production use. Please do NOT use this code for any project beyond evaluation and research.*

## Key points
* A single cryptographic instruction that is cheap and simple to integrate into modern processors.
* Useful as the primary building block in any type of symmetric primitive.
* Full mixing of 128 bits.
* May be implemented as a single 1/2, 1/4 or 1/8 instruction that can be called multiple times to simulate the full instruction, reducing die space consumption at the cost of speed.
* Operates on standard 128 bit registers. 1/8 instruction may be implemented using 32 bit registers (2 in 2 out).
* Full instruction consists of approximately 4500 gates.
* Depth of full instruction is 25 binary gates.
* Designed for circuit layout with minimal wiring.
* Many competing algorithms may use the same instruction, cryptography is allowed to evolve without needing new hardware.
* Optional double instruction for doing even more work per instruction call.
* Demonstration function encrypts/decrypts 4 bytes per instruction invocation. Real world hardware performance expected to be between 1/4 and 16 bytes per cycle, depending on implementation and algorithm.

## Content
**cpc.h** - Instruction definition, written using x86 128 bit intrinsics.

**main.c** - Example code, implements a generic key stretch function, and an example AES-CTR-like encryption function, and use those functions to encrypt and decrypt a string.

## Request for comments
The cost and performance numbers are my best guesses, someone experienced in chip design will probably be able to provide better numbers and possibly direct comparison to other cryptography hardware.

Any insight into the cryptographic properties of the instruction is of course most welcome, along with suggestions for CPC-based primitives and cryptanalysis of those.

Suggestions for improvements to the instruction are also welcome, but please bear in mind that the design must balance cryptographic performance with hardware cost, while maintaining decent software performance.

Feel free to use the Github issue tracker for questions and comments.

## Version R2 changes from R1

Changed shuffling pattern, mixing now happens faster and as a result the output is cleaner.

Changed the third block XOR constant in order to break the symmetry-preserving nature of the new shuffling pattern.
