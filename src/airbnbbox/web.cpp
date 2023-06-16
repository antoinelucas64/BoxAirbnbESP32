#include "web.h"
#include "commands.h"

String headHtml = "<!DOCTYPE html> <html>"
                  "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
                  "<title>Appartement Lucas</title>\n"
                  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
                  "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n"
                  ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n"
                  ".button-on {background-color: #1abc9c;}\n"
                  ".button-on:active {background-color: #16a085;}\n"
                  ".button-off {background-color: #34495e;}\n"
                  ".button-off:active {background-color: #2c3e50;}\n"
                  "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n"
                  "body {	padding: 20px;	}"
                  "label {  "
                  "font-size: 17px;"
                  "font-family: sans-serif;"
                  "}"
                  "input {"
                  "	display: block;"
                  "	width: 300px;"
                  "	height: 40px;"
                  "	padding: 4px 10px;"
                  "	margin: 10 0 10 0;"
                  "	border: 1px solid #03A9F4;"
                  "	background: #cce6ff;"
                  "	color: #1c87c9;"
                  "	font-size: 17px;"
                  "}"
                  "}"
                  "</style>\n"
                  "</head>\n"
                  "<body>\n"
                  "<h1>Appartement Lucas</h1>\n"
                  "<h3>A & A</h3>\n";
String responseHTML = headHtml + "<p>Ouverture</p><a class=\"button button-on\" href=\"/open\">OUVRE</a>"
                                 "<p><a href='/admin'>Configure</a>"
                                 "</body></html>";

String Web::createAdminPage() {
  String out = headHtml + "<p>Wifi</p>"
                          "<p>SSID: "
               + config.getSSID() + "</p>"
                                    "<p>Password "
               + config.getPassword() + "</p><a class=\"button button-on\" href=\"/admin/wifi\">Modifier</a>"
                                        "<p>Admin</p>"
                                        "<p>Password "
               + config.getWWWPassword() + "</p><a class=\"button button-on\" href=\"/admin/password\">Modifier</a>"
                                           "</body>"
                                           "</html>";

  return out;
}

String Web::formWifiPage() {
  String out = headHtml + "<p>Wifi</p>"
                          "<form action=/updateWifi>"
                          "<label for='text1'>SSID</label>"
                          "<input type='text' id='ssid' name='ssid' value='"
               + config.getSSID() + "'/>"
                                    "<label for='text2'>Password</label>"
                                    "<input type='text' id='password' name='password' value='"
               + config.getPassword() + "'/>"
                                        " <button type='button' id='boot-btn' class='btn btn-primary'>Save</button>"
                                        " </form>"
                                        "</body>"
                                        "</html>";

  return out;
}

Web::Web(Config& config_p)
  : config(config_p),
    server(80) {
}



void Web::init() {

  server.on("/", [&]() {
    server.send(200, "text/html", responseHTML);
  });
  server.on("/open", [&]() {
    openDoor();
    server.send(200, "text/html", responseHTML);
  });
  server.on("/admin", [&]() {
    if (!server.authenticate(config.getWWWUser().c_str(), config.getWWWPassword().c_str())) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", createAdminPage());
  });
  server.on("/admin/wifi", [&]() {
    if (!server.authenticate(config.getWWWUser().c_str(), config.getWWWPassword().c_str())) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", formWifiPage());
  });
  server.onNotFound([&]() {
    server.send(200, "text/html", responseHTML);
  });

  Serial.println("Start web server");

  server.begin();
}


void Web::handleClient() {
  server.handleClient();
}
