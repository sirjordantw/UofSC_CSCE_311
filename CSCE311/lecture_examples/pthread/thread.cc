#include <cstddef>
#include <vector>
#include <pthread.h>
#include <iostream>

void* StartRoutine(void* arg);
std::size_t currT, maxT = 0;
//--all sets both to 8
//--rate maxT = 8
//--thread

int main(){
    std::size_t n = 4;
    std::vector<::pthread_t> thread_pool(n);

    //non-blocking calls
    for(::pthread_t thread : thread_pool){
        ::pthread_create(&thread, nullptr, StartRoutine, nullptr);
    }

    std::size_t row_n;
    std:: cin >> row_n;
    std::vector<Row> rows(row_n);
    for(Row& row : rows){
        std::cin >> row.id >> row.value >> row.iterations;
    }

    std::ifstream cin("/dev/tty");
    std::cout << "yipee";
    std::cin >> k;

    if(mode == CLI_MODE_RATE){
        for(size_t i = 0; i<k; i++){
            currT++;
            Timings_SleepMs(1);
        }
    }

    //blocking calls
    for(::pthread_t& thread : thread_pool){
        ::pthread_join(thread, nullptr);
    }

    return 0;
}

struct Row{
    std::string id;
    std::string value;
    std::size_t iterations;
};

void* StartRoutine(void* arg){
    while (my_index < currT){
        Timings_SleepMs(1); //Will be kicked out if sleeping for that little.
    }

    Timings_t start = Timings_Now();

    while(my_rows < total_rows && Timings_Now() - start < max_time){ //Timing_TimeOut
        
    }

    std::cout << "StartRoutine" << std::endl;
    return nullptr;
}