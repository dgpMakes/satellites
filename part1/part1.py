from constraint import *

problem = Problem()

# Input
different = [("SAT2", "SAT4", "SAT5")]
equal = [("SAT1", "SAT2")]

var = dict()

var["SAT1"] = [1, 2, 3, 4]
var["SAT2"] = [1, 2, 3]

var["SAT3_6-12"] = [4, 6]
var["SAT3_13-16"] = [7, 9, 10]

var["SAT4"] = [8, 11, 12]
var["SAT5"] = [1, 7, 12]

var["SAT6_9-13"] = [7, 9]
var["SAT6_13-19"] = [3, 4, 5]

# Add the variables to the problem
for sat in var:
    problem.addVariable(sat, var[sat])

# Add the constraint two
# Since SAT1 and SAT2 have similar orbits, it is required to assign them the same antenna.
# We use the eq array which contains tuples of satellites that must be assigned the same antenna
for eq in equal:
    problem.addConstraint(AllEqualConstraint(), eq)

# Add the constraint three
# Satellites SAT2, SAT4 and SAT5 should have assigned different antennae.
# We use the dif array which contains tuples of satellites that must be assigned different antennas
for dif in different:
    problem.addConstraint(AllDifferentConstraint(), dif)

# Add the constraint four
# In case SAT5 communicates with ANT12, then SAT4 cannot communicate with ANT11.
def checkSAT(SAT5, SAT4):
    if SAT5 == 12:
        return SAT4 != 11
    return True


problem.addConstraint(checkSAT, ("SAT5", "SAT4"))


# Add the constraint five
# If, in any solution, ANT7 and ANT12 are used, then both must be
# assigned to time slots beginning before 12:00 or after
def checkTime(SAT3, SAT4, SAT5, SAT6):

    # ad-hoc strategy
    if (anyEqual(SAT3, SAT4, SAT5, SAT6, 7)) and (anyEqual(SAT3, SAT4, SAT5, SAT6, 12)):
        sol = ((SAT3 == 7 or SAT4 == 12) and SAT5 != 7 and SAT5 != 12 and SAT6 != 7) or (
            (SAT5 == 7 or SAT6 == 12 or SAT5 == 12) and SAT4 != 12 and SAT3 != 7)
        return sol

    return True


def anyEqual(u, x, y, z, a):
    return (u == a or x == a or y == a or z == a)


problem.addConstraint(checkTime, ("SAT3_13-16", "SAT4", "SAT5", "SAT6_9-13"))

# Get a solution and print it
print(problem.getSolution())
print("Number of solutions ->", len(problem.getSolutions()))
