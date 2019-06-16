/*
 * Example with base class Becnhmark - parameters can be passed to test as class members
 * */

#include <benchmark/benchmark.hpp>

#include <stdexcept>

#include <algorithm>

using namespace std;
using namespace benchmark;

class StdSortBench : public Benchmark {
  public:
	BENCH_CONSTRUCT(StdSortBench);

	size_t data_size = 0;

	int64_t bench() {
		if (data_size == 0)
			throw std::runtime_error("set data_size");

		// test and verify dataset
		vector<int> data = {1, 5, 2, 8, 45, 13, -1, 1};
		vector<int> verify = {-1, 0, 0, 1, 1, 2, 5, 8, 13, 45};

		data.resize(data_size);

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

	{
		StdSortBench b("Sort (10)", "std::sort", &r);
		b.data_size = 10;
		b.run(1000, 10);
	}

	return 0;
}
