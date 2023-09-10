import numpy as np
from numba import njit
from tqdm import tqdm
import matplotlib.pyplot as plt
import matplotlib.animation as animation

@njit
def neighbours(c, L):
    """Find the neighbouring sites of a site on a square lattice.
    
    Parameters
    ----------
    c : numpy.ndarray
        Coordinates of the site.
    L : int
    Side length of the square lattice.
    
    Returns
    -------
    numpy.ndarray
    Coordinates of the neighbouring sites.
    """

    return np.array([[(c[0]-1)%L, c[1]], [(c[0]+1)%L, c[1]], [c[0], (c[1]-1)%L], [c[0], (c[1]+1)%L]])

@njit
def init_lattice(L, N):
    """Initialize the lattice with N bacteria randomly placed on the lattice.

    Parameters
    ----------
    L : int
        Side length of the square lattice.
    N : int
        Number of bacteria to place on the lattice.

    Returns
    -------
    soil_lattice : numpy.ndarray
        Lattice with bacteria randomly placed on it.
    """

    soil_lattice = np.ones((L, L), dtype=np.int8)
    # note about lattice:
    #   0 = empty
    #   1 = soil
    #   2 = bacteria
    # set half the sites to 0
    empty_sites = np.random.choice(L*L, size=L*L//2, replace=False)
    for site in empty_sites:
        row = site // L
        col = site % L
        soil_lattice[row, col] = 0
    # choose random sites to place N bacteria
    sites = np.random.choice(L*L, size=N, replace=False)
    # place bacteria on the lattice
    for site in sites:
        row = site // L
        col = site % L
        soil_lattice[row, col] = 2
    return soil_lattice


@njit
def update(soil_lattice, L, r, d, s):
    """Update the lattice. Called once every timestep.

    The function mutates a global variable, to avoid slowdowns from numba primitives.
    
    Parameters:
    -----------
    soil_lattice : numpy.ndarray
        Lattice with bacteria randomly placed on it.
    L : int
        Side length of the square lattice.
    r : float
        Reproduction rate.
    d : float
        Death rate.
    s : float
        Soil filling rate.
    
    Returns:
    --------
    None
    """

    # NEW SOIL FILLING MECHANICS
    n_sites_to_fill = s * L**2
    # randomly choose n_sites_to_fill on the lattice
    sites_to_fill = np.random.choice(L*L, size=int(n_sites_to_fill), replace=False)
    # fill the sites
    for site in sites_to_fill:
        row = site // L
        col = site % L
        if soil_lattice[row, col] == 0:
            soil_lattice[row, col] = 1

    # NEW DEATH MECHANICS
    n_sites_to_kill = d * L**2
    # randomly choose n_sites_to_fill on the lattice
    sites_to_kill = np.random.choice(L*L, size=int(n_sites_to_kill), replace=False)
    # kill the sites
    for site in sites_to_kill:
        row = site // L
        col = site % L
        if soil_lattice[row, col] == 2:
            soil_lattice[row, col] = 0

    
    # find bacteria sites
    bacteria_sites = np.argwhere(soil_lattice == 2)

    # check if there are no bacteria left
    if len(bacteria_sites) == 0:
        return

    # choose a random bacteria site
    site = bacteria_sites[np.random.randint(len(bacteria_sites))]
    # select a random neighbour
    new_site = neighbours(site, L)[np.random.randint(4)]
    # check the value of the new site
    new_site_value = soil_lattice[new_site[0], new_site[1]]
    # move the bacteria
    soil_lattice[new_site[0], new_site[1]] = 2
    soil_lattice[site[0], site[1]] = 0

    # check if the new site is soil
    if new_site_value == 1:
        # find neighbouring sites
        neighbours_sites = neighbours(new_site, L)
        for nbr in neighbours_sites:  # todo: Optimize
            if (nbr[0], nbr[1]) != (site[0], site[1]):
                if soil_lattice[nbr[0], nbr[1]] == 0:
                    if np.random.rand() < r:
                        soil_lattice[nbr[0], nbr[1]] = 2
                        break

    # check if the new site is a bacteria
    elif new_site_value == 2:
        # keep both with bacteria (undo the vacant space in original site)
        soil_lattice[site[0], site[1]] = 2

    

def main():

    # initialize the parameters
    n_steps = 1_000_000  # number of bacteria moves
    L = 10  # side length of the square lattice
    N = int(L**2 / 10)  # initial number of bacteria
    r = 1  # reproduction rate
    d = 0.02  # death rate
    s = 0.1  # soil filling rate
    soil_lattice = init_lattice(L, N)

    n_frames = 100  # number of potential frames in the animation (will be less in practice because only unique frames are saved)
    datasteps = np.geomspace(1, n_steps, n_frames, dtype=np.int32)  # steps at which to save the data
    datasteps = np.unique(datasteps)  # remove duplicates
    n_frames = len(datasteps)  # update the number of frames

    soil_lattice_data = np.zeros((len(datasteps), L, L), dtype=np.int8)
    soil_lattice_data[0] = soil_lattice

    # run the simulation
    for step in tqdm(range(1, n_steps+1)):
        update(soil_lattice, L, r, d, s)
        if step in datasteps:
            soil_lattice_data[np.where(datasteps == step)] = soil_lattice

    # animate the lattice
    fig, ax = plt.subplots()
    ax.set_xticks(np.arange(-.5, L, 1), minor=True)
    ax.set_yticks(np.arange(-.5, L, 1), minor=True)
    ax.set_xticklabels([])
    ax.set_yticklabels([])
    ax.grid(which='minor', linewidth=2)
    ax.set_title(f"{L=}, {r=:.2f}, {d=:.2f}, {s=:.2f}\nstep {datasteps[0]}")
    im = ax.imshow(soil_lattice_data[0], cmap="cubehelix_r", vmin=0, vmax=2)
    def animate(i):
        ax.set_title(f"{L=}, {r=:.2f}, {d=:.2f}, {s=:.2f}\nstep {datasteps[i]}")
        im.set_data(soil_lattice_data[i])
        return im,
    ani = animation.FuncAnimation(fig, animate, frames=n_frames, interval=1000, blit=True)
    ani.save("src/single_species_RW/single_species_logspace.gif", fps=1)

    

if __name__ == "__main__":
    main()
