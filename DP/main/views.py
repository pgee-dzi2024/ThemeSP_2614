import json
import asyncio
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from bleak import BleakScanner, BleakClient
from django.shortcuts import render

TARGET_NAME = "HC-05"


async def send_ble_command_async(command_str):
    """Помощна асинхронна функция за изпращане на команда през BLE"""
    try:
        # Търсене на устройството (timeout 3 секунди)
        devices = await BleakScanner.discover(timeout=3.0)
        target_device = next((d for d in devices if d.name and TARGET_NAME in d.name), None)

        if not target_device:
            return False, "Устройството HC-05 не е намерено в обсег."

        async with BleakClient(target_device) as client:
            rx_char, tx_char = None, None

            for service in client.services:
                for char in service.characteristics:
                    if "write" in char.properties or "write-without-response" in char.properties:
                        rx_char = char.uuid
                    if "notify" in char.properties or "indicate" in char.properties:
                        tx_char = char.uuid

            if not rx_char or not tx_char:
                return False, "Не са намерени нужните BLE канали."

            response_event = asyncio.Event()
            response_text = ""

            def notification_handler(sender, data):
                nonlocal response_text
                response_text = data.decode('utf-8', errors='ignore').strip()
                if "OK" in response_text:
                    response_event.set()

            await client.start_notify(tx_char, notification_handler)
            await client.write_gatt_char(rx_char, command_str.encode('utf-8'))

            try:
                await asyncio.wait_for(response_event.wait(), timeout=2.0)
                await client.stop_notify(tx_char)
                return True, response_text
            except asyncio.TimeoutError:
                await client.stop_notify(tx_char)
                return False, "Командата е изпратена, но няма отговор от Arduino."

    except Exception as e:
        return False, f"Грешка при BLE комуникация: {str(e)}"


# ВРЪЩАМЕ ИЗГЛЕДА ДА Е СИНХРОНЕН (без async)
@csrf_exempt
def control_led(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            mode = int(data.get('mode', 0))
            r = int(data.get('r', 0))
            g = int(data.get('g', 0))
            b = int(data.get('b', 0))
            speed = int(data.get('speed', 30))

            command_str = f"{mode},{r},{g},{b},{speed}\n"

            # Използваме asyncio.run(), за да изпълним асинхронната функция вътре в синхронния изглед!
            success, msg = asyncio.run(send_ble_command_async(command_str))

            if success:
                return JsonResponse({'status': 'success', 'message': msg})
            else:
                return JsonResponse({'status': 'error', 'message': msg}, status=503)

        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)}, status=400)

    return JsonResponse({'status': 'error', 'message': 'Invalid request method'}, status=405)

def index(request):
    """Рендерира главната страница (Vue.js интерфейса)"""
    return render(request, 'main/index.html')  # Провери дали пътят до темплейта съвпада с твоя

