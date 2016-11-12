# WiFi-Serial
Turn an Adafruit Huzzah ESP8266 into a WiFi Serial converter 

You can connect any MCU to the Huzzah Serial port (115200 is the default baud rate) and send commands to connect to an AP and then to any server (IP/name : port).

To switch from command mode to byte stream mode (when you are connected to a server and only want to send/receive data from it): drive pin 5 to HIGH for command mode and to LOW for byte stream mode.

Command mode:

/*
 * Digital pin 5: HIGH -> command mode
 *                LOW ->  byte stream mode
 *                
 * Commands: Answers: "S OK" or "S NOK"
 * "S" commands to set different parameters
 * S SSID param_ssid(string)
 * S KEY param_key(string)
 * S IP_ADD param_ip_addr(IP Adress: static IP address)
 * S TRANSMIT type(int): set transmission type: ANSWER: S TRANSMIT OK 
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
 */