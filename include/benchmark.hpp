#ifndef _BENCHMARK_HPP_
#define _BENCHMARK_HPP_

#include <math.h>
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
	int64_t max;
	int64_t min;
	int64_t avg;
	int64_t p90;
	int64_t p95;
	int64_t p99;
	int64_t div_min;
	int64_t div_max;
};

int64_t percentile(std::vector<int64_t>::const_iterator b,
                   std::vector<int64_t>::const_iterator e, size_t pcnt) {
	size_t size = std::distance(b, e);
	if (size > 1) {
		size_t i = pcnt * size / 100;
		if (size % 2 == 0 && i < size - 2) {
			return (*std::next(b, i) + *std::next(b, i + 1)) / 2;
		} else {
			return *std::next(b, i);
		}
	} else if (size == 1) {
		return *b;
	} else
		return 0;
}

BenchmarkStat calc_stat(std::vector<int64_t> &r) {
	BenchmarkStat stat = {0, 0, 0, 0, 0, 0, 0, 0};
	std::sort(r.begin(), r.end());
	stat.max = r[r.size() - 1];
	stat.min = r[0];

	size_t last = r.size() - 1;
	/* trunc high deviated result */
	while (last > 20 && last > r.size() - 5) {
		stat.p95 = percentile(r.cbegin(), std::next(r.cbegin(), last + 1), 95);
		stat.div_max = stat.max - stat.p95;
		if (stat.div_max > stat.p95)
			last--;
		else
			break;
	}

	auto end_it = std::next(r.cbegin(), last + 1);
	stat.max = r[last];
	stat.avg = percentile(r.cbegin(), end_it, 50);
	stat.p90 = percentile(r.cbegin(), end_it, 90);
	stat.p95 = percentile(r.cbegin(), end_it, 95);
	stat.p99 = percentile(r.cbegin(), end_it, 99);
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
		durations.reserve(samples);
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
						duration += d;
					} else {
						throw std::runtime_error("negative duration");
					}
				}
				durations.push_back(duration / iterations);
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

	std::string          group;
	std::string          name;
	size_t               threads;
	size_t               samples;
	size_t               iterations;
	bool                 success;
	std::string          err;
	std::vector<int64_t> durations;

	BenchmarkReporter *r;
};

class BenchmarkStdoutReporter : public BenchmarkReporter {
  public:
	BenchmarkStdoutReporter() {
		std::string s_delim(135, '_');
		printf("%s\n", s_delim.c_str());
		printf("%10s | %10s | %8s | %10s | %10s | %14s | %14s | %14s | %20s |\n",
		       "Group", "Benchmark", "Threads", "Samples", "Iterations",
		       "us/Iter P90" ,"P95", "P99", "P95 Div Min/Max");
	}

	virtual void report(Benchmark *b) {
		printf("%10s | %10s | %8lu | %10lu | %10lu |", b->group.c_str(),
		       b->name.c_str(), b->threads, b->samples, b->iterations);
		if (b->success && !b->durations.empty()) {
			BenchmarkStat stat = calc_stat(b->durations);
/*
			std::cout << std::endl;
			for (auto r : b->durations) {
				std::cout << r << std::endl;
			}
*/
			std::string div = (stat.div_min == 0 ? "" : "-") +
			                  std::to_string(stat.div_min) + "/" +
			                  std::to_string(stat.div_max);
			printf(" %14lu | %14lu | %14lu | %20s |\n", stat.p90,
			       stat.p95, stat.p99, div.c_str());
		} else if (b->err.empty()) {
			printf(" SKIP\n");
		} else {
			printf(" FAIL: %s\n", b->err.c_str());
		}
	}
};
} // namespace benchmark
#endif /* _BENCHMARK_HPP_ */
