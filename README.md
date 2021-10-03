# Chaotic Permutation Circuit
*WARNING: The code in this repository is intended solely for evaluation by hardware designers and cryptographers. It has not been studied sufficiently for production use. Please do NOT use this code for any project beyond evaluation and research.*

## Motivation
While researching cryptographic design I found a lack of efficient instructions in modern CPUs. The commonly used mix of additions, shifts and bitwise instructions need a lot of instruction invocations to produce the desired result, with each invocation doing relatively little work. Multiplication looks like a neat supplement, but on closer inspection turns out to be stuffed with undesirable qualities. Modern instruction sets have taken to adopt instructions designed to speed up specific algorithms. While these instructions could be used in new algorithms, they are often different between instruction sets, and it may be hard to make a substantially different algorithm without losing cryptographic properties. Furthermore none of the existing instructions do nearly as much work as they could reasonably do, usually because they end up limited on constraints of how many inputs and outputs a single instruction may have. I therefore concluded that a more efficient approach would be to design an instruction that does a lot of work within the usual instruction signature constraints.

## Reading guide
The full instruction can be found in the function `cpc_scalar` in the file `cpc.h`, an alternative implementation is available in the function `cpc`. The file `main.c` contains example use, but is not required reading. In the `cpc_scalar` function the two 128 bit inputs are first loaded into 64 bit variables, the `pi` constant is also loaded into 64 bit variables. Then the `a` input is permuted through 4 almost identical quarter rounds.

First the pi constant is xored into the state, only half of it is used, except for the third round where the other half is also used. Using the same constant means that it can be incorporated into the partial functions, the single difference breaks the symmetry so that swapping the two 64 bit halves produce completely different results, the position of the difference is chosen so that the partial functions can use the common constant and the special case can be taken care of with an xor instruction or modified input.

Then the bytes are shuffled, the shuffling pattern is chosen so that when a difference always spreads from one byte to its neighbours, any difference will spread to all 16 bytes in 3 quarter rounds. The result after 3 quarter rounds has significant statistical skew, the 4th quarter round smoothens this out.

Finally the bits are shuffled in 3 similar steps. Two neighbours to the same side are ored together, providing non-linearity, xored with a bit further away, and the destination bit. Picking all the bits on the same side of the destination ensures reversibility. The chosen distances, 4 left, 10 right, 7 left, ensures that any difference always spreads to the neighbour bytes. Keeping them relatively short hopefully makes less connection-spaghetti in a physical implementation.

After the 4 quarter-rounds the `b` input is xored into the state, this is practical as any real use case will likely need to mix in key material, or mix states between lanes. Choosing to do this xor after mixing makes the instruction easily useful in a Feistel cipher, where `b=cpc(a,b)` is self-reversing.

## Key points
* A single cryptographic instruction that is cheap and simple to integrate into modern processors.
* Useful as the primary building block in any type of symmetric primitive.
* Full mixing of 128 bits.
* May be implemented as a single 1/2, 1/4 or 1/8 instruction that can be called multiple times to simulate the full instruction, reducing die space consumption at the cost of speed.
* Operates on standard 128 bit registers. 1/8 instruction may be implemented using 32 bit registers (2 in, 2 out).
* Full instruction consists of approximately 4500 gates.
* Depth of full instruction is 25 binary gates.
* Designed for circuit layout with minimal wiring.
* Many competing algorithms may use the same instruction, cryptography is allowed to evolve without needing new hardware.
* Optional double instruction for doing even more work per instruction call.
* Demonstration function encrypts/decrypts 4 bytes per instruction invocation. Real world hardware performance expected to be between 1/4 and 16 bytes per cycle, depending on implementation and algorithm.

## Content
**cpc.h** - Instruction definition, written using x86 128 bit intrinsics. An alternate version using 64 bit integers is also supplied.

**main.c** - Example code, implements a generic key stretch function, and an example AES-CTR-like cipher function, and use those functions to encrypt and decrypt a string.

## Target practice
The `crypt1` cipher is provided as a concrete target for strength discussion. While using 640 bits of key, the cipher is obviously limited by a 256 bit meet-in-the-middle attack. My gut feeling is that there is some probability that the 256 bit limit will be broken, but that a practical attack is unlikely, so for sport I'll claim a strength of 192 bits.

## Request for comments
The cost and performance numbers are my best guesses, someone experienced in chip design will probably be able to provide better numbers and possibly direct comparison to other cryptography hardware.

Any insight into the cryptographic properties of the instruction is of course most welcome, along with suggestions for CPC-based primitives and cryptanalysis of those.

Suggestions for improvements to the instruction are also welcome, but please bear in mind that the design must balance cryptographic performance with hardware cost, while maintaining decent software performance.

Feel free to use the Github issue tracker for questions and comments.

## Version R2 changes from R1

Changed shuffling pattern, mixing now happens faster and as a result the output is cleaner.

Changed the third block XOR constant in order to break the symmetry-preserving nature of the new shuffling pattern.
