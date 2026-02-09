**Makefile**
Unchanged to however it was created with the original zip file.

**main.h**
No need to look at this, I did not use.

**main.cc**
*Includes*
sys/sysinfo.h: Gets the information of the current system.
fstream & iostream: Allows for inputs and outputs to be produced.
lib/*.h: All of the headers provided were helpful when coding. (I would have been stuck for hours trying to fix lines not being printed on a new line.)

*Structures*
Row (Represents each row of a "thread".)
- id: The id of that row.
- value: The data that row contains.
- iterations: An unsigned int that determines how many iterations of counts/loops to perform.

Shared (Represents shared data between threads.)
- threadID: ID of a thread.
- digest: Shows the final SHA-256 result for the specific row in 64 hex characters. Used for the encryption code.
- finished: Determiens when the row has completed fully before timeout.

Thread (Represents a thread.)
- threadID: ID of a thread.
- k: The inputed max active thread size.
- currID: The amount of threads that are running. Used to control how many threads get released.
- rows: The inputted rows from the txt file.
- out: The output of each thread's encyption.
- timeout_ms: How long the thread is allowed to run for in milliseconds.

*Global Vars*
mode: Contains the arguments of all, thread, and rate. Is global so all functions know what mode the program is in.
(Note: Most variables are local for memory purposes.)

*Functions*
Reduce
- Made to just reduce text size of printed strings with "..." for stylization purposes.

StartRoutine
- Made to start thread execution after one is created.
- arg allows data from main() to get collected, and is static_casted into a dereferenced Thread so that arg isn't treated as void.
- Checks at first to see the threadID it is given does not surpass max threads given (k).
- The while loop acts as a pause so not all threads process all at once, enforcing the mode system this program has.
- Before it starts working, it checks if the mode is thread and index is less then max so it knows to release threads in increments, attempting to not be out of order.
- The for loop goes through the entire list of rows without surpassing the row list's size and not continuing after the thread timesout.
- Inside the for loop, it gets a row, hashes the data of that row in 64-bit hex form to obtain the encryption of that data, then gives that data to the out struct, along with its id and when it finishes.
- Logs when the thread starts, completes, and then returns.

*Main Function*
- First, parses arguments that came from the command line into the code.
- Obtains amount of cores your CPU has, and creates a thread pool.
- Gets rows from inputted file, and puts the information from thoes rows into a Row structure.
- Asks user for max threads to process (k), and autosets if input goes beyond or below processer range.
- Lists threads based off max threads to process given, and creates a list of shared data between those threads.
- Creates a thread and sends it to StartRoutine. (Non-blocking call)
- Sets the release mode of threads based off command line argument.
- Blocks other threads from processing until thread in process is terminated.
- Finally, gives a list of threads with its id, value, and encryption.