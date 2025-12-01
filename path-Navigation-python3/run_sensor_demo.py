"""Demo showing sensor-driven workflow:
- Accepts 16 polarization sensors (I0,I90) per timestep
- Takes flow vector per timestep as velocity input
- Uses polarization.estimate_heading_from_sensors -> measured heading
- Uses trials.update_cells with measured heading and flow vector to run CX
- Saves results to data/sensor_demo.npz
"""
from datetime import datetime, timezone
import numpy as np

import polarization
import trials
import bee_simulator
import cx_rate


def main():
    T = 200
    lat = 51.5
    lon = -0.1
    start_dt = datetime.now(timezone.utc)

    # Generate a true route and velocities to synthesize sensor inputs
    true_h, v = trials.generate_route(T=T, mean_acc=0.15, vary_speed=False)

    # sensor layout: 8 sensors
    sensor_angles = np.linspace(0, 360, 8, endpoint=False)

    # simulate sensor_data per timestep using true heading & sun position
    sensor_data_seq = np.zeros((T, len(sensor_angles), 2))
    for t in range(T):
        dt = start_dt + np.timedelta64(t, 's').astype(object)
        sun_world = polarization.sun_azimuth(lat, lon, dt)
        sensor_data_seq[t] = polarization.simulate_polar_sensors(true_heading=true_h[t],
                                                                 sun_azimuth_world=sun_world,
                                                                 sensor_angles_deg=sensor_angles,
                                                                 noise_sigma=0.02)

    # Use velocities (v array returned from generate_route) as flow vectors
    # v shape: (T,2)
    flow_seq = v.copy()

    # run sensor driven trial
    # hold motor for n seconds: set motor_hold to control how often motor updates
    motor_hold = 3
    measured_headings, motors, log = trials.run_sensor_driven_trial(
        sensor_data_seq, flow_seq, cx=None, logging=True, lat=lat, lon=lon, start_dt=start_dt,
        sensor_angles_deg=sensor_angles, sensor_noise=0.02, motor_hold=motor_hold)
        

    print('Example motors :', motors)
    np.savez('data/sensor_demo1.npz', measured_headings=measured_headings, motors=motors)
    print('Saved data/sensor_demo1.npz')


if __name__ == '__main__':
    main()
