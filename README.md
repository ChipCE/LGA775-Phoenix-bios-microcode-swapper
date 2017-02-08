# Bios Intel microcode editor
ver 1.4b 2017-02-09

# Warning
* This program is based on "intelmicrocodelist" source code .
* This tool is use to edit bios intel microcode.
* This tool cannot add or delete exsist microcode , it can only REPLACE exsist microcode with another microcode with the same size to keep the size of bios file unchanged.
* Use with caution , always have your backup.

# How to use 
* you will need the original bios file that you need to edit and load it with option(1).
* you will need the microcode lib you need to insert and load it with option (3).Microcode lib file can be something like "ncpucode-lga-771-775-microcode-cpuid-1067x-6Fx.bin" or another bios file.
* replace original bios microcode with another microcode with the same size in the lib file  (5).
* export edited bios with option(7).
