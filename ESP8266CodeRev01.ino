/*  ESP8266 code to mix drinks in parallel for cocktails from web page 
    Copyright (C) 2015  D O'Connor

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

//Includes
#include <ESP8266WiFi.h>

//Wifi credentials
const char* ssid = "ASUS";
const char* password = "TIMETOGO";

//Globals
WiFiServer server(80);                    //HTTP port 80
byte pourstatus = 1;                      //Initilize to no glass (1)

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)   //If not connected
    delay(500);                           //Wait a little

  server.begin();                         //Then start listening
}

void loop()
{
  while (Serial.available())              //Update the status while clearing any old ones in the buffer
    pourstatus = Serial.read();

  WiFiClient client = server.available(); //Refresh client
  if (!client)                            //If no client, exit
    return;

  while (!client.available())             //While no data available
    delay(1); //Wait

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/update") != -1)         //Request is just for status update
  {
    //client.println(
    client.println(pourstatus, DEC);
  }
  else                                          //Request is for full page
  {
    if (request.indexOf("/drink") != -1)        //Request is a drink order
    {
      int i = request.indexOf("/drink") + 6;
      int x = request.indexOf(" ", i);
      Serial.println(request.substring(i, x));
    }
    
    //Response Header
    client.println
    (
      "HTTP/1.1 200 OK\r"
      "Content-Type: text/html\r\r"
      "<!DOCTYPE HTML>\r"
    );

    //Page Header
    client.println  //The HTML body can all go here
    (
      "<html>\r"
      "<head>\r"
      "<script src=\"http://ajax.aspnetcdn.com/ajax/jQuery/jquery-1.11.3.min.js\"></script>\r"
      "<script>"
      "$(document).ready(function() {\r"
      "$(\"#statusdiv\").load(\"/update/\");\r"
      "var refreshId = setInterval(function() {$(\"#statusdiv\").load(\"/update/\");}, 1000);\r"
      "$.ajaxSetup({ cache: false });});\r"
      "</script>"
      "</head>\r"
    );

    //Page Body
    client.println  //The HTML body can all go here
    (
      "<body>\r"
      "<div id=\"statusdiv\">"
      "</div>"
      "Click <a href=\"/drink19\">here</a> for a huge vodka<br>\r"
      "Click <a href=\"/drink4921\">here</a> for a martini<br>\r"
      "<img src=\"https://upload.wikimedia.org/wikipedia/commons/4/4f/Matterhorn_"
      "Riffelsee_2005-06-11.jpg\" alt=\"Mountain View\" style=\"width:304px;height:228px;\">\r"
      "</body>\r"
      "</html>\r"
    );
  };
  
  //Connection should be closed, but to be sure (I'm open to advice)
  client.stop();
}
