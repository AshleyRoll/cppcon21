#!/bin/bash


gcc -O3 --std=c++20 -c main.cpp -o main.o

ld -o config.out -T config_register.ld main.o

objdump -ds config.out

