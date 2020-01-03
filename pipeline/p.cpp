#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#define MAX_SIZE 101
#define SETW_NUM 18

using namespace std;

// register
int regSize = 10;
int regs[10] = {0, 9, 8, 7, 1,
                2, 3, 4, 5, 6};
// data memory
int memSize = 5;
int mem[5] = {5, 9, 4, 8, 7};

// read file
fstream file;
char buffer[1024];

// instuction
string inst[ MAX_SIZE ];
int instNum = 0;

// program counter
static int PC = 0;

int binaryToDecimal(string);
int twoPow(int);
void readFile();
void regState();
void memState();
void fetchInst();

struct IF_ID {
    string instCode = "00000000000000000000000000000000";
    int rs = 0;
    int rt = 0;
    int rd = 0;
    // has no instuction check
    bool noInst = false;
    bool fir = true;

    void fetchInst(string icode) {
        
        instCode = icode;
        rs = binaryToDecimal( instCode.substr(6, 5) );
        rt = binaryToDecimal( instCode.substr(11, 5) );
    }

    void reset() {

        instCode = "00000000000000000000000000000000";
        rs = 0;
        rt = 0;
        rd = 0;
    }
    void hasNoInst() {
        
        noInst = true;
        instCode = "00000000000000000000000000000000";
    }
    void state() {
        
        cout << "IF/ID :\n";
        cout << setw(SETW_NUM) << "PC" << dec << PC*4 << '\n';
        cout << setw(SETW_NUM) << "Instruction" << instCode << '\n';
        cout << '\n';
    }
};

struct ID_EX {
    // control signals
    int RegDst = 0;
    int ALUOp1 = 0;
    int ALUOp0 = 0;
    int ALUSrc = 0;
    int Branch = 0;
    int MemRead = 0;
    int MemWrite = 0;
    int RegWrite = 0;
    int MemToReg = 0;
    string signalCode = "000000000";
    // datas
    int readData1 = 0;
    int readData2 = 0;
    int signExt = 0;
    int rs = 0;
    int rt = 0;
    int rd = 0;
    int ALUctl = 0;
    // first time
    bool first = true;

    void reset() {
        RegDst = 0;
        ALUOp1 = 0;
        ALUOp0 = 0;
        ALUSrc = 0;
        Branch = 0;
        MemRead = 0;
        MemWrite = 0;
        RegWrite = 0;
        MemToReg = 0;
        signalCode = "000000000";
        readData1 = 0;
        readData2 = 0;
        signExt = 0;
        rs = 0;
        rt = 0;
        rd = 0;
        ALUctl = 0;
    }

    void codeDeal(IF_ID IFID) {
        
        reset();
        // if(IFID.fir) {
        //     IFID.fir = false;
        //     return;
        // }
        cout << "TEST:   " << IFID.rd << '\n';

        // if(first) {
        //     first = false;
        //     return;
        // }
        if(IFID.noInst) return;
        string op = IFID.instCode.substr(0, 6);
        string func = IFID.instCode.substr(26, 6);
        signExt = binaryToDecimal( IFID.instCode.substr(16, 16) );

        // R-type
        if(op == "000000") {
            
            rs = binaryToDecimal( IFID.instCode.substr(6, 5) );
            rt = binaryToDecimal( IFID.instCode.substr(11, 5) );
            rd = binaryToDecimal( IFID.instCode.substr(16, 5) );
            /** signals initial **/
            RegDst = 1;
            ALUOp1 = 1;
            ALUOp0 = 0;
            ALUSrc = 0;

            Branch = 0;
            MemRead = 0;
            MemWrite = 0;

            RegWrite = 1;
            MemToReg = 0;

            signalCode = "110000010";
        // I-type
        } else {
            rs = binaryToDecimal( IFID.instCode.substr(6, 5) );
            rt = binaryToDecimal( IFID.instCode.substr(11, 5) );
            // lw
            if(op == "100011") {
                /** signals initial **/
                RegDst = 0;
                ALUOp1 = 0;
                ALUOp0 = 0;
                ALUSrc = 1;

                Branch = 0;
                MemRead = 1;
                MemWrite = 0;

                RegWrite = 1;
                MemToReg = 1;

                signalCode = "000101011";
            // sw
            } else if(op == "101011") {
                /** signals initial **/
                RegDst = 0;
                ALUOp1 = 0;
                ALUOp0 = 0;
                ALUSrc = 1;

                Branch = 0;
                MemRead = 0;
                MemWrite = 1;

                RegWrite = 0;
                MemToReg = 0;

                signalCode = "000100100";
            // beq
            } else if(op == "000100") {
                /** signals initial **/
                RegDst = 0;
                ALUOp1 = 0;
                ALUOp0 = 1;
                ALUSrc = 0;

                Branch = 1;
                MemRead = 0;
                MemWrite = 0;

                RegWrite = 0;
                MemToReg = 0;
                signalCode = "001010000";
            }   
        }
        string tempALUCtl;
        // ALUctr
        if(ALUOp1 == 1 && ALUOp0 == 1) {
            tempALUCtl = "000";
        } else if(ALUOp1 == 0 && ALUOp0 == 0) {
            tempALUCtl = "010";
        } else if(ALUOp1 == 0 && ALUOp0 == 1) {
            tempALUCtl = "110";        
        } else {
            if(func == "100000") {
                tempALUCtl = "010";
            } else if(func == "100010") {
                tempALUCtl = "110";
            } else if(func == "100100") {
                tempALUCtl = "000";
            } else if(func == "100101") {
                tempALUCtl = "001";
            } else if(func == "101010") {
                tempALUCtl = "111";
            }
        }
        ALUctl = binaryToDecimal(tempALUCtl);
        // read data
        readData1 = regs[ rs ];
        readData2 = regs[ rt ];
    }

    void state() {
        
        cout << "ID/EX :\n";
        cout << setw(SETW_NUM) << "ReadData1" << readData1 << '\n';
        cout << setw(SETW_NUM) << "ReadData2" << readData2 << '\n';
        cout << setw(SETW_NUM) << "sign_ext" << signExt << '\n';
        cout << setw(SETW_NUM) << "Rs" << rs << '\n';
        cout << setw(SETW_NUM) << "Rt" << rt << '\n';
        cout << setw(SETW_NUM) << "Rd" << rd << '\n';
        cout << setw(SETW_NUM) << "Control signals" << signalCode << '\n';
        cout << '\n';
    }
};

struct EX_MEM {
    // control signals
    int Branch = 0;
    int MemRead = 0;
    int MemWrite = 0;
    int RegWrite = 0;
    int MemToReg = 0;
    string signalCode = "00000";
    // datas
    int ALUout = 0;
    int writeData = 0;
    int reg = 0;
    int immeALU = 0;

    void reset() {
        Branch = 0;
        MemRead = 0;
        MemWrite = 0;
        RegWrite = 0;
        MemToReg = 0;
        signalCode = "00000";
        ALUout = 0;
        writeData = 0;
        reg = 0;
        immeALU = 0;
    }
    void passSignalsAndOperation(ID_EX IDEX) {

        reset();
        /**control signals**/
        Branch = IDEX.Branch;
        MemRead = IDEX.MemRead;
        MemWrite = IDEX.MemWrite;
        RegWrite = IDEX.RegWrite;
        MemToReg = IDEX.MemToReg;
        signalCode = IDEX.signalCode.substr(4, 5);
        /**datas**/
        writeData = IDEX.readData2;
        // R-type
        if(IDEX.RegDst) {
            reg = IDEX.rd;
        // lw
        } else {
            reg = IDEX.rt;
        }

        /**operation**/
        // sw or lw
        if(IDEX.ALUSrc) {
            ALUout = IDEX.readData1 + IDEX.signExt;
            immeALU = IDEX.readData1 + (IDEX.signExt << 2);
        // branch 
        } else{
            // and
            if(IDEX.ALUctl == 0) {
                ALUout = IDEX.readData1 & IDEX.readData2;
            // or
            } else if(IDEX.ALUctl == 1) {
                ALUout = IDEX.readData1 | IDEX.readData2;            
            // add
            } else if(IDEX.ALUctl == 2) {
                ALUout = IDEX.readData1 + IDEX.readData2;
            // sub
            } else if(IDEX.ALUctl == 6) {
                ALUout = IDEX.readData1 - IDEX.readData2;
            // slt
            } else if(IDEX.ALUctl == 7) {
                ALUout = IDEX.readData1 < IDEX.readData2 ? 1 : 0;
            }
            // for branch
            immeALU = PC + (IDEX.signExt << 2);
        }
    }
    
    void state() {
        
        cout << "EX/MEM :\n";
        cout << setw(SETW_NUM) << "ALUout" << ALUout << '\n';
        cout << setw(SETW_NUM) << "WriteData" << writeData << '\n';
        cout << setw(SETW_NUM) << "Rt/Rd" << reg << '\n';
        cout << setw(SETW_NUM) << "Control signals" << signalCode << '\n';
        cout << '\n';
    }
};

struct MEM_WB {
    // control signals
    int RegWrite = 0;
    int MemToReg = 0;
    string signalCode = "00";
    // data
    int readData = 0;
    int ALUout = 0;
    int reg = 0;
    
    void reset() {
        RegWrite = 0;
        MemToReg = 0;
        signalCode = "00";
        readData = 0;
        ALUout = 0;
        reg = 0;
    }
    void passSignalsAndOperation(EX_MEM EXMEM) {

        reset();
        /**control signals**/
        RegWrite = EXMEM.RegWrite;
        MemToReg = EXMEM.MemToReg;
        signalCode = EXMEM.signalCode.substr(3, 2);
        /**datas**/
        ALUout = EXMEM.ALUout;
        reg = EXMEM.reg;
        
        /**operation**/
        // branch
        if(EXMEM.Branch && ALUout == 0) {
            PC = EXMEM.immeALU;
        // lw
        } else if(EXMEM.MemRead) {
            readData = mem[ (ALUout >> 2) ];
        // sw
        } else if(EXMEM.MemWrite) {
            mem[ (ALUout >> 2) ] = EXMEM.writeData;
        }
    }
    void state() {

        cout << "MEM/WB :\n";
        cout << setw(SETW_NUM) << "ReadData" << readData << '\n';
        cout << setw(SETW_NUM) << "ALUout" << ALUout << '\n';
        cout << setw(SETW_NUM) << "Rt/Rd" << reg << '\n';
        cout << setw(SETW_NUM) << "Control signals" << signalCode << '\n';
        cout << "=================================================================\n";
    }
};

struct ForwardingUnit {

    void forward(ID_EX IDEX, EX_MEM EXMEM, MEM_WB MEMWB) {
        if(EXMEM.RegWrite && EXMEM.reg == IDEX.rs) {
            IDEX.readData1 = EXMEM.ALUout;
        }
        if(MEMWB.RegWrite && EXMEM.reg != IDEX.rs && MEMWB.reg == IDEX.rs) {
            IDEX.readData1 = MEMWB.ALUout;
        }
        if(EXMEM.RegWrite && EXMEM.reg == IDEX.rt) {
            IDEX.readData2 = EXMEM.ALUout;
        }
        if(MEMWB.RegWrite && EXMEM.reg != IDEX.rt && MEMWB.reg == IDEX.rt) {
            IDEX.readData2 = MEMWB.ALUout;
        }
        
    }
};

struct StallingUnit {

    bool handlingStall(IF_ID IFID, ID_EX IDEX) {
        if(IDEX.MemRead && ((IDEX.rt == IFID.rs) || (IDEX.rt == IFID.rt))) {
            return true;
        }
        return false;
    }
};

struct BranchDelayUnit {

    bool handlingBranch(ID_EX IDEX) {

    }
};

// hazard handle unit
static ForwardingUnit FUnit;
static StallingUnit SUnit;
static BranchDelayUnit BUnit;

// pipeline registers
static IF_ID IFID;
static ID_EX IDEX;
static EX_MEM EXMEM;
static MEM_WB MEMWB;

// step 5
void writeBack(MEM_WB);

int main() {

    int CC = 1;

    readFile();
    fetchInst();
    while(CC <= 5+instNum-1) {
        writeBack(MEMWB);
        MEMWB.passSignalsAndOperation(EXMEM);
        EXMEM.passSignalsAndOperation(IDEX);
        if(!SUnit.handlingStall(IFID, IDEX)) {
            IDEX.codeDeal(IFID);
            if(BUnit.handlingBranch(IDEX)) {
                IFID.reset();
            }
            FUnit.forward(IDEX, EXMEM, MEMWB);
            if(CC > instNum) {
                IFID.hasNoInst();
            } else {
                IFID.fetchInst(inst[PC]);
            }
            PC += 1;
        }
        // print State
        cout << "CC " << CC << ":\n\n";
        regState();
        memState();
        IFID.state();
        IDEX.state();
        EXMEM.state();
        MEMWB.state();
        CC++;
    }
    return 0;
}

void regState() {
    
    cout << "Registers:\n";
    for(int i = 0; i < regSize; i++) {
        cout << "$" << i << ": ";
        cout << regs[i] << '\n';
    }
    cout << '\n';
}

void memState() {

    cout << "Data memory:\n";
    for(int i = 0; i < memSize; i++) {
        cout << "0x" << setw(2) << setfill('0') << hex << uppercase << i*4 << ": ";
        cout << mem[i] << '\n';
    }
    cout << setfill(' ') << left;
    cout << '\n';
}

void readFile() {
    
    file.open("SampleInput", ios::in);
    if(!file) {
        cout << "Open file ERROR\n";
    } else {
        file.read(buffer, sizeof(buffer));
    }
    file.close();
}

void fetchInst() {

    int i = 0;
    for(int j = 0; i < MAX_SIZE; i++) {
        inst[i] = "";
        do {
            inst[i] += buffer[j];
            j++;
        } while(j%32 != 0);
        // eof
        if(buffer[j] == '\0') break;
    }
    instNum = i+1;
}

// 2^a
int twoPow(int a) {

    if(a == 0) return 1;

    int i = 1;
    while(a--) {
        i *= 2;
    }

    return i;
}

int binaryToDecimal(string s) {

    reverse(s.begin(), s.end());
    int dec = 0;
    for(int i = s.size()-1; i >= 0; i--) {
        if(s[i] == '1') {
            dec += twoPow(i);
        }
    }
    return dec;
}

void writeBack(MEM_WB MEMWB) {

    int data;
    if(MEMWB.MemToReg) {
        data = MEMWB.readData;
    } else {
        data = MEMWB.ALUout;
    }
    if(MEMWB.RegWrite) {
        regs[ MEMWB.reg ] = data;
    }
}