#!/bin/bash

g++ main.cpp classes/*.cpp `pkg-config --cflags --libs sdl2`
