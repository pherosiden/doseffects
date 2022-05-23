
             
              PACKET EFFECTS (c) 1998 by Nguyen Ngoc Van

In this packet, included:

- Folder EFFECTS   : All graphics effects (In 44 subfolders)
- Folder UTILITIES :
  1. SEA 1.3 : Use to view and convert images to CEL format.
  2. PKLITE 1.5 : Use to compress your EXEs file after building.
  3. EXE2COM 1.04 : Use to convert EXEs file to COMs file.
     (See each effect for more details)

All sources code are written with Borland C++ 3.1 and
Turbo Assembler 5.0 (for 32bit protect mode use Open Watcom C++)

Before using it, you have to settings up environment
so that programs can be run exactly.

In Borland C++ 3.1 IDEs, following steps:

1 - Menu Options->Compiler->Code Generation then check on
    compact memory mode.

2 - Menu Options->Compiler->Advance code... then check on
    80287/387 floating point. Check on 80386 instruction set.

3 - Menu Options->Debugger... then type 640 in the program heap
    size box.

4 - Menu Options->Reference..->Editor then type 3 in the tab size box. 
    (Easy to view your codes. For funny only!)

5 - Open file autoexec.bat add the path to Borland C++ 3.1 and PKLITE.
    (ie. SET PATH=E:\BORLANDC\BIN;E:\PKLITE, I install them on driver E:)
    (Easy to use built.bat in each effect)

NOTE : All images in this packet are CELs format. You can use utility
SEA 1.3 (Photodex) to view and convert them.

All execute programs start with Space Bar key and close with ESC key.
You must run them under DOS (using cmd.exe or MS-DOS Prompt).

File format CEL (320x200x256 colors):

      +-------------------+
      |      Header       |
      |     32 bytes      |
      +-------------------+
      |      Palette      |
      |     768 bytes     |
      +-------------------+
      |     Data raw      |
      |   64000 bytes     |
      +-------------------+

If you don't like images in this packet, you can use any image that
you like (with the same size and colors palette) then convert them to
the CEL format. (use SEA 1.3 utility).

            I.   Combination modules ASM into main program

1. Create a project then add module cra.asm and crc.cpp

2. Press ALT + F9, then press F9 to link project file to EXE

3. Using PKLITE utility to compress project file
   PKLITE testasm.exe (You will prompt a message, please chose [Y])

             II.  Combination image into main program

1. Using the BCC to generate file fadeio.cpp -> fadeio.exe
   BCC -mc -1 fadeio.cpp (You must copy fadeio.cpp to X:\BORLANDC\BIN\)

2. Appending image arnold.cel into fadeio.exe
   COPY /B fadeio.exe + arnold.cel (You will prompt a message, please
   chose [Y] then enter)

3. Using PKLITE utility to compress fadeio.exe
   PKLITE fadeio.exe (You will prompt a message, please chose [Y])

                                              Have fun :)
                                            Nguyen Ngoc Van						
                                         pherosiden@yahoo.com
                                        http://www.codedemo.net
