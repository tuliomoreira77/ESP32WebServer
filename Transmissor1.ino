#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}

#define WIFI_CHANNEL 1
uint8_t remoteMac[] = {0x18, 0xFE, 0x34, 0xFE, 0xCF, 0x5F}; //Esp01
uint8_t remoteMac1[] = {0x18, 0xFE, 0x34, 0xF3, 0x6A, 0xBA}; //Node1 Marcado
uint8_t remoteMac2[] = {0x18, 0xFE, 0x34, 0xF3, 0x63, 0xB2}; //Node2 Sem marca
byte cnt=0;
int controle = 0;

String readContent(uint8_t* data, uint8_t len)
{
  String rc = "";
  for(int i=0;i<len;i++)
  {
    char c = data[i];
    rc = rc + c;
  }
  return rc;
}

void criaPacote(uint8_t* pckt, String analog1, String analog2)
{
  String a = "+";
  a = a + analog1;
  a = a + "+";
  a = a + analog2;
  char c[10];
  a.toCharArray(c,10);
  for(int i=0;i<10;i++)
  {
    pckt[i] = c[i];
  }
}

void sendContent(uint8_t* mac)
{
  int anl1 = 0;
  int anl2 = 0;
  float a = 0;
  int potA = leituraAnalogica(0);
  if(potA < 30)
    potA = 0;
  if(potA > 900)
    potA = 1023;
  int potB = leituraAnalogica(1);
  if(potB < 500 && potB > 400)
  {
    potB = 450;
    anl1 = potA;
    anl2 = potA;
  }
  if(potB > 900)
  {
    potB = 900;
  }
  if(potB <= 400)
  {
    a = map(potB,0,400,0,100);
    anl1 = potA * (a/100.0);
    anl2 = potA;
  }
  if(potB >= 500)
  {
    a = map(potB,500,900,100,0);
    anl1 = potA;
    anl2 = potA * (a/100.0);
  }
  uint8_t dados[10];
  criaPacote(dados,String(anl1),String(anl2));
  char macString[50] = {0};
  esp_now_send(mac, dados, 10);
}

int leituraAnalogica(int porta)
{
  if(porta == 0)
  {
    digitalWrite(D0,HIGH);
    digitalWrite(D1,LOW);
    delay(10);
    return analogRead(A0);
  }
  if(porta == 1)
  {
    digitalWrite(D0,LOW);
    digitalWrite(D1,HIGH);
    delay(10);
    return analogRead(A0);

  }
}

void receiveData(uint8_t *mac, uint8_t *data, uint8_t len)
{
  int x = readContent(data,len).toInt();
  Serial.print("Tensao na bateria: ");
  Serial.println(x);
  if(x < 2100)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void setup() {
  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  digitalWrite(D0,LOW);
  digitalWrite(D1,LOW);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0)
  {
    ESP.restart();
  }
  delay(10);
  
  Serial.print("This node AP mac: "); Serial.print(WiFi.softAPmacAddress());
  Serial.print(", STA mac: "); Serial.println(WiFi.macAddress());

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_CONTROLLER, WIFI_CHANNEL, NULL, 0);
  esp_now_register_recv_cb(receiveData);
}

void loop() {
  sendContent(remoteMac);
  delay(100);
}
