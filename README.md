
## Running Instructions:
* Make sure that the device is unloaded or the device with same name doesn't exist. Type "sudo ./scull_unload".
* Load the scull buffer using command "sudo ./scull_load". This will create a device called /dev/scullbuffer.
* In case the number of items have to be specified by the user then type "sudo ./scull_load <nitems>" where nitems is a placeholder.
* Type make command to compile all relevant files.
* The test cases are present in a file called testcase.sh. 
* This script takes the test case number as the parameter. To execute a test case type "./testcase.sh <testcasenumber>" where <testcasenumber> is placeholde for testcase number.

## Output Format:
* The producers each write the item they have produced into their respective log files. 
* The log file of a producer has the following format: Prod<third-arg>.log where the <third-arg> is the parameter passed to the producer which is the name of   item that a particular producer will produce. For example: if producer writes items called "blue" then its output would be in file Prodblue.log.
* The consumers each write the item they have consumed into their respective log files.
   The log file of a consumer has the following format: Cons<second-arg>.log where <second-arg> is the parameter passed to the consumer program which is an id   entifier for a consumer. This argument must be unique per consumer.

## Test Case Description:

### Testcase #1: 
This test case consists of a single producer consumer each of which produce and consume 50 items respectively.

### Execution: 
Type "./testcase.sh 1" to execute this test case. The producer output can be seen in file Prod_case1_red.log and the consumer output can be seen in the file Cons_case1.log.

### Expected Output: 
The producer and consumer must exit normally and the consumer must consume all the 50 items.   

### Testcase #2: 
This test case consists of a single producer and consumer. The producer will try to produce 50 items before exiting and the consumer consumes only 10 items and exits. 
   
### Execution:
Type "./testcase.sh 2" to execute this test case. The producer output can be seen in file Prod_case2_red.log and the consumer output can be seen in the file Cons_case2.log.

### Expected Output: 
Depending on the size of the scull buffer the producer can deposit (scullbuffersize/32) + 10 items into the scull buffer and exit once the consumer exits. The consumer will read 10 items and exit.
   
### Testcase #3:
This test case consists of a single producer and consumer. The producer produces 50 items before exiting and the consumer consumes 100 items before exiting.

### Execution:
Type "./testcase.sh 3" to execute this test case. The producer output can be seen in file Prod_case3_red.log and the consumer output can be seen in the file Cons_case3.log.

### Expected Output:
The producer will write 50 items into the scull buffer regardless of the scull buffer size. The consumer will consumer 50 items before exiting since there are no producers.

4.	Testcase #4:
	-------------

	This test consists of two producers and one consumer. Each producer produces 50 items and the consumer will try to consumer 200 items before exiting.

	Expected Output:
	-----------------

	Both the items will produce 50 items regardless of the scullbuffer size and the consumer will exit after consuming 100 items.

	Running Instructions:
	----------------------

	Type "./testcase.sh 4" to execute this test case. The producer output can be seen in file Prod_case4_red.log and Prod_case4_black.log. The consumer output 	  can be seen in Cons_case4.log.

5.	Testcase #5:
	------------

	This test consists of one producer and two consumers. The producer will produce 50 items and the two consumers together (or individually) will consume th	  e 50 items.

	Expected Output:
	----------------

	The producer will produce 50 items regardless of the scullbuffer size. The consumers will consumer variable number of items that add upto 50 items combine	  d.

	Running Instructions:
	---------------------

	Type "./testcase.sh 5" to execute this test case. The producer output can be seen in the file Prod_case5_red.log. The consumers output can be seen in the 	  two log files Cons_case5_1.log and Cons_case5_2.log.
