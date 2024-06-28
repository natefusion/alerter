#!/usr/bin/env sh

cd src/

make PLATFORM=PLATFORM_DESKTOP

cd ..

cp ./src/libraylib.a ../target/raylib/
