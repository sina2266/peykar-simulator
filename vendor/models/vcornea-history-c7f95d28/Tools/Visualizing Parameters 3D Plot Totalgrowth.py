import numpy as np
import matplotlib.pyplot as plt

# Constants for independent growth factors
# EGF_BASAL_HalfMaxValue = 2.25 
# DensityBASAL_HalfMaxValue = 65.0

# Constants for synergistic interaction of growth factors
EGF_BASAL_HalfMaxValue = 3.25
DensityBASAL_HalfMaxValue = 65.0


BASAL_doubling = ((1/(10*8))*25.0)  

BASAL_doubling

DensityGrowthScalar_BASAL = 1  
EGF_GrowthScalar_BASAL = 1  

# Define the function
def TotalGrowth(x, y):
    EGF_Growth = x**4 / (EGF_BASAL_HalfMaxValue**4 + x**4)
    DensityGrowth = DensityBASAL_HalfMaxValue**4 / (DensityBASAL_HalfMaxValue**4 + y**4)
    return (EGF_Growth * DensityGrowth) # for Independent Growth Factors (+)

def growth_rate_hill(x, max_growth_rate, K, n):
    return max_growth_rate * (x**n / (K**n + x**n))

# Generate meshgrid for x and y
x = np.linspace(0, 14, 100)  # The range here is just an example; adjust as per your requirements
y = np.linspace(0, 300, 100)
X, Y = np.meshgrid(x, y)

# Calculate Z values
Z = growth_rate_hill(TotalGrowth(X, Y), BASAL_doubling, 1, 2)
# Z = TotalGrowth(X, Y) * BASAL_doubling

# Plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot_surface(X, Y, Z, cmap='viridis')
ax.set_xlabel('EGF')
ax.set_ylabel('Pressure')
ax.set_zlabel('TotalGrowth')
ax.set_title('3D Surface plot of TotalGrowth')

# Adding the plane for TotalGrowth = 0.208 equivalent to 12 hours doubling time
Z_plane = np.full((100, 100), 0.208)
ax.plot_surface(X, Y, Z_plane, color='r', alpha=0.5)

# Calculate intersection point (this is an approximation based on the grid resolution)
intersection_points = []
for i in range(100):
    for j in range(100):
        if abs(Z[i, j] - 0.208) < 0.005:  # A threshold to determine if points are close enough
            intersection_points.append((X[i, j], Y[i, j], Z[i, j]))
# print(intersection_points)
# We're assuming there might be multiple points of intersection 
# and hence plotting all of them.
# for point in intersection_points:
#     ax.scatter(point[0], point[1], point[2], color='black', s=100)  # s sets the size of the point

plt.show()