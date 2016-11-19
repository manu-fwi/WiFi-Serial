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

  HuzzahWiFi();

  void set_ssid(char * SSID)
  {
    strncpy(WiFi_SSID, SSID, WiFi_SSID_L);
  }
  void set_key(char * PASSWD)
  {
    strncpy(WiFi_PASSWD, PASSWD, WiFi_PASSWD_L);
  }
  void set_server_ip(IPAddress addr)
  {
    server_add_type = SERVER_ADD_IP;
    server_ip=addr;
  }
  void set_server_name(const char * ser_name)
  {
    strncpy(server_name,ser_name,WiFi_SERVER_NAME_L);
    server_add_type = SERVER_ADD_NAME;
  }
  void set_server_port(int _port)
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
  const char * get_server_name()
  {
    return server_name;
  }
  const IPAddress& get_server_ip()
  {
    return server_ip;
  }
  int get_server_port()
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
  
  private:
  char WiFi_SSID[WiFi_SSID_L+1];
  char WiFi_PASSWD[WiFi_PASSWD_L+1];

  char server_name[WiFi_SERVER_NAME_L+1];
  IPAddress server_ip;
  int port;
  int nb_networks;
  byte server_add_type;
};

extern HuzzahWiFi Huzzah;

#endif // WIFI_STATUS_H
