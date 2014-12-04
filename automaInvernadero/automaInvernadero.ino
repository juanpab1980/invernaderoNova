#include <DHT22.h>
#include <SPI.h>
#include <Ethernet.h>
#define TEMPMAXIMA  30
#define TEMPMINIMA  20

// Mac unica de cada EthernetShield (deben cambiarla)

byte mac[]={0x90,0xA2,0xDA,0x0F,0x09,0xD5}; //MAC
IPAddress ip(192,168,1,72); //IP
EthernetServer servidor(80);

// Inicializa la instancia client
EthernetClient client;

// Direccion del servidor
char server[] = "192.168.1.77";

// Variable de tiempo de la ultima conexion en milisegundos
unsigned long ultimaConexion = 0;
// Estado de la ultima conexion
boolean ultimoEstado = false;
// Intervalo en milisegundos entre conexiones
const unsigned long intervaloConexion = 5000;

int RHT03_PIN=2;
DHT22 myRHT03(RHT03_PIN);

int RHT03_PIN2=3;
DHT22 sensor2(RHT03_PIN2);

int RHT03_PIN3=5;
DHT22 sensor3(RHT03_PIN3);

int extractor1 = 8;
int extractor2 = 9;

int temPromedio = 0;

void setup() {
      // Inicializa puerto serial
      Serial.begin(9600);
      Serial.println("#### AUTOMATIZACION DE INVERNADERO ####");
      Serial.println("Sensor temperatura y humedad relativa RHT03");
      // Espera 1 segundo para que se inicie la tarjeta Ethernet
      delay(1000);
      // Configuración de los puertos de entrada para los sensores
      pinMode(RHT03_PIN,INPUT);
      pinMode(RHT03_PIN2,INPUT);
      pinMode(RHT03_PIN3,INPUT);
      
      // Configuracion  de los puertos de salida para control de los extractores
      pinMode(extractor1, OUTPUT);
      digitalWrite(extractor1, LOW);
      pinMode(extractor2, OUTPUT);
       digitalWrite(extractor2, LOW);
      
      Ethernet.begin(mac, ip);
      servidor.begin();
      
      // Imprime la direccion IP de la tarjeta
      Serial.print("Direccion IP: ");
      Serial.println(Ethernet.localIP());  
}



// Loop principal
void loop() {
      delay(60000);  // Retardo de 1 min
      myRHT03.readData();
      sensor2.readData();
      sensor3.readData();
      
      /*char buf[128];
      sprintf(buf, ": Temperatura %hi.%01hi C, Humedad Relativa %i.%01i %%",
                       myRHT03.getTemperatureCInt()/10, abs(myRHT03.getTemperatureCInt()%10),
                       myRHT03.getHumidityInt()/10, myRHT03.getHumidityInt()%10);
      */
  
      // Si hay datos que llegan por la conexion los envia a la puerta serial
      if (client.available()) {
           char c = client.read();
           Serial.print(c);
      }

      // Si no hay conexion de red y se conecto correctamente la ultima vez
      // detiene el cliente Ehternet
      if (!client.connected() && ultimoEstado) {
          Serial.println();
          Serial.println("Desconectando...");
          client.stop();
      }
      if(!client.connected() && (millis() - ultimaConexion > intervaloConexion)) {
           httpRequest();
      }
      // Actualiza la variable ultimoEstado
      ultimoEstado = client.connected();
}


// Realiza la conexion http al servidor
void httpRequest() {
      // Se conecta al servidor en el puerto 80 (web)
     if (client.connect(server, 80)) {
          // Envia los datos de los sensores al puerto serial para monitorear
          // Sensor invernadero interno
          Serial.print("Temperatura sensor 1: ");
          Serial.print(myRHT03.getTemperatureC());
          Serial.println(" grados Celsius");
          Serial.print("Humedad sensor 1: ");
          Serial.print(myRHT03.getHumidity());
          Serial.println(" %");
          
                   
          // Sensor invernadero puerta
          Serial.print("Temperatura sensor 2: ");
          Serial.print(sensor2.getTemperatureC());
          Serial.println(" grados Celsius");
          Serial.print("Humedad sensor 2: ");
          Serial.print(sensor2.getHumidity());
          Serial.println(" %");
          
          
          // Sensor externo
          Serial.print("Temperatura sensor externo: ");
          Serial.print(sensor3.getTemperatureC());
          Serial.println(" grados Celsius");
          Serial.print("Humedad sensor 2: ");
          Serial.print(sensor3.getHumidity());
          Serial.println(" %");
    
    
          // Envia el requerimiento al servidor via GET
          Serial.println("Iniciando conexion...");
         //Sensor Interno
            client.print("GET /sensorarduino.php?Temp1=");
          client.print(myRHT03.getTemperatureC());     
          client.print("&Hum1=");
          client.print(myRHT03.getHumidity()); 
          //Sensor puerta
           client.print("&Temp2=");
           client.print(sensor2.getTemperatureC()); 
           client.print("&Hum2=");
          client.print(sensor2.getHumidity());
          
          //sensor externo
           client.print("&Temp3=");
           client.print(sensor3.getTemperatureC()); 
           client.print("&Hum3=");
          client.print(sensor3.getHumidity());
          
                    
          client.println(" HTTP/1.1");
          client.print("Host: ");
          client.println(server);
          client.println("User-Agent: Arduino-Ethernet");
          client.println("Connection: close");
          client.println();

        
          // Actualiza el tiempo en milisegundos de la ultima conexion
          ultimaConexion = millis();
   client.stop();  
   }
     else {
         // Si la conexion fallo se desconecta
         Serial.println("Error al conectarse al servidor");
         Serial.println("Desconectando...");
         client.stop();
    }
    
    // Calcular la temperatura promedio 
    temPromedio = (sensor2.getTemperatureC()+myRHT03.getTemperatureC())/2;
    
    
    // Si la temperatura excede el maximo estipulado
    if (myRHT03.getTemperatureC() >= TEMPMAXIMA) {      
        digitalWrite(extractor1, HIGH);  
    }
    
    else 
        digitalWrite(extractor1, LOW); 
    
}

