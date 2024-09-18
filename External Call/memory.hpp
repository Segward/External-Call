#pragma once

#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

class Memory {
public:

    Memory(const char processName[]);
    ~Memory();

    uintptr_t getModuleBase(const char moduleName[]);
    DWORD getPid() { return pid; }
    HANDLE getHandle() { return hProcess; }
    void callFunctionEx(uintptr_t address, void* args, size_t size);
    
private:

    const char* processName;
    DWORD pid = 0;
    HANDLE hProcess = NULL;
    void setProcessId();
    void setHandle();

};

Memory::Memory(const char processName[]) {
    this->processName = processName;
    try {
        setProcessId();
        setHandle();

    } catch (const char* error) {
        std::cout << error << std::endl;
        CloseHandle(hProcess);
    }
}

Memory::~Memory() {
    CloseHandle(hProcess);
}

void Memory::setProcessId() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        throw "Error: CreateToolhelp32Snapshot failed";

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    if (!Process32First(hSnap, &pe32))
        throw "Error: Process32First failed";

    do {
        this->pid = strcmp(reinterpret_cast<const char *>(pe32.szExeFile),processName) == 0 ? 
        pe32.th32ProcessID : 0;
    } while (Process32Next(hSnap, &pe32) && this->pid == 0);

    if (this->pid == 0)
        throw "Error: Process not found";

    CloseHandle(hSnap);
}

void Memory::setHandle() {
    this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE)
        throw "Error: OpenProcess failed";
}

uintptr_t Memory::getModuleBase(const char moduleName[]) {
    try {
        if (hProcess == NULL)
            throw "Error: Process handle is NULL";

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->pid);
        if (hSnap == INVALID_HANDLE_VALUE)
            throw "Error: CreateToolhelp32Snapshot failed";

        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        uintptr_t baseAddress = 0;

        if (!Module32First(hSnap, &modEntry))
            throw "Error: Module32First failed";

        do {
            baseAddress = strcmp(reinterpret_cast<const char *>(modEntry.szModule), moduleName) == 0 ? 
            reinterpret_cast<uintptr_t>(modEntry.modBaseAddr) : 0;
        } while (Module32Next(hSnap, &modEntry) && baseAddress == 0);

        if (baseAddress == 0)
            throw "Error: Module not found";

        CloseHandle(hSnap);
        return baseAddress;

    } catch (const char* error) {
        std::cout << error << std::endl;
    }

    return 0;
}

void Memory::callFunctionEx(uintptr_t address, void* args, size_t size) {
    try {
        if (hProcess == NULL)
            throw "Error: Process handle is NULL";
        
        if (address == 0)
            throw "Error: Address is NULL";

        void* pMemory = VirtualAllocEx(hProcess, 0, size,
        MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (pMemory == NULL)
            throw "Error: VirtualAllocEx failed";

        if (!WriteProcessMemory(hProcess, pMemory, args, size, nullptr))
            throw "Error: WriteProcessMemory failed";

        HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, 
        (LPTHREAD_START_ROUTINE)address, pMemory, 0, 0);
        if (hThread == NULL)
            throw "Error: CreateRemoteThread failed";

        WaitForSingleObject(hThread, INFINITE);
        VirtualFreeEx(hProcess, pMemory, 0, MEM_RELEASE);
        CloseHandle(hThread);

    } catch (const char* error) {
        std::cout << error << std::endl;
    }
}