#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// --- Configuración de pines ---
// GPS: RX=4, TX=3  (Arduino recibe en 4, transmite en 3)
// SIM800L: RX=8, TX=9
SoftwareSerial gpsSerial(4, 3);
SoftwareSerial gsmSerial(8, 9);

// Objeto TinyGPS++ para parsear NMEA
TinyGPSPlus gps;

// Configuración del servidor
const char APN[]     = "datos.personal.com"; // APN de tu operador
const char SERVER[]  = "tu_servidor.com";    // dominio o IP (ej: 192.168.0.10)
const int  PORT      = 5000;                 // puerto de Flask
const char PATH[]    = "/coordenadas";       // endpoint

void setup() {
  Serial.begin(9600);    
  gpsSerial.begin(9600); 
  gsmSerial.begin(9600); 
  
  Serial.println("Iniciando tracker GPS → servidor web (GET)...");
  
  initGSM();
}

void loop() {
  // Leer datos del GPS
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Si tenemos ubicación válida
  if (gps.location.isUpdated()) {
    double lat = gps.location.lat();
    double lon = gps.location.lng();

    Serial.print("Lat: "); Serial.println(lat, 6);
    Serial.print("Lon: "); Serial.println(lon, 6);

    // Enviar al servidor vía SIM800L
    sendToServer(lat, lon);

    delay(10000); // esperar 10 segundos antes del próximo envío
  }
}

// --- Inicializar GSM y configurar APN ---
void initGSM() {
  Serial.println("Configurando GSM...");

  sendCommand("AT", "OK", 2000);
  sendCommand("AT+CSQ", "OK", 2000);        
  sendCommand("AT+CREG?", "OK", 2000);      
  sendCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 2000);
  sendCommand("AT+SAPBR=3,1,\"APN\",\"" + String(APN) + "\"", "OK", 2000);
  sendCommand("AT+SAPBR=1,1", "OK", 3000);  
  sendCommand("AT+SAPBR=2,1", "OK", 2000);  
}

// --- Enviar coordenadas al servidor con GET ---
void sendToServer(double lat, double lon) {
  // Construimos la URL con parámetros
  String url = "http://" + String(SERVER) + ":" + String(PORT) + String(PATH)
               + "?lat=" + String(lat, 6) + "&lon=" + String(lon, 6);

  Serial.println("Enviando a: " + url);

  sendCommand("AT+HTTPTERM", "OK", 2000); 
  sendCommand("AT+HTTPINIT", "OK", 2000);
  sendCommand("AT+HTTPPARA=\"CID\",1", "OK", 2000);
  sendCommand("AT+HTTPPARA=\"URL\",\"" + url + "\"", "OK", 2000);

  sendCommand("AT+HTTPACTION=0", "200", 10000); // 0 = GET
  sendCommand("AT+HTTPREAD", "OK", 5000);
  sendCommand("AT+HTTPTERM", "OK", 2000);
}

// --- Helper: enviar comandos AT ---
bool sendCommand(String cmd, String ack, unsigned long timeout) {
  gsmSerial.println(cmd);
  Serial.println(">> " + cmd);
  return waitForResponse(ack, timeout);
}

bool waitForResponse(String ack, unsigned long timeout) {
  unsigned long start = millis();
  String resp = "";
  while (millis() - start < timeout) {
    while (gsmSerial.available()) {
      char c = gsmSerial.read();
      resp += c;
    }
    if (resp.indexOf(ack) != -1) {
      Serial.println("<< " + resp);
      return true;
    }
  }
  Serial.println("Timeout esperando: " + ack);
  return false;
}
