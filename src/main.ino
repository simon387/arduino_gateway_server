#include <WiFiS3.h>
#include "secrets.h" // Include il file con le credenziali

// Definizione porta e pin
WiFiServer server(80);
const int relayPin = 7;  // Pin collegato al rel&egrave;
const int pulseTime = 100;  // Durata dell'impulso in millisecondi

void setup() {
  // Imposta la seriale a 9600 baud per maggiore compatibilità
  Serial.begin(9600);

  // Attendi che la seriale sia disponibile (importante per il debug)
  delay(3000);
  Serial.println("======================================");
  Serial.println("Arduino UNO R4 WiFi - REST to Relay");
  Serial.println("======================================");

  // Configurazione del pin del rel&egrave;
  Serial.println("[SETUP] Configurazione pin del rel&egrave;...");
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Inizializza il rel&egrave; spento
  Serial.println("[SETUP] Pin rel&egrave; configurato come LOW");

  // Connessione alla rete WiFi
  Serial.println("[WIFI] Inizializzazione WiFi...");
  Serial.print("[WIFI] Tentativo di connessione a: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) {  // Timeout dopo 10 secondi
      Serial.println();
      Serial.println("[WIFI] Connessione fallita. Riavvio...");
      delay(1000);
      setup();  // Riavvia tutto il processo
      return;
    }
  }

  Serial.println();
  Serial.println("[WIFI] Connessione stabilita!");
  Serial.print("[WIFI] Indirizzo IP Arduino: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WIFI] Potenza segnale: ");
  Serial.println(WiFi.RSSI());

  // Avvia il server
  Serial.println("[SERVER] Avvio del server web...");
  server.begin();
  Serial.println("[SERVER] Server avviato con successo");

  // Informazioni sul funzionamento
  Serial.println("======================================");
  Serial.println("[INFO] Per attivare il rel&egrave;, usa questo URL:");
  Serial.print("[INFO] http://");
  Serial.print(WiFi.localIP());
  Serial.println("/trigger");
  Serial.println("======================================");
}

void loop() {
  // Verifica periodicamente lo stato della connessione WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Connessione persa. Riconnessione...");
    setup();
    return;
  }

  // Controlla se ci sono client connessi
  WiFiClient client = server.available();

  if (client) {
    Serial.println("[CLIENT] Nuovo client connesso");
    Serial.print("[CLIENT] Indirizzo remoto: ");
    IPAddress clientIP = client.remoteIP();
    Serial.println(clientIP);

    // Variabile per memorizzare la richiesta HTTP
    String currentLine = "";
    String requestLine = "";
    bool firstLine = true;

    // Inizializza un timer per il timeout
    unsigned long connectionTime = millis();

    // Continua finché il client è connesso
    while (client.connected() && millis() - connectionTime < 2000) {
      if (client.available()) {
        char c = client.read();

        // Registra la prima riga della richiesta HTTP
        if (firstLine) {
          if (c == '\n') {
            firstLine = false;
            Serial.print("[HTTP] Richiesta: ");
            Serial.println(requestLine);
          } else if (c != '\r') {
            requestLine += c;
          }
        }

        if (c == '\n') {
          // Se la linea corrente è vuota, significa che è la fine della richiesta HTTP
          if (currentLine.length() == 0) {
            Serial.println("[HTTP] Fine dell'intestazione della richiesta");

            // Ottieni l'indirizzo IP attuale
            IPAddress arduinoIP = WiFi.localIP();
            String ipString = String(arduinoIP[0]) + "." + String(arduinoIP[1]) + "." +
                              String(arduinoIP[2]) + "." + String(arduinoIP[3]);

            // Verifica se la richiesta contiene "trigger"
            if (requestLine.indexOf("GET /trigger") >= 0) {
              Serial.println("[RELAY] Attivazione rel&egrave; richiesta");

              // Invia un impulso al pin del rel&egrave;
              Serial.println("[RELAY] Impostazione pin a HIGH");
              digitalWrite(relayPin, HIGH);

              Serial.print("[RELAY] Attesa di ");
              Serial.print(pulseTime);
              Serial.println(" ms");
              delay(pulseTime);

              Serial.println("[RELAY] Impostazione pin a LOW");
              digitalWrite(relayPin, LOW);

              Serial.println("[RELAY] Impulso completato");
            } else {
              Serial.println("[HTTP] Richiesta pagina principale");
            }

            // Invia una risposta HTTP standard
            Serial.println("[HTTP] Invio risposta HTTP...");
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Invia la pagina HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Arduino Relay Control</title>");
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
            client.println(".info-box { background-color: #f0f0f0; padding: 10px; border-radius: 5px; margin-bottom: 15px; }");
            client.println(".success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }");
            client.println("</style>");
            client.println("</head><body>");

            // Aggiungi box con indirizzo IP
            client.println("<div class='info-box'>");
            client.println("<strong>Indirizzo IP Arduino:</strong> " + ipString);
            client.println("<br><strong>URLs disponibili:</strong>");
            client.println("<br>- <a href='/'>/</a> - Questa pagina");
            client.println("<br>- <a href='/trigger'>/trigger</a> - Attiva il rel&egrave;");
            client.println("</div>");

            if (requestLine.indexOf("GET /trigger") >= 0) {
              client.println("<div class='info-box success'>");
              client.println("<h2>Comando inviato con successo</h2>");
              client.println("<p>Il rel&egrave; sul pin " + String(relayPin) + " &egrave; stato attivato per " + String(pulseTime) + " ms</p>");
              client.println("</div>");
            } else {
              client.println("<h1>Arduino UNO R4 WiFi - REST Controller</h1>");
              client.println("<p>Per attivare il rel&egrave;, clicca sul pulsante qui sotto o visita <a href='/trigger'>/trigger</a></p>");
              client.println("<button onclick='location.href=\"/trigger\"' style='padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer;'>Attiva rel&egrave;</button>");
            }

            client.println("</body></html>");
            Serial.println("[HTTP] Risposta inviata");

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          // Aggiungi il carattere alla linea corrente
          currentLine += c;
        }
      }
    }

    // Chiudi la connessione
    client.stop();
    Serial.println("[CLIENT] Client disconnesso");
    Serial.println("======================================");
  }
}