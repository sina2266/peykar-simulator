/*
 * hy3s_check -- assertions over a Hy3S solution file.
 *
 * Hy3S writes its solution back into the NetCDF file it was given: Time(t) and
 * State(trial, t, species). This checks properties of that solution.
 *
 * The interesting ones are exact and hold for any seed, any platform and any
 * of the three integrators, because they follow from the reaction
 * stoichiometry rather than from the trajectory:
 *
 *   --conserve A,B      the named species sum to the same total at every
 *                       timepoint as they did at t=0
 *   --monotonic P       the named species never decreases
 *   --nonneg            no species count is negative
 *
 * --ran is the blunt one, and the reason this program exists at all. A bug
 * fixed during the repo split left the solver exiting 0 after writing nothing,
 * so "it did not crash" was not evidence that it had run. --ran fails unless
 * the solution actually reaches TEnd and something happened.
 *
 * --baseline compares against a committed reference run. That is the only
 * check here that depends on the trajectory, so it is also the only one that
 * could legitimately differ between platforms.
 *
 * Written in C against the vendored NetCDF C library so the tests need nothing
 * that the solver itself does not already need, on any of the three platforms.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netcdf.h"

#define MAX_SEL 64

static void fail_nc(const char *what, int status)
{
	fprintf(stderr, "hy3s_check: %s: %s\n", what, nc_strerror(status));
	exit(2);
}

struct solution {
	int      ntrials, ntimes, nspecies, namelen;
	double  *time;          /* [ntimes] */
	double  *state;         /* [ntrials][ntimes][nspecies] */
	char    *names;         /* [nspecies][namelen], space padded */
	double   tend;
};

/* State is (trial, time, species) in that order, matching the Fortran writer's
 * NetCDF dimension order as seen from C. */
static double at(const struct solution *s, int trial, int t, int sp)
{
	return s->state[((size_t)trial * s->ntimes + t) * s->nspecies + sp];
}

static size_t dim_len(int nc, const char *name)
{
	int id, status;
	size_t len;

	status = nc_inq_dimid(nc, name, &id);
	if (status != NC_NOERR)
		fail_nc(name, status);
	status = nc_inq_dimlen(nc, id, &len);
	if (status != NC_NOERR)
		fail_nc(name, status);
	return len;
}

static void read_double_var(int nc, const char *name, double *out)
{
	int id, status;

	status = nc_inq_varid(nc, name, &id);
	if (status != NC_NOERR)
		fail_nc(name, status);
	status = nc_get_var_double(nc, id, out);
	if (status != NC_NOERR)
		fail_nc(name, status);
}

static void load(const char *path, struct solution *s)
{
	int nc, status, id;

	status = nc_open(path, NC_NOWRITE, &nc);
	if (status != NC_NOERR) {
		fprintf(stderr, "hy3s_check: cannot open %s: %s\n", path, nc_strerror(status));
		exit(2);
	}

	s->ntrials  = (int)dim_len(nc, "NumTrials");
	s->ntimes   = (int)dim_len(nc, "NumTimePoints");
	s->nspecies = (int)dim_len(nc, "NumSpecies");
	s->namelen  = (int)dim_len(nc, "StringLen");

	s->time  = malloc(sizeof(double) * s->ntimes);
	s->state = malloc(sizeof(double) * (size_t)s->ntrials * s->ntimes * s->nspecies);
	s->names = malloc((size_t)s->nspecies * s->namelen + 1);
	if (!s->time || !s->state || !s->names) {
		fprintf(stderr, "hy3s_check: out of memory\n");
		exit(2);
	}

	read_double_var(nc, "Time", s->time);
	read_double_var(nc, "State", s->state);
	read_double_var(nc, "TEnd", &s->tend);

	status = nc_inq_varid(nc, "Species_names", &id);
	if (status != NC_NOERR)
		fail_nc("Species_names", status);
	status = nc_get_var_text(nc, id, s->names);
	if (status != NC_NOERR)
		fail_nc("Species_names", status);

	nc_close(nc);
}

/* Species names are fixed-width and padded, with either spaces or NULs
 * depending on what the writer left behind, so compare on the trimmed form. */
static int species_index(const struct solution *s, const char *want)
{
	int i, n;
	char buf[512];

	for (i = 0; i < s->nspecies; i++) {
		memcpy(buf, s->names + (size_t)i * s->namelen,
		       (size_t)s->namelen < sizeof(buf) ? (size_t)s->namelen : sizeof(buf) - 1);
		n = s->namelen < (int)sizeof(buf) ? s->namelen : (int)sizeof(buf) - 1;
		buf[n] = '\0';
		while (n > 0 && (buf[n - 1] == ' ' || buf[n - 1] == '\0'))
			buf[--n] = '\0';
		if (strcmp(buf, want) == 0)
			return i;
	}
	fprintf(stderr, "hy3s_check: no species named '%s'. Present:", want);
	for (i = 0; i < s->nspecies; i++) {
		memcpy(buf, s->names + (size_t)i * s->namelen, (size_t)s->namelen);
		buf[s->namelen] = '\0';
		n = s->namelen;
		while (n > 0 && (buf[n - 1] == ' ' || buf[n - 1] == '\0'))
			buf[--n] = '\0';
		fprintf(stderr, " '%s'", buf);
	}
	fprintf(stderr, "\n");
	exit(2);
}

static int split_names(char *arg, char **out)
{
	int n = 0;
	char *tok = strtok(arg, ",");

	while (tok && n < MAX_SEL) {
		out[n++] = tok;
		tok = strtok(NULL, ",");
	}
	return n;
}

static void usage(void)
{
	fprintf(stderr,
		"usage: hy3s_check <solution.nc> [checks]\n"
		"  --ran                    solution reaches TEnd and is not all zero\n"
		"  --nonneg                 no negative species counts\n"
		"  --conserve A,B[,C]       named species sum to a constant over time\n"
		"  --monotonic S            named species never decreases\n"
		"  --baseline ref.nc        compare Time and State against a reference\n"
		"  --rtol X                 relative tolerance for --baseline (default 1e-9)\n");
	exit(2);
}

int main(int argc, char **argv)
{
	struct solution s, ref;
	const char *path = NULL, *baseline = NULL;
	/* A model can have several independent conservation laws -- the enzyme
	 * kinetics used by the tests has two -- so --conserve may be repeated. */
	char *conserve_groups[MAX_SEL];
	int nconserve = 0, g;
	char *conserve = NULL, *monotonic = NULL;
	double rtol = 1e-9;
	int do_ran = 0, do_nonneg = 0;
	int i, t, sp, trial, failures = 0;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--ran") == 0)
			do_ran = 1;
		else if (strcmp(argv[i], "--nonneg") == 0)
			do_nonneg = 1;
		else if (strcmp(argv[i], "--conserve") == 0 && i + 1 < argc) {
			if (nconserve >= MAX_SEL) {
				fprintf(stderr, "hy3s_check: too many --conserve groups\n");
				exit(2);
			}
			conserve_groups[nconserve++] = argv[++i];
		}
		else if (strcmp(argv[i], "--monotonic") == 0 && i + 1 < argc)
			monotonic = argv[++i];
		else if (strcmp(argv[i], "--baseline") == 0 && i + 1 < argc)
			baseline = argv[++i];
		else if (strcmp(argv[i], "--rtol") == 0 && i + 1 < argc)
			rtol = atof(argv[++i]);
		else if (argv[i][0] == '-')
			usage();
		else if (!path)
			path = argv[i];
		else
			usage();
	}
	if (!path)
		usage();

	load(path, &s);
	printf("hy3s_check: %s -- %d trial(s), %d timepoints, %d species\n",
	       path, s.ntrials, s.ntimes, s.nspecies);

	if (do_ran) {
		/* The bug this exists for wrote nothing and exited 0, leaving Time
		 * and State full of the zeros the model file was created with. */
		double tmax = s.time[s.ntimes - 1];
		int any_nonzero = 0;

		for (i = 0; i < s.ntrials * s.ntimes * s.nspecies && !any_nonzero; i++)
			if (s.state[i] != 0.0)
				any_nonzero = 1;

		if (fabs(tmax - s.tend) > 1e-6 * (fabs(s.tend) > 0 ? fabs(s.tend) : 1.0)) {
			printf("  FAIL ran: last timepoint %.10g, expected TEnd %.10g\n", tmax, s.tend);
			failures++;
		} else if (!any_nonzero) {
			printf("  FAIL ran: every state value is zero -- the solver wrote nothing\n");
			failures++;
		} else {
			printf("  ok   ran: reaches TEnd = %g with a non-trivial solution\n", s.tend);
		}
	}

	if (do_nonneg) {
		int bad = 0;
		double worst = 0.0;

		for (i = 0; i < s.ntrials * s.ntimes * s.nspecies; i++)
			if (s.state[i] < 0.0) {
				bad++;
				if (s.state[i] < worst)
					worst = s.state[i];
			}
		if (bad) {
			printf("  FAIL nonneg: %d negative value(s), most negative %.10g\n", bad, worst);
			failures++;
		} else {
			printf("  ok   nonneg: no negative species counts\n");
		}
	}

	for (g = 0; g < nconserve; g++) {
		char *names[MAX_SEL];
		char label[512];
		int idx[MAX_SEL], n, bad = 0;
		double worst = 0.0;

		conserve = conserve_groups[g];
		/* split_names replaces the commas with NULs, so keep the readable
		 * form for the message before taking it apart. */
		snprintf(label, sizeof(label), "%s", conserve);
		n = split_names(conserve, names);
		for (i = 0; i < n; i++)
			idx[i] = species_index(&s, names[i]);

		for (trial = 0; trial < s.ntrials; trial++) {
			double total0 = 0.0;

			for (i = 0; i < n; i++)
				total0 += at(&s, trial, 0, idx[i]);

			for (t = 1; t < s.ntimes; t++) {
				double total = 0.0, diff;

				for (i = 0; i < n; i++)
					total += at(&s, trial, t, idx[i]);
				diff = fabs(total - total0);
				if (diff > worst)
					worst = diff;
				/* These are molecule counts held in doubles, so the sum
				 * is exact and any drift at all is a real violation. */
				if (diff != 0.0) {
					if (bad == 0)
						printf("  FAIL conserve: trial %d t=%d sum %.10g, expected %.10g\n",
						       trial, t, total, total0);
					bad++;
				}
			}
		}
		if (bad) {
			printf("  FAIL conserve: %s -- %d violation(s), worst drift %.10g\n",
			       label, bad, worst);
			failures++;
		} else {
			printf("  ok   conserve: %s constant over all timepoints\n", label);
		}
	}

	if (monotonic) {
		int idx = species_index(&s, monotonic), bad = 0;

		for (trial = 0; trial < s.ntrials; trial++)
			for (t = 1; t < s.ntimes; t++)
				if (at(&s, trial, t, idx) < at(&s, trial, t - 1, idx)) {
					if (bad == 0)
						printf("  FAIL monotonic: %s fell from %.10g to %.10g at t=%d\n",
						       monotonic, at(&s, trial, t - 1, idx),
						       at(&s, trial, t, idx), t);
					bad++;
				}
		if (bad) {
			printf("  FAIL monotonic: %d decrease(s) in %s\n", bad, monotonic);
			failures++;
		} else {
			printf("  ok   monotonic: %s never decreases\n", monotonic);
		}
	}

	if (baseline) {
		int bad = 0;
		double worst = 0.0;

		load(baseline, &ref);
		if (ref.ntrials != s.ntrials || ref.ntimes != s.ntimes ||
		    ref.nspecies != s.nspecies) {
			printf("  FAIL baseline: shape %dx%dx%d does not match reference %dx%dx%d\n",
			       s.ntrials, s.ntimes, s.nspecies,
			       ref.ntrials, ref.ntimes, ref.nspecies);
			failures++;
		} else {
			for (t = 0; t < s.ntimes; t++) {
				double d = fabs(s.time[t] - ref.time[t]);
				double scale = fabs(ref.time[t]) > 1.0 ? fabs(ref.time[t]) : 1.0;

				if (d / scale > rtol) {
					if (bad == 0)
						printf("  FAIL baseline: Time[%d] %.17g vs %.17g\n",
						       t, s.time[t], ref.time[t]);
					bad++;
				}
				if (d / scale > worst)
					worst = d / scale;
			}
			for (trial = 0; trial < s.ntrials; trial++)
				for (t = 0; t < s.ntimes; t++)
					for (sp = 0; sp < s.nspecies; sp++) {
						double a = at(&s, trial, t, sp), b = at(&ref, trial, t, sp);
						double scale = fabs(b) > 1.0 ? fabs(b) : 1.0;
						double d = fabs(a - b) / scale;

						if (d > rtol) {
							if (bad == 0)
								printf("  FAIL baseline: State[%d][%d][%d] %.17g vs %.17g\n",
								       trial, t, sp, a, b);
							bad++;
						}
						if (d > worst)
							worst = d;
					}
			if (bad) {
				printf("  FAIL baseline: %d value(s) differ, worst relative %.3g (rtol %.3g)\n",
				       bad, worst, rtol);
				failures++;
			} else {
				printf("  ok   baseline: matches %s within rtol %.3g (worst %.3g)\n",
				       baseline, rtol, worst);
			}
		}
	}

	if (failures)
		printf("hy3s_check: %d check(s) FAILED\n", failures);
	else
		printf("hy3s_check: all checks passed\n");
	return failures ? 1 : 0;
}
