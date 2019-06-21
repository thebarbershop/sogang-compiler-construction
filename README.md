# sogang-cse4120

Course project for CSE4120: Fundamentals of Compiler Construction, Spring Semester 2019, Sogang University.

Implementation of a C-Minus Programming Language Compiler.

C-Minus Programming Langauge is a subset of the C Programming Langauge, described in "Appendix A", [1].

This project is based on codes for a TINY Language compiler, whose source code is given in "Appendix B", [1].

## Project 1: Lexical Analyzer

- [x] Implemented a C-Minus scanner with flex.

## Project 2: Syntax Analyzer

- [x] Implemented a C-Minus parser with bison.

## Project 3: Semantic Analyzer

- [x] Constructed a symbol table.
- [x] Implemented a semantic analyzer that performs semantic error check.

## Project 4: Code Generator

- [x] Implemented a stack-based environment with a frame pointer.
- [x] Completed a code generator for the SPIM 8.0 simulator. [2]

---

## References

[1]: Louden, Kenneth C., Compiler Construction: Principles and Practice, PWS Publishing Company, 1997.

[2]: For the SPIM project, see [the official site](http://spimsimulator.sourceforge.net/); The site does not offer download links for SPIM 8.0, but you can run `sudo apt-get install spim` in Debian/Ubuntu. I recommend using the latest QtSpim distribution from the official site. It runs the apparently same MIPS assembly language with better UI and debugging tools.
