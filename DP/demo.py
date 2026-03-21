import asyncio
from bleak import BleakScanner, BleakClient

TARGET_NAME = "HC-05"

# Списък с тестови команди във формат: "Режим,R,G,B,Скорост\n"
COMMANDS = [
    "1,255,0,0,50\n",  # Режим 1: Плътно Червено
    "2,0,255,0,30\n",  # Режим 2: Бягаща светлина Зелено (бърза)
    "3,0,0,255,20\n",  # Режим 3: Knight Rider Синьо (много бързо)
    "1,255,255,0,50\n",  # Режим 1: Плътно Жълто
    "0,0,0,0,0\n"  # Режим 0: Изключване
]


async def main():
    print("Стартиране на LED Контролер (BLE) с автоматично възстановяване на връзката.")
    print("Натисни Ctrl+C за спиране.\n")

    cmd_index = 0

    while True:
        try:
            print("Търсене на устройство...")
            devices = await BleakScanner.discover(timeout=5.0)

            target_device = None
            for d in devices:
                if d.name and TARGET_NAME in d.name:
                    target_device = d
                    break

            if not target_device:
                print("Устройството не е намерено. Нов опит след 5 секунди...\n")
                await asyncio.sleep(5)
                continue

            print(f"Намерено: {target_device.name} [{target_device.address}]. Свързване...")

            async with BleakClient(target_device) as client:
                print("Връзката е успешна!")

                rx_char = None
                tx_char = None

                for service in client.services:
                    for char in service.characteristics:
                        if "write" in char.properties or "write-without-response" in char.properties:
                            rx_char = char.uuid
                        if "notify" in char.properties or "indicate" in char.properties:
                            tx_char = char.uuid

                if not rx_char or not tx_char:
                    print("Грешка: Не бяха намерени нужните канали. Рестартиране...\n")
                    await asyncio.sleep(5)
                    continue

                def notification_handler(sender, data):
                    # Принтираме отговора от Arduino-то (очакваме "OK: Mode X")
                    text = data.decode('utf-8', errors='ignore').strip()
                    print(f"Отговор от Arduino: {text}")

                await client.start_notify(tx_char, notification_handler)
                print("Комуникацията започна!\n")

                # Основен цикъл за изпращане на команди
                while client.is_connected:
                    # Взимаме текущата команда
                    msg = COMMANDS[cmd_index]

                    # Изпращаме я
                    await client.write_gatt_char(rx_char, msg.encode('utf-8'))
                    print(f"Изпратено: {msg.strip()}")

                    # Преминаваме към следващата команда за следващия цикъл
                    cmd_index = (cmd_index + 1) % len(COMMANDS)

                    # Чакаме 10 секунди, за да се насладим на ефекта
                    await asyncio.sleep(10)

        except asyncio.CancelledError:
            break
        except Exception as e:
            print(f"\n[!] Връзката падна или възникна грешка: {e}")
            print("Опит за възстановяване след 5 секунди...\n")
            await asyncio.sleep(5)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nПрограмата е спряна успешно.")

