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
    const std::vector<Row>* rows;
};

void* StartRoutine(void* arg);

int main(int argc, char *argv[]){
    CliMode mode;
    Time_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    int n = (argc == 2) ? ::atoi(argv[1]) : get_nprocs();
    std::vector<::pthread_t> thread_pool(n);

    int rows;
    std::cin >> rows;

    std::vector<Row> rowsv;
    rowsv.reserve(rows);

    for(int i = 0; i < rows; i++){
        Row r;
        std::cin >> r.id >> r.value >> r.iterations;
        rowsv.push_back(r);
    }

    std::cout << "Enter max threads (1 - 8): " << std::flush;

    int k;
    std::ifstream tty_in("/dev/tty");
    if(tty_in){
        tty_in >> k;
        
        if(k < 1){
            std::cout << "Input under 1. Autosetting k = 1." << std::endl;
            k = 1;
        }
        else if(k > 8){
            std::cout << "Input over 8. Autosetting k = 8." << std::endl;
            k = 8;
        }
    }

    std::vector<Thread> threads(k);

    for (int i = 0; i < k; i++) {
        threads[i].threadID = i + 1;
        threads[i].rows = &rowsv;

        ::pthread_create(&thread_pool[i], nullptr, StartRoutine, &threads[i]);
    }

    for (int i = 0; i < k; i++) {
        ::pthread_join(thread_pool[i], nullptr);
    }

    return 0;
}

void* StartRoutine(void* arg){
    Thread* x = static_cast<Thread*>(arg); //Gets Thread object
    int i = x->threadID;
    std::string threadNum = "[thread " + std::to_string(i) + "]";

    std::cout << threadNum + " started" << std::endl << std::flush;

    if((i - 1) < (int)x->rows->size()){
        const Row& r = (*(x->rows))[i - 1];
        std::cout << threadNum + " completed row " + std::to_string(i - 1) << std::endl << std::flush;
    }

    std::cout << threadNum + " returned" << std::endl << std::flush;
    return nullptr;
}