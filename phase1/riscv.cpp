#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include <cstdint>

using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
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
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
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
			// read instruction memory
			// return bitset<32> val
         uint32_t addr = ReadAddress.to_ulong();
    uint32_t val = 0;

    cout << "ReadAddress: " << ReadAddress.to_ulong() << endl;
    for (int i = 0; i < 4; i++) {
        uint32_t byteVal = IMem[addr + i].to_ulong();
        cout << "Byte " << i << ": " << IMem[addr + i] << " (" << byteVal << ")" << endl;
        val |= (byteVal << (8 * (3-i))); // 大端存储
    }

    cout << "Combined 32-bit instruction: " << bitset<32>(val) << " (" << val << ")" << endl;
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
			// read data memory
			// return bitset<32> val
           // cout << "Reading DMEM at address " << addr << endl;

             uint32_t addr = Address.to_ulong();
        uint32_t val = 0;
        cout << "ReadAddress: " << addr << endl;
        for (int i = 0; i < 4; i++) {
            uint32_t byteVal = DMem[addr + i].to_ulong();
            cout << "Byte " << i << ": " << DMem[addr + i] << " (" << byteVal << ")" << endl;
            val |= (byteVal << (8 * (3 - i))); // big endian
        }
        cout << "Combined 32-bit data: " << bitset<32>(val) << " (" << val << ")" << endl;
        return bitset<32>(val);
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
			// write into memory
             uint32_t addr = Address.to_ulong();
    uint32_t val  = WriteData.to_ulong();

    cout << "WriteAddress: " << addr << ", WriteData: " 
         << bitset<32>(val) << " (" << val << ")" << endl;

    for (int i = 0; i < 4; i++) {
        DMem[addr + i] = bitset<8>((val >> (8 * (3 - i))) & 0xFF); // 大端
        cout << "  Byte " << i << " written: " << DMem[addr + i] 
             << " (" << DMem[addr + i].to_ulong() << ")" << endl;
    }

    cout << "Memory after write (4 bytes at " << addr << "): ";
    for (int i = 0; i < 4; i++)
        cout << bitset<8>(DMem[addr + i]) << " ";
    cout << endl << endl;
        }   
        void printAllDataMem() {
    cout << "===== Full DMEM Dump =====" << endl;
    for (size_t addr = 0; addr < DMem.size(); addr += 4) {
        // 每 4 个字节组成一个 32 位大端值
        uint32_t val = (DMem[addr].to_ulong() << 24) |
                       (DMem[addr + 1].to_ulong() << 16) |
                       (DMem[addr + 2].to_ulong() << 8) |
                       (DMem[addr + 3].to_ulong());
        cout << "0x" << hex << addr << ": " 
             << bitset<32>(val) << " (" << dec << val << ")" << endl;
    }
    cout << "===========================" << endl;
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
            // Fill in
            int idx = Reg_addr.to_ulong();
            return Registers[idx];
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
            // Fill in
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
                    cout << Registers[j] << endl; // <<<<<<<<<<<<<<
				}
			}
			else cout<<"Unable to open RF output file."<<endl;
			rfout.close();               
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

class SingleStageCore : public Core {
	public:
		SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "/SS_", imem, dmem), opFilePath(ioDir + "/StateResult_SS.txt") {}

		void step() {
			/* Your implementation*/
                if (halted) return;

    cout << "==================== Cycle " << cycle << " ====================" << endl;

    // 1️⃣ 取指阶段 IF
    bitset<32> instruction = ext_imem.readInstr(state.IF.PC);
    cout << "Instruction: " << instruction << endl;

    // 如果指令全1表示halt
    if (instruction.to_ulong() == 0xFFFFFFFF) {
        halted = true;
        return;
    }

    // 2️⃣ 译码阶段 ID
    string instrStr = instruction.to_string();
    bitset<7> opcode(instrStr.substr(25, 7));
    bitset<5> rd(instrStr.substr(20, 5));
    bitset<3> funct3(instrStr.substr(17, 3));
    bitset<5> rs1(instrStr.substr(12, 5));
    bitset<5> rs2(instrStr.substr(7, 5));
    bitset<7> funct7(instrStr.substr(0, 7));

    uint32_t pc_next = state.IF.PC.to_ulong() + 4;
    uint32_t val1 = myRF.readRF(rs1.to_ulong()).to_ulong();
    uint32_t val2 = myRF.readRF(rs2.to_ulong()).to_ulong();

    uint32_t alu_result = 0;
    uint32_t mem_data = 0;
    uint32_t imm = 0;
    bool pc_changed = false;

    // 3️⃣  EXECUTE ACCESS MEMORY AND WRITE BACK
    switch (opcode.to_ulong()) {

        case 0x33: { // R-type
             switch (funct3.to_ulong()) {
        case 0x0: // ADD / SUB
            if (funct7.to_ulong() == 0x00) alu_result = val1 + val2; // ADD
            else if (funct7.to_ulong() == 0x20) alu_result = val1 - val2; // SUB
            break;
        case 0x7: alu_result = val1 & val2; break; // AND
        case 0x6: alu_result = val1 | val2; break; // OR
        case 0x4: alu_result = val1 ^ val2; break; // XOR
        default:
            cout << "Unknown R-type funct3: " << funct3.to_ulong() << endl;
            halted = true;
            return;
    }

            myRF.writeRF(rd.to_ulong(), bitset<32>(alu_result));
            break;
        }

        case 0x13: { // I-type, ADDI
            int32_t imm_i = (int32_t)(instruction.to_ulong()) >> 20;
             
        switch(funct3.to_ulong()) {
            case 0x0: alu_result = val1 + imm_i; break;        // ADDI
            case 0x7: alu_result = val1 & imm_i; break;        // ANDI
            case 0x6: alu_result = val1 | imm_i; break;        // ORI
            case 0x4: alu_result = val1 ^ imm_i; break;        // XORI
        }
            myRF.writeRF(rd.to_ulong(), bitset<32>(alu_result));
            break;
        }

        case 0x03: { // I-type load (LW)
            int32_t imm_i = (int32_t)(instruction.to_ulong()) >> 20;
            alu_result = val1 + imm_i; // address
            mem_data = ext_dmem.readDataMem(bitset<32>(alu_result)).to_ulong();
            myRF.writeRF(rd.to_ulong(), bitset<32>(mem_data));
            break;
        }

        case 0x23: { // S-type store (SW)
            // imm = [31:25][11:7]
            uint32_t imm_s = ((instruction.to_ulong() >> 25) << 5) | ((instruction.to_ulong() >> 7) & 0x1F);
            int32_t simm = (imm_s & 0x800) ? (imm_s | 0xFFFFF000) : imm_s;
            alu_result = val1 + simm;
            ext_dmem.writeDataMem(bitset<32>(alu_result), myRF.readRF(rs2.to_ulong()));
            break;
        }

        case 0x63: { // B-type branch (BEQ)
    uint32_t imm_b =
        ((instruction[31] ? 1 : 0) << 12) |    // bit 31 -> imm[12]
        ((instruction[7]  ? 1 : 0) << 11) |    // bit 7  -> imm[11]
        ((instruction.to_ulong() >> 25 & 0x3F) << 5) |  // bits [30:25] -> imm[10:5]
        ((instruction.to_ulong() >> 8  & 0xF)  << 1);   // bits [11:8]  -> imm[4:1]

    int32_t simm = (imm_b & 0x1000) ? (imm_b | 0xFFFFE000) : imm_b;  // sign-extend

    if (funct3.to_ulong() == 0) { // BEQ
            if (val1 == val2) { 
                nextState.IF.PC = state.IF.PC.to_ulong() + simm; 
                pc_changed = true;
            }
        } else if (funct3.to_ulong() == 1) { // BNE
            if (val1 != val2) { 
                nextState.IF.PC = state.IF.PC.to_ulong() + simm; 
                pc_changed = true;
            }
        }
    break;
}


case 0x6F: { // JAL
        int32_t imm_j =
            ((instruction[31] ? 1 : 0) << 20) |
            ((instruction.to_ulong() >> 12 & 0xFF) << 12) |
            ((instruction[20] ? 1 : 0) << 11) |
            ((instruction.to_ulong() >> 21 & 0x3FF) << 1);
        int32_t simm = (imm_j & 0x100000) ? (imm_j | 0xFFE00000) : imm_j;

        myRF.writeRF(rd.to_ulong(), bitset<32>(state.IF.PC.to_ulong() + 4)); // store PC+4
        nextState.IF.PC = state.IF.PC.to_ulong() + simm;
        pc_changed = true;
        break;
    }

        default:
            cout << "Unknown opcode: " << opcode.to_ulong() << endl;
            halted = true;
            break;
    }

    // 4️⃣ UPDATE PC
    if (!pc_changed)
        nextState.IF.PC = pc_next;

    // 5️⃣ PRINT THE REGISTER
    myRF.outputRF(cycle);
    printState(nextState, cycle);
    //ext_dmem.printAllDataMem();//THIS IS PRINT ALL THE MEMORY

    // 6️⃣ RENEW THE STATE
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
    		    printstate << "State after executing cycle:\t" << cycle << endl;

        // 打印 IF 状态
        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        // 打印寄存器文件
        printstate << "Registers:\n";
        for (int i = 0; i < 32; i++) {
            printstate << "R" << i << ": " << myRF.readRF(bitset<5>(i)) << endl;
        }
    		}
    		else cout<<"Unable to open SS StateResult output file." << endl;
    		printstate.close();
		}
	private:
		string opFilePath;
};

class FiveStageCore : public Core{
	public:
		
		FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir + "/FS_", imem, dmem), opFilePath(ioDir + "/StateResult_FS.txt") {}

		void step() {
			/* Your implementation */
			/* --------------------- WB stage --------------------- */
			
			
			
			/* --------------------- MEM stage -------------------- */
			
			
			
			/* --------------------- EX stage --------------------- */
			
			
			
			/* --------------------- ID stage --------------------- */
			
			
			
			/* --------------------- IF stage --------------------- */
			
			
			halted = true;
			if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
				halted = true;
        
            myRF.outputRF(cycle); // dump RF
			printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
			state = nextState; //The end of the cycle and updates the current state with the values calculated in this cycle
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
    DataMem dmem_ss = DataMem("SS", ioDir);
	DataMem dmem_fs = DataMem("FS", ioDir);
    

	SingleStageCore SSCore(ioDir, imem, dmem_ss);
	FiveStageCore FSCore(ioDir, imem, dmem_fs);

    while (1) {
		if (!SSCore.halted)
			SSCore.step();
		
		if (!FSCore.halted)
			FSCore.step();

		if (SSCore.halted && FSCore.halted)
			break;
    }
    
	// dump SS and FS data mem.
	dmem_ss.outputDataMem();
	dmem_fs.outputDataMem();
// ---------------- PERFORMANCE MEASUREMENT ----------------
    ofstream perfOut(ioDir + "/PerformanceResult.txt", ios::trunc);
    if (!perfOut.is_open()) {
        cout << "Unable to open PerformanceResult.txt" << endl;
        return -1;
    }

    double ss_cycles = SSCore.cycle+2;
  // double fs_cycles = FSCore.cycle;

    double ss_instr = ss_cycles-1;  // 举例近似
   // double fs_instr = fs_cycles - 7;  // pipeline 有填充阶段，扣掉几条空周期（视你的设计可调）

    double ss_cpi = ss_cycles / ss_instr;
    double ss_ipc = ss_instr / ss_cycles;

  //  double fs_cpi = fs_cycles / fs_instr;
   // double fs_ipc = fs_instr / fs_cycles;

    cout << endl << "================ Performance Summary ================" << endl;
    cout << "Performance of Single Stage:" << endl;
    cout << "#Cycles -> " << ss_cycles << endl;
    cout << "#Instructions -> " << ss_instr << endl;
    cout << "CPI -> " << ss_cpi << endl;
    cout << "IPC -> " << ss_ipc << endl << endl;

 //   cout << "Performance of Five Stage:" << endl;
//    cout << "#Cycles -> " << fs_cycles << endl;
  //  cout << "#Instructions -> " << fs_instr << endl;
  //  cout << "CPI -> " << fs_cpi << endl;
//    cout << "IPC -> " << fs_ipc << endl;
    cout << "=====================================================" << endl;

    perfOut << "Performance of Single Stage:" << endl;
    perfOut << "#Cycles -> " << ss_cycles << endl;
    perfOut << "#Instructions -> " << ss_instr << endl;
    perfOut << "CPI -> " << ss_cpi << endl;
    perfOut << "IPC -> " << ss_ipc << endl << endl;

    //perfOut << "Performance of Five Stage:" << endl;
   // perfOut << "#Cycles -> " << fs_cycles << endl;
   // perfOut << "#Instructions -> " << fs_instr << endl;
   // perfOut << "CPI -> " << fs_cpi << endl;
   // perfOut << "IPC -> " << fs_ipc << endl;

    perfOut.close();
    // ----------------------------------------------------------

    
	return 0;
}