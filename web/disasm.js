const NAMES = [
    "HALT",
    "ADD","SUB","MUL","DIV",
    "LOADI",
    "CMP",
    "JMP","JZ","JNZ","JEQ","JNE","JLT","JGT","JLE","JGE",
    "LOAD","STORE","MOV",
    "CALL","RET",
    "AND","OR","XOR","NOT","SHL","SHR",
    "PRINT"
];

const SHAPES = [
    "NONE",
    "DSS","DSS","DSS","DSS",
    "DI","SS",
    "D",
    "DS",
    "DS","D","D","D","D","D","D",
    "DS","DS","DS",
    "D","NONE",
    "DSS","DSS","DSS","DS","DSS","DSS",
    "S"
];

function decode(){
    let pc = Module._vm_pc();
    let inst = (Module._vm_mem(pc) << 8) | (Module._vm_mem(pc + 1));
    let num = (Module._vm_mem(pc+2) << 8) | (Module._vm_mem(pc + 3));
    let op = inst >> 11;
    let d = (inst >> 8) & 7;
    let s1 = (inst >> 5) & 7;
    let s2 = (inst >> 2) & 7;

    let text = NAMES[op];
    let shape = SHAPES[op];

    if(shape === "DSS") text += " R" + d + ", R" + s1 + ", R" + s2;
    else if(shape === "DS") text += " R" + d + ", R" + s1;
    else if(shape === "SS") text += " R" + s1 + ", R" + s2;
    else if(shape === "D") text += " R" + d;
    else if(shape === "S") text += " R" + s1;
    else if(shape === "DI") text += " R" + d + ", " + num;

    return text;
}