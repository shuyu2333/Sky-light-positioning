"""Utilities to simulate polarized-sky sensor readings and estimate heading.

This module provides:
- sun_azimuth(lat, lon, dt): approximate solar azimuth (radians, from North, clockwise)
- simulate_polar_sensors(true_heading, sun_azimuth, sensor_angles_deg, noise)
  -> returns sensor_data array shaped (N, 2) with (I0, I90) per sensor
- estimate_sun_azimuth_from_sensors(sensor_data, sensor_angles_deg)
- estimate_heading_from_sensors(sensor_data, sensor_angles_deg, lat, lon, dt)

The goal is to allow experiments where the CX receives a heading estimate
derived from polarized-sky sensors instead of the ground-truth simulator heading.
"""
from __future__ import annotations
import numpy as np
from datetime import datetime, timezone
import math


def _normalize_angle_rad(a: float) -> float:
    """Normalize angle to [-pi, pi)."""
    return (a + np.pi) % (2 * np.pi) - np.pi


def sun_azimuth(lat_deg: float, lon_deg: float, dt: datetime) -> float:
    """Approximate solar azimuth (radians) for given location and UTC datetime.

    Returns azimuth measured clockwise from North (0 = North), range [-pi, pi).

    This uses a compact version of the NOAA solar position calculation
    and is accurate to a few degrees for typical experimental use.
    """
    # Convert to Julian day
    # Source: NOAA / simplified astronomical formulas
    # dt assumed timezone-aware (UTC) or naive UTC.
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=timezone.utc)
    # Julian day
    year = dt.year
    month = dt.month
    day = dt.day + (dt.hour + dt.minute / 60.0 + dt.second / 3600.0) / 24.0
    if month <= 2:
        year -= 1
        month += 12
    A = int(year / 100)
    B = 2 - A + int(A / 4)
    jd = int(365.25 * (year + 4716)) + int(30.6001 * (month + 1)) + day + B - 1524.5

    jc = (jd - 2451545.0) / 36525.0  # Julian century

    # Geometric mean longitude of the sun (deg)
    L0 = (280.46646 + jc * (36000.76983 + jc * 0.0003032)) % 360.0
    # Geometric mean anomaly (deg)
    M = 357.52911 + jc * (35999.05029 - 0.0001537 * jc)
    # eccentricity
    e = 0.016708634 - jc * (0.000042037 + 0.0000001267 * jc)
    # Sun eq of center
    C = (1.914602 - jc * (0.004817 + 0.000014 * jc)) * math.sin(math.radians(M))
    C += (0.019993 - 0.000101 * jc) * math.sin(math.radians(2 * M))
    C += 0.000289 * math.sin(math.radians(3 * M))
    # true longitude
    true_long = L0 + C
    # apparent longitude
    omega = 125.04 - 1934.136 * jc
    lam = true_long - 0.00569 - 0.00478 * math.sin(math.radians(omega))
    # obliquity
    eps0 = 23.0 + (26.0 + ((21.448 - jc * (46.815 + jc * (0.00059 - jc * 0.001813)))) / 60.0) / 60.0
    eps = eps0 + 0.00256 * math.cos(math.radians(omega))

    # declination
    sin_dec = math.sin(math.radians(eps)) * math.sin(math.radians(lam))
    dec = math.degrees(math.asin(sin_dec))

    # equation of time (minutes)
    y = math.tan(math.radians(eps / 2.0)) ** 2
    eq_time = 4.0 * math.degrees(y * math.sin(2.0 * math.radians(L0)) - 2.0 * e * math.sin(math.radians(M))
                                  + 4.0 * e * y * math.sin(math.radians(M)) * math.cos(2.0 * math.radians(L0))
                                  - 0.5 * y * y * math.sin(4.0 * math.radians(L0))
                                  - 1.25 * e * e * math.sin(2.0 * math.radians(M)))

    # solar time
    timezone_offset = 0  # input dt is UTC
    time_offset = eq_time + 4.0 * lon_deg - 60.0 * timezone_offset
    true_solar_time = (dt.hour * 60.0 + dt.minute + dt.second / 60.0 + time_offset) % 1440

    # hour angle
    if true_solar_time / 4.0 < 0:
        ha = true_solar_time / 4.0 + 180.0
    else:
        ha = true_solar_time / 4.0 - 180.0

    # convert to radians
    lat_r = math.radians(lat_deg)
    dec_r = math.radians(dec)
    ha_r = math.radians(ha)

    # Solar zenith angle
    cos_zenith = math.sin(lat_r) * math.sin(dec_r) + math.cos(lat_r) * math.cos(dec_r) * math.cos(ha_r)
    cos_zenith = min(1.0, max(-1.0, cos_zenith))
    zenith = math.acos(cos_zenith)

    # Solar azimuth (clockwise from North)
    sin_az = -(math.sin(ha_r) * math.cos(dec_r)) / math.sin(zenith) if math.sin(zenith) != 0 else 0.0
    cos_az = (math.sin(dec_r) - math.sin(lat_r) * math.cos(zenith)) / (math.cos(lat_r) * math.sin(zenith)) if math.sin(zenith) != 0 else 0.0
    az = math.degrees(math.atan2(sin_az, cos_az))
    # az now degrees from North, clockwise positive
    az_rad = math.radians(az)
    # normalize
    az_rad = _normalize_angle_rad(az_rad)
    return az_rad


def simulate_polar_sensors(true_heading: float,
                           sun_azimuth_world: float,
                           sensor_angles_deg=None,
                           noise_sigma: float = 0.02):
    """Simulate simple polarized-sky sensor array.

    sensor_angles_deg: iterable of sensor azimuths relative to body (degrees).
    Returns sensor_data shaped (N, 2) with columns (I0, I90) per sensor.
    """
    if sensor_angles_deg is None:
        # default 8 sensors at 45deg spacing
        sensor_angles_deg = np.arange(0, 360, 45)
    sensor_angles = np.deg2rad(np.array(sensor_angles_deg))
    # sun angle in body coordinates
    sun_body = _normalize_angle_rad(sun_azimuth_world - true_heading)

    N = len(sensor_angles)
    data = np.zeros((N, 2))
    for i, ang in enumerate(sensor_angles):
        # relative angle between sensor and sun direction
        d = _normalize_angle_rad(sun_body - ang)
        # simple intensity model: stronger when facing sun
        I0 = max(0.0, math.cos(d))
        I90 = max(0.0, math.cos(d + math.pi / 2.0))
        # add gaussian noise
        I0 += np.random.normal(scale=noise_sigma)
        I90 += np.random.normal(scale=noise_sigma)
        # clip
        data[i, 0] = max(0.0, I0)
        data[i, 1] = max(0.0, I90)
    return data


def estimate_sun_azimuth_from_sensors(sensor_data, sensor_angles_deg=None):
    """Estimate sun azimuth (radians, clockwise from North) from sensor readings.

    sensor_data shape: (N, 2) columns (I0, I90) per sensor.
    sensor_angles_deg: angles of sensors relative to body (deg). If None assume even spacing.
    Returns (sun_azimuth_rad_body, sigma, I_values, p_values, c_values)
    where sun_azimuth_rad_body is the sun azimuth in body coordinates (radians).
    """
    data = np.asarray(sensor_data)
    N = data.shape[0]
    if sensor_angles_deg is None:
        sensor_angles_deg = np.linspace(0, 360, N, endpoint=False)
    sensor_angles_rad = np.deg2rad(sensor_angles_deg)

    I_values = np.zeros(N)
    p_values = np.zeros(N)
    c_values = np.zeros(N)

    for i in range(N):
        I0 = data[i, 0]
        I90 = data[i, 1]
        I = (I90 + I0) / 2.0
        p = (I90 - I0) / I if I > 0 else 0.0
        c = I - p
        I_values[i] = I
        p_values[i] = p
        c_values[i] = c

    z_c = 0 + 0j
    for k in range(N):
        z_c += c_values[k] * np.exp(1j * sensor_angles_rad[k])
    z_c /= float(N)
    magnitude = np.abs(z_c)
    if magnitude > 0:
        sun_azimuth_rad = (_normalize_angle_rad((-1j * np.log(z_c / magnitude)).real))
        sigma_c = math.sqrt(max(0.0, 2.0 * (1.0 - magnitude)))
    else:
        sun_azimuth_rad = 0.0
        sigma_c = 0.0
    return sun_azimuth_rad, sigma_c, I_values, p_values, c_values


def estimate_heading_from_sensors(sensor_data, sensor_angles_deg, lat, lon, dt: datetime):
    """Estimate agent heading (radians) from sensor_data and geographic location/time.

    Heading is computed as: heading = sun_azimuth_world - sun_azimuth_body
    where sun_azimuth_world comes from geographic/time calculation and
    sun_azimuth_body is estimated from sensors (both clockwise from North).
    """
    sun_body, sigma, I, p, c = estimate_sun_azimuth_from_sensors(sensor_data, sensor_angles_deg)
    sun_world = sun_azimuth(lat, lon, dt)
    heading = _normalize_angle_rad(sun_world - sun_body)
    return heading, sigma, sun_world, sun_body
