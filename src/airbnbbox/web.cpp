#include "web.h"
#include "commands.h"

bool anthenticate = false;
int timeRef = 0;

String Web::headHtml() {
  return "<!DOCTYPE html> <html>"
         "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
         "<title>Appartement Lucas</title>\n"
         "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
         "body{margin-top: 50px;} \n"
         "h1 {color: #444444;margin: 50px auto 30px;} \n"
         "h3 {color: #444444;margin-bottom: 50px;}\n"
         ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n"
         ".button-on {background-color: #1abc9c;}\n"
         ".button-on:active {background-color: #16a085;}\n"
         ".button-off {background-color: #34495e;}\n"
         ".button-off:active {background-color: #2c3e50;}\n"
         "p {font-size: 14px;color: #888;}\n"
         "label {  "
         "font-size: 17px;"
         "font-family: sans-serif;"
         "}\n"
         "input {"
         "	border: 1px solid #16a085;"
         "	background: #1abc9c;"
         "	color: #2c3e50;"
         "	font-size: 17px;"
         "}\n"
         "</style>\n"
         "</head>\n"
         "<body>\n"
         "<h1>"
         + config.getSSID() + "</h1>\n"
                              "<h3>A & A</h3>\n";
}

String Web::responseHTML() {

  return headHtml() + "<p>Door</p><a class=\"button button-on\" href=\"/open\">OPEN</a>"
                      "<p><a href=\""
         + "/admin\">Configure</a>"
                "<p><a href=\""
         + "/reboot\">Reboot</a>"
                "</body></html>";
}

String Web::incorrectPassword() {

  return headHtml() + "Error - Incorrect password"
                      "<p><a href=\""
         + "/\">back</a>"
                "<p><a href=\""
         + "/reboot\">Reboot</a>"
                "</body></html>";
}

String Web::createAdminPage() {
 Serial.println("admin web page");

  String out = headHtml() + "<p>Wifi</p>";
  if (admin) out += "<p>SSID: "
                    + config.getSSID() + "</p>";
  out += "<p>Password "
         + config.getPassword() + "</p><a class=\"button button-on\" href=\"/admin/wifi\">Modify</a>";

  if (admin) out += "<p>Web administration</p>"
                    "<p>User "
                    + config.getWWWUser() + "<p>Password "
                    + config.getWWWPassword() + "</p><a class=\"button button-on\" href=\"/admin/password\">Modify</a>";

  out += "<p><a href=\"/\">Back</a>"
         "</body>"
         "</html>";

  return out;
}

// Check if header is present and correct
bool Web::is_authenticated() {
  Serial.println("Enter is_authenticated");
  if ((millis() - timeRef) > 120000) {
    anthenticate = false;
    admin = false;
  }
  if (anthenticate) return true;

  Serial.println("Authentication Failed");
  return false;
}

// login page, also called for disconnect
void Web::handleLogin() {
  String msg;
  Serial.println("Handle login");

  if (server.hasArg("PASSWORD")) {
    Serial.println("user " + server.arg("user") + " vs " + config.getWWWUser());
    Serial.println("Password " + server.arg("PASSWORD") + " vs " + config.getWWWPassword());
    Serial.println("checked  " + server.arg("isadmin"));

    if (server.arg("isadmin") == "1") {
      if (server.arg("PASSWORD") == config.getWWWPassword() && server.arg("user") == config.getWWWUser()) {
        anthenticate = true;
        admin = true;
      }
    } else if (server.arg("PASSWORD") ==  config.getWWWPassword()) {
      anthenticate = true;
      admin = false;
    }

    if (anthenticate) {
      timeRef = millis();
      server.sendHeader("Location", "/admin");
      server.send(301);

      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = headHtml() + "\n<form action='/login' method='POST'>Log in<p> \n";
  content += "<input type='checkbox' name='isadmin' id='isadmin' value='1'>as admin<p>\n";
  content += "<input type='TEXT' name='user'  id='user' placeholder='username'><p>\n";
  content += "<input type='password' name='PASSWORD' placeholder='password'><p>\n";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>\n";
  content += "<script language='javascript'>\n";
  content += "<!--\n";
  content += "document.getElementById('user').hidden = true; \n";
  content += "document.getElementById('isadmin').addEventListener('click',\n";
  content += "() => {\n";
  content += "document.getElementById('user').hidden = !document.getElementById('isadmin').checked; \n";
  content += "},false,);\n";
  content += "-->\n";
  content += "</script>\n";

  content += "You also can go <a href='/'>back</a></body></html>\n";
  server.send(200, "text/html", content);
}

String Web::formWifiPage() {
  String out = headHtml() + "<p>Wifi</p>"
                            "<form action=\""
               + "/updateWifi\">";
  if (admin) out += "<label for='text1'>SSID</label><br>"
                    "<input type='text' id='ssid' name='ssid' value='\n"
                    + config.getSSID() + "'/>";
  out += "<label for='text2'>Password</label><br>"
         "<input type='text' id='password' name='password' value='\n"
         + config.getPassword() + "'/>"
                                  " <input type='submit' name='SUBMIT' value='Submit'>"
                                  " </form>"
                                  "<p><a href=\"/\">Back</a>"
                                  "</body>"
                                  "</html>";

  return out;
}

String Web::formAdminPage() {
  String out = headHtml() + "<p>Web admin</p>"
                            "<form action=\""
               + "/updateAdmin\">";

  out += "<label for='text1'>User</label><br>"
                    "<input type='text' id='user' name='user' value='\n"
                    + config.getWWWUser() + "'/>";

  out += "<label for='text2'>Password</label><br>"
         "<input type='text' id='password' name='password' value='\n"
         + config.getWWWPassword() + "'/>"
                                     " <input type='submit' name='SUBMIT' value='Submit'>"
                                     " </form>"
                                     "<p><a href=\"/\">Back</a>"
                                     "</body>"
                                     "</html>";

  return out;
}

Web::Web(Config& config_p)
  : server(80),
    admin(false),
    config(config_p),
    wantReboot(false) {
}

bool Web::haveToReboot() {
  return wantReboot;
}

String Web::info() {
  if(admin)return config.info();
  return "type=\tESP8266-1\nSSID=\t"+config.getSSID() + "\nPassword=\t" + config.getPassword() + "\nWeb password=\t" + config.getWWWPassword() + "Version=\t 1\nDate=\t" + __DATE__ + "\n";
}

void Web::init() {
  Serial.println("wifi " + config.getPassword());
  Serial.println("admin " + config.getWWWPassword());

  server.on("/", [&]() {
    server.send(200, "text/html", responseHTML());
  });
  server.on("/open", [&]() {
    openDoor();
    server.send(200, "text/html", responseHTML());
  });
  server.on("/admin", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    server.send(200, "text/html", createAdminPage());
  });
  server.on("/login", [&]() {
    handleLogin();
  });

  server.on("/admin/wifi", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    //server.send(200, "text/html", responseHTML);
    server.send(200, "text/html", formWifiPage());
  });
  server.on("/admin/password", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    //server.send(200, "text/html", responseHTML);
    server.send(200, "text/html", formAdminPage());
  });
  server.on("/updateWifi", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }

    if (server.args() >= 2) {
      String newPassword = server.arg("password");
      if (strlen(newPassword.c_str()) > 7 && admin) {
        Serial.println("udpate ssid,password to " + server.arg("ssid") + " " + newPassword);
        config.writeConfig(server.arg("ssid"), newPassword);
        wantReboot = true;
      } else if (strlen(newPassword.c_str()) > 7) {
        Serial.println("udpate ssid,password to " + server.arg("ssid") + " " + newPassword);
        config.writeConfig(newPassword);
        wantReboot = true;
      } else {
        server.send(200, "text/html", incorrectPassword());
      }
    }

    for (int i = 0; i < server.args(); i++) {
      String message = "Number of args received:";
      message += "Arg nº" + (String)i + " –> ";
      message += server.argName(i) + ": ";
      message += server.arg(i) + "\n";
      Serial.println(message);
    }
    server.send(200, "text/html", responseHTML());
    //server.send(200, "text/html", formWifiPage());
  });



  server.on("/updateAdmin", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    if (server.args() >= 1) {
      String newPassword = server.arg("password");
      if (strlen(newPassword.c_str()) > 3) {
        Serial.println("udpate www password to " + newPassword + "user to "+ server.arg("user"));
        config.writeWWWConfig( server.arg("user"),newPassword);
      }
    }

    for (int i = 0; i < server.args(); i++) {
      String message = "Number of args received:";
      message += "Arg nº" + (String)i + " –> ";
      message += server.argName(i);
      message += server.arg(i) + "\n";
      Serial.println(message);
    }
    server.send(200, "text/html", responseHTML());
    //server.send(200, "text/html", formWifiPage());
  });
  server.onNotFound([&]() {
    server.send(200, "text/html", responseHTML());
  });


  server.on("/reboot", [&]() {
    ESP.restart();
  });

  server.on("/info", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    server.send(200, "text/html", info());
  });

  Serial.println("Start web server");

  server.begin();
}


void Web::handleClient() {
  server.handleClient();
}
