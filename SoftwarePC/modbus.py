import serial
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Button as MPLButton, TextBox

SER = serial.Serial('COM7', 115200, timeout=1)
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
    SER.write(bytes.fromhex('01 03 00 00 00 06 C5 C8'))
    time.sleep(0.15)
    n = SER.in_waiting
    if n < 17:
        SER.read(n)
        return None
    resp = SER.read(n)
    if len(resp) >= 17 and resp[1] == 0x03:
        temp   = ((resp[3]  << 8) | resp[4])  / 10.0
        humi   = ((resp[5]  << 8) | resp[6])  / 10.0
        light  = ((resp[7]  << 8) | resp[8])  / 10.0
        dev_st = (resp[9]  << 8) | resp[10]
        fw_ver = (resp[11] << 8) | resp[12]
        sla    = (resp[13] << 8) | resp[14]
        return (temp, humi, light, dev_st, fw_ver, sla)
    return None

# ---- 写单个寄存器 (06) ----
def write_single_register(slave, addr, value):
    frame = bytes([slave, 0x06, (addr >> 8) & 0xFF, addr & 0xFF,
                   (value >> 8) & 0xFF, value & 0xFF])
    crc = modbus_crc16(frame)
    frame += bytes([crc & 0xFF, (crc >> 8) & 0xFF])
    SER.write(frame)
    time.sleep(0.15)
    n = SER.in_waiting
    if n >= 8:
        resp = SER.read(n)
        if len(resp) >= 8 and resp[1] == 0x06:
            return f"OK: {resp.hex().upper()}"
        return f"ERR: {resp.hex().upper()}"
    SER.read(SER.in_waiting)
    return "ERR: 无回复"

# ---- 数据缓存 ----
temp_data, humi_data, light_data = [], [], []
MAX_POINTS = 200
latest = (0, 0, 0, 0, 0, 0)

# ---- matplotlib 曲线 ----
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
    ax.legend(loc='upper left', fontsize=8)
    ax.set_title('Industrial Env Monitor V2')

    texts = [
        f"Temp:  {latest[0]:.1f} °C",
        f"Humi:  {latest[1]:.1f} %",
        f"Light: {latest[2]:.1f} Lux",
        f"Status:{latest[3]}",
        f"FW:    {latest[4]}",
        f"Addr:  {latest[5]}",
    ]
    for i, line in enumerate(texts):
        ax.text(0.99, 0.95 - i*0.06, line, transform=ax.transAxes,
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

# 写寄存器控件 (底部一行)
# 标签: Slave
fig.text(0.02, 0.14, "Slave:", fontsize=9, color='black')
tb_slave_ax = fig.add_axes([0.08, 0.12, 0.06, 0.04])
tb_slave = TextBox(tb_slave_ax, '', initial='1')

# 标签: Register
fig.text(0.16, 0.14, "Reg:", fontsize=9, color='black')
tb_addr_ax = fig.add_axes([0.21, 0.12, 0.06, 0.04])
tb_addr = TextBox(tb_addr_ax, '', initial='5')

# 标签: Value
fig.text(0.29, 0.14, "Value:", fontsize=9, color='black')
tb_value_ax = fig.add_axes([0.35, 0.12, 0.07, 0.04])
tb_value = TextBox(tb_value_ax, '', initial='2')

# 发送按钮
write_btn_ax = fig.add_axes([0.44, 0.12, 0.08, 0.04])
write_btn = MPLButton(write_btn_ax, 'Write 06')
write_btn.on_clicked(on_write)

# 日志区域 (文本)
log_text = fig.text(0.55, 0.12, "\n".join(log_lines), fontsize=8,
                     fontfamily='monospace', va='bottom',
                     bbox=dict(boxstyle='round', facecolor='#f0f0f0', alpha=0.9))

ani = animation.FuncAnimation(fig, animate, interval=1000, cache_frame_data=False)
plt.show()
SER.close()
