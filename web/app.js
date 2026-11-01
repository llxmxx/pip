let rid = null;
let prog = document.getElementById("prog").value
let frame = 0;
let progLen = 1;
let lineOf = {};
let curLine = -1;

const speedEl = document.getElementById("speed");
const speedLabel = document.getElementById("speedlabel");
let budget = 0, last = performance.now();

const ALU_OPS = {1: "ADD", 2: "SUB", 3: "MUL", 4: "DIV", 6: "CMP", 21: "AND", 22: "OR", 23: "XOR", 24: "NOT", 25: "SHL", 26: "SHR"};

const WIRES = {"rf-ctrl": ["rf", "ctrl"], "rf-alu": ["rf", "alu"], "ctrl-mem": ["ctrl", "mem"], "ctrl-stk": ["ctrl", "stk"], "stk-mem": ["stk", "mem"], "alu-stk": ["alu", "stk"], "alu-io": ["alu", "io"]};
const PATHS = {};
[1, 2, 3, 4, 6, 21, 22, 23, 24, 25, 26].forEach(o => PATHS[o] = ["rf-alu"]);
PATHS[5] = ["rf-ctrl"];
PATHS[18] = ["rf-ctrl"];
PATHS[16] = PATHS[17] = ["rf-ctrl", "ctrl-mem"];
for(let o = 7; o <= 15; o++) PATHS[o] = ["ctrl-stk", "stk-mem"];
PATHS[19] = PATHS[20] = ["ctrl-stk", "stk-mem"];
PATHS[27] = ["alu-io"];

function ips(){
    return Math.pow(1.2e6, Number(speedEl.value)/100);
}
function fmt(x){
    if(x >= 1e6) return (x/1e6).toFixed(1)+"M";
    if(x >= 1e3) return (x/1e3).toFixed(1)+"k";
    return x>=10 ? String(Math.round(x)) : x.toFixed(1);
}
speedEl.oninput = () =>{speedLabel.textContent = fmt(ips()) + " instr/s";}
speedEl.oninput();

async function load(file){
    Module._vm_reset();
    let bytes = new Uint8Array(await (await fetch(file)).arrayBuffer());
    bytes.forEach((b, i) => Module._vm_load_byte(i, b));
    progLen = bytes.length;
    try {
        const srcRes = await ( await fetch(file.replace(/\.bin$/, ".asm")));
        const mapRes = await( await fetch(file + ".map"));
        if(!srcRes.ok || !mapRes.ok) throw new Error("missing source or map");
        const srcText = await srcRes.text();
        const mapText = await mapRes.text();
        lineOf = {};
        mapText.split("\n").forEach(l => {
            const p = l.trim().split(" ");
            if(p.length === 2) lineOf[p[0]] = Number(p[1]);
        });
        buildSrc(srcText);
    }
    catch(e){
        lineOf = {};
        document.getElementById("srcbox").textContent = "no source for this program";
    }
    let box = document.getElementById("out");
    const opt = document.querySelector('#prog option[value="' + file + '"]');
    speedEl.value = opt && opt.dataset.speed ? opt.dataset.speed : 40;
    speedEl.oninput();
    box.textContent = opt && opt.dataset.hint ? opt.dataset.hint + "\n" : "";
}

function drawGrid(){
    let canvas = document.getElementById("grid");
    let ctx = canvas.getContext("2d");
    for(let y = 0; y < 64; y++){
        for(let x = 0; x < 64; x++){
            let addr = 0x6000 + (y*64+x)*2;
            let v = (Module._vm_mem(addr) << 8) | Module._vm_mem(addr + 1);
            let r = Math.round(((v >> 11) & 31) * 255/31);
            let g = Math.round(((v >> 5) & 63) * 255/63);
            let b = Math.round((v & 31) * 255/31);
            ctx.fillStyle = "rgb(" + r + "," + g +"," + b +")";
            ctx.fillRect(x*6, y*6, 5.5, 5.5);
        }
    }
}

function showBar(){
    let pc = Module._vm_pc();
    let sp = Module._vm_reg(7);
    if(sp === 0) sp = 65535;
    let w = Math.max(progLen / 65536 * 100, 0.6);
    document.getElementById("used").style.width = w+"%";
    document.getElementById("conn").style.clipPath = "polygon(0 0," + w + "% 0, 100% 100%, 0 100%)";
    document.getElementById("pcmark").style.left = (pc/65536 *100) + "%";
    document.getElementById("spmark").style.left = (sp/65536 * 100) + "%";
    document.getElementById("pcz").style.left = Math.min(100, pc/progLen * 100) + "%";
    document.getElementById("codezoom").dataset.len = progLen + " BYTES";
}

function showAlu(){
    let box = document.getElementById("aluinfo");
    let pc = Module._vm_pc();
    let w = (Module._vm_mem(pc) << 8) | Module._vm_mem(pc + 1);
    let op = (w >> 11) & 31, d = (w >> 8) & 7, s1 = (w >> 5) & 7, s2 = (w >> 2) & 7;
    let name = ALU_OPS[op];
    if(!name){
        box.textContent = "idle";
        box.style.color = "var(--dim)";
        return;
    }
    box.style.color = "var(--signal)";
    let a = Module._vm_reg(s1), b = Module._vm_reg(s2);
    let line2, line3;
    if(name === "NOT"){
        line2 = "R" + d + "<- ~R" + s1;
        line3 = "~" + a + " = " + ((~a) & 0xFFFF);
    }
    else if(name === "CMP"){
        line2 = "R" + s1 + " vs R" + s2;
        line3 = a + " vs " + b + " -> " + (a < b ? "less" : a===b ? "equal" : "greater");
    }
    else{
        const sym = {ADD: "+", SUB: "-", MUL: "*", DIV: "/", AND: "&", OR: "|", XOR: "^", SHL: "<<", SHR: ">>"}[name];
        let r;
        if(name === "ADD") r = (a+b) & 0xFFFF;
        else if(name === "SUB") r = (a-b) & 0xFFFF;
        else if(name === "MUL") r = (a*b) & 0xFFFF;
        else if(name === "DIV") r = b===0 ? "div by 0" : Math.floor(a/b);
        else if(name === "AND") r = (a&b);
        else if(name === "OR") r = (a|b);
        else if(name === "XOR") r = (a^b);
        else if(name === "SHL") r = (a<<b) & 0xFFFF;
        else r = a >> b;
        line2 = "R" + d + " <- R" + s1 + " " + sym + " R" + s2;
        line3 = a + " " + sym + " " + b + " = " + r;
    }
    box.textContent = name + "\n" + line2 + "\n" + line3;
}

function showStack(){
    let st = document.getElementById("stack");
    let sp = Module._vm_reg(7);
    if(sp === 0){
        st.textContent = "stack: empty";
    }
    else{
        let text = "stack: \n";
        for (let a = sp; a <= 65534; a+=2){
            let ret = (Module._vm_mem(a) << 8) | (Module._vm_mem(a+1));
            text += "return to " + ret + "\n";
        }
        st.textContent = text;
    }
}

function showMem(){
    let pc = Module._vm_pc();
    let start = Math.max(0, pc-8);
    let text = "";
    for(let row = 0; row < 3; row++){
        let base = start + row*8;
        text += base.toString(16).padStart(4, "0").toUpperCase() + ": ";
        for(let i = 0; i < 8; i++){
            let a = base + i;
            let hex = Module._vm_mem(a).toString(16).padStart(2, "0").toUpperCase();
            if(a === pc || a === pc+1) text+= "[" + hex + "]";
            else text += " " +hex + " ";
        }
        text+="\n";
    }
    document.getElementById("hex").textContent = text;
}

function buildSrc(text){
    const box = document.getElementById("srcbox");
    box.textContent = "";
    text.split("\n").forEach((t, i) => {
        const d= document.createElement("div");
        d.className = "ln";
        d.textContent = String(i+1).padStart(3, " ") + " " + t.replace("\r", "");
        box.appendChild(d);
    });
    curLine = -1;
}

function showSrc(){
    const ln = lineOf[Module._vm_pc()];
    if(!ln || ln === curLine) return;
    const box = document.getElementById("srcbox");
    if (curLine > 0) box.children[curLine - 1].classList.remove("cur");
    const el = box.children[ln-1];
    el.classList.add("cur");
    box.scrollTop = el.offsetTop - box.clientHeight/2;
    curLine = ln;
}

function placeWires(){
    const die = document.getElementById("die").getBoundingClientRect();
    for(const name in WIRES){
        const a = document.getElementById(WIRES[name][0]).getBoundingClientRect();
        const b = document.getElementById(WIRES[name][1]).getBoundingClientRect();
        const l = document.getElementById("w-" + name);
        l.setAttribute("x1", a.left + a.width/2 - die.left);
        l.setAttribute("y1", a.top + a.height/2 - die.top);
        l.setAttribute("x2", b.left + b.width/2 - die.left);
        l.setAttribute("y2", b.top + b.height/2 - die.top);
    }
}

function buildWires(){
    const svg = document.getElementById("wires");
    for(const name in WIRES){
        const l = document.createElementNS("http://www.w3.org/2000/svg", "line");
        l.setAttribute("class", "wire");
        l.id = "w-" + name;
        svg.appendChild(l);
    }
    placeWires();
}

function showWires(){
    buildWires();
    const pc = Module._vm_pc();
    const op = ((Module._vm_mem(pc) << 8) | Module._vm_mem(pc+1)) >> 11;
    document.querySelectorAll(".wire.hot").forEach(w => w.classList.remove("hot"));
    (PATHS[op] || []).forEach(n => {
        const el = document.getElementById("w-" + n);
        if(el) el.classList.add("hot");
    });
}

function show(){
    let text = "";
    for(let i = 0; i < 8; i ++){
        text += "R"+i+": "+Module._vm_reg(i) + "\n";
    }
    document.getElementById("regs").textContent = text;
    text = "";
    text += "pc: "+Module._vm_pc() + "\n";
    let flags = Module._vm_flags();
    text += "overflow: " + ((flags & 1) ? "on" : "off") + "\n";
    text += "zero: " + ((flags & 2) ? "on" : "off") + "\n";
    text += "less: " + ((flags & 4) ? "on" : "off") + "\n";
    text += "next: " + decode();
    document.getElementById("ctrlinfo").textContent = text;
    showMem();
    showStack();
    showAlu();
    showBar();
    showSrc();
    drawGrid();
    showWires();
}

var Module = {
    print(text){
        let box = document.getElementById("out");
        box.textContent += text + "\n";
    },
    async onRuntimeInitialized(){
        await load(prog);
        show();
    }
}

document.getElementById("step").onclick = () => {
    Module._vm_step();
    show();
}

document.getElementById("run").onclick = () => {
    if(rid === null){
        budget = 0;
        last = performance.now();
        rid = setInterval(() =>{
            const now = performance.now();
            budget += ips()*Math.min(now-last, 100)/1000;
            last = now;
            const n = Math.floor(budget);
            budget -= n;
            frame++;
            Module._vm_poke(0xBFFD, frame & 255);
            if(n > 0) Module._vm_run(n);
            show();
            if(Module._vm_halted()){
                clearInterval(rid);
                rid = null;
            }
        }, 16);
    }
    show();
}

document.getElementById("pause").onclick = () => {
    if(rid !== null){
        clearInterval(rid);
        rid = null;
    }
}

document.getElementById("reset").onclick =  async () => {
    if(rid !== null){
        clearInterval(rid);
        rid = null;
    }
    await load(prog);
    show();
}

document.getElementById("prog").onchange = async () => {
    if(rid !== null){
        clearInterval(rid);
        rid = null;
    }
    prog = document.getElementById("prog").value
    await load(prog);
    show();
}

document.addEventListener("keydown", e => {
    if(e.code === "Space"){
        e.preventDefault();
        Module._vm_poke(0xBFFF, 1);
    }
})

document.getElementById("grid").addEventListener("pointerdown", () => {
    Module._vm_poke(0xBFFF, 1);
})

new ResizeObserver(placeWires).observe(document.getElementById("die"));