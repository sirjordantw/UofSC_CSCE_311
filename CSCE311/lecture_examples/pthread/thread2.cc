#include <cstddef>
#include <vector>
#include <pthread.h>
#include <iostream>

void* StartRoutine(void* arg);
std::size_t currT, maxT = 0;
//--all sets both to 8
//--rate maxT = 8
//--thread

std ThreadData{
    ThreadData(std::size_t index_) : index(index_) {

    }
    std::size_t index;
}

int main(){
    std::size_t n = 4;
    std::vector<::pthread_t> thread_pool(n);
    std::vector<std::size_t> indices(n);

    for(std::size_t i = 0; i<n; i++){
        indices[i] = ThreadData(i);
        ::pthread_create(&thread_pool[i], nullptr, StartRoutine, &indices[i]);
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
    ThreadData datum = *static_cast<ThreadData *>(arg);

    std::cout << datum.index << "'s StartRoutine" << std::endl;
    return nullptr;
}