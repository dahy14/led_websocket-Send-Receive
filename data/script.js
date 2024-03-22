const output_led1 = document.getElementById("output-led1");
const output_led2 = document.getElementById("output-led2");
const output_led3 = document.getElementById("output-led3");

const led1Slider = document.getElementById("led1");
const led2Slider = document.getElementById("led2");
const led3Slider = document.getElementById("led3");

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

led1Slider.oninput = function () {
  UpdateSliderVal(led1Slider, output_led1);
};
led2Slider.oninput = function () {
  UpdateSliderVal(led2Slider, output_led2);
};
led3Slider.oninput = function () {
  UpdateSliderVal(led3Slider, output_led3);
};

function UpdateSliderVal(slider, output) {
  const value = slider.value;
  output.innerText = value;
  sendValue();
}

const outputLeds = document.querySelectorAll(".slider-value");
const sliders = document.querySelectorAll(".slider");
const pots = document.querySelectorAll(".pot");

// Init web socket when the page loads
window.addEventListener("load", onload);

function onload(event) {
  initWebSocket();
}

function getReadings() {
  websocket.send("getReadings");
}

function initWebSocket() {
  console.log("Trying to open a WebSocket connection…");
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
  console.log("Connection opened");
  getReadings();
  sendValue();
}

function onClose(event) {
  console.log("Connection closed");
  setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
  // console.log(event.data);
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);

  pots.forEach((pot, index) => {
    pot.innerHTML = myObj[keys[index]];
  });
}

// Function that sends the value of the sliders to the ESP32
function sendValue() {
  setTimeout(() => {
    const jsonString = JSON.stringify({
      led1: output_led1.innerText,
      led2: output_led2.innerText,
      led3: output_led3.innerText,
    });
    websocket.send(jsonString);
  }, 100);
}
