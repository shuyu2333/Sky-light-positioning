import numpy as np


def calculate_sun_azimuth_from_sensors(sensor_data):
    """
    根据论文公式1-5，从8个传感器的32个偏振光数据计算太阳方位角
    
    参数:
    sensor_data: 形状为(8, 4)的数组，8个传感器 × 4个偏振方向 (0°,45°,90°,135°)
                输入为0-4096范围的ADC值
    
    返回:
    sun_azimuth_deg: 太阳方位角（度）
    sigma_c: angular deviation角度偏差
    """
    
    # 添加ADC值归一化处理
    # 将0-4096范围的ADC值转换为0-1范围
    normalized_sensor_data = sensor_data / 4096.0
    
    # 传感器方向（度）
    sensor_azimuths_deg = np.array([0, 45, 90, 135, 180, 225, 270, 315])
    sensor_azimuths_rad = np.deg2rad(sensor_azimuths_deg)
    Number_sensors = 8
    
    # 初始化结果数组
    I_values = np.zeros(8)  # 总光强
    p_values = np.zeros(8)  # 偏振 opponency
    c_values = np.zeros(8)  # 天体积分

    # 对每个传感器计算参数（公式1-3）
    for i in range(Number_sensors):
        # 获取该传感器的2个偏振通道数据（修改索引以正确获取数据）
        I0, I90 = normalized_sensor_data[i, 0], normalized_sensor_data[i, 2]

        # 公式1: 总光强 I = (I90 + I0)/2
        I = (I90 + I0) / 2

        # 公式2: 偏振 opponency p = (I90 - I0)/I
        if I > 0:
            p = (I90 - I0) / I
        else:
            p = 0

        # 公式3: 天体积分 c = I - p
        c = I - p

        I_values[i] = I
        p_values[i] = p
        c_values[i] = c

    # 公式4: 向量平均计算 z_c = (1/K) * Σ [c_k * e^(i * azimuth_k)]
    z_c = 0 + 0j
    for k in range(Number_sensors):
        vector = c_values[k] * np.exp(1j * sensor_azimuths_rad[k])  # sensors的弧度
        z_c += vector
    z_c /= Number_sensors

    # 公式5: 提取角度和置信度
    # α_c = -i * ln(z_c / ||z_c||)
    # σ_c = sqrt(2*(1-||z_c||))

    magnitude = np.abs(z_c)

    if magnitude > 0:
        # 计算太阳方位角（弧度）
        sun_azimuth_rad = (-1j * np.log(z_c / magnitude)).real # 复数实部
        # 转换为度并归一化到0-360
        sun_azimuth_deg = np.rad2deg(sun_azimuth_rad) % 360
        sigma_c = np.sqrt(2 * (1 - magnitude))  # 论文公式 5


    else:
        sun_azimuth_deg = 0
        sigma_c = 0

    return sun_azimuth_deg, sigma_c, I_values, p_values, c_values


# 使用示例
if __name__ == "__main__":
    # 示例数据：8个传感器，每个传感器4个偏振通道
    # 假设数据已经归一化到0-1范围
    example_sensor_data = np.array([
        [0.3, 0.4],  # 0° - 远离太阳，光强较弱
        [0.4, 0.5],  # 45°
        [0.9, 0.8],  # 90° - 指向太阳，光强最强
        [0.5, 0.4],  # 135°
        [0.3, 0.2],  # 180°
        [0.2, 0.3],  # 225°
        [0.3, 0.4],  # 270°
        [0.4, 0.5]  # 315°
    ])

    # 计算太阳方位角
    sun_azimuth, sigma_c, I, p, c = calculate_sun_azimuth_from_sensors(example_sensor_data)

    print(f"太阳方位角: {sun_azimuth:.1f}°")
    print(f"角度误差: {sigma_c:.3f}")
    print(f"各传感器光强(I): {I}")
    print(f"各传感器偏振(p): {p}")
    print(f"各传感器天体积分(c): {c}")