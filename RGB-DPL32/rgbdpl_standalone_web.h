#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include "rgbdpl_ldpl_patterns.h"

void processSerialCommand();
static WebServer localWeb(80);
static bool apActive=false, apRestartPending=false, apStopPending=false;
static uint32_t apIdleSince=0;
static float measuredVoltage=NAN;
class WebCommandOutput : public Print {
public:
    String text;
    size_t write(uint8_t value) override { if(text.length()<2048)text+=char(value);return 1; }
};
struct LdplProfileExtra { uint32_t magic; uint8_t r,g,b,reserved; };
struct LocalApConfig { uint32_t magic,timeout; char password[64]; };
static LocalApConfig apConfig={0x52474131,120,{0}};
static const int legacyEepromSize=sizeof(SystemConfig)+5*sizeof(UserProfile);
static const int apConfigAddress=legacyEepromSize+5*sizeof(LdplProfileExtra);
static const char* const ldplNames[]={"Off","Dot bounce","Fade trail","Rainbow dot",
 "KITT / KnightRider","Cylon","Beat bars","Breathe","Juggle","Sparkle","Comet",
 "Alarm","Fire","Logic sweep","Rainbow bars","Theater chase"};

void loadLdplProfileExtra(uint8_t slot) {
    LdplProfileExtra extra;
    EEPROM.get(legacyEepromSize+slot*sizeof(extra),extra);
    if(extra.magic==0x4C445031) ldpl.baseColor=CRGB(extra.r,extra.g,extra.b);
    else {
        const uint8_t legacyModes[]={7,14,0,1};
        currentProfile.ldplMode=currentProfile.ldplMode<4?legacyModes[currentProfile.ldplMode]:1;
        ldpl.baseColor=CRGB::Red;
    }
    if(currentProfile.ldplMode>15) currentProfile.ldplMode=1;
}
void saveLdplProfileExtra(uint8_t slot) {
    LdplProfileExtra extra={0x4C445031,ldpl.baseColor.r,ldpl.baseColor.g,ldpl.baseColor.b,0};
    EEPROM.put(legacyEepromSize+slot*sizeof(extra),extra);
}
bool saveApConfig() { EEPROM.put(apConfigAddress,apConfig);return EEPROM.commit(); }
void startLocalAp() {
    WiFi.mode(WIFI_AP);
    apActive=WiFi.softAP(RGBDPL_AP_NAME,apConfig.password[0]?apConfig.password:nullptr);
    apIdleSince=millis();
    if(apActive) localWeb.begin();
    commandOutput->println(apActive?F("AP ready: http://192.168.4.1"):F("ERROR: AP start failed"));
}
void stopLocalAp() { localWeb.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_OFF);apActive=false; }
bool standaloneCommand(const char* input) {
    String line(input);line.trim();String cmd=line.substring(0,line.indexOf(' ')<0?line.length():line.indexOf(' '));cmd.toUpperCase();
    String arg=line.indexOf(' ')<0?String():line.substring(line.indexOf(' ')+1);arg.trim();
    if(cmd=="APPW") {
        if(arg=="CLEAR") arg="";
        if((arg.length()>0&&arg.length()<8)||arg.length()>63){commandOutput->println(F("ERROR: APPW needs 8-63 chars or CLEAR"));return true;}
        LocalApConfig old=apConfig;
        memset(apConfig.password,0,sizeof(apConfig.password));arg.toCharArray(apConfig.password,sizeof(apConfig.password));
        if(!saveApConfig()){apConfig=old;commandOutput->println(F("ERROR: AP save failed"));return true;}
        apRestartPending=apActive;commandOutput->println(F("OK,APPW (reconnect to AP)"));return true;
    }
    if(cmd=="APTIMEOUT") {
        char* end=nullptr;const unsigned long value=strtoul(arg.c_str(),&end,10);
        if(arg.length()==0||*end||value>86400){commandOutput->println(F("ERROR: APTIMEOUT 0-86400"));return true;}
        const uint32_t old=apConfig.timeout;apConfig.timeout=value;
        if(!saveApConfig()){apConfig.timeout=old;commandOutput->println(F("ERROR: AP save failed"));}
        else {apIdleSince=millis();commandOutput->println(F("OK,APTIMEOUT"));}return true;
    }
    if(cmd=="AP") {
        arg.toUpperCase();if(arg=="ON") apRestartPending=true;
        else if(arg=="OFF") apStopPending=true;
        else commandOutput->println(F("ERROR: AP ON/OFF"));return true;
    }
    if(cmd=="LDPLCOLOR") {
        int r,g,b;char extra;
        if(sscanf(arg.c_str(),"%d %d %d %c",&r,&g,&b,&extra)!=3||r<0||r>255||g<0||g>255||b<0||b>255)commandOutput->println(F("ERROR: LDPLCOLOR R G B (0-255)"));
        else {ldpl.baseColor=CRGB(r,g,b);commandOutput->println(F("OK,LDPLCOLOR; SAVE to persist"));}return true;
    }
    return false;
}
String standaloneStatus() {
    char color[8];snprintf(color,sizeof(color),"#%02x%02x%02x",ldpl.baseColor.r,ldpl.baseColor.g,ldpl.baseColor.b);
    String s="{\"version\":\"" VERSION "\",\"ldplMode\":"+String(currentProfile.ldplMode)+",\"ldplColor\":\""+color+"\"";
    s+=",\"brightness\":"+String(currentProfile.brightness)+",\"speed\":"+String(currentProfile.animSpeed);
    s+=",\"cbiMode\":"+String(currentProfile.cbiMode)+",\"cbiSpeed\":"+String(currentProfile.cbiSpeed)+",\"topBlocks\":"+String(currentProfile.topBlocksMode);
    s+=",\"scheme\":"+String(currentProfile.colorScheme)+",\"personality\":"+String(currentProfile.personality)+",\"sequence\":"+String(currentProfile.sequenceMode?1:0);
    s+=",\"bargraph\":\""+String(currentProfile.bargraphSplit?"SPLIT":"CLASSIC")+"\",\"defaultProfile\":"+String(sysConfig.defaultProfile)+",\"customColors\":[";
    for(uint8_t i=0;i<SEC_COUNT;i++) {
        snprintf(color,sizeof(color),"#%02x%02x%02x",currentProfile.customColors[i].r,currentProfile.customColors[i].g,currentProfile.customColors[i].b);
        if(i)s+=",";
        s+="\""+String(color)+"\"";
    }
    s+="]";
    s+=",\"vm\":"+String(sysConfig.monitorVccEnabled?1:0)+",\"apTimeout\":"+String(apConfig.timeout)+",\"apProtected\":"+String(apConfig.password[0]?"true":"false");
    s+=",\"leftOpen\":"+String(digitalRead(LEFT_DOOR_PIN)==LOW?"true":"false")+",\"rightOpen\":"+String(digitalRead(RIGHT_DOOR_PIN)==LOW?"true":"false");
    s+=",\"vccAvailable\":"+String(!apActive&&sysConfig.monitorVccEnabled&&isfinite(measuredVoltage)?"true":"false");
    s+=",\"limits\":["+String(sysConfig.limitRed)+","+String(sysConfig.limitYellow)+","+String(sysConfig.limitGreen)+","+String(sysConfig.limitCharge)+"]";
    s+=",\"resistors\":["+String(sysConfig.voltageR1)+","+String(sysConfig.voltageR2)+"],\"hasCsl\":"+String(RGBDPL_HAS_CSL?"true":"false");
    s+=",\"startupStyle\":"+String(rgbdplStartupStyle)+",\"panelMask\":"+String(rgbdplPanelMask);
#if RGBDPL_HAS_CSL
    s+=",\"cslMode\":"+String(currentProfile.cslMode)+",\"cslSpeed\":"+String(currentProfile.cslSpeed)+",\"cslBrightness\":"+String(currentProfile.cslBrightness);
#if RGBDPL_HAS_CSL_DOOR
    s+=",\"cslDoor\":"+String(currentProfile.cslDoorPin);
#endif
#endif
    return s+"}";
}
static const char standalonePage[] PROGMEM=R"HTML(
<!doctype html><html lang="en"><meta name="viewport" content="width=device-width,initial-scale=1"><title>RGB-DPL</title>
<style>:root{color-scheme:dark;--accent:#49dcc3}*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at top,#183a43,#08131d 65%);color:#eef7f6;font:16px 'Trebuchet MS',sans-serif}header,main{max-width:1050px;margin:auto;padding:24px}header{border-bottom:1px solid #35505a}h1{margin:0;letter-spacing:.04em}h2{font-size:19px;color:var(--accent)}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(285px,1fr));gap:20px}section{background:#10242de8;border:1px solid #35505a;border-radius:18px;padding:22px}label{display:block;margin:14px 0 6px}input,select,button{font:inherit;max-width:100%;background:#071720;color:#fff;border:1px solid #52747b;border-radius:8px;padding:9px}input[type=number]{width:90px}input[type=color]{width:80px;height:44px;padding:3px}button{cursor:pointer;margin:4px 4px 4px 0}button:hover{border-color:var(--accent)}#message{position:sticky;bottom:0;background:#071720;padding:12px;border-top:1px solid #52747b}small{color:#b0c6cb}.row{display:flex;flex-wrap:wrap;gap:8px}#limits input{width:74px}</style>
<header><small>LOCAL CONTROL / STANDALONE</small><h1 id="name">RGB-DPL</h1><p id="version"></p></header><main>
<section><h2>LDPL</h2><label>Pattern</label><select id="ldplMode" onchange="send('LDPLMODE '+this.value)"></select><label>Base color</label><input id="ldplColor" type="color" onchange="const h=this.value;send('LDPLCOLOR '+[1,3,5].map(i=>parseInt(h.slice(i,i+2),16)).join(' '))"><p><small>KITT: clean bounce. Cylon: bounce with trail. Save a profile to retain color and pattern.</small></p></section>
<section><h2>Panels & Appearance</h2><div id="settings"></div><label>Sequence</label><select id="sequence" onchange="send('SEQUENCE '+(this.value==='1'?'ON':'OFF'))"><option value="0">Off</option><option value="1">On</option></select><label>Bargraph</label><select id="bargraph" onchange="send('BARGRAPH '+this.value)"><option>CLASSIC</option><option>SPLIT</option></select><p id="doors"></p></section>
<section><h2>Power & Startup</h2><label>Startup style (next reboot)</label><select id="startupStyle" onchange="send('STARTUP '+this.value)"><option value="0">Rainbow wave + white flashes</option><option value="1">Sequential Pro-X style</option></select><div class="row"><button onclick="send('ALLON')">All on</button><button onclick="send('ALLOFF')">All off</button></div><p><small>Panel power is saved and overrides door switches. On restores normal door gating.</small></p><div id="panelPower"></div></section>
<section><h2>Profiles & Custom Colors</h2><select id="profile"><option value="1">Profile 1</option><option value="2">Profile 2</option><option value="3">Profile 3</option><option value="4">Profile 4</option><option value="5">Profile 5</option></select><div><button onclick="send('SAVE '+profile.value)">Save</button><button onclick="send('LOAD '+profile.value)">Load</button><button onclick="send('DEFAULT '+profile.value)">Boot default</button><button onclick="send('DEFAULT 0')">No boot default</button></div><label>Custom section</label><select id="section" onchange="syncCustom()"></select><input id="custom" type="color" value="#ff0000"><button onclick="const h=custom.value;send('COLOR '+section.value+' '+[1,3,5].map(i=>parseInt(h.slice(i,i+2),16)).join(' '))">Apply custom color</button></section>
<section><h2>Voltage Monitor</h2><label>Monitoring (default off)</label><select id="vm" onchange="send('VM '+this.value)"><option value="0">Off</option><option value="1">On</option></select><p>ADC measurement is unavailable while Wi-Fi is active. Status LEDs remain off, never false voltage values.</p><div id="limits"></div><button onclick="send('VL '+[...document.querySelectorAll('#limits input')].map(e=>e.value).join(' '))">Save thresholds</button><label>Divider R1 / R2 (ohms)</label><input id="r1" type="number" min="1"><input id="r2" type="number" min="1"><button onclick="send('RR '+r1.value+' '+r2.value)">Save divider</button></section>
<section><h2>Access Point</h2><p id="protection"></p><label>Password (8-63 characters)</label><input id="password" type="password" maxlength="63" autocomplete="new-password"><button onclick="send('APPW '+password.value);password.value=''">Set password</button><button onclick="if(confirm('Open the AP without a password?'))send('APPW CLEAR')">Open AP</button><label>Idle timeout (seconds, 0 = never)</label><input id="apTimeout" type="number" min="0" max="86400"><button onclick="send('APTIMEOUT '+apTimeout.value)">Save timeout</button><button onclick="if(confirm('Disconnect this UI and turn Wi-Fi off?'))send('AP OFF')">Wi-Fi off</button><p><small>AP ON over serial or power-cycle opens the AP again.</small></p></section>
<section id="csl" hidden><h2>Coin Slot Logics</h2><div id="cslSettings"></div></section>
</main><div id="message" role="status">Loading...</div><script>
const $=id=>document.getElementById(id);let initialized=false;let customColors=[];function syncCustom(){if(document.activeElement!==$('custom')&&customColors[$('section').value])$('custom').value=customColors[$('section').value]}
const names=['Off','Dot bounce','Fade trail','Rainbow dot','KITT / KnightRider','Cylon','Beat bars','Breathe','Juggle','Sparkle','Comet','Alarm','Fire','Logic sweep','Rainbow bars','Theater chase'];
names.forEach((n,i)=>$('ldplMode').add(new Option(i+' - '+n,i)));
function field(parent,id,title,cmd,options){const label=document.createElement('label');label.textContent=title;const select=document.createElement('select');select.id=id;options.forEach((n,i)=>select.add(new Option(n,i)));select.onchange=()=>send(cmd+' '+select.value);$(parent).append(label,select)}
field('settings','cbiMode','CBI','CBIMODE',['Organic','ESB','Rainbow','Sparkle','Heart','Smiley','Matrix rain']);field('settings','topBlocks','Top blocks','TOPBLOCKS',['Classic','Horizontal','Paired','Chaos','Single']);field('settings','scheme','Color scheme','SCHEME',['Classic','Blue','Pink','Green','Cyberpunk','Forest','Sunset','Custom']);field('settings','personality','Personality','PERSONALITY',['Normal','Happy','Grumpy','Excited','Sleepy']);
for(const [id,title,cmd,max] of [['brightness','Brightness (%)','BRIGHTNESS',100],['speed','Animation speed','SPEED',10],['cbiSpeed','CBI speed','CBISPEED',10]]){const l=document.createElement('label');l.textContent=title;const e=document.createElement('input');e.id=id;e.type='number';e.min=1;e.max=max;e.onchange=()=>send(cmd+' '+e.value);$('settings').append(l,e)}
['Red','Yellow','Green','Charge'].forEach(n=>{const e=document.createElement('input');e.type='number';e.step='.1';e.title=n;$('limits').append(e)});
async function send(cmd){$('message').textContent='Applying...';try{const r=await fetch('/command',{method:'POST',body:new URLSearchParams({cmd})});if(!r.ok)throw Error(await r.text());$('message').textContent=await r.text();await refresh()}catch(e){$('message').textContent=e.message+' (reconnect if Wi-Fi settings changed)'}}
async function refresh(){const r=await fetch('/status',{cache:'no-store'});if(!r.ok)throw Error('Status unavailable');const d=await r.json();if(!initialized){$('name').textContent=d.name;$('version').textContent='Firmware '+d.version;const sections=['Top yellow','Top green','Blue column','Bargraph low','Bargraph mid','Bargraph high','Bottom white','Large red','VU','CBI matrix','CBI status','LDPL'];if(d.hasCsl)sections.push('CSL');sections.forEach((n,i)=>$('section').add(new Option(n,i)));$('csl').hidden=!d.hasCsl;const panels=['bottom','red','bluecolumn','bargraph','topblocks','vu','ldpl','cbi'];if(d.hasCsl)panels.push('csl');panels.forEach((name,i)=>{const row=document.createElement('div');row.className='row';const label=document.createElement('span');label.textContent=name;label.style.minWidth='110px';const button=document.createElement('button');button.id='power_'+name;button.onclick=()=>send(((d.panelMask&(1<<i))?'OFF ':'ON ')+name);row.append(label,button);$('panelPower').append(row)});if(d.hasCsl){field('cslSettings','cslMode','Pattern','CSLMODE',['Off','Default','Static','Glow','Running 1','Running 2','Running 3','Running 4']);for(const [id,cmd,max] of [['cslSpeed','CSLSPEED',10],['cslBrightness','CSLBRIGHTNESS',100]]){const e=document.createElement('input');e.id=id;e.type='number';e.min=1;e.max=max;e.onchange=()=>send(cmd+' '+e.value);$('cslSettings').append(e)}if('cslDoor'in d)field('cslSettings','cslDoor','Switch assignment','CSLDOOR',['Always on','Left switch','Right switch'])}initialized=true}
const panels=['bottom','red','bluecolumn','bargraph','topblocks','vu','ldpl','cbi'];if(d.hasCsl)panels.push('csl');panels.forEach((name,i)=>{const button=$('power_'+name);button.textContent=d.panelMask&(1<<i)?'On':'Off';button.onclick=()=>send(((d.panelMask&(1<<i))?'OFF ':'ON ')+name)});for(const id of ['startupStyle','ldplMode','ldplColor','brightness','speed','cbiSpeed','cbiMode','topBlocks','scheme','personality','sequence','vm','apTimeout','cslMode','cslSpeed','cslBrightness','cslDoor','bargraph'])if($(id)&&document.activeElement!==$(id))$(id).value=d[id];customColors=d.customColors;syncCustom();$('profile').title='Boot default: '+(d.defaultProfile||'none');$('doors').textContent='Switch gating: left '+(d.leftOpen?'open / enabled':'closed / off')+', right '+(d.rightOpen?'open / enabled':'closed / off');$('protection').textContent=d.apProtected?'Password protected':'Open AP - set a password if needed';if(document.activeElement.tagName!=='INPUT'){[...document.querySelectorAll('#limits input')].forEach((e,i)=>e.value=d.limits[i]);$('r1').value=d.resistors[0];$('r2').value=d.resistors[1]}}
refresh().then(()=>$('message').textContent='Ready').catch(e=>$('message').textContent=e.message);setInterval(()=>refresh().catch(()=>{}),4000);
</script></html>
)HTML";

void beginStandaloneWeb() {
    LocalApConfig saved;EEPROM.get(apConfigAddress,saved);
    if(saved.magic==apConfig.magic&&saved.timeout<=86400&&strnlen(saved.password,sizeof(saved.password))<sizeof(saved.password)&&(!saved.password[0]||strlen(saved.password)>=8))apConfig=saved;
    localWeb.on("/",HTTP_GET,[]{localWeb.send_P(200,"text/html",standalonePage);});
    localWeb.on("/status",HTTP_GET,[]{String status=standaloneStatus();status.remove(status.length()-1);status+=",\"name\":\"" RGBDPL_AP_NAME "\"}";localWeb.sendHeader("Cache-Control","no-store");localWeb.send(200,"application/json",status);});
    localWeb.on("/command",HTTP_POST,[]{
        if(isWaitingForCommand){localWeb.send(409,"text/plain","Serial command in progress");return;}
        String command=localWeb.arg("cmd");
        if(!command.length()||command.length()>=SERIAL_BUFFER_SIZE||command.indexOf('\n')>=0||command.indexOf('\r')>=0){localWeb.send(400,"text/plain","Invalid command");return;}
        WebCommandOutput output;
        command.toCharArray(serialBuffer,sizeof(serialBuffer));commandOutput=&output;
        processSerialCommand();commandOutput=&Serial;memset(serialBuffer,0,sizeof(serialBuffer));
        localWeb.send(output.text.indexOf("ERROR:")>=0?400:200,"text/plain",output.text);
    });
    startLocalAp();
}
void serviceStandaloneWeb() {
    if(apStopPending){apStopPending=false;stopLocalAp();}
    if(apRestartPending){apRestartPending=false;stopLocalAp();startLocalAp();}
    if(!apActive)return;
    localWeb.handleClient();
    const uint32_t now=millis();
    if(WiFi.softAPgetStationNum())apIdleSince=now;
    else if(apConfig.timeout&&uint32_t(now-apIdleSince)>=apConfig.timeout*1000UL)stopLocalAp();
    yield();
}
