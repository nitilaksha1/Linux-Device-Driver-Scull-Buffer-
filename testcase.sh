#!/bin/bash
#
# testcase.sh - simple script to run each of the 5 test cases for scullbuffer
# Author: Andrew Banman <banma001@umn.edu>, Niti Halakatti <halak004@umn.edu>

case $1 in
	1) ./producer 50 case1_red & ./consumer 50 case1;;
	2) ./producer 50 case2_red & ./consumer 10 case2;;
	3) ./producer 50 case3_red & ./consumer 100 case3;;
	4) ./producer 50 case4_red & ./producer 50 case4_black & ./consumer 200 case4;;
	5) ./producer 50 case5_red & ./consumer 50 case5_1 & ./consumer 50 case5_2;;
esac
