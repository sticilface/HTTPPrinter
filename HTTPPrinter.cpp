#include <HTTPPrinter.h>

  HTTPPrinter::HTTPPrinter() {
    _bufferCount = 0;  
    _buffer = NULL;
    _size = 0; 
    _headerSent = false; 
    _CountMode = false; 
  }


  void HTTPPrinter::Begin(WiFiClient &c) {
    _bufferCount = 0;  
    _buffer = (uint8_t *)malloc(HTTPPrinterSize);
    _client = c;
    _size = 0; 
    _headerSent = false; 
    _CountMode = false; 
  }
  
  void HTTPPrinter::Begin(WiFiClient &c, size_t memory) {
    _bufferCount = 0;  
    _buffer = (uint8_t *)malloc(memory);
    _client = c;
    _size = 0; 
    _headerSent = false; 
    _CountMode = false; 
  }

  void HTTPPrinter::End() {
     free(_buffer);
   }

  void HTTPPrinter::Setsize(size_t s) {
    _size = s; 
  }

  size_t HTTPPrinter::SetCountMode(bool mode) {

    if (mode) {
      _CountMode = true;
      _size = 0;
    } else {
      _CountMode = false; 
      _sizeTotal = _size; 
    }

    return _size; 

  }

  size_t HTTPPrinter::GetSize() {
    return _sizeTotal; 
  }

  
  size_t HTTPPrinter::write(uint8_t data) {

    if (_buffer == NULL) return 0; 

    switch(_CountMode) {

      case true:
        _size++;
        break; 

      case false:

        _buffer[_bufferCount++] = data; // put byte into buffer 

         if(_bufferCount == HTTPPrinterSize || _size == _bufferCount ) { // send it if full, or remaining bytes = buffer

            if (!_headerSent) Send_Header(_code, _headerContent); 

              _client.write(_buffer + 0, _bufferCount);
              //Serial.write(_buffer + 0, _bufferCount);
              _size -= _bufferCount; // keep track of remaining bytes.. 
              _bufferCount = 0; // reset the buffer to begining.. 
          } 
        break;

    } // end of switch

    return true; 

  } // end of write 

void HTTPPrinter::SetHeader(int code, const char* content) {
  if (code < 0) code = 0; 
  _code = code; 
  _headerContent = content; 
}


size_t HTTPPrinter::Send(WiFiClient client, int code, const char* content, printer_callback fn ){

      Begin(client); 
      SetHeader(code, content);
      SetCountMode(true);
      fn();
      size_t size = SetCountMode(false);
      fn();
      End(); 

     client.stop();
     while(client.connected()) yield();

     return size; 
  }

 void HTTPPrinter::Send_Header () {

      Send_Header(_code, _headerContent);
      SetCountMode(false);


 }

 size_t HTTPPrinter::Send_Buffer(int code, const char* content) {

    _size = _bufferCount; 
    Send_Header(code, content); 
    _client.write(_buffer + 0, _size);
    return _size; 
    End(); 

 }



 void HTTPPrinter::Send_Header (int code, const char * content) {

        if (_headerSent) return; 

          uint8_t *headerBuff = (uint8_t*)malloc(128);
          sprintf((char*)headerBuff, "HTTP/1.1 %u OK\r\nContent-Type: %s\r\nContent-Length: %u\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\n\r\n", code, content,_size);
          //Serial.println("Header");
          //Serial.println((char*)headerBuff);

          size_t headerLen = strlen((const char*)headerBuff);
          _client.write((const uint8_t*)headerBuff, headerLen);
          free(headerBuff);
          _headerSent = true; 

 }


void HTTPPrinter::SendPage() {

    long _time = millis();
    uint8_t clock = 0; 
    SetCountMode(true);
    uint8_t count; 

    if (!F) { 
      Serial.println(F("File open error ")); 
      return; 
    } else {

      F.setTimeout(0); // kills the 1 second time out.  not needed with SPIFFS file.. might need to be 1... 
      bool found = false;  
      bool found_end = false;      
      char foundseq[32]; 


  for (uint8_t loopround = 0; loopround < 2; loopround++) {    


    if (loopround == 1) {
        size_t bytes = SetCountMode(false);
      }
      
      uint32_t old_position = 0; 
       count = 0;
     

    do {

      F.seek(old_position, SeekSet);
      found = F.findUntil("%_","EOF"); 
      uint32_t start_position = F.position() - 2; 
      found_end = F.findUntil("_%","EOF");

      // this is needed to push out rest of file if there are no more inserts to find... 
      if ( (!found || !found_end) && F.position() == F.size() ) {
        F.seek(old_position, SeekSet);
            while ( F.position() < F.size() ) {
              write(F.read()); 
        }
        break; 
      }

      uint32_t end_position = F.position(); 
      uint32_t length_of_field = end_position - start_position - 4 ; 

      if (length_of_field < 32) {
        count++; 
      
      F.seek(old_position, SeekSet); // go to last point

      //  send up untill start to write
      for (int i = old_position; i < start_position; i++) {
        unsigned char value = F.read();
        write(value); 
      }


      F.seek(start_position + 2, SeekSet); // go to start of field 
      memset(foundseq, 0, sizeof(foundseq)); // reset found seq buffer

      for (int i = 0; i < length_of_field; i++) {
        if (i < 32) {
          foundseq[i] = F.read(); 
        }
      }      

//      check each value in fields array... 
      for (uint8_t i = 0; i < _items; i++) {
        if (_HTTP_Vars[i].text != NULL) {
           HTTP_Vars_t * params = &_HTTP_Vars[i];
          if (strcmp (foundseq, params->field) == 0 ) {  
              print( (char*) (_HTTP_Vars[i].text));
//              if (!loopround) Serial.printf(", FOUND MATCH n = %u ", i);
          } 
        }   
      }

      old_position = end_position; 
           
//      if (!loopround) Serial.println(); 
      } 

    } while (found && found_end);// && count < _items); 

    
    }

    _client.stop();
     while(_client.connected()) yield();

     Serial.printf(" %u variables inserted (%ums)\n", count, millis()-_time);

  }

    EndPage(); 

}

void HTTPPrinter::EndPage() {
  
  F.close();
  free(_buffer);
   delete[] _HTTP_Vars; 
  _HTTP_Vars = NULL; 

}


void HTTPPrinter::BeginPage(WiFiClient & c, fs::FS& fs, const char* path, uint8_t items) {

    _HTTP_Vars = new HTTP_Vars_t[items];  // allocate memory for the vars... 

    _items = items; 

    F = fs.open(path, "r");

    if (!F) { 
      Serial.println(F("File open error ")); 
      return; 
    } else {

    Begin(c); 

    SetHeader(200, "text/html");
  }

}


bool HTTPPrinter::AddVariable ( uint8_t n, const char * field,  const char * text) {

 if (n > _items) return false;  

  HTTP_Vars_t * params = &_HTTP_Vars[n];
  params->text = text; 
  params->field = field; 

}









