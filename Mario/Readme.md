Mis correcciones en el codigo de Mario

Este es el codigo corregido de acuerdo a las indicaciones que me dio la maestra Liz

Cambie el for de rango a un for tradicional como me indico ya que no comprendia la logica del anterior.

Cambie el uso de la libreria vectores y use arreglos tradicionales tipo char, 
ademas quite el uso total de strings ya que no estaban permitidos

El codigo de los saltos de mario funciona asi
Se lee cada escenario con alturas de muros usando una funcion recursiva
Si el siguiente muro es mas alto, cuenta como salto arriba, si es mas bajo, como salto abajo

Mario avanza muro por muro usando interpolacion lineal para moverse suavemente, 
la posicion de mario es determinada por mario_x y mario_y cuya funcion es llegar a el punto de destino (x,y)

El resultado se muestra y Mario se mueve automaticamente por los muros

Con la nueva correccion las nubes se gestionan mediante un arreglo de estructuras como me habian sugerido,
donde cada nube tiene su posicion y velocidad, en cada ciclo del temporizador, 
el programa recorre todas las nubes y actualiza su posicion horizontal sumando su velocidad

Sobre las funciones que permiten el movimiento
Hay una funcion que se llama cada vez que el temporizador se activa, esta funcion es la que actualiza todo en pantalla
Dentro de esa funcion, primero se revisa si Mario esta en salto, si es asi se calcula cuanto le falta para llegar al siguiente muro y se le suma una parte de esa distancia a mario_x y mario_y para que avance poco a poco
Cuando Mario llega al destino, se actualizan las variables para que se prepare el siguiente salto
Para las nubes, en esa misma funcion se recorre el arreglo de nubes y a cada una se le suma su velocidad a la posicion x, asi parece que se mueven solas por el cielo
Todo esto pasa en cada ciclo del temporizador, por eso todo se mueve al mismo tiempo y le da ese toque fluido

Angel Emmanuel Rodriguez Diaz.
339924