// Alberto Ames
// 827838213
// Christian Ruiz
// 825290759
#ifndef TLB_H
#define TLB_H

#include <unordered_map>
#include <list>

class TLB {
public:
    // Constructor for TLB, with a set size from the user
    TLB(int size);
    // Constructors for functions in the TLB.cpp
    int lookup(unsigned int vpn, unsigned int virtualTime);  // Accepts virtual time
    void insert(unsigned int vpn, unsigned int pfn, unsigned int virtualTime);  // Accepts virtual time

private:
    // Max size of the cache inputted by the user
    int sizeM;
    // Least Recently Used (LRU) list for VPNs
    std::list<unsigned int> lru;
    // VPN to PFN with LRU iterator
    std::unordered_map<unsigned int, std::pair<unsigned int, std::list<unsigned int>::iterator>> cache;
    // VPN to last access time (virtual time)
    std::unordered_map<unsigned int, unsigned int> accessTimes;
};

#endif // TLB_H
