# LGA775 Phoenix bios microcode swapper
Simple tool to swap microcode of Phoenix based bios file.


# Warning

* This program is based on "intelmicrocodelist" source code .
* This tool is use to edit/swap intel cpu microcode in Phoenix based bios file.
* This tool cannot add or delete exsist microcode , it can only SWAP exsist microcode with another microcode with the same size to keep the size of bios file unchanged.
* Use with caution , always have your backup.

# How to use 
* compile the program with <code>make</code>
* Run compiled file <code>./microcode-swapper</code> and follow the instructions.
* Flash new bios.
