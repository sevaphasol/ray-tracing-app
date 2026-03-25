#!/bin/bash

clang++ -L lib -Wl,-rpath,core -lrta $(llvm-config --cppflags --ldflags --libs)  main.cpp
