const char * INDEX_HTML = R"(
  <!DOCTYPE html><html><head><title>Carrito</title><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1'>
  <style>
    * {font-family: monospace;}
    body{background: black; color: white;}
    button {background: #5e5; color: black; font-weight: bold; font-size:40px; display: flex; flex-grow: 1; height: 200px;text-align: center; display: inline-block;border: 0px solid; margin: 10px; border-radius: 20px; }
    p {display: flex;}
    .no-select {
      -webkit-user-select: none; /* Safari */
      -ms-user-select: none; /* IE 10 and IE 11 */
      user-select: none; /* Standard syntax */
    }
    p.main {padding-top: 240px;}
    #logs {font-family:monospace;}
  </style>
  </head><body>
  <script>
  var SERVO_CHN = 1;
  var M_CHN = 10;
  var SERVO_PIN = 33;
  var M_PIN = 32

  function setupPWM(channel, freq, res, pin) {
    var ajaxP = new XMLHttpRequest();
    ajaxP.open('GET', '/pinPWM?channel='+channel+'&pin='+pin, true);
    ajaxP.send();

    var ajax = new XMLHttpRequest();
    ajax.open('GET', '/setupPWM?channel='+channel+'&freq='+freq+'&res='+res, true);
    ajax.send();
  }
  function writePWM(channel, duty) {
    var ajax = new XMLHttpRequest();
    ajax.open('GET', '/writePWM?channel='+channel+'&duty=' + duty, true);
    ajax.send();
  }
  function setKeyValue(key, value) {
    var ajax = new XMLHttpRequest();
    ajax.open('GET', '/keyStore/set?key='+key+'&value='+value);
    ajax.send();
  }
  function setup() {
    var ajax = new XMLHttpRequest();
    ajax.open('GET', '/keyStore/get?key=setup');
    ajax.onload = (e) => {
      if (ajax.responseText != 'done') {
        setupPWM(M_CHN, 10000, 8, M_PIN);
        setupPWM(SERVO_CHN, 50, 16, SERVO_PIN);
        writePWM(M_CHN, 0)
        writePWM(SERVO_CHN, 5000)
        setKeyValue('setup', 'done');
      }
    }
    ajax.send();
  }
  function log(message) {
    console.log("SEND LOG: " + message);
    var ajax = new XMLHttpRequest();
    ajax.open('GET', '/log?message='+btoa(message), true);
    ajax.send();
    document.getElementById("logs").innerHTML += "LOG: " + message + "<br>";
  }

  var touchStart = null;
  var touchCoordsMove = null;
  function resetTouch() {
    log('Reset touch');
    touchStart = null;
    touchCoordsMove = null;
    writePWM(M_CHN, 0);
    writePWM(SERVO_CHN, 5000)
  }
  function startMoving() {
    var e = event;
    touchStart = [e.targetTouches[0].pageX, e.targetTouches[0].pageY];
    log('Touch start ' + touchStart);
  }
  function touchMove() {
    var e = event;
    touchCoordsMove = [e.targetTouches[0].pageX, e.targetTouches[0].pageY];
  }
  var servoDutyPrev = null, motorDutyPrev = null;
  var touchHandle = function () {
    if (touchStart && touchCoordsMove) {
      var servoDuty = 5000 - (touchCoordsMove[0] - touchStart[0])*10;
      if (servoDuty < 1000) servoDuty = 1000;
      if (servoDuty > 10000) servoDuty = 10000;
      if (servoDuty != servoDutyPrev) {
        writePWM(SERVO_CHN, servoDuty);
        servoDutyPrev = servoDuty;
      }
      var motorDuty = Math.min(250, 100 + (touchStart[1] - touchCoordsMove[1])*1)
      if (motorDuty != motorDutyPrev && motorDuty > 100) {
        writePWM(M_CHN, motorDuty);
        motorDutyPrev = motorDuty
      }
    }
  }
  var touchTimer = setInterval( touchHandle, 250);

  function test() {
    log("test message");
    var ajax = new XMLHttpRequest();
    ajax.open('GET', '/keyStore/get?key=setup');
    ajax.onload = (e) => {
      log("Got keyStore value");
      log(ajax.responseText);
      var ajax2 = new XMLHttpRequest();
      ajax2.open('GET', '/keyStore/set?key=setup&value=test');
      ajax2.onload = (e) => {
        log("Set a keyStore value");
        log(ajax.responseText);
      }
      ajax2.send();
    }
    ajax.send();
  }

  window.addEventListener('load', function () {
    setup();
  })
  </script>
  <p class="no-select main"><button ontouchstart='startMoving()' ontouchend='resetTouch()' ontouchmove='touchMove()'></button> </p>
  <br><br><br><p></p>
  <div id="logs"></div>
  </body></html>

)";
