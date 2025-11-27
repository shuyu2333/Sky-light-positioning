"""Example script showing how to use polarized-sky sensor module with CX model.

This script does NOT modify existing trials functions. It shows a simple
closed-loop run where TL layer receives a heading estimated from simulated
polarized sensors (and the global sun position calculated from lat/lon/time).

Run from project root:
    python run_polar_example.py
"""
from datetime import datetime, timedelta, timezone
import numpy as np

import trials
import cx_rate
import polarization


def run_example():
    T = 200
    # location (example): latitude, longitude in degrees
    lat = 51.5
    lon = -0.1
    start_dt = datetime.now(timezone.utc)

    # generate a short outbound route
    h_true, v = trials.generate_route(T=T, mean_acc=0.1, vary_speed=False)

    cx = cx_rate.CXRatePontin()

    # sensor layout: 8 sensors default
    sensor_angles = np.arange(0, 360, 45)

    tb1 = np.zeros(trials.central_complex.N_TB1)
    memory = 0.5 * np.ones(trials.central_complex.N_CPU4)

    measured_headings = np.zeros(T)
    true_headings = h_true.copy()

    for t in range(T):
        dt = start_dt + timedelta(seconds=t)
        # compute true sun azimuth in world frame
        sun_world = polarization.sun_azimuth(dt)

        # simulate sensors given true heading
        sensor_data = polarization.simulate_polar_sensors(true_heading=true_headings[t],
                                                          sun_azimuth_world=sun_world,
                                                          sensor_angles_deg=sensor_angles,
                                                          noise_sigma=0.03)

        # estimate heading from sensors + location/time
        measured_h, sigma, sun_w, sun_b = polarization.estimate_heading_from_sensors(
            sensor_data, sensor_angles, dt)
        measured_headings[t] = measured_h

        # Use measured heading for TL/CL/TB, but use true heading+velocity for flow (optic flow)
        tl2 = cx.tl2_output(measured_h)
        cl1 = cx.cl1_output(tl2)
        tb1 = cx.tb1_output(cl1, tb1)

        flow = cx.get_flow(true_headings[t], v[t])
        tn1 = cx.tn1_output(flow)
        tn2 = cx.tn2_output(flow)

        memory = cx.cpu4_update(memory, tb1, tn1, tn2)
        cpu4 = cx.cpu4_output(memory)
        cpu1 = cx.cpu1_output(tb1, cpu4)
        motor = cx.motor_output(cpu1)

        # small console output for first steps
        if t < 10:
            print(f"t={t:3d} true_h={true_headings[t]:+.3f} meas_h={measured_h:+.3f} sigma={sigma:.3f} motor={motor:.3f}")

    # Save a compact comparison for later plotting/analysis
    out = {
        'true_headings': true_headings,
        'measured_headings': measured_headings,
        'lat': lat,
        'lon': lon,
    }
    np.savez('data/polar_example.npz', **out)
    print('Saved data/polar_example.npz')


if __name__ == '__main__':
    run_example()
