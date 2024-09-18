#include <SPI.h>
#include <Ethernet.h>

const int pinBoton = 2;             // Pin al que está conectado el botón
unsigned long tiempoRetardo = 200;  // Tiempo en milisegundos para validar el botón presionado
int estadoBoton;                    // Estado actual del botón
int ultimoEstadoBoton = HIGH;       // Estado anterior del botón (inicialmente no presionado)
unsigned long ultimoTiempoCambio = 0;  // Tiempo en que se cambió el estado del botón

// Configuración de red
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Dirección MAC del módulo Ethernet (puedes personalizarla)
IPAddress ip(192, 168, 100, 177);                       // Dirección IP estática del Arduino
IPAddress server(192, 168, 100, 21);                   // Dirección IP del servidor (cambia por tu servidor)

EthernetClient client;

void setup() {
  // Configurar el pin del botón con la resistencia pull-up interna
  pinMode(pinBoton, INPUT_PULLUP);

  // Iniciar la comunicación serie para la depuración
  Serial.begin(9600);

  // Iniciar Ethernet
  Serial.println("Inicializando Ethernet...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Fallo al configurar Ethernet usando DHCP, configurando IP estática...");
    Ethernet.begin(mac, ip);  // Usar la IP estática si no se puede usar DHCP
  }

  // Esperar a que se conecte a la red
  delay(1000);
  Serial.print("Dirección IP asignada: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Leer el estado del botón
  int lectura = digitalRead(pinBoton);

  // Si el estado del botón ha cambiado
  if (lectura != ultimoEstadoBoton) {
    // Reiniciar el tiempo de debounce
    ultimoTiempoCambio = millis();
  }

  // Verificar si han pasado más de 200 ms desde el último cambio de estado
  if ((millis() - ultimoTiempoCambio) > tiempoRetardo) {
    // Si el estado del botón ha cambiado después del debounce, actualizamos el estado
    if (lectura != estadoBoton) {
      estadoBoton = lectura;

      // Si el botón está presionado (LOW, porque usamos pull-up)
      if (estadoBoton == LOW) {
        Serial.println("Botón presionado, enviando petición GET...");

        // Enviar petición GET
        enviarPeticionGET();
      }
    }
  }

  // Guardar el estado actual como el último estado
  ultimoEstadoBoton = lectura;
}

void enviarPeticionGET() {
  if (client.connect(server, 5555)) {  // Conectar al servidor en el puerto 5555
    Serial.println("Conectado al servidor");

    // Enviar la solicitud HTTP GET
    client.println("GET /leer_placa HTTP/1.1");
    client.println("Host: localhost");
    client.println("Connection: close");
    client.println();  // Línea en blanco para finalizar la petición

    // Esperar una respuesta del servidor
    while (client.connected()) {
      if (client.available()) {
        String linea = client.readStringUntil('\r');
        Serial.print(linea);
      }
    }

    // Cerrar la conexión
    client.stop();
    Serial.println("\nDesconectado del servidor");
  } else {
    Serial.println("Error: No se pudo conectar al servidor");
  }
}