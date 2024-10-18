import numpy as np
import matplotlib.pyplot as plt

def persistence_landscape(diagram, resolution=1000):
    """
    Computes the persistence landscape from a persistence diagram.

    Parameters:
    diagram (list of tuples): A list of (birth, death) tuples representing the persistence diagram.
    resolution (int): The number of points to evaluate the landscape at.

    Returns:
    xs (numpy array): The x-coordinates where the landscape is evaluated.
    landscape (numpy array): The y-coordinates of the landscape.
    """
    # Extract birth and death times
    births = np.array([point[0] for point in diagram])
    deaths = np.array([point[1] for point in diagram])

    # Compute midpoints and heights
    midpoints = (births + deaths) / 2
    heights = (deaths - births) / 2

    # Create the x-coordinates for the landscape
    xs = np.linspace(min(births), max(deaths), resolution)

    # Initialize the landscape
    landscape = np.zeros((len(diagram), resolution))

    # Compute the landscape
    for i, (b, d, m, h) in enumerate(zip(births, deaths, midpoints, heights)):
        left = np.maximum(0, xs - b)
        right = np.maximum(0, d - xs)
        landscape[i, :] = np.minimum(left, right)

    # Sum the individual landscapes to get the final landscape
    final_landscape = np.sum(landscape, axis=0)

    return xs, final_landscape

# Example usage
diagram = [(1, 4), (2, 3)]
xs, landscape = persistence_landscape(diagram)

# Plot the persistence landscape
plt.plot(xs, landscape)
plt.xlabel('Time')
plt.ylabel('Persistence Landscape')
plt.title('Persistence Landscape')
plt.show()
