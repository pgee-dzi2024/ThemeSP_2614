import serial
from django.shortcuts import render
from rest_framework.decorators import api_view
from rest_framework.response import Response

# Опитваме се да отворим порта глобално при стартиране на сървъра.
# ВНИМАНИЕ: Промени 'COM3' на порта, който виждаш в Arduino IDE!
try:
    arduino_port = serial.Serial('COM3', 9600, timeout=1)
    print("Успешна връзка с Arduino!")
except Exception as e:
    arduino_port = None
    print(f"Грешка при отваряне на COM порта: {e}")


def index(request):
    """ Това е изгледът за главната страница, където ще живее Vue.js """
    return render(request, 'main/index.html')


@api_view(['POST'])
def control_led(request):
    """ API endpoint, който приема команди от фронтенда и ги праща към Arduino """
    if not arduino_port or not arduino_port.is_open:
        return Response({'status': 'error', 'message': 'Няма връзка с хардуера (Arduino).'}, status=503)

    try:
        # Взимаме данните от POST заявката (с дефолтни стойности, ако липсват)
        mode = int(request.data.get('mode', 0))
        r = int(request.data.get('r', 0))
        g = int(request.data.get('g', 0))
        b = int(request.data.get('b', 0))
        speed = int(request.data.get('speed', 30))

        # Валидираме данните, за да сме сигурни, че няма да пратим глупости
        r, g, b = [max(0, min(255, val)) for val in (r, g, b)]
        speed = max(5, speed)  # Минимално закъснение 5ms

        # Форматираме стринга точно както го очаква Arduino-то: РЕЖИМ,R,G,B,СКОРОСТ\n
        command_str = f"{mode},{r},{g},{b},{speed}\n"

        # Изпращаме стринга по серийния кабел
        arduino_port.write(command_str.encode('utf-8'))

        return Response({
            'status': 'success',
            'message': 'Командата е изпратена успешно',
            'command_sent': command_str.strip()
        })

    except ValueError:
        return Response({'status': 'error', 'message': 'Невалиден формат на данните.'}, status=400)
    except Exception as e:
        return Response({'status': 'error', 'message': str(e)}, status=500)