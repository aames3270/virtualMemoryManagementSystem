// Alberto Ames
// 827838213
// Christian Ruiz
// 825290759
#include "PageTable.h"
#include <string>
#include <sstream>

using namespace std;

/*
 * This is just a constructor for nextLevelPtr..
 * Sets number of entries (array) in level pointer,
 * each pointer to null, and sets number of accesses to 0.
 * (Below)
 */

nextLevelPtr::nextLevelPtr(int numEntries)
{
    numOfAccesses = 0;
    this -> numEntries = numEntries;
    level = new nextLevelPtr*[numEntries];
    pfnPt = nullptr;

    for (int i = 0; i < numEntries; ++i)
    {
        level[i] = nullptr;
    }
}

/*
 * The argument string "4 8 8" sent from main is put into
 * numBits integer by integer. numBits is then stored into
 * levelCount array. (Below)
 */

PageTable::PageTable(const std::string& bitLevels) {
    istringstream bitsLevel(bitLevels);
    int defaultBitShift = 32;
    //int singleLevelShift = 24;
    int minNumEntries = 1;
    int numBits;
    int shift = defaultBitShift;

    // Level count for each level.
    while (bitsLevel >> numBits)
    {
        levelCount.push_back(numBits);
    }

    // Starting at size 1.
    root = new nextLevelPtr(minNumEntries);

    for (unsigned int i = 0; i < levelCount.size(); ++i)
    {
        numBits = levelCount[i];
        // Subtracting from shift to keep track of bit position.
        shift -= numBits;

        unsigned int mask = (1 << numBits) - 1;
        mask = mask << shift;

        bitMaskArray.push_back(mask);
        shiftArray.push_back(shift);
    }
}

/*
 * This extract bits from the address to make vpn.
 * Iterates through each level in the page table.
 * bitMaskArray.size() is the number of levels.
 */
unsigned int PageTable::vpnExtract(unsigned int address) {
    unsigned int vpn = 0;

    for (unsigned int i = 0; i < bitMaskArray.size(); ++i)
    {
        unsigned int vpnLevel = extractPageNumberFromAddress(address, bitMaskArray[i], shiftArray[i]);
        vpn = vpn << levelCount[i];
        vpn = vpn + vpnLevel;
    }
    return vpn;
}

/*
 * Function goes through the page table using bitmasks and shifts to find VPN.
 * If page is found, returns map with vpn and pfn. If not, returns null.
 */
Map* PageTable::lookup_vpn2pfn(unsigned int address) {
    nextLevelPtr* current = root;

    for (unsigned int i = 0; i < bitMaskArray.size(); ++i)
    {
        unsigned int index = extractPageNumberFromAddress(address, bitMaskArray[i], shiftArray[i]);

        if (current->level == nullptr || current->level[index] == nullptr)
        {
            return nullptr; // VPN not found
        }
        current = current->level[index]; // NEXT
    }

    if (current->pfnPt)
    {
        return new Map{vpnExtract(address), current->pfnPt[0]};
    }
    else
    {
        return nullptr;
    }
}

/*
 * Function goes through the page table to find the correct position
 * for a VPN. At the final level, it assigns or updates the PFN frame for that
 * VPN (BELOW)
 */
void PageTable::insert_vpn2pfn(unsigned int address, unsigned int frame) {
    nextLevelPtr* current = root;
    int minNumEntries = 1;

    for (unsigned int i = 0; i < bitMaskArray.size(); ++i)
    {
        // Index is based off page number from address
        unsigned int index = extractPageNumberFromAddress(address, bitMaskArray[i], shiftArray[i]);

        // Creates an array of nextLevelPtr* pointers of size current->numEntries
        // Sets every entry to null
        if (current->level == nullptr)
        {
            current->level = new nextLevelPtr*[current->numEntries];
            for (size_t j = 0; j < current->numEntries; ++j)
            {
                current->level[j] = nullptr;
            }
        }

        // Checking if entry is null
        if (current->level[index] == nullptr)
        {
            // Creates new nextLevelPtr at index
            current->level[index] = new nextLevelPtr(minNumEntries);
        }

        current = current->level[index]; // NEXT
    }

    /*
     * At final level it checks if current->pfnPt is null...
     * If null, creates an array for pfnPt holds frame at pfnPt[0].
     * If not null it just updates frame. (BELOW)
     */
    if (!current->pfnPt)
    {
        current->pfnPt = new unsigned int[minNumEntries];
        current->pfnPt[0] = frame;
    }
    else
    {
        current->pfnPt[0] = frame;
    }
}

/*
 * Extracting page number from address function..
 */
unsigned int PageTable::extractPageNumberFromAddress(unsigned int address, unsigned int mask, unsigned int shift)
{
    return (address & mask) >> shift;
}

/*
 * Recursive function that sums up the total number of
 * page table entries... (BELOW)
 */
unsigned long int PageTable::getPageEntries(nextLevelPtr* current, int level) const {
    if (current == nullptr)
    {
        current = root;
    }

    if (current == nullptr || current->level == nullptr)
    {
        return 0;
    }

    unsigned long int totalPageEntriesPt = 0;
    int numEntriesAtLevel = 1 << levelCount[level];

    for (unsigned int i = 0; i < numEntriesAtLevel; ++i)
    {
        if (current->level[i] != nullptr)
        {
            if (level + 1 < levelCount.size())
            {
                totalPageEntriesPt += (1 << levelCount[level + 1]);
                totalPageEntriesPt += getPageEntries(current->level[i], level + 1);
            }
        }
    }
    return totalPageEntriesPt;
}

/*
 * Function that tracks page accesses by going through
 * and updating the page table. (BELOW)
 */
void PageTable::recordPageAccess(unsigned int address, const string& outputMode) {

    int minNumEntries = 1;

    nextLevelPtr* current = root;

    unsigned int levels = bitMaskArray.size();

    /*
     * A loop that goes through each level of page table.
     * For each level, grabs index by extracting bits using
     * bitMaskArray and shitArray.
     */
    for (unsigned int i = 0; i < levels; ++i)
    {
        unsigned int index = extractPageNumberFromAddress(address, bitMaskArray[i], shiftArray[i]);
        unsigned int size = 1 << levelCount[i];
        // Creates new array of nextLevelPtr pointers with size calculated
        nextLevelPtr** entries = new nextLevelPtr*[size];

        // Copies old pointers from current->level into the new array.
        for (unsigned int j = 0; j < current->numEntries; ++j)
        {
            entries[j] = current->level[j];
        }

        // Sets new entries to null..
        for (unsigned int j = current->numEntries; j < size; ++j)
        {
            entries[j] = nullptr;
        }

        // Deletes old array and sets current->level to new entries array
        // Updates size of numEntries
        delete[] current->level;
        current->level = entries;
        current->numEntries = size;

        // If current level at index is null, create new nextLevelPtr
        if (current->level[index] == nullptr)
        {
            current->level[index] = new nextLevelPtr(minNumEntries);
        }

        // Incrementing number of accesses
        current = current->level[index];
        current->numOfAccesses++;
    }

    // Handles single level, special case max
    if (levels == 1)
    {
        unsigned int index = extractPageNumberFromAddress(address, bitMaskArray[0], shiftArray[0]);

        // If current->level null, it sets numEnties to 2^levelCount[0]..
        unsigned int numberEntries = 1 << levelCount[0];

        if (!current->level) {
            current->level = new nextLevelPtr*[numberEntries];
            for (unsigned int i = 0; i < numberEntries; ++i)
            {
                current->level[i] = nullptr;
            }
            current->numEntries = numberEntries;
        }

        // Calculates a new index to make sure its within range..
        unsigned int reducedI = index % current->numEntries;

        if (current->level[reducedI] == nullptr)
        {
            current->level[reducedI] = new nextLevelPtr(minNumEntries);
        }

        // Increment number of accesses at leaf.
        current->level[reducedI]->numOfAccesses++;
    }

    // Creating a pageIndices array to keep page numbers for each level.
    // Set to root to do another go through.
    unsigned int pageIndices[levels];
    nextLevelPtr* numAccessesCountPtr = root;

    // Storing pageIndices for each level.
    for (unsigned int i = 0; i < levels; ++i)
    {
        pageIndices[i] = extractPageNumberFromAddress(address, bitMaskArray[i], shiftArray[i]);
        numAccessesCountPtr = numAccessesCountPtr->level[pageIndices[i]];
    }
}


