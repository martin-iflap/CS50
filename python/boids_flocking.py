from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
import matplotlib.pyplot as plt
import numpy as np
import matplotlib



WIDTH = 225
HEIGHT = 225
N = 400 # number of boids
SPEED = 4.0
DRIFT_STRENGTH = 0.9
PERCEPTION_RADIUS = 25
TOO_CLOSE_RADIUS = 15
SMOOTHING = 0.55
FOV = 240

COH_WEIGHT = 1.0     # 1.0
DRIFT_WEIGHT = 0.9   # 0.3
SEP_WEIGHT = 2.5     # 2.5
ALIGN_WEIGHT = 1.4   # 1.4

positions = None
velocities = None
quiver = None


matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


def rebuild_quiver(ax):
    """Rebuild the quiver"""
    global positions, velocities, quiver
    if quiver is not None:
        quiver.remove()

    quiver = ax.quiver(
        positions[:, 0], positions[:, 1],
        velocities[:, 0], velocities[:, 1],
        color='white',
        scale=225,
        scale_units='width',
        width=0.002,
        headwidth=3,
        headlength=5,
        headaxislength=3,
        minlength=0.05,
        minshaft=0.15,
    )

    return [quiver]


def init_boids(ax):
    """Initialize the boids objects"""
    global positions, velocities, quiver

    positions = np.random.uniform(0, [WIDTH, HEIGHT], (N, 2))
    velocities = np.random.uniform(-1, 1, (N, 2))
    magnitudes = np.linalg.norm(velocities, axis=1, keepdims=True)
    velocities = (velocities / magnitudes) * SPEED

    return rebuild_quiver(ax)


def apply_drift(posit, mask):
    """Apply a small random drift to the velocities to simulate natural movement."""
    has_no_neighbors = (np.sum(mask, axis=1) == 0)

    result = np.zeros_like(posit)

    drift_velocities = np.random.uniform(-1, 1, (len(posit), 2)) * DRIFT_STRENGTH
    magnitudes = np.linalg.norm(drift_velocities, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)

    result[has_no_neighbors] = drift_velocities[has_no_neighbors] / magnitudes[has_no_neighbors]

    return result


def cohesion(posit, mask):
    neighbor_count = np.sum(mask, axis=1, keepdims=True)  # (N, 1)
    has_neighbors = (neighbor_count > 0).flatten()        # (N,) boolean

    neighbor_sum = np.sum(
        posit[np.newaxis, :, :] * mask[:, :, np.newaxis],
        axis=1
    )  # (N, 2)

    center = np.zeros_like(posit)
    center[has_neighbors] = neighbor_sum[has_neighbors] / neighbor_count[has_neighbors]

    steering = np.zeros_like(posit)
    steering[has_neighbors] = center[has_neighbors] - posit[has_neighbors]

    magnitudes = np.linalg.norm(steering, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)

    result = np.zeros_like(posit)
    result[has_neighbors] = steering[has_neighbors] / magnitudes[has_neighbors]

    return result


def alignment(vel, mask):
    """Calculate the alignment velocity for each boid based on the average velocity of its neighbors."""
    neighbor_count = np.sum(mask, axis=1, keepdims=True) # (N, 1)
    has_neighbors = (neighbor_count > 0).flatten() # (N,)

    vel_sum = np.sum(
        vel[np.newaxis, :, :] * mask[:, :, np.newaxis], # (1, N, 2) * (N, N, 1) = (N, N, 2)
        axis=1
    ) # (N, 2)

    avg_vel = np.zeros_like(vel) # (N, 2)
    avg_vel[has_neighbors] = vel_sum[has_neighbors] / neighbor_count[has_neighbors] # (N, 2)

    steering = np.zeros_like(vel)
    steering[has_neighbors] = avg_vel[has_neighbors] - vel[has_neighbors]

    magnitudes = np.linalg.norm(steering, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)

    result = np.zeros_like(vel)
    result[has_neighbors] = steering[has_neighbors] / magnitudes[has_neighbors]

    return result


def separation(posit, distances):
    """Calculate the separation velocity for each boid based on the inverse of the distance to its neighbors."""
    sep_mask = (distances > 0) & (distances < TOO_CLOSE_RADIUS)  # (N, N)

    diff = posit[:, np.newaxis, :] - posit[np.newaxis, :, :]  # (N, N, 2)

    weights = np.where(sep_mask, distances ** 3, 1.0)  # avoid division by zero
    weighted_diff = diff / weights[:, :, np.newaxis]   # (N, N, 2)
    weighted_diff *= sep_mask[:, :, np.newaxis]        # zero out non-neighbors

    steering = np.sum(weighted_diff, axis=1)           # (N, 2)
    return steering


def move_boids(pending: dict, ax):
    """Move the boids according to the rules of alignment, cohesion, and separation."""
    global positions, velocities, quiver

    resized = False

    # add new boids
    if pending['add'] > 0:
        num = pending['add']
        pending['add'] = 0
        new_pos = np.random.uniform(0, [WIDTH, HEIGHT], (num, 2))
        new_vel = np.random.uniform(-1, 1, (num, 2))
        new_vel = (new_vel / np.linalg.norm(new_vel, axis=1, keepdims=True)) * SPEED
        positions = np.vstack((positions, new_pos))
        velocities = np.vstack((velocities, new_vel))
        resized = True

    # remove boids
    if pending['remove'] > 0:
        actual = min(pending['remove'], len(positions))
        pending['remove'] = 0
        positions = np.delete(positions, np.arange(actual), axis=0)
        velocities = np.delete(velocities, np.arange(actual), axis=0)
        resized = True

    diff = positions[np.newaxis, :, :] - positions[:, np.newaxis, :] # (N, N, 2)
    distances = np.linalg.norm(diff, axis=2) # (N, N)

    cos_fov = np.cos(np.radians(FOV / 2))
    vel_normalized = velocities / np.linalg.norm(velocities, axis=1, keepdims=True) # (N, 2)
    diff_normalized = diff / np.maximum(distances[:, :, np.newaxis], 1e-6) # (N, N, 2)
    dot_products = np.sum(vel_normalized[:, np.newaxis, :] * diff_normalized, axis=2) # (N, N)
    fov_mask = dot_products > cos_fov # (N, N)

    mask = (distances > 0) & (distances < PERCEPTION_RADIUS) & fov_mask # (N, N)

    drift_velocities = apply_drift(positions, mask)
    cohesion_velocities = cohesion(positions, mask)
    alignment_velocities = alignment(velocities, mask)
    separation_velocities = separation(positions, distances)

    final_velocity = (velocities + COH_WEIGHT * cohesion_velocities + DRIFT_WEIGHT * drift_velocities + ALIGN_WEIGHT * alignment_velocities + SEP_WEIGHT * separation_velocities)
    magnitudes = np.linalg.norm(final_velocity, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-5)
    target = (final_velocity / magnitudes) * SPEED

    smooth_velocities = velocities + SMOOTHING * (target - velocities)
    magnitudes = np.linalg.norm(smooth_velocities, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-5)

    velocities = (smooth_velocities / magnitudes) * SPEED

    positions += velocities
    positions = np.mod(positions, [WIDTH, HEIGHT])

    if resized:
        rebuild_quiver(ax)
    else:
        quiver.set_offsets(positions)
        quiver.set_UVC(velocities[:, 0], velocities[:, 1])

    return [quiver]


def update(pause_state: dict, exit_state: dict, pending: dict, ax):
    """Main function to update the animation"""
    global quiver

    if exit_state['exit']:
        plt.close()
        return [quiver]

    return move_boids(pending, ax)


def main():
    """The main function to run the simulation"""
    fig, axes = plt.subplots(1, 1, figsize=(8, 8))

    axes.set_xlim(0, WIDTH)
    axes.set_ylim(0, HEIGHT)
    axes.set_facecolor('black')

    pause_state = {'paused': False}
    exit_state = {'exit': False}
    ani_state = {'ani': None}
    pending = {'add': 0, 'remove': 0}

    def pause_callback(event):
        """Toggle pause state when button is clicked."""
        pause_state['paused'] = not pause_state['paused']
        if pause_state['paused']:
            button.label.set_text('Resume')
            ani_state['ani'].pause()
        else:
            button.label.set_text('Pause')
            ani_state['ani'].resume()
        fig.canvas.draw()

    def reset_callback(event):
        """Reset the simulation"""
        global positions, velocities, quiver
        positions, velocities = [], []
        init_boids(axes)

    def exit_callback(event):
        """Exit the simulation when button is clicked."""
        exit_state['exit'] = True
        plt.close()

    def add_callback(event, num: int = 10):
        """Add boids to the simulation"""
        pending['add'] += num

    def remove_callback(event, num: int = 10):
        """Remove boids from the simulation"""
        pending['remove'] += num

    # reset button
    reset_button_ax = plt.axes([0.32, 0.02, 0.08, 0.04])
    reset_button = Button(reset_button_ax, 'Reset')
    reset_button.on_clicked(reset_callback)

    # pause button
    button_ax = plt.axes([0.47, 0.02, 0.08, 0.04])
    button = Button(button_ax, 'Pause')
    button.on_clicked(pause_callback)

    # exit button
    exit_button_ax = plt.axes([0.62, 0.02, 0.08, 0.04])
    exit_button = Button(exit_button_ax, 'Exit', hovercolor="red")
    exit_button.on_clicked(exit_callback)

    # add button
    add_button_ax = plt.axes([0.30, 0.88, 0.2, 0.05])
    add_button = Button(add_button_ax, 'Add (+10)', hovercolor="green")
    add_button.on_clicked(add_callback)

    # remove button
    remove_button_ax = plt.axes([0.50, 0.88, 0.2, 0.05])
    remove_button = Button(remove_button_ax, 'Remove (-10)', hovercolor="red")
    remove_button.on_clicked(remove_callback)

    ani = FuncAnimation(
        fig,
        lambda frame: update(pause_state, exit_state, pending, axes),
        init_func=lambda: init_boids(axes),
        interval=1,
        blit=True,
        cache_frame_data=False
    )
    ani_state['ani'] = ani

    plt.show()

if __name__ == "__main__":
    main()
