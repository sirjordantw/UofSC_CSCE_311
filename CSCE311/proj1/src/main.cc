#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/sysinfo.h>

//Structure of a row from txt file.
struct Row{
    std::string id;
    std::string value;
    std::size_t iterations;
};

//Structure of shared threads per output of rows.
struct Shared{
    int threadID;
    std::string digest; //The result of several SHA-256 hashes linked.
    bool finished; //Determines if a row finishes before timeout, and afterwards produces a digest.
};

//Structure of a thread of that row.
struct Thread{
    int threadID;
    int k;
    //Shared
    int *currID;
    const std::vector<Row>* rows;
    std::vector<Shared>* out; //Shared outputs per row
    Timings_t timeout_ms;
};

CliMode mode;

void* StartRoutine(void* arg);

int main(int argc, char *argv[]){
    Time_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    int n = get_nprocs();
    std::vector<::pthread_t> thread_pool(n);
    int currID = 0;

    int rows;
    std::cin >> rows;

    std::vector<Row> rowsv;
    rowsv.reserve(rows);

    for(int i = 0; i < rows; i++){
        Row r;
        std::cin >> r.id >> r.value >> r.iterations;
        rowsv.push_back(r);
    }

    ThreadLog("Enter max threads (1 - %i): ", n);

    int k;
    std::ifstream tty_in("/dev/tty");
    if(tty_in){
        tty_in >> k;
        
        if(k < 1){
            ThreadLog("Input under 1. Autosetting k = 1.");
            k = 1;
        }
        else if(k > n){
            ThreadLog("Input over %i. Autosetting k = %i.", n, n);
            k = n;
        }
    }

    std::vector<Shared> out(rowsv.size());
    std::vector<Thread> threads(k);

    for (int i = 0; i < k; i++) {
        threads[i].threadID = i + 1;
        threads[i].k = k;
        threads[i].rows = &rowsv;
        threads[i].currID = &currID;
        threads[i].timeout_ms = timeout_ms;
 
        ::pthread_create(&thread_pool[i], nullptr, StartRoutine, &threads[i]);
    }

    if(mode == CLI_MODE_ALL){
        currID = k;
    }
    else if(mode == CLI_MODE_RATE){
        for(int relT = 1; relT <= k; relT++){
            currID = relT;
            Timings_SleepMs(1);
        }
    }
    else if(mode == CLI_MODE_THREAD){
        currID = 1;
    }

    for (int i = 0; i < k; i++) {
        ::pthread_join(thread_pool[i], nullptr);
    }

    ThreadLog("Thread   Start   Encryption");
    for(int i = 0; i < (int)rowsv.size(); i++){
        ThreadLog("%i        %i       %s", i, rowsv[i].id, (out[i].finished) ? out[i].digest : "timeout");
    }

    return 0;
}

void* StartRoutine(void* arg){
    Thread* x = static_cast<Thread*>(arg);
    int i = x->threadID;

    //Checks if index goes beyond max threads given
    if(i > x->k){
        ThreadLog("[thread %i] returned", i);
        return nullptr;
    }

    //Pause
    while (i > *(x->currID)) {
        Timings_SleepMs(1);
    }

    ThreadLog("[thread %i] started", i);

    //Racy
    if(mode == CLI_MODE_THREAD && i < x->k){
        *(x->currID) = i + 1;
    }

    Timings_t startTime = Timings_NowMs();

    if (mode == CLI_MODE_THREAD && i < x->k) {
        *(x->currID) = i + 1;
    }

    for(int ind = i; ind < (int)x->rows->size() && !Timings_TimeoutExpired(startTime, x->timeout_ms); ind += x->k){
        ThreadLog("[thread %i] completed row %i", i, i - 1);
    }

    ThreadLog("[thread %i] returned", i);
    return nullptr;
}