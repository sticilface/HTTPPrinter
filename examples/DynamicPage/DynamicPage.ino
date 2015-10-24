/*-------------------------------------------------------------------------------------------------------

Dynamic Page generator from static SPIFFS file.

Beerware licence.

Sticilface. 

--------------------------------------------------------------------------------------------------------*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>

#include <HTTPPrinter.h>



const char * version =  "WS2812-OTA";
const char * host = "HTTPPrinter";
const char compile_date[] = __DATE__ " " __TIME__;
const uint16_t aport = 8266;


WiFiServer TelnetServer(aport);
WiFiClient Telnet;
WiFiUDP OTA;
ESP8266WebServer HTTP(80);
HTTPPrinter printer; 


void setup() {

  Serial.begin(115200);

  SPIFFS.begin();
  
  delay(500);

  Serial.println("");
  Serial.println("Dynamic web page example");

  Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
  Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());

  Serial.println("SPIFFS");
    {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }

      uint8_t i = 0;
      
  while (WiFi.status() != WL_CONNECTED ) {
        delay(500);
        i++;
        Serial.print(".");
        if (i == 20) { 
          Serial.print("Entering AP setup..."); 
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(host);
          break; 
        } ;
  }


  if(WiFi.waitForConnectResult() == WL_CONNECTED){

    MDNS.begin(host);
    MDNS.addService("arduino", "tcp", aport);
    OTA.begin(aport);

    TelnetServer.begin();
    TelnetServer.setNoDelay(true);
    delay(10);
    Serial.print("\nIP address: ");
    String IP = String(WiFi.localIP()); 
    Serial.println(WiFi.localIP());


    HTTP.on("/", handle_aboutdevice);
    HTTP.begin();

    Serial.println("Services Started....");
  } else {
    Serial.println("Services NOT Started....");
    ESP.reset();
  }

  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());



}


void loop() {

  HTTP.handleClient();
  yield(); 

  OTA_handle();


}



void OTA_handle(){

  if (OTA.parsePacket()) {
    IPAddress remote = OTA.remoteIP();
    int cmd  = OTA.parseInt();
    int port = OTA.parseInt();
    int size   = OTA.parseInt();

    Serial.print("Update Start: ip:");
    Serial.print(remote);
    Serial.printf(", port:%d, size:%d\n", port, size);
    uint32_t startTime = millis();

    WiFiUDP::stopAll();

    if(!Update.begin(size)){
      Serial.println("Update Begin Error");
      return;
    }

    WiFiClient client;

    bool connected = false; 

    delay(2000);
    
      connected = client.connect(remote, port); 

    if (connected) {

      uint32_t written;
      while(!Update.isFinished()){
        written = Update.write(client);
        if(written > 0) client.print(written, DEC);
      }
      Serial.setDebugOutput(false);

      if(Update.end()){
        client.println("OK");
        Serial.printf("Update Success: %u\nRebooting...\n", millis() - startTime);
        ESP.restart();
      } else {
        Update.printError(client);
        Update.printError(Serial);
      }
    

    } else {
      Serial.printf("Connect Failed: %u\n", millis() - startTime);
      ESP.restart(); 
    }
  }
  
  //IDE Monitor (connected to Serial)
  if (TelnetServer.hasClient()){
    if (!Telnet || !Telnet.connected()){
      if(Telnet) Telnet.stop();
      Telnet = TelnetServer.available();
    } else {
      WiFiClient toKill = TelnetServer.available();
      toKill.stop();
    }
  }
  if (Telnet && Telnet.connected() && Telnet.available()){
    while(Telnet.available())
      Serial.write(Telnet.read());
  }
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t * sbuf = (uint8_t *)malloc(len);
    Serial.readBytes(sbuf, len);
    if (Telnet && Telnet.connected()){
      Telnet.write((uint8_t *)sbuf, len);
      yield();
    }
    free(sbuf);
  }
}



//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}





void handle_aboutdevice() {

    const uint8_t bufsize = 10; 

    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;
    int Vcc = analogRead(A0); 
    
    char Up_time[bufsize]; 
    snprintf ( Up_time, bufsize, "%02d:%02d:%02d", hr, min % 60, sec % 60 );

  uint32_t _time = millis();


// at the moment char arrays need to be created.. to cast to, especially for the ESP function returns.

  String string1 = "This is a test string 1......  lalalalalalalalalala";
  String string2 = " Another String...... "; 

  char heap[bufsize];
  char flashsize[bufsize];
  char flashsizeid[bufsize];
  char flashid[bufsize] ; 
  char chipid[bufsize] ; 
  char sketchsize[bufsize] ; 
  char freespace[bufsize] ; 
  char millisvar[bufsize] ; 
  char vcc[bufsize] ; 
  char rssi[bufsize] ; 
  char cpu[bufsize] ; 

  strcpy(heap,        String(   ESP.getFreeHeap()).c_str() );
  strcpy(flashsize,   String(   ESP.getFlashChipSize()).c_str()   );
  strcpy(flashsizeid, String(   ESP.getFlashChipSizeByChipId()).c_str()   );
  strcpy(flashid, String(   ESP.getFlashChipId()               ).c_str()   );
  strcpy(chipid, String(   ESP.getChipId()      ).c_str()   );
  strcpy(sketchsize, String(   ESP.getSketchSize()  ).c_str()   );
  strcpy(freespace, String(   ESP.getFreeSketchSpace()  ).c_str()   );  
  strcpy(millisvar, String(   millis()      ).c_str()   );
  strcpy(vcc, String(   ESP.getVcc()         ).c_str()   ); 
  strcpy(rssi, String(   WiFi.RSSI()       ).c_str()   );
  strcpy(cpu, String(   ESP.getCpuFreqMHz()         ).c_str()   );

    
    WiFiClient c = HTTP.client();

printer.BeginPage(c, SPIFFS, "/aboutdevice.htm", 30);  // create the page, and sets aside memory for the variable pointers...  

//  Add the variable.  Does not have to be in order of appearence... and it will replace all instances in target   use %_version_%  for example.. 
   printer.AddVariable(0, "version",     version); 
   printer.AddVariable(1, "compile",     compile_date); 
   printer.AddVariable(2, "sdk",         (const char *) ESP.getSdkVersion()); 
   printer.AddVariable(3, "heap",         heap  ); 
   printer.AddVariable(4, "flashsize",    flashsize  );   
   printer.AddVariable(5, "flashsizeid", flashsizeid  ); 
   printer.AddVariable(6, "flashid",     flashid  ); 
   printer.AddVariable(7, "chipid",      chipid   ); 
   printer.AddVariable(8, "sketchsize",  sketchsize  ); 
   printer.AddVariable(9, "freespace",   freespace   ); 
   printer.AddVariable(10,"millis" ,     millisvar   ); 
   printer.AddVariable(11,"uptime" ,     Up_time);
   printer.AddVariable(12, "vcc",        vcc ); 
   printer.AddVariable(13, "rssi",       rssi   ); 
   printer.AddVariable(14, "cpu"       , cpu   ); 
   printer.AddVariable(15, "teststring1",       string2.c_str()   ); 
   printer.AddVariable(16, "teststring2"       , string1.c_str()   );


  printer.SendPage();
  printer.EndPage(); 

  Serial.printf("Sent page %ums, heap = %u B\n", millis() - _time, ESP.getFreeHeap());


}





