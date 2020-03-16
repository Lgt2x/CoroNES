#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


/**
 * Returns the adressing modes that can be used by the given adress
 * 0 : accumulator | implied
 * 1 : absolute
 * 2 : immediate
 * 3 : zero page | relative
 * 4 : absolute indexed with X
 * 5 : absolute indexed with Y
 * 6 : zero page indexed with X
 * 7 : zero page indexed with Y
 * 8 : zero page indexed indirect
 * 9 : zero page indexed with Y
 * a : indirect
 * b : parsing error
 *
 * @param addr string corresponding to the adress in the assembly code
 * @return the adressing modes stored as a char, given the correpondance above
 */
char addressingMode(string addr) {
    if (addr.length() == 0) {
        // Nothing after the instruction : Accumulator or implied
        return 0;
    } else if (addr.length() == 5 && addr[0] == '$' && addr[3] == ',') {
        if (addr[4] == 'X' || addr[4] == 'x') {
            // $XX,x : zero page indexed with X
            return 6;
        } else if (addr[4] == 'Y' || addr[4] == 'x') {
            // $XX,Y : zero page indexed with Y
            return 7;
        }
    } else if (addr.length() == 5 && addr[0] == '$') {
        // $XXXX : absolute
        return 1;
    } else if (addr.length() == 4 && addr[0] == '#' && addr[1] == '$') {
        // #$XX : immediate
        return 2;
    } else if (addr.length() == 3 && addr[0] == '$') {
        // $XX : zero page / relative
        return 3;
    } else if (addr.length() == 7 && addr[0] == '$' && addr[5] == ',') {
        if (addr[6] == 'X' || addr[6] == 'x') {
            // $XX,x : absolute indexed with X
            return 4;
        } else if (addr[6] == 'Y' || addr[6] == 'y') {
            // $XX,y : absolute indexed with X
            return 5;
        }
    } else if (addr.length() == 7 && addr[0] == '(' && addr[1] == '$' && addr[4] == ','
               && (addr[5] == 'X' || addr[5] == 'x') && addr[6] == ')') {
        // ($XX,X) : zero page indexed indirect
        return 8;
    } else if (addr.length() == 7 && addr[0] == '(' && addr[1] == '$' && addr[4] == ')'
               && addr[5] == ',' && (addr[6] == 'Y' || addr[6] == 'y')) {
        // ($XX), Y :zero page indexed with Y
        return 9;
    } else if (addr.length() == 7 && addr[0] == '(' && addr[1] == '$' && addr[6] == ')') {
        // ($XXXX) : indirect
        return 0xa;
    } else {
        // parsing error
        return 0xb;
    }
    return 0xb;
}

/**
 * Returns the register value associated with the instruction
 * @param addr register address
 * @param mode addressing mode
 * @return register value, 1 or 2 bytes depending on the addressing mode
 */
int addressValue(const string &addr, char mode) {
    unsigned int value = 0;

    switch (mode) {
        case 0:
            value = 0;
            break;
        case 1:
            value = stoi(addr.substr(1), 0, 16);
            break;
        case 2:
            value = stoi(addr.substr(2), 0, 16);
            break;
        case 3:
            value = stoi(addr.substr(1), 0, 16);
            break;
        case 4:
        case 5:
            value = stoi(addr.substr(1, 4), 0, 16);
            break;
        case 6:
        case 7:
            value = stoi(addr.substr(1, 2), 0, 16);
            break;
        case 8:
        case 9:
            value = stoi(addr.substr(2, 2), 0, 16);
            break;
        case 10:
            value = stoi(addr.substr(2, 4), 0, 16);
            break;
        default:
            value = 0;
    }

    return value;
}


/**
 * Returns the byte representing the instruction && addressing mode
 *
 * @param instruction command to compute
 * @param mode addressing mode used along with the instruction
 * @return 1 byte corresponding to the instruction
 */
unsigned char opcode(string instruction, char mode) {
    if (instruction == "ADC") {
        switch (mode) {
            case 1:
                return 0x6D;
            case 2:
                return 0x69;
            case 3:
                return 0x65;
            case 4:
                return 0x7D;
            case 5:
                return 0x79;
            case 6:
                return 0x75;
            case 8:
                return 0x61;
            case 9:
                return 0x71;
        }
    } else if (instruction == "AND") {
        // AND : bitwise AND with accumulator
        switch (mode) {
            case 1:
                return 0x2D;
            case 2:
                return 0x29;
            case 3:
                return 0x25;
            case 4:
                return 0x3D;
            case 5:
                return 0x39;
            case 6:
                return 0x35;
            case 8:
                return 0x21;
            case 9:
                return 0x31;
        }
    } else if (instruction == "ASL") {
        // ASL : Arithmetic Shift Left
        switch (mode) {
            case 0:
                return 0x0A;
            case 1:
                return 0x25;
            case 3:
                return 0x06;
            case 4:
                return 0x1E;
            case 6:
                return 0x16;
        }
    } else if (instruction == "BIT") {
        // BIT : test BITS
        switch (mode) {
            case 1:
                return 0x2C;
            case 3:
                return 0x24;
        }
    } else if (instruction == "BPL" && mode == 3) {
        // BPL : Branch on PLus
        return 0x10;
    } else if (instruction == "BMI" && mode == 3) {
        // BMI : Branch on MInus
        return 0x30;
    } else if (instruction == "BVC" && mode == 3) {
        // BVC : Branch on oVerflow Clear
        return 0x50;
    } else if (instruction == "BVS" && mode == 3) {
        // BPL : Branch on oVerflow Set
        return 0x70;
    } else if (instruction == "BCC" && mode == 3) {
        // BPL : Branch on Carry Clear
        return 0x90;
    } else if (instruction == "BCS" && mode == 3) {
        // BPL : Branch on Carry Set
        return 0xB0;
    } else if (instruction == "BNE" && mode == 3) {
        // BNE : Branch on Not Equal
        return 0xD0;
    } else if (instruction == "BEQ" && mode == 3) {
        // BEQ : Branch on EQual
        return 0xF0;
    } else if (instruction == "BRK" && mode == 0) {
        // BRK : Break
        return 0x00;
    } else if (instruction == "CMP") {
        // CMP : CoMPaire accumulator
        switch (mode) {
            case 1:
                return 0xCD;
            case 2:
                return 0xC9;
            case 3:
                return 0xC5;
            case 4:
                return 0xDD;
            case 5:
                return 0xD9;
            case 6:
                return 0xD5;
            case 8:
                return 0xC1;
            case 9:
                return 0xD1;
        }
    } else if (instruction == "CPX") {
        // CPX: comPare X register
        switch (mode) {
            case 1:
                return 0xEC;
            case 2:
                return 0xE0;
            case 3:
                return 0xE4;
        }
    } else if (instruction == "CPY") {
        // CPY: comPare Y register
        switch (mode) {
            case 1:
                return 0xCC;
            case 2:
                return 0xC0;
            case 3:
                return 0xC4;
        }
    } else if (instruction == "DEC") {
        // DEC : DECrement memory
        switch (mode) {
            case 1:
                return 0xCE;
            case 3:
                return 0xC6;
            case 4:
                return 0xDE;
            case 6:
                return 0xD6;
        }
    } else if (instruction == "EOR") {
        // EOR : bitwise Exclusive OR
        switch (mode) {
            case 1:
                return 0x4D;
            case 2:
                return 0x49;
            case 3:
                return 0x45;
            case 4:
                return 0x5D;
            case 5:
                return 0x59;
            case 6:
                return 0x55;
            case 8:
                return 0x41;
            case 9:
                return 0x51;
        }
    } else if (instruction == "CLC" && mode == 0) {
        // CLC : CLear Carry
        return 0x18;
    } else if (instruction == "SEC" && mode == 0) {
        // SEC : SEt Carry
        return 0x38;
    } else if (instruction == "CLI" && mode == 0) {
        // CLI : CLear Interrupt
        return 0x58;
    } else if (instruction == "SEI" && mode == 0) {
        // SEI : SEt Interrupt
        return 0x78;
    } else if (instruction == "CLV" && mode == 0) {
        // CLV : CLear oVerflow
        return 0xB8;
    } else if (instruction == "CLD" && mode == 0) {
        // CLD : CLear Decimal
        return 0xD8;
    } else if (instruction == "SED" && mode == 0) {
        // SED : SEt Decimal
        return 0xF8;
    } else if (instruction == "INC") {
        // INC : INCrement memory
        switch (mode) {
            case 1:
                return 0xEE;
            case 3:
                return 0xE6;
            case 4:
                return 0xFE;
            case 6:
                return 0xF6;
        }
    } else if (instruction == "JMP") {
        // JMP : JuMP
        switch (mode) {
            case 1:
                return 0x4C;
            case 0xA:
                return 0x6C;
        }
    } else if (instruction == "JSR") {
        // JSR : Jump to SubRoutine
        switch (mode) {
            case 1:
                return 0x20;
        }
    } else if (instruction == "LDA") {
        // LDA : LoaD Accumulator
        switch (mode) {
            case 1:
                return 0xAD;
            case 2:
                return 0xA9;
            case 3:
                return 0xA5;
            case 4:
                return 0xBD;
            case 5:
                return 0xB9;
            case 6:
                return 0xB5;
            case 8:
                return 0xA1;
            case 9:
                return 0xB1;
        }
    } else if (instruction == "LDX") {
        // LDX (LoaD X register
        switch (mode) {
            case 1:
                return 0xAE;
            case 2:
                return 0xA2;
            case 3:
                return 0xA6;
            case 5:
                return 0xBE;
            case 7:
                return 0xB6;
        }
    } else if (instruction == "LDY") {
        // LDY (LoaD Y register
        switch (mode) {
            case 1:
                return 0xAC;
            case 2:
                return 0xA0;
            case 3:
                return 0xA4;
            case 4:
                return 0xBC;
            case 6:
                return 0xB4;
        }
    } else if (instruction == "LSR") {
        // LSR : Logical Shift Right
        switch (mode) {
            case 0:
                return 0x4A;
            case 1:
                return 0x4E;
            case 3:
                return 0x46;
            case 4:
                return 0x5E;
            case 6:
                return 0x56;
        }
    } else if (instruction == "NOP" && mode == 0) {
        // NOP : No OPeration
        return 0xEA;
    } else if (instruction == "ORA") {
        // ORA : bitwise OR with Accumulator
        switch (mode) {
            case 1:
                return 0x0D;
            case 2:
                return 0x09;
            case 3:
                return 0x05;
            case 4:
                return 0x1D;
            case 5:
                return 0x19;
            case 6:
                return 0xB5;
            case 8:
                return 0x01;
            case 9:
                return 0x11;
        }
    } else if (instruction == "TAX" && mode == 0) {
        // TAX : Transfer A to X
        return 0xAA;
    } else if (instruction == "TXA" && mode == 0) {
        // TXA : Transfer X to A
        return 0x8A;
    } else if (instruction == "DEX" && mode == 0) {
        // DEX : DEcrement X
        return 0xCA;
    } else if (instruction == "INX" && mode == 0) {
        // INX : INcrement X
        return 0xE8;
    } else if (instruction == "TAY" && mode == 0) {
        // TAY : Transfer A to Y
        return 0xA8;
    } else if (instruction == "TYA" && mode == 0) {
        // TYA : Transfer Y to A
        return 0x98;
    } else if (instruction == "DEY" && mode == 0) {
        // DEY : DEcrement Y
        return 0x88;
    } else if (instruction == "INY" && mode == 0) {
        // INY : INcrement Y
        return 0xC8;
    } else if (instruction == "ROL") {
        // ROtate Left
        switch (mode) {
            case 0:
                return 0x2A;
            case 1:
                return 0x3E;
            case 3:
                return 0x26;
            case 4:
                return 0X3E;
            case 6:
                return 0X36;
        }
    } else if (instruction == "ROR") {
        // ROtate Right
        switch (mode) {
            case 0:
                return 0x6A;
            case 1:
                return 0x6E;
            case 3:
                return 0x66;
            case 4:
                return 0X76;
            case 6:
                return 0X7E;
        }
    } else if (instruction == "RTI" && mode == 0) {
        // RTI : ReTurn from Interrupt
        return 0x40;
    } else if (instruction == "RTS" && mode == 0) {
        // RTS : ReTurn from Subroutine
        return 0x60;
    } else if (instruction == "SBC") {
        // SBC : Subtract with Carry
        switch (mode) {
            case 1:
                return 0xED;
            case 2:
                return 0xE9;
            case 3:
                return 0xE5;
            case 4:
                return 0xFD;
            case 5:
                return 0xF9;
            case 6:
                return 0xF5;
            case 8:
                return 0xE1;
            case 9:
                return 0xF1;
        }
    } else if (instruction == "STA") {
        // STA : STore Accumulator
        switch (mode) {
            case 1:
                return 0x8D;
            case 3:
                return 0x85;
            case 4:
                return 0x9D;
            case 5:
                return 0x99;
            case 6:
                return 0x95;
            case 8:
                return 0x81;
            case 9:
                return 0x91;
        }
    } else if (instruction == "TXS" && mode == 0) {
        // TXS : Transfer X to Stack pointer
        return 0x9A;
    } else if (instruction == "TSX" && mode == 0) {
        // TSX : Transfer Stack pointer to X
        return 0xBA;
    } else if (instruction == "PHA" && mode == 0) {
        // PHA : PusH Accumulator
        return 0x48;
    } else if (instruction == "PLA" && mode == 0) {
        // PLA : PuLl Accumulator
        return 0x68;
    } else if (instruction == "PHP" && mode == 0) {
        // PHP : PusH Processor status
        return 0x08;
    } else if (instruction == "PLP" && mode == 0) {
        // PLP : PuLl Processos status
        return 0x28;
    } else if (instruction == "STX") {
        // STX : STore X register
        switch (mode) {
            case 1:
                return 0x8E;
            case 3:
                return 0x86;
            case 7:
                return 0x96;
        }
    } else if (instruction == "STY") {
        // STY : STore Y register
        switch (mode) {
            case 1:
                return 0x8C;
            case 3:
                return 0x84;
            case 6:
                return 0x94;
        }
    }

    // 0xFF is returned as an error code, when the instruction and addressing mode don't match
    return 0xFF;
}


int main(int argc, char **argv) {
    ifstream file("prgm.asm");
    ofstream dest("rom.nes", ios::binary);

    if (!file) {
        cerr << "Unable to open program";
        return 1;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string command, addressing;
        if (!(iss >> command >> addressing)) { addressing = ""; }

        char mode = addressingMode(addressing);
        int value = addressValue(addressing, mode);
        unsigned char code = opcode(command, mode);

        dest << code << (char)value;
        if (mode == 1 || mode == 4 || mode == 5 || mode == 10) {
            dest  << (char)(value>>8);
        }
    }

    dest.close();
    file.close();

    return 0;
}
