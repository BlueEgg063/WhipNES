#include "cpu.h"
CPU cpu;
void znflags(uint8_t input){
    if (!input){
    cpu.status=cpu.status | 0b00000010;
}
else{
    cpu.status=cpu.status & 0b11111101;
}
if (input & 0b10000000){
    cpu.status |= 0b10000000;
}
else{
    cpu.status &= ~0b10000000;
}
}
uint8_t read(uint16_t memaddr) {
    // RAM: $0000–$1FFF (mirrored every 0x800)
    if (memaddr < 0x2000) {
        return cpu.memory[memaddr % 0x800];
    }

    // PPU registers: $2000–$2007
    else if (memaddr < 0x2008) {
        return 0;
    }

    // PPU mirrors: $2008–$3FFF (mirrored every 8 bytes)
    else if (memaddr < 0x4000) {
        // mirror to $2000–$2007
        //uint16_t mirrored_addr = 0x2000 + (memaddr % 8);
        //return ppu_read(mirrored_addr);
        return 0;
    }

    // APU, I/O Registers: $4000–$401F (stubbed)
    else if (memaddr < 0x4020) {
        // return apu_io_read(memaddr);
        return 0;
    }

    // Cartridge space: $4020–$FFFF
    else {
        // This would call mapper-specific PRG read (ROM, RAM, etc.)
        // return cart_read(memaddr);
        return cpu.memory[memaddr];
    }
}

void write(uint16_t memaddr, uint8_t value) {
    // RAM: $0000–$1FFF (mirrored every 0x800)
    if (memaddr < 0x2000) {
        cpu.memory[memaddr % 0x800] = value;
    }
    // PPU registers: $2000–$2007
    else if (memaddr < 0x2008) {
        // ppu_write(memaddr, value);
        // placeholder for now
    }
    // PPU registers mirrored: $2008–$3FFF (mirror every 8 bytes)
    else if (memaddr < 0x4000) {
        uint16_t mirrored_addr = 0x2000 + (memaddr % 8);
        // ppu_write(mirrored_addr, value);
    }
    // APU and I/O registers: $4000–$401F
    else if (memaddr < 0x4020) {
        // apu_io_write(memaddr, value);
    }
    // Cartridge space: $4020–$FFFF
    else {
        // cart_write(memaddr, value);
    }
}

void push_stack(uint8_t input){
    write(0x0100+cpu.sp,input);
    cpu.sp--;
}
uint8_t pull_stack(){
    cpu.sp++;
    return read(0x0100+cpu.sp);
}
void NOP(){}
//access opcodes
void LDA(){
cpu.a=cpu.fetched;
znflags(cpu.a);
}
void LDX(){
cpu.x=cpu.fetched;
znflags(cpu.x);
}
void LDY(){
cpu.y=cpu.fetched;
znflags(cpu.y);
}
void STA(){
    write(cpu.addr_abs,cpu.a);
}
void STX(){
    write(cpu.addr_abs,cpu.x);
}
void STY(){
    write(cpu.addr_abs,cpu.y);
}
//transfer
void TAX(){
    cpu.x=cpu.a;
    znflags(cpu.x);
}
void TAY(){
    cpu.y=cpu.a;
    znflags(cpu.y);
    
}
void TXA(){
    cpu.a=cpu.x;
    znflags(cpu.a);
}
void TYA(){
    cpu.a=cpu.y;
    znflags(cpu.a);
}
//arithmetic
void ADC(){
    uint8_t carry=cpu.status & 0b00000001;
    uint16_t result = carry + (uint16_t)cpu.fetched+(uint16_t)cpu.a;
    uint8_t old_a= cpu.a;
    uint8_t res8=(uint8_t)(result & 0xFF);
    if (result > 0xFF){
        cpu.status |= 0b00000001;
    }
    else{
        cpu.status &= ~0b00000001;
    }
    if (~(old_a ^ cpu.fetched) & (old_a ^ res8) & 0x80){
        cpu.status |= 0b01000000;
    }
    else{
        cpu.status &= ~0b01000000;
    }
    cpu.a=res8;
    znflags(cpu.a);
}
void SBC(){
    uint8_t value = cpu.fetched;
    uint8_t carry = cpu.status & 0b00000001;
    uint16_t result = (uint16_t)cpu.a + (uint16_t)(~value) + (uint16_t)carry;
    uint8_t old_a = cpu.a;
    uint8_t res8 = (uint8_t)(result & 0xFF);

    // Correct carry flag: set if result >= 0x100, clear if borrow happened
    if (result & 0x100){
        cpu.status |= 0b00000001;  // carry set
    }
    else{
        cpu.status &= ~0b00000001; // carry clear
    }

    // Overflow flag check (same as ADC but with inverted operand)
    if (((old_a ^ res8) & (old_a ^ ~value)) & 0x80) {
    cpu.status |= 0b01000000;  // overflow set
} else {
    cpu.status &= ~0b01000000; // overflow clear
}


    cpu.a = res8;
    znflags(cpu.a);
}
void INC(){
    uint8_t original = read(cpu.addr_abs);
    original+=1;
    write(cpu.addr_abs, original);
    znflags(original);
}
void DEC(){
    uint8_t original = read(cpu.addr_abs);
    original-=1;
    write(cpu.addr_abs, original);
    znflags(original);
}
void INX(){
    cpu.x+=1;
    znflags(cpu.x);
}
void INY(){
    cpu.y+=1;
    znflags(cpu.y);
}
void DEX(){
    cpu.x-=1;
    znflags(cpu.x);
}
void DEY(){
    cpu.y-=1;
    znflags(cpu.y);
}
//shift
void ASL(){
    uint8_t out=cpu.fetched << 1;
    if (cpu.fetched & 0b10000000){
        cpu.status |=0b00000001;
    }
    else{
        cpu.status &= ~0b00000001;
    }
    if (cpu.ACU){
        cpu.a = out;
    }
    else{
        write(cpu.addr_abs, out);
    }
    cpu.ACU=false;
    znflags(out);
}
void LSR(){
uint8_t out=cpu.fetched >> 1;
if (cpu.fetched & 0b00000001){
    cpu.status |= 0b00000001;
}
else{
    cpu.status &= ~0b00000001;
}
if (cpu.ACU){
    cpu.a=out;
}
else{
    write(cpu.addr_abs, out);
}
if (!out){
    cpu.status |= 0b00000010;
}
else {
    cpu.status &= ~0b00000010;
}
cpu.status &= ~0b10000000;
cpu.ACU=false;
}
void ROL() {
    uint8_t carry_in = (cpu.status & 0x01);
    uint8_t carry_out = (cpu.fetched & 0x80) >> 7;
    uint8_t out = (cpu.fetched << 1) | carry_in;

    // Set or clear carry flag
    if (carry_out)
        cpu.status |= 0x01;
    else
        cpu.status &= ~0x01;

    // Store result
    if (cpu.ACU) {
        cpu.a = out;
        cpu.ACU = false;
    } else {
        write(cpu.addr_abs, out);
    }

    znflags(out);
}
void ROR() {
    // Extract carry flag (bit 0) and shift it to bit 7 for rotation
    uint8_t carry_in = (cpu.status & 0x01) << 7;

    // Extract bit 0 from fetched value to set carry later
    uint8_t carry_out = cpu.fetched & 0x01;

    // Perform rotate right: shift right, then OR in the old carry
    uint8_t out = (cpu.fetched >> 1) | carry_in;

    // Set or clear carry flag based on bit that was shifted out
    if (carry_out)
        cpu.status |= 0x01;        // Set carry
    else
        cpu.status &= ~0x01;       // Clear carry

    // Write result back to accumulator or memory
    if (cpu.ACU) {
        cpu.a = out;
        cpu.ACU = false;
    } else {
        write(cpu.addr_abs, out);
    }

    // Set or clear Zero and Negative flags
    znflags(out);
}
//bitwise
void AND(){
    cpu.a &= cpu.fetched;
    znflags(cpu.a);
}
void ORA(){
    cpu.a |= cpu.fetched;
    znflags(cpu.a);
}
void EOR(){
    cpu.a ^= cpu.fetched;
    znflags(cpu.a);
}
void BIT(){
    if (!(cpu.a & cpu.fetched)){
        cpu.status |= 0b00000010;
    }
    else{
        cpu.status &= ~0b00000010;
    }
    if (cpu.fetched & 0b01000000){
        cpu.status |= 0b01000000;
    }
    else{
        cpu.status &= ~0b01000000;
    }
    if (cpu.fetched & 0x80){
        cpu.status |= 0b10000000;
    }
    else{
        cpu.status &= ~0b10000000;
    }
}
//compare
void CMP() {
    uint8_t value = cpu.fetched;
    uint16_t temp = (uint16_t)cpu.a - (uint16_t)value;

    // Set carry if A >= M (no borrow)
    if (cpu.a >= value)
        cpu.status |= 0b00000001;  // Carry flag set
    else
        cpu.status &= ~0b00000001; // Carry flag clear

    // Set zero flag if equal
    if ((temp & 0xFF) == 0)
        cpu.status |= 0b00000010;  // Zero flag set
    else
        cpu.status &= ~0b00000010; // Zero flag clear

    // Set negative flag based on bit 7 of result
    if (temp & 0x80)
        cpu.status |= 0b10000000;  // Negative flag set
    else
        cpu.status &= ~0b10000000; // Negative flag clear
}
void CPX() {
    uint8_t value = cpu.fetched;
    uint16_t temp = (uint16_t)cpu.x - (uint16_t)value;

    // Set carry if A >= M (no borrow)
    if (cpu.x >= value)
        cpu.status |= 0b00000001;  // Carry flag set
    else
        cpu.status &= ~0b00000001; // Carry flag clear

    // Set zero flag if equal
    if ((temp & 0xFF) == 0)
        cpu.status |= 0b00000010;  // Zero flag set
    else
        cpu.status &= ~0b00000010; // Zero flag clear

    // Set negative flag based on bit 7 of result
    if (temp & 0x80)
        cpu.status |= 0b10000000;  // Negative flag set
    else
        cpu.status &= ~0b10000000; // Negative flag clear
}
void CPY() {
    uint8_t value = cpu.fetched;
    uint16_t temp = (uint16_t)cpu.y - (uint16_t)value;

    // Set carry if A >= M (no borrow)
    if (cpu.y >= value)
        cpu.status |= 0b00000001;  // Carry flag set
    else
        cpu.status &= ~0b00000001; // Carry flag clear

    // Set zero flag if equal
    if ((temp & 0xFF) == 0)
        cpu.status |= 0b00000010;  // Zero flag set
    else
        cpu.status &= ~0b00000010; // Zero flag clear

    // Set negative flag based on bit 7 of result
    if (temp & 0x80)
        cpu.status |= 0b10000000;  // Negative flag set
    else
        cpu.status &= ~0b10000000; // Negative flag clear
}
//branch
void BCC(){
    uint16_t oldpc= cpu.pc;
    if (!(cpu.status & 0b00000001)){
        cpu.extracycles++;
        int8_t offset = (int8_t)cpu.addr_rel;
        cpu.pc+=offset;
        if ((oldpc & 0xFF00)!=(cpu.pc & 0xFF00)){
        cpu.extracycles++;
    }
    }
}
void BCS(){
    uint16_t oldpc= cpu.pc;
    if ((cpu.status & 0b00000001)){
        cpu.extracycles++;
        int8_t offset = (int8_t)cpu.addr_rel;
        cpu.pc+=offset;
        if ((oldpc & 0xFF00)!=(cpu.pc & 0xFF00)){
        cpu.extracycles++;
    }
    }
}
void BEQ(){
    uint16_t oldpc= cpu.pc;
    if ((cpu.status & 0b00000010)){
        cpu.extracycles++;
        int8_t offset = (int8_t)cpu.addr_rel;
        cpu.pc+=offset;
        if ((oldpc & 0xFF00)!=(cpu.pc & 0xFF00)){
        cpu.extracycles++;
    }
    }
}
void BNE(){
    uint16_t oldpc= cpu.pc;
    if (!(cpu.status & 0b00000010)){
        cpu.extracycles++;
        int8_t offset = (int8_t)cpu.addr_rel;
        cpu.pc+=offset;
        if ((oldpc & 0xFF00)!=(cpu.pc & 0xFF00)){
        cpu.extracycles++;
    }
    }
}
//jump
void JMP(){
    cpu.pc=cpu.fetched;
}
void JSR() {
    uint16_t return_addr = cpu.pc - 1;
    push_stack((return_addr >> 8) & 0xFF); // Push high byte
    push_stack(return_addr & 0xFF);        // Push low byte
    cpu.pc = cpu.addr_abs; // use the address already computed
}

void RTS(){
    uint8_t lo=pull_stack();
    uint8_t hi=pull_stack();
    uint16_t target = ((uint16_t)hi << 8) | lo;
    target+=1;
    cpu.pc=target; 
}
void BRK() {
    cpu.pc++; // BRK is a 1-byte instruction, but it pushes PC + 2
    push_stack((cpu.pc >> 8) & 0xFF); // high byte
    push_stack(cpu.pc & 0xFF);        // low byte
    push_stack(cpu.status | 0x30);    // B and unused bits set
    cpu.status |= 0x04;               // Set Interrupt Disable
    cpu.pc = ((uint16_t)read(0xFFFF) << 8) | read(0xFFFE);
}

void RTI(){
    cpu.status=pull_stack();
    uint8_t lo = pull_stack();
    uint8_t hi = pull_stack();
    uint16_t target = ((uint16_t)hi << 8) | lo;
    cpu.pc=target;
}
//stack
void PHA(){
    push_stack(cpu.a);
}
void PLA(){
    cpu.a=pull_stack();
    znflags(cpu.a);
}
void PHP(){
    push_stack(cpu.status | 0b00110000);
}
void PLP(){
    cpu.status= pull_stack();
}
void TSX(){
    cpu.x=cpu.sp;
}
void TXS(){
    cpu.sp=cpu.x;
}
//flags
void CLC(){
    cpu.status &= ~0b00000001;
}
void SEC(){
    cpu.status |= 0b00000001;
}
void CLI(){
    cpu.status &= ~0b00000100;
}
void SEI(){
    cpu.status |= 0b00000100;
}
void CLD(){
    cpu.status &= ~0b00001000;
}
void SED(){
    cpu.status |= 0b00001000;
}
void CLV(){
    cpu.status &= ~0b01000000;
}
//addrmodes
void IMM(){
    cpu.addr_abs=cpu.pc+1;
    cpu.fetched = read(cpu.addr_abs); 
    cpu.pc +=2;
}
void ZP0(){
    uint8_t addr= read(cpu.pc+1);
    cpu.addr_abs=addr;
    cpu.fetched = read(addr);
    cpu.pc+=2;
}
void ZPX(){
    uint8_t addr= (read(cpu.pc+1)+cpu.x)&0xFF;
    cpu.addr_abs=addr;
    cpu.fetched = read(addr);
    cpu.pc+=2;
}
void ZPY(){
    uint8_t addr= (read(cpu.pc+1)+cpu.y)&0xFF;
    cpu.addr_abs=addr;
    cpu.fetched = read(addr);
    cpu.pc+=2;
}
void ABS(){
    uint8_t lo= read(cpu.pc+1);
    uint8_t hi= read(cpu.pc+2);
    cpu.addr_abs=(hi<<8)|lo;
    cpu.fetched = read(cpu.addr_abs);
    cpu.pc += 3;
}
void ABSX(){
    uint8_t lo= read(cpu.pc+1);
    uint8_t hi= read(cpu.pc+2);
    uint16_t base_addr = (hi <<8) | lo;
    uint16_t new_addr = base_addr + cpu.x;
    cpu.addr_abs=new_addr;
    cpu.fetched = read(new_addr);
    cpu.extracycles= ((base_addr & 0xFF00)!=(new_addr & 0xFF00)) ? 1 : 0;
    cpu.pc += 3;
}
void ABSY(){
   uint8_t lo= read(cpu.pc+1);
    uint8_t hi= read(cpu.pc+2);
    uint16_t base_addr = (hi <<8) | lo;
    uint16_t new_addr = base_addr + cpu.y;
    cpu.addr_abs=new_addr;
    cpu.fetched = read(new_addr);
    cpu.extracycles= ((base_addr & 0xFF00)!=(new_addr & 0xFF00)) ? 1 : 0;
    cpu.pc += 3;
}
void IND() {
    uint8_t lo = read(cpu.pc + 1);
    uint8_t hi = read(cpu.pc + 2);
    uint16_t ptr = (hi << 8) | lo;

    // Simulate 6502 bug: if low byte is 0xFF, high byte wraps around same page
    uint8_t lo_addr = read(ptr);
    uint8_t hi_addr = read((ptr & 0xFF00) | ((ptr + 1) & 0x00FF));

    uint16_t jump_addr = (hi_addr << 8) | lo_addr;
    cpu.addr_abs = jump_addr;
    cpu.pc+=3;
}
void INDX() {
    uint8_t zp_addr = (read(cpu.pc + 1) + cpu.x) & 0xFF; // wrap around zero page
    uint8_t lo = read(zp_addr);
    uint8_t hi = read((zp_addr + 1) & 0xFF); // zero-page wrap
    uint16_t addr = (hi << 8) | lo;
    cpu.addr_abs=addr;
    cpu.fetched = read(addr);
    cpu.pc += 2;
}
void INDY() {
    uint8_t zp_addr = read(cpu.pc + 1);
    uint8_t lo = read(zp_addr);
    uint8_t hi = read((zp_addr + 1) & 0xFF);
    uint16_t base_addr = (hi << 8) | lo;
    uint16_t addr = base_addr + cpu.y;
    cpu.addr_abs=addr;
    cpu.fetched = read(addr);
    cpu.extracycles = ((base_addr & 0xFF00) != (addr & 0xFF00)) ? 1 : 0;

    cpu.pc += 2;
}
void IMP(){
    cpu.fetched = cpu.a;
    cpu.pc += 1;  // Advance PC past the opcode
}

void ACU(){
    cpu.fetched=cpu.a;
    cpu.ACU=true;
}
void REL(){
    cpu.addr_rel =(int8_t)read(cpu.pc+1);
    cpu.pc+=2;
}
Instruction lookup[256];
void init(){
    cpu.ACU = false;
    for (int i=0; i<256;i++){
        lookup[i]=(Instruction){"NOP", NOP, IMP, 2};
    }
    lookup[0xA9]=(Instruction){"LDA", LDA, IMM, 2};
    lookup[0xAD]=(Instruction){"LDA", LDA, ABS, 4};
    lookup[0xA5]=(Instruction){"LDA", LDA, ZP0, 3};
    lookup[0xB5]=(Instruction){"LDA", LDA, ZPX, 4};
    lookup[0xBD]=(Instruction){"LDA", LDA, ABSX, 4};
    lookup[0xB9]=(Instruction){"LDA", LDA, ABSY, 4};
    lookup[0xA1]=(Instruction){"LDA", LDA, INDX, 4};
    lookup[0xB1]=(Instruction){"LDA", LDA, INDY, 4};
    lookup[0x85] = (Instruction){"STA", STA, ZP0, 3};
    lookup[0x95] = (Instruction){"STA", STA, ZPX, 4};
    lookup[0x8D] = (Instruction){"STA", STA, ABS, 4};
    lookup[0x9D] = (Instruction){"STA", STA, ABSX, 5};
    lookup[0x99] = (Instruction){"STA", STA, ABSY, 5};
    lookup[0x81] = (Instruction){"STA", STA, INDX, 6};
    lookup[0x91] = (Instruction){"STA", STA, INDY, 6};
    lookup[0xA2] = (Instruction){"LDX", LDX, IMM, 2};    // Immediate
    lookup[0xA6] = (Instruction){"LDX", LDX, ZP0, 3};    // Zero Page
    lookup[0xB6] = (Instruction){"LDX", LDX, ZPY, 4};    // Zero Page,Y
    lookup[0xAE] = (Instruction){"LDX", LDX, ABS, 4};    // Absolute
    lookup[0xBE] = (Instruction){"LDX", LDX, ABSY, 4};   // Absolute,Y
    lookup[0x86] = (Instruction){"STX", STX, ZP0, 3};    // Zero Page
    lookup[0x96] = (Instruction){"STX", STX, ZPY, 4};    // Zero Page,Y
    lookup[0x8E] = (Instruction){"STX", STX, ABS, 4};    // Absolute
    lookup[0x84] = (Instruction){"STY", STY, ZP0, 3};    // Zero Page
    lookup[0x94] = (Instruction){"STY", STY, ZPX, 4};    // Zero Page,Y
    lookup[0x8C] = (Instruction){"STY", STY, ABS, 4};    // Absolute
    lookup[0xAA] = (Instruction){"TAX", TAX, IMP, 2};
    lookup[0xA8] = (Instruction){"TAY", TAY, IMP, 2};
    lookup[0x8A] = (Instruction){"TXA", TXA, IMP, 2};
    lookup[0x98] = (Instruction){"TYA", TYA, IMP, 2};
    lookup[0x69] = (Instruction){"ADC", ADC, IMM, 2};    // Immediate
    lookup[0x65] = (Instruction){"ADC", ADC, ZP0, 3};    // Zero Page
    lookup[0x75] = (Instruction){"ADC", ADC, ZPX, 4};    // Zero Page,X
    lookup[0x6D] = (Instruction){"ADC", ADC, ABS, 4};    // Absolute
    lookup[0x7D] = (Instruction){"ADC", ADC, ABSX, 4};   // Absolute,X (+1 if page crossed)
    lookup[0x79] = (Instruction){"ADC", ADC, ABSY, 4};   // Absolute,Y (+1 if page crossed)
    lookup[0x61] = (Instruction){"ADC", ADC, INDX, 6};   // Indirect,X
    lookup[0x71] = (Instruction){"ADC", ADC, INDY, 5};   // Indirect,Y (+1 if page crossed)
    lookup[0xE9] = (Instruction){"SBC", SBC, IMM, 2};    // Immediate
    lookup[0xE5] = (Instruction){"SBC", SBC, ZP0, 3};    // Zero Page
    lookup[0xF5] = (Instruction){"SBC", SBC, ZPX, 4};    // Zero Page,X
    lookup[0xED] = (Instruction){"SBC", SBC, ABS, 4};    // Absolute
    lookup[0xFD] = (Instruction){"SBC", SBC, ABSX, 4};   // Absolute,X
    lookup[0xF9] = (Instruction){"SBC", SBC, ABSY, 4};   // Absolute,Y
    lookup[0xE1] = (Instruction){"SBC", SBC, INDX, 6};   // (Indirect,X)
    lookup[0xF1] = (Instruction){"SBC", SBC, INDY, 5};   // (Indirect),Y
    lookup[0xE6] = (Instruction){"INC", INC, ZP0, 5};    // Zero Page
    lookup[0xF6] = (Instruction){"INC", INC, ZPX, 6};    // Zero Page,X
    lookup[0xEE] = (Instruction){"INC", INC, ABS, 6};    // Absolute
    lookup[0xFE] = (Instruction){"INC", INC, ABSX, 7};   // Absolute,X
    lookup[0xC6] = (Instruction){"DEC", DEC, ZP0, 5};
    lookup[0xD6] = (Instruction){"DEC", DEC, ZPX, 6};
    lookup[0xCE] = (Instruction){"DEC", DEC, ABS, 6};
    lookup[0xDE] = (Instruction){"DEC", DEC, ABSX, 7};
    lookup[0xE8] = (Instruction){"INX", INX, IMP, 2};
    lookup[0xC8] = (Instruction){"INY", INY, IMP, 2};
    lookup[0xCA] = (Instruction){"DEX", DEX, IMP, 2};
    lookup[0x88] = (Instruction){"DEY", DEY, IMP, 2};
    lookup[0x0A] = (Instruction){"ASL", ASL, ACU, 2};    // Accumulator
    lookup[0x06] = (Instruction){"ASL", ASL, ZP0, 5};    // Zero Page
    lookup[0x16] = (Instruction){"ASL", ASL, ZPX, 6};    // Zero Page,X
    lookup[0x0E] = (Instruction){"ASL", ASL, ABS, 6};    // Absolute
    lookup[0x1E] = (Instruction){"ASL", ASL, ABSX, 7};   // Absolute,X
    // LSR (Logical Shift Right)
    lookup[0x4A] = (Instruction){"LSR", LSR, ACU, 2};     // Accumulator
    lookup[0x46] = (Instruction){"LSR", LSR, ZP0, 5};     // Zero Page
    lookup[0x56] = (Instruction){"LSR", LSR, ZPX, 6};     // Zero Page,X
    lookup[0x4E] = (Instruction){"LSR", LSR, ABS, 6};     // Absolute
    lookup[0x5E] = (Instruction){"LSR", LSR, ABSX, 7};    // Absolute,X
    lookup[0x2A] = (Instruction){"ROL", ROL, ACU, 2};    // Accumulator
    lookup[0x26] = (Instruction){"ROL", ROL, ZP0, 5};    // Zero Page
    lookup[0x36] = (Instruction){"ROL", ROL, ZPX, 6};    // Zero Page,X
    lookup[0x2E] = (Instruction){"ROL", ROL, ABS, 6};    // Absolute
    lookup[0x3E] = (Instruction){"ROL", ROL, ABSX, 7};   // Absolute,X
    lookup[0x6A] = (Instruction){"ROR", ROR, ACU, 2};    // Accumulator
    lookup[0x66] = (Instruction){"ROR", ROR, ZP0, 5};    // Zero Page
    lookup[0x76] = (Instruction){"ROR", ROR, ZPX, 6};    // Zero Page,X
    lookup[0x6E] = (Instruction){"ROR", ROR, ABS, 6};    // Absolute
    lookup[0x7E] = (Instruction){"ROR", ROR, ABSX, 7};   // Absolute,X
    lookup[0x29] = (Instruction){"AND", AND, IMM, 2};
    lookup[0x25] = (Instruction){"AND", AND, ZP0, 3};
    lookup[0x35] = (Instruction){"AND", AND, ZPX, 4};
    lookup[0x2D] = (Instruction){"AND", AND, ABS, 4};
    lookup[0x3D] = (Instruction){"AND", AND, ABSX, 4};
    lookup[0x39] = (Instruction){"AND", AND, ABSY, 4};
    lookup[0x21] = (Instruction){"AND", AND, INDX, 6};
    lookup[0x31] = (Instruction){"AND", AND, INDY, 5};
    lookup[0x09] = (Instruction){"ORA", ORA, IMM, 2};
    lookup[0x05] = (Instruction){"ORA", ORA, ZP0, 3};
    lookup[0x15] = (Instruction){"ORA", ORA, ZPX, 4};
    lookup[0x0D] = (Instruction){"ORA", ORA, ABS, 4};
    lookup[0x1D] = (Instruction){"ORA", ORA, ABSX, 4};
    lookup[0x19] = (Instruction){"ORA", ORA, ABSY, 4};
    lookup[0x01] = (Instruction){"ORA", ORA, INDX, 6};
    lookup[0x11] = (Instruction){"ORA", ORA, INDY, 5};
    lookup[0x49] = (Instruction){"EOR", EOR, IMM, 2};
    lookup[0x45] = (Instruction){"EOR", EOR, ZP0, 3};
    lookup[0x55] = (Instruction){"EOR", EOR, ZPX, 4};
    lookup[0x4D] = (Instruction){"EOR", EOR, ABS, 4};
    lookup[0x5D] = (Instruction){"EOR", EOR, ABSX, 4};
    lookup[0x59] = (Instruction){"EOR", EOR, ABSY, 4};
    lookup[0x41] = (Instruction){"EOR", EOR, INDX, 6};
    lookup[0x51] = (Instruction){"EOR", EOR, INDY, 5};
    lookup[0x24] = (Instruction){"BIT", BIT, ZP0, 3};
    lookup[0x2C] = (Instruction){"BIT", BIT, ABS, 4};
    lookup[0xC9] = (Instruction){"CMP", CMP, IMM, 2};    // Immediate
    lookup[0xC5] = (Instruction){"CMP", CMP, ZP0, 3};    // Zero Page
    lookup[0xD5] = (Instruction){"CMP", CMP, ZPX, 4};    // Zero Page,X
    lookup[0xCD] = (Instruction){"CMP", CMP, ABS, 4};    // Absolute
    lookup[0xDD] = (Instruction){"CMP", CMP, ABSX, 4};   // Absolute,X (+1 if page crossed)
    lookup[0xD9] = (Instruction){"CMP", CMP, ABSY, 4};   // Absolute,Y (+1 if page crossed)
    lookup[0xC1] = (Instruction){"CMP", CMP, INDX, 6};   // Indirect,X
    lookup[0xD1] = (Instruction){"CMP", CMP, INDY, 5};   // Indirect,Y (+1 if page crossed)
    // Add CPX instructions:
    lookup[0xE0] = (Instruction){"CPX", CPX, IMM, 2};
    lookup[0xE4] = (Instruction){"CPX", CPX, ZP0, 3};
    lookup[0xEC] = (Instruction){"CPX", CPX, ABS, 4};
    lookup[0xC0] = (Instruction){"CPY", CPY, IMM, 2};
    lookup[0xC4] = (Instruction){"CPY", CPY, ZP0, 3};
    lookup[0xCC] = (Instruction){"CPY", CPY, ABS, 4};
    lookup[0x90] = (Instruction){"BCC", BCC, REL, 2};
    lookup[0xB0] = (Instruction){"BCS", BCS, REL, 2};
    lookup[0xF0] = (Instruction){"BEQ", BEQ, REL, 2};
    lookup[0xD0] = (Instruction){"BNE", BNE, REL, 2};
    lookup[0x4C] = (Instruction){"JMP", JMP, ABS, 3};
    lookup[0x6C] = (Instruction){"JMP", JMP, IND, 5};
    lookup[0x20] = (Instruction){"JSR", JMP, ABS, 6};
    lookup[0x60] = (Instruction){"RTS", RTS, IMP, 6};
    lookup[0x00] = (Instruction) {"BRK", BRK, IMP, 7};
    lookup[0x40] = (Instruction) {"RTI", RTI, IMP, 6};
    lookup[0x48] = (Instruction){"PHA", PHA, IMP, 3};
    lookup[0x68] = (Instruction){"PLA", PLA, IMP, 4};
    lookup[0x08] = (Instruction){"PHP", PHP, IMP, 3};
    lookup[0x28] = (Instruction){"PLP", PLP, IMP, 4};
    lookup[0x9A] = (Instruction){"TSX", TSX, IMP, 2};
    lookup[0xBA] = (Instruction){"TXS", TXS, IMP, 2};
    lookup[0x18] = (Instruction){"CLC", CLC, IMP, 2};
    lookup[0x38] = (Instruction){"SEC", SEC, IMP, 2};
    lookup[0x58] = (Instruction){"CLI", CLI, IMP, 2};
    lookup[0x78] = (Instruction){"SEI", SEI, IMP, 2};
    lookup[0xD8] = (Instruction){"CLD", CLD, IMP, 2};
    lookup[0xF8] = (Instruction){"SED", SED, IMP, 2};
    lookup[0xB8] = (Instruction){"CLV", CLV, IMP, 2};
}
void run_instruction() {
    Instruction instr = lookup[cpu.memory[cpu.pc]];
    instr.addr();
    instr.op();
    cpu.cycles = instr.cycles + cpu.extracycles;
    cpu.extracycles = 0;
}

void reset_cpu() {
        cpu.a = 0; cpu.x = 0; cpu.y = 0; cpu.status = 0b00000000;
        cpu.pc = 0; cpu.fetched = 0; cpu.addr_abs = 0;
        cpu.cycles = 0; cpu.extracycles = 0;
        cpu.sp=0xFD;
        for(int i=0x0000; i<=0x1FFF; i++) cpu.memory[i] = 0;
        for(int i=0x6000; i<=0x7FFF; i++) cpu.memory[i] = 0;
        cpu.pc = (read(0xFFFC)) | (read(0xFFFD) << 8);
    }
void nmi(){
    uint8_t hi = ((cpu.pc)>>8) & 0xFF;
    uint8_t lo = (cpu.pc) & 0xFF;
    push_stack(hi);
    push_stack(lo);
    uint8_t flagspushed=cpu.status | 0b00100000;
    push_stack(flagspushed);
    uint16_t vector = ((uint16_t)read(0xFFFB) << 8) | read(0xFFFA);
    cpu.pc = vector;
    cpu.status |= 0b00000100;
}
void cpu_clock() {
    if (cpu.cycles == 0) {
        run_instruction();
    } else if (cpu.cycles > 0) {
        cpu.cycles--;
    }
}
