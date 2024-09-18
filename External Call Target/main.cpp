#include <iostream>
#include <thread>
#include <chrono>

void print(const char* message) {
    std::cout << message << std::endl;
}

int main() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        print("Hello, World!");
    }
    return 0;
}