import serial
from django.shortcuts import render
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt  # ДОБАВЕНО: За изключване на CSRF защитата
import json
import time

arduino_port = None


def get_serial_connection():
    global arduino_port
    if arduino_port is None or not arduino_port.is_open:
        try:
            arduino_port = serial.Serial('COM5', 9600, timeout=1)
            time.sleep(1) # Даваме време на Bluetooth модула да се "осъзнае" след отварянето
            print("Успешна връзка с Bluetooth на COM5!")
        except Exception as e:
            print(f"Грешка: {e}")
            arduino_port = None
    return arduino_port


def index(request):
    return render(request, 'main/index.html')


@csrf_exempt  # ДОБАВЕНО: Позволява на Vue.js (Axios) да праща POST заявки свободно
def control_led(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            mode = int(data.get('mode', 0))
            r = int(data.get('r', 0))
            g = int(data.get('g', 0))
            b = int(data.get('b', 0))
            speed = int(data.get('speed', 30))

            r = max(0, min(255, r))
            g = max(0, min(255, g))
            b = max(0, min(255, b))
            speed = max(5, speed)

            command = f"{mode},{r},{g},{b},{speed}\n"

            conn = get_serial_connection()
            conn = get_serial_connection()
            if conn:
                print(f"Опит за изпращане на: {command.strip()}")  # Да видим в конзолата какво пращаме

                # Изпращаме и ЗАДЪЛЖИТЕЛНО изчистваме буфера!
                conn.write(command.encode('ascii'))  # ascii е по-сигурно за числа
                conn.flush()  # Това принуждава Windows да изпрати данните ВЕДНАГА

                print("Успешно изпратено и флъшнато!")
                return JsonResponse({'status': 'success'})
            else:
                return JsonResponse({'status': 'error', 'message': 'Няма връзка с COM5'})

        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)})

    return JsonResponse({'status': 'error', 'message': 'Невалиден метод'})