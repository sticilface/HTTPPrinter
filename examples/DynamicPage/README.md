This is a library that allows you to inject dynamic content into SPIFFS files, for ESP8266 environment. 


The SPIFFS upload tool must be used to upload the contents of the data directory to the ESP. 


Create object :
```
HTTPPrinter printer; 
```

Then in webpage callback, 
```
WiFiClient c = HTTP.client();
printer.BeginPage(c, SPIFFS, "/file", 30);   BeginPage(WiFiClient & c, fs::FS& fs, const char* path, uint8_t items);
printer.AddVariable(0, "version",   version);  AddVariable(uint8_t n, const char * field, const char * text); 
printer.SendPage(); // sends the page, reclaims the memory.. 
```
