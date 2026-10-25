let rid = null;
let prog = document.getElementById("prog").value

async function load(file){
    Module._vm_reset();
    let bytes = new Uint8Array(await (await fetch(file)).arrayBuffer());
    bytes.forEach((b, i) => Module._vm_load_byte(i, b));
    let box = document.getElementById("out");
    box.textContent = "";
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

function show(){
    let text = "";
    for(let i = 0; i < 8; i ++){
        text += "R"+i+": "+Module._vm_reg(i) + "\n";
    }
    text += "pc: "+Module._vm_pc() + "\n";
    let flags = Module._vm_flags();
    text += "overflow: " + ((flags & 1) ? "on" : "off") + "\n";
    text += "zero: " + ((flags & 2) ? "on" : "off") + "\n";
    text += "less: " + ((flags & 4) ? "on" : "off") + "\n";
    text += "next: " + decode();
    document.getElementById("regs").textContent = text;
    showStack();
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
        rid = setInterval(() =>{
            let n = Number(document.getElementById("speed").value);
            for (let i = 0; i < n; i++){
                Module._vm_step();
                if(Module._vm_halted()){
                    clearInterval(rid);
                    rid = null;
                    break;
                }
            }
            show();
        }, 50);
    }
    show()
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