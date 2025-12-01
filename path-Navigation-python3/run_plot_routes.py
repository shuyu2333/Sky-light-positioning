"""Run a trial and plot outbound and inbound trajectories using plotter.plot_route

Saves a PDF in the `plots/` directory named 'example_route_plot.pdf'.
"""
from datetime import datetime, timezone
import numpy as np

import trials
import plotter


def main():
    T_out = 200
    T_in = 200
    print('Running trial to generate routes...')
    # Run a trial (with optional use_polarized=False to use true heading for TL)
    h, v, log, cpu4 = trials.run_trial(T_outbound=T_out, T_inbound=T_in, logging=True,
                                       use_polarized=False)

    print('Plotting route...')
    fig, ax = plotter.plot_route(h, v, T_outbound=T_out, T_inbound=T_in,
                                plot_speed=False, plot_heading=True,
                                outbound_color='purple', inbound_color='green')
    plotter.save_plot(fig, 'example_route_plot1')
    print('Saved plots/example_route_plot1.pdf')


if __name__ == '__main__':
    main()
