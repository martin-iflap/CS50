from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
from numpy.typing import NDArray
import matplotlib.pyplot as plt
from typing import Tuple
import numpy as np
import matplotlib


WIDTH = 225
HEIGHT = 225
N = 400  # number of boids
P = 2  # number of predators
SPEED = 4.0
DRIFT_STRENGTH = 0.9
PERCEPTION_RADIUS = 25
TOO_CLOSE_RADIUS = 15
FEAR_RADIUS = 25
SMOOTHING = 0.55
FOV = 240

COH_WEIGHT = 1.0
DRIFT_WEIGHT = 0.9
SEP_WEIGHT = 2.5
ALIGN_WEIGHT = 1.4
FLEE_WEIGHT = 41000
AVOID_W_WEIGHT = 0.0025 # 0.0025 without predators 0.005 with predators

positions: NDArray = None
velocities: NDArray = None
predator_positions: NDArray = None
predator_velocities: NDArray = None
quiver_boids = None # quivers are matplotlib objects
quiver_predators = None
predator_state: dict = {'active': True}
avoid_walls: dict = {'active': False}

matplotlib.use('TkAgg')  # open a matplotlib window in Pycharm


def rebuild_quiver(ax) -> list:
    """Rebuild the quiver for boids and also call the rebuild_quiver_predators if predators are active
    RETURN: list of quiver objects to be redrawn (boids, predators)
    """
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
    """Rebuild the quiver for predators and return the quiver object"""
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


def init_predators(ax) -> None:
    """Initialize the predators
     - compute random positions and velocities and normalize the speed
    """
    global predator_positions, predator_velocities, quiver_predators

    predator_positions = np.random.uniform(0, [WIDTH, HEIGHT], (P, 2))
    predator_velocities = np.random.uniform(-1, 1, (P, 2))
    p_magnitudes = np.linalg.norm(predator_velocities, axis=1, keepdims=True)
    predator_velocities = (predator_velocities / p_magnitudes) * SPEED


def init_boids(ax) -> list:
    """Initialize the boids
     - compute random positions and velocities and normalize the speed
     - if predators are active, call the init_predators
     - RETURN: the rebuild quiver objects to be redrawn
    """
    global positions, velocities, quiver_boids

    positions = np.random.uniform(0, [WIDTH, HEIGHT], (N, 2))
    velocities = np.random.uniform(-1, 1, (N, 2))
    magnitudes = np.linalg.norm(velocities, axis=1, keepdims=True)
    velocities = (velocities / magnitudes) * SPEED

    if predator_state['active']:
        init_predators(ax)

    return rebuild_quiver(ax)


def apply_drift(posit: NDArray, mask: NDArray) -> NDArray:
    """Apply a small random drift to the velocities to simulate natural movement.
     - calculate random velocities and multiply by DRIFT_STRENGTH
     - filter out the boids that do have neighbors and normalize the steering vectors
     RETURN: adjusted velocity vectors (N, 2)
    """
    has_no_neighbors = (np.sum(mask, axis=1) == 0)

    result = np.zeros_like(posit)

    drift_velocities = np.random.uniform(-1, 1, (len(posit), 2)) * DRIFT_STRENGTH
    magnitudes = np.linalg.norm(drift_velocities, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)

    result[has_no_neighbors] = drift_velocities[has_no_neighbors] / magnitudes[has_no_neighbors]

    return result


def cohesion(posit: NDArray, mask: NDArray) -> NDArray:
    """Calculate the cohesion for all the boids
     - compute the neighbor count and the sum of the neighbor positions for each boid
     - calculate the center of neighbors and the vector towards it
     - filter out the boids that do not have neighbors and normalize the steering vectors
     RETURN: adjusted velocity vectors (N, 2)
    """
    neighbor_count = np.sum(mask, axis=1, keepdims=True)  # (N, 1)
    has_neighbors = (neighbor_count > 0).flatten()  # (N,) boolean

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


def alignment(vel: NDArray, mask: NDArray) -> NDArray:
    """Calculate the alignment velocity for each boid based on the average velocity of its neighbors.
     - compute the neighbor cound and the sum of neighbor velocity vectors for each boid
     - calculate the average velocity of neighbors and adjust the current velocity towards it
     - filter out boids with no neighbors and normalize the steering vectors
     RETURN: adjusted velocity vectors (N, 2)
    """
    neighbor_count = np.sum(mask, axis=1, keepdims=True)  # (N, 1)
    has_neighbors = (neighbor_count > 0).flatten()  # (N,)

    vel_sum = np.sum(
        vel[np.newaxis, :, :] * mask[:, :, np.newaxis],  # (1, N, 2) * (N, N, 1) = (N, N, 2)
        axis=1
    )  # (N, 2)

    avg_vel = np.zeros_like(vel)  # (N, 2)
    avg_vel[has_neighbors] = vel_sum[has_neighbors] / neighbor_count[has_neighbors]  # (N, 2)

    steering = np.zeros_like(vel)
    steering[has_neighbors] = avg_vel[has_neighbors] - vel[has_neighbors]

    magnitudes = np.linalg.norm(steering, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)

    result = np.zeros_like(vel)
    result[has_neighbors] = steering[has_neighbors] / magnitudes[has_neighbors]

    return result


def separation(posit: NDArray, distances: NDArray, diff: NDArray) -> NDArray:
    """Calculate the separation velocity for each boid based on the inverse of the distance to its neighbors.
     - create a separation mask and compute the weighted difference vectors based on the inverse of the distance to neighbors
     - zero out the non-neighbors and sum the weighted differences to get the steering vector for separation
     RETURN: adjusted velocity vectors (N, 2)
    """
    sep_mask = (distances > 0) & (distances < TOO_CLOSE_RADIUS)  # (N, N)

    diff = -diff  # (N, N, 2)

    weights = np.where(sep_mask, distances ** 3, 1.0)  # avoid division by zero
    weighted_diff = diff / weights[:, :, np.newaxis]  # (N, N, 2)
    weighted_diff *= sep_mask[:, :, np.newaxis]  # zero out non-neighbors

    steering = np.sum(weighted_diff, axis=1)  # (N, 2)

    return steering


def steer_from_walls(posit: NDArray) -> NDArray:
    """Compute correction vectors for the boids to avoid walls
     - compute the distances from all 4 walls and combine to compute the final steering vector
     - wall_radius sets the radius at which the boids start turning away from the wall
     - Computation: 1. compute the distance from a wall for each boid
                    2. create a mask for boids within the wall_radius
                    3. calculate the steering vector to push the boids away from the wall based on how close they are
     ARGS: posit: (N, 2) array of boid positions
     RETURN: adjusted velocity vectors (N, 2)
    """
    diff_to_walls = np.zeros_like(posit)
    wall_radius = 40

    # left wall
    dist_left = posit[:, 0]  # (N, 1)
    mask_left = dist_left < wall_radius
    diff_to_walls[mask_left, 0] += (wall_radius - dist_left[mask_left]) ** 2

    # right wall
    dist_right = WIDTH - posit[:, 0]  # (N, 1)
    mask_right = dist_right < wall_radius
    diff_to_walls[mask_right, 0] -= (wall_radius - dist_right[mask_right]) ** 2

    # top wall
    dist_top = HEIGHT - posit[:, 1]  # (N, 1)
    mask_top = dist_top < wall_radius
    diff_to_walls[mask_top, 1] -= (wall_radius - dist_top[mask_top]) ** 2

    # bottom wall
    dist_bottom = posit[:, 1]  # (N, 1)
    mask_bottom = dist_bottom < wall_radius
    diff_to_walls[mask_bottom, 1] += (wall_radius - dist_bottom[mask_bottom]) ** 2

    return diff_to_walls


def flee(posit: NDArray, pr_posit: NDArray) -> Tuple[NDArray, NDArray, NDArray]:
    """Compute the steering vectors for boids to avoid predators based on the inverse of the distance to the predators.
     - compute the differences and distances to predators
     - also create the fear_mask, in_danger mask and min_dist use the latter 2 to calculate speed boosts in move_boids
     - calculate the weighted difference vectors based on the inverse of the distance
       to predators and sum them to get the steering vector for fleeing
     RETURN: adjusted velocity vectors (N, 2), in_danger mask (N,), and min_dist to nearest predator (N,).
    """
    diff_to_predator = posit[:, np.newaxis, :] - pr_posit[np.newaxis, :, :]  # (N, P, 2)
    if not avoid_walls['active']:
        diff_to_predator -= np.round(diff_to_predator / [WIDTH, HEIGHT]) * [WIDTH, HEIGHT]
    dist_to_predator = np.linalg.norm(diff_to_predator, axis=2, keepdims=True)  # (N, P, 1)

    fear_mask = (dist_to_predator > 0) & (dist_to_predator < FEAR_RADIUS)  # (N, P, 1)
    in_danger = np.any(fear_mask[:, :, 0], axis=1)  # (N,) boolean

    min_dist = np.min(np.where(fear_mask[:, :, 0], dist_to_predator[:, :, 0], FEAR_RADIUS), axis=1)

    weights = np.where(fear_mask, dist_to_predator ** 3, 1.0)  # (N, P, 1)
    weighted_diff = (diff_to_predator / weights) * fear_mask  # (N, P, 2) / (N, P, 1) = (N, P, 2)

    steering = np.sum(weighted_diff, axis=1)  # (N, 2)

    return steering, in_danger, min_dist


def move_predators(posit: NDArray, pr_posit: NDArray, pr_vel: NDArray) -> Tuple[NDArray, NDArray]:
    """Move predators to chase the nearest boid.
     - compute the steering vectors to follow prey based on inverse of the distance to the boids
     - also compute the steering vectors to avoid collisions with other predators based on the inverse of the distance to other predators
     - if walls are active compute the wall avoidance vectors with steer_from_walls function to prevent collisions
     - combine the steering vectors with the current velocity to get the target velocity and apply smoothing for better visuals
     - normalize the velocity vectors at the end to SPEED - 0.2
     RETURN: predator positions and velocity arrays (N, 2).
    """
    # compute the steering to follow the prey
    diff_to_boids = posit[np.newaxis, :, :] - pr_posit[:, np.newaxis, :]  # (P, N, 2)
    if not avoid_walls['active']:
        diff_to_boids -= np.round(diff_to_boids / [WIDTH, HEIGHT]) * [WIDTH, HEIGHT]
    dist_to_boids = np.linalg.norm(diff_to_boids, axis=2, keepdims=True)  # (P, N, 1)

    weights = dist_to_boids ** 5  # (P, N, 1)
    weighted_diff = diff_to_boids / weights  # (P, N, 2)

    steering = np.sum(weighted_diff, axis=1)  # (P, 2)
    mag = np.linalg.norm(steering, axis=1, keepdims=True)
    mag = np.maximum(mag, 1e-6)
    steering = (steering / mag) * (SPEED - 0.2)

    # compute the steering to avoid collisions with other predators
    diff_pred_to_pred = pr_posit[:, np.newaxis, :] - pr_posit[np.newaxis, :, :]  # (P, 1, 2) - (P, 1, 2) = (P, P, 2)
    dist_pred_to_pred = np.linalg.norm(diff_pred_to_pred, axis=2)  # (P, P)
    collision_mask = (dist_pred_to_pred > 0) & (dist_pred_to_pred < (TOO_CLOSE_RADIUS + 30))  # (P, P)

    collision_weights = np.where(collision_mask, dist_pred_to_pred ** 3, 1.0)  # (P, P)
    weighted_diff_ptp = (diff_pred_to_pred / collision_weights[:, :, np.newaxis]) * collision_mask[
        :, :, np.newaxis]  # (P, P, 2)

    collision_steering = np.sum(weighted_diff_ptp, axis=1)  # (P, 2)

    # compute the steering to avoid collisions with walls
    walls_steering = np.zeros_like(predator_positions)
    if avoid_walls['active']:
        walls_steering = steer_from_walls(predator_positions)

    # compute final velocity
    target = 1.0 * pr_vel + 1.0 * steering + 30 * collision_steering + AVOID_W_WEIGHT * walls_steering

    magnitudes = np.linalg.norm(target, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-6)
    target = target / magnitudes * (SPEED - 0.2)

    smooth = pr_vel + SMOOTHING * (target - pr_vel)
    smooth_mag = np.linalg.norm(smooth, axis=1, keepdims=True)
    pr_vel = smooth / np.maximum(smooth_mag, 1e-6) * (SPEED - 0.2)

    # Update predator positions and velocities
    pr_posit += pr_vel
    if not avoid_walls['active']:
        pr_posit = np.mod(pr_posit, [WIDTH, HEIGHT])
    else:
        pr_posit = np.clip(pr_posit, [0, 0], [WIDTH, HEIGHT])

    return pr_posit, pr_vel


def move_boids(pending: dict, ax) -> list:
    """Move the boids according to the rules of alignment, cohesion, and separation.
     - Check if there are pending additions or removals of boids and update the positions and velocities arrays accordingly.
     - Compute the difference and distance between all pairs of boids.
     - Calculate the mask for neighbors based on the perception radius and field of view.
     - Calculate the velocity adjustments for each rule and combine them with the current velocity to get the target velocity.
     - Apply smoothing to the velocity changes for better visuals and normalize the final velocity to SPEED.
     - If predators are active, also calculate the flee velocities and apply a speed boost for endangered boids based on their distance to the nearest predator.
     - Update the positions of the boids and wrap around the edges of the screen.
     - If predators are active, also move the predators using the move_predators function.
     - If walls are active call the steer_from_walls function to get the wall avoidance velocities and include them in the final velocity calculation.
     - Update the quiver positions for the boids and predators if active.
     RETURN: the quiver arrays to be redrawn
    """
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
    diff = positions[np.newaxis, :, :] - positions[:, np.newaxis, :]  # (N, N, 2)
    if not avoid_walls['active']:
        diff -= np.round(diff / [WIDTH, HEIGHT]) * [WIDTH, HEIGHT]
    distances = np.linalg.norm(diff, axis=2)  # (N, N)

    # calculate the mask based off of the FOV
    cos_fov = np.cos(np.radians(FOV / 2))
    vel_normalized = velocities / np.linalg.norm(velocities, axis=1, keepdims=True)  # (N, 2)
    diff_normalized = diff / np.maximum(distances[:, :, np.newaxis], 1e-6)  # (N, N, 2)
    dot_products = np.sum(vel_normalized[:, np.newaxis, :] * diff_normalized, axis=2)  # (N, N)
    fov_mask = dot_products > cos_fov  # (N, N)

    mask = (distances > 0) & (distances < PERCEPTION_RADIUS) & fov_mask  # (N, N)

    # calculate velocities for all boids based on the rules
    drift_velocities = apply_drift(positions, mask)
    cohesion_velocities = cohesion(positions, mask)
    alignment_velocities = alignment(velocities, mask)
    separation_velocities = separation(positions, distances, diff)

    if avoid_walls['active']:
        avoid_w_velocities = steer_from_walls(positions)
    else:
        avoid_w_velocities = np.zeros_like(positions)

    if predator_state['active'] and predator_positions is not None:
        flee_velocities, in_danger, min_dist = flee(positions, predator_positions)
    else:
        flee_velocities = np.zeros_like(positions)
        in_danger = np.zeros(len(positions), dtype=bool)
        min_dist = np.full(len(positions), FEAR_RADIUS)

    # compute and normalize the final velocity
    final_velocity = (velocities
                      + COH_WEIGHT * cohesion_velocities
                      + DRIFT_WEIGHT * drift_velocities
                      + ALIGN_WEIGHT * alignment_velocities
                      + SEP_WEIGHT * separation_velocities
                      + FLEE_WEIGHT * flee_velocities
                      + AVOID_W_WEIGHT * avoid_w_velocities
                      )
    magnitudes = np.linalg.norm(final_velocity, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-5)
    target = (final_velocity / magnitudes) * SPEED

    # add smoothing
    smooth_velocities = velocities + SMOOTHING * (target - velocities)
    magnitudes = np.linalg.norm(smooth_velocities, axis=1, keepdims=True)
    magnitudes = np.maximum(magnitudes, 1e-5)

    velocities = (smooth_velocities / magnitudes) * SPEED

    # apply speed up for endangered boids
    boost_factor = 1.0 + 0.6 * (1.0 - min_dist / FEAR_RADIUS) * in_danger
    velocities = velocities * boost_factor[:, np.newaxis]

    positions += velocities
    if not avoid_walls['active']:
        positions = np.mod(positions, [WIDTH, HEIGHT])
    else:
        positions = np.clip(positions, [0, 0], [WIDTH, HEIGHT])

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


def update(pause_state: dict, exit_state: dict, pending: dict, ax) -> list:
    """Update function for the animation. This function is called by FuncAnimation.
     - check for exit state and exit if True
    """
    global quiver_boids, quiver_predators

    if exit_state['exit']:
        plt.close()
        quivers = [quiver_boids]
        if predator_state['active'] and quiver_predators is not None:
            quivers.append(quiver_predators)
        return quivers

    return move_boids(pending, ax)


def main() -> None:
    """The main function to run the simulation and construct the FuncAnimation
     - create the plot and set up the buttons for pause, reset, exit, add/remove boids and toggle predators and walls
     - the button callbacks will update the respective states and call the necessary functions to update the simulation
     - start the animation with FuncAnimation and show the plot
     - the animation will call the update function at each frame, which will move the boids and update the quiver positions accordingly
     - the simulation can be paused, reset, or exited using the buttons, and boids can be added or removed dynamically
     - the predators and walls can also be toggled on or off
    """
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
        """Enable or disable the predators
         - adjust the AVOID_W_WEIGHT based on the predator_state to work with walls nicely
        """
        global predator_positions, predator_velocities, quiver_predators, AVOID_W_WEIGHT

        predator_state['active'] = not predator_state['active']
        if predator_state['active']:
            predators_button.label.set_text('Predators: ON')
            init_predators(axes)
            rebuild_quiver_predators(axes)
            if avoid_walls['active']:
                AVOID_W_WEIGHT = 0.005
        else:
            predators_button.label.set_text('Predators: OFF')
            if quiver_predators is not None:
                quiver_predators.remove()
                quiver_predators = None
            predator_positions = None
            predator_velocities = None
            if avoid_walls['active']:
                AVOID_W_WEIGHT = 0.0025
        fig.canvas.draw()

    def walls_callback(event):
        """Enable and disable the walls
         - when walls and predators are active increase the AVOID_W_WEIGHT
         - allways decrease SPEED when walls are active for better visuals
        """
        global SPEED, AVOID_W_WEIGHT
        avoid_walls['active'] = not avoid_walls['active']

        if avoid_walls['active']:
            walls_button.label.set_text('Walls: ON')
            global positions, predator_positions
            positions = np.mod(positions, [WIDTH, HEIGHT])
            if predator_positions is not None:
                predator_positions = np.mod(predator_positions, [WIDTH, HEIGHT])
                AVOID_W_WEIGHT = 0.005
            else:
                AVOID_W_WEIGHT = 0.0025

            SPEED = 3
        else:
            walls_button.label.set_text('Walls: OFF')
            SPEED = 4

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
    add_button_ax = plt.axes([0.20, 0.88, 0.15, 0.05])
    add_button = Button(add_button_ax, 'Add (+10)', hovercolor="green")
    add_button.on_clicked(add_callback)

    # remove button
    remove_button_ax = plt.axes([0.35, 0.88, 0.15, 0.05])
    remove_button = Button(remove_button_ax, 'Remove (-10)', hovercolor="red")
    remove_button.on_clicked(remove_callback)

    # predators button
    predators_button_ax = plt.axes([0.50, 0.88, 0.15, 0.05])
    predators_button = Button(predators_button_ax, 'Predators: ON', hovercolor="orange")
    predators_button.on_clicked(predators_callback)

    # walls button
    walls_button_ax = plt.axes([0.65, 0.88, 0.15, 0.05])
    walls_button = Button(walls_button_ax, 'Walls: OFF', hovercolor="yellow")
    walls_button.on_clicked(walls_callback)


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
