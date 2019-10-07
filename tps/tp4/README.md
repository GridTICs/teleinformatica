# Descripcion breve de funciones del programa.

Este programa esta orientado para trabajar sobre la misma plataforma de hardware
del proyecto de Matias Pecchia. El programa se ejecuta sobre un OpenMote. 
Actua en una arquitectura cliente-servidor. El Mote actua como servidor 
esperando peticiones de un cliente. El Mote espera peticiones en el puerto 3000.
Desde el cliente se elige entre un numero limitado de tareas que puede realizar
el servidor. En las pruebas realizadas el cliente se genero desde una PC con 
linux mediante el uso de la consola con el comando "nc".

Ejemplo:	nc -6 -u <Direccion IPv6 Mote> 3000
			get /all(Ctrl+D) 

## Uso del programa, datos de entrada, opciones y resultado.

El cliente envia al puerto 3000 del mote una de las siguientes instrucciones o 
cadenas de caracteres:

	get /all
	get /tem
	get /lgh
	get /hum
	get /acc

	Nota: 	Cabe aclarar que estas cadenas deben ser enviadas con la combinacion
			de teclas "Ctrl+D", NO con la tecla ENTER.

El mote al recibir la petición comenzara a enviar datos segun la opcion elegida.

get /all: Envia las mediciones de todos los sensores.
get /tem: Envia solamente las mediciones de temperatura.
get /lgh: Envia solamente las mediciones de luz.
get /hum: Envia solamente las mediciones de humedad.
get /acc: Envia solamente las mediciones correspondiente a la acceleraciones.

Todos estos datos seran enviados por el Mote cada 15 segundos que es el valor 
por defecto del tiempo. A su vez el cliente puede configurar este tiempo 
mediante la siguiente instrucción:

	set /XXX
	
	Nota:	En lugar de las "XXX" usted deberia completar con números lo que 
			corresponderia con el tiempo en segundos. La instrucción no acepta
			el valor 000, si usted envia el valor antes mencionado el valor del
			se ajustara a su valor por defecto, o sea, 15 segundos.
			Ejemplo: Si usted envia "set /295", los lecturas de los sensores
			seran enviadas cada 295 segundos.

	Nota: 	Cabe aclarar que estas cadenas deben ser enviadas con la combinacion
			de teclas "Ctrl+D", NO con la tecla ENTER.

Otro tema de para aclarar es que si usted establece el tiempo pero no elige una 
tarea (como get /all, o get /tem) el programa le informara de que hay un error 
de protocolo. Usted deberia establecer primero que lectura de sensor quiere y 
luego deberia especificar el tiempo.

La ultima opción que posee el cliente es enviar la siguiente instrucción:

	get /out

	Nota: 	Cabe aclarar que estas cadenas deben ser enviadas con la combinacion
			de teclas "Ctrl+D", NO con la tecla ENTER.

Esta opción liberara la conexión y pondra al mote en estado de espera de un 
nuevo cliente. 
