#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>
#include <cstdio>
#include <cctype>
#include <cstdlib>

using namespace std;

struct Encoding
{
    string opcode;
    int operandCount;
    string operand1;
    string operand2;
};

string toUpper(string str)
{
    for (char &c : str)
        c = toupper(c);

    return str;
}

bool isReg(string operand)
{
    if (operand == "EAX" || operand == "EBX" ||
        operand == "ECX" || operand == "EDX" ||
        operand == "ESI" || operand == "EDI" ||
        operand == "EBP" || operand == "ESP")
    {
        return true;
    }

    return false;
}

void initDS(unordered_map<char, unordered_map<string, vector<Encoding>>> &ds)
{
    FILE *opcodeFile = fopen("opcode.txt", "r");

    if (opcodeFile == NULL)
    {
        perror("optable.txt does not exist");
        exit(1);
    }

    char line[256];

    while (fgets(line, sizeof(line), opcodeFile))
    {
        stringstream words(line);

        string mnemonic;
        Encoding enc;

        words >> mnemonic;

        if (mnemonic == "Instruction")
            continue;

        words >> enc.opcode;
        words >> enc.operandCount;

        if (enc.operandCount >= 1)
            words >> enc.operand1;
        else
            enc.operand1 = "-";

        if (enc.operandCount == 2)
            words >> enc.operand2;
        else
            enc.operand2 = "-";

        mnemonic = toUpper(mnemonic);

        ds[mnemonic[0]][mnemonic].push_back(enc);
    }

    fclose(opcodeFile);
}

string getOperandType(string operand)
{
    operand = toUpper(operand);

    if (isReg(operand))
        return "REG32";

    if (operand == "CL")
        return "CL";

    if (operand.length() >= 2 &&
        operand[0] == '[' &&
        operand[operand.length() - 1] == ']')
    {
        return "RM32";
    }

    if (operand.length() == 0)
        return "UNKNOWN";

    bool number = true;
    int start = 0;

    if (operand[0] == '-' || operand[0] == '+')
        start = 1;

    if (start == operand.length())
        return "UNKNOWN";

    for (int i = start; i < operand.length(); i++)
    {
        if (!isdigit(operand[i]))
        {
            number = false;
            break;
        }
    }

    if (number)
    {
        long long value = stoll(operand);

        if (value >= -128 && value <= 255)
            return "IMM8";

        if (value >= -32768 && value <= 65535)
            return "IMM16";

        return "IMM32";
    }

    return "UNKNOWN";
}

bool operandMatches(string operand, string expected)
{
    operand = toUpper(operand);
    expected = toUpper(expected);

    if (operand == expected)
        return true;

    if (expected == "REG32")
    {
        if (isReg(operand))
            return true;
    }

    if (expected == "RM32")
    {
        if (isReg(operand))
            return true;

        if (operand.length() >= 2 &&
            operand[0] == '[' &&
            operand[operand.length() - 1] == ']')
        {
            return true;
        }
    }

    string type = getOperandType(operand);

    if (expected == "IMM8" && type == "IMM8")
        return true;

    if (expected == "IMM16" &&
        (type == "IMM8" || type == "IMM16"))
        return true;

    if (expected == "IMM32" &&
        (type == "IMM8" ||
         type == "IMM16" ||
         type == "IMM32"))
        return true;

    if (expected == "REL8" && type == "IMM8")
        return true;

    if (expected == "REL32" &&
        (type == "IMM8" ||
         type == "IMM16" ||
         type == "IMM32"))
        return true;

    return false;
}

bool matchEncoding(Encoding &enc, vector<string> &operands)
{
    if (enc.operandCount != operands.size())
        return false;

    if (enc.operandCount == 0)
        return true;

    if (enc.operandCount >= 1)
    {
        if (!operandMatches(operands[0], enc.operand1))
            return false;
    }

    if (enc.operandCount == 2)
    {
        if (!operandMatches(operands[1], enc.operand2))
            return false;
    }

    return true;
}

void validate(
    FILE *fp,
    unordered_map<char, unordered_map<string, vector<Encoding>>> &ds)
{
    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        int i = 0;

        while (line[i] == ' ' || line[i] == '\t')
            i++;

        if (line[i] == '\0' || line[i] == '\n')
            continue;

        if (!isalpha(line[i]))
        {
            cout << "Error: Invalid beginning of line: " << line;
            continue;
        }

        string mnemonic;

        while (isalpha(line[i]))
        {
            mnemonic += line[i];
            i++;
        }

        mnemonic = toUpper(mnemonic);

        auto typeIt = ds.find(mnemonic[0]);

        if (typeIt == ds.end())
        {
            cout << "Error: Unknown mnemonic "
                 << mnemonic << endl;
            continue;
        }

        auto mnemonicIt = typeIt->second.find(mnemonic);

        if (mnemonicIt == typeIt->second.end())
        {
            cout << "Error: Unknown mnemonic "
                 << mnemonic << endl;
            continue;
        }

        vector<Encoding> &encodings = mnemonicIt->second;

        while (line[i] == ' ' || line[i] == '\t')
            i++;

        vector<string> operands;
        bool syntaxError = false;

        if (line[i] != '\0' && line[i] != '\n')
        {
            while (true)
            {
                while (line[i] == ' ' || line[i] == '\t')
                    i++;

                if (line[i] == '\0' || line[i] == '\n')
                {
                    syntaxError = true;
                    break;
                }

                if (line[i] == ',')
                {
                    cout << "Error: Unexpected comma in line: "
                         << line << endl;

                    syntaxError = true;
                    break;
                }

                string operand;

                while (line[i] != ',' &&
                       line[i] != ' ' &&
                       line[i] != '\t' &&
                       line[i] != '\n' &&
                       line[i] != '\0')
                {
                    operand += line[i];
                    i++;
                }

                operand = toUpper(operand);

                if (operand.empty())
                {
                    cout << "Error: Invalid operand in line: "
                         << line << endl;

                    syntaxError = true;
                    break;
                }

                operands.push_back(operand);

                while (line[i] == ' ' || line[i] == '\t')
                    i++;

                if (line[i] == '\0' || line[i] == '\n')
                    break;

                if (line[i] == ',')
                {
                    i++;

                    while (line[i] == ' ' || line[i] == '\t')
                        i++;

                    if (line[i] == '\0' || line[i] == '\n')
                    {
                        cout << "Error: Missing operand after comma: "
                             << line << endl;

                        syntaxError = true;
                        break;
                    }

                    continue;
                }

                cout << "Error: Expected comma in line: "
                     << line << endl;

                syntaxError = true;
                break;
            }
        }

        if (syntaxError)
            continue;

        bool matched = false;
        Encoding *selectedEncoding = nullptr;

        for (Encoding &enc : encodings)
        {
            if (matchEncoding(enc, operands))
            {
                matched = true;
                selectedEncoding = &enc;
                break;
            }
        }

        if (!matched)
        {
            cout << "Error: Invalid operands for "
                 << mnemonic << endl;

            cout << "Given operands: ";

            for (string &operand : operands)
            {
                cout << operand
                     << "("
                     << getOperandType(operand)
                     << ") ";
            }

            cout << endl << endl;
            continue;
        }

        cout << "Valid: " << mnemonic << " ";

        for (string &operand : operands)
        {
            cout << operand
                 << "("
                 << getOperandType(operand)
                 << ") ";
        }

        cout << endl;

        cout << "Encoding: "
             << selectedEncoding->opcode
             << endl;

        cout << endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "./executable [sample_assembly_file]"
             << endl;

        return 1;
    }

    unordered_map<char, unordered_map<string, vector<Encoding>>> ds;

    initDS(ds);

    FILE *fp = fopen(argv[1], "r");

    if (fp == NULL)
    {
        perror("File not open");
        return 1;
    }

    validate(fp, ds);

    fclose(fp);

    return 0;
}
