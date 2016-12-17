tput setaf 6
echo
echo "  Project files are generated in \"build\" directory."
echo "  \"compile_commands.json\" is file used by eg. clang tools."
echo
echo "  NOTE: Add \"use_clang\" arg if you want to use clang compiler."
echo
tput sgr0

if [ -d "build" ]
then
    rm -R build/*
else
    mkdir build
fi

CLANG="OFF"
if [ ! -z $1 ] && [ "$1" = "use_clang" ] || [ "$1" = "USE_CLANG" ]
then
    CLANG="ON"
fi
    
cd build
cmake -GNinja -DUSE_CLANG=$CLANG -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../
cd ..

if [ -f "compile_commands.json" ]
then
    rm compile_commands.json
fi

ln -s $PWD/build/compile_commands.json compile_commands.json
