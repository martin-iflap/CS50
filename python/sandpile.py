from collections import deque, Counter
import matplotlib.pyplot as plt
import numpy as np
import matplotlib

matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


size = 50
grid = np.zeros((size, size), dtype=int)


def neighbors(i, j):
    """Yield valid 4-neighborhood coordinates for a grid cell."""
    for di, dj in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
        ni, nj = i + di, j + dj
        if 0 <= ni < size and 0 <= nj < size:
            yield ni, nj


def relax(queue):
    """Relax unstable cells until the queue is empty."""
    avalanche_size = 0

    while queue:
        i, j = queue.popleft()

        if grid[i, j] < 4:
            continue

        topples = grid[i, j] // 4
        grid[i, j] -= 4 * topples
        avalanche_size += topples

        for ni, nj in neighbors(i, j):
            grid[ni, nj] += topples
            if grid[ni, nj] >= 4:
                queue.append((ni, nj))

        if grid[i, j] >= 4:
            queue.append((i, j))

    return avalanche_size

def plot_log(log_ax, sizes, counts):
    """Plot the log-log graph of avalanche size distribution vs probability."""
    log_ax.cla()

    sizes = np.asarray(sizes, dtype=float)
    counts = np.asarray(counts, dtype=float)

    mask = (sizes > 0) & (counts > 0)
    sizes = sizes[mask]
    counts = counts[mask]

    if len(sizes) < 10:
        return

    # create a order of the sizes indexes and order sizes and counts based off of it
    order = np.argsort(sizes)
    sizes = sizes[order]
    counts = counts[order]

    bins = np.logspace(np.log10(min(sizes)), np.log10(max(sizes)), 15)
    digitized = np.digitize(sizes, bins)

    binned_sizes = []
    binned_counts = []

    for i in range(1, len(bins)):
        mask = digitized == i
        if np.any(mask):
            binned_sizes.append(np.exp(np.mean(np.log(sizes[mask]))))
            binned_counts.append(np.sum(counts[mask]))

    if len(binned_counts) == 0:
        return

    binned_counts = np.array(binned_counts)
    binned_counts = binned_counts / np.sum(binned_counts)

    log_ax.scatter(binned_sizes, binned_counts)
    log_ax.set_xscale('log')
    log_ax.set_yscale('log')
    log_ax.set_title("Avalanche distribution")
    log_ax.set_xlabel("Avalanche size")
    log_ax.set_ylabel("Probability")



def drop(img, log_ax, drops=50000, pause=0.01):
    """Drop the sandpiles and call the relax and plot_log functions."""
    counts = Counter()

    for step in range(drops):
        y = np.random.randint(size)
        x = np.random.randint(size)
        grid[y, x] += 1

        if grid[y, x] >= 4:
            avalanche_size = relax(deque([(y, x)]))

            if avalanche_size > 0:
                counts[avalanche_size] += 1

        if step % 20 == 0:
            img.set_data(grid)
            if len(counts) > 30:
                plot_log(log_ax, list(counts.keys()), list(counts.values()))
            plt.pause(pause)


if __name__ == "__main__":
    plt.ion()
    fig, axes = plt.subplots(
        1,
        3,
        figsize=(12, 5),
        gridspec_kw={"width_ratios": [1.0, 0.1, 1.0]}
    )
    plt.subplots_adjust(wspace=0.5)
    ax = axes[0]
    cbar_ax = axes[1]
    log_axes = axes[2]

    image = ax.imshow(grid, cmap="YlOrBr", origin="lower", vmin=1, vmax=4)
    fig.colorbar(image, cax=cbar_ax, label="Sandpile height")
    ax.set_title("Sandpile Grid")
    log_axes.set_title("Avalanche distribution")
    log_axes.set_xlabel("Avalanche size")
    log_axes.set_ylabel("Count")

    plt.show(block=False)

    drop(image, log_axes)

    plt.ioff()
    plt.show()
