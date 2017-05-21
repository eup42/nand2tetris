// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

    @24576
    D=A
    @bottm
    M=D-1

(LOOP)
    @bottm
    D=M
    @current
    M=D

    @KBD
    D=M

    @WHITE
    D;JEQ

    @BLACK
    0;JMP

(WHITE)
    @current
    D=M
    @SCREEN
    D=D-M
    @LOOP
    D;JLT

    @KBD
    D=M
    @LOOP
    D;JNE

    @current
    A=M
    M=0

    @current
    M=M-1

    @WHITE
    0;JMP


(BLACK)
    @current
    D=M
    @SCREEN
    D=D-M
    @LOOP
    D;JLT

    @KBD
    D=M
    @LOOP
    D;JEQ

    @current
    A=M
    M=-1

    @current
    M=M-1

    @BLACK
    0;JMP
