#include "kernel.hpp"
#include "common.hpp"

#include "sqstdblob.h"
#include "sqstdio.h"
#include "sqstdaux.h"
#include "sqstdsystem.h"
#include "sqstdmath.h"
#include "sqstdstring.h"

SQInteger quit(HSQUIRRELVM v)
{
    int *done;
    sq_getuserpointer(v,-1,(SQUserPointer*)&done);
    *done=1;
    return 0;
}

void printfunc(HSQUIRRELVM v,const SQChar *s,...)
{
	char buffer[1024];
	va_list vl;
	va_start(vl, s);
	vsnprintf(buffer, 1024, s, vl);
	va_end(vl);
	machine.kernel->tty->puts(buffer);
}

E64::kernel_t::kernel_t()
{
	v = sq_open(1024); // creates a squirrel VM with initial stack size 1024
	sq_setprintfunc(v, printfunc, printfunc);
	sq_pushroottable(v);

	sqstd_register_bloblib(v);
	sqstd_register_iolib(v);
	sqstd_register_systemlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);

	 //aux library
	 //sets error handlers
	 sqstd_seterrorhandlers(v);
	
	
	build_character_ram();
	tty = new tty_t(0x56, cbm_font, C64_LIGHTBLUE, C64_BLUE);
	
	blocks =0;
	string=0;
	retval=0;
	done=0;
}

E64::kernel_t::~kernel_t()
{
	delete tty;
	sq_close(v);
}

void E64::kernel_t::reset()
{
	machine.vicv->set_horizontal_border_color(C64_BLACK);
	machine.vicv->set_horizontal_border_size(16);
	machine.blitter->set_clearcolor(C64_BLUE);
	
	// clear sids
	for (int i=0; i<128; i++) machine.sids->write_byte(i, 0);
	for (int i=0; i<8; i++) machine.sids->write_byte(128+i, 255);
	machine.sids->write_byte(0x18, 0x0f);		// volume sid0
	machine.sids->write_byte(0x38, 0x0f);		// volume sid1

	// sounds
	machine.sids->write_byte(0x00, 0xc4);		// note d3
	machine.sids->write_byte(0x01, 0x09);
	machine.sids->write_byte(0x05, 0b00001001);	// attack/decay
	machine.sids->write_byte(0x02, 0x0f);		// pulsewidth
	machine.sids->write_byte(0x03, 0x0f);
	machine.sids->write_byte(0x80, 0xff);		// sid0 left
	machine.sids->write_byte(0x81, 0x10);		// sid0 right
	machine.sids->write_byte(0x04, 0b01000001);	// voice control
	
	machine.sids->write_byte(0x20, 0xa2);		// note a3
	machine.sids->write_byte(0x21, 0x0e);
	machine.sids->write_byte(0x25, 0b00001001);	// attack/decay
	machine.sids->write_byte(0x22, 0x0f);		// pulsewidth
	machine.sids->write_byte(0x23, 0x0f);
	machine.sids->write_byte(0x82, 0x10);		// sid1 left
	machine.sids->write_byte(0x83, 0xff);		// sid1 right
	machine.sids->write_byte(0x24, 0b01000001);	// voice control
	
	tty->clear();
	tty->puts("E64-SQ Virtual Computer System\n");
	tty->printf("Copyright (C)%u elmerucr\n\n", E64_SQ_YEAR);
	tty->printf(_SC("%s\n"),SQUIRREL_VERSION);
	tty->printf(_SC("%s (%d bits)\n"),SQUIRREL_COPYRIGHT,((int)(sizeof(SQInteger)*8)));
	tty->prompt();
	
	sq_pushroottable(v);
	sq_pushstring(v,_SC("quit"),-1);
	sq_pushuserpointer(v,&done);
	sq_newclosure(v,quit,1);
	sq_setparamscheck(v,1,NULL);
	sq_newslot(v,-3,SQFalse);
	sq_pop(v,1);
	
	devices.cia_set_keyboard_repeat_delay(50);
	devices.cia_set_keyboard_repeat_speed(5);
	devices.cia_generate_key_events();
	
	devices.timer_set(0, 3600);
	tty->activate_cursor();
}

void E64::kernel_t::process_keypress()
{
	uint8_t key_value = machine.cia->read_byte(0x04);
	switch (key_value) {
		case ASCII_CURSOR_LEFT:
			tty->cursor_left();
			break;
		case ASCII_CURSOR_RIGHT:
			tty->cursor_right();
			break;
		case ASCII_CURSOR_UP:
			tty->cursor_up();
			break;
		case ASCII_CURSOR_DOWN:
			tty->cursor_down();
			break;
		case ASCII_BACKSPACE:
			tty->backspace();
			break;
		case ASCII_LF: {
				char *buffer = tty->enter_command();
			tty->putchar('\n');
			int i = strlen(buffer);
			
			SQInteger oldtop=sq_gettop(v);
			if(SQ_SUCCEEDED(sq_compilebuffer(v,buffer,i,_SC("interactive console"),SQTrue))){
			    sq_pushroottable(v);
			    if(SQ_SUCCEEDED(sq_call(v,1,retval,SQTrue)) &&  retval){
				tty->printf(_SC("\n"));
				sq_pushroottable(v);
				sq_pushstring(v,_SC("print"),-1);
				sq_get(v,-2);
				sq_pushroottable(v);
				sq_push(v,-4);
				sq_call(v,2,SQFalse,SQTrue);
				retval=0;
				tty->printf(_SC("\n"));
			    }
			}

			sq_settop(v,oldtop);
			
				tty->prompt();
				//tty_reset_start_end_command();
			}
			break;
		default:
			tty->putchar(key_value);
			break;
	}
}

void E64::kernel_t::execute()
{
	while (machine.cia->read_byte(0x00)) {
		tty->deactivate_cursor();
		process_keypress();
		tty->activate_cursor();
	}
}

void E64::kernel_t::vblank_event()
{
	machine.vicv->swap_buffers();
	machine.blitter->clear_framebuffer();
	machine.blitter->add_blit(tty->text_screen, 0, 16);
}

void E64::kernel_t::timer_0_event()
{
	tty->timer_callback();
}

void E64::kernel_t::timer_1_event()
{
	//
}

void E64::kernel_t::timer_2_event()
{
	//
}

void E64::kernel_t::timer_3_event()
{
	//
}

void E64::kernel_t::timer_4_event()
{
	//
}

void E64::kernel_t::timer_5_event()
{
	//
}

void E64::kernel_t::timer_6_event()
{
	//
}

void E64::kernel_t::timer_7_event()
{
	//
}

void E64::kernel_t::build_character_ram()
{
	cbm_font = (uint16_t *)machine.mmu->malloc(64 * 256 * sizeof(uint16_t));
	
	uint16_t *dest = cbm_font;
	
	for (int i=0; i<2048; i++) {
		uint8_t byte = cbm_cp437_font[i];
		uint8_t count = 8;
		while (count--) {
			*dest = (byte & 0b10000000) ? C64_GREY : 0x0000;
			dest++;
			byte = byte << 1;
		}
	}
}
