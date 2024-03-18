#include "rom/rtc.h"
#include "websim.h"
#define ADD_PHONE "Add"
#define REMOVE_PHONE "Remove"

WebSim::WebSim(ConfigSim& config_p, Sim& sim_p)
  : Web(config_p),
    config(config_p),
    sim(sim_p) {
  Serial.println("WebSim (contructor)");
}

int readExpectedSize(WebServer& server, std::vector<String>& phones) {
  int size = phones.size();
  Serial.print("size config ");
  Serial.println(size);
  if (size == 0) size = 1;
  String sizeStr = server.arg("SIZE");
  if (!sizeStr.isEmpty()) {
    size = atoi(sizeStr.c_str());
  }
  String submit = server.arg("SUBMIT");
  if (submit == ADD_PHONE) {
    size++;
  }
  if (submit == REMOVE_PHONE && size > 1) size--;
  return size;
}


void WebSim::init() {
  for (int i = 0; i < config.getPhones().size(); i++) {
    Serial.println("phone " + config.getPhones()[i]);
  }


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

      String submit = server.arg("SUBMIT");
      if (admin) {
        std::vector<String> lastPhones = config.getPhones();
        int expectedSize = readExpectedSize(server, lastPhones);
        std::vector<String> phones;
        int i = 0;
        while (true) {
          char phoneId[12];
          sprintf(phoneId, "phone_%d", i);
          String newPhone = server.arg(phoneId);
          if (newPhone.isEmpty()) break;
          Serial.println("udpate phone " + newPhone);
          phones.push_back(newPhone);
          i++;
        }
        if (phones.size() > expectedSize) {
          phones.resize(expectedSize);
        }
        config.writePhones(phones);
        if (submit == ADD_PHONE || submit == REMOVE_PHONE) server.send(200, "text/html", formPhone());
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
  std::vector<String> phones = config.getPhones();
  int size = readExpectedSize(server, phones);


  String out = headHtml() + "<p>Phone</p>"
                            "<form action=\""
                            "/updatePhone\">";

  for (int i = 0; i < size; i++) {
    char phoneId[12];
    sprintf(phoneId, "'phone_%d'", i);
    out += "<label for='text2'>phone</label><br>"
           "<input type='text' id='phone' name=";
    out += phoneId;
    out += " value='\n";
    if (i < phones.size()) out += phones[i];
    out += "'/>";
  }
  out += " <input type='hidden' name='SIZE' value=";
  out += size;
  out += ">"
         " <input type='submit' name='SUBMIT' value='" ADD_PHONE
         "'>"
         " <input type='submit' name='SUBMIT' value='" REMOVE_PHONE
         "'><br>"
         " <input type='submit' name='SUBMIT' value='Submit'>"
         " </form>"
         "<p><a href=\"/\">Back</a>"
         "</body>"
         "</html>";

  return out;
}

String WebSim::responseHTML() {
  String btnPw = config.getPowerState() == POWER_ON ? "on": "off" ;

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

  if (admin) {
    out += "<p>Web administration</p>"
           "<p>User "
           + config.getWWWUser() + "<p>Password "
           + config.getWWWPassword() + "</p><a class=\"button button-on\" href=\"/admin/password\">Modify</a>"
                                       "<p>Phone</p>"
                                       "<p>phone ";
    for (int i = 0; i < config.getPhones().size(); i++) {
      if (i) out += " ";
      out += config.getPhones()[i];
    }
    out += "</p><a class=\"button button-on\" href=\"/admin/phone\">Modify</a>";
  }
  out += "<p><a href=\"/\">Back</a>"
         "</body>"
         "</html>";

  return out;
}

void WebSim::reboot() {
  sim.reboot();
}
