[![Build Status](https://travis-ci.org/piojanu/btree.svg?branch=master)](https://travis-ci.org/piojanu/btree)

# b+tree
University project from subject Data Base Structures

# Where is what?
'tests' directory contains unit tests. Gtest library is used for testing.
Content of 'include' and 'source' directories with main.cpp in top directory are project sources and headers.

# Building it
Use cmake to generate makefiles or workspaces. If you want to use clang, you can use -DUSE_CLANG=1 param.

Alternatively you can use scripts 'gen_makefiles.sh' and 'gen_ninjafiles.sh' to generate makefiles for make or ninja.
In both, you can specify 'use_clang' param to use clang compiler.
Those scripts will generate makefiles in 'build\' directory.

# Run it
After build, binaries will be put in 'bin\' directory.
'runtest_btree' is unit tests binary.
'btree' is project binary.
