#ifndef HUIZZAHWIFI_H
#define HUZZAHWIFI_H

#define WiFi_SSID_L 30
#define WiFi_PASSWD_L 40
#define WiFi_SERVER_NAME_L 100

#define SERVER_ADD_NONE 0
#define SERVER_ADD_IP   1
#define SERVER_ADD_NAME 2


class HuzzahWiFi
{
  public:
  WiFiClient client;
  WiFiServer * server;

  HuzzahWiFi();

  void set_ssid(char * SSID)
  {
    strncpy(WiFi_SSID, SSID, WiFi_SSID_L);
  }
  void set_key(char * PASSWD)
  {
    strncpy(WiFi_PASSWD, PASSWD, WiFi_PASSWD_L);
  }
  void set_remote_ip(IPAddress addr)
  {
    server_add_type = SERVER_ADD_IP;
    remote_ip=addr;
  }
  void set_remote_name(const char * ser_name)
  {
    strncpy(remote_name,ser_name,WiFi_SERVER_NAME_L);
    server_add_type = SERVER_ADD_NAME;
  }
  void set_remote_port(int _port)
  {
    port = _port;
  }

  bool ap_connect();
  void ap_disconnect() {
    WiFi.disconnect();
  }
  int scanNetworks() {
    nb_networks = WiFi.scanNetworks();
  }
  int get_nb_networks() {
    return nb_networks;
  }
  const char * get_remote_name()
  {
    return remote_name;
  }
  const IPAddress& get_remote_ip()
  {
    return remote_ip;
  }
  int get_remote_port()
  {
    return port;
  }
  void set_server_add_type(byte type)
  {
    server_add_type = type;
  }
  byte get_server_add_type()
  {
    return server_add_type;
  }
  void stop_server()
  {
    if (server) {
      delete server;
      server = NULL;
    }
  }
  bool server_listen()
  {
    stop_server();
    if (port>0) {
      server = new WiFiServer(port);
      // Stop the current client
      client.stop();
      server->begin();
      return true;
    }
    return false;
  }
  
  private:
  char WiFi_SSID[WiFi_SSID_L+1];
  char WiFi_PASSWD[WiFi_PASSWD_L+1];

  char remote_name[WiFi_SERVER_NAME_L+1];
  IPAddress remote_ip;
  int port;           // remote port if server==NULL, listening port otherwise
  int nb_networks;
  byte server_add_type;

};

extern HuzzahWiFi Huzzah;

#endif // WIFI_STATUS_H
