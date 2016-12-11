  #include <ESP8266WiFi.h>

#include "config.h"
#include "HuzzahWiFi.h"


/*
 * Digital pin 5: HIGH -> command mode
 *                LOW ->  byte stream mode
 *                
 * Each "S" COMMAND receives an answer: S COMMAND OK/NOK
 * "S" commands to set different parameters
 * S SSID param_ssid(string)
 * S KEY param_key(string)
 * S IP_ADD param_ip_addr(IP Adress: static IP address)
 * S TRANSMIT type(int): set transmission type: ANSWER: S TRANSMIT OK 
 * S REMOTE_IP param_ip_addr : set the IP/name of the remote server  --> ANSWER: S REMOTE_IP OK
 * S REMOTE_PORT port(int) : set the port number of the remote server -> ANSWER : S REMOTE_PORT OK
 * S SERVER_PORT (int): Set the port number we will be listening to
 * 
 * "G" commands to get parameters and status
 *  G STATUS --> ANSWER: G STATUS OK param_status(number)
 *  G IP_ADD --> ANSWER: G IP_ADD OK param_ip_addr(ip address, can be -1 if no IP is set)
 *  G NETWORK param_index(int) : get ssid of network number index ssid --> ANSWER: G NETWORK OK param_SSID(string) or G NETWORK NOK UNKNOWN_INDEX
 * 
 * "A" commands to trigger actions:
 * A AP_CONNECT : Connect to WiFi AP  --> ANSWER: "A AP_CONNECT OK" or "A AP_CONNECT NOK";
 * A SCAN : Scan networks --> ANSWER: A SCAN OK param_nb_netw(int) or A SCAN NOK
 * A CONNECT --> ANSWER: A CONNECT OK or A CONNECT NOK
 * A FLUSH : Flush the full transmlit buffer --> ANSWER:  A FLUSH OK nb_bytes(int) [+] number of bytes in the buffer that got sent,
 *                                                        a "+" sign means there are still chars in the tx buf, probably an error occured
 * A LISTEN  -->> ANSWER: A LISTEN OK/NOK : listen to incoming connection on the port set by S SET_SERVER_PORT
 */


const char ANS_NOK_UNK_CMD[] PROGMEM=" NOK UNKNOWN COMMAND";
const char ACT_AP_CONNECT[] PROGMEM="AP_CONNECT";
const char ACT_SCAN[] PROGMEM="SCAN";
const char GET_NETW[] PROGMEM = "NETWORK";
const char SET_REMOTE_IP[] PROGMEM = "REMOTE_IP";
const char SET_REMOTE_PORT[] PROGMEM = "REMOTE_PORT";
const char ACT_CONNECT[] PROGMEM = "CONNECT";
const char GET_CONECTED[] PROGMEM="G IS_CONNECTED";
const char ACT_FLUSH[] PROGMEM="FLUSH ";
const char SET_SERVER_PORT[] PROGMEM = "SERVER_PORT";
const char ACT_LISTEN[] PROGMEM = "LISTEN";


#define CMD_L 100
char cmd[CMD_L+1];
byte cmd_pos=0;

#define BUF_L 512
byte rx_buf[BUF_L];  // buffer receiving bytes from the wifi network waiting to be sent to the serial port
unsigned int rx_buf_start,rcv_buf_end=0;
unsigned long last_rx;
byte tx_buf[BUF_L+1];  // buffer receiving bytes from the serial port waiting to be sent to the wifi network
unsigned int tx_buf_start=0,tx_buf_end=0,tx_to_send=0;
unsigned long last_tx,tx_flush_timeout=10;  // Default time_out 10 ms around 150 bytes at 115200 bds

void setup() {
  pinMode(CMD_PIN, INPUT);
  cmd[CMD_L]='\0';
  tx_buf[BUF_L]='\0';
  Serial.begin(115200);
  while(!Serial);
  WiFi.mode(WIFI_STA);
  last_rx = last_tx = millis();
}

// Send as many bytes from the tx_buf to the wifi
size_t tx_buf_send()
{
  if (Huzzah.client.connected() && (tx_buf_start!=tx_buf_end)) {
    unsigned int end = (tx_buf_start<tx_buf_end) ? tx_buf_end : BUF_L;
    size_t written = Huzzah.client.write((const char*)&tx_buf[tx_buf_start],(size_t)(end-tx_buf_start));
    size_t written_again = 0;
    tx_to_send-=written;
    tx_buf_start+=written;
    if (tx_buf_start >= BUF_L) {
      tx_buf_start = 0;
      if (tx_to_send>0) {
         written_again = Huzzah.client.write((const char*)tx_buf,(size_t)tx_buf_end);
         tx_buf_start+=written_again;
         tx_to_send-=written_again;
      }
    }
    last_tx = millis();
    return written+written_again;
  } else return 0;
}

void set_command(char * cmd)
{
  if (strncmp(cmd+2,"SSID",4)==0) {
    Huzzah.set_ssid(cmd+7);
    Serial.println(cmd+7);
    Serial.println("S SSID OK");
  }
  else if (strncmp(cmd+2,"KEY",3)==0) {
    Huzzah.set_key(cmd+6);
    Serial.println(cmd+6);
    Serial.print("S KEY OK");
  }
  else if (strncmp_P(cmd+2, SET_REMOTE_IP,strlen_P(SET_REMOTE_IP))==0) {
    char * current=cmd+3+strlen_P(SET_REMOTE_IP), * beginning = current;
    char * new_pos=NULL;
    byte i = 0;
    int ip_nbs[4];
    // Check if its an IP address and get it
    while (i<4) {
      ip_nbs[i]=strtol(current,&new_pos,10);      
      if (current==new_pos) 
        break;
      current = new_pos+1;
      i++;
    }
    Serial.print("S ");
    Serial.print(FPSTR(SET_REMOTE_IP));
    Serial.println(" OK");
    if (i<4) // Does not look like an IP address so its a normal address
      Huzzah.set_remote_name(beginning);
    else Huzzah.set_remote_ip(IPAddress(ip_nbs[0],ip_nbs[1],ip_nbs[2],ip_nbs[3]));
  }
  else if ((strncmp_P(cmd+2, SET_REMOTE_PORT,strlen_P(SET_REMOTE_PORT))==0) || (strncmp_P(cmd+2, SET_SERVER_PORT,strlen_P(SET_SERVER_PORT))==0)) {
    char * current;
    bool serv = false;
    if (strncmp_P(cmd+2, SET_REMOTE_PORT,strlen_P(SET_REMOTE_PORT))==0)
      current = cmd+3+strlen_P(SET_REMOTE_PORT);
    else {
      current = cmd+3+strlen_P(SET_SERVER_PORT);
      serv = true;
    }
    char * beginning = current;
    char * new_pos=NULL;
    byte i = 0;
    int port = strtol(current,&new_pos,10);
    if (current==new_pos) 
        port = -1;
    if ((port>0) && (port<=65535)) {
      Serial.print("S ");
      if (!serv)
        Serial.print(FPSTR(SET_REMOTE_PORT));
      else
        Serial.print(FPSTR(SET_SERVER_PORT));
      Serial.println(" OK");
      Huzzah.set_remote_port(port);
    }
  }
  else {
    Serial.print("S");
    Serial.println(FPSTR(ANS_NOK_UNK_CMD));
  }
}

void get_command(char * cmd)
{
  byte pos = 2;
  if (strncmp_P(cmd+pos,GET_NETW,strlen_P(GET_NETW))==0) {
    pos+=strlen_P(GET_NETW)+1;
    byte index = atoi(cmd+pos);
    Serial.print("G ");
    Serial.println(FPSTR(GET_NETW));    
    if ((index<0) || (index>Huzzah.get_nb_networks()-1))
      Serial.println(" NOK");
    else {
      Serial.print(" OK ");
      Serial.println(WiFi.SSID(index));
    }
  }
}

void action_commands(char * cmd)
{
  if (strncmp_P(cmd+2,ACT_AP_CONNECT,strlen_P(ACT_AP_CONNECT))==0) {
    bool conn=Huzzah.ap_connect();
    Serial.print("A ");
    Serial.print(FPSTR(ACT_AP_CONNECT));
    if (conn)
      Serial.println(" OK");
    else Serial.println(" NOK");
  } else if (strncmp_P(cmd+2,ACT_SCAN,strlen_P(ACT_SCAN))==0) {
    int nb_netw = Huzzah.scanNetworks();
    Serial.print("A ");
    Serial.print(FPSTR(ACT_SCAN));
    if (nb_netw == WIFI_SCAN_FAILED)
      Serial.println(" NOK");
    else {
      Serial.print(" OK ");
      Serial.println(nb_netw);
    }
  } else if (strncmp_P(cmd+2,ACT_CONNECT,strlen_P(ACT_CONNECT))==0) {
    bool success=false;
    if (Huzzah.get_server_add_type()==SERVER_ADD_NAME)
    {
      String s = String("Connection to ")+Huzzah.get_remote_name()+":";
      DEBUG((s+String(Huzzah.get_remote_port())).c_str());
      success = Huzzah.client.connect(Huzzah.get_remote_name(),Huzzah.get_remote_port());
    }
    else if (Huzzah.get_server_add_type()==SERVER_ADD_IP)
    {
      String s = "Connection to "+Huzzah.get_remote_ip();
      DEBUG((s+":"+String(Huzzah.get_remote_port())).c_str());
      if (Huzzah.server) // we are in server mode, kill it then
        Huzzah.stop_server();
      success = Huzzah.client.connect(Huzzah.get_remote_ip(),Huzzah.get_remote_port());
    }
    Serial.print("A ");
    Serial.print(FPSTR(ACT_CONNECT));
    if (success)
      Serial.println(" OK");
    else Serial.println(" NOK");
  } else if (strncmp_P(cmd+2,ACT_FLUSH,strlen_P(ACT_FLUSH))==0) {
    int nb_bytes = 0;
    if (Huzzah.client.connected()) {
      long unsigned int beg = millis();
      while ((tx_to_send>0) && (millis()<beg+5000))   // FIXME: flush timeout hardcoded as 5s
        nb_bytes += tx_buf_send();
    }
    Serial.print("A ");
    Serial.print(FPSTR(ACT_FLUSH));
    Serial.print(" ");
    Serial.print(nb_bytes);
    if (tx_to_send>0)
      Serial.println(" +");
    else
      Serial.println("");
  } else if (strncmp_P(cmd+2,ACT_LISTEN,strlen_P(ACT_LISTEN))==0) {
    Serial.print("A ");
    Serial.print(FPSTR(ACT_LISTEN));
    if (Huzzah.server_listen())
      Serial.println(" OK");
    else Serial.println(" NOK");
  } else {
    Serial.print("A ");
    Serial.println(FPSTR(ANS_NOK_UNK_CMD));
  }
}

void parse_command(char * cmd)
{
  bool result = false;
  switch (*cmd) {
    case 'S':
      set_command(cmd);
      break;
    case 'G':
      get_command(cmd);
      break;
    case 'A':
      action_commands(cmd);
      break;
  }
  cmd_pos = 0;
}

void loop() {
  if (Serial.available()) {
    if (digitalRead(CMD_PIN)==HIGH) {
      if (cmd_pos<CMD_L)
        cmd[cmd_pos++]=Serial.read();
        // End of line
        if ((cmd_pos>1) && (cmd[cmd_pos-1]==10) && (cmd[cmd_pos-2]==13)) {
          cmd[cmd_pos-2]='\0';
          parse_command(cmd);
        }
    } else {
      char c = Serial.read();
      if (Huzzah.client.connected()) {// Text to send, make sure we are connected
       if (tx_to_send<BUF_L) {  // its a ring buffer, make sure its not full
          tx_to_send++;
          tx_buf[tx_buf_end++]=c;
          if (tx_buf_end == BUF_L) // we reached the end of the buf let's start over from 0
            tx_buf_end = 0;
        }
      }
    }
  }
  // Check if we are in server mode and then check if a client connected
  if (Huzzah.server && !Huzzah.client.connected())
    Huzzah.client = Huzzah.server->available();
  if (Huzzah.client.connected() && Huzzah.client.available()) {
    // Write bytes as they arrive
    Serial.write(Huzzah.client.read());
  }
  // Check tx time out and if buffer is almost full
  if ((millis()-last_tx > tx_flush_timeout) || (tx_to_send*3>BUF_L*2 )) {
    last_tx = millis();
    tx_buf_send();
  }
}
