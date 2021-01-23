OS Project 4 - Benjamin Wright

Compile:
1. Open terminal at project location
2. Run "make"

Run:
1. Open terminal at project location
2. Run "./proj4 *inputFileName* *outputFileName* *seed*"


Note* even though the program looks for the input files in a subdirectory, the argument is simply fileName.txt

Example: ./proj4 infile1.txt outfile1.txt 1

I have not been able to work out a bug where some processes' page tables get emptied out randomly. There seems to be some kind of race condition which causes this behavior and I looked all over and never found how its happening. The tables all initialize properly every time and the class doesn't have a destructor (nor does PageTableEntry). This only seems to occur when the thread count is high.