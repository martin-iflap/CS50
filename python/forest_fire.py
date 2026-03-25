from matplotlib.colors import ListedColormap
from matplotlib.widgets import Button
import matplotlib.pyplot as plt
from collections import deque
import numpy as np
import matplotlib



matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


size = 100
grid = np.zeros((size, size), dtype=int)
is_active = np.zeros((size, size), dtype=bool)


def neighbors(i, j):
    """Yield valid 4-neighborhood coordinates for a grid cell."""
    for di, dj in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
        ni, nj = i + di, j + dj
        if 0 <= ni < size and 0 <= nj < size:
            yield ni, nj


def propagate_fire(img, pause: float = 0.01) -> int:
    """Propagate fire through the grid until the queue is empty."""
    current_layer = deque(np.argwhere(grid == 2))

    fire_size: int = 0

    while current_layer:
        next_layer = deque()

        for i, j in current_layer:
            if grid[i, j] == 2:
                fire_size += 1

                for ni, nj in neighbors(i, j):
                    if grid[ni, nj] == 1:
                        grid[ni, nj] = 2
                        next_layer.append((ni, nj))

        img.set_data(grid)
        plt.pause(pause)

        for i, j in current_layer:
            grid[i, j] = 0

        current_layer = next_layer

    return fire_size


def grow_trees(img, steps = 10000, pause = 0.0001, p = 0.01, f = 0.0001) -> None:
    """Grow trees on the grid with a certain probability."""
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

    for s in range(steps):
        # Check exit state
        if exit_state['exit']:
            break

        # Check pause state
        if pause_state['paused']:
            plt.pause(0.1)
            s -= 1
            continue

        growth = (grid == 0) & (np.random.rand(size, size) < p)
        grid[growth] = 1

        lightning = (grid == 1) & (np.random.rand(size, size) < f)
        grid[lightning] = 2
        # if any(lightning):


        fire_size = propagate_fire(img)
        img.set_data(grid)
        plt.pause(pause)


if __name__ == "__main__":
    plt.ion()
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    plt.subplots_adjust(wspace=0.5)
    ax = axes[0]
    log_axes = axes[1]

    cmap = ListedColormap(["white", "green", "red"])
    image = ax.imshow(grid, cmap=cmap, origin="lower", vmin=0, vmax=2)
    ax.set_title("Forest Fire Simulation")

    log_axes.set_title("Fire distribution")
    log_axes.set_xlabel("Fire size")
    log_axes.set_ylabel("Frequency")

    plt.show(block=False)

    grow_trees(image)

    plt.ioff()
    plt.show()
