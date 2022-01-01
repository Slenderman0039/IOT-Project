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
#define BUZZER 35

WiFiServer server(80);

const char* ssid = "iPhone";
const char* password = "IoT2021_2022";

int startTimer = 0; //SALVA L'INIZIO DEL TIMER PER IL RIAVVIO
int timer = 0; //TEMPO COUNTDOWN
bool flag = false; //FLAG ATTIVAZIONE COUNTDOWN

void setPin(){ //ISTANZIO I PIN DEL DISPLAY
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

  // In attesa di connessione
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

  //Il client rimane in ascolto di nuovi client:
  WiFiClient client = server.available();   // Rimane in ascolto
  //Controlliamo se un nuovo client si registra
  if (client) {                             
    Serial.println("Nuovo client registrato!");          // stampiamo il messaggio di avvenuta registrazione
    String currentLine = "";                // Dati che trasmette il client (il pacchetto inizialmente è vuoto)
    unsigned long currentTime = millis();   
    unsigned long previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop che verifica fino a quando il client è connesso
      currentTime = millis();         
      if (client.available()) {             // Controlla se ci sono bytes che il client sta inviando
        char c = client.read();             // se è true, ci sono dei bytes e legge i bytes ricevuti
        header += c;
        if (c == '\n') {                    // se i bytes contengono uno \n, automaticamente va su una nuova line
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");      //viene costruito il pacchetto HTTP di risposta
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html>");
            Serial.println(header);
            client.println("<head><link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/uikit@3.8.1/dist/css/uikit.min.css'/><title>Wemos Timer!</title><center><h1>Timer</h1></center></head>");
            String index = header.substring(0,6); //PRELEVO LA SOTTOSTRINGA "GET /" DALLA RICHIESTA
            index.trim(); //SE TOLGO GLI SPAZI VUOTI, LA LUNGHEZZA NON SARA' MAI COME QUELLA DELLA HOMEPAGE
            String count = header.substring(10,17); //ESTRAGGO IL VALORE DEL PARAMETRO COUNT (SE CONTENUTO NELL'URL)
            String timer = header.substring(0,11);  //ESTRAGGO /TIMER SE E' CONTENUTO NELL'URL
            timer.trim(); //TOLGO GLI SPAZI DALLO /TIMER
            Serial.println("");
            Serial.println(header);   //stampo il pacchetto
            Serial.println("");
            if(index.length()==5){  //VERIFICO SE LA RICHIESTA VIENE FATTA ALLA RADICE 
            client.println("<script>document.location.href='/timer';</script>"); //NEL CASO DI ESITO POSITIVO, REINDIRIZZA ALLO /TIMER
            } else if (header.indexOf("GET /timer") >= 0 && timer.length()==10) { //VERIFICO SE LA RICHIESTA VIENE FATTA ALLO /TIMER
              client.println("<body>"); //NEL CASO DI ESITO POSITIVO COSTRUISCO LA PAGINA
              client.println("<br><center>AMMINISTRATORE<br><div class='uk-margin'><div class='uk-inline'><span class='uk-form-icon' uk-icon='icon: future'></span><input style='text-align:right;' class='uk-input' id='ptext' type='text' placeholder='Timer' required></div></div> <button class='uk-button uk-button-primary' onclick='bclick()'>Avvia</button></center>");
              client.println("<script>function bclick(){var val = document.getElementById('ptext').value; if(val===''){alert('Errore, devi inserire il valore del timer prima di proseguire!');}else{location.href = './timer?count='+val;}}");
              client.println("</script></body>");
            } else if (header.indexOf("GET /timer?count=")>=0 && count.equals("?count=")) { //VERIFICO SE LA RICHIESTA VIENE FATTA ALLO /TIMER E SE CONTIENE PARAMETRO
                int value;
                int pos1 = header.indexOf('=');
                int pos2 = header.indexOf('HTTP');      
                value = header.substring(pos1+1, pos2-4).toInt();   //ESTRAGGO IL VALORE DEL PARAMETRO E LO CONVERTO IN NUMERO INTERO
                if(value<=9 && value>=1){  //EFFETTUO UN CONTROLLO PER VERIFICARE SE L'INPUT IMMESSO E' VALIDO AI FINI DEL TIMER 
                  startTimer = value; //MEMORIZZO IL VALORE INIZIALE NELLA VARIABILE startTimer 
                  setTimer(value,client); //AVVIO IL TIMER MEDIANTE LA FUNZIONE setTimer()
                }else{  //SE L'INPUT NON E' VALIDO ALLORA RESISTUISCE VALORE NON VALIDO
                   client.println("<center><div><div class='uk-card uk-card-primary uk-card-hover uk-card-body uk-light'><h2 class='uk-card-title'>Valore non valido</h2><p>Errore valore non valido: Il valore dev'essere compreso tra 9 e 1.</p></div></div></center>");
                   client.println("<script>window.setTimeout(function(){window.location.href = './timer';}, 3000);</script>");
                }
            }else{  //SE L'URL E' DIVERSO DA TUTTE LE POSSIBILI SOLUZIONI ALLORA RESISTUISCE UNA PAGINA DOVE DICE "ERRORE 404"
              client.println("<center><div><div class='uk-card uk-card-primary uk-card-hover uk-card-body uk-light'><h2 class='uk-card-title'>404</h2><p>Errore 404: Pagina non trovata.</p></div></div></center>");
            }
            client.println("<script src='https://cdn.jsdelivr.net/npm/uikit@3.8.1/dist/js/uikit.min.js'></script><script src='https://cdn.jsdelivr.net/npm/uikit@3.8.1/dist/js/uikit-icons.min.js'></script>");
            client.println("</html>");
            break;
          } else {  
            currentLine = "";
          }
        } else if (c != '\r') {   //SE I BYTES NON CONTENGONO /r , AGGIUNGE ALLA CURRENT LINE TUTTI I BYTES  
          currentLine += c;      
        }
      }
    }
    //IL CLIENT NON E' PIU' CONNESSO E LO DISCONNETTE
    header = "";
    delay(5000);
    client.stop();
    Serial.println("Client disconnesso.");
    Serial.println("");
  }
}

void setTimer(int value,WiFiClient client){ //FUNZIONE CHE GESTISCE IL TIMER VIRTUALE ED IL DISPLAY A 7 SEGMENTI
  long previousMillis = 0;
  long interval = 1000;          
  long currentMillis = millis();
  client.println("<body><center><div id='countdown'></div><br><button class='uk-button uk-button-danger' onclick='onrestart()'>Riavvia</button></center>"); //STAMPIAMO UN BOTTONE NELLA PAGINA CHE RIAVVIA IL TIMER
    for(int i=value;i>=-2;--i){ //3*2 --> 6 //CICLO FOR CHE EFFETTUA IL CONTO ALLA ROVESCIA
      if(digitalRead(BUTTON) == 1){         //VERIFICO CONTINUAMENTE AD OGNI CICLO SE IL PULSANTE E' PIGGIATO
              i = startTimer;               //SE E' PIGGIATO, RIPRISTINA IL CONTATORE AL VALORE INIZIALE
              client.print("<script>countdown =");
              client.print(startTimer);
              client.print(";</script>");
       }
      delay(1000);
      switch(i){
        case 1:
            do{
                currentMillis = millis();
                if(currentMillis - previousMillis > interval){      //VERIFICO SE E' PASSATO 1 SECONDO
                  digitalWrite(A,LOW);                              //COMBINAZIONE DEL NUMERO 1 DEL DISPLAY A 7 SEGMENTI
                  digitalWrite(B,HIGH);
                  digitalWrite(C,HIGH);
                  digitalWrite(D,LOW);
                  digitalWrite(E,LOW);
                  digitalWrite(F,LOW);
                  digitalWrite(G,LOW);
                  client.print("<script>var countdown =");          //IMPOSTO IL TIMER VIRTUALE AL VALORE 1
                  client.print(i);
                  client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
               }
              }while(currentMillis-previousMillis < interval+1);
              previousMillis = currentMillis;
        break;
        case 2:
             do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,HIGH); //COMBINAZIONE DEL NUMERO 2 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,HIGH);
                    digitalWrite(C,LOW);
                    digitalWrite(D,HIGH); 
                    digitalWrite(E,HIGH);
                    digitalWrite(F,LOW);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 2
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                }
              }while(currentMillis-previousMillis < interval+1);
              previousMillis = currentMillis;
        break;
        case 3:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 3 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,LOW);
                    digitalWrite(F,LOW);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 3
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                }
              }while(currentMillis-previousMillis < interval+1);
              previousMillis = currentMillis;
        break;
        case 4:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,LOW);  //COMBINAZIONE DEL NUMERO 4 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,LOW);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 4
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 5:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 5 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,LOW);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 5
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
               }while(currentMillis-previousMillis < interval+1);
               previousMillis = currentMillis;
        break;
        case 6:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 6 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,LOW);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,HIGH);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 6
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 7:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 7 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,LOW);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,LOW);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 7
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 8:
               do{
                    currentMillis = millis();
                    if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                      digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 8 DEL DISPLAY A 7 SEGMENTI
                      digitalWrite(B,HIGH);
                      digitalWrite(C,HIGH);
                      digitalWrite(D,HIGH);
                      digitalWrite(E,HIGH);
                      digitalWrite(F,HIGH);
                      digitalWrite(G,HIGH);
                      client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 8
                      client.print(i);
                      client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                    }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
          break;
        case 9:
              do{
                  currentMillis = millis();
                  if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                    digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 9 DEL DISPLAY A 7 SEGMENTI
                    digitalWrite(B,HIGH);
                    digitalWrite(C,HIGH);
                    digitalWrite(D,HIGH);
                    digitalWrite(E,LOW);
                    digitalWrite(F,HIGH);
                    digitalWrite(G,HIGH);
                    client.print("<script>var countdown ="); //IMPOSTO IL TIMER VIRTUALE AL VALORE 9
                    client.print(i);
                    client.print("; document.getElementById('countdown').innerHTML = countdown + ' secondi rimanenti'; </script>");
                  }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
        break;
        case 0:
               do{
                    currentMillis = millis();
                    if(currentMillis - previousMillis > interval){ //VERIFICO SE E' PASSATO 1 SECONDO
                      digitalWrite(A,HIGH);  //COMBINAZIONE DEL NUMERO 0 DEL DISPLAY A 7 SEGMENTI
                      digitalWrite(B,HIGH);
                      digitalWrite(C,HIGH);
                      digitalWrite(D,HIGH);
                      digitalWrite(E,HIGH);
                      digitalWrite(F,HIGH);
                      digitalWrite(G,LOW);
                      client.print("<script>document.getElementById('countdown').innerHTML = 'Timer terminato'; </script>"); //IMPOSTO IL TIMER VIRTUALE A "Timer terminato".
                    }
                }while(currentMillis-previousMillis < interval+1);
                previousMillis = currentMillis;
            //EMISSIONE DEL SUONO
        break;
        default:
        do{
                    currentMillis = millis();
                    if(digitalRead(BUTTON) == 1){ //VERIFICO SE SCADUTO IL TIMER, L'UTENTE PIGIA IL BOTTONE DI RESET
                      i = startTimer;
                      client.print("<script>countdown =");
                      client.print(startTimer);
                      client.print(";</script>");
                    }
                }while(currentMillis-previousMillis < interval*3);  //VERIFICO PER 6 SECONDI SE IL PULSANTE E' PIGIATO (L'INTERVALLO E' DI 3 SECONDI MA POICHE' IL FOR VA DA 9 A -2 ALLORA FA 2 CICLI * 3 SECONDI = 6 SECONDI
                previousMillis = currentMillis;
        break;
      }
    client.print("<script>function onrestart(){window.location.reload(); document.getElementById('countdown').innerHTML = 'Riavvio in corso...';}</script>"); //SE CLICCA SUL RIAVVIO VIRTUALE, APPARE LA SCRITTA 'RIAVVIO IN CORSO' E RICARICA LA PAGINA
    }
    flag = false;
    client.println("</body>");
}
