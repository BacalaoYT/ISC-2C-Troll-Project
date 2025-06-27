Para este codigo seguimos con las mismas modificaciones pero mas simples
esta vez quite los strings y los vectores de rango, reemplazandolos por normales,
separe en funciones aun mas especificas el codigo para que podamos modificarlo de forma
mas sencilla


NOTA: 
En algunos casos el programa solo muestra la pantalal en negro, pero con solo darle un click afuera en la barra de tareas
o en otro lugar se muestra el codigo

Explicacion:
Este sistema es un verificador de codigos de barras EAN que trabaja con dos formatos principales EAN-8 y EAN-13,
primero lee los codigos desde un archivo de entrada txt donde cada linea contiene un codigo a validar,
luego analiza cada uno para determinar si es EAN-8 u EAN-13 segun su cantidad de digitos, 
completa con ceros a la izquierda cuando es necesario para alcanzar la longitud requerida.

Sistema para identificar el digito de control
El sistema calcula el digito de control usando un algoritmo especial que alterna multiplicaciones por 3 y por 1 entre los digitos, 
para los codigos EAN-13 validos ademas identifica el pais de origen examinando los primeros tres digitos y comparandolos con rangos preestablecidos en el programa.

Angel Emmanuel Rodriguez Diaz.
339924