import asyncio
from bleak import BleakClient

# ================= 配置区 =================
DEVICE_MAC = "9D:F1:64:BE:37:65"
CHARACTERISTIC_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"

# 协议帧: [帧头0xAB] [Sign] [~Sign取反校验] [帧尾0xBA]
SIGN = 9    # 9=Sit(坐)
DATA_TO_SEND = bytes([0xAB, SIGN, (~SIGN) & 0xFF, 0xBA])
# ==========================================

async def main():
    print(f"正在尝试连接设备: {DEVICE_MAC}...")

    # 使用异步上下文管理器自动管理连接与断开
    async with BleakClient(DEVICE_MAC) as client:
        if client.is_connected:
            print("连接成功！")

            print(f"正在向特征值 {CHARACTERISTIC_UUID} 写入数据...")
            # print(f"发送帧: {DATA_TO_SEND.hex(' ').upper()} (Sign={SIGN}, ~Sign={((~SIGN) & 0xFF):#04x})")
            # write_gatt_char 参数: (特征值UUID, 字节数据, response=False/True)
            # JDY-23 透传通常不需要回复，设为 False 类似于 bluetoothctl 的 write 命令
            await client.write_gatt_char(CHARACTERISTIC_UUID, DATA_TO_SEND, response=False)

            print("数据发送完毕！")
        else:
            print("连接失败！")

# 启动异步事件循环
if __name__ == "__main__":
    asyncio.run(main())