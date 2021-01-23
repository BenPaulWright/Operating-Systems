#include <fstream>
#include <math.h>
#include <string>

int args[7];

void generateProcess(int tid)
{
    // Generate file name
    std::string fileName = "output/thread";
    fileName.append(std::to_string(tid));
    fileName.append(".txt");

    // Open file
    std::FILE *outFile = fopen(fileName.c_str(), "w");

    // Random power of 2 from 512 to 8192
    int memCapacity = pow(2, 9 + rand() % 4);
    fprintf(outFile, "%d\n", memCapacity);

    char rwInstruction = 'X';
    if (args[4] && !args[5])
        rwInstruction = 'R';
    if (!args[5] && args[4])
        rwInstruction = 'W';

    for (int i = 0; i < args[3]; i++)
    {
        if (args[4] && args[5])
            rwInstruction = rand() % 2 == 1 ? 'R' : 'W';
        fprintf(outFile, "%c r%d %d%s",
                rwInstruction,
                rand() % 32,
                rand() % memCapacity,
                i != args[3] - 1 ? "\n" : "");
    }
}

int main(int argc, char **argv)
{
    std::FILE *inFile = fopen("config.txt", "r");
    fscanf(inFile, "%*s %d\n%*s %d\n%*s %d\n%*s %d\n%*s %d\n%*s %d\n%*s %d\n", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6]);
    srand(args[6]);

    // Open file
    std::FILE *outFile = fopen("output/infile1.txt", "w");

    fprintf(outFile, "%d\n%d\n%d\n", args[0], args[1], args[2]);
    for (int i = 1; i <= args[2]; i++)
    {
        fprintf(outFile, "thread%d.txt%s",
                i,
                i != args[2] ? "\n" : "");
        generateProcess(i);
    }

    return 0;
}