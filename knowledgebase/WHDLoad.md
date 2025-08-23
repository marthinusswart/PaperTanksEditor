Here's a step-by-step table for packaging **your own coded Amiga game** into WHDLoad:

| **Step** | **Task**                        | **What You Do**                                                           | **Files/Tools Needed**               | **Difficulty** |
| -------- | ------------------------------- | ------------------------------------------------------------------------- | ------------------------------------ | -------------- |
| **1**    | **Prepare Game Files**          | Organize all your game files (executable, data, graphics, sounds)         | Your compiled game files             | Easy           |
| **2**    | **Create Game Directory**       | Make folder: `YourGame/` and copy all game files into it                  | File manager                         | Easy           |
| **3**    | **Download WHDLoad SDK**        | Get the official WHDLoad development package                              | WHDLoad DEV package from WHDLoad.de  | Easy           |
| **4**    | **Study Simple Slave Template** | Examine basic slave examples (look for non-cracked game slaves)           | Example .slave source files          | Medium         |
| **5**    | **Write Basic Slave File**      | Create `YourGame.slave` - simple assembly wrapper that launches your game | Text editor, assembler (VASM/AsmOne) | Medium         |
| **6**    | **Configure Slave Header**      | Set game name, memory requirements, chipset in slave header               | WHDLoad documentation                | Easy           |
| **7**    | **Add Clean Exit Support**      | Ensure your game responds to quit key (Ctrl+C) to return to Workbench     | Modify your game source code         | Easy-Medium    |
| **8**    | **Handle Save Games**           | Make sure saves go to PROGDIR: instead of hardcoded paths                 | Modify your game source code         | Easy           |
| **9**    | **Compile Slave**               | Assemble your .slave file into working binary                             | VASM assembler                       | Easy           |
| **10**   | **Create Install Script**       | Write simple `Install` script to copy files and setup                     | Text editor, basic AmigaDOS          | Easy           |
| **11**   | **Add Documentation**           | Create readme with controls, requirements, version info                   | Text editor                          | Easy           |
| **12**   | **Test on Emulator**            | Test the WHDLoad package works correctly                                  | UAE/WinUAE emulator                  | Medium         |
| **13**   | **Create .info Files**          | Generate Workbench icons for your files                                   | IconEdit or copy existing ones       | Easy           |
| **14**   | **Package into LHA**            | Compress everything into distributable archive                            | LhA archiver                         | Easy           |
| **15**   | **Final Testing**               | Test packaged .lha works on clean system                                  | Clean emulator setup                 | Medium         |

**Key Advantage for Your Own Game:**

- **Steps 5 & 9** are much simpler since you don't need complex patches
- Your slave is basically just a launcher with memory setup
- **Steps 7 & 8** can be done by modifying your source code rather than binary patching

**Most Time-Consuming Steps:**

- **Step 5**: Writing the slave (but much simpler for your own game)
- **Steps 12 & 15**: Testing and debugging

**Estimated Time:** 1-3 days for a first-time WHDLoad package of your own game, versus weeks/months for reverse-engineering commercial games.
