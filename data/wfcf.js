function sendConfig() {
    var ssid = document.getElementById("ssid").value;
    var pass = document.getElementById("pass").value;
    console.log("SSID: " + ssid);
    console.log("PASS: " + pass);
    if (ssid == "") {
        console.log("SSID is empty");
        ssid_elem.focus();
    } else if (pass == "") {
        console.log("PASS is emty");
        pass_elem.focus();
    } else if (pass.length < 8) {
        console.log("PASS too short");
        pass_elem.focus();
    }
    var http = new XMLHttpRequest();
    const url = "/configWifi";
    var json_data = JSON.stringify({ "ssid": ssid, "pass": pass });
    console.log("JSON will be send to server: " + json_data);
    http.open("POST", "/wificonfig", true);
    http.setRequestHeader("Content-type", "application/json");
    http.send(json_data);

    return
}