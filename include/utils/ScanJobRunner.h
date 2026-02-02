#ifndef SCAN_JOB_RUNNER_H
#define SCAN_JOB_RUNNER_H

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

class ScanJobRunner
{
public:
    struct Options
    {
        bool multithreaded = false;
        size_t maxThreads = 0;
        std::function<void()> threadInit;
    };

    template <typename ScanItem, typename Job>
    static void run(const std::vector<ScanItem>& scans, const Options& options, Job job)
    {
        if (scans.empty())
            return;

        if (!options.multithreaded || scans.size() == 1)
        {
            if (options.threadInit)
                options.threadInit();
            for (size_t index = 0; index < scans.size(); ++index)
                job(index, scans[index]);
            return;
        }

        const size_t requestedThreads = options.maxThreads == 0 ? static_cast<size_t>(std::thread::hardware_concurrency()) : options.maxThreads;
        size_t threadCount = std::max<size_t>(1, requestedThreads);
        threadCount = std::min(threadCount, scans.size());

        std::atomic<size_t> nextIndex{0};
        std::vector<std::thread> threads;
        threads.reserve(threadCount);
        for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
        {
            threads.emplace_back([&]()
                {
                    if (options.threadInit)
                        options.threadInit();
                    for (;;)
                    {
                        size_t index = nextIndex.fetch_add(1);
                        if (index >= scans.size())
                            break;
                        job(index, scans[index]);
                    }
                });
        }

        for (std::thread& worker : threads)
            worker.join();
    }
};

#endif
