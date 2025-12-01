
import numpy as np
import matplotlib.pyplot as plt

import cx_rate
import trials
import plotter

import json

route_file = 'route.npz'
T_outbound = 1500
T_inbound = 1500

cx = cx_rate.CXRatePontin(noise=0)

h, v, _ = trials.load_route(filename=route_file)
h, v, log, cpu4_snapshot = trials.run_trial(logging=True,
                                            T_outbound=T_outbound,
                                            T_inbound=T_inbound,
                                            noise=0,
                                            cx=cx,
                                            route=(h[:T_outbound], v[:T_outbound]))

fig, ax = plotter.plot_route(h, v, T_outbound=T_outbound, T_inbound=T_inbound,
                   plot_speed=True, plot_heading=True, quiver_color='black')