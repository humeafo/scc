SCC

A C/C++ shellcode compiler based on llvm/clang technology, make writing 
and testing shellcode more easier, you can write the code in C++, not only in 
C, you can use globals(see limitations) and C++ with full compiler syntax
checking for APIs.

[1] Limitations

now only windows x86/64 coff target is supported and the msvc link.exe should 
be used to generation of final executable.

global initialized pointers to globals should be avoided, c lib and global 
c++ constructor will not be supported.

clang should be stable enough for most shellcode generation purpose and it'd
be more compatible in the future according to de community efforts towards windows.

[2] plan:

linux/macos and non-x86 target support when I've spare time.

[3] Why?

Some day I'm bored of tuning compiler options and make various msvc tweaks to coding
shellcode, so on a weekend I started hacking llvm/clang codebase and make clang -fPIC
working on windows/COFF target.

NB, It's for fun only, it's a hack on a weekend, don't blame me for not working hard!

[4] OpenSource or not?

No.

License:

Any commercial usage must be allowed by the author. Any CTF usage should explicitly 
mention the work.

