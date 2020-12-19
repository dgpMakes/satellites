from constraint import *

problem = Problem()

SAT1 = {
    "time_start": 9,
    "time_end": 13
}
SAT2 = {

}

satellites = []


# Input
different = [("SAT2", "SAT4", "SAT5")]
equal = [("SAT1","SAT2")]


var = dict()

var["SAT1"] = [1, 2, 3, 4]
var["SAT2"] = [1, 2, 3]

var["SAT3_6-12"] = [4, 6]
var["SAT3_13-16"] = [7, 9, 10]

var["SAT4"] = [8, 11, 12]
var["SAT5"] = [1, 7, 12]

var["SAT6_9-13"] = [7, 9]
var["SAT6_13-19"] = [3, 4, 5]

# Add the variables
for sat in var:
    problem.addVariable(sat, var[sat])

# Add the constraint two
for eq in equal:
    problem.addConstraint(AllEqualConstraint(), eq)

# Add the constraint three
for dif in different:
    problem.addConstraint(AllDifferentConstraint(), dif)

# Add the constraint four

def checkSAT(a, b):
    if a == 12: return b != 11
    if b == 11: return a != 12
    return True

def checkIncompat(tupl):
    return True


problem.addConstraint(checkSAT, ("SAT5", "SAT4"))

# Add the constraint five
def checkTime(SAT3, SAT4, SAT5, SAT6):

    # ad-hoc strategy
    if (SAT3 == 7 or SAT4 == 7 or SAT5 == 7 or SAT6 == 7) and (SAT3 == 12 or SAT4 == 12 or SAT5 == 12 or SAT6 == 12):
        sol = ((SAT3 == 7 or SAT4 == 12) and SAT5 != 7 and SAT5 != 12 and SAT6 != 7) or ((SAT5 == 7 or SAT6 == 12 or SAT5 == 12) and SAT4 != 12 and SAT3 != 7)
        return sol

    return True

problem.addConstraint(checkTime,("SAT3_13-16","SAT4","SAT5","SAT6_9-13"))


print(problem.getSolution())
print("Number of solutions -> " + str(len(problem.getSolutions())))