#Numerically stable quickhull killer, recurses to a depth of 836.

import math


S = 10**305     # stretch in y direction
factor = -0.5698   # Best factor is -0.5698. Solution to x^4+x^2+x-1 = 0 is 0.56984...
eps = 10**-204  # Smallest angle that we allow
                # We want epsˆ3 * S to still be representable by double

cutoff_quadratic_est = 10**-2 # 1-cos will have bad precision for small angles.

points = []
n = 100000 

angle = math.pi/2
while abs(angle) > eps:
    x = -math.sin(angle)
    if abs(angle) > cutoff_quadratic_est:
        y = -S*(1 - math.cos(angle))
    else:
        y = -S*angle*angle/2
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
