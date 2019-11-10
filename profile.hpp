#pragma once

#include <iostream>
#include <map>
#include <mutex>
#include <functional>

#include <sys/time.h>

#include "papi.h"

using ProfileFn = std::function<void()>;
using AugmentorFn = std::function<void(std::ostream& out, bool header)>;

std::once_flag flag;
bool printHeader = true;

inline double gettime() {
    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec) / 1000000.0;
}

struct Counters {
    using PAPI_Event = int;

    std::map<std::string, PAPI_Event> events;
    std::vector<PAPI_Event> rawEvents;
    std::vector<long long> values;

    int numEvents;
    long long startVirtual, endVirtual;

    Counters() {
        // TODO CPU dependent initialization
        {
            // Zen
            events["Instructions"] = PAPI_TOT_INS;
            events["Cycles"]       = PAPI_TOT_CYC;
            // currently not implemented in PAPI
            // see also: https://lkml.org/lkml/2019/5/2/696
//            events["L1Dmiss"]      = PAPI_L1_DCM;
            events["StaleCycles"]  = PAPI_STL_ICY;
            events["BRmiss"]       = PAPI_BR_MSP;
        }
        // TODO check hw counters
        for (const auto& e : events) {
            rawEvents.push_back(e.second);
        }
        numEvents = events.size();
        values.resize(numEvents);
    }

    void start() {
        int retval;
        if ((retval = PAPI_start_counters(&rawEvents[0], numEvents)) != PAPI_OK) {
            std::cerr << "Failed to start PAPI counters." << std::endl;
            exit(1);
        }
        startVirtual = PAPI_get_virt_usec();
    }

    void stop() {
        endVirtual = PAPI_get_virt_usec();
        int retval;
        if ((retval = PAPI_stop_counters(&values[0], numEvents)) != PAPI_OK) {
            std::cerr << "Failed to stop PAPI counters." << std::endl;
            exit(1);
        }
    }

    long long getVirtualUsec() {
        return endVirtual - startVirtual;
    }

    void printHeader(std::ostream& out) {
        for (const auto& e : events) {
            out << ",";
            printf("%8.8s", e.first.c_str());
        }
    }

    void printAll(std::ostream& out, double n) {
        for (auto value : values) {
            out << ",";
            printf("%8.2f", value / n);
        }
    }

    double operator[](const std::string& counterName) {
        size_t index = std::distance(events.begin(), events.find(counterName));
        return static_cast<double>(values[index]);
    }
};

void profile(std::string name, uint64_t count, uint64_t repetitions, const ProfileFn& fn, const AugmentorFn& augFn = nullptr)  {
    // init PAPI
    std::call_once(flag, [](){
        int num_hwcntrs = 0;
        int retval;
        char errstring[PAPI_MAX_STR_LEN];

        if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
            fprintf(stderr, "Error: %d %s\n", retval, errstring);
            exit(1);
        }

        if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK) {
            printf("There are no counters available. \n");
            exit(1);
        }

        printf("There are %d counters in this system\n", num_hwcntrs);
    });

    Counters counters;

    // warmup round
    if (repetitions > 4) {
        fn();
        repetitions--;
    }

    counters.start();
    double start = gettime();
    for (size_t i = 0; i < repetitions; ++i) { fn(); }
    double end = gettime();
    counters.stop();

    // write header
    if (printHeader) {
        printf("        name,");
        printf("  timems,    MOPS, CPUtime,   IPC,   GHz");
        counters.printHeader(std::cout);
        if (augFn) { augFn(std::cout, true); }
        printf("\n");
    }

    double runtime = end - start;

    // print derived values
    printf("%12s,", name.c_str());
    printf("%8.2f,", (runtime * 1e3) / repetitions); // timems
    printf("%8.2f,", ((count * repetitions) / 1e6) / runtime); // MOPS
    printf("%8.2f,", (runtime * 1e6 / counters.getVirtualUsec())); // CPUtime
    printf("%6.2f,", (counters["Instructions"] / counters["Cycles"])); // IPC
    printf("%6.2f", (counters["Cycles"] / counters.getVirtualUsec() / 1e3)); // GHz

    counters.printAll(std::cout, count * repetitions);

    if (augFn) { augFn(std::cout, false); }

    std::cout << std::endl;
    printHeader = false;
}
