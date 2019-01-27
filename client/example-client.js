/**
 * This a minimal fully functional example for setting up a client written in JavaScript that
 * communicates with a server via WebRTC data channels. This uses WebSockets to perform the WebRTC
 * handshake (offer/accept SDP) with the server. We only use WebSockets for the initial handshake
 * because TCP often presents too much latency in the context of real-time action games. WebRTC
 * data channels, on the other hand, allow for unreliable and unordered message sending via SCTP
 */

// URL to the server with the port we are using for WebSockets.
const webSocketUrl = 'ws://localhost:8080';

// The WebSocket object used to manage a connection.
let webSocketConnection = null;

// The data channel used to communicate.
let dataChannel = null;

const pingTimes = {};
const pingLatency = {};
let pingCount = 0;
const PINGS_PER_SECOND = 20;
const SECONDS_TO_PING = 20;
let pingInterval;
let startTime;

const PROTOCOL_VERSION = "0"; // TODO: use it

const WS_PING_OPCODE = "0";

function sleep(delay) {
    var start = new Date().getTime();
    while (new Date().getTime() < start + delay);
}

/*
 * Callback for when the WebSocket is successfully opened.
 * Then we send a socket.io message to the server,
 * so that it knows to prepare a peerconnection for us to connect to.
**/
function onWebSocketOpen() {
  console.log("onWebSocketOpen ")
}

// Callback for when we receive a message from the server via the WebSocket.
function onWebSocketMessage(event) {
  console.log("onWebSocketMessage")
  let messageObject = event.data;
  if (messageObject.length < 2) {
    console.log('Invalid messageObject ', messageObject);
    return;
  }

  let type = messageObject.charAt(0);
  let payload = messageObject.substr(1, messageObject.length);

  console.log("onWebSocketMessage type =", type, ";payload= ", payload)
  if (type === WS_PING_OPCODE) {
    const key = messageObject.payload;
    pingLatency[key] = performance.now() - pingTimes[key];
  } else {
    console.log('Unrecognized WebSocket message type.', messageObject);
  }
}

function loopСonnect() {
    // NOTE: limit for chromium is set to 256 https://stackoverflow.com/a/36904475/10904212
    for (let i = 0; i < 200; i++) {
    connect();
    // sleep(10);
    }
}

// Connects by creating a new WebSocket connection and associating some callbacks.
function connect() {
    webSocketConnection = new WebSocket(webSocketUrl);
    /*webSocketConnection = new WebSocketClient({
                                            httpServer: webSocketUrl//,
                                            //maxReceivedFrameSize: 131072,
                                            //maxReceivedMessageSize: 10 * 1024 * 1024,
                                            //autoAcceptConnections: false
                                        });*/
    webSocketConnection.onopen = onWebSocketOpen;
    webSocketConnection.onmessage = onWebSocketMessage;
}

// Connects by creating a new WebSocket connection and associating some callbacks.
function connectWSOnly() {
    webSocketConnection = new WebSocket(webSocketUrl);
    /*webSocketConnection = new WebSocketClient({
                                          httpServer: webSocketUrl//,
                                          //maxReceivedFrameSize: 131072,
                                          //maxReceivedMessageSize: 10 * 1024 * 1024,
                                          //autoAcceptConnections: false
                                      });*/
    webSocketConnection.onmessage = onWebSocketMessage;
}

function disconnectWSOnly() {
  webSocketConnection.onclose = function () {}; // disable onclose handler first
  webSocketConnection.close();
  //webSocketConnection = null;
  //webSocketConnection.onmessage = null;
}

function printLatency() {
  for (let i = 0; i < PINGS_PER_SECOND * SECONDS_TO_PING; i++) {
    console.log(i + ': ' + pingLatency[i + '']);
  }
}

function sendWebSocketPing() {
  const key = pingCount + '';
  pingTimes[key] = performance.now();
  pingWebSocketOnce(key);
  pingCount++;
  if (pingCount === PINGS_PER_SECOND * SECONDS_TO_PING) {
    clearInterval(pingInterval);
    console.log('total time: ' + (performance.now() - startTime));
    setTimeout(printLatency, 10000);
    pingCount = 0;
  }
}

// Pings the server via the DataChannel once the connection has been established.
function pingWebSocket() {
  startTime = performance.now();
  pingInterval = setInterval(sendWebSocketPing, 1000.0 / PINGS_PER_SECOND);
}

// Pings the server via the DataChannel once the connection has been established.
function pingWebSocketOnce(key) {

    let message = WS_PING_OPCODE + key;
    //webSocketConnection.send(JSON.stringify({type: WS_PING_OPCODE, payload: key}));
    webSocketConnection.send(message);
}
// Pings the server via the DataChannel once the connection has been established.
function sendWebSocketCustom(key) {
    var x = document.getElementById("pingcustom").value;
    let message = WS_PING_OPCODE + x;
    webSocketConnection.send(message);
}
