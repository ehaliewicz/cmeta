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

void check_and_consume_inner(int(*chk)(int), int tokenize) {
  skip_whitespace();
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
    //printf("%i: op %s/%s\n", pc-1, opcode_names[op], opcode_long_names[op]);
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
  
  // read bytes into buffer until EOF
  char c = getc(f);
  while(c != EOF) {
    add_byte_to_program(c);
    c = getc(f);
  }
  

  execute_prog(prog, prog_sz);
  // execute program
  
  /*
  char prog[] = {
    T, '(',
    E,
// num_list: 3
    J, long_num, 
    T, ',',
    I, skip_branch_to_end_num_list,
// branch to end_num_list: 9
    B, 12,
// skip_branch_to_end_num_list
    S,
    B, 3, 
// end_num_list: 12
    T, ')',
    E,
    B, 25, 
    
//long_num: 17
    N,
    T, skip_exit_long_num
//exit_long_num:
    B, 23, 
//skip_exit_long_num:
    C,
    B, 17, 
//end_num: 23
    O,
    R
//exit: 25
    
  };
  
  execute_prog(prog, sizeof(prog)/sizeof(char));
  */
  
  return 0;
}
