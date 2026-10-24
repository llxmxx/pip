let rid = null;
let prog = document.getElementById("prog").value
async function load(file){
    Module._vm_reset();
    let bytes = new Uint8Array(await (await fetch(file)).arrayBuffer());
    bytes.forEach((b, i) => Module._vm_load_byte(i, b));
    let box = document.getElementById("out");
    box.textContent = "";
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
    text += "next: " + decode() + "\n";
    document.getElementById("regs").textContent = text;
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
            Module._vm_step();
            show();
            if(Module._vm_halted()){
                clearInterval(rid);
                rid = null;
            }
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