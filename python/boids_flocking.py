from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
import matplotlib.pyplot as plt
import numpy as np
import matplotlib

WIDTH = 300
HEIGHT = 300
N = 40 # number of boids
SPEED = 5.0
DRIFT_STRENGTH = 0.1
PERCEPTION_RADIUS = 80
TOO_CLOSE_RADIUS = 30

COH_WEIGHT = 1.0
DRIFT_WEIGHT = 1.0
SEP_WEIGHT = 20.0
ALIGN_WEIGHT = 1.0

positions = None
velocities = None
quiver = None


matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


def init_boids(ax):
    """Initialize the boids objects"""
    global positions, velocities, quiver

    positions = np.random.uniform(0, [WIDTH, HEIGHT], (N, 2))
    velocities = np.random.uniform(-1, 1, (N, 2))
    magnitudes = np.linalg.norm(velocities, axis=1, keepdims=True)
    velocities = (velocities / magnitudes) * SPEED

    quiver = ax.quiver(positions[:, 0], positions[:, 1], velocities[:, 0], velocities[:, 1], color='white')
    return [quiver]


def apply_drift(posit, mask):
    """Apply a small random drift to the velocities to simulate natural movement."""
    len_posit = len(posit)
    steering = np.zeros((len_posit, 2))

    for i in range(len_posit):
        neighbors = posit[mask[i]]
        if len(neighbors) == 0:
            drift_velocity = np.random.uniform(-1, 1, 2) * DRIFT_STRENGTH
            magnitude = np.linalg.norm(drift_velocity)
            steering[i] += drift_velocity / magnitude
    return steering


def cohesion(posit, mask):
    """Calculate the cohesion steering vector for each boid based on its neighbors."""
    len_posit = len(posit)
    steering = np.zeros((len_posit, 2))

    for i in range(len_posit):
        neighbors = posit[mask[i]]
        if len(neighbors) > 0:
            center = np.mean(neighbors, axis=0)
            center_velocity = center - posit[i]
            magnitude = np.linalg.norm(center_velocity)
            steering[i] += center_velocity / magnitude
    return steering


def alignment(vel, mask):
    """Calculate the alignment velocity for each boid based on the average velocity of its neighbors."""
    len_vel = len(vel)
    steering = np.zeros((len_vel, 2))

    for i in range(len_vel):
        neighbor_velocities = vel[mask[i]]
        if len(neighbor_velocities) > 0:
            bunch = np.mean(neighbor_velocities, axis=0)
            bunch_velocity = bunch - vel[i]
            magnitude = np.linalg.norm(bunch_velocity)
            steering[i] += bunch_velocity / magnitude
    return steering


def separation(posit, distances):
    """Calculate the separation steering vector for each boid to avoid crowding."""
    len_posit = len(posit)
    steering = np.zeros((len_posit, 2))

    mask = (distances > 0) & (distances < TOO_CLOSE_RADIUS)

    for i in range(len_posit):
        neighbors = posit[mask[i]]
        neighbor_distances = distances[i][mask[i]]
        if len(neighbors) > 0:
            diff = posit[i] - neighbors
            weights = (neighbor_distances ** 3)[:, np.newaxis]
            steering[i] = np.sum(diff / weights, axis=0)
    return steering


def move_boids():
    """Move the boids according to the rules of alignment, cohesion, and separation."""
    global positions, velocities, quiver

    diff = positions[np.newaxis, :, :] - positions[:, np.newaxis, :]
    distances = np.linalg.norm(diff, axis=2)
    mask = (distances > 0) & (distances < PERCEPTION_RADIUS)

    drift_velocities = apply_drift(positions, mask)
    cohesion_velocities = cohesion(positions, mask)
    alignment_velocities = alignment(velocities, mask)
    separation_velocities = separation(positions, distances)

    final_velocity = (velocities + COH_WEIGHT * cohesion_velocities + DRIFT_WEIGHT * drift_velocities + ALIGN_WEIGHT * alignment_velocities + SEP_WEIGHT * separation_velocities)
    magnitude = np.linalg.norm(final_velocity, axis=1, keepdims=True)
    velocities = (final_velocity / magnitude) * SPEED
    positions += velocities
    positions = np.mod(positions, [WIDTH, HEIGHT])

    quiver.set_offsets(positions)
    quiver.set_UVC(velocities[:, 0], velocities[:, 1])

    return [quiver]


def update(pause_state: dict, exit_state: dict):
    """Main function to update the animation"""
    global quiver

    if pause_state['paused']:
        plt.pause(0.1)
        return [quiver]

    if exit_state['exit']:
        plt.close()
        return [quiver]

    return move_boids()


def main():
    """The main function to run the simulation"""
    fig, axes = plt.subplots(1, 1, figsize=(7, 7))

    axes.set_xlim(0, WIDTH)
    axes.set_ylim(0, HEIGHT)
    axes.set_facecolor('black')

    pause_state = {'paused': False}
    exit_state = {'exit': False}

    def pause_callback(event):
        """Toggle pause state when button is clicked."""
        pause_state['paused'] = not pause_state['paused']
        button.label.set_text('Resume' if pause_state['paused'] else 'Pause')
        plt.draw()

    def reset_callback(event):
        """Reset the simulation"""
        pass

    def exit_callback(event):
        """Exit the simulation when button is clicked."""
        exit_state['exit'] = True
        plt.close()

    # reset button
    reset_button_ax = plt.axes([0.32, 0.02, 0.08, 0.04])
    reset_button = Button(reset_button_ax, 'Clear')
    reset_button.on_clicked(reset_callback)

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
        lambda frame: update(pause_state, exit_state),
        init_func=lambda: init_boids(axes),
        interval=10,
        blit=True,
        cache_frame_data=False
    )

    plt.show()

if __name__ == "__main__":
    main()
