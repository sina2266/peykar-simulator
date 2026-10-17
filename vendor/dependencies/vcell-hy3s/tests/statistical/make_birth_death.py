#!/usr/bin/env python3
"""Write the birth-death reference model, tests/resources/birth_death.nc.

    0 -> X    propensity k          (zeroth order)
    X -> 0    propensity gamma * X  (first order)

with X(0) = x0 molecules. The file has exactly the layout VCell's NetCDFWriter
(vcell-core, cbit.vcell.solver.stoch.NetCDFWriter) produces for a
non-messaging run: the same dimensions, variables, rate-law codes and the same
unit convention for a zeroth-order rate constant (k / 6.02214179e23, which Hy3S
multiplies back by 6.022e23 * Volume).

Its exact solution is known in closed form, which is what makes it a reference
for the three SDE integrators (see check_birth_death.py): X(t) is the sum of a
Binomial(x0, e^{-gamma t}) survivor count and an independent
Poisson(k/gamma (1 - e^{-gamma t})) newborn count.

With --broker HOST:PORT the file also carries the JMS_* variables NetCDFWriter
adds for a server run (bMessaging), pointing a messaging build at that REST
endpoint; the container smoke test uses it against a mock broker.

Usage: make_birth_death.py <out.nc> [--trials N] [--broker HOST:PORT]
"""
import argparse

import numpy as np
from scipy.io import netcdf_file

# Model constants, shared with check_birth_death.py.
K = 1000.0          # birth propensity, molecules / time
GAMMA = 1.0         # death rate constant, 1 / time
X0 = 2000           # initial count
T_END = 3.0
SAVE_TIME = 0.25

VCELL_AVOGADRO = 6.02214179e23   # what NetCDFWriter divides a zeroth-order constant by
HY3S_AVOGADRO = 6.022e23         # what ratelaws.f90 multiplies it back by


def effective_birth_rate() -> float:
    """The birth propensity Hy3S actually uses, after the unit round trip.

    VCell and Hy3S disagree on Avogadro's number in the fourth significant
    figure, so a VCell model's zeroth-order rate reaches the solver scaled by
    6.022 / 6.02214179 (a relative 2.4e-5). Negligible, but modelled exactly.
    """
    return K / VCELL_AVOGADRO * HY3S_AVOGADRO


def fixed_string(s: str, n: int = 72) -> np.ndarray:
    return np.frombuffer(s.ljust(n).encode(), dtype="S1")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--trials", type=int, default=1000)
    ap.add_argument("--broker", help="host:port of a JMS REST bridge; adds the messaging variables")
    args = ap.parse_args()

    n_time = int(round(T_END / SAVE_TIME)) + 1
    f = netcdf_file(args.out, "w", version=1)
    f.createDimension("NumTrials", args.trials)
    f.createDimension("NumSpecies", 1)
    f.createDimension("NumReactions", 2)
    f.createDimension("NumTimePoints", n_time)
    f.createDimension("NumModels", 1)
    f.createDimension("NumMaxDepList", 6)
    f.createDimension("NumMaxStoichList", 25)
    f.createDimension("StringLen", 72)

    def scalar(name, typ, value):
        v = f.createVariable(name, typ, ())
        v[...] = value

    if args.broker:
        # NetCDFWriter pads these with NULs (ucar ArrayChar.setString), which
        # is what lets the C side of msgwrapper read them as C strings.
        for name, value in [("JMS_BROKER", args.broker), ("JMS_USER", "clientUser"),
                            ("JMS_PASSWORD", "dummy"), ("JMS_QUEUE", "workevent"),
                            ("JMS_TOPIC", "servicecontrol"), ("VCELL_USER", "hy3s-smoke")]:
            v = f.createVariable(name, "c", ("StringLen",))
            v[:] = np.frombuffer(value.encode().ljust(72, b"\0"), dtype="S1")
        scalar("SIMULATION_KEY", "i", 123456789)
        scalar("JOB_INDEX", "i", 0)

    scalar("TStart", "d", 0.0)
    scalar("TEnd", "d", T_END)
    scalar("SaveTime", "d", SAVE_TIME)
    scalar("Volume", "d", 1.0)
    scalar("CellGrowthTime", "d", 0.0)
    scalar("CellGrowthTimeSD", "d", 0.0)
    scalar("ExpType", "i", 1)
    scalar("LastTrial", "i", 0)
    scalar("LastModel", "i", 0)
    scalar("MaxNumModels", "i", 1)
    scalar("NumModels", "i", 1)

    def array(name, typ, dims, values):
        v = f.createVariable(name, typ, dims)
        v[:] = values

    array("SpeciesSplitOnDivision", "i", ("NumSpecies",), [0])
    array("SaveSpeciesData", "i", ("NumSpecies",), [1])
    # NetCDFWriter.ReactionRateLaw: order_0 = 1, order_1 = 2.
    array("Reaction_Rate_Laws", "i", ("NumReactions",), [1, 2])
    array("Reaction_DListLen", "i", ("NumReactions",), [0, 1])
    array("Reaction_StoichListLen", "i", ("NumReactions",), [1, 1])
    array("Reaction_OptionalData", "i", ("NumReactions",), [0, 0])

    stoich_coeff = np.zeros((2, 25), dtype=np.int32)
    stoich_species = np.zeros((2, 25), dtype=np.int32)
    stoich_coeff[0, 0], stoich_species[0, 0] = +1, 1    # species indices are 1-based
    stoich_coeff[1, 0], stoich_species[1, 0] = -1, 1
    array("Reaction_StoichCoeff", "i", ("NumReactions", "NumMaxStoichList"), stoich_coeff)
    array("Reaction_StoichSpecies", "i", ("NumReactions", "NumMaxStoichList"), stoich_species)

    dep = np.zeros((2, 6), dtype=np.int32)
    dep[1, 0] = 1
    array("Reaction_DepList", "i", ("NumReactions", "NumMaxDepList"), dep)

    array("Reaction_names", "c", ("NumReactions", "StringLen"),
          np.stack([fixed_string("birth"), fixed_string("death")]))
    array("Species_names", "c", ("NumSpecies", "StringLen"), fixed_string("X")[None, :])
    array("SpeciesIC", "i", ("NumSpecies",), [X0])

    rates = np.zeros((2, 6))
    rates[0, 0] = K / VCELL_AVOGADRO
    rates[1, 0] = GAMMA
    array("Reaction_Rate_Constants", "d", ("NumReactions", "NumMaxDepList"), rates)
    f.close()


if __name__ == "__main__":
    main()
