#!/bin/bash

cd ./build
cmake ../
cd ../
cmake --build ./build --target gtl-test-directed
cmake --build ./build --target gtl-test-undirected
