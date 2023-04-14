#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_NUMBERS 100 // número máximo de números generados y almacenados


def 


int main() {
    int numbers[MAX_NUMBERS]; // números generados
    time_t timestamps[MAX_NUMBERS]; // tiempos de generación
    int num_numbers = 0; // número actual de números generados y almacenados

    time_t time_last_generated = time(NULL); // tiempo de generación del último número
    int already_generated = 0;

    while (1) { // ciclo infinito para generar números hasta que se encuentre uno válido
        int number = rand(); // generar un número aleatorio

    }

    if (!already_generated) { // si el número no ha sido generado previamente, almacenarlo y retornarlo
        
        return number;
        

        
        // actualizar el tiempo de generación del último número generado
        time_last_generated = timestamps[num_numbers];
        num_numbers++;
        

        // imprimir el número generado y el tiempo de generación para verificar que todo funciona correctamente
        printf("Número generado: %d\n", number);
        printf("Tiempo de generación: %ld\n", timestamps[num_numbers-1]);
    }


    // // comprobar si han pasado más de 10 minutos desde el último número generado y, en ese caso, actualizar el tiempo de generación
    // if (time(NULL) - time_last_generated >= 600) {
    //   time_last_generated = time(NULL);
    // }

  return 0;
}





