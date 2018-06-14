const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<div id="demo">
<h1>The ESP8266 NodeMCU AJAX</h1>
	<button type="button" onclick="sendData1(1)">LED 1 ON</button>
	<button type="button" onclick="sendData1(0)">LED 1 OFF</button><BR>
  <button type="button" onclick="sendData2(1)">LED 2 ON</button>
  <button type="button" onclick="sendData2(0)">LED 2 OFF</button><BR>
</div>

<div>
	ADC Value is : <span id="ADCValue">0</span><br>
    LED 1 State is : <span id="LEDState1">NA</span><br>
    LED 2 State is : <span id="LEDState2">NA</span>
</div>

<script>
function sendData1(led1) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState1").innerHTML = this.responseText;
      
    }
  };
  xhttp.open("GET", "setLED1?LEDstate1="+led1, true);
  
  xhttp.send();
}

function sendData2(led2) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      
      document.getElementById("LEDState2").innerHTML = this.responseText;
    }
  };
  
  xhttp.open("GET", "setLED2?LEDstate2="+led2, true);
  xhttp.send();
}


setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ADCValue").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "readADC", true);
  xhttp.send();
}
</script>

</body>
</html>
)=====";
