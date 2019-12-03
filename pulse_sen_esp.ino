#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#define PULSE_PIN 0

IPAddress server_addr(192, 168, 0, 4);
char user[] = "csingh";
char password[] = "password";

char ssid[] = "csingh";
char pass[] = "password";

WiFiClient client;
WiFiServer server(80);
MySQL_Connection conn((Client *)&client);

bool connSt;
char INSERT_SQL_DB[] = "INSERT INTO pulse_db.data (pulse, stat) VALUES ('%s', '%s')";
char mainQuery[128];

void setup(){
  pinMode(PULSE_PIN, INPUT);
  Serial.begin(9600);
  delay(500);
  connectWIFI();
  connectDB();
}

void loop(){

  String stat;
  
  int pulseVal = analogRead(PULSE_PIN);
  if (connSt == false) {
    connectDB();
  }

  if(pulseVal < 500){
    stat = "Pulse Normal";
  }else{
    stat = "Something's wrong !!!";
  }
  String plse = String(pulseVal);
  
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  sprintf(mainQuery, INSERT_SQL_DB, plse, stat);
  cur_mem->execute(mainQuery);
  delete cur_mem;
  closeDB();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  //Message on Browser
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nPulse Status: ";
  s += stat;

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
  
  delay(10000);
}

void connectDB() {
  Serial.println("Connecting to database...");
  if (conn.connect(server_addr, 3306, user, password)) {
    connSt = true;
    Serial.println("DB Connection Created");
  } else {
    Serial.println("A Connection Could not be Established with Database, Trying Again");
    connSt = false;
  }
}

void closeDB() {
  conn.close();
  connSt = false;
  Serial.println("DB Connection Closed");
}

void connectWIFI() {
  WiFi.begin(ssid, pass);
  int count = 0;
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    if (count > 60) {
      count = 0;
      WiFi.disconnect();
      ESP.eraseConfig();
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print ("IP address: ");
  Serial.println (WiFi.localIP());

  server.begin();
  Serial.println("Server started");
  WiFi.mode(WIFI_STA);
}

