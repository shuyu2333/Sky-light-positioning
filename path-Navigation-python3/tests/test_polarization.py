from datetime import datetime, timezone
import math

from polarization import sun_azimuth


def test_sun_azimuth_simplified_north():
    # At lat >= 22.5 the simplified formula should apply.
    # 06:00 UTC -> hour=6.0 -> az_deg = 90 deg -> pi/2 rad
    dt = datetime(2025, 1, 1, 6, 0, 0, tzinfo=timezone.utc)
    az = sun_azimuth(dt)
    assert math.isclose(az, math.pi / 2, rel_tol=1e-9, abs_tol=0.0)


def test_sun_azimuth_fractional_hour():
    # 06:30 UTC -> hour = 6.5 -> az = 97.5 degrees
    dt = datetime(2025, 1, 1, 6, 30, 0, tzinfo=timezone.utc)
    expected = math.radians(97.5)
    az = sun_azimuth(dt)
    assert math.isclose(az, expected, rel_tol=1e-9)
