"""Demo for stepwise / live processing using LiveCXRunner.

This illustrates how to feed recorded (or streaming) polarized-sensor data and
flow vectors into the CX model one timestep at a time. For a real experiment
you would replace the demo data source with the actual acquisition code that
yields a (sensor_data, flow_vec, dt) tuple each timestep.
"""
from datetime import datetime, timezone
import numpy as np
import os

import trials
import polarization
import plotter
import matplotlib.pyplot as plt


def main(input_file='data/live_input.npz'):
    # Try to load an NPZ containing 'sensor_data_seq' (T,N,2) and 'flow_seq' (T,2)
    if os.path.exists(input_file):
        print('Loading live inputs from', input_file)
        with np.load(input_file) as data:
            sensor_data_seq = data['sensor_data_seq']
            flow_seq = data['flow_seq']
            start_dt = data.get('start_dt', None)
            if start_dt is not None:
                # saved as strings sometimes
                start_dt = np.datetime64(start_dt).astype('datetime64[s]').tolist()
    else:
        # Fallback: generate a small synthetic dataset (as example only)
        print('No input file found — creating small synthetic dataset for demonstration')
        T = 50
        lat = 51.5
        lon = -0.1
        start_dt = datetime.now(timezone.utc)

        # Create a short route and velocities
        true_h, v = trials.generate_route(T=T, mean_acc=0.15, vary_speed=False)

        # sensor layout: 8 sensors
        sensor_angles = np.linspace(0, 360, 8, endpoint=False)

        sensor_data_seq = np.zeros((T, len(sensor_angles), 2))
        for t in range(T):
            dt = start_dt + np.timedelta64(t, 's').astype(object)
            sun_world = polarization.sun_azimuth(lat, lon, dt)
            sensor_data_seq[t] = polarization.simulate_polar_sensors(true_heading=true_h[t],
                                                                     sun_azimuth_world=sun_world,
                                                                     sensor_angles_deg=sensor_angles,
                                                                     noise_sigma=0.02)

        flow_seq = v.copy()

    # Create the runner — it will maintain internal state between steps
    runner = trials.LiveCXRunner(cx=None, logging=True, lat=51.5, lon=-0.1,
                                  start_dt=start_dt, sensor_angles_deg=np.linspace(0, 360, 8, endpoint=False),
                                  sensor_noise=0.02, motor_hold=1)

    T = sensor_data_seq.shape[0]
    motors = np.zeros(T)
    measured_headings = np.zeros(T)

    for t in range(T):
        dt = start_dt + np.timedelta64(t, 's').astype(object)
        sd = sensor_data_seq[t]
        flow = flow_seq[t]
        mh, motor, outputs = runner.step(sd, flow, dt=dt)
        measured_headings[t] = mh
        motors[t] = motor

        # In a real experiment: use outputs['rotation_to_apply'] to command a robot
        print(f'step {t}: measured_h={mh:.3f}, motor={motor:.4f}')

    # Optional: save a run summary for offline analysis
    os.makedirs('data', exist_ok=True)
    np.savez_compressed('data/live_run_result.npz', measured_headings=measured_headings, motors=motors)
    print('Saved data/live_run_result.npz')

    # --- Plot trajectory from flow_seq and measured headings ---
    # positions = cumsum of flow vectors, start at origin
    positions = np.vstack([np.array([0.0, 0.0]), np.cumsum(flow_seq, axis=0)])
    # measured_headings corresponds to T steps; for plotting we pass measured_headings
    try:
        fig, ax = plotter.plot_route(measured_headings, flow_seq, T_outbound=T, T_inbound=0,
                                     plot_heading=False, title='Live-run trajectory')
        plotter.save_plot(fig, 'live_run_trajectory')
        print('Saved plots/live_run_trajectory.pdf')
    except Exception as e:
        print('Plotting failed:', e)


if __name__ == '__main__':
    main()
