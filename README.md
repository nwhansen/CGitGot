# CGitGot
A port of GeneHacks https://github.com/genehack/app-gitgot into native code. This is done due to the high level of dependencies that are required for GitGot to be built.
This makes the setup in git bash next to impossible, epecially due to the lack of a compiler. Making this a windows compatable executable allows for the MSYS2 enviroment to convert paths allowing for git-bash compatability. (Ideally) 

Currently this is only compatable with MSVC++ compilers however it mainly uses the standard library. And patches that make it cross operating system compatable are appreciated. 

## Building 

*Wait until after libgit2 before opening the sln file*

First clone the entire repo and download the libgit2 submodule via the commands

```
git submodule init
git submodule update
```

Then follow the instructions to build libgit2 https://libgit2.github.com/docs/guides/build-and-link/

Open the solution which should automatically now find the libgit2 project.

Modify the output directory of the libgit2 project to point to the Debug folder in the root of the solution for example

> ..\Debug\


