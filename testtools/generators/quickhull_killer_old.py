import math
S = 10**305     # stretch in x direction
factor = 0.618  # 1/golden ratio rounded down slightly
eps = 10**-204  # Smallest angle that we allow
                # We want epsˆ3 * S to still be representable by double

cutoff_quadratic_est = 10**-2 # 1-cos will just become 0 for small angles.

points = []
n = 1000000

points.append((0.0,0.0))
angle = math.pi/2
while angle > eps:
    y = -math.sin(angle) # For some reason this makes qh get the correct answer. Positive y makes qh fail. Don't know why. Other algs get correct for positive y
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
