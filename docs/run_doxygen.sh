#!/bin/bash

sed -e 's/#define.*DLL_PUBLIC.*//g' -e 's/DLL_PUBLIC //g' ../library/include/rocfft.h > rocfft.h
doxygen Doxyfile

