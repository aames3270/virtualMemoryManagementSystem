// Alberto Ames
// 827838213
// Christian Ruiz
// 825290759
#include <iostream>
#include <cstdio>
#include <getopt.h>
#include <sstream>
#include <vector>
#include "tracereader.h"
#include "PageTable.h"
#include "TLB.h"
#include "log.h"

using namespace std;

int main(int argc, char **argv) {
    int n = -1;  // If no "n," we process all addresses.
    string output = "summary";  // The default output mode is "summary"
    int getop;
    int cache = 0; // The default for no "c," is 0

    // Avoiding magic numbers..
    int defaultPageTableSize = 32;
    int maxTotalBits = 28;
    int defaultOffsetBits = 12;

    // Tracking stats variables..
    unsigned int cHits = 0; // Counting cache hits
    unsigned int ptHits = 0; // Counting page hits
    unsigned int virtualTimeAddressCount = 0; // Tracking processed addresses but also as a "virtual timer"
    unsigned int frameCounter = 0;  // Used for assigning physical frame numbers

    /*
     *  This while loop is for checking arguments in the user input (getopt).
     *  -n = number of addresses to process
     *  -o = this is the output mode
     *  -c = setting the cache (TLB capacity)
     *  (BELOW)
     */
    while ((getop = getopt(argc, argv, "n:o:c:")) != -1) {
        switch (getop) {
            case 'n':
                n = stoi(optarg);
                if (n <= 0)
                {
                    cerr << "Number of memory accesses must be a number, greater than 0\n";
                    return 1;
                }
                break;
            case 'o':
                output = optarg;
                break;
            case 'c':
                cache = stoi(optarg);
                if (cache < 0)
                {
                    cerr << "Cache capacity must be a number, greater than or equal to 0\n";
                    return 1;
                }
                break;
        }
    }

    /*
     * This is for opening the trace file and also checking
     * if "bitmasks" is not the output mode. If it's not, we open the
     * trace file. If trace file does not exist, it issues out
     * an error message. (BELOW)
     */
    bool bitmaskFlag = false;
    if (output == "bitmasks")
    {
        bitmaskFlag = true;
    }

    FILE *traceFile = nullptr;
    if (!bitmaskFlag)
    {
        traceFile = fopen(argv[optind], "rb");
        if (!traceFile)
        {
            cerr << "Unable to open <<trace.tr>>" << argv[optind] << "\n";
            return 1;
        }
    }

    /*
     * This grabs the string of numbers, for example: "8 8 4,"
     * from the user input and converts it into a string (bitLevels).
     * (BELOW)
     */
    string bitLevels;
    for (unsigned int i = optind + 1; i < argc; ++i) //optind is part getopt.. next non-opetion argument..
    {
        if (!bitLevels.empty())
        {
            bitLevels += " ";
        }
        bitLevels += argv[i];
    }


    /*
     * It's "grabbing" from bitLevels and converting to integers.
     * It's also checking if number is less than 1, is so, issue error.
     * It is also adding up each integer into totalBits so it checks
     * if total bits is greater than 28, if so, issue error. (BELOW)
     */
    istringstream grab(bitLevels);
    vector<int> levelBits;
    int levelNum, levelIndex = 0, totalBits = 0;

    while (grab >> levelNum)
    {
        if (levelNum < 1)
        {
            cerr << "Level " << levelIndex << " page table must be at least 1 bit\n";
            return 1;
        }
        levelBits.push_back(levelNum);
        totalBits += levelNum;
        levelIndex++;
    }

    int offSett = defaultPageTableSize - totalBits;
    int pageSizeCounter = 1 << offSett;

    if (totalBits > maxTotalBits)
    {
        cerr << "Too many bits used in page tables\n";
        return 1;
    }

    // Sending bitLevels to PageTable (BELOW)

    PageTable pageTable(bitLevels);

    // Printing accordingly for "bitmasks," using log. (BELOW)

    if (output == "bitmasks")
    {
        log_bitmasks(pageTable.bitMaskArray.size(), pageTable.bitMaskArray.data());
        return 0;
    }

    // If "c" is provided we initialize TLB. (BELOW)

    TLB* tlb = nullptr;
    if (cache > 0)
    {
        tlb = new TLB(cache);
    }

    /*
     * This processing every address in the trace file using trace reader.
     * If "n" is provided we process that amount, if not, it processes the
     * entire file. It also sends every processed address to recordPageAccess.
     * virtualTimeAddressCount keeping track of each address processed. (BELOW)
     */
    p2AddrTr trace;
    while (true)
    {
        if (!NextAddress(traceFile, &trace))
        {
            break;
        }

        if (n != -1 && virtualTimeAddressCount >= (unsigned int)n)
        {
            break;
        }

        virtualTimeAddressCount++;
        pageTable.recordPageAccess(trace.addr, output);

        /*
         * Using vpnExtract to extract the virtual page number.
         * If offset is the given "o" then we print using hexnum from log.
         * (BELOW)
         */
        unsigned int vpn = pageTable.vpnExtract(trace.addr);
        if (output == "offset")
        {
            unsigned int offset = trace.addr & ((1 << defaultOffsetBits) - 1);
            hexnum(offset);
            continue;
        }

        /*
         * If TLB is set, we look up the vpn in the TLB.
         * Also setting tlb hit and page table hit flags to false.
         * (BELOW)
         */
        int pfn = -1;
        bool tlbHit = false;
        bool ptHit = false;

        if (tlb != nullptr)
        {
            pfn = tlb->lookup(vpn, virtualTimeAddressCount);
        }

        /*
         * If vpn was not located in TLB, we page table look up. If it's
         * in the page table it logs it in as a ptHit. If not, it creates a new frame
         * and updates page table and TLB.
         * (BELOW)
         */
        if (pfn == -1)
        {
            // Looking it up...
            Map* mapping = pageTable.lookup_vpn2pfn(trace.addr);

            // If found...
            if (mapping)
            {
                // Setting PFN to physical frame number
                pfn = mapping->pfn;
                // Sets page table hit flag to true
                // Mark a page table hit
                ptHit = true;
                ptHits++;

                // if TLB is in use, not null, it inserts vpn to pfn mapping into TLB
                if (tlb != nullptr)
                {
                    tlb->insert(vpn, pfn, virtualTimeAddressCount);
                }
                // If not found in the page table, it inserts new entry with framecounter as PFN.
            }
            else
            {
                pageTable.insert_vpn2pfn(trace.addr, frameCounter);
                if (tlb != nullptr)
                {
                    tlb->insert(vpn, frameCounter, virtualTimeAddressCount);
                }
                // Sets pfn to current framecounter and then increments framecounter.
                pfn = frameCounter;
                frameCounter++;
            }
            // If not -1, TLB hit!
        }
        else
        {
            tlbHit = true;
            cHits++;
        }

        /*
         * Checks if the "o" is "vpn2pfn," is so, it prints accordingly.
         * (BELOW)
         */
        if (output == "vpn2pfn")
        {
            // Determining the number of levels in page table
            unsigned int levels = pageTable.bitMaskArray.size();
            // Creating an array for page indices using levels.
            unsigned int pageIndices[levels];
            // Pointer to the root of pageTable.
            // Used to go through each level of the page table.
            nextLevelPtr* current = pageTable.root;
            // Going through each level and extracting each page number.
            for (unsigned int i = 0; i < levels; ++i)
            {
                pageIndices[i] = pageTable.extractPageNumberFromAddress(trace.addr, pageTable.bitMaskArray[i], pageTable.shiftArray[i]);
                current = current->level[pageIndices[i]];
            }
            // Frame variable used to hold PFN.
            unsigned int frame;

            // Checks if PFN exists, if not, frame is set to 0.
            // if it exists, frame is set to current->pfnPT[0]
            if (current->pfnPt)
            {
                frame = current->pfnPt[0];
            }
            else
            {
                frame = 0;
            }
            // Prints accordingly for "vpn2pfn" using log.
            log_pagemapping(levels, pageIndices, frame);  // Use log_pagemapping to print the result
        }

        /*
         *  Checks if "o" is either "va2pa" or "va2pa_atc_ptwalk" to print
         *  accordingly using log.
         */
        if (output == "va2pa" || output == "va2pa_atc_ptwalk")
        {
            /*
             * Calculates the total for vpnBits, pageTable.levelCount is the bit count
             * for each level in page table.
             */
            unsigned int vpnBits = 0;
            for (unsigned int i = 0; i < pageTable.levelCount.size(); ++i)
            {
                vpnBits += pageTable.levelCount[i];
            }

            // Calculates the offset by subtracting vpn bits from 32.
            unsigned int offsetBits = defaultPageTableSize - vpnBits;
            // Isolation the offset using a mask.
            unsigned int offsetMask = (1 << offsetBits) - 1;

            /*
             * Shifts pfn to the left by offsetBits.
             * Grabs offset from trace.addr by using offsetMask from above ^^.
             * The OR "|" combines to form physical address.
             */
            unsigned int physicalAddress = (pfn << offsetBits) | (trace.addr & offsetMask);

            // Print based on output.
            if(output == "va2pa_atc_ptwalk")
            {
                log_va2pa_ATC_PTwalk(trace.addr, physicalAddress, tlbHit, ptHit);
            }

            if (output == "va2pa")
            {
                log_virtualAddr2physicalAddr(trace.addr, physicalAddress);
            }
        }
    }

    // Checking if "o" is "summary."
    if (output == "summary")
    {
        // totalPageEntries is for counting the page entries.
        unsigned long int totalPageEntries;

        //If table has only 1 level, we calculate page entries by 2^[levelBits]
        if (pageTable.levelCount.size() == 1)
        {
            int levelBits = pageTable.levelCount[0];
            totalPageEntries = 1 << levelBits;
            //if table has more than 1 level we send to getPageEntries
        }
        else
        {
            totalPageEntries = pageTable.getPageEntries();
            // Grabbing the fist level bits so we can add it to totalPageEntries
            int firstLevelBits = pageTable.levelCount[0];
            totalPageEntries += (1 << firstLevelBits);
        }

        // Using log to print.
        log_summary(pageSizeCounter, cHits, ptHits, virtualTimeAddressCount, frameCounter, totalPageEntries);
    }

    fclose(traceFile);
    return 0;
}