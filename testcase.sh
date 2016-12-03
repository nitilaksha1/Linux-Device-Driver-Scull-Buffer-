#!/bin/bash
#
# testcase.sh - simple script to run each of the 5 test cases for scullbuffer
# Author: Andrew Banman <banma001@umn.edu>, Niti Halakatti <halak004@umn.edu>

case $1 in
	1) ./producer 50 red & ./consumer 50;;
	2) ./producer 50 red & ./consumer 10;;
	3) ./producer 50 red & ./consumer 100;;
	4) ./producer 50 red & ./producer 50 black & ./consumer 200;;
	5) ./producer 50 red & ./consumer 50 & ./consumer 50;;
esac
