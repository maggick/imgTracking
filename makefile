#############################
# makefile of Matthieu Keller
# <keller.mdpa@gmail.com>
# in order to compile imgTracking
#############################

# The compiler to use
CC=gcc  								# C complier
CXX=g++ 								# C++ compiler
# options I will pass to the compiler
CXXFLAGS=-c -pipe -Wall -lcv -lhighgui 	# C++ options
# to really clean
RM=rm
.PHONY: clean, mrproper
# désactivation des règles implicites
.SUFFIXES:

# Creat the imgTracking exe
all: main.o 
	$(CXX) $^  `pkg-config opencv --cflags --libs` -lcv -lhighgui -o imgTracking
 
main.o: main.cpp main.hpp
	$(CXX) $< $(CXXFLAGS) -o $@
 
# suppression des fichiers temporaires
clean:
	$(RM) -rf *.o
 
# suppression de tous les fichiers, sauf les sources,
# en vue d'une reconstruction complète
mrproper: clean
	$(RM) -rf Programme

