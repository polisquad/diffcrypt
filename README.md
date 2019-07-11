# DiffCrypt

## Contributors

- Guglielmo Manneschi @ [nondecidibile](https://github.com/nondecidibile)
- Andrea Mecchia @ [sneppy](https://github.com/sneppy)

## How to use this code

This code contains a custom bit vector class with methods that implement transformation components (e.g. permutation, expansion, substitution, addition, rotation) as well as search algorithms for DES differential cryptanalysis.

A compliant DES encryption can be easily obtained using the `BitArray` class methods:

```cpp
BitArray input("plaintxt", 64), output(64);
BitArray l, r;
BitArray u(32), v(32);
BitArray key("mysecret", 48), e(48);
BitArray keys[16];

keySchedule(keys, key);

input.permute(l, ip);
input.permute(r, ip + 32);

for (uint32 i = 0; i < 15; ++i)
{
	// DES round
	(r.permute(e, xpn) ^= keys[i]).substitute<6, 4>(u, subs, 8).permute(v, perm) ^= l;
	l = r, r = v;
}

// Last round
l ^= (r.permute(e, xpn) ^= k[15]).substitute<6, 4>(u, _subs, 8).permute(v, perm);
output = l.merge(r);
```