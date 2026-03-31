from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button, Slider
from matplotlib.colors import ListedColormap
import matplotlib.pyplot as plt
from collections import deque
import numpy as np
import matplotlib



matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


size = 200
grid = np.zeros((size, size), dtype=int)
is_active = np.zeros((size, size), dtype=bool)


def neighbors(i, j):
    """Yield valid 4-neighborhood coordinates for a grid cell."""
    for di, dj in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
        ni, nj = i + di, j + dj
        if 0 <= ni < size and 0 <= nj < size:
            yield ni, nj

current_layer = deque()
fire_active: bool = False


def update(img, pause_s: dict, exit_s: dict, p: float = 0.001, f: float = 0.0001, grow_while_fire: bool = True) -> list:
    """Update the grid for the animation. This function is called by FuncAnimation."""
    global current_layer, fire_active

    if pause_s['paused']:
        plt.pause(0.1)
        return [img]

    if exit_s['exit']:
        plt.close()
        return [img]

    if not fire_active or grow_while_fire:
        growth = (grid == 0) & (np.random.rand(size, size) < p)
        grid[growth] = 1

    lightning = (grid == 1) & (np.random.rand(size, size) < f)
    grid[lightning] = 2

    if np.any(lightning):
        current_layer = deque(np.argwhere(grid == 2))
        fire_active = True

    if fire_active:
        next_layer = deque()

        for i, j in current_layer:
            for ni, nj in neighbors(i, j):
                if grid[ni, nj] == 1:
                    grid[ni, nj] = 2
                    next_layer.append((ni, nj))

        for i, j in current_layer:
            grid[i, j] = 0

        current_layer = next_layer

        if not current_layer:
            fire_active = False

    img.set_data(grid)
    return [img]


def main() -> None:
    """Main function to set up the plot and start the animation."""
    fig, ax = plt.subplots(figsize=(7, 7))

    cmap = ListedColormap(["white", "green", "red"])
    image = ax.imshow(grid, cmap=cmap, origin="lower", vmin=0, vmax=2)
    ax.set_title("Forest Fire Simulation")
    plt.subplots_adjust(bottom=0.12, top=0.80)

    # Variables to hold slider values and toggle state
    params = {'p': 0.001, 'f': 0.0001, 'grow_while_fire': True}

    def update_p(val):
        params['p'] = val

    def update_f(val):
        params['f'] = val

    def grow_while_fire_callback(event):
        """Toggle grow_while_fire state when button is clicked."""
        params['grow_while_fire'] = not params['grow_while_fire']
        toggle_button.label.set_text('Grow During Fire: ON' if params['grow_while_fire'] else 'Grow During Fire: OFF')
        plt.draw()

    # Toggle button (grow while fire)
    toggle_ax = plt.axes([0.25, 0.93, 0.50, 0.03])
    toggle_button = Button(toggle_ax, 'Grow During Fire: ON', color='lightgreen', hovercolor='green')
    toggle_button.on_clicked(grow_while_fire_callback)

    # p slider (tree growth probability)
    ax_p = plt.axes([0.15, 0.88, 0.25, 0.03])
    slider_p = Slider(ax_p, 'p', 0.0, 0.05, valinit=0.001, valstep=0.0001)
    slider_p.on_changed(update_p)

    # f slider (lightning probability)
    ax_f = plt.axes([0.60, 0.88, 0.25, 0.03])
    slider_f = Slider(ax_f, 'f', 0.0, 0.005, valinit=0.0001, valstep=0.00001)
    slider_f.on_changed(update_f)

    pause_state = {'paused': False}
    exit_state = {'exit': False}

    def pause_callback(event):
        """Toggle pause state when button is clicked."""
        pause_state['paused'] = not pause_state['paused']
        button.label.set_text('Resume' if pause_state['paused'] else 'Pause')
        plt.draw()

    def clear_callback(event):
        """Clear the grid and reset simulation state."""
        global grid, current_layer, fire_active
        grid[:] = 0
        current_layer.clear()
        fire_active = False
        image.set_data(grid)
        plt.draw()

    def exit_callback(event):
        """Exit the simulation when button is clicked."""
        exit_state['exit'] = True
        plt.close()

    # clear button
    clear_button_ax = plt.axes([0.32, 0.02, 0.08, 0.04])
    clear_button = Button(clear_button_ax, 'Clear')
    clear_button.on_clicked(clear_callback)

    # pause button
    button_ax = plt.axes([0.47, 0.02, 0.08, 0.04])
    button = Button(button_ax, 'Pause')
    button.on_clicked(pause_callback)

    # exit button
    exit_button_ax = plt.axes([0.62, 0.02, 0.08, 0.04])
    exit_button = Button(exit_button_ax, 'Exit', hovercolor="red")
    exit_button.on_clicked(exit_callback)

    ani = FuncAnimation(
        fig,
        lambda frame: update(image, pause_state, exit_state, params['p'], params['f'], params['grow_while_fire']),
        interval=1,
        blit=True,
        cache_frame_data=False
    )

    plt.show()


if __name__ == "__main__":
    main()
