import serial
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Button as MPLButton, TextBox
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei'] # 优先使用黑体或微软雅黑
plt.rcParams['axes.unicode_minus'] = False # 解决有时候负号显示成方块的问题
# 开启超时机制，确保没有数据时不会无限死等
SER = serial.Serial('COM11', 115200, timeout=1)
running = True

# ---- Modbus CRC16 ----
def modbus_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

# ---- 读寄存器 (03) ----
def read_modbus():
    # 发送前彻底清空输入缓冲区
    SER.reset_input_buffer()
    
    # 构造读 10 个寄存器的请求 (01 03 00 00 00 0A)
    req = bytes([0x01, 0x03, 0x00, 0x00, 0x00, 0x0A])
    crc = modbus_crc16(req)
    req += bytes([crc & 0xFF, (crc >> 8) & 0xFF])
    
    SER.write(req)
    
    # 严格读取 25 字节: 1站号 + 1功能码 + 1字节数(20) + 20数据(10个寄存器) + 2校验 = 25
    resp = SER.read(25) 
    
    if len(resp) < 25:
        return None
        
    if resp[1] == 0x03:
        crc_calc = modbus_crc16(resp[:23])
        crc_recv = resp[23] | (resp[24] << 8)
        
        if crc_calc != crc_recv:
            return None
            
        # 循环精准解析 10 个寄存器
        regs = []
        for i in range(10):
            val = (resp[3 + i*2] << 8) | resp[4 + i*2]
            regs.append(val)
            
        # 解析前三个通道的物理量
        temp  = regs[0] / 10.0
        humi  = regs[1] / 10.0
        light = regs[2] / 10.0
        
        # 返回所有 10 个数据
        return (temp, humi, light, regs[3], regs[4], regs[5], regs[6], regs[7], regs[8], regs[9])
        
    return None

# ---- 写单个寄存器 (06) ----
def write_single_register(slave, addr, value):
    SER.reset_input_buffer()
    frame = bytes([slave, 0x06, (addr >> 8) & 0xFF, addr & 0xFF,
                   (value >> 8) & 0xFF, value & 0xFF])
    crc = modbus_crc16(frame)
    frame += bytes([crc & 0xFF, (crc >> 8) & 0xFF])
    SER.write(frame)
    
    resp = SER.read(8)
    if len(resp) >= 8 and resp[1] == 0x06:
        crc_calc = modbus_crc16(resp[:6])
        crc_recv = resp[6] | (resp[7] << 8)
        if crc_calc == crc_recv:
            return f"OK: {resp.hex().upper()}"
        return f"CRC ERR: {resp.hex().upper()}"
    return "ERR: 无回复或长度错误"

# ---- 数据缓存 ----
temp_data, humi_data, light_data = [], [], []
MAX_POINTS = 200
latest = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0) 

# ---- matplotlib 曲线与界面刷新 ----
def animate(_):
    global latest
    if not running:
        return
    d = read_modbus()
    if d is None:
        return
    latest = d
    t, h, l = d[0], d[1], d[2]
    temp_data.append(t); humi_data.append(h); light_data.append(l)
    while len(temp_data) > MAX_POINTS:
        temp_data.pop(0); humi_data.pop(0); light_data.pop(0)
        
    ax.clear()
    ax.plot(temp_data,  label='Temperature (°C)',  color='#f87171')
    ax.plot(humi_data,  label='Humidity (%)',       color='#34d399')
    ax.plot(light_data, label='Illuminance (Lux)',  color='#22d3ee')
    ax.legend(loc='lower left', fontsize=8) # 图例移到左下角，给左上角警报留出空间
    ax.set_title('Industrial Env Monitor V2')

    # ================= 🌟 核心新增：Modbus 位域警报解析 =================
    alarm_word = latest[9] # 第 10 个寄存器
    active_alarms = []
    
    # 检查 bit0 (0x01)
    if alarm_word & 0x01:
        active_alarms.append("温度过高警报！")
    # 检查 bit1 (0x02)
    if alarm_word & 0x02:
        active_alarms.append("湿度异常警报！")
    # 检查 bit2 (0x04)
    if alarm_word & 0x04:
        active_alarms.append("光照过低警报！")
        
    # 如果有任何警报被触发，在图表左上角弹框显示
    if active_alarms:
        alarm_string = "【系统警报】\n" + "\n".join(active_alarms)
        # 绘制在左上角 (x=0.02, y=0.96)
        ax.text(0.02, 0.96, alarm_string, transform=ax.transAxes,
                fontsize=10, color='#b91c1c', fontweight='bold', 
                verticalalignment='top', horizontalalignment='left',
                bbox=dict(boxstyle='round,pad=0.5', facecolor='#fee2e2', edgecolor='#ef4444', alpha=0.9))

    # ================= 右侧：10个寄存器数值数据显示 =================
    texts = [
        f"Reg0 (温度):  {latest[0]:.1f} °C",
        f"Reg1 (湿度):  {latest[1]:.1f} %",
        f"Reg2 (光照): {latest[2]:.1f} Lux",
        f"Reg3 (状态): {latest[3]}",
        f"Reg4 (版本):    {latest[4]}",
        f"Reg5 (从机地址):  {latest[5]}",
        f"Reg6 (温度阈值):        {latest[6]}",
        f"Reg7 (湿度阈值):        {latest[7]}",
        f"Reg8 (光照阈值):        {latest[8]}",
        f"Reg9 (警报位): {latest[9]} (0x{latest[9]:02X})",
    ]
    
    for i, line in enumerate(texts):
        ax.text(0.99, 0.98 - i*0.045, line, transform=ax.transAxes,
                fontsize=9, verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))

def toggle_pause(_):
    global running
    running = not running
    btn_pause.label.set_text("Pause" if running else "Resume")

# ---- 06 写寄存器回调 ----
def on_write(_):
    try:
        slave = int(tb_slave.text)
        addr  = int(tb_addr.text)
        value = int(tb_value.text)
        frame = bytes([slave, 0x06, (addr>>8)&0xFF, addr&0xFF, (value>>8)&0xFF, value&0xFF])
        crc = modbus_crc16(frame)
        hex_str = frame.hex().upper() + f" {crc&0xFF:02X} {(crc>>8)&0xFF:02X}"
        log_lines.append(f">>> {hex_str}")
        result = write_single_register(slave, addr, value)
        log_lines.append(f"<<< {result}")
        while len(log_lines) > 8:
            log_lines.pop(0)
        log_text.set_text("\n".join(log_lines))
    except Exception as e:
        log_lines.append(f"ERR: {e}")
        log_text.set_text("\n".join(log_lines))

log_lines = ["日志:"]

# ---- 构建界面 ----
fig = plt.figure("Env Monitor", figsize=(11, 5.5))
ax = fig.add_subplot(111)
plt.subplots_adjust(bottom=0.20)

# 暂停按钮
btn_ax = fig.add_axes([0.91, 0.02, 0.07, 0.06])
btn_pause = MPLButton(btn_ax, 'Pause')
btn_pause.on_clicked(toggle_pause)

# 写寄存器控件
fig.text(0.02, 0.14, "Slave:", fontsize=9, color='black')
tb_slave_ax = fig.add_axes([0.08, 0.12, 0.06, 0.04])
tb_slave = TextBox(tb_slave_ax, '', initial='1')

fig.text(0.16, 0.14, "Reg:", fontsize=9, color='black')
tb_addr_ax = fig.add_axes([0.21, 0.12, 0.06, 0.04])
tb_addr = TextBox(tb_addr_ax, '', initial='5')

fig.text(0.29, 0.14, "Value:", fontsize=9, color='black')
tb_value_ax = fig.add_axes([0.35, 0.12, 0.07, 0.04])
tb_value = TextBox(tb_value_ax, '', initial='2')

write_btn_ax = fig.add_axes([0.44, 0.12, 0.08, 0.04])
write_btn = MPLButton(write_btn_ax, 'Write 06')
write_btn.on_clicked(on_write)

log_text = fig.text(0.55, 0.12, "\n".join(log_lines), fontsize=8,
                     fontfamily='monospace', va='bottom',
                     bbox=dict(boxstyle='round', facecolor='#f0f0f0', alpha=0.9))

ani = animation.FuncAnimation(fig, animate, interval=1000, cache_frame_data=False)
plt.show()
SER.close()