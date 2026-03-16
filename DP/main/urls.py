from django.urls import path
from . import views
from django.conf import settings
from django.conf.urls.static import static

urlpatterns = [
    # Главна страница с уеб интерфейса
    path('', views.index, name='index'),

    # API endpoint за управление на лентата
    path('api/control-led/', views.control_led, name='control_led'),
]

if settings.DEBUG:
    urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)