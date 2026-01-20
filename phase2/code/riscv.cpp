#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include <cstdint>
#include <iomanip> // 用于 hex 输出格式

using namespace std;

#define MemSize 1000 

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bitset<32>  PC; // 修正：用于存储当前指令的PC
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class InsMem
{
    public:
        string id, ioDir;
        InsMem(string name, string ioDir) {       
            id = name;
            IMem.resize(MemSize);
            ifstream imem;
            string line;
            int i=0;
            imem.open(ioDir + "/imem.txt");
            if (imem.is_open())
            {
                while (getline(imem,line))
                {      
                    IMem[i] = bitset<8>(line);
                    i++;
                }                    
            }
            else cout<<"Unable to open IMEM input file."<<endl;
            imem.close();                     
        }

        bitset<32> readInstr(bitset<32> ReadAddress) {    
            uint32_t addr = ReadAddress.to_ulong();
            uint32_t val = 0;
            for (int i = 0; i < 4; i++) {
                if(addr + i < MemSize)
                    val |= (IMem[addr + i].to_ulong() << (8 * (3-i))); 
            }
            return bitset<32>(val);
        }     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public: 
        string id, opFilePath, ioDir;
        DataMem(string name, string ioDir) : id{name}, ioDir{ioDir} {
            DMem.resize(MemSize);
            opFilePath = ioDir + "/" + name + "_DMEMResult.txt";
            ifstream dmem;
            string line;
            int i=0;
            dmem.open(ioDir + "/dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open DMEM input file.";
                dmem.close();          
        }
        
        bitset<32> readDataMem(bitset<32> Address) {    
            uint32_t addr = Address.to_ulong();
            uint32_t val = 0;
            for (int i = 0; i < 4; i++) {
                if(addr + i < MemSize)
                    val |= (DMem[addr + i].to_ulong() << (8 * (3 - i))); 
            }
            return bitset<32>(val);
        }
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
            uint32_t addr = Address.to_ulong();
            uint32_t val  = WriteData.to_ulong();
            for (int i = 0; i < 4; i++) {
                if(addr + i < MemSize)
                    DMem[addr + i] = bitset<8>((val >> (8 * (3 - i))) & 0xFF); 
            }
        }   
        
        void outputDataMem() {
            ofstream dmemout;
            dmemout.open(opFilePath, std::ios_base::trunc);
            if (dmemout.is_open()) {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open "<<id<<" DMEM result file." << endl;
            dmemout.close();
        }             

    private:
        vector<bitset<8> > DMem;      
};

class RegisterFile
{
    public:
        string outputFile;
        RegisterFile(string ioDir): outputFile {ioDir + "RFResult.txt"} {
            Registers.resize(32);  
            Registers[0] = bitset<32> (0);  
        }
    
        bitset<32> readRF(bitset<5> Reg_addr) {   
            int idx = Reg_addr.to_ulong();
            return Registers[idx];
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
            int idx = Reg_addr.to_ulong();
            if(idx != 0)
                Registers[idx] = Wrt_reg_data;
        }
         
        void outputRF(int cycle) {
            ofstream rfout;
            if (cycle == 0)
                rfout.open(outputFile, std::ios_base::trunc);
            else 
                rfout.open(outputFile, std::ios_base::app);
            if (rfout.is_open())
            {
                rfout<<"State of RF after executing cycle:\t"<<cycle<<endl;
                for (int j = 0; j<32; j++)
                {
                    rfout << Registers[j]<<endl;
                }
            }
            rfout.close();               
        } 

        void printNonZero() {
            cout << "  [Register State (Non-Zero)]" << endl;
            for(int i=0; i<32; i++) {
                if(Registers[i].to_ulong() != 0) {
                    cout << "   R" << dec << i << " = " << Registers[i].to_ulong() << " (0x" << hex << Registers[i].to_ulong() << ")" << dec << endl;
                }
            }
        }
            
    private:
        vector<bitset<32> >Registers;
};

class Core {
    public:
        RegisterFile myRF;
        uint32_t cycle = 0;
        bool halted = false;
        string ioDir;
        struct stateStruct state, nextState;
        InsMem &ext_imem;
        DataMem &ext_dmem;
        
         Core(string ioDir, InsMem &imem, DataMem &dmem)
        : myRF(ioDir), ioDir{ioDir}, ext_imem(imem), ext_dmem(dmem) {}

        virtual void step() {}
        virtual void printState() {}
};

class FiveStageCore : public Core{
    public:
        uint32_t instr_count = 0;
        FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "/FS_", imem, dmem), opFilePath(ioDir + "/StateResult_FS.txt") {
            state.IF.nop = false;
            state.ID.nop = true;
            state.EX.nop = true;
            state.MEM.nop = true;
            state.WB.nop = true;
            instr_count = 0;
        }

        string disassemble(bitset<32> instr) {
            string s = instr.to_string();
            if (instr.to_ulong() == 0xFFFFFFFF) return "HALT";
            if (instr.to_ulong() == 0) return "NOP";

            uint32_t opcode = bitset<7>(s.substr(25, 7)).to_ulong();
            uint32_t rd     = bitset<5>(s.substr(20, 5)).to_ulong();
            uint32_t funct3 = bitset<3>(s.substr(17, 3)).to_ulong();
            uint32_t rs1    = bitset<5>(s.substr(12, 5)).to_ulong();
            uint32_t rs2    = bitset<5>(s.substr(7, 5)).to_ulong();
            uint32_t funct7 = bitset<7>(s.substr(0, 7)).to_ulong();

            string rs1Str = "R" + to_string(rs1);
            string rs2Str = "R" + to_string(rs2);
            string rdStr  = "R" + to_string(rd);

            switch(opcode) {
                case 0x33: // R-Type
                    if(funct3 == 0x0) {
                        if(funct7 == 0x00) return "ADD " + rdStr + ", " + rs1Str + ", " + rs2Str;
                        else return "SUB " + rdStr + ", " + rs1Str + ", " + rs2Str;
                    } else if (funct3 == 0x7) return "AND " + rdStr + ", " + rs1Str + ", " + rs2Str;
                    else if (funct3 == 0x6) return "OR  " + rdStr + ", " + rs1Str + ", " + rs2Str;
                    else if (funct3 == 0x4) return "XOR " + rdStr + ", " + rs1Str + ", " + rs2Str;
                    break;
                case 0x13: // I-Type (ADDI etc)
                {
                    int16_t imm = (int16_t)bitset<12>(s.substr(0, 12)).to_ulong();
                    if(funct3 == 0x0) return "ADDI " + rdStr + ", " + rs1Str + ", " + to_string(imm);
                    else if(funct3 == 0x7) return "ANDI " + rdStr + ", " + rs1Str + ", " + to_string(imm);
                    else if(funct3 == 0x6) return "ORI  " + rdStr + ", " + rs1Str + ", " + to_string(imm);
                    else if(funct3 == 0x4) return "XORI " + rdStr + ", " + rs1Str + ", " + to_string(imm);
                    break;
                }
                case 0x03: // LW
                {
                    int16_t imm = (int16_t)bitset<12>(s.substr(0, 12)).to_ulong();
                    return "LW " + rdStr + ", " + to_string(imm) + "(" + rs1Str + ")";
                }
                case 0x23: // SW
                {
                    string immStr = s.substr(0, 7) + s.substr(20, 5);
                    int16_t imm = (int16_t)bitset<12>(immStr).to_ulong();
                    return "SW " + rs2Str + ", " + to_string(imm) + "(" + rs1Str + ")";
                }
                case 0x63: // BEQ
                {
                    string immStr = s.substr(0, 1) + s.substr(24, 1) + s.substr(1, 6) + s.substr(20, 4);
                    int16_t imm = (int16_t)bitset<12>(immStr).to_ulong();
                    if(funct3 == 0x0) return "BEQ " + rs1Str + ", " + rs2Str + ", offset=" + to_string(imm*2);
                    else return "BNE " + rs1Str + ", " + rs2Str + ", offset=" + to_string(imm*2);
                }
                case 0x6F: // JAL
                    return "JAL " + rdStr + ", offset"; 
            }
            return "UNKNOWN/NOP";
        }

        void step() {
            cout << "\n================ Cycle " << cycle << " Start ================" << endl;

            /* --------------------- WB stage (Write Back) --------------------- */
            cout << ">> [WB Stage]: ";
            if (state.WB.nop) {
                cout << "NOP (Bubble)" << endl;
            } else {
                instr_count++;
                if (state.WB.wrt_enable) {
                    cout << "WriteBack -> R" << state.WB.Wrt_reg_addr.to_ulong() 
                         << " = " << state.WB.Wrt_data.to_ulong() << " (0x" << hex << state.WB.Wrt_data.to_ulong() << ")" << dec << endl;
                    myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
                } else {
                    cout << "Instruction finished (No Register Write)" << endl;
                }
            }
            
            /* --------------------- MEM stage (Memory Access) -------------------- */
            cout << ">> [MEM Stage]: ";
            nextState.WB.nop = state.MEM.nop;
            
            if (state.MEM.nop) {
                cout << "NOP (Bubble)" << endl;
            } else {
                nextState.WB.Rs = state.MEM.Rs;
                nextState.WB.Rt = state.MEM.Rt;
                nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
                nextState.WB.wrt_enable = state.MEM.wrt_enable;
                
                if (state.MEM.rd_mem) {
                    uint32_t addr = state.MEM.ALUresult.to_ulong();
                    nextState.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
                    cout << "LOAD MEM[" << addr << "] -> Read Data: " << nextState.WB.Wrt_data.to_ulong() << endl;
                } else if (state.MEM.wrt_mem) {
                    uint32_t addr = state.MEM.ALUresult.to_ulong();
                    uint32_t val  = state.MEM.Store_data.to_ulong();
                    ext_dmem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                    nextState.WB.Wrt_data = state.MEM.ALUresult; 
                    cout << "STORE MEM[" << addr << "] <- Value: " << val << endl;
                } else {
                    nextState.WB.Wrt_data = state.MEM.ALUresult;
                    cout << "Passthrough ALU Result: " << state.MEM.ALUresult.to_ulong() << endl;
                }
            }
            if(state.MEM.nop) nextState.WB.wrt_enable = false;

            /* --------------------- EX stage (Execute) --------------------- */
            cout << ">> [EX Stage]: ";
            nextState.MEM.nop = state.EX.nop;
            
            bool branch_taken = false; 

            if (state.EX.nop) {
                cout << "NOP (Bubble)" << endl;
            } else {
                nextState.MEM.Rs = state.EX.Rs;
                nextState.MEM.Rt = state.EX.Rt;
                nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
                nextState.MEM.rd_mem = state.EX.rd_mem;
                nextState.MEM.wrt_mem = state.EX.wrt_mem;
                nextState.MEM.wrt_enable = state.EX.wrt_enable;

                // --- Forwarding Unit ---
                uint32_t operand1 = state.EX.Read_data1.to_ulong();
                uint32_t operand2 = state.EX.Read_data2.to_ulong();
                string fwdMsg = "";

                if (state.MEM.wrt_enable && state.MEM.Wrt_reg_addr.to_ulong() != 0 && state.MEM.Wrt_reg_addr == state.EX.Rs) {
                    operand1 = state.MEM.ALUresult.to_ulong(); 
                    fwdMsg += " [Fwd-A from MEM]";
                } else if (state.WB.wrt_enable && state.WB.Wrt_reg_addr.to_ulong() != 0 && state.WB.Wrt_reg_addr == state.EX.Rs) {
                    operand1 = state.WB.Wrt_data.to_ulong();
                    fwdMsg += " [Fwd-A from WB]";
                }

                uint32_t forwardB_val = operand2; 
                if (state.MEM.wrt_enable && state.MEM.Wrt_reg_addr.to_ulong() != 0 && state.MEM.Wrt_reg_addr == state.EX.Rt) {
                    forwardB_val = state.MEM.ALUresult.to_ulong();
                    fwdMsg += " [Fwd-B from MEM]";
                } else if (state.WB.wrt_enable && state.WB.Wrt_reg_addr.to_ulong() != 0 && state.WB.Wrt_reg_addr == state.EX.Rt) {
                    forwardB_val = state.WB.Wrt_data.to_ulong();
                    fwdMsg += " [Fwd-B from WB]";
                }
                
                if (state.EX.is_I_type && !state.EX.wrt_mem && !state.EX.rd_mem) {
                     operand2 = (int32_t)((int16_t)state.EX.Imm.to_ulong()); 
                } else if (state.EX.rd_mem || state.EX.wrt_mem) {
                     operand2 = (int32_t)((int16_t)state.EX.Imm.to_ulong()); 
                } else {
                     operand2 = forwardB_val;
                }
                
                nextState.MEM.Store_data = bitset<32>(forwardB_val);
                uint32_t result = 0;
                
                if (state.EX.alu_op) {
                     result = operand1 + operand2; 
                     cout << "ALU ADD: " << operand1 << " + " << operand2 << " = " << result << fwdMsg << endl;
                } else {
                     result = operand1 - operand2; 
                     cout << "ALU SUB: " << operand1 << " - " << operand2 << " = " << result << fwdMsg << endl;
                }

                nextState.MEM.ALUresult = bitset<32>(result);
            }

            /* --------------------- ID stage (Decode) --------------------- */
            cout << ">> [ID Stage]: ";
            nextState.EX.nop = state.ID.nop;
            
            bool stall = false; 
            
            if (state.ID.nop) {
                cout << "NOP (Bubble)" << endl;
            } else {
                bitset<32> instr = state.ID.Instr;
                string disasm = disassemble(instr);
                cout << "Decoding: " << disasm << endl;

                string instrStr = instr.to_string();
                uint32_t opcode = bitset<7>(instrStr.substr(25, 7)).to_ulong();
                uint32_t rd     = bitset<5>(instrStr.substr(20, 5)).to_ulong();
                uint32_t funct3 = bitset<3>(instrStr.substr(17, 3)).to_ulong();
                uint32_t rs1    = bitset<5>(instrStr.substr(12, 5)).to_ulong();
                uint32_t rs2    = bitset<5>(instrStr.substr(7, 5)).to_ulong();
                uint32_t funct7 = bitset<7>(instrStr.substr(0, 7)).to_ulong();

                // Load-Use Hazard Detection
                if (state.EX.rd_mem && !state.EX.nop) {
                    if (state.EX.Wrt_reg_addr.to_ulong() == rs1 || state.EX.Wrt_reg_addr.to_ulong() == rs2) {
                        stall = true;
                        cout << "  [!] Load-Use Hazard Detected! Stalling..." << endl;
                    }
                }

                if (stall) {
                    nextState.EX.nop = true;
                    nextState.ID = state.ID;
                    nextState.IF = state.IF;
                } 
                else {
                    nextState.EX.Rs = rs1;
                    nextState.EX.Rt = rs2;
                    nextState.EX.Wrt_reg_addr = rd;
                    nextState.EX.Read_data1 = myRF.readRF(rs1);
                    nextState.EX.Read_data2 = myRF.readRF(rs2);
                    
                    nextState.EX.wrt_enable = false;
                    nextState.EX.rd_mem = false;
                    nextState.EX.wrt_mem = false;
                    nextState.EX.alu_op = true; 
                    nextState.EX.is_I_type = false;

                    switch (opcode) {
                        case 0x33: // R-Type
                            nextState.EX.wrt_enable = true;
                            nextState.EX.alu_op = (funct7 == 0x20) ? 0 : 1;
                            break;
                        case 0x13: // ADDI
                            nextState.EX.wrt_enable = true;
                            nextState.EX.is_I_type = true;
                            nextState.EX.Imm = bitset<16>(instrStr.substr(0, 12));
                            break;
                        case 0x03: // LW
                            nextState.EX.wrt_enable = true;
                            nextState.EX.rd_mem = true;
                            nextState.EX.is_I_type = true;
                            nextState.EX.Imm = bitset<16>(instrStr.substr(0, 12));
                            break;
                        case 0x23: // SW
                            nextState.EX.wrt_mem = true;
                            nextState.EX.is_I_type = false; 
                            {
                                string immS = instrStr.substr(0, 7) + instrStr.substr(20, 5);
                                nextState.EX.Imm = bitset<16>(immS);
                            }
                            break;
                            case 0x63: // BEQ / BNE (Branch Hazard Optimized with EX-Forwarding)
{
    // 1. 读取寄存器值 (默认从 RF)
    uint32_t val1 = myRF.readRF(rs1).to_ulong();
    uint32_t val2 = myRF.readRF(rs2).to_ulong();

    // 2. 增强的前递逻辑 (Priority: EX > MEM > WB)
    // 关键优化：利用 nextState.MEM 来获取当前 EX 阶段刚刚计算出的结果

    // --- Rs1 前递 ---
    if (rs1 != 0) {
        // [New!] 检查 EX 阶段 (针对算术指令进行前递)
        // 注意：必须排除 Load 指令 (state.EX.rd_mem)，因为 Load 结果还未产生
        if (state.EX.wrt_enable && !state.EX.nop && !state.EX.rd_mem && state.EX.Wrt_reg_addr.to_ulong() == rs1) {
            val1 = nextState.MEM.ALUresult.to_ulong(); 
            // cout << " [Fwd-ID from EX]"; // Debug info
        } 
        // 检查 MEM 阶段
        else if (state.MEM.wrt_enable && !state.MEM.nop && state.MEM.Wrt_reg_addr.to_ulong() == rs1) {
            val1 = state.MEM.ALUresult.to_ulong(); 
        } 
        // 检查 WB 阶段
        else if (state.WB.wrt_enable && !state.WB.nop && state.WB.Wrt_reg_addr.to_ulong() == rs1) {
            val1 = state.WB.Wrt_data.to_ulong();
        }
    }

    // --- Rs2 前递 ---
    if (rs2 != 0) {
        // [New!] 检查 EX 阶段
        if (state.EX.wrt_enable && !state.EX.nop && !state.EX.rd_mem && state.EX.Wrt_reg_addr.to_ulong() == rs2) {
            val2 = nextState.MEM.ALUresult.to_ulong();
        } 
        else if (state.MEM.wrt_enable && !state.MEM.nop && state.MEM.Wrt_reg_addr.to_ulong() == rs2) {
            val2 = state.MEM.ALUresult.to_ulong();
        } 
        else if (state.WB.wrt_enable && !state.WB.nop && state.WB.Wrt_reg_addr.to_ulong() == rs2) {
            val2 = state.WB.Wrt_data.to_ulong();
        }
    }
    
    // 3. 检查剩余的 Stall (仅针对 Load-Use Hazard)
    bool hazard = false;
    
    // 3a. EX 阶段依赖：现在只针对 Load 指令 Stall
    // 如果上一条指令是 LW，结果还没读出来，必须等。如果是 ADD，上面已经前递了，不用等。
    if (state.EX.rd_mem && !state.EX.nop && (state.EX.Wrt_reg_addr.to_ulong() == rs1 || state.EX.Wrt_reg_addr.to_ulong() == rs2)) {
        hazard = true;
    }

    // 3b. Load 指令在 MEM 阶段的依赖 (Load结果在MEM末端才可用，可能需 Stall)
    // 你的旧逻辑这里也是 Stall，保留原样以防 Load 延迟
    if (state.MEM.rd_mem && !state.MEM.nop && (state.MEM.Wrt_reg_addr.to_ulong() == rs1 || state.MEM.Wrt_reg_addr.to_ulong() == rs2)) {
            hazard = true;
    }

    if (hazard) {
        stall = true;
        nextState.EX.nop = true;
        nextState.ID = state.ID;
        nextState.IF = state.IF;
        cout << "  [!] Load Hazard Detected! Stalling..." << endl;
    } else {
        // 4. 计算分支目标
        string immB = instrStr.substr(0, 1) + instrStr.substr(24, 1) + instrStr.substr(1, 6) + instrStr.substr(20, 4);
        int32_t imm = 0;
        int32_t raw_imm = std::stoi(immB, nullptr, 2);
        if (immB[0] == '1') imm = raw_imm - (1<<12);
        else imm = raw_imm;
        imm = imm << 1; 

        bool take = (funct3 == 0) ? (val1 == val2) : (val1 != val2);
        
        if (take) {
            cout << "  [!] Branch TAKEN. Updating PC to " << (state.ID.PC.to_ulong() + imm) << endl;
            nextState.IF.PC = state.ID.PC.to_ulong() + imm; 
            nextState.ID.nop = true; 
            branch_taken = true;
        } else {
            cout << "  Branch NOT taken." << endl;
        }
        
        nextState.EX.nop = false; 
        nextState.EX.wrt_enable = false;
        nextState.EX.wrt_mem = false;
        nextState.EX.rd_mem = false;
    }
}
break;
                        // case 0x63: // BEQ / BNE (Branch Hazard Optimization)
                        //     {
                        //         // 1. 读取寄存器值 (默认从 RF)
                        //         uint32_t val1 = myRF.readRF(rs1).to_ulong();
                        //         uint32_t val2 = myRF.readRF(rs2).to_ulong();

                        //         // 2. 检查并应用 ID 阶段前递 (Forwarding to ID)
                        //         // 优先级：MEM (EX/MEM) > WB (MEM/WB)
                                
                        //         // --- Rs1 前递 ---
                        //         if (rs1 != 0) {
                        //             if (state.MEM.wrt_enable && !state.MEM.nop && state.MEM.Wrt_reg_addr.to_ulong() == rs1) {
                        //                 val1 = state.MEM.ALUresult.to_ulong(); 
                        //             } else if (state.WB.wrt_enable && !state.WB.nop && state.WB.Wrt_reg_addr.to_ulong() == rs1) {
                        //                 val1 = state.WB.Wrt_data.to_ulong();
                        //             }
                        //         }

                        //         // --- Rs2 前递 ---
                        //         if (rs2 != 0) {
                        //             if (state.MEM.wrt_enable && !state.MEM.nop && state.MEM.Wrt_reg_addr.to_ulong() == rs2) {
                        //                 val2 = state.MEM.ALUresult.to_ulong();
                        //             } else if (state.WB.wrt_enable && !state.WB.nop && state.WB.Wrt_reg_addr.to_ulong() == rs2) {
                        //                 val2 = state.WB.Wrt_data.to_ulong();
                        //             }
                        //         }
                                
                        //         // 3. 检查剩余的 Stall (数据仍在 EX 阶段或 Load 在 MEM 阶段)
                        //         bool hazard = false;
                                
                        //         // 3a. EX 阶段依赖 (必须 Stall 1 周期)
                        //         if (state.EX.wrt_enable && !state.EX.nop && (state.EX.Wrt_reg_addr.to_ulong() == rs1 || state.EX.Wrt_reg_addr.to_ulong() == rs2)) {
                        //             hazard = true;
                        //         }

                        //         // 3b. Load 指令在 MEM 阶段的依赖 (Load结果在MEM末端才可用，需Stall 1 周期)
                        //         if (state.MEM.rd_mem && !state.MEM.nop && (state.MEM.Wrt_reg_addr.to_ulong() == rs1 || state.MEM.Wrt_reg_addr.to_ulong() == rs2)) {
                        //              hazard = true;
                        //         }

                        //         if (hazard) {
                        //             stall = true;
                        //             nextState.EX.nop = true;
                        //             nextState.ID = state.ID;
                        //             nextState.IF = state.IF;
                        //             cout << "  [!] Branch/Load Hazard Detected (Need 1 Stall)! Stalling..." << endl;
                        //         } else {
                        //             // 4. 计算分支目标 (使用前递后的 val1, val2)
                                    
                        //             string immB = instrStr.substr(0, 1) + instrStr.substr(24, 1) + instrStr.substr(1, 6) + instrStr.substr(20, 4);
                        //             int32_t imm = 0;
                        //             // Fix: std::stoi needs a string compatible with its base, or use bitset<12> correctly.
                        //             // Since you're using C++11 std::stoi, and the bit string is 12 chars long (plus 1 for padding):
                        //             int32_t raw_imm = std::stoi(immB, nullptr, 2);
                        //             if (immB[0] == '1') imm = raw_imm - (1<<12); // Sign extension
                        //             else imm = raw_imm;
                        //             imm = imm << 1; 

                        //             bool take = (funct3 == 0) ? (val1 == val2) : (val1 != val2);
                                    
                        //             if (take) {
                        //                 cout << "  [!] Branch TAKEN (Forwarded Data Used). Updating PC to " << (state.ID.PC.to_ulong() + imm) << endl;
                        //                 nextState.IF.PC = state.ID.PC.to_ulong() + imm; 
                        //                 nextState.ID.nop = true; 
                        //                 branch_taken = true;
                        //             } else {
                        //                 cout << "  Branch NOT taken (Forwarded Data Used)." << endl;
                        //             }
                                    
                        //             // 分支指令流向 EX (用于计数)
                        //             nextState.EX.nop = false; 
                        //             nextState.EX.wrt_enable = false;
                        //             nextState.EX.wrt_mem = false;
                        //             nextState.EX.rd_mem = false;
                        //         }
                        //     }
                        //     break;
                        case 0x6F: // JAL
                             {
                                string immJ = instrStr.substr(0, 1) + instrStr.substr(12, 8) + instrStr.substr(11, 1) + instrStr.substr(1, 10);
                                int32_t imm = 0;
                                int32_t raw = std::stoi(immJ, nullptr, 2);
                                if (immJ[0] == '1') imm = raw - (1<<20);
                                else imm = raw;
                                imm = imm << 1;

                                cout << "  [!] JUMP (JAL). Updating PC." << endl;
                                nextState.IF.PC = state.ID.PC.to_ulong() + imm;
                                
                                nextState.EX.wrt_enable = true;
                                nextState.EX.Wrt_reg_addr = rd;
                                uint32_t pc_plus_4 = state.ID.PC.to_ulong() + 4;
                                nextState.EX.Read_data1 = bitset<32>(pc_plus_4);
                                nextState.EX.Read_data2 = bitset<32>(0);
                                nextState.EX.Imm = bitset<16>(0);
                                nextState.EX.alu_op = true; // ADD
                                nextState.EX.rd_mem = false;
                                nextState.EX.wrt_mem = false;
                                
                                // 修正：让 JAL 指令流下去
                                nextState.EX.nop = false;

                                branch_taken = true;
                                nextState.ID.nop = true; 
                             }
                            break;
                    }
                }
            }

            /* --------------------- IF stage --------------------- */
            cout << ">> [IF Stage]: ";
            if (!stall && !branch_taken) {
                if (!state.IF.nop) {
                    bitset<32> instr = ext_imem.readInstr(state.IF.PC);
                    cout << "Fetching Instr at PC: " << state.IF.PC.to_ulong();
                    if (instr.to_ulong() == 0xFFFFFFFF) {
                        cout << " -> HALT DETECTED." << endl;
                        state.IF.nop = true; 
                        nextState.IF.nop = true;
                        nextState.ID.nop = true; 
                    } else {
                        cout << " -> Raw: " << hex << instr.to_ulong() << dec << endl;
                        nextState.ID.Instr = instr;
                        nextState.ID.PC = state.IF.PC; 
                        nextState.ID.nop = false;
                        nextState.IF.PC = state.IF.PC.to_ulong() + 4;
                        nextState.IF.nop = false;
                    }
                } else {
                     cout << "NOP (Stop Fetching)" << endl;
                     nextState.ID.nop = true;
                }
            } else if (stall) {
                cout << "Stalled (Waiting for Hazard Resolution)" << endl;
            } else if (branch_taken) {
                cout << "Flush due to Branch (Next PC will be " << nextState.IF.PC.to_ulong() << ")" << endl;
            }
            
            if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
                halted = true;
        
            myRF.outputRF(cycle); 
            myRF.printNonZero(); 

            printState(nextState, cycle); 
       
            state = nextState; 
            cycle++;
        }

        void printState(stateStruct state, int cycle) {
            ofstream printstate;
            if (cycle == 0)
                printstate.open(opFilePath, std::ios_base::trunc);
            else 
                printstate.open(opFilePath, std::ios_base::app);
            if (printstate.is_open()) {
                printstate<<"State after executing cycle:\t"<<cycle<<endl; 
                printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
                printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
                printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
                printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
                printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
                printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
                printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
                printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
                printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
                printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
                printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
                printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
                printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
                printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
                printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
                printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        
                printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
                printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
                printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
                printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
                printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
                printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
                printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
                printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
                printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        
                printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
                printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
                printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
                printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
                printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
                printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
            }
            else cout<<"Unable to open FS StateResult output file." << endl;
            printstate.close();
        }
    private:
        string opFilePath;
};

int main(int argc, char* argv[]) {
    
    string ioDir = "";
    if (argc == 1) {
        cout << "Enter path containing the memory files: ";
        cin >> ioDir;
    }
    else if (argc > 2) {
        cout << "Invalid number of arguments. Machine stopped." << endl;
        return -1;
    }
    else {
        ioDir = argv[1];
        cout << "IO Directory: " << ioDir << endl;
    }

    InsMem imem = InsMem("Imem", ioDir);
    DataMem dmem_fs = DataMem("FS", ioDir);
    
    FiveStageCore FSCore(ioDir, imem, dmem_fs);

    while (1) {
        if (!FSCore.halted)
            FSCore.step();

        if (FSCore.halted)
            break;
    }
    
    dmem_fs.outputDataMem();

    // Performance Measurement
    ofstream perfOut(ioDir + "/PerformanceResult.txt", ios::trunc);
    if (!perfOut.is_open()) {
        cout << "Unable to open PerformanceResult.txt" << endl;
        return -1;
    }
    
    double fs_cycles = FSCore.cycle;
    double fs_instrs = FSCore.instr_count; 
    double fs_cpi = 0.0;
    double fs_ipc = 0.0;

    if (fs_instrs > 0) {
        fs_cpi = fs_cycles / fs_instrs;
        fs_ipc = fs_instrs / fs_cycles;
    }

    cout << endl << "================ Performance Summary ================" << endl;
    cout << "Performance of Five Stage:" << endl;
    cout << "#Cycles       -> " << fs_cycles << endl;
    cout << "#Instructions -> " << fs_instrs << endl;
    cout << "CPI           -> " << fs_cpi << endl;
    cout << "IPC           -> " << fs_ipc << endl;
    cout << "=====================================================" << endl;

    perfOut << "Performance of Five Stage:" << endl;
    perfOut << "#Cycles       -> " << fs_cycles << endl;
    perfOut << "#Instructions -> " << fs_instrs << endl;
    perfOut << "CPI           -> " << fs_cpi << endl;
    perfOut << "IPC           -> " << fs_ipc << endl;

    perfOut.close();

    return 0;
}