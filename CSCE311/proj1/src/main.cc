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

//Structure of a thread of that row.
struct Thread{
    int threadID;
    int k;
    //Shared
    int *currID;
    const std::vector<Row>* rows;
    const Timings_t* timeout_ms;
};

void* StartRoutine(void* arg);

int main(int argc, char *argv[]){
    CliMode mode;
    Time_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    int n = get_nprocs();
    std::vector<::pthread_t> thread_pool(n);
    int currID;

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

    if(mode == CLI_MODE_ALL){
        currID = k;
    }
    else if(mode == CLI_MODE_RATE){
        currID = 0;
    }
    else if(mode == CLI_MODE_THREAD){
        currID = 1;
    }

    std::vector<Thread> threads(k);

    for (int i = 0; i < k; i++) {
        threads[i].threadID = i + 1;
        threads[i].k = k;
        threads[i].rows = &rowsv;
        threads[i].currID = &currID;
        threads[i].timeout_ms = (Timings_t)&timeout_ms;

        ::pthread_create(&thread_pool[i], nullptr, StartRoutine, &threads[i]);
    }

    for (int i = 0; i < k; i++) {
        ::pthread_join(thread_pool[i], nullptr);
    }

    return 0;
}

void* StartRoutine(void* arg){
    Thread* x = static_cast<Thread*>(arg);
    int i = x->threadID;

    ThreadLog("[thread %i] started", i);
    Timings_t startTime = Timings_NowMs();

    while(i < *(x->currID) || i < x->k){ }

    for(int ind = i; ind < (int)x->rows->size() && !Timings_TimeoutExpired(startTime, *(x->timeout_ms)); ind += x->k){

    }

    ThreadLog("[thread %i] returned", i);
    return nullptr;
}