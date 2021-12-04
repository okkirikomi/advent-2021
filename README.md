Advent of Code 2021
=======

I write simple and fast code, making more suitable wheels when needed. No CPU specific optimizations other than what the compiler gives you with `-march=native`.

* Better than naive algorithms when needed.
* Try to optimize cache access.
* Avoid as much C++ Standard Library or trendy paradigms as possible.
* Write readable and maintainable code.
* Keep it simple.

Sure, this doesn't change much for the small size of these problems. Still this is usually about 2-4x faster than other C or C++ solutions available simply by avoiding multiple allocations when growing vectors or smarter I/O.

Tested on a Linux machine with GCC 7.5.0. Should work on anything POSIX with a compiler supporting C++11. Or at least with minimal plumbing.

A Windows port needs to implement some POSIX stuff in the Windows way. Check for `TODO`s in include directory.

Check `BETTER` comments for low hanging performance fruits.

Usage:
* Put your input files in input folder under `day$n` name.
* build.sh will build everything, or pass a range of days in parameter.
* run.sh will run everything, or pass a range of days in parameter.
* If executing manually, each program expects the input file path as parameter, no stdin.

[Advent of Code 2021](https://adventofcode.com/2021)
