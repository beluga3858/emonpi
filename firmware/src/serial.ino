#define PGMSTR(x) (__FlashStringHelper*)(x)

const char helpText1[] PROGMEM =                                 // Available Serial Commands
"\r\n"
"Available commands:\r\n"
#if !defined(RF_DISABLE)
"  <nn> i     - set node IDs (standard node ids are 1..30)\r\n"
"  <n> b      - set MHz band (4 = 433, 8 = 868, 9 = 915)\r\n"
"  <nnn> g    - set network group (RFM12 only allows 212, 0 = any)\r\n"
"  ...,<nn> a - send data packet to node <nn>, request ack\r\n"
"  ...,<nn> s - send data packet to node <nn>, no ack\r\n"
"  <n> q      - set quiet mode (1 = don't report bad packets)\r\n"
#endif // RF_DISABLE
"  <n> p      - set AC Adapter Vcal 1p = UK, 2p = USA\r\n"
"  v          - show firmware version\r\n"
;

void serial_print_startup(){

  Serial.print(F("CT 1 Cal: ")); Serial.println(Ical1);
  Serial.print(F("CT 2 Cal: ")); Serial.println(Ical2);
  if (USA) Serial.println(F("USA mode"));

  if (ACAC) {
    Serial.println(F("AC Wave Detected - Real Power calc enabled"));
    Serial.print(F("Vcal: ")); Serial.println(Vcal);
    Serial.print(F("Vrms: ")); Serial.print(Vrms); Serial.println(F("V"));
    Serial.print(F("Phase Shift: ")); Serial.println(phase_shift);
  } else {
    Serial.println(F("AC NOT detected - Apparent Power calc enabled"));
    Serial.print(F("Assuming VRMS: ")); Serial.print(Vrms); Serial.println(F("V"));
  }

  Serial.print(F("Detected ")); Serial.print(CT_count); Serial.println(F(" CT's"));

  Serial.print(F("Detected ")); Serial.print(numSensors); Serial.println(F(" DS18B20"));

#if !defined(RF_DISABLE)
  #if (RF69_COMPAT)
    Serial.println(F("RFM69CW Init: "));
  #else
    Serial.println(F("RFM12B Init: "));
  #endif

  Serial.print(F("Node ")); Serial.print(nodeID);
  Serial.print(F(" Freq "));
  if (RF_freq == RF12_433MHZ) Serial.print(F("433Mhz"));
  if (RF_freq == RF12_868MHZ) Serial.print(F("868Mhz"));
  if (RF_freq == RF12_915MHZ) Serial.print(F("915Mhz"));
  Serial.print(F(" Network ")); Serial.println(networkGroup);
#endif

  Serial.print(PGMSTR(helpText1));
  delay(20);
}

//Send emonPi data to Pi serial /dev/ttyAMA0 using struct JeeLabs RF12 packet structure
void send_emonpi_serial()
{
  byte binarray[sizeof(emonPi)];
  memcpy(binarray, &emonPi, sizeof(emonPi));

  Serial.print(F("OK "));
  Serial.print(nodeID);
  for (byte i = 0; i < sizeof(binarray); i++) {
    Serial.print(F(" "));
    Serial.print(binarray[i]);
  }
  Serial.print(F(" (-0)"));
  Serial.println();

  delay(10);
}

static void serial_handle_input (char c) {
  if ('0' <= c && c <= '9') {
    value = 10 * value + c - '0';
    return;
  }

  if (c == ',') {
    if (top < sizeof stack)
      stack[top++] = value; // truncated to 8 bits
    value = 0;
    return;
  }

  if (c > ' ') {

    switch (c) {

#if !defined(RF_DISABLE)
      case 'i': //set node ID
        if (value){
          nodeID = value;
          rf12_initialize(nodeID, RF_freq, networkGroup);
        }
        break;

      case 'b': // set band: 4 = 433, 8 = 868, 9 = 915
        value = bandToFreq(value);
        if (value){
          RF_freq = value;
          rf12_initialize(nodeID, RF_freq, networkGroup);
        }
        break;
    
      case 'g': // set network group
        if (value>=0){
          networkGroup = value;
          rf12_initialize(nodeID, RF_freq, networkGroup);
        }
        break;

      case 'q': // turn quiet mode on or off (don't report bad packets)
        quiet_mode = value;
        break;

      case 'a': // send packet to node ID N, request an ack
      case 's': // send packet to node ID N, no ack
        rf_cmd = c;
        rf_send_len = top;
        rf_dest = value;
        break;
#endif // RF_DISABLE        

      case 'p': // set Vcc Cal 1=UK/EU 2=USA
        if (value){
          if (value==1) USA=false;
          if (value==2) USA=true;
        }
        Serial.print(F("[USA=")); Serial.print(USA); Serial.print(F("]"));
        break;
        
      case 'v': // print firmware version
        Serial.print(F("[emonPi.")); Serial.print(firmware_version*0.01); Serial.print(F("]"));
        break;
        
      default:
        Serial.print(PGMSTR(helpText1));
    } //end case 

#if !defined(RF_DISABLE)
    // Print Current RF config  
    Serial.print(F(" "));
    Serial.print((char) ('@' + (nodeID & RF12_HDR_MASK)));
    Serial.print(F(" i"));
    Serial.print(nodeID & RF12_HDR_MASK);
    Serial.print(F(" g"));
    Serial.print(networkGroup);
    Serial.print(F(" @ "));
    Serial.print(RF_freq == RF12_433MHZ ? 433 :
                 RF_freq == RF12_868MHZ ? 868 :
                 RF_freq == RF12_915MHZ ? 915 : 0);
    Serial.print(F(" MHz"));
    Serial.print(F(" q")); 
    Serial.print(quiet_mode);
#endif // RF_DISABLE      

    Serial.println(F(" "));
    
  }

  value = top = 0;
}
