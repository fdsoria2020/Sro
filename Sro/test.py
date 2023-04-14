from GUI import GUI
from HAL import HAL
import cv2
import numpy as np

KP = 0.5
KI = 0.015
KD = 0.2

# Definimos el tiempo de muestreo
delta_time = 0.05 # segundos

# Definimos las variables para el cálculo del error
last_error_a = 0.0
integral_a = 0.0


def angular_speed(error):
    global KD, KI, KP, last_error_a, integral_a, delta_time

    # Calculamos la parte proporcional
    proportional = error * KP

    # Calculamos la parte integral
    integral_a += error * delta_time
    integral_a = integral_a * KI

    # Calculamos la parte derivativa
    derivative = (error - last_error_a) / delta_time * KD

    # El error actual se convierte en el anterior
    last_error_a = error
    
    # Sumamos las tres partes para obtener la salida
    output = proportional + integral_a + derivative

    # Limitamos la salida a un rango máximo y mínimo
    output = max(min(output, 1.0), -1.0)
    
    return output


def get_image():
    
    img  = HAL.getImage()
    img_hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # Se genera una mascara menor y mayor de rojo
    mask1 = cv2.inRange(img_hsv, (0,50,20), (5,255,255))
    mask2 = cv2.inRange(img_hsv, (175,50,20), (180,255,255))
    
    # Se unen las mascaras y se recorta la imagen para que solo aparezca componentes rojas
    mask = cv2.bitwise_or(mask1, mask2 )
    croped = cv2.bitwise_and(img, img, mask=mask)
    
    # Se obtienen las dimensiones de la imagen y se dibuja un circulo en su centro
    alto, ancho, _ = croped.shape
    centro_x = (ancho // 2) +11 #11 es la distancia entre este centro y el centro de la linea
    centro_y = (alto // 2)  +159 #159 es la distancia entre este centro y el centro de la linea
    cv2.circle(croped, (centro_x, centro_y), 5, (0, 255, 0), -1)
    
    # Se encuentra los contornos de la linea roja para dibujar un circulo en el centro de esta
    contornos, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    mayor_contorno = max(contornos, key=cv2.contourArea)
    momentos = cv2.moments(mayor_contorno)
    centro_linea_x = int(momentos["m10"] / momentos["m00"])
    centro_linea_y = int(momentos["m01"] / momentos["m00"])
    cv2.circle(croped, (centro_linea_x, centro_linea_y), 5, (255, 0, 0), -1) # Centro de la linea
    
    return croped, centro_linea_x

def set_speed(centro_linea_x):
    # Se calcula la diferencia entre el centro de la imagen y el centro de la linea y se normaliza
    error = centro_linea_x - (img.shape[1] / 2)
    ang_vel = -float(error) / (img.shape[1] / 2)
    
    # Se calcula la velocidad angular
    w = angular_speed(ang_vel)
    HAL.setW(w)

    # Se calcula la velocidad lineal
    if abs(w) <= 0.30:
        HAL.setV(7.5)
    else:
        HAL.setV(2)
        

while True:
    img, centro_linea_x = get_image()
    GUI.showImage(img)
    set_speed(centro_linea_x)

    
    