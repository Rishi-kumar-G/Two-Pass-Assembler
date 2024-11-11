/*Name - Rishi kumar 
   Roll number -2301cs83*/


#include <bits/stdc++.h>
using namespace std;

class AssemblerInfo {
private:
    struct CodeLine {
        string label;
        string mnemonic;
        string operand;
        string prevOperand;
        int pctr;
        int location;
    };

    struct Message {
        int location;
        string content;
        bool operator<(const Message& other) const {
            return location < other.location;
        }
    };

    vector<Message> errors;
    vector<Message> warnings;
    vector<CodeLine> codeLines;
    map<string, pair<int, int>> symbolTable;
    map<string, vector<int>> labelReferences;
    map<string, string> variables;
    map<int, string> lineComments;
    map<string, pair<string, int>> opcodeTable;
    
    struct OutputLine {
        string address;
        string machineCode;
        string statement;
    };
    vector<OutputLine> listingContent;
    vector<string> machineCodes;

public:
    void initializeOpcodeTable() {
        opcodeTable = {
            {"data", {"", 1}}, {"ldc", {"00", 1}}, {"adc", {"01", 1}},
            {"ldl", {"02", 2}}, {"stl", {"03", 2}}, {"ldnl", {"04", 2}},
            {"stnl", {"05", 2}}, {"add", {"06", 0}}, {"sub", {"07", 0}},
            {"shl", {"08", 0}}, {"shr", {"09", 0}}, {"adj", {"0A", 1}},
            {"a2sp", {"0B", 0}}, {"sp2a", {"0C", 0}}, {"call", {"0D", 2}},
            {"return", {"0E", 0}}, {"brz", {"0F", 2}}, {"brlz", {"10", 2}},
            {"br", {"11", 2}}, {"HALT", {"12", 0}}, {"SET", {"", 1}}
        };
    }

    bool isValidCharacter(char c, bool isFirst = false) {
        if (isFirst) return isalpha(c);
        return isalnum(c) || c == '_';
    }

    bool isValidLabel(const string& label) {
        if (label.empty() || !isValidCharacter(label[0], true)) return false;
        return all_of(label.begin() + 1, label.end(), 
                     [this](char c) { return isValidCharacter(c); });
    }

    bool isNumber(const string& str, int base) {
        if (str.empty()) return false;
        try {
            size_t pos;
            stoll(str, &pos, base);
            return pos == str.length();
        } catch (...) {
            return false;
        }
    }


string convertNumber(const string& num) {
    if (num.empty()) return "";  // Return empty string for "no value"
    
    string value = num;
    string sign = "";
    if (value[0] == '+' || value[0] == '-') {
        sign = value[0];
        value = value.substr(1);
    }

    try {
        if (value.length() >= 2 && value[0] == '0') {
            if (tolower(value[1]) == 'x') {
                if (!isNumber(value.substr(2), 16)) return "";
                return sign + to_string(stoll(value.substr(2), nullptr, 16));
            }
            if (isNumber(value, 8)) {
                return sign + to_string(stoll(value, nullptr, 8));
            }
        }
        
        if (!isNumber(value, 10)) return "";
        return sign + value;
    } catch (...) {
        return "";
    }
}


    string toHex8Bit(int num) {
        stringstream ss;
        ss << setfill('0') << setw(8) << uppercase << hex << (unsigned int)num;
        return ss.str();
    }

    vector<string> tokenizeLine(const string& line, int lineNum) {
        vector<string> tokens;
        istringstream ss(line);
        string token;
        
        while (ss >> token) {
            if (token[0] == ';') {
                string comment;
                getline(ss, comment);
                lineComments[lineNum] = token + comment;
                break;
            }
            
            if (token.back() == ';') {
                token.pop_back();
                if (!token.empty()) tokens.push_back(token);
                string comment;
                getline(ss, comment);
                lineComments[lineNum] = ";" + comment;
                break;
            }
            
            size_t colonPos = token.find(':');
            if (colonPos != string::npos && colonPos != token.length() - 1) {
                tokens.push_back(token.substr(0, colonPos + 1));
                tokens.push_back(token.substr(colonPos + 1));
            } else {
                tokens.push_back(token);
            }
        }
        
        // Handle trailing comments
        string rest;
        getline(ss, rest);
        size_t commentPos = rest.find(';');
        if (commentPos != string::npos) {
            lineComments[lineNum] = rest.substr(commentPos);
        }
        
        return tokens;
    }

    void processLabel(const string& label, int lineNum, int pctr) {
        if (label.empty()) return;
        
        if (!isValidLabel(label)) {
            errors.push_back({lineNum, "Bogus Label name"});
            return;
        }

        if (symbolTable.count(label) && symbolTable[label].first != -1) {
            errors.push_back({lineNum, "Duplicate label definition"});
            return;
        }

        symbolTable[label] = {pctr, lineNum};
    }

    string processOperand(const string& operand, int lineNum) {
        if (operand.empty()) return "";
        
        if (isValidLabel(operand)) {
            if (!symbolTable.count(operand)) {
                symbolTable[operand] = {-1, lineNum};
            }
            labelReferences[operand].push_back(lineNum);
            return operand;
        }

        string converted = convertNumber(operand);
        if (converted.empty()) {
            return "";
        }
        return converted;
    }

    void firstPass(const vector<string>& sourceLines) {
        int lineNum = 0, pctr = 0;
        
        for (const string& line : sourceLines) {
            lineNum++;
            vector<string> tokens = tokenizeLine(line, lineNum);
            if (tokens.empty()) continue;

            CodeLine currentLine;
            currentLine.location = lineNum;
            currentLine.pctr = pctr;
            
            size_t pos = 0;
            if (pos < tokens.size() && tokens[pos].back() == ':') {
                currentLine.label = tokens[pos].substr(0, tokens[pos].length() - 1);
                pos++;
            }

            if (pos < tokens.size()) {
                currentLine.mnemonic = tokens[pos++];
            }

            if (pos < tokens.size()) {
                currentLine.operand = tokens[pos];
                currentLine.prevOperand = tokens[pos];
                pos++;
            }

            // Check for extra tokens
            if (pos < tokens.size()) {
                errors.push_back({lineNum, "extra on end of line"});
            }

            // Process the line
            processLabel(currentLine.label, lineNum, pctr);
            
            bool validInstruction = false;
            if (!currentLine.mnemonic.empty()) {
                if (opcodeTable.count(currentLine.mnemonic)) {
                    int operandType = opcodeTable[currentLine.mnemonic].second;
                    
                    if (operandType > 0 && currentLine.operand.empty()) {
                        errors.push_back({lineNum, "Missing operand"});
                    } else if (operandType == 0 && !currentLine.operand.empty()) {
                        errors.push_back({lineNum, "unexpected operand"});
                    } else {
                        if (operandType > 0) {
                            string processedOperand = processOperand(currentLine.operand, lineNum);
                            if (processedOperand.empty()) {
                                errors.push_back({lineNum, "Invalid format: not a valid label or a number"});
                            } else {
                                currentLine.operand = processedOperand;
                                validInstruction = true;
                            }
                        } else {
                            validInstruction = true;
                        }
                    }
                } else {
                    errors.push_back({lineNum, "Bogus Mnemonic"});
                }
            }

            if (validInstruction) {
                if (currentLine.mnemonic == "SET") {
                    if (currentLine.label.empty()) {
                        errors.push_back({lineNum, "label(or variable) name missing"});
                    } else {
                        variables[currentLine.label] = currentLine.operand;
                    }
                }
                pctr++;
            }

            codeLines.push_back(currentLine);
        }

        // Validate labels
        for (const auto& symbol : symbolTable) {
            if (symbol.second.first == -1) {
                for (int lineNum : labelReferences[symbol.first]) {
                    errors.push_back({lineNum, "no such label"});
                }
            } else if (!labelReferences.count(symbol.first)) {
                warnings.push_back({symbol.second.second, "label declared but not used"});
            }
        }
    }

    void secondPass() {
        for (const auto& line : codeLines) {
            string machineCode = "        ";
            
            if (!line.mnemonic.empty() && opcodeTable.count(line.mnemonic)) {
                int operandType = opcodeTable[line.mnemonic].second;
                
                if (operandType == 2) {
                    try {
                        int offset;
                        if (symbolTable.count(line.operand)) {
                            offset = symbolTable[line.operand].first - (line.pctr + 1);
                        } else {
                            offset = stoi(line.operand);
                        }
                        machineCode = toHex8Bit(offset).substr(2) + opcodeTable[line.mnemonic].first;
                    } catch (...) {
                        // Skip invalid conversion
                    }
                }
                else if (operandType == 1) {
                    try {
                        if (line.mnemonic != "data" && line.mnemonic != "SET") {
                            int value;
                            if (variables.count(line.operand)) {
                                value = stoi(variables[line.operand]);
                            } else if (symbolTable.count(line.operand)) {
                                value = symbolTable[line.operand].first;
                            } else {
                                value = stoi(line.operand);
                            }
                            machineCode = toHex8Bit(value).substr(2) + opcodeTable[line.mnemonic].first;
                        } else {
                            machineCode = toHex8Bit(stoi(line.operand));
                        }
                    } catch (...) {
                        // Skip invalid conversion
                    }
                }
                else if (operandType == 0) {
                    machineCode = "000000" + opcodeTable[line.mnemonic].first;
                }
            }

            machineCodes.push_back(machineCode);
            
            string statement = (line.label.empty() ? "" : line.label + ": ") +
                             (line.mnemonic.empty() ? "" : line.mnemonic + " ") +
                             line.prevOperand;
            
            listingContent.push_back({toHex8Bit(line.pctr), machineCode, statement});
        }
    }

    void writeOutput(const string& baseFilename) {
        string baseName = baseFilename.substr(0, baseFilename.find_last_of("."));
        
        // Write listing file
        ofstream listFile(baseName + ".lst");
        for (const auto& line : listingContent) {
            listFile << line.address << " " << line.machineCode << " " << line.statement << "\n";
        }
        listFile.close();
        cout << "Listing file generated: " << baseName << ".lst" << endl;

        // Write object file if no errors
        if (errors.empty()) {
            ofstream objFile(baseName + ".o", ios::binary);
            for (const auto& code : machineCodes) {
                if (code != "        ") {
                    unsigned int value = stoul(code, nullptr, 16);
                    objFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
                }
            }
            objFile.close();
            cout << "Machine code object file generated: " << baseName << ".o" << endl;
        }

        // Write log file
        ofstream logFile(baseName + ".log");
        if (errors.empty()) {
            logFile << "No errors!\n";
            for (const auto& warning : warnings) {
                logFile << "Line " << warning.location << " WARNING: " << warning.content << "\n";
            }
        } else {
            for (const auto& error : errors) {
                logFile << "Line " << error.location << " ERROR: " << error.content << "\n";
            }
        }
        logFile.close();
        cout << "Log file generated: " << baseName << ".log" << endl;
    }

    void assemble(const string& filename) {
        // Validate file extension
        if (filename.substr(filename.find_last_of(".") + 1) != "asm") {
            throw runtime_error("Input file must have .asm extension");
        }

        // Read source file
        ifstream sourceFile(filename);
        if (!sourceFile) {
            throw runtime_error("Unable to open source file: " + filename);
        }

        vector<string> sourceLines;
        string line;
        while (getline(sourceFile, line)) {
            sourceLines.push_back(line);
        }
        sourceFile.close();

        // Process the assembly
        initializeOpcodeTable();
        firstPass(sourceLines);
        
        if (errors.empty()) {
            secondPass();
        }
        
        writeOutput(filename);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename.asm>" << endl;
        return 1;
    }

    try {
        AssemblerInfo assembler;
        assembler.assemble(argv[1]);
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}