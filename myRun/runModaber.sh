#!/bin/bash

#In the name of God


Algorithm="Simple"
Algorithm="Evolutionary"
NPG="1"




time ./Modaber $1 $2 --algorithm "$Algorithm" --NPG "$NPG"
echo "*******************************"
./validate $1 $2 solution
