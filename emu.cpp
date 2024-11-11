/*Name - Rishi kumar 
   Roll number -2301cs83*/

#include<bits/stdc++.h>

class Emulator {
private:
    struct MemoryAccess {
        uint32_t address;
        int32_t value;
        int32_t previousValue;  // for writes only
    };

    struct CPUState {
        CPUState() : pc(0), sp(0), a(0), b(0) {}
        int32_t pc;
        int32_t sp;
        int32_t a;
        int32_t b;
    };

    CPUState state;
    std::vector<int32_t> memory;
    std::vector<MemoryAccess> reads;
    std::vector<MemoryAccess> writes;
    size_t programSize;  // Added to track program size
    
    typedef std::function<void(int32_t)> InstructionHandler;
    std::map<int, InstructionHandler> instructionSet;

    static std::string toHex(int32_t value) {
        std::stringstream ss;
        ss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << static_cast<uint32_t>(value);
        return ss.str();
    }

    void initInstructionSet() {
        instructionSet[0] = [this](int32_t offset) { state.b = state.a; state.a = offset; };
        instructionSet[1] = [this](int32_t offset) { state.a += offset; };
        instructionSet[2] = [this](int32_t offset) { 
            state.b = state.a;
            recordRead(state.sp + offset);
            state.a = memory[state.sp + offset];
        };
        instructionSet[3] = [this](int32_t offset) {
            recordWrite(state.sp + offset, state.a);
            memory[state.sp + offset] = state.a;
            state.a = state.b;
        };
        instructionSet[4] = [this](int32_t offset) {
            int32_t addr = state.a + offset;
            recordRead(addr);
            state.a = memory[addr];
        };
        instructionSet[5] = [this](int32_t offset) {
            recordWrite(state.a + offset, state.b);
            memory[state.a + offset] = state.b;
        };
        instructionSet[6] = [this](int32_t) { state.a = state.b + state.a; };
        instructionSet[7] = [this](int32_t) { state.a = state.b - state.a; };
        instructionSet[8] = [this](int32_t) { state.a = (state.b << state.a); };
        instructionSet[9] = [this](int32_t) { state.a = (state.b >> state.a); };
        instructionSet[10] = [this](int32_t offset) { state.sp += offset; };
        instructionSet[11] = [this](int32_t) { state.sp = state.a; state.a = state.b; };
        instructionSet[12] = [this](int32_t) { state.b = state.a; state.a = state.sp; };
        instructionSet[13] = [this](int32_t offset) { 
            state.b = state.a;
            state.a = state.pc;
            state.pc += offset;
        };
        instructionSet[14] = [this](int32_t) { state.pc = state.a; state.a = state.b; };
        instructionSet[15] = [this](int32_t offset) { if (state.a == 0) state.pc += offset; };
        instructionSet[16] = [this](int32_t offset) { if (state.a < 0) state.pc += offset; };
        instructionSet[17] = [this](int32_t offset) { state.pc += offset; };
    }

    void recordRead(uint32_t address) {
        MemoryAccess access;
        access.address = address;
        access.value = memory[address];
        reads.push_back(access);
    }

    void recordWrite(uint32_t address, int32_t newValue) {
        MemoryAccess access;
        access.address = address;
        access.value = newValue;
        access.previousValue = memory[address];
        writes.push_back(access);
    }

    std::pair<int, int32_t> decodeInstruction(int32_t instruction) {
        int type = instruction & 0xFF;
        int32_t offset = (instruction >> 8) & 0x7FFFFF;
        if (instruction & (1 << 31)) {
            offset -= (1 << 23);
        }
        return std::make_pair(type, offset);
    }

public:
    Emulator() : memory(20000005, 0), programSize(0) {  // Initialize programSize
        initInstructionSet();
    }

    bool loadProgram(const std::string& filename) {
        std::ifstream file(filename.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        // Get file size
        file.seekg(0, std::ios::end);
        programSize = file.tellg() / sizeof(int32_t);
        file.seekg(0, std::ios::beg);

        // Check if program size is valid
        if (programSize == 0 || programSize >= memory.size()) {
            std::cout << "Error: Invalid program size" << std::endl;
            return false;
        }

        // Read program into memory
        for (size_t i = 0; i < programSize; ++i) {
            int32_t instruction;
            if (file.read(reinterpret_cast<char*>(&instruction), sizeof(int32_t))) {
                memory[i] = instruction;
            } else {
                std::cout << "Error: Failed to read instruction at position " << i << std::endl;
                return false;
            }
        }

        std::cout << "Program loaded successfully. Size: " << programSize << " instructions" << std::endl;
        return true;
    }

    void execute() {  // Modified to use internal programSize
        if (programSize == 0) {
            std::cout << "No program loaded!" << std::endl;
            return;
        }

        uint32_t instructionCount = 0;
        const uint32_t MAX_INSTRUCTIONS = 1 << 24;

        std::cout << "\nExecution trace:" << std::endl;
        while (state.pc < static_cast<int32_t>(programSize)) {
            if (++instructionCount > MAX_INSTRUCTIONS || state.pc < 0) {
                std::cout << "Segmentation fault or execution limit exceeded\n";
                return;
            }

            // Display current instruction
            int32_t instruction = memory[state.pc];
            std::pair<int, int32_t> decoded = decodeInstruction(instruction);
            int type = decoded.first;
            int32_t offset = decoded.second;

            // Print state before execution
            std::cout << "PC=" << toHex(state.pc) 
                      << " SP=" << toHex(state.sp)
                      << " A=" << toHex(state.a)
                      << " B=" << toHex(state.b)
                      << " instruction=" << toHex(instruction)
                      << " type=" << type
                      << " offset=" << offset << std::endl;

            if (type == 18) {
                std::cout << "HALT instruction encountered" << std::endl;
                break;
            }
            
            std::map<int, InstructionHandler>::iterator it = instructionSet.find(type);
            if (it != instructionSet.end()) {
                it->second(offset);
            } else {
                std::cout << "Warning: Unknown instruction type " << type << std::endl;
            }
            
            state.pc++;
        }

        std::cout << "\nTotal instructions executed: " << instructionCount << std::endl;
    }

    void displayMemoryDump(bool afterExecution) {
        if (programSize == 0) {
            std::cout << "No program loaded!" << std::endl;
            return;
        }

        std::cout << "Memory dump " << (afterExecution ? "after" : "before") << " execution\n";
        for (size_t i = 0; i < programSize; i += 4) {
            std::cout << toHex(static_cast<int32_t>(i)) << " ";
            for (size_t j = i; j < std::min(programSize, i + 4); ++j) {
                std::cout << toHex(memory[j]) << " ";
            }
            std::cout << "\n";
        }
    }

    void displayMemoryReads() const {
        if (reads.empty()) {
            std::cout << "No memory reads recorded" << std::endl;
            return;
        }
        for (std::vector<MemoryAccess>::const_iterator it = reads.begin(); it != reads.end(); ++it) {
            std::cout << "Read memory[" << toHex(it->address) << "] found " 
                      << toHex(it->value) << "\n";
        }
    }

    void displayMemoryWrites() const {
        if (writes.empty()) {
            std::cout << "No memory writes recorded" << std::endl;
            return;
        }
        for (std::vector<MemoryAccess>::const_iterator it = writes.begin(); it != writes.end(); ++it) {
            std::cout << "Wrote memory[" << toHex(it->address) << "] was " 
                      << toHex(it->previousValue) << " now " << toHex(it->value) << "\n";
        }
    }

    void reset() {
        state = CPUState();
        reads.clear();
        writes.clear();
        // Don't clear memory or program size - just reset execution state
    }

    static void displayISA() {
        const char* isa[] = {
            "0      ldc      value",
            "1      adc      value",
            "2      ldl      value",
            "3      stl      value",
            "4      ldnl     value",
            "5      stnl     value",
            "6      add           ",
            "7      sub           ",
            "8      shl           ",
            "9      shr           ",
            "10     adj      value",
            "11     a2sp          ",
            "12     sp2a          ",
            "13     call     offset",
            "14     return        ",
            "15     brz      offset",
            "16     brlz     offset",
            "17     br       offset",
            "18     HALT          ",
            "       SET      value"
        };
        std::cout << "Opcode Mnemonic Operand\n";
        for (size_t i = 0; i < sizeof(isa)/sizeof(isa[0]); ++i) {
            std::cout << isa[i] << "\n";
        }
    }
};

int main() {
    Emulator emulator;
    std::string filename;
    std::cout << "Enter the filename to run emulator for: ";
    std::cin >> filename;
    
    if (!emulator.loadProgram(filename)) {
        std::cout << "Failed to load program. Exiting." << std::endl;
        return 1;
    }

    while (true) {
        std::cout << "\nMENU\n"
                  << "1: Get trace\n"
                  << "2: Display ISA\n"
                  << "3: Reset flags\n"
                  << "4: Show memory dump before execution\n"
                  << "5: Show memory dump after execution\n"
                  << "6: Show memory reads\n"
                  << "7: Show memory writes\n"
                  << "8: Exit\n\n"
                  << "Enter choice: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1: emulator.execute(); break;
            case 2: Emulator::displayISA(); break;
            case 3: emulator.reset(); break;
            case 4: emulator.displayMemoryDump(false); break;
            case 5: emulator.displayMemoryDump(true); break;
            case 6: emulator.displayMemoryReads(); break;
            case 7: emulator.displayMemoryWrites(); break;
            case 8: return 0;
            default: std::cout << "Invalid choice" << std::endl;
        }
    }
}