// Alberto Ames
// 827838213
// Christian Ruiz
// 825290759
#include "TLB.h"
#include <unordered_map>
#include <list>

TLB::TLB(int size) : sizeM(size) {}

/*
 * Goes through cache..
 * pair.first is the vpn..
 * pair.second.second is pointing to vpn position in the
 * lru list.
 */
int TLB::lookup(unsigned int vpn, unsigned int virtualTime) {
    for (std::pair<const unsigned int, std::pair<unsigned int, std::list<unsigned int>::iterator>>& pair : cache) {
        // Checks if vpn matches
        if (pair.first == vpn) {
            /*
             * Removes current vpn position from lru
             * Moves to the front, most recently used
             * Updates the pointer to point to new position.
             */
            lru.erase(pair.second.second);
            lru.push_front(vpn);
            pair.second.second = lru.begin();
            /*
             * Records "virtual time" into accessTimes
             * Returns the PFN associated with VPN
             */
            accessTimes[vpn] = virtualTime;
            return pair.second.first;
        }
    }
    // Returns -1 if not found
    return -1;
}

//Takes in vpn, pfn, and virtualTime
void TLB::insert(unsigned int vpn, unsigned int pfn, unsigned int virtualTime) {
    /*
     * Checks if TLB has reached its max size "sizeM"
     * If full, it removes the least recently used entry
     * Identifies last item of the lru list..
     * And removes VPN
     */
    if (cache.size() == sizeM) {
        unsigned int lru_vpn = lru.back();
        lru.pop_back();
        cache.erase(lru_vpn);
        accessTimes.erase(lru_vpn);
    }
    /*
     * Inserts vpn into front of the list (most recently used)
     * adds VPN-to-PFN mapping into cache
     * Records vitualTime into access time.
     */
    lru.push_front(vpn);
    cache[vpn] = std::make_pair(pfn, lru.begin());
    accessTimes[vpn] = virtualTime;
}



