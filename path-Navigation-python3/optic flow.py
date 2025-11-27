import cv2
import numpy as np

# 读取视频（或摄像头）
cap = cv2.VideoCapture("input_video.mp4")
ret, prev_frame = cap.read()
prev_gray = cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY)

# 获取视频帧率（用于计算时间间隔dt）
fps = cap.get(cv2.CAP_PROP_FPS)
dt = 1.0 / fps  # 两帧之间的时间间隔（秒）

while True:
    ret, curr_frame = cap.read()
    if not ret:
        break
    curr_gray = cv2.cvtColor(curr_frame, cv2.COLOR_BGR2GRAY)
    
    # 计算稠密光流（Farneback算法）
    # flow是一个双通道数组，flow[...,0]是x方向位移，flow[...,1]是y方向位移
    flow = cv2.calcOpticalFlowFarneback(
        prev_gray, curr_gray, None,
        pyr_scale=0.5,  # 金字塔缩放比例
        levels=3,       # 金字塔层数
        winsize=15,     # 窗口大小
        iterations=3,   # 迭代次数
        poly_n=5,       # 多项式拟合窗口大小
        poly_sigma=1.2, # 高斯标准差
        flags=0
    )
    
    # 计算速度（像素/秒）：位移 / 时间间隔dt
    vx = flow[..., 0] / dt  # x方向速度
    vy = flow[..., 1] / dt  # y方向速度
    
    # （可选）可视化光流（箭头表示方向和大小）
    h, w = curr_gray.shape[:2]
    step = 16  # 每隔16个像素画一个箭头
    y, x = np.mgrid[step/2:h:step, step/2:w:step].reshape(2, -1).astype(int)
    fx, fy = flow[y, x].T
    lines = np.vstack([x, y, x+fx, y+fy]).T.reshape(-1, 2, 2)
    lines = np.int32(lines + 0.5)
    vis = cv2.cvtColor(curr_gray, cv2.COLOR_GRAY2BGR)
    cv2.polylines(vis, lines, 0, (0, 255, 0))
    for (x1, y1), (x2, y2) in lines:
        cv2.circle(vis, (x1, y1), 1, (0, 255, 0), -1)
    
    cv2.imshow("Optical Flow & Velocity", vis)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
    # 更新前一帧
    prev_gray = curr_gray

cap.release()
cv2.destroyAllWindows()