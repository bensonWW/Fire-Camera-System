// lwIP httpd 自訂檔案：
//   "/"、"/index.html" → 靜態網頁（只載入一次，內含 JS 輪詢）
//   "/data"           → JSON 即時資料（JS 每 0.5 秒抓一次，局部更新）
// 好處：煙霧/聲音/火災/隱私鎖 0.5 秒即時更新，溫濕度維持 2 秒更新（JS 控制）。
extern "C" {
#include "lwip/apps/fs.h"
}
#include "shared.h"
#include <stdio.h>
#include <string.h>

// 🎯【火災影像】火災觸發時顯示的圖片網址 —— 改成你自己的圖即可。
#define FIRE_IMAGE_URL "https://images.unsplash.com/photo-1508873699372-7aeab60b44ab?w=500"

// 靜態網頁（常數，放 flash）。數值留白，由 JS fetch('/data') 填入。
static const char PAGE[] =
"<!DOCTYPE html><html lang='zh-TW'><head>"
"<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>智慧消防監控系統</title><style>"
"body{font-family:'Segoe UI',Arial;background:#1a1a2e;color:#e0e0e0;margin:0;padding:20px;"
"display:flex;flex-direction:column;align-items:center;}"
".card{background:#16213e;border-radius:16px;padding:30px 50px;text-align:center;"
"box-shadow:0 4px 25px rgba(0,0,0,.5);border:3px solid #00cc66;width:340px;transition:border-color .3s;}"
"h1{color:#4a90d9;margin:0 0 10px;font-size:1.3em;}"
".state{font-size:1.25em;font-weight:bold;color:#00cc66;margin-bottom:18px;}"
".row{display:flex;justify-content:space-around;margin-bottom:18px;}"
".value{font-size:2.2em;font-weight:bold;}.temp{color:#fc8181;}.hum{color:#7dd3fc;}"
".label{font-size:.8em;color:#888;}"
".badges{display:flex;gap:10px;justify-content:center;margin-bottom:14px;}"
".badge{padding:4px 12px;border-radius:12px;font-size:.8em;background:#0f3460;color:#999;}"
".badge.on{background:#ff3333;color:#fff;font-weight:bold;}"
".cam{height:200px;border-radius:10px;display:flex;flex-direction:column;justify-content:center;"
"align-items:center;margin-top:8px;overflow:hidden;background:#000;color:#666;font-size:2.4em;}"
".cam span{font-size:.4em;letter-spacing:2px;}"
".cam img{width:100%;height:100%;object-fit:cover;}"
".caption{font-size:.8em;color:#777;margin-top:10px;}.caption.fire{color:#ff6b6b;font-weight:bold;}"
".ts{font-size:.65em;color:#555;margin-top:14px;}"
"</style></head><body>"
"<div class='card' id='card'>"
"<h1>🚨 智慧消防監控系統</h1>"
"<div class='state' id='state'>載入中…</div>"
"<div class='row'>"
"<div><div class='value temp' id='temp'>--°C</div><div class='label'>溫度</div></div>"
"<div><div class='value hum' id='hum'>--%</div><div class='label'>濕度</div></div>"
"</div>"
"<div class='badges'>"
"<span class='badge' id='smokeBadge'>💨 煙霧 --</span>"
"<span class='badge' id='soundBadge'>🔊 聲響 --</span>"
"</div>"
"<div class='cam' id='cam'>🔒<br><span>數位隱私鎖定中</span></div>"
"<div class='caption' id='cap'>連線中…</div>"
"<div class='ts' id='ts'></div>"
"</div>"
"<script>"
"var FIRE_IMG='" FIRE_IMAGE_URL "';var tick=0;"
"function setBadge(id,on,label){var e=document.getElementById(id);"
"e.className='badge'+(on?' on':'');e.innerText=label+' '+(on?'偵測到':'正常');}"
"function upd(d){"
"var s='🟢 正常',c='#00cc66';"
"if(d.fire){s='🔥 火災警報';c='#ff3333';}else if(d.valid&&d.t<5){s='❄ 溫度過低';c='#4a90d9';}"
"document.getElementById('state').innerText=s;document.getElementById('state').style.color=c;"
"document.getElementById('card').style.borderColor=c;"
"setBadge('smokeBadge',d.smoke,'💨 煙霧');setBadge('soundBadge',d.sound,'🔊 聲響');"
"var cam=document.getElementById('cam'),cap=document.getElementById('cap');"
"if(d.fire){cam.innerHTML=\"<img src='\"+FIRE_IMG+\"'>\";cap.className='caption fire';"
"cap.innerText='🚨 火災觸發！數位隱私鎖已解除';}"
"else{cam.innerHTML='🔒<br><span>數位隱私鎖定中</span>';cap.className='caption';"
"cap.innerText='無火災事件 — 畫面鎖死';}"
"if(tick%4===0){document.getElementById('temp').innerText=(d.valid?d.t.toFixed(1):'--')+'°C';"
"document.getElementById('hum').innerText=(d.valid?d.h.toFixed(1):'--')+'%';}"
"document.getElementById('ts').innerText='更新：'+d.ts+' ms ｜ Pico 2 W';tick++;}"
"function poll(){fetch('/data',{cache:'no-store'}).then(function(r){return r.json();}).then(upd).catch(function(){});}"
"setInterval(poll,500);poll();"
"</script></body></html>";

static char s_json[256];

extern "C" int fs_open_custom(struct fs_file *file, const char *name) {
    if (strcmp(name, "/") == 0 || strcmp(name, "/index.html") == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data  = PAGE;
        file->len   = (int)(sizeof(PAGE) - 1);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    if (strncmp(name, "/data", 5) == 0) {
        float t = gSharedData.valid ? gSharedData.temperature : 0.0f;
        float h = gSharedData.valid ? gSharedData.humidity    : 0.0f;
        bool  smoke = gSharedData.smoke;
        bool  fire  = (gSharedData.valid && gSharedData.temperature > TEMP_ALARM_HIGH) || smoke;
        int n = snprintf(s_json, sizeof s_json,
            "{\"t\":%.1f,\"h\":%.1f,\"smoke\":%d,\"sound\":%d,\"fire\":%d,\"valid\":%d,\"ts\":%lu}",
            t, h, smoke ? 1 : 0, g_sound_alert ? 1 : 0, fire ? 1 : 0,
            gSharedData.valid ? 1 : 0, (unsigned long)gSharedData.timestamp_ms);
        if (n < 0) return 0;
        memset(file, 0, sizeof(struct fs_file));
        file->data  = s_json;
        file->len   = (n < (int)sizeof s_json) ? n : (int)sizeof s_json - 1;
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    return 0;
}

extern "C" void fs_close_custom(struct fs_file *file) {
    LWIP_UNUSED_ARG(file);
}
