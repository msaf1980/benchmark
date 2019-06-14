#ifndef _BENCHMARK_HPP_
#define _BENCHMARK_HPP_

#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cycleclock.h"

#define BENCHMARK_HEAD()                                                       \
	do {                                                                       \
	} while (0)

#define BENCH_CONSTRUCT(_class_)                                               \
	_class_(const std::string &group, const std::string &name,                 \
	        BenchmarkReporter *reporter) {                                     \
		this->group = group;                                                   \
		this->name = name;                                                     \
		this->r = reporter;                                                    \
	}

namespace benchmark {

class Benchmark;

struct BenchmarkStat {
	uint64_t max;
	uint64_t min;
	uint64_t avg;
	uint64_t p95;
	uint64_t p99;
	uint64_t div_min;
	uint64_t div_max;
};

uint64_t percentile(const std::vector<uint64_t> &r, size_t pcnt) {
	size_t size = r.size();
	if (size > 1) {
		size_t i = pcnt * size / 100;
		if (size % 2 == 0) {
			return (r[i] + r[i + 1]) / 2;
		} else {
			return r[i];
		}
	} else if (size == 1) {
		return r[0];
	} else
		return 0;
}

BenchmarkStat calc_stat(std::vector<uint64_t> &r) {
	BenchmarkStat stat = {0, 0, 0, 0, 0, 0, 0};
	std::sort(r.begin(), r.end());
	stat.max = r[r.size() - 1];
	stat.min = r[0];
	stat.avg = percentile(r, 50);
	stat.p95 = percentile(r, 95);
	stat.p99 = percentile(r, 99);
	stat.div_min = stat.p95 - stat.min;
	stat.div_max = stat.max - stat.p95;
	return stat;
}

class BenchmarkReporter {
  public:
	virtual void report(Benchmark *b) = 0;
};

class Benchmark {
  public:
	virtual ~Benchmark(){};

	virtual void run(size_t samples, size_t iterations) {
		success = false;
		durations.reserve(samples * iterations);
		this->samples = samples;
		this->iterations = iterations;
		this->threads = 0;
		try {
			prepare();
			for (size_t s = 0; s < samples; s++) {
				int64_t duration = 0;
				for (size_t i = 0; i < iterations; i++) {
					int64_t d = bench();
					if (d >= 0) {
						durations.push_back(d / 1000);
					} else {
						throw std::runtime_error("negative duration");
					}
				}
			}
			success = true;
		} catch (std::exception &e) {
			err = e.what();
		}
		cleanup();
		report();
	}

	virtual void    prepare(){};
	virtual int64_t bench() = 0;
	virtual void    cleanup(){};

	void report() {
		if (r != NULL)
			r->report(this);
	}

	std::string           group;
	std::string           name;
	size_t                threads;
	size_t                samples;
	size_t                iterations;
	bool                  success;
	std::string           err;
	std::vector<uint64_t> durations;

	BenchmarkReporter *r;
};

class BenchmarkStdoutReporter : public BenchmarkReporter {
  public:
	BenchmarkStdoutReporter() {
		printf("%10s | %10s | %8s | %10s | %10s | %14s | %14s | %14s |\n",
		       "Group", "Benchmark", "Threads", "Samples", "Iterations",
		       "us/Iter P95", "P99", "Div Min/Max");
	}

	virtual void report(Benchmark *b) {
		printf("%10s | %10s | %8lu | %10lu | %10lu |", b->group.c_str(),
		       b->name.c_str(), b->threads, b->samples, b->iterations);
		if (b->success && !b->durations.empty()) {
			BenchmarkStat stat = calc_stat(b->durations);
			std::string div = std::to_string(-stat.div_min) + "/" + std::to_string(stat.div_max);
			printf(" %14lu | %14lu | %14s | \n", stat.p95,
			       stat.p99, div.c_str());
		} else if (b->err.empty()) {
			printf(" SKIP\n");
		} else {
			printf(" FAIL: %s\n", b->err.c_str());
		}
	}
};
} // namespace benchmark
#endif /* _BENCHMARK_HPP_ */
