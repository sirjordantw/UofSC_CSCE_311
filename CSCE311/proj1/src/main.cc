// Copyright 2026 Jordan Weinstein
#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"

#include <sys/sysinfo.h>

#include <iostream>
#include <fstream>
#include <vector>

// Structure of a row from txt file.
struct Row{
    std::string id;
    std::string value;
    std::size_t iterations;
};

// Structure of shared threads per output of rows.
struct Shared{
    int threadID;
    std::string digest;
    bool finished;
};

// Structure of a thread of that row.
struct Thread{
    int threadID;
    int k;
    // Shared
    int *currID;
    const std::vector<Row>* rows;
    std::vector<Shared>* out;
    Timings_t timeout_ms;
};

// Reduces string size to make the print look more stylized
static std::string reduce(const std::string& s, std::size_t max) {
    if (max <= 3){
        return s.substr(0, max);
    }
    if (s.size() <= max){ 
        return s;
    }
    return s.substr(0, max - 3) + "...";
}


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

    // Initialize list of rows.
    std::vector<Row> rowsv;
    rowsv.reserve(rows);

    for (int i = 0; i < rows; i++){
        Row r;
        std::cin >> r.id >> r.value >> r.iterations;
        rowsv.push_back(r);
    }

    // Obtain max input size.
    ThreadLog("Enter max threads (1 - %i): ", n);

    int k;
    std::ifstream tty_in("/dev/tty");
    if (tty_in){
        tty_in >> k;
        
        if (k < 1){
            ThreadLog("Input under 1. Autosetting k = 1.");
            k = 1;
        }
        else if (k > n){
            ThreadLog("Input over %i. Autosetting k = %i.", n, n);
            k = n;
        }
    }

    // Creates threads and shared data.
    std::vector<Shared> out(rowsv.size());
    std::vector<Thread> threads(k);

    // Non-Blocking
    for (int i = 0; i < k; i++) {
        threads[i].threadID = i + 1;
        threads[i].k = k;
        threads[i].rows = &rowsv;
        threads[i].out = &out;
        threads[i].currID = &currID;
        threads[i].timeout_ms = timeout_ms;
 
        ::pthread_create(&thread_pool[i], nullptr, StartRoutine, &threads[i]);  
        // Creates an exectable thread
    }

    // Mode setting
    if (mode == CLI_MODE_ALL){
        currID = k;
    }
    else if (mode == CLI_MODE_RATE){
        for (int relT = 1; relT <= k; relT++){
            currID = relT;
            Timings_SleepMs(1);
        }  // Loop made to gradually increase currID by 1 ms, 
        //while releasing an additional thread each time.
    }
    else if (mode == CLI_MODE_THREAD){
        currID = 1;
    }

    // Blocking
    for (int i = 0; i < k; i++) {
        ::pthread_join(thread_pool[i], nullptr);  
        // Blocks call of other threads in pool until thread target terminates
    }

    // Goes through each thread within the row.
    ThreadLog("Thread       Start           Encryption");
    for (int i = 1; i < static_cast<int>(rowsv.size()); i++){
        const char* dig = 
        (out[i].finished) ? out[i].digest.c_str() : "timeout";
        ThreadLog("%i            %-16s%s", i, 
            reduce(rowsv[i].value, 12).c_str(), dig);
    }

    return 0;
}

void* StartRoutine(void* arg){
    Thread* x = static_cast<Thread*>(arg);
    int i = x->threadID;

    // Checks if index goes beyond max threads given
    if (i > x->k){
        ThreadLog("[thread %i] returned", i);
        return nullptr;
    }

    // Pause
    while (i > *(x->currID)) {
        Timings_SleepMs(1);
    }

    ThreadLog("[thread %i] started", i);

    // Racy
    if (mode == CLI_MODE_THREAD && i < x->k){
        *(x->currID) = i + 1;
    }

    Timings_t startTime = Timings_NowMs();

    // Work
    for (int ind = i; ind < static_cast<int>(x->rows->size()) && !Timings_TimeoutExpired(startTime, x->timeout_ms); ind += x->k){
        const Row& r = (*(x->rows))[ind];

        // Hashes the data for encryption in 64-bit hex form
        char hexOut[65];
        ComputeIterativeSha256Hex(
            reinterpret_cast<const uint8_t*>(r.value.data()),
            r.value.size(),
            static_cast<uint32_t>(r.iterations),
            hexOut
        );

        // Gives data to Shared structure.
        (*(x->out))[ind].threadID = i;
        (*(x->out))[ind].digest = hexOut;
        (*(x->out))[ind].finished = true;
        
        ThreadLog("[thread %i] completed row %i", i, i - 1);
    }

    ThreadLog("[thread %i] returned", i);
    return nullptr;
}