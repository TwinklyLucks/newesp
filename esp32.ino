#include <AsyncTCP_SSL.h>
#include <AsyncTCP_SSL.hpp>

// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "*******"; // Colocar nome da rede e senha
const char* password = "*******";

bool ledState = 0; // Remover
// Adicionar uma variavel para o angulo do servo
const int ledPin = 26; // Mudar para pino do servo

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

  <head>
    <title>ESP WebServer</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <title>Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <script type="text/javascript" src="./index.js" defer=""></script>
    <link rel="stylesheet" href="./index.css">

    <style>
    html {
      font-family: Arial, Helvetica, sans-serif;
      text-align: center;
    }
  
    h1 {
      font-size: 1.8rem;
      color: white;
    }
  
    h2{
      font-size: 1.5rem;
      font-weight: bold;
      color: #143642;
    }
  
    .topnav {
      overflow: hidden;
      background-color: #143642;
    }
  
    body {
      margin: 0;
    }
  
    .content {
      padding: 30px;
      max-width: 600px;
      margin: 0 auto;
    }
  
    .card {
      background-color: #F8F7F9;;
      box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
      padding-top:10px;
      padding-bottom:20px;
    }
  
    .button {
      padding: 15px 50px;
      font-size: 24px;
      text-align: center;
      outline: none;
      color: #fff;
      background-color: #0f8b8d;
      border: none;
      border-radius: 5px;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      -khtml-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
      -webkit-tap-highlight-color: rgba(0,0,0,0);
     }
  
     /*.button:hover {background-color: #0f8b8d}*/
     .button:active {
       background-color: #0f8b8d;
       box-shadow: 2 2px #CDCDCD;
       transform: translateY(2px);
     }
  
     .state {
       font-size: 2rem;
       color:#000000;
       font-weight: bold;
     }
     
     .slider {
    appearance: none;
    width: 77%; 
    height: 5px;
    background: #c9c9c9;
    opacity: 0.7;
  }
</style>

  </head>

  <body>

    <div class="topnav">
      <h1>ESP WebServer</h1>
    </div>
    <div class="content">
      <div class="card">
        <p class="state">Status do Servo-Motor: <span id="demo"></span></p>
        <label for="motor">Motor State (between 0 and 180):</label>
        <input type="range" name="motor" min="0" max="180" value="90" step="1" class="slider" id="motorange" onchange="sliderChange()">
      </div>
    </div>

    <script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  // Mudar a funcao onMessage para que mude o valor selecionado no slider quando receber a mensagem da ESP
  // O valor enviado esta em event.data
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    // O ID do spam é 'demo'
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  // Pode remover essa funcao pois o evento ja esta escrito no html
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  // Remover essa funcao (o send sera feito na funcao sliderChange)
  function toggle(){
    websocket.send('toggle');
  }
  
  // Remover variavel
  let colour = 0;
  // Remover funcao abaixo
  function led(){
    if(colour == 0){
      button.style.backgroundColor = "gray";
    colour = 1;
    } else if(colour == 1){
      button.style.backgroundColor = "rgb(95, 158, 160)";
    colour = 0;
    }
  }
  
  function sliderChange(){
      output.innerHTML = slider.value;
      // Adicionar websocket.send() enviando o valor do slider
  }

    let slider = document.getElementById("motorange");
    let output = document.getElementById("demo");
    output.innerHTML = slider.value;

    </script>
  </body>

</html>
)rawliteral";

// Notifica os clientes mandando o angulo do servo, ao inves de ledState
void notifyClients() {
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) { // strcmp serve para comparar duas strings, nao precisa desse if ja que vamos mandar o valor do servo
      // Aqui mudamos a variavel do angulo do servo
      // Como recebemos uma string, devemos mudar para um int para escrever no servo
      // Uma possibilidade seria fazer stoi(data)
      ledState = !ledState;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Essa funcao pode retornar o valor do angulo do servo, transformando ele numa string
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

// Adicionar objeto do servo
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT); // remover
  digitalWrite(ledPin, LOW); // remover
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();

  // Fazer attach do servo
}

void loop() {
  ws.cleanupClients();
  digitalWrite(ledPin, ledState); // Remover
  // Adicionar comando para escrever o angulo (variavel declarada la em cima) no servo
}
