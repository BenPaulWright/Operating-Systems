Program which generates input files for Professor Geiger's OS project 4

To compile:
make

To run:
./InFileGenerator

Config file:
Physical_memory_capicity: (some multiple of Frame_size)
Frame_size: (int)
Process_count: (int)
Instructions_per_process: 16
Read_instruction_enabled: (1 / 0)
Write_instruction_enabled: (1 / 0)
Seed: (int)

Note* do not change the structure of the config file or the program will crash