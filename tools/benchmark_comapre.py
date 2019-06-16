#!/usr/bin/env python

import argparse
import sys


class Benchmark:
    def __init__(self, group, benchmark, threads):
        self.group = group
        self.benchmark = benchmark
        self.threads = threads

    def __str__(self):
        return "%s %s %d" % (self.group, self.benchmark, self.threads)

    def __eq__(self, other):
        if not isinstance(other, type(self)):
            return False
        else:
            return self.group == other.group and \
                   self.benchmark == other.benchmark and \
                   self.threads == other.threads

    def __hash__(self):
        return hash((self.group, self.benchmark, self.threads))


class BenchmarkStat:
    def __init__(self):
        self.p90 = 0.0
        self.p95 = 0.0
        self.p99 = 0.0

    def __str__(self):
        return "%f %f %f" % (self.p90, self.p95, self.p99)


def load_benchmark(filename):
    with open(filename, "r") as f:
        line = f.readline()
        if line.startswith("----"):
            first = True
            index = dict()
            result = dict()
            for line in f:
                if line.startswith("----"):
                    continue
                fields = [x.strip() for x in line.split("|")]
                if first:
                    first = False
                    if fields[0] == "Group" and fields[
                            1] == "Benchmark" and fields[2] == "Threads":
                        for i in range(len(fields)):
                            if fields[i] == "Group":
                                index['group'] = i
                            elif fields[i] == "Benchmark":
                                index['benchmark'] = i
                            elif fields[i] == "Threads":
                                index['threads'] = i
                            elif fields[i] == "us/Iter P90":
                                index["P90"] = i
                            elif fields[i] == "P95":
                                index["P95"] = i
                            elif fields[i] == "P99":
                                index["P99"] = i
                    else:
                        raise ValueError("invalid header in %s" % filename)

                else:
                    b = Benchmark(fields[index['group']],
                                  fields[index['benchmark']],
                                  fields[index['threads']])
                    stat = BenchmarkStat()
                    stat.p90 = float(fields[index['P90']])
                    stat.p95 = float(fields[index['P95']])
                    stat.p99 = float(fields[index['P99']])

                    #print("(%s) = %s" % (b, stat))
                    result[b] = stat

            return result


def format_benchmark_stat_diff(e, i):
    return ("%.2f (%.2f%s)" % (i.p90, 100 * (i.p90 - e.p90) / i.p90, '%'),
            "%.2f (%.2f%s)" % (i.p95, 100 * (i.p95 - e.p95) / i.p95, '%'),
            "%.2f (%.2f%s)" % (i.p99, 100 * (i.p99 - e.p99) / i.p99, '%'))



def print_benchmark_diff(etalon, input):
    s_delim = '-' * 144
    sys.stdout.write("%s\n" % s_delim)
    sys.stdout.write(
        "%26s | %20s | %8s | %25s | %24s | %24s |\n" %
        ("Group", "Benchmark", "Threads", "us/Iter P90", "P95", "P99"))
    sys.stdout.write("%s\n" % s_delim)
    for k in input:
        e = etalon.get(k)
        sys.stdout.write("%26s | %20s | %8s | " %
                         (k.group, k.benchmark, k.threads))
        if e is None:
            sys.stdout.write("not found\n")
        else:
            i = input[k]
            sys.stdout.write(" %24s | %24s | %24s |\n" %
                             format_benchmark_stat_diff(e, i))


def parse_cmdline():
    parser = argparse.ArgumentParser(description='Compare benchmark result')

    parser.add_argument('-e',
                        '--etalon',
                        dest='etalon',
                        action='store',
                        type=str,
                        required=True,
                        help='etalon file')

    parser.add_argument('-i',
                        '--input',
                        dest='input',
                        action='store',
                        type=str,
                        required=True,
                        help='input file')

    return parser.parse_args()


def main():
    args = parse_cmdline()
    etalon = load_benchmark(args.etalon)
    input = load_benchmark(args.input)
    print_benchmark_diff(etalon, input)


if __name__ == "__main__":
    main()
