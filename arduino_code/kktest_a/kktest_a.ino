//a
#include <WiFi.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include "ECC.h"
#include "AES.h"
#define LED_PIN LED_BUILTIN
uint64_t ecc_key[16];
uint8_t s_msg[16];
uint8_t r_msg[16];
uint64_t s_box[256]={0}; uint64_t invs_box[256]={0};uint64_t nk=4;uint64_t nr=10; uint64_t rc[10]={0}; uint64_t w[ (10+1) * 4] ={0};
using namespace std;

//local
const char* ssid = "kkESP32";
const char* password = "987654321";
WiFiServer server(3559);

//connect
const char* c_SSID = "shESP32";
const char* c_Password = "987654321";
const int c_ServerPort = 3559;
WiFiClient client;

void show_wifi_info(){
    Serial.print("connect IP->");
    Serial.println(WiFi.localIP());
    Serial.print("connect ssid->");
    Serial.println(WiFi.SSID());
    Serial.print("My IP->");
    Serial.println(WiFi.softAPIP());
}

void check_connect(){
    int ck=1;
    if(WiFi.status() != WL_CONNECTED){
        ck=0;
        led_toggle();
        Serial.println("Wifi disconnected,reconnect...");
        WiFi.begin(c_SSID, c_Password);
        delay(100);
    }
    else{
        led_on();
    }

    if(ck==0 && WiFi.status() == WL_CONNECTED){
        Serial.println("\nReconnect OK\n");
        show_wifi_info();
        ecc_connect();
        AES_setting();
        ck=1;
    }
}

void led_on(){
    digitalWrite(LED_PIN, HIGH);
}

void led_off(){
    digitalWrite(LED_PIN, LOW);
}

void led_toggle() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

//ECC area
String ecc_receive_msg(){
    //a to b
    Serial.println("Waiting for message from B...");
    while (!server.hasClient()) {
        delay(100);   // 等待B建立連接
    }
    String message;
    WiFiClient client = server.available();
    if (client) {
        while (client.connected()) {
            if (client.available()) {
                message = client.readStringUntil('\n');
                Serial.print("[ECC] ");
                Serial.print(WiFi.SSID());
                Serial.print(" : ");
                Serial.println(message);
                break;
            }
        }
        client.stop();
    }
    message.remove(message.length()-1);   //Delete "\n"
    return message;
}

void ecc_send_msg(String message){
    //a to b
    message.trim();
    if (!message.isEmpty()) {
        Serial.println("[ECC] Sending to B:" + message);
        if (client.connect(WiFi.gatewayIP(), c_ServerPort)) {
            client.println(message);
            client.stop();
        } else {
            Serial.println("Connection to B failed");
        }
    }
}

void ecc_cut_key(String hex_string,uint64_t ecc_key[], mbits K){
    String hex_string1;
    int count=0;

    /* test message
    Serial.print("hex_str-->");
    Serial.println(hex_string);  //D*G
    */

    points DG;

    for(int i=0;i<3;i++){
        hex_string1 = hex_string.substring(hex_string.length()-16,hex_string.length());
        uint64_t num = strtoull(hex_string1.c_str(), NULL, 16);
        DG.y.a[i]=num;
        hex_string.remove(hex_string.length()-16);
    }

    for(int i=0;i<3;i++){
        hex_string1 = hex_string.substring(hex_string.length()-16,hex_string.length());
        uint64_t num = strtoull(hex_string1.c_str(), NULL, 16);
        DG.x.a[i]=num;
        hex_string.remove(hex_string.length()-16);
    }

    /* test message
    Serial.print("DG:");
    for(int i=0;i<3;i++)
        Serial.println(DG.x.a[i],HEX);
    */
    points KDG=scalarmA2(K,DG);
    //showword(KDG.x);
    //showword(KDG.y);

    for(int i=0;i<2;i++){
        count=0;
        for(int j=8*i;j<8*(i+1);j++){
        ecc_key[j]=((KDG.x.a[i]>>(8*count))&0xff);
        count++;
        }
    }
    /* test message(show ecc key)*/
    for(int i=0;i<16;i++){
        /*Serial.print("ecc_key[");
        Serial.print(i);
        Serial.print("] = ");*/
        Serial.print(ecc_key[i],HEX);
        Serial.print(",");
    }
    
}

uint64_t getRandom64(){
    uint64_t high = (uint64_t)random(0x7FFFFFFF) << 32;
    uint64_t low = (uint64_t)random(0x7FFFFFFF);
    return high | low;
}

uint64_t getRandom35(){
    uint64_t high = (uint64_t)random(7) << 32;
    uint64_t low = (uint64_t)random(0x7FFFFFFF);
    return high | low;
}

void ecc_connect(){
    points  G,Q,D;
    G.x={0xDE4E6D5E5C94EEE8,0x7BBC11ACAA07D793,0X2FE13C053};
    G.y={0x0536D538CCDAA3D9,0x5D38FF58321F2E80,0X289070FB0};
    mbits K;
    //Serial.print("Random 64-bit number: 0x");
    for(int i=0;i<2;i++){
        uint64_t rand = getRandom64();
        K.a[i]=rand;
      //Serial.print(rand,HEX);
    }

    uint64_t rand1 = getRandom35();
    K.a[2]=rand1;
    /* test message
    Serial.println(rand1,HEX);
    for(int i=0;i<3;i++){
        Serial.print("K[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(K.a[i],HEX);
    }
    */

    D=scalarmA2(K,G);
    /*
    showword(D.x);
    showword(D.y);
    */

    int h;
    String str1[]={"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};                   
    String str="";
    for(int i=wsize-1;i>=0;i--){
        for(int j=(63-3);j>=0;j=j-4)
        str=str+str1[(D.x.a[i]>>j)&0x0f];
    }
    for(int i=wsize-1;i>=0;i--){
        for(int j=(63-3);j>=0;j=j-4)
        str=str+str1[(D.y.a[i]>>j)&0x0f];
    }

    ecc_send_msg(str);
    String re_str2;     //接收到的D*G
    re_str2=ecc_receive_msg();

    //Serial.print("re_str2-->");
    //Serial.println(re_str2);
    ecc_cut_key(re_str2,ecc_key,K);
    Serial.println("ECC over-A");
}

//AES area
void AES_setting(){
    S_box_invS_box(s_box,invs_box);
    rcv(nr, rc);
    keyexpansion(ecc_key, w, nk, nr,rc,s_box);
}

void AES_s_cut_state(String text){
    //text="abcd123456";
    uint64_t utf8_bytes[16];
    int utf8_len = text.length();
    int index = 0;
    
    int current_byte_count = 0;  // 當前區塊中的總字節數

    // 將最多 16 字節的資料存入 utf8_bytes，確保不超出邊界
    while (index < utf8_len) {
        // 判斷當前字元的 UTF-8 編碼長度
        char current_char = text.charAt(index);
        int bytes_copied;
        if ((current_char & 0x80) == 0) {
            bytes_copied = 1;
        }
        else if ((current_char & 0xE0) == 0xC0) {
            bytes_copied = 2; 
        }
        else {
            bytes_copied = 3; 
        }

        // 如果當前字元填入後會超過16字節，則結束當前區塊
        if (current_byte_count + bytes_copied > 16) {
            break; // 超過16字節，結束這個區塊
        }

        // 將 UTF-8 字符拆分成 bytes 並存入 utf8_bytes
        if (bytes_copied == 1) {
            utf8_bytes[current_byte_count] = current_char;
        } else if (bytes_copied == 2) {
            utf8_bytes[current_byte_count] = current_char;
            index++; 
            utf8_bytes[current_byte_count + 1] = text.charAt(index);
        } else if (bytes_copied == 3) {
            utf8_bytes[current_byte_count] = current_char; 
            index++; 
            utf8_bytes[current_byte_count + 1] = text.charAt(index); 
            index++; 
            utf8_bytes[current_byte_count + 2] = text.charAt(index); 
        }
        // 更新當前字節計數與索引
        current_byte_count += bytes_copied;
        index++;  
    }
    
    // 顯示每個 8-bit 塊的 16 進制數值
    Serial.println("UTF-8 encoded values (16 bytes):");
    for (int i = 0; i < current_byte_count; i++) {
    Serial.print("0x");
    if (utf8_bytes[i] < 0x10) Serial.print("0");  // 補零確保兩位數
    Serial.print(utf8_bytes[i], HEX);
    Serial.print(" ");
    }
    Serial.println();

/*
    // 將有效字節轉回字串
    String recoveredText = "";
    for (int i = 0; i < current_byte_count; i++) {
    recoveredText += (char)utf8_bytes[i];
    }
*/
    cipher(utf8_bytes, nr,s_box,w);
    Serial.print("[s_cut]Recovered text: ");
    Serial.println(text);
}

void AES_r_cut_state(String message){
    d0 = r_msg[0]  << 24 | r_msg[1]  << 16 | r_msg[2]  << 8 | r_msg[3];
    d1 = r_msg[4]  << 24 | r_msg[5]  << 16 | r_msg[6]  << 8 | r_msg[7];
    d2 = r_msg[8]  << 24 | r_msg[9]  << 16 | r_msg[10] << 8 | r_msg[11];
    d3 = r_msg[12] << 24 | r_msg[13] << 16 | r_msg[14] << 8 | r_msg[15];
    state[0] = d0;
    state[1] = d1;
    state[2] = d2;
    state[3] = d3;
    show(state);
}

string AES_Encryption(uint64_t state[]){
    cipher(state, nr,s_box,w);
    //show(state);
    // 將有效字節轉回字串
    string recoveredText = "";
    for (int i = 0; i < current_byte_count; i++) {
        recoveredText += (char)state[i];
    }
    Serial.print("[AES_Encry]Recovered text: ");
    Serial.println(recoveredText);
    return recoveredText;
}

void AES_Decryption(uint64_t state[]){
    invcipher(state, nr,invs_box,w);
    show(state);
    
    /*for (int i = 0; i < 16; i++) {
        // 檢查是否是非零的字節
        if (utf8_bytes[i] != 0) {
            message += (char)utf8_bytes[i];  // 轉回字符
        }
    }*/
    Serial.println("Reconstructed Text:");
    //Serial.println(message);
}

//message receive and send
void receive_msg(){
    //b to a
    WiFiClient client = server.available();
    if (client) {
        //Serial.println("Client connected");
        while (client.connected()) {
            if (client.available()) {
                String message = client.readStringUntil('\n');
                Serial.print(WiFi.SSID());
                Serial.print(" : ");
                Serial.println(message);
                AES_r_cut_state(message);
            }
        }
        client.stop();
        //Serial.println("Client disconnected");
    }
}

void send_msg(){
    //a to b
    if (Serial.available() > 0) {
        String message = Serial.readStringUntil('\n');
        message.trim();
        if (!message.isEmpty()) {
            Serial.println("Sending to B:" + message);
            if (client.connect(WiFi.gatewayIP(), c_ServerPort)) {
                client.println(AES_Encryption(AES_s_cut_state(message)));
            } else {
                Serial.println("Connection to B failed");
            }
        }
    }
}

void setup() {
    Serial.begin(115200); 
    //ECC maketable
    makepowt(1,anT1)    ;
    makepowt(2,anT2)    ;
    makepowt(5,anT5)    ;
    makepowt(10,anT10)  ;
    makepowt(20,anT20)  ;
    makepowt(40,anT40)  ;
    WiFi.softAP(ssid, password);
    WiFi.begin(c_SSID, c_Password);

    pinMode(LED_PIN, OUTPUT);
    led_off();

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to B WiFi...");
    }

    show_wifi_info();
    server.begin();
    ecc_connect();
    AES_setting();
    Serial.println("setup OK");
}

void loop() {
    check_connect();
    receive_msg();
    delay(10);
    send_msg();    
    delay(10);
}
