#include </home/jordantw128/UofSC/CSCE311/proj1/lib/cli_parser.h>
#include </home/jordantw128/UofSC/CSCE311/proj1/lib/error.h>
#include </home/jordantw128/UofSC/CSCE311/proj1/lib/sha256.h>
#include </home/jordantw128/UofSC/CSCE311/proj1/lib/thread_log.h>
#include </home/jordantw128/UofSC/CSCE311/proj1/lib/timings.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/sysinfo.h>

int main(int argc, char *argv[]){
    CliMode mode;
    Time_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    int n = argc == 2 ? ::atoi(argv[1]) : get_nprocs();
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
    }

    
    
    return 0;
}

struct Row{
    std::string id;
    std::string value;
    std::size_t iterations;
};