// interpreter for CMETA virtual machine in c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "opcode.h"


int flag = 0;

char c;
int got_char = 0;


void consume_char() {
  got_char = 1;
  c = getc(stdin);
}

char peek_char() {
  if(!got_char) { consume_char(); }
  return c;
}

void skip_whitespace() {
  while(isspace(peek_char())) {
    consume_char();
  }
}

void skip_whitespace_except(char c) {
  while(isspace(peek_char())) {
    if(peek_char() == c) { break; }
    consume_char();
  }
}

char token_buf;
char* output_buf = NULL;
int output_buf_cap = 0;
int output_buf_len = 0;
int output_col = 8;

void add_char_to_output_buffer(char token_buf) {
  if(output_buf_len >= output_buf_cap) {
    output_buf_cap = (output_buf_cap < 8 ? 8 : output_buf_cap * 1.5);
    output_buf = realloc(output_buf, output_buf_cap);
  }
  output_buf[output_buf_len++] = token_buf;
}

void clear_output_buf() {
  output_buf_len = 0;
}

void flush_output_buf() {
  for(int i = 0; i < output_col; i++) {
    putchar(' ');
  }
  
  printf("%.*s\n", output_buf_len, output_buf);
  
  output_buf_len = 0;
  output_col = 8;
}


int lit_char;
int matches_lit_char(int c) {
  return c == lit_char;
}

int not_matches_lit_char(int c) {
  return c != lit_char;
}

void check_and_consume_inner(int(*chk)(int), int tokenize) {
  if(lit_char == '\n' &&
     ((chk == &matches_lit_char) || (chk == &not_matches_lit_char))) {
    // HACK
    skip_whitespace_except('\n');
  } else {
    skip_whitespace();
  }
  char c = peek_char();
  if(chk(c)) {
    if(tokenize) {
      token_buf = c;
    }
    consume_char();
    flag = 1;
  } else {
    flag = 0;
  }
}


void check_and_consume(int(*chk)(int)) {
  check_and_consume_inner(chk, 0);
}


void check_and_tokenize(int(*chk)(int)) {
  check_and_consume_inner(chk, 1);
}

char error_buf[64];
void error() {
  printf("%s\n", error_buf);
  exit(1);
}

int lbl_cnt = 1;
int get_new_label() {
  int val = lbl_cnt++;
  return val;
}

int lbl1 = 0;
int lbl2 = 0;
int stack[1024];
int sp = 0;

void execute_prog(char* prog, int len) {
  int pc = 0;

  

  while(pc < len) {
    opcode op = prog[pc++];
#ifdef DEBUG
    printf("%i/%x: %s/%s", pc-1, pc-1, opcode_names[op], opcode_long_names[op]);
    if(op == T) {
      printf(" '%c' f: %i\n", prog[pc], flag);
    } else {
      printf(" f: %i\n", flag);
    }
#endif
    switch(op) {
      
    case T: do {
	char op = prog[pc++];
	lit_char = op;
	check_and_consume(&matches_lit_char);

	if(!flag) {
	  sprintf(error_buf, "Expected '%c' character, got '%c'.", 
		  lit_char, peek_char());
	}
      } while(0);
      break;

    case U: do {
	char op = prog[pc++];
	lit_char = op;
	check_and_tokenize(&not_matches_lit_char);
	if(!flag) {
	  sprintf(error_buf, "Expected anything but '%c' but failed.",
		  lit_char);
	}
      } while(0);
      break;
      
    case A: do {
	check_and_tokenize(&isalpha);
	if(!flag) {
	  sprintf(error_buf, "Expected alpha character, got '%c'.", peek_char());
	}
      } while(0);
      break;
      
    case N: do {
	check_and_tokenize(&isdigit);
	if(!flag) { 
	  sprintf(error_buf, "Expected numeric character, got '%c'.", peek_char());
	}
      } while(0);
      break;
      
    case J: do {
	uint8_t tgt_low = prog[pc++];
	uint8_t tgt_high = prog[pc++];
	uint16_t tgt = (tgt_high << 8 | tgt_low);
	stack[sp++] = lbl1;
	stack[sp++] = lbl2;
	stack[sp++] = pc;
	lbl1 = 0;
	lbl2 = 0;
	pc = tgt;
      } while(0);
      break;

    case R: do {
	pc = stack[--sp];
	lbl2 = stack[--sp];
	lbl1 = stack[--sp];
      } while(0);
      break;

    case S: do {
	flag = 1;
      } while(0);
      break;
      
    case B: do {
	uint8_t tgt_low = prog[pc++];
	uint8_t tgt_high = prog[pc++];
	uint16_t tgt = (tgt_high << 8 | tgt_low);
	pc = tgt;
      } while(0);
      break;

    case I: do {
	uint8_t tgt_low = prog[pc++];
	uint8_t tgt_high = prog[pc++];
	uint16_t tgt = (tgt_high << 8 | tgt_low);
	if(flag) {
#ifdef DEBUG
	  printf("BRANCHED to %i/%x\n", tgt, tgt);
#endif
	  pc = tgt;
	}
      } while(0);
      break;
      
    case E: do {
	if(flag == 0) {
	  error();
	}
      } while(0);
      break;

    case L: do {
	char c = prog[pc++];
	add_char_to_output_buffer(c);
      } while(0);
      break;
      
    case C: do {
	add_char_to_output_buffer(token_buf);
      } while(0);
      break;

    case H: do {
	uint8_t l = token_buf%16;
	uint8_t h = (token_buf/16)%16;
	char low = (l >= 10) ? ('A' + l-10) : ('0' + l);
	char high = (h >= 10) ? ('A' + h-10) : ('0' + h);
	add_char_to_output_buffer(high);
	add_char_to_output_buffer(low);
      } while(0);
      break;
      
    case F: do {
	output_col = 0;
      } while(0);
      break;
      
    case G: do {
	int lbl_num = prog[pc++];
	int used_label;
	
	if(lbl_num == 1) {
	  if(lbl1 == 0) {
	    lbl1 = get_new_label();
	  }
	  used_label = lbl1;
	} else if (lbl_num == 2) {
	  if(lbl2 == 0) {
	    lbl2 = get_new_label();
	  }
	  used_label = lbl2;
	}

	char buf[32];
	char* fmt_string;

	if(output_buf_len == 0) {
	  fmt_string = ":l%03i";
	} else {
	  fmt_string = "&l%03i";
	}
	int chars = sprintf(buf, fmt_string, used_label);
	for(int i = 0; i < chars; i++) {
	  add_char_to_output_buffer(buf[i]);
	}
      } while(0);
      break;
      
    case O: do {
	flush_output_buf();
      } while(0);
      break;

    case D: do {
	clear_output_buf();	
      } while(0);
      break;

    default: do {
	printf("Unexpected opcode %i\n", op);
	exit(1);
      } while(0);
      break;
      
    }
  }

}

char* prog = NULL;
int prog_sz = 0;
int prog_cap = 0;


void add_byte_to_program(char c) {
  if(prog_sz >= prog_cap) {
    prog_cap = (prog_cap < 8) ? 8 : (prog_cap * 1.5);
    prog = realloc(prog, prog_cap);
  }

  prog[prog_sz++] = c;
  
}
  
int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Expected a filepath argument with a cmeta program\n");
    exit(1);
  }
  
  FILE* f = fopen(argv[1], "rb");
  if(!f) {
    printf("Error opening program file\n");
    exit(1);
  }
  
  // read bytes into buffer until EOF
  uint8_t cnt_low = getc(f);
  uint8_t cnt_high = getc(f);
  uint16_t cnt_bytes = (cnt_high << 8 | cnt_low);

  
  for(int i = 0; i < cnt_bytes; i++) {
    add_byte_to_program(getc(f));
  }
  

  execute_prog(prog, prog_sz);
    
  
  return 0;
}
