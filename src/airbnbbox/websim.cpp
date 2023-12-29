#include "rom/rtc.h"
#include "websim.h"



WebSim::WebSim(ConfigSim& config_p, Sim & sim_p)
  : Web(config_p),
    config(config_p),
    sim(sim_p) {
  Serial.println("WebSim (contructor)");
}


void WebSim::init() {
  Serial.println("phone " + config.getPhone());


  server.on("/power", [&]() {
    config.writePowerState(!config.getPowerState());
    //server.send(200, "text/html", responseHTML);
    server.send(200, "text/html", responseHTML());
  });

  server.on("/admin/phone", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }
    //server.send(200, "text/html", responseHTML);
    server.send(200, "text/html", formPhone());
  });
  server.on("/updatePhone", [&]() {
    if (!is_authenticated()) {
      server.sendHeader("Location", "/login");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
    }

    if (server.args() >= 1) {
      String newPhone = server.arg("phone");
      if (admin) {
        Serial.println("udpate phone " + newPhone);
        config.writePhone(newPhone);
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

  Web::init();
}

String WebSim::formPhone() {
  String out = headHtml() + "<p>Wifi</p>"
                            "<form action=\""
                            "/updatePhone\">"
                            "<label for='text2'>phone</label><br>"
                            "<input type='text' id='phone' name='phone' value='\n"
               + config.getPhone() + "'/>"
                                     " <input type='submit' name='SUBMIT' value='Submit'>"
                                     " </form>"
                                     "<p><a href=\"/\">Back</a>"
                                     "</body>"
                                     "</html>";

  return out;
}

String WebSim::responseHTML() {
  String btnPw = config.getPowerState() == POWER_ON ? "off" : "on";

  String out = headHtml() + "<p>Door</p><a class=\"button button-on\" href=\"/open\">OPEN</a>"
                            "<p>Power</p><a class=\"button button-"
               + btnPw + "\" href=\"/power\">";
  btnPw.toUpperCase();
  out += btnPw + "</a>"
                 "<p><a href=\""
                 "/admin\">Configure</a>"
                 "<p><a href=\""
                 "/reboot\">Reboot</a>"
                 "</body></html>";

  return out;
}

String WebSim::createAdminPage() {
  Serial.println("admin page with sim option");
  String out = headHtml() + "<p>Wifi</p>";
  if (admin) out += "<p>SSID: "
                    + config.getSSID() + "</p>";
  out += "<p>Password "
         + config.getPassword() + "</p><a class=\"button button-on\" href=\"/admin/wifi\">Modify</a>";

  if (admin) out += "<p>Web administration</p>"
                    "<p>User "
                    + config.getWWWUser() + "<p>Password "
                    + config.getWWWPassword() + "</p><a class=\"button button-on\" href=\"/admin/password\">Modify</a>"
                                                "<p>Phone</p>"
                                                "<p>phone "
                    + config.getPhone() + "</p><a class=\"button button-on\" href=\"/admin/phone\">Modify</a>";

  out += "<p><a href=\"/\">Back</a>"
         "</body>"
         "</html>";

  return out;
}

void WebSim::reboot(){
  sim.reboot();
}
