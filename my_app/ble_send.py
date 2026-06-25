import asyncio
from bleak import BleakClient

# ================= 配置区 =================
DEVICE_MAC = "9D:F1:64:BE:37:65" 
CHARACTERISTIC_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
DATA_TO_SEND = b"HelloHello" # 加b代表强制转化为字符类型
# ==========================================

async def main():
    print(f"正在尝试连接设备: {DEVICE_MAC}...")
    
    # 使用异步上下文管理器自动管理连接与断开
    async with BleakClient(DEVICE_MAC) as client:
        if client.is_connected:
            print("连接成功！")
            
            print(f"正在向特征值 {CHARACTERISTIC_UUID} 写入数据...")
            # write_gatt_char 参数: (特征值UUID, 字节数据, response=False/True)
            # JDY-23 透传通常不需要回复，设为 False 类似于 bluetoothctl 的 write 命令
            await client.write_gatt_char(CHARACTERISTIC_UUID, DATA_TO_SEND, response=False)
            
            print("数据发送完毕！")
        else:
            print("连接失败！")

# 启动异步事件循环
if __name__ == "__main__":
    asyncio.run(main())