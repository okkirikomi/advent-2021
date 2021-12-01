My solutions to Advent of Code 2021
=======

I write simple and fast code and make more suitable wheels when needed. No CPU specific optimizations other than what the compiler gives you with `-march=native`.

* Better than naive algorithms when needed.
* Try to optimize cache access.
* Avoid as much C++ Standard Library or trendy paradigms as possible.
* Write readable and maintainable code.
* Keep it simple.

Tested on a Linux machine with GCC 7.5.0. Should work on anything POSIX with a compiler supporting C++11. Or at least with minimal plumbing.

A Windows port needs to implement some POSIX stuff in the Windows way. Check for `TODO`s in include directory.

Usage:
* Put your input files in input folder under `day$n` name.
* build.sh will build everything, or pass a range of days in parameter.
* run.sh will run everything, or pass a range of days in parameter.
* Each program expects the input file path as parameter.

[Advent of Code 2021](https://adventofcode.com/2021)
