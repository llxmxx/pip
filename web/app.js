let rid = null;
let prog = document.getElementById("prog").value
let frame = 0;

const speedEl = document.getElementById("speed");
const speedLabel = document.getElementById("speedlabel");
let budget = 0, last = performance.now();

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
    let box = document.getElementById("out");
    const opt = document.querySelector('#prog option[value="' + file + '"]');
    speedEl.value = opt && opt.dataset.speed ? opt.dataset.speed : 40;
    speedEl.oninput();
    box.textContent = "";
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
            ctx.fillRect(x*6, y*6, 6, 6);
        }
    }
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
    drawGrid();
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