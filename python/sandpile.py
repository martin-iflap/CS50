from collections import deque, Counter
from matplotlib.widgets import Button
import matplotlib.pyplot as plt
import numpy as np
import matplotlib

matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


size = 50
grid = np.zeros((size, size), dtype=int)
is_active = np.zeros((size, size), dtype=bool)


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

        is_active[i, j] = True  # Mark cell active
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
    for collection in log_ax.collections[:]:
        collection.remove()

    sizes = np.asarray(sizes, dtype=float)
    counts = np.asarray(counts, dtype=float)

    mask = (sizes > 0) & (counts > 0)
    sizes = sizes[mask]
    counts = counts[mask]

    if len(sizes) < 10:
        return

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

    log_ax.scatter(binned_sizes, binned_counts, color="blue")
    log_ax.set_xscale('log')
    log_ax.set_yscale('log')
    log_ax.set_title("Avalanche distribution")
    log_ax.set_xlabel("Avalanche size")
    log_ax.set_ylabel("Probability")


def plot_ccdf(log_ax, data):
    data = np.array(data)
    data = data[data > 0]

    if len(data) < 50:
        return

    data = np.sort(data)
    n = len(data)

    y = np.arange(n, 0, -1) / n

    # for collection in log_ax.collections[:]:
    #     collection.remove()

    log_ax.scatter(data, y, color="green")
    log_ax.set_xscale('log')
    log_ax.set_yscale('log')
    log_ax.set_title("CCDF")
    log_ax.set_xlabel("Avalanche size")
    log_ax.set_ylabel("P(S ≥ s)")


def estimate_alpha(log_ax, all_sizes, line_obj, s_min=1):
    """Estimate the slope using Hill estimator on raw data and update the line object."""
    data = np.array(all_sizes)
    data = data[data >= s_min]

    n = len(data)
    if n == 0:
        return line_obj

    alpha = 1 + n / np.sum(np.log(data / s_min))

    x = np.linspace(min(data), max(data), 100)
    y = x ** (-alpha)

    if line_obj is None:
        line_obj, = log_ax.plot(x, y, color='red', linewidth=2, label=f'α={alpha:.2f}')
        log_ax.legend()
    else:
        line_obj.set_data(x, y)
        line_obj.set_label(f'α={alpha:.2f}')
        log_ax.legend()
    
    return line_obj


def drop(img, log_ax, drops=50000, pause=0.01):
    """Drop the sandpiles and update the grid and log plot every 20 steps.
    - Highlight the sandpiles that are currently part of an avalanche.
    - Call the relax function to relax cells on each drop when height > 4.
    - Call plot_log and plot_ccdf functions to plot the log-log distribution every 20 steps.
    - Call estimate_alpha function to estimate the slope of the distribution and plot it on the log-log graph.
    - Create a stop Button and pause the simulation when the button is clicked, and resume when clicked again.
    - Create an exit Button and plt.close() and break the function when clicked.
    - Calculate and display the statistics of the grid (total mass, average height).
    """
    counts = Counter()
    all_sizes = []
    line_obj = None
    overlay = None
    overlay_data = np.zeros((size, size, 4))
    pause_state = {'paused': False}
    exit_state = {'exit': False}
    
    def pause_callback(event):
        """Toggle pause state when button is clicked."""
        pause_state['paused'] = not pause_state['paused']
        button.label.set_text('Resume' if pause_state['paused'] else 'Pause')
        plt.draw()
    
    def exit_callback(event):
        """Exit the simulation when button is clicked."""
        exit_state['exit'] = True
        plt.close()
    
    # pause button
    button_ax = plt.axes([0.45, 0.02, 0.08, 0.04])
    button = Button(button_ax, 'Pause')
    button.on_clicked(pause_callback)
    
    # exit button
    exit_button_ax = plt.axes([0.55, 0.02, 0.08, 0.04])
    exit_button = Button(exit_button_ax, 'Exit', hovercolor="red")
    exit_button.on_clicked(exit_callback)
    
    # text box for statistics
    stats_text = img.axes.text(-0.37, 0.99, '', transform=img.axes.transAxes,
                               verticalalignment='top', fontsize=10,
                               bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    for step in range(drops):
        # Check exit state
        if exit_state['exit']:
            break
        
        # Check pause state
        if pause_state['paused']:
            plt.pause(0.1)
            continue
        
        y = np.random.randint(size)
        x = np.random.randint(size)
        grid[y, x] += 1

        if grid[y, x] >= 4:
            avalanche_size = relax(deque([(y, x)]))

            if avalanche_size > 0:
                counts[avalanche_size] += 1
                all_sizes.append(avalanche_size)

        if step % 20 == 0:
            img.set_data(grid)

            # Update overlay data with active cells
            overlay_data.fill(0)  # Clear previous overlay
            overlay_data[is_active] = [0, 0, 1, 0.5]  # Blue with 50% alpha for active cells

            if overlay is None:
                overlay = img.axes.imshow(overlay_data, origin="lower", vmin=0, vmax=1, alpha=1)
            else:
                overlay.set_data(overlay_data)

            is_active.fill(False)  # Reset active cells
            
            # Calculate and display statistics
            total_mass = np.sum(grid)
            avg_height = total_mass / (size * size)
            stats_text.set_text(f'Step: {step}\nTotal Mass: {total_mass}\nAvg Height: {avg_height:.2f}')
            
            if len(counts) > 30:
                plot_log(log_ax, list(counts.keys()), list(counts.values()))
                plot_ccdf(log_ax, all_sizes)
                line_obj = estimate_alpha(log_ax, all_sizes, line_obj)

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
