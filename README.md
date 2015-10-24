# HTTPPrinter

This is a library that allows you to inject dynamic content into SPIFFS files, for ESP8266 environment. 


The SPIFFS upload tool must be used to upload the contents of the data directory to the ESP. 


Create object :
```
HTTPPrinter printer; 
```

Then in webpage callback, 
```
WiFiClient c = HTTP.client();
printer.BeginPage(c, SPIFFS, "/file", 30);   
printer.AddVariable(0, "version",   version);  
printer.SendPage(); // sends the page, reclaims the memory.. 
```
