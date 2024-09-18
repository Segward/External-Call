#include "memory.hpp"

struct _args {
    const char a[4096];
};

int main() {

    uintptr_t rebase =      0x140000000;
    uintptr_t print =       0x140001450;

    Memory memory("Target.exe");
    uintptr_t baseAddress = memory.getModuleBase("Target.exe");
    uintptr_t address = baseAddress + (print - rebase);

    _args args = { "I'm called externally!" };

    memory.callFunctionEx(address, &args, sizeof(args));

    return 0;
}