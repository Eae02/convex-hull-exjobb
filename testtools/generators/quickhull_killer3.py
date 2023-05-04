import math

# In theory the triangle defining a quickhull recursive call will decrease by at least a factor 4 in area. This means that with
# 11 bit exponent doubles we should at most be able to recurse to a depth of 2^10. This code tries to achieve this but fails.
# Perhaps different starting coordinates will perform better.

S = 10**305     # Max area triangle
eps = 10**-14  # distance that we shift B by


A = (0,0)
B = (1,0)
C = (0.5,10**-100)
area = 1/2

points = [A,B]
n = 400
while area<S:
    
    newC = (-A[0]+2*C[0], -A[1]+2*C[1]) #A mirrored in C
    Bs = B#((1+eps)*B[0]-eps*A[0],(1+eps)*B[1]-eps*A[1]) #B shifted slightly away from A
    newA = (-newC[0]+2*Bs[0], -newC[1]+2*Bs[1]) #newC mirrored in Bs
    points.append(newA)
    A,B,C = A,newA,newC
    area= area*(1+eps)*4

# Append more of last point added
while len(points)<n:
    points.append(A)

points = points[:n] # If we generate too many points
print (2)
print(n)
for i in range(n):
    print(points[i][0],points[i][1])
