# This generates an input that causes Quickhull to recurse to a depth of 978. 
# However it is not very stable and is dependent on how Quickhull internally computes side of line queries.
import math
S = 10**305     # stretch in x direction
factor = 0.618  # 1/golden ratio rounded down slightly
eps = 10**-204  # Smallest angle that we allow
                # We want epsˆ3 * S to still be representable by double

cutoff_quadratic_est = 10**-2 # 1-cos will just become 0 for small angles.

points = []
n = 100000

points.append((0.0,0.0))
angle = math.pi/2
while angle > eps:
    y = -math.sin(angle) # y needs to be negative, if it is positive our version of quickhull runs into precision issues.
    if angle > cutoff_quadratic_est:
        x = S*(1 - math.cos(angle))
    else:
        x = S*angle*angle/2
    points.append((x,y))
    angle *= factor

# Append more of last point added
while len(points)<n:
    points.append((x,y))

points = points[:n] # If we generate too many points
print (2)
print(n)
for i in range(n):
    print(points[i][0],points[i][1])
