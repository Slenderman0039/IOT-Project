#include <Arduino.h>
#include <WiFi.h>

#define A 2
#define B 15
#define C 16
#define D 17
#define E 18
#define F 4
#define G 5
#define BUTTON 27

WiFiServer server(80);

const char* ssid = "Vodafone-F";
const char* password = "famigliacama2020";

int startTimer = 0; //SALVA L'INIZIO DEL TIMER PER IL RIAVVIO
int timer = 0; //TEMPO COUNTDOWN
bool flag = false; //FLAG ATTIVAZIONE COUNTDOWN

void setPin(){
    pinMode(BUTTON, INPUT);
    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(C, OUTPUT);
    pinMode(D, OUTPUT);
    pinMode(E, OUTPUT);
    pinMode(F, OUTPUT);
    pinMode(G, OUTPUT);
}

void setup() {
  Serial.begin(115200);
  setPin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

const long timeoutTime = 2000;
String header;
void loop() {

  //The ESP is always listening for incoming clients with this line:
  WiFiClient client = server.available();   // Listen for incoming clients
  //When a request is received from a client, we’ll save the incoming data. 
  if (client) {                             
    Serial.println("Nuovo client registrato!");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    unsigned long currentTime = millis();
    unsigned long previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html>");
            Serial.println(header);
            client.println("<head><link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/uikit@3.8.1/dist/css/uikit.min.css'/><title>Wemos Timer!</title><center><h1>Timer</h1></center></head>");
            String index = header.substring(0,6); //PRELEVO LA SOTTOSTRINGA "GET / " DALLA RICHIESTA
            index.trim(); //SE TOLGO GLI SPAZI VUOTI, LA LUNGHEZZA NON SARA' MAI COME QUELLA DELL'INDEX
            String count = header.substring(10,17);
            String restart = header.substring(10,19);
            Serial.println(restart);
            String timer = header.substring(0,11);
            timer.trim();
            Serial.println("");
            Serial.println(header);
            Serial.println("");
            if(index.length()==5){
            client.println("<script>document.location.href='/timer';</script>");
            } else if (header.indexOf("GET /timer") >= 0 && timer.length()==10) {
              client.println("<body>");
              client.println("<br><center>AMMINISTRATORE<br><div class='uk-margin'><div class='uk-inline'><span class='uk-form-icon' uk-icon='icon: future'></span><input style='text-align:right;' class='uk-input' id='ptext' type='text' placeholder='Timer' required></div></div> <button class='uk-button uk-button-primary' onclick='bclick()'>Avvia</button></center>");
              client.println("<script>function bclick(){var val = document.getElementById('ptext').value; if(val===''){alert('Errore, devi inserire il valore del timer prima di proseguire!');}else{location.href = './timer?count='+val;}}");
              client.println("</script></body>");
            } else if (header.indexOf("GET /timer?count=")>=0 && count.equals("?count=")) {
                int value;
                int pos1 = header.indexOf('=');
                int pos2 = header.indexOf('HTTP');
                value = header.substring(pos1+1, pos2-4).toInt();
                if(value<=9 && value>=1){
                  startTimer = value;
                  setTimer(value,client);
                }else{
                   client.println("<center><div><div class='uk-card uk-card-primary uk-card-hover uk-card-body uk-light'><h2 class='uk-card-title'>Valore non valido</h2><p>Errore valore non valido: Il valore dev'essere compreso tra 9 e 1.</p></div></div></center>");
                   client.println("<script>window.setTimeout(function(){window.location.href = './timer';}, 3000);</script>");
                }
            }else{
              client.println("<center><div><div class='uk-card uk-card-primary uk-card-hover uk-card-body uk-light'><h2 class='uk-card-title'>404</h2><p>Errore 404: Pagina non trovata.</p></div></div></center>");
            }
            client.println("<script src='https://cdn.jsdelivr.net/npm/uikit@3.8.1/dist/js/uikit.min.js'></script><script src='https://cdn.jsdelivr.net/npm/uikit@3.8.1/dist/js/uikit-icons.min.js'></script>");
            client.println("</html>");
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    header = "";
   delay(5000);
    client.stop();
    Serial.println("Client disconnesso.");
    Serial.println("");
  }
}

void setTimer(int value,WiFiClient client){
  long previousMillis = 0;
  long interval = 1000;          
  long currentMillis = millis();
  client.println("<body><center><div id='countdown'></div><br><button class='uk-button uk-button-danger' onclick='onrestart()'>Riavvia</button></center>");
    for(int i=value;i>=-2;--i){ //3*2 --> 6 
      if(digitalRead(BUTTON) == 1){
              i = startTimer;
              client.print("<script>countdown =");
              client.print(startTimer);
              client.print(";</script>");
       }
      delay(1000);
      switch(i){
        case 1:
            do{
                currentMillis = millis();
                if(currentMillis - previousMillis > interval){
                  digitalWrite(A,LOW);
                  digitalWrite(B,HIGH);
                  digitalWrite(C,HIGH);
                  digitalWrite(D,LOW);
                  digitalWrite(E,LOW);
                  digitalWrite(F,LOW);
                  digitalWrite(G,LOW);
                  client.print("<script>var countdown =");
                  client.print(i);
                  client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
               }
              }while(currentMillis-previousMillis < interval+1);
              previousMillis = currentMillis;
        break;
        case 2:
             do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,HIGH);
                    digitalWrite(B,HIGH);
                    digitalWrite(C,LOW);
                    digitalWrite(D,HIGH); 
                    digitalWrite(E,HIGH);
                    digitalWrite(F,LOW);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                }
              }while(currentMillis-previousMillis < interval+1);
              previousMillis = currentMillis;
        break;
        case 3:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,HIGH);
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,LOW);
                    digitalWrite(F,LOW);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                }
              }while(currentMillis-previousMillis < interval+1);
              previousMillis = currentMillis;
        break;
        case 4:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,LOW);
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,LOW);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 5:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,HIGH);
                    digitalWrite(B,LOW);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
               }while(currentMillis-previousMillis < interval+1);
               previousMillis = currentMillis;
        break;
        case 6:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,HIGH);
                    digitalWrite(B,LOW);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,HIGH);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 7:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,HIGH);
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,LOW);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,LOW);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 8:
               do{
                    currentMillis = millis();
                    if(currentMillis - previousMillis > interval){
                      digitalWrite(A,HIGH);
                      digitalWrite(B,HIGH);
                      digitalWrite(C,HIGH);
                      digitalWrite(D,HIGH);
                      digitalWrite(E,HIGH);
                      digitalWrite(F,HIGH);
                      digitalWrite(G,HIGH);
                      client.print("<script>var countdown =");
                      client.print(i);
                      client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                    }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
          break;
        case 9:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){
                    digitalWrite(A,HIGH);
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown =");
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 0:
               do{
                    currentMillis = millis();
                    if(currentMillis - previousMillis > interval){
                      digitalWrite(A,HIGH);
                      digitalWrite(B,HIGH);
                      digitalWrite(C,HIGH);
                      digitalWrite(D,HIGH);
                      digitalWrite(E,HIGH);
                      digitalWrite(F,HIGH);
                      digitalWrite(G,LOW);
                      client.print("<script>document.getElementById('countdown').innerHTML = 'Timer terminato'; </script>");
                    }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
            //EMISSIONE DEL SUONO
        break;
        default:
        do{
                    currentMillis = millis();
                    if(digitalRead(BUTTON) == 1){
                      i = startTimer;
                      client.print("<script>countdown =");
                      client.print(startTimer);
                      client.print(";</script>");
                    }
                }while(currentMillis-previousMillis < interval*3);
                previousMillis = currentMillis;
        break;
      }
    client.print("<script>function onrestart(){window.location.reload(); document.getElementById('countdown').innerHTML = 'Riavvio in corso...';}</script>");
    }
    flag = false;
    client.println("</body>");
}
