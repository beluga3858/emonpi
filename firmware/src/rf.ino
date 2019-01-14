#if !defined(RF_DISABLE)

void RF_Setup(){
	//--------------------------------------------------Initalize RF and send out RF test packets--------------------------------------------------------------------------------------------  
  delay(10);
  rf12_initialize(nodeID, RF_freq, networkGroup);                          // initialize RFM12B/rfm69CW
   for (int i=10; i>=0; i--)                                                                  //Send RF test sequence (for factory testing)
   {
     emonPi.power1=i; 
     rf12_sendNow(0, &emonPi, sizeof emonPi);
     delay(100);
   }
  rf12_sendWait(2);
  emonPi.power1=0;
 //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
}

bool RF_Rx_Handle(){

  if (rf12_recvDone()) {		//if RF Packet is received
    if (rf12_crc == 0) {		//Check packet is good
      Serial.print(F("OK"));		//Print "good packet" line prefix
      print_frame(rf12_len);		//Print recieved data
      if (RF12_WANTS_ACK==1) {
        // Serial.print(F(" -> ack"));
        rf12_sendStart(RF12_ACK_REPLY, 0, 0);
      }
      return true;
    } else {
      if (quiet_mode == 0) {            //if the packet is bad
        Serial.print(F(" ?"));    	//Print the "bad packet" line prefix
        print_frame(20);          	//Print only the first 20 bytes of a bad packet
      }
    }
  } //end recDone
  return false;
}

void print_frame (int len) {
  Serial.print(F(" "));
  Serial.print(rf12_hdr & 0x1F);        // Extract and print node ID
  Serial.print(F(" "));
  for (byte i = 0; i < len; ++i) {
    Serial.print((word)rf12_data[i]);
    Serial.print(F(" "));
  }
#if RF69_COMPAT
  // display RSSI value after packet data e.g (-xx)
  Serial.print(F("("));
  Serial.print(-(RF69::rssi>>1));
  Serial.print(F(")"));
#endif
  Serial.println();
}

void send_RF(){

	if (rf_cmd && rf12_canSend() ) {                                                //if command 'cmd' is waiting to be sent then let's send it
    digitalWrite(LEDpin, HIGH); delay(200); digitalWrite(LEDpin, LOW);
    Serial.print(F(" -> "));
    Serial.print((word) rf_send_len);
    Serial.print(F(" b\n"));
    byte header = rf_cmd == 'a' ? RF12_HDR_ACK : 0;
    if (rf_dest)
      header |= RF12_HDR_DST | rf_dest;
    rf12_sendStart(header, stack, rf_send_len);
    rf_cmd = 0;
  }
}

static byte bandToFreq (byte band) {
  return band == 4 ? RF12_433MHZ : band == 8 ? RF12_868MHZ : band == 9 ? RF12_915MHZ : 0;
}

#endif // RF_DISABLE
