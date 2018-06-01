# iRobotxJoystick
![License](https://img.shields.io/badge/License-GPL--3.0-brightgreen.svg)

2018年第三届新生杯科技创新大赛机器人手柄控制程序

实现了手柄消息的处理和转换发送。

## 硬件
机器人和模块：iRobot® Roomba® 600、ESP8266、Xbox One S

## 软件
- 方向前进采用两轮差速控制机器人，手柄旋钮采用xaxis和yaxis描述，取值为-1000~1000，因此需要进行一下换算：
    1. 由于传感过于灵敏导致回复后值无法归零，我们采取了将两个值/100后取整再*100，这样可以极大平滑操作。
    2. 向前(0,-1000)两轮转速均最大255，向后(0,1000)两轮转速最小-255，向左(-1000,±0)两轮转速(0,±255)，向右(1000, ±0)两轮转速(±255,0)
    3. 其他方向令θ=arctan(double)yaxis/xaxis，然后根据角度值将四个象限左右轮转速从0到两极值均匀递增即可，具体代码见文件。
- 考虑到有可能需要快速转向，我们添加了转向和直行按钮，转向使用(-255,255)和(255,-255)实现原地转向
- 考虑到吸尘和扫帚的灵活性，我们将其操作按键独立出来，可以互不相干独立进行（虽然对于机器人代码是同一条指令），并且扫帚可以随时更换旋转方向
- 考虑到不同操作环境，我们设置了三挡变速，即计算输出速度乘以100%/66%/33%后才为实际输出速度，方便微操。
- 增加战歌按钮（参见“艺术性”），考虑到防误触，战歌的控制指令将会以多线程模式独立进行且在播放结束前忽略播放按钮操作，不会影响其他指令。同时考虑到带宽和机器人处理速度的问题，其他操作指令总是优先于战歌控制指令进行。
- 为了尽可能减少操作延迟，软件不进行任何日志记录或命令输出，仅刷新手柄输出值便于调试。
- Wifi模块固件重写
- Wifi配置，做到随开随用
- 消息无损转发
