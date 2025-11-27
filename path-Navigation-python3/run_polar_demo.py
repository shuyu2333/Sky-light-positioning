"""Demo that runs run_trial with polarized-sky heading estimation enabled.

This script runs a short trial with use_polarized=True and saves the result
to data/polar_demo.npz. It prints a small summary to the console.
"""
from datetime import datetime, timezone
import numpy as np

import trials


def main():
    T_out = 100
    T_in = 100
    lat = 51.5
    lon = -0.1
    start_dt = datetime.now(timezone.utc)

    print("Running polarized demo: this may take a few seconds...")
    h, v, log, cpu4_snapshot = trials.run_trial(
        T_outbound=T_out,
        T_inbound=T_in,
        logging=True,
        use_polarized=True,
        lat=lat,
        lon=lon,
        start_dt=start_dt,
        sensor_noise=0.03,
        cx=None)

    print(f"Done. Outbound length: {log.T_outbound}, Inbound length: {log.T_inbound}")
    print(f"Saved CPU4 snapshot (shape): {cpu4_snapshot.shape}")
    # Save compact file
    np.savez('data/polar_demo.npz', h=h, v=v, cpu4_snapshot=cpu4_snapshot)
    print('Saved data/polar_demo.npz')


if __name__ == '__main__':
    main()
