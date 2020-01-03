#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#define MAX_SIZE 101
#define SETW_NUM 16
#define STEP 5
#define BUFFERSIZE 1024
#define FILENUM 4
using namespace std;

// register
int regSize = 10;
int regsValue[10] = {0, 9, 5, 7, 1,
                    2, 3, 4, 5, 6};
int regs[10];
// data memory
int memSize = 5;
int memValue[5] = {5, 9, 4, 8, 7};
int mem[5];

// read file
fstream file;
fstream write;
char buffer[ BUFFERSIZE ];

// instuction
string inst[ MAX_SIZE ];
int instNum = 0;

// program counter
static int PC = 0;

// others
int CC = 1;
int stallNum = 0;
bool first = true;
bool stall = false;

// file
int currentFile = 0;
string writeFileName[ FILENUM ] = {"branchResult.txt", "dataResult.txt", "genResult.txt", "loadResult.txt"};
string readFileName[ FILENUM ] = {"Branchhazard", "Datahazard", "General", "Lwhazard"};

void init();
int binaryToDecimal(string);
int twoPow(int);
void readFile();
void regState();
void memState();
void fetchInst();

/*
Every struct is a pipeline register or a function unit for forwarding, stalling and branching.
In those structs, each one has own datas, a function for printing the result and a function for flush registers.
*/

struct IF_ID {
    string instCode = "00000000000000000000000000000000";
    int rs = 0;
    int rt = 0;
    int rd = 0;
    // has no instuction check
    bool noInst = false;

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
        
        write << "IF/ID :\n";
        write << setw(SETW_NUM) << "PC" << dec << PC << '\n';
        write << setw(SETW_NUM) << "Instruction" << instCode << '\n';
        write << '\n';
    }
};
static IF_ID IFID;

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

    void codeDeal() {
        
        reset();
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
            // addi
            } else if(op == "001000") {
                /** signals initial **/
                RegDst = 0;
                ALUOp1 = 0;
                ALUOp0 = 0;
                ALUSrc = 1;

                Branch = 0;
                MemRead = 0;
                MemWrite = 0;

                RegWrite = 1;
                MemToReg = 0;
                signalCode = "000100010";
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
        
        write << "ID/EX :\n";
        write << setw(SETW_NUM) << "ReadData1" << readData1 << '\n';
        write << setw(SETW_NUM) << "ReadData2" << readData2 << '\n';
        write << setw(SETW_NUM) << "sign_ext" << signExt << '\n';
        write << setw(SETW_NUM) << "Rs" << rs << '\n';
        write << setw(SETW_NUM) << "Rt" << rt << '\n';
        write << setw(SETW_NUM) << "Rd" << rd << '\n';
        write << setw(SETW_NUM) << "Control signals" << signalCode << '\n';
        write << '\n';
    }
};
static ID_EX IDEX;

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
    // forward
    int forwardRs = 0;
    int forwardRt = 0;
    int forwardRsVal = 0;
    int forwardRtVal = 0;

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

    void passSignalsAndOperation() {

        reset();
        /**control signals**/
        Branch = IDEX.Branch;
        MemRead = IDEX.MemRead;
        MemWrite = IDEX.MemWrite;
        RegWrite = IDEX.RegWrite;
        MemToReg = IDEX.MemToReg;
        signalCode = IDEX.signalCode.substr(4, 5);
        /**datas**/
        // R-type
        if(IDEX.RegDst) {
            reg = IDEX.rd;
        // lw
        } else {
            reg = IDEX.rt;
        }
        int tmp1, tmp2;
        // be forwarded, update readData
        if(forwardRs) {
            tmp1 = IDEX.readData1;
            IDEX.readData1 = forwardRsVal;
        }
        if(forwardRt) {
            tmp2 = IDEX.readData2;
            IDEX.readData2 = forwardRtVal;
        }
        writeData = IDEX.readData2;
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
        }
        if(forwardRs) {
            IDEX.readData1 = tmp1;
        }
        if(forwardRt) {
            IDEX.readData2 = tmp2;
        }
        forwardRs = 0;
        forwardRt = 0;
        forwardRsVal = 0;
        forwardRtVal = 0;
    }
    
    void state() {
        
        write << "EX/MEM :\n";
        write << setw(SETW_NUM) << "ALUout" << ALUout << '\n';
        write << setw(SETW_NUM) << "WriteData" << writeData << '\n';
        write << setw(SETW_NUM) << "Rt/Rd" << reg << '\n';
        write << setw(SETW_NUM) << "Control signals" << signalCode << '\n';
        write << '\n';
    }
};
static EX_MEM EXMEM;

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
    void passSignalsAndOperation() {

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
        
        // lw
        } else if(EXMEM.MemRead) {
            readData = mem[ (ALUout >> 2) ];
        // sw
        } else if(EXMEM.MemWrite) {
            mem[ (ALUout >> 2) ] = EXMEM.writeData;
        }
    }
    void state() {

        write << "MEM/WB :\n";
        write << setw(SETW_NUM) << "ReadData" << readData << '\n';
        write << setw(SETW_NUM) << "ALUout" << ALUout << '\n';
        write << setw(SETW_NUM) << "Rt/Rd" << reg << '\n';
        write << setw(SETW_NUM) << "Control signals" << signalCode << '\n';
        write << "=================================================================\n";
    }
};
static MEM_WB MEMWB;

struct ForwardingUnit {
    void forward(){
        // forward hazard happens

        // for rs
        if(EXMEM.RegWrite && EXMEM.reg == IDEX.rs && EXMEM.reg != 0) {
            EXMEM.forwardRs = 1;
            EXMEM.forwardRsVal = EXMEM.ALUout;
        }
        if(MEMWB.RegWrite && EXMEM.reg != IDEX.rs && MEMWB.reg == IDEX.rs && MEMWB.reg != 0) {
            EXMEM.forwardRs = 1;
            EXMEM.forwardRsVal = MEMWB.ALUout;
        }
        //for rt
        if(EXMEM.RegWrite && EXMEM.reg == IDEX.rt && EXMEM.reg != 0) {
            EXMEM.forwardRt = 1;
            EXMEM.forwardRtVal = EXMEM.ALUout;
        }
        if(MEMWB.RegWrite && EXMEM.reg != IDEX.rt && MEMWB.reg == IDEX.rt && MEMWB.reg != 0) {
            EXMEM.forwardRt = 1;
            EXMEM.forwardRtVal = MEMWB.ALUout;
        }   
    }
};

struct StallingUnit {

    bool handlingStall() {
        // lw
        if(IDEX.MemRead && ((IDEX.rt == IFID.rs) || (IDEX.rt == IFID.rt))) {
            return true;
        }
        return false;
    }
};

struct BranchDelayUnit {

    bool handlingBranch() {
        if(IDEX.Branch && (IDEX.readData1 == IDEX.readData2)) {
            // pre instruction
            int oldPC = PC;
            PC += ((IDEX.signExt << 2)-4);
            instNum -= ((PC - oldPC) >> 2);
            return true;
        }
        return false;
    }
};

// hazard handle unit
static ForwardingUnit FUnit;
static StallingUnit SUnit;
static BranchDelayUnit BUnit;

// step 5
void writeBack();
void init();

int main() {
    // total five files
    while(currentFile < FILENUM) {
        write.open(writeFileName[ currentFile ], ios::trunc | ios::out);
        init();
        readFile();
        fetchInst();
        // cycle nums, instruction nums and stall nums
        while(CC <= STEP+instNum-1+stallNum) {
            writeBack();
            FUnit.forward();
            MEMWB.passSignalsAndOperation();
            EXMEM.passSignalsAndOperation();
            // lw
            if(stall) {
                stall = false;
                IDEX.reset();
            // not lw
            } else {
                if(first) {
                    first = false;
                } else {
                    // there is no more instructions
                    // or
                    // branch event happens
                    if(IFID.noInst || BUnit.handlingBranch()) {
                        IDEX.reset();
                    } else IDEX.codeDeal();
                }
                if(CC-stallNum > instNum) {
                    // there is no more instruction, so do not need fetch instructions
                    IFID.hasNoInst();
                } else {
                    IFID.fetchInst(inst[PC/4]);
                }
                PC += 4;
            }
            if(SUnit.handlingStall()) {
                // if next cycle will happen stall event
                stallNum++;
                stall = true;
            }
            // print all State
            write << "CC " << CC << ":\n\n";
            regState();
            memState();
            IFID.state();
            IDEX.state();
            EXMEM.state();
            MEMWB.state();
            CC++;
        }
        write.close();
        currentFile++;
    }
    
    return 0;
}

void regState() {
    
    write << "Registers:\n";
    for(int i = 0; i < regSize; i++) {
        write << "$" << i << ": ";
        write << regs[i] << '\n';
    }
    write << '\n';
}

void memState() {

    write << "Data memory:\n";
    for(int i = 0; i < memSize; i++) {
        write << "0x" << setw(2) << right << setfill('0') << hex << uppercase << i*4 << ": ";
        write << mem[i] << '\n';
    }
    write << setfill(' ') << left;
    write << '\n';
}

void readFile() {
    
    // read file and put datas in buffer
    file.open(readFileName[ currentFile ], ios::in);
    if(!file) {
        cerr << "Open file ERROR\n";
    } else {
        file.read(buffer, sizeof(buffer));
    }
    file.close();
}

void fetchInst() {

    // fetch instruction from file read buffer
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

// binary number of string type to integer
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

void writeBack() {
    // write back to memorys or registers
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

void init() {

    // initial for evrey instruction set
    for(int i = 0; i < regSize; i++) {
        regs[i] = regsValue[i];
    }
    for(int i = 0; i < memSize; i++) {
        mem[i] = memValue[i];
    }
    for(int i = 0; i < BUFFERSIZE; i++) {
        buffer[i] = '\0';
    }
    for(int i = 0; i < MAX_SIZE; i++) {
        inst[i] = "";
    }
    CC = 1;
    stallNum = 0;
    first = true;
    stall = false;
    instNum = 0;
    PC = 0;
    IFID.noInst = false;
}