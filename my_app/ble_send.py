import sys
import os
import asyncio
import binascii
from bleak import BleakClient

# ================= 配置区 =================
DEVICE_MAC = "9D:F1:64:BE:37:65"
CHARACTERISTIC_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
# ==========================================


def fmt_hex(data):
    """将 bytes 格式化为空格分隔的大写十六进制字符串（兼容 Python 3.7）"""
    return ' '.join(f'{b:02X}' for b in data)


def check_oled():
    """检查 OLED 内核模块是否已加载（/dev/oled 是否存在）"""
    if not os.path.exists("/dev/oled"):
        print("错误: OLED 内核模块未加载，/dev/oled 不存在！", file=sys.stderr)
        sys.exit(1)


def write_oled(sign, data_frame):
    """向 /dev/oled 写入当前控制状态和发送的数据帧"""
    hex_str = fmt_hex(data_frame)
    oled_content = f"0,0,sign={sign};0,2,{hex_str}"
    try:
        with open("/dev/oled", "w") as f:
            f.write(oled_content)
    except Exception as e:
        print(f"警告: 写入 OLED 失败: {e}", file=sys.stderr)


async def main(sign):
    print(f"正在尝试连接设备: {DEVICE_MAC}...")

    # 协议帧: [帧头0xAB] [Sign] [~Sign取反校验] [帧尾0xBA]
    data_frame = bytes([0xAB, sign, (~sign) & 0xFF, 0xBA])
    print(f"数据帧: {fmt_hex(data_frame)} (Sign={sign})")

    # 写入 OLED 显示当前状态
    write_oled(sign, data_frame)

    async with BleakClient(DEVICE_MAC) as client:
        if client.is_connected:
            print("连接成功！")
            print(f"正在向特征值 {CHARACTERISTIC_UUID} 写入数据...")
            await client.write_gatt_char(CHARACTERISTIC_UUID, data_frame, response=False)
            print("数据发送完毕！")
        else:
            print("连接失败！")


if __name__ == "__main__":
    # 解析命令行参数
    if len(sys.argv) != 2:
        print(f"用法: {sys.argv[0]} <sign>", file=sys.stderr)
        print("  sign: 1-12 的整数", file=sys.stderr)
        sys.exit(1)

    try:
        sign = int(sys.argv[1])
    except ValueError:
        print(f"错误: sign 必须是整数，收到 '{sys.argv[1]}'", file=sys.stderr)
        sys.exit(1)

    if sign < 1 or sign > 12:
        print(f"错误: sign 必须在 1-12 范围内，收到 {sign}", file=sys.stderr)
        sys.exit(1)

    # 检查 OLED 内核模块是否已加载
    check_oled()

    asyncio.run(main(sign))
