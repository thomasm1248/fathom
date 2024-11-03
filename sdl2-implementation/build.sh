#!/bin/bash

g++ main.cpp classes/*.cpp `pkg-config --cflags --libs sdl2 SDL2_ttf`
