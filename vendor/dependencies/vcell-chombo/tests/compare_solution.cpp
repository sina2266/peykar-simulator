// Numerical regression check for the solver's HDF5 output.
//
//   compare_solution --check    <solution.hdf5> <baseline.txt> [rtol] [atol]
//   compare_solution --write    <solution.hdf5> <baseline.txt>
//   compare_solution --analytic <solution.hdf5> <dataset> <expected> [band]
//
// Reads every floating-point dataset in the file, in a stable order, and either
// compares it against a stored baseline or writes that baseline out. The --write
// mode exists so regenerating a baseline is a documented one-liner rather than
// folklore; see tests/README.md.
//
// --analytic is a different kind of check. Given an EXACT expression the solver
// computes its own error against it and records "relative L2 error" as an
// attribute; that mode reads it and compares against what the discrete scheme is
// predicted to produce. See tests/README.md for the derivation.
//
// Why a tool rather than h5diff: the Conan HDF5 package ships no command line
// utilities, and the project already links the HDF5 C API, so this costs one
// small translation unit and no new dependency.

#include <hdf5.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

// VCell marks cells outside the solved region with this value rather than a NaN.
// It must match exactly -- a change in which cells are covered is a real
// regression, not a rounding difference.
constexpr double OUTSIDE_DOMAIN = 1.23456789e+300;

using Fields = std::map<std::string, std::vector<double>>;

herr_t collect(hid_t root, const char* name, const H5O_info2_t* info, void* out)
{
	if (info->type != H5O_TYPE_DATASET) return 0;

	const hid_t dset = H5Dopen2(root, name, H5P_DEFAULT);
	if (dset < 0) return -1;

	const hid_t type = H5Dget_type(dset);
	const bool isFloat = H5Tget_class(type) == H5T_FLOAT;
	H5Tclose(type);
	if (!isFloat)
	{
		// Integer datasets in this output are indices and sizes, not results.
		H5Dclose(dset);
		return 0;
	}

	const hid_t space = H5Dget_space(dset);
	const hssize_t count = H5Sget_simple_extent_npoints(space);
	std::vector<double> values(static_cast<std::size_t>(count));
	const herr_t status = H5Dread(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data());
	H5Sclose(space);
	H5Dclose(dset);
	if (status < 0) return -1;

	static_cast<Fields*>(out)->emplace(name, std::move(values));
	return 0;
}

bool read(const std::string& path, Fields& fields)
{
	const hid_t file = H5Fopen(path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if (file < 0)
	{
		std::cerr << "cannot open " << path << "\n";
		return false;
	}
	// H5_INDEX_NAME gives a deterministic order, so the baseline is stable.
	const herr_t status = H5Ovisit3(file, H5_INDEX_NAME, H5_ITER_INC, collect, &fields, H5O_INFO_BASIC);
	H5Fclose(file);
	if (status < 0)
	{
		std::cerr << "failed reading datasets from " << path << "\n";
		return false;
	}
	return true;
}

bool write(const Fields& fields, const std::string& path)
{
	std::ofstream out(path);
	if (!out)
	{
		std::cerr << "cannot write " << path << "\n";
		return false;
	}
	out << "# vcell-chombo solution baseline\n"
	    << "# regenerate with: compare_solution --write <solution.hdf5> <this file>\n"
	    << "# one '@ <dataset> <count>' header per dataset, then one value per line\n";
	// 17 significant digits round-trips an IEEE double exactly, so re-reading the
	// baseline cannot itself introduce a difference.
	out << std::setprecision(17);
	for (const auto& [name, values] : fields)
	{
		out << "@ " << name << ' ' << values.size() << '\n';
		for (const double v : values) out << v << '\n';
	}
	return out.good();
}

bool load(const std::string& path, Fields& fields)
{
	std::ifstream in(path);
	if (!in)
	{
		std::cerr << "cannot open baseline " << path << "\n";
		return false;
	}
	std::string line, current;
	std::size_t expected = 0;
	while (std::getline(in, line))
	{
		if (line.empty() || line[0] == '#') continue;
		if (line[0] == '@')
		{
			std::istringstream header(line.substr(1));
			header >> current >> expected;
			fields[current].reserve(expected);
			continue;
		}
		if (current.empty())
		{
			std::cerr << "baseline " << path << " has a value before any '@' header\n";
			return false;
		}
		fields[current].push_back(std::stod(line));
	}
	return true;
}

// Mixed absolute/relative comparison. The absolute floor matters because these
// fields legitimately contain values at and near zero, where a pure relative
// test is meaningless.
bool close(double a, double b, double rtol, double atol)
{
	if (std::isnan(a) || std::isnan(b)) return false;
	if (a == b) return true;
	if (a == OUTSIDE_DOMAIN || b == OUTSIDE_DOMAIN) return false;  // handled by a == b above
	return std::fabs(a - b) <= atol + rtol * std::fabs(b);
}

int check(const Fields& actual, const Fields& expected, double rtol, double atol)
{
	int failures = 0;

	for (const auto& [name, want] : expected)
	{
		const auto it = actual.find(name);
		if (it == actual.end())
		{
			std::cerr << "MISSING dataset " << name << "\n";
			++failures;
			continue;
		}
		const std::vector<double>& got = it->second;
		if (got.size() != want.size())
		{
			std::cerr << "SIZE " << name << ": got " << got.size() << ", expected " << want.size() << "\n";
			++failures;
			continue;
		}

		std::size_t bad = 0;
		double worst = 0.0;
		std::size_t worstAt = 0;
		for (std::size_t i = 0; i < want.size(); ++i)
		{
			if (close(got[i], want[i], rtol, atol)) continue;
			++bad;
			const double denom = std::fabs(want[i]) > 0.0 ? std::fabs(want[i]) : 1.0;
			const double rel = std::fabs(got[i] - want[i]) / denom;
			if (rel > worst) { worst = rel; worstAt = i; }
		}
		if (bad > 0)
		{
			++failures;
			std::cerr << "DIFF " << name << ": " << bad << " of " << want.size()
			          << " values outside tolerance; worst at index " << worstAt
			          << " got " << std::setprecision(17) << got[worstAt]
			          << " expected " << want[worstAt]
			          << " (relative " << std::setprecision(3) << worst << ")\n";
		}
		else
		{
			std::cout << "  ok  " << name << " (" << want.size() << " values)\n";
		}
	}

	for (const auto& [name, _] : actual)
	{
		if (expected.find(name) == expected.end())
		{
			std::cerr << "UNEXPECTED dataset " << name << " not in baseline\n";
			++failures;
		}
	}
	return failures;
}

// Reads a scalar double attribute off a dataset, e.g. the "relative L2 error"
// the solver records when an EXACT expression is supplied.
bool readAttribute(const std::string& path, const std::string& dataset,
                   const std::string& attribute, double& value)
{
	const hid_t file = H5Fopen(path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if (file < 0)
	{
		std::cerr << "cannot open " << path << "\n";
		return false;
	}
	bool ok = false;
	const hid_t dset = H5Dopen2(file, dataset.c_str(), H5P_DEFAULT);
	if (dset < 0)
	{
		std::cerr << "no dataset " << dataset << " in " << path << "\n";
	}
	else
	{
		const hid_t attr = H5Aopen(dset, attribute.c_str(), H5P_DEFAULT);
		if (attr < 0)
		{
			std::cerr << "no attribute \"" << attribute << "\" on " << dataset
			          << " -- the input needs an EXACT expression for the solver to compute it\n";
		}
		else
		{
			ok = H5Aread(attr, H5T_NATIVE_DOUBLE, &value) >= 0;
			H5Aclose(attr);
		}
		H5Dclose(dset);
	}
	H5Fclose(file);
	return ok;
}

// Compares the solver's own error against the analytic solution with what the
// discrete scheme is predicted to produce. A plain upper bound would pass just
// as happily on a solver that had become more accurate for the wrong reason, or
// on one whose time integrator had quietly changed order; a band around the
// prediction does not.
int checkAnalytic(const std::string& path, const std::string& dataset,
                  double expected, double band)
{
	double measured = 0.0;
	if (!readAttribute(path, dataset, "relative L2 error", measured)) return 2;

	double maxError = 0.0;
	const bool haveMax = readAttribute(path, dataset, "max error", maxError);

	const double lo = expected * (1.0 - band), hi = expected * (1.0 + band);
	std::cout << "  relative L2 error " << std::setprecision(6) << measured
	          << " against a predicted " << expected
	          << " (accepting " << lo << " to " << hi << ")\n";
	if (haveMax) std::cout << "  max error " << maxError << "\n";

	if (measured < lo || measured > hi)
	{
		std::cerr << "ANALYTIC " << dataset << ": relative L2 error " << measured
		          << " is outside " << lo << " to " << hi << ", predicted " << expected
		          << " (ratio " << measured / expected << ")\n";
		return 1;
	}
	std::cout << "  agrees with the analytic solution to within the discretisation error\n";
	return 0;
}

void usage()
{
	std::cerr << "usage: compare_solution --check    <solution.hdf5> <baseline.txt> [rtol] [atol]\n"
	          << "       compare_solution --write    <solution.hdf5> <baseline.txt>\n"
	          << "       compare_solution --analytic <solution.hdf5> <dataset> <expected> [band]\n";
}

}  // namespace

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		usage();
		return 2;
	}
	const std::string mode = argv[1], solution = argv[2], baseline = argv[3];

	if (mode == "--analytic")
	{
		if (argc < 5)
		{
			usage();
			return 2;
		}
		const double expected = std::stod(argv[4]);
		const double band = argc > 5 ? std::stod(argv[5]) : 0.15;
		std::cout << "checking " << solution << " against the analytic solution\n";
		return checkAnalytic(solution, baseline, expected, band);
	}

	Fields actual;
	if (!read(solution, actual)) return 2;
	if (actual.empty())
	{
		std::cerr << "no floating point datasets found in " << solution << "\n";
		return 2;
	}

	if (mode == "--write")
	{
		if (!write(actual, baseline)) return 2;
		std::cout << "wrote " << actual.size() << " datasets to " << baseline << "\n";
		return 0;
	}
	if (mode != "--check")
	{
		usage();
		return 2;
	}

	const double rtol = argc > 4 ? std::stod(argv[4]) : 1e-9;
	const double atol = argc > 5 ? std::stod(argv[5]) : 1e-12;

	Fields expected;
	if (!load(baseline, expected)) return 2;

	std::cout << "comparing " << solution << " against " << baseline
	          << " (rtol=" << rtol << ", atol=" << atol << ")\n";
	const int failures = check(actual, expected, rtol, atol);
	if (failures > 0)
	{
		std::cerr << failures << " dataset(s) failed\n";
		return 1;
	}
	std::cout << "all " << expected.size() << " datasets match\n";
	return 0;
}
