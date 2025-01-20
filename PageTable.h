// Alberto Ames
// 827838213
// Christian Ruiz
// 825290759
#ifndef ASSIGNMENT1__PAGETABLE_H
#define ASSIGNMENT1__PAGETABLE_H

#include <vector>
#include <string>

using namespace std;
//Struct for VPN to PFN mappings
struct Map {
    // Virtual page number
    unsigned int vpn;
    // Physical frame number
    unsigned int pfn;
};

class nextLevelPtr;

class PageTable {

public:
    // Constructors for functions in PageTable.cpp
    PageTable(const std::string& bitLevels);
    unsigned int extractPageNumberFromAddress(unsigned int address, unsigned int mask, unsigned int shift);
    unsigned int vpnExtract(unsigned int address);
    void recordPageAccess(unsigned int address, const string& outputMode);
    Map* lookup_vpn2pfn(unsigned int address);
    void insert_vpn2pfn(unsigned int address, unsigned int frame);
    unsigned long int getPageEntries(nextLevelPtr* current = nullptr, int level = 0) const;

    // Array to store the number of bits per level
    vector<int> levelCount;
    // Array to store bitmasks
    vector<unsigned int> bitMaskArray;
    // Array for shifts
    vector<unsigned int> shiftArray;
    // Pointer to root node
    nextLevelPtr* root;

};

class nextLevelPtr {
public:
    // Node with number of entries
    nextLevelPtr(int numEntries);
    // Array of pointers
    nextLevelPtr **level;
    // Pointer to PFN
    unsigned int *pfnPt;
    // Number of accesses
    int numOfAccesses;
    // number of entries
    int numEntries;

};

#endif