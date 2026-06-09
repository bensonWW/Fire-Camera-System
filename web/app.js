// 🚀 換成台灣創客最愛用、超穩定的 EMQX 公共伺服器，走安全加密 Port 8084
const brokerUrl = 'wss://broker.emqx.io:8084/mqtt';
const topic = 'ntut/cs/fireproject'; 

const client = mqtt.connect(brokerUrl);
const mqttStatus = document.getElementById('mqttStatus');
const connectBadge = document.getElementById('connectBadge');

const camPanel = document.getElementById('camPanel');
const statusText = document.getElementById('statusText');
const soundStatus = document.getElementById('soundStatus');
const videoBox = document.getElementById('videoBox');
const tempVal = document.getElementById('tempVal');

client.on('connect', () => {
    mqttStatus.innerText = "🌐 成功連線至物聯網公共伺服器！";
    connectBadge.innerText = "已連線";
    connectBadge.style.color = "#00cc66";
    client.subscribe(topic);
});

client.on('message', (receivedTopic, message) => {
    const signal = message.toString();
    console.log("收到廣播訊號:", signal);
    
    if (signal === 'SOUND') triggerSound();
    else if (signal === 'FIRE') triggerFire();
    else if (signal === 'NORMAL') triggerNormal();
});

function publishSignal(status) {
    if(client.connected) {
        client.publish(topic, status);
    } else {
        alert("郵局尚未連線成功，請稍候再試！");
    }
}

function triggerSound() {
    camPanel.className = "camera-container alert-sound";
    statusText.innerText = "⚠️ 警告：偵測到異常聲響！";
    statusText.style.backgroundColor = "#ff9900";
    soundStatus.innerText = "🚨 偵測到噪音 (疑似呼救聲)";
    soundStatus.style.color = "#ff9900";
}

function triggerFire() {
    camPanel.className = "camera-container alert-fire";
    statusText.innerText = "🔥 💥 嚴重火災警報！";
    statusText.style.backgroundColor = "#ff3333";
    tempVal.innerText = "58.2";
    soundStatus.style.color = "#ff3333";
    videoBox.style.backgroundColor = "#555";
    videoBox.style.color = "#fff";
    videoBox.innerHTML = "<img src='https://images.unsplash.com/photo-1508873699372-7aeab60b44ab?w=400' style='width:100%; height:100%; border-radius:10px; object-fit:cover;'>";
}

function triggerNormal() {
    camPanel.className = "camera-container";
    statusText.innerText = "🟢 系統監控中 - 安全";
    statusText.style.backgroundColor = "#00cc66";
    soundStatus.innerText = "正常";
    soundStatus.style.color = "#ffffff";
    tempVal.innerText = "26.5";
    videoBox.style.backgroundColor = "#000000";
    videoBox.style.color = "#888";
    videoBox.innerHTML = "🔒 數位隱私鎖定中 (無火災事件)";
}