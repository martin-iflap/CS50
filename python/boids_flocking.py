from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
import matplotlib.pyplot as plt
import numpy as np
import matplotlib



WIDTH = 225
HEIGHT = 225
N = 400 # number of boids
P = 2 # number of predators
SPEED = 4.0
DRIFT_STRENGTH = 0.9
PERCEPTION_RADIUS = 25
TOO_CLOSE_RADIUS = 15
FEAR_RADIUS = 25
SMOOTHING = 0.55
FOV = 240

COH_WEIGHT = 1.0     # 1.0
DRIFT_WEIGHT = 0.9   # 0.9
SEP_WEIGHT = 2.5     # 2.5
ALIGN_WEIGHT = 1.4   # 1.4
FLEE_WEIGHT = 41000

positions = None
velocities = None
predator_positions = None
predator_velocities = None
quiver_boids = None
quiver_predators = None
predator_state = {'active': True}


matplotlib.use('TkAgg') # open a matplotlib window in Pycharm


def rebuild_quiver(ax):
    """Rebuild the quiver"""
    global positions, velocities, quiver_boids

    if quiver_boids is not None:
        quiver_boids.remove()

    quiver_boids = ax.quiver(
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

    quivers = [quiver_boids]

    if predator_state['active']:
        quivers.append(rebuild_quiver_predators(ax))

    return quivers


def rebuild_quiver_predators(ax):
    """Rebuild the quiver for predators"""
    global predator_positions, predator_velocities, quiver_predators

    if quiver_predators is not None:
        quiver_predators.remove()

    quiver_predators = ax.quiver(
        predator_positions[:, 0], predator_positions[:, 1],
        predator_velocities[:, 0], predator_velocities[:, 1],
        color='red',
        scale=180,
        scale_units='width',
        width=0.005,
        headwidth=3.2,
        headlength=5.5,
        headaxislength=3.5,
        minlength=0.08,
        minshaft=0.20,
    )
    return quiver_predators


def init_predators(ax):
    """Initialize the predators objects"""
    global predator_positions, predator_velocities, quiver_predators

    predator_positions = np.random.uniform(0, [WIDTH, HEIGHT], (P, 2))
    predator_velocities = np.random.uniform(-1, 1, (P, 2))
    p_magnitudes = np.linalg.norm(predator_velocities, axis=1, keepdims=True)
    predator_velocities = (predator_velocities / p_magnitudes) * SPEED


def init_boids(ax):
    """Initialize the boids objects"""
    global positions, velocities, quiver_boids

    positions = np.random.uniform(0, [WIDTH, HEIGHT], (N, 2))
    velocities = np.random.uniform(-1, 1, (N, 2))
    magnitudes = np.linalg.norm(velocities, axis=1, keepdims=True)
    velocities = (velocities / magnitudes) * SPEED
    
    if predator_state['active']:
        init_predators(ax)

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


def separation(posit, distances, diff):
    """Calculate the separation velocity for each boid based on the inverse of the distance to its neighbors."""
    sep_mask = (distances > 0) & (distances < TOO_CLOSE_RADIUS)  # (N, N)

    diff = -diff # (N, N, 2)

    weights = np.where(sep_mask, distances ** 3, 1.0)  # avoid division by zero
    weighted_diff = diff / weights[:, :, np.newaxis]   # (N, N, 2)
    weighted_diff *= sep_mask[:, :, np.newaxis]        # zero out non-neighbors

    steering = np.sum(weighted_diff, axis=1)           # (N, 2)

    return steering


def flee(posit, pr_posit):
    """Compute the steering vectors for boids to avoid predators based on the inverse of the distance to the predators."""
    diff_to_predator = posit[:, np.newaxis, :] - pr_posit[np.newaxis, :, :] # (N, P, 2)
    dist_to_predator = np.linalg.norm(diff_to_predator, axis=2, keepdims=True) # (N, P, 1)

    fear_mask = (dist_to_predator > 0) & (dist_to_predator < FEAR_RADIUS) # (N, P, 1)

    weights = np.where(fear_mask, dist_to_predator ** 3, 1.0) # (N, P, 1)
    weighted_diff = (diff_to_predator / weights) * fear_mask # (N, P, 2) / (N, P, 1) = (N, P, 2)

    steering = np.sum(weighted_diff, axis=1) # (N, 2)

    return steering


def move_predators(posit, pr_posit, pr_vel):
    """Move predators to chase the nearest boid."""
    # compute the steering to follow the prey
    diff_to_boids = posit[np.newaxis, :, :] - pr_posit[:, np.newaxis, :]  # (P, N, 2)
    dist_to_boids = np.linalg.norm(diff_to_boids, axis=2, keepdims=True)  # (P, N, 1)

    weights = dist_to_boids ** 5 # (P, N, 1)
    weighted_diff = diff_to_boids / weights # (P, N, 2)

    steering = np.sum(weighted_diff, axis=1) # (P, 2)
    mag = np.linalg.norm(steering, axis=1, keepdims=True)
    mag = np.maximum(mag, 1e-6)
    steering = (steering / mag) * (SPEED - 0.2)

    # compute the steering to avoid collisions with other predators
    diff_pred_to_pred = pr_posit[:, np.newaxis, :] - pr_posit[np.newaxis, :, :] # (P, 1, 2) - (P, 1, 2) = (P, P, 2)
    dist_pred_to_pred = np.linalg.norm(diff_pred_to_pred, axis=2) # (P, P)
    collision_mask = (dist_pred_to_pred > 0) & (dist_pred_to_pred < (TOO_CLOSE_RADIUS + 30)) # (P, P)

    collision_weights = np.where(collision_mask, dist_pred_to_pred ** 3, 1.0) # (P, P)
    weighted_diff_ptp = (diff_pred_to_pred / collision_weights[:, :, np.newaxis]) * collision_mask[:, :, np.newaxis] # (P, P, 2)

    collision_steering = np.sum(weighted_diff_ptp, axis=1) # (P, 2)

    # compute final velocity
    target = 1.0 * pr_vel + 1.0 * steering + 30 * collision_steering

    magnitudes = np.linalg.norm(target, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)
    target = target / magnitudes * (SPEED - 0.2)

    smooth = pr_vel + SMOOTHING * (target - pr_vel)
    smooth_mag = np.linalg.norm(smooth, axis=1, keepdims=True)
    pr_vel = smooth / np.maximum(smooth_mag, 1e-6) * (SPEED - 0.2)

    # Update predator positions and velocities
    pr_posit += pr_vel
    pr_posit = np.mod(pr_posit, [WIDTH, HEIGHT])

    return pr_posit, pr_vel


def move_boids(pending: dict, ax):
    """Move the boids according to the rules of alignment, cohesion, and separation."""
    global positions, velocities, predator_positions, predator_velocities, quiver_boids, quiver_predators

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

    # compute the difference and distance between all pairs of boids
    diff = positions[np.newaxis, :, :] - positions[:, np.newaxis, :] # (N, N, 2)
    distances = np.linalg.norm(diff, axis=2) # (N, N)

    # calculate the mask based off of the FOV
    cos_fov = np.cos(np.radians(FOV / 2))
    vel_normalized = velocities / np.linalg.norm(velocities, axis=1, keepdims=True) # (N, 2)
    diff_normalized = diff / np.maximum(distances[:, :, np.newaxis], 1e-6) # (N, N, 2)
    dot_products = np.sum(vel_normalized[:, np.newaxis, :] * diff_normalized, axis=2) # (N, N)
    fov_mask = dot_products > cos_fov # (N, N)

    mask = (distances > 0) & (distances < PERCEPTION_RADIUS) & fov_mask # (N, N)

    # calculate velocities for all boids based on the rules
    drift_velocities = apply_drift(positions, mask)
    cohesion_velocities = cohesion(positions, mask)
    alignment_velocities = alignment(velocities, mask)
    separation_velocities = separation(positions, distances, diff)
    flee_velocities = (flee(positions, predator_positions)
                       if predator_state['active'] and predator_positions is not None
                       else np.zeros_like(positions))

    # compute and normalize the final velocity
    final_velocity = (velocities
                      + COH_WEIGHT * cohesion_velocities
                      + DRIFT_WEIGHT * drift_velocities
                      + ALIGN_WEIGHT * alignment_velocities
                      + SEP_WEIGHT * separation_velocities
                      + FLEE_WEIGHT * flee_velocities
                      )
    magnitudes = np.linalg.norm(final_velocity, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-5)
    target = (final_velocity / magnitudes) * SPEED

    # add smoothing
    smooth_velocities = velocities + SMOOTHING * (target - velocities)
    magnitudes = np.linalg.norm(smooth_velocities, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-5)

    velocities = (smooth_velocities / magnitudes) * SPEED

    positions += velocities
    positions = np.mod(positions, [WIDTH, HEIGHT])

    # Move predators
    if predator_state['active']:
        predator_positions, predator_velocities = move_predators(positions, predator_positions, predator_velocities)

    # update the quiver positions
    if resized:
        return rebuild_quiver(ax)
    else:
        quiver_boids.set_offsets(positions)
        quiver_boids.set_UVC(velocities[:, 0], velocities[:, 1])

        quivers = [quiver_boids]
        if predator_state['active']:
            quiver_predators.set_offsets(predator_positions)
            quiver_predators.set_UVC(predator_velocities[:, 0], predator_velocities[:, 1])
            quivers.append(quiver_predators)

        return quivers


def update(pause_state: dict, exit_state: dict, pending: dict, ax):
    """Main function to update the animation"""
    global quiver_boids, quiver_predators

    if exit_state['exit']:
        plt.close()
        quivers = [quiver_boids]
        if predator_state['active'] and quiver_predators is not None:
            quivers.append(quiver_predators)
        return quivers

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
        global positions, velocities, predator_positions, predator_velocities, quiver_boids, quiver_predators
        positions, velocities = [], []
        if predator_state['active']:
            predator_positions, predator_velocities = [], []
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

    def predators_callback(event):
        """Enable or disable the predators"""
        global predator_positions, predator_velocities, quiver_predators

        predator_state['active'] = not predator_state['active']
        if predator_state['active']:
            predators_button.label.set_text('Predators: ON')
            init_predators(axes)
            rebuild_quiver_predators(axes)
        else:
            predators_button.label.set_text('Predators: OFF')
            if quiver_predators is not None:
                quiver_predators.remove()
                quiver_predators = None
            predator_positions = None
            predator_velocities = None
        fig.canvas.draw()

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
    add_button_ax = plt.axes([0.20, 0.88, 0.2, 0.05])
    add_button = Button(add_button_ax, 'Add (+10)', hovercolor="green")
    add_button.on_clicked(add_callback)

    # remove button
    remove_button_ax = plt.axes([0.40, 0.88, 0.2, 0.05])
    remove_button = Button(remove_button_ax, 'Remove (-10)', hovercolor="red")
    remove_button.on_clicked(remove_callback)

    predators_button_ax = plt.axes([0.60, 0.88, 0.2, 0.05])
    predators_button = Button(predators_button_ax, 'Predators: ON', hovercolor="orange")
    predators_button.on_clicked(predators_callback)

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
