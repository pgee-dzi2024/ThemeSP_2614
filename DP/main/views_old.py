from django.shortcuts import render
from django.http import JsonResponse
import json
import asyncio
from bleak import BleakClient

# ВЪВЕДИ ТУК MAC адреса на твоя Bluetooth модул!
MAC_ADDRESS = "33:B2:C3:09:4F:C0"

# Това е стандартният UUID за серийна комуникация при 99% от BLE клонингите (като твоя)
CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"


async def send_ble_command(command_str):
    """Асинхронна функция за изпращане на команда през BLE"""
    try:
        # Свързваме се с модула
        async with BleakClient(MAC_ADDRESS, timeout=5.0) as client:
            # Изпращаме данните (кодирани в байтове) към специфичната GATT характеристика
            await client.write_gatt_char(CHAR_UUID, command_str.encode('utf-8'))
            return True, "Командата е изпратена успешно"
    except Exception as e:
        return False, str(e)


def index(request):
    """Рендерира главната страница (Vue.js интерфейса)"""
    return render(request, 'main/index.html')  # Провери дали пътят до темплейта съвпада с твоя


def control_led(request):
    """API endpoint за приемане на команди от фронтенда"""
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            mode = int(data.get('mode', 0))
            r = int(data.get('r', 0))
            g = int(data.get('g', 0))
            b = int(data.get('b', 0))
            speed = int(data.get('speed', 30))

            # Валидация на стойностите
            r = max(0, min(255, r))
            g = max(0, min(255, g))
            b = max(0, min(255, b))
            speed = max(5, speed)

            # Форматираме стринга за Arduino (задължително с \n накрая)
            command = f"{mode},{r},{g},{b},{speed}\n"

            # Тъй като Django е синхронен (по подразбиране), а bleak е асинхронен,
            # използваме asyncio.run(), за да изпълним комуникацията
            success, msg = asyncio.run(send_ble_command(command))

            if success:
                return JsonResponse({'status': 'success', 'message': msg})
            else:
                return JsonResponse({'status': 'error', 'message': f'Bluetooth грешка: {msg}'})

        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)})

    return JsonResponse({'status': 'error', 'message': 'Невалиден HTTP метод'})