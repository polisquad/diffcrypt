# DiffCrypt

## Contributors

- Guglielmo Manneschi @ [nondecidibile](https://github.com/nondecidibile)
- Andrea Mecchia @ [sneppy](https://github.com/sneppy)

## Index

- [Using the BitArray class](#Using-the-BitArray-class)
- [Using DES](#Using-DES)

## Using the BitArray class

To create an uninitialized array of length k bits:

```cpp
BitArray ptx(k);
```

The array can be populated by providing a source buffer and the number of bits to read:

```cpp
char msg[] = "abcdefgh";
BitArray ptx(msg, 64);
```

The default constructor is conceptually identical to `BitArray(0)`

> Note that in this case no storage is initially allocated

To retrieve the length of the array use `getCount()`:

```cpp
printf("length: %u\n", ptx.getCount());
```

Single bit values can be accessed using the overloaded subscript operator `[]` which returns an unsigned byte:

```cpp
if (ptx[0] & ptx[1])
	; //
```

It is also possible to retrieve a pointer to the underlying buffer:

```cpp
auto data8 = ptx.getData();
auto data64 = ptx.getData<uint64>();
```

Two bit arrays can be compared using the overloaded operators `==` and `!=`:

```cpp
BitArray dx("abcdefgh", 64);
BitArray dy("abcdefgh", 64);

if (dx == dy)
	; //
```

A bit array can be sliced using either `slice()` or `slicebit()`:

```cpp
dl = ptx.slice(32, 0);
dr = ptx.slicebit(32, 64);
```

The former takes the length and a **BYTE** offset whereas the latter takes first and last bit. Conceptually `slice(32, 0)` and `slicebit(0, 32)` are the same, but `slicebit(12, 24)` cannot be performed with `slice()`.

Two bit arrays can be merged together using `merge()` and `append()`:

```cpp
BitArray output = dl.merge(dr);
// OR
BitArray output = dl;
output.append(dr);
```

Both functions perform the same operation, but `append()` modifies the object itself while `merge()` returns a new array without modifying the other two.

Bitwise xor is performed with the overloaded operator `^=` and `^`:

```cpp
BitArray dy = dr ^ dl;
dx ^= dy;
```

`rotateLeft()` rotates the bit array by `n` bits to the left:

```cpp
dx.rotateLeft(n);
```

`permute()` performs a permutation (or expansion):

```cpp
ptx.permute(dl, ip);
ptx.permute(dr, ip + 32);
```

The permuted vector is saved in the first argument (and a reference is returned). The second argument is a pointer to the permutation table. The length of the destination bit array (`dl` and `dr` in the example) determines the length of the permutation.

> In the example `ip` is an array of 64 integers. The first 32 are saved in `dl` while the second are saved in `dr`.

The same example above can be rewritten using a single `premute()` and two `slice()` operations:

```cpp
BitArray t(64);
ptx.permute(t, ip);
dl = t.slice(32, 0);
dr = t.slice(32, 4);
```

We can perform substitutions (i.e. model a substitution box) using `substitute()`:

```cpp
BitArray dx(48);
BitArray dy(32);

dx.substitute<6, 4>(dy, subs);
```

`substitute()` is a templated function. The template arguments describe the input and output size of a single substitution box. For example DES S-boxes transform 6-bit sequences in 4-bit sequences, thus `<6, 4>` is used. As with `permute()` the first argument is the destination bit array, while the second argument is a pointer to the substitution table.

To handle cases like DES where multiple S-boxes are present, an overload of `substitute` accepts an array of substitution tables and the number of S-boxes as a third argument:

```cpp
dx.substitute<6, 4>(dy, subs, 8);
```

> `subs[0]` is applied to `dx[0, 5]`, `subs[1]` to `dx[6, 11]` and so on

Many of this methods return a reference to the bit array, hence it is possible to concatenate multiple transformation and create a very compact transformation:

```cpp
(dr.permute(e, xpn) ^= key).substitute<6, 4>(u, subs, 8).permute(v, perm);
```

## Using DES

The `DES` class provides a quick way to encrypt/decrypt a DES block:

```cpp
// Generate round keys
BitArray roundKeys[16];
DES::std.keySchedule(roundKeys, BitArray("_mykey_!", 64));

// Encrypt plaintext block
BitArray ctx = DES::std.encryptBlock(BitArray("mysecret", 64), roundKeys);

// Decrypt ciptertext block
BitArray ptx = DES::std.decryptBlock(ctx, roundKeys);
```

`DES::std` uses standard DES tables. The DES structure can be customized by providing different tables for permutations and sboxes, as well as different number of rounds:

```cpp
// Lightweight DES with only 8 rounds
DES des{Des::std};
des.numRounds = 8;

des.keySchedule(roundKeys, BitArray("_mykey_!", 64));
// ...
```