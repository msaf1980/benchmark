/*
 * Example with templated class TBecnhmark - pass parameters to test
 * */

#include <benchmark.hpp>

#include <algorithm>

using namespace std;
using namespace benchmark;

struct BenchParam {
	size_t data_size;
};

class StdSortBench : public TBenchmark<BenchParam> {
  public:
	BENCH_CONSTRUCT(StdSortBench);

	int64_t bench() {
		// test and verify dataset
		vector<int> data = {1, 5, 2, 8, 45, 13, -1, 1};
		data.resize(param->data_size);
		vector<int> verify = data;
		sort(std::begin(verify), std::end(verify));

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

	StdSortBench b("Sort (10)", "std::sort", &r);
	BenchParam b_param = { 10 };
	b.set_param(&b_param);
	b.run(1000, 10);
	return 0;
}
