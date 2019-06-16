/*
 * Example with base class Becnhmark - no parameters pass to test
 * */

#include <benchmark.hpp>

#include <algorithm>

using namespace std;
using namespace benchmark;

class StdSortBench : public Benchmark {
  public:
	BENCH_CONSTRUCT(StdSortBench);

	int64_t bench() {
		// test and verify dataset
		vector<int> data = {1, 5, 2, 8, 45, 13, -1, 1};
		vector<int> verify = {-1, 1, 1, 2, 5, 8, 13, 45};

		int64_t start = Now();

		sort(std::begin(data), std::end(data));

		int64_t duration = Now() - start;
		if (data != verify) {
			throw runtime_error("result mismatch");
		}
		return duration;
	}
};

int main() {
	BenchmarkStdoutReporter r;

	StdSortBench b("Sort", "std::sort", &r);
	b.run(1000, 10);
	return 0;
}
