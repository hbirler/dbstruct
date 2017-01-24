[![Build Status](https://travis-ci.org/hbirler/dbstruct.svg?branch=master)](https://travis-ci.org/hbirler/dbstruct)

## Synopsis
B+Tree implementation in C++11 (it works, but may not be pretty)

### Using
The B+Tree implementation is a single header file `btree.h`

Have fun

### Performance
Benchmark with `g++ -O2`:
* 1 Million random insert
* 1 Million random find
* 1 Million random erase

```
Bench insert btree:		257ms
Bench find btree:		194ms
Bench erase btree:		233ms
-----
Total btree: 			686ms

Bench insert std::map:	587ms
Bench find std::map:	557ms
Bench erase std::map:	508ms
-----
Total std::map: 		1653ms
```

Test it yourself with `make bench`