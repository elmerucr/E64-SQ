/* Generated by buildtables.py */

static const char *mnemonics[256] = {
	// $0X
	/* $00 */ "brk ",
	/* $01 */ "ora ($%02x,x)",
	/* $02 */ "nop ",
	/* $03 */ "nop ",
	/* $04 */ "tsb $%02x",
	/* $05 */ "ora $%02x",
	/* $06 */ "asl $%02x",
	/* $07 */ "rmb0 $%02x",
	/* $08 */ "php ",
	/* $09 */ "ora #$%02x",
	/* $0A */ "asl a",
	/* $0B */ "nop ",
	/* $0C */ "tsb $%04x",
	/* $0D */ "ora $%04x",
	/* $0E */ "asl $%04x",
	/* $0F */ "bbr0 $%02x, $%04x",

	// $1X
	/* $10 */ "bpl $%02x",
	/* $11 */ "ora ($%02x),y",
	/* $12 */ "ora ($%02x)",
	/* $13 */ "nop ",
	/* $14 */ "trb $%02x",
	/* $15 */ "ora $%02x,x",
	/* $16 */ "asl $%02x,x",
	/* $17 */ "rmb1 $%02x",
	/* $18 */ "clc ",
	/* $19 */ "ora $%04x,y",
	/* $1A */ "inc a",
	/* $1B */ "nop ",
	/* $1C */ "trb $%04x",
	/* $1D */ "ora $%04x,x",
	/* $1E */ "asl $%04x,x",
	/* $1F */ "bbr1 $%02x, $%04x",

	// $2X
	/* $20 */ "jsr $%04x",
	/* $21 */ "and ($%02x,x)",
	/* $22 */ "nop ",
	/* $23 */ "nop ",
	/* $24 */ "bit $%02x",
	/* $25 */ "and $%02x",
	/* $26 */ "rol $%02x",
	/* $27 */ "rmb2 $%02x",
	/* $28 */ "plp ",
	/* $29 */ "and #$%02x",
	/* $2A */ "rol a",
	/* $2B */ "nop ",
	/* $2C */ "bit $%04x",
	/* $2D */ "and $%04x",
	/* $2E */ "rol $%04x",
	/* $2F */ "bbr2 $%02x, $%04x",

	// $3X
	/* $30 */ "bmi $%02x",
	/* $31 */ "and ($%02x),y",
	/* $32 */ "and ($%02x)",
	/* $33 */ "nop ",
	/* $34 */ "bit $%02x,x",
	/* $35 */ "and $%02x,x",
	/* $36 */ "rol $%02x,x",
	/* $37 */ "rmb3 $%02x",
	/* $38 */ "sec ",
	/* $39 */ "and $%04x,y",
	/* $3A */ "dec a",
	/* $3B */ "nop ",
	/* $3C */ "bit $%04x,x",
	/* $3D */ "and $%04x,x",
	/* $3E */ "rol $%04x,x",
	/* $3F */ "bbr3 $%02x, $%04x",

	// $4X
	/* $40 */ "rti ",
	/* $41 */ "eor ($%02x,x)",
	/* $42 */ "nop ",
	/* $43 */ "nop ",
	/* $44 */ "nop ",
	/* $45 */ "eor $%02x",
	/* $46 */ "lsr $%02x",
	/* $47 */ "rmb4 $%02x",
	/* $48 */ "pha ",
	/* $49 */ "eor #$%02x",
	/* $4A */ "lsr a",
	/* $4B */ "nop ",
	/* $4C */ "jmp $%04x",
	/* $4D */ "eor $%04x",
	/* $4E */ "lsr $%04x",
	/* $4F */ "bbr4 $%02x, $%04x",

	// $5X
	/* $50 */ "bvc $%02x",
	/* $51 */ "eor ($%02x),y",
	/* $52 */ "eor ($%02x)",
	/* $53 */ "nop ",
	/* $54 */ "nop ",
	/* $55 */ "eor $%02x,x",
	/* $56 */ "lsr $%02x,x",
	/* $57 */ "rmb5 $%02x",
	/* $58 */ "cli ",
	/* $59 */ "eor $%04x,y",
	/* $5A */ "phy ",
	/* $5B */ "nop ",
	/* $5C */ "nop ",
	/* $5D */ "eor $%04x,x",
	/* $5E */ "lsr $%04x,x",
	/* $5F */ "bbr5 $%02x, $%04x",

	// $6X
	/* $60 */ "rts ",
	/* $61 */ "adc ($%02x,x)",
	/* $62 */ "nop ",
	/* $63 */ "nop ",
	/* $64 */ "stz $%02x",
	/* $65 */ "adc $%02x",
	/* $66 */ "ror $%02x",
	/* $67 */ "rmb6 $%02x",
	/* $68 */ "pla ",
	/* $69 */ "adc #$%02x",
	/* $6A */ "ror a",
	/* $6B */ "nop ",
	/* $6C */ "jmp ($%04x)",
	/* $6D */ "adc $%04x",
	/* $6E */ "ror $%04x",
	/* $6F */ "bbr6 $%02x, $%04x",

	// $7X
	/* $70 */ "bvs $%02x",
	/* $71 */ "adc ($%02x),y",
	/* $72 */ "adc ($%02x)",
	/* $73 */ "nop ",
	/* $74 */ "stz $%02x,x",
	/* $75 */ "adc $%02x,x",
	/* $76 */ "ror $%02x,x",
	/* $77 */ "rmb7 $%02x",
	/* $78 */ "sei ",
	/* $79 */ "adc $%04x,y",
	/* $7A */ "ply ",
	/* $7B */ "nop ",
	/* $7C */ "jmp ($%04x,x)",
	/* $7D */ "adc $%04x,x",
	/* $7E */ "ror $%04x,x",
	/* $7F */ "bbr7 $%02x, $%04x",

	// $8X
	/* $80 */ "bra $%02x",
	/* $81 */ "sta ($%02x,x)",
	/* $82 */ "nop ",
	/* $83 */ "nop ",
	/* $84 */ "sty $%02x",
	/* $85 */ "sta $%02x",
	/* $86 */ "stx $%02x",
	/* $87 */ "smb0 $%02x",
	/* $88 */ "dey ",
	/* $89 */ "bit #$%02x",
	/* $8A */ "txa ",
	/* $8B */ "nop ",
	/* $8C */ "sty $%04x",
	/* $8D */ "sta $%04x",
	/* $8E */ "stx $%04x",
	/* $8F */ "bbs0 $%02x, $%04x",

	// $9X
	/* $90 */ "bcc $%02x",
	/* $91 */ "sta ($%02x),y",
	/* $92 */ "sta ($%02x)",
	/* $93 */ "nop ",
	/* $94 */ "sty $%02x,x",
	/* $95 */ "sta $%02x,x",
	/* $96 */ "stx $%02x,y",
	/* $97 */ "smb1 $%02x",
	/* $98 */ "tya ",
	/* $99 */ "sta $%04x,y",
	/* $9A */ "txs ",
	/* $9B */ "nop ",
	/* $9C */ "stz $%04x",
	/* $9D */ "sta $%04x,x",
	/* $9E */ "stz $%04x,x",
	/* $9F */ "bbs1 $%02x, $%04x",

	// $AX
	/* $A0 */ "ldy #$%02x",
	/* $A1 */ "lda ($%02x,x)",
	/* $A2 */ "ldx #$%02x",
	/* $A3 */ "nop ",
	/* $A4 */ "ldy $%02x",
	/* $A5 */ "lda $%02x",
	/* $A6 */ "ldx $%02x",
	/* $A7 */ "smb2 $%02x",
	/* $A8 */ "tay ",
	/* $A9 */ "lda #$%02x",
	/* $AA */ "tax ",
	/* $AB */ "nop ",
	/* $AC */ "ldy $%04x",
	/* $AD */ "lda $%04x",
	/* $AE */ "ldx $%04x",
	/* $AF */ "bbs2 $%02x, $%04x",

	// $BX
	/* $B0 */ "bcs $%02x",
	/* $B1 */ "lda ($%02x),y",
	/* $B2 */ "lda ($%02x)",
	/* $B3 */ "nop ",
	/* $B4 */ "ldy $%02x,x",
	/* $B5 */ "lda $%02x,x",
	/* $B6 */ "ldx $%02x,y",
	/* $B7 */ "smb3 $%02x",
	/* $B8 */ "clv ",
	/* $B9 */ "lda $%04x,y",
	/* $BA */ "tsx ",
	/* $BB */ "nop ",
	/* $BC */ "ldy $%04x,x",
	/* $BD */ "lda $%04x,x",
	/* $BE */ "ldx $%04x,y",
	/* $BF */ "bbs3 $%02x, $%04x",

	// $CX
	/* $C0 */ "cpy #$%02x",
	/* $C1 */ "cmp ($%02x,x)",
	/* $C2 */ "nop ",
	/* $C3 */ "nop ",
	/* $C4 */ "cpy $%02x",
	/* $C5 */ "cmp $%02x",
	/* $C6 */ "dec $%02x",
	/* $C7 */ "smb4 $%02x",
	/* $C8 */ "iny ",
	/* $C9 */ "cmp #$%02x",
	/* $CA */ "dex ",
	/* $CB */ "wai ",
	/* $CC */ "cpy $%04x",
	/* $CD */ "cmp $%04x",
	/* $CE */ "dec $%04x",
	/* $CF */ "bbs4 $%02x, $%04x",

	// $DX
	/* $D0 */ "bne $%02x",
	/* $D1 */ "cmp ($%02x),y",
	/* $D2 */ "cmp ($%02x)",
	/* $D3 */ "nop ",
	/* $D4 */ "nop ",
	/* $D5 */ "cmp $%02x,x",
	/* $D6 */ "dec $%02x,x",
	/* $D7 */ "smb5 $%02x",
	/* $D8 */ "cld ",
	/* $D9 */ "cmp $%04x,y",
	/* $DA */ "phx ",
	/* $DB */ "dbg ",
	/* $DC */ "nop ",
	/* $DD */ "cmp $%04x,x",
	/* $DE */ "dec $%04x,x",
	/* $DF */ "bbs5 $%02x, $%04x",

	// $EX
	/* $E0 */ "cpx #$%02x",
	/* $E1 */ "sbc ($%02x,x)",
	/* $E2 */ "nop ",
	/* $E3 */ "nop ",
	/* $E4 */ "cpx $%02x",
	/* $E5 */ "sbc $%02x",
	/* $E6 */ "inc $%02x",
	/* $E7 */ "smb6 $%02x",
	/* $E8 */ "inx ",
	/* $E9 */ "sbc #$%02x",
	/* $EA */ "nop ",
	/* $EB */ "nop ",
	/* $EC */ "cpx $%04x",
	/* $ED */ "sbc $%04x",
	/* $EE */ "inc $%04x",
	/* $EF */ "bbs6 $%02x, $%04x",

	// $FX
	/* $F0 */ "beq $%02x",
	/* $F1 */ "sbc ($%02x),y",
	/* $F2 */ "sbc ($%02x)",
	/* $F3 */ "nop ",
	/* $F4 */ "nop ",
	/* $F5 */ "sbc $%02x,x",
	/* $F6 */ "inc $%02x,x",
	/* $F7 */ "smb7 $%02x",
	/* $F8 */ "sed ",
	/* $F9 */ "sbc $%04x,y",
	/* $FA */ "plx ",
	/* $FB */ "nop ",
	/* $FC */ "nop ",
	/* $FD */ "sbc $%04x,x",
	/* $FE */ "inc $%04x,x",
	/* $FF */ "bbs7 $%02x, $%04x"};
