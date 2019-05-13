#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "opcode.h"

// stage0 compiler for CMETA ASM
// handles four character labels, single characters and all CMETA opcodes

/* grammar

:labl -- defines label labl

&labl -- inserts absolute reference to label labl
T 'c' -- tests for 'c'


*/



int got_char = 0;
char cur_char;

void consume() {
  
  got_char = 1;
  cur_char = getc(stdin);
}

char peek() {
  if(!got_char) { consume(); }
  
  return cur_char;
}

void skip_whitespace() {
  while(isspace(peek())) {
    consume();
  }
}

char* buf = NULL;
int buf_sz = 0;
int buf_cap = 0;

void add_char_to_buf(char c) {
  if(buf_sz >= buf_cap) {
    buf_cap = (buf_cap < 8) ? 8 : buf_cap * 1.5;
    buf = realloc(buf, buf_cap);
  }
  buf[buf_sz++] = c;
}

int cur_addr = 0;
void output_byte(char c) {
  printf("%c", c);
  cur_addr++;
}


int num_labels = 0;

typedef struct {
  char sym[4];
  uint16_t addr;
} label;

label label_defs[256];


label* find_label(char lbl[4]) {
  
  for(int i = 0; i < num_labels; i++) {
    for(int j = 0; j < 4; j++) {
      if(lbl[j] != label_defs[i].sym[j]) {
	goto next_label_def;
      }
    }
    
    return &(label_defs[i]);
    
  next_label_def:;
  }

  return NULL;
}

void handle_label_definition(char lbl[4], int output) {
    
  if(output) {
    return;
  }

  if(find_label(lbl) != NULL) {
    printf("Label '%.*s' already defined!\n", 4, lbl);
    exit(1);
  } else {
    if(num_labels >= 256) {
      printf("Too many labels defined.\n");
      exit(1);
    }
    memcpy(label_defs[num_labels].sym, lbl, 4);
    label_defs[num_labels++].addr = cur_addr;
  }
}

void handle_label_reference(char* lbl, int output) {
  if(!output) {
    cur_addr++;
    cur_addr++;
    return;
  }

  label* lab = find_label(lbl);
  if(lab == NULL) {
    printf("Reference to undefined label '%.*s'\n", 4, lbl);
    exit(1);
  } else {
    
    output_byte(lab->addr & 0xFF);
    output_byte((lab->addr >> 8) & 0xFF);
    
  }
}

void handle_literal_char(char c, int output) {
  if(output) {
    output_byte(c);
  } else {
    cur_addr++;
  }
}

void handle_number(char c, int output) {
  handle_literal_char(c-'0', output);
}

void handle_opcode(char op, int output) {

  int num_opcodes = sizeof(opcode_names)/sizeof(opcode_names[0]);

  for(int i = 0; i < num_opcodes; i++) {
    if(opcode_names[i][0] == op) {
      if(output) {
	output_byte(i);
      } else {
	cur_addr++;
      }
      return;
    }
  }

  printf("unexpected opcode '%c'\n", op);
  exit(1);
}

int main() {

  while(1) {
    skip_whitespace();
    char c = peek();
    if(c == EOF) {
      break;
    } else if (c == '\'') {
      add_char_to_buf('\'');
      consume();
      c = peek();
      add_char_to_buf(c);
      consume();
      c = peek();
      if(c != '\'') {
	printf("Unbalanced single quote\n");
	exit(1);
      }
      add_char_to_buf('\'');
      consume();
    } else {
      add_char_to_buf(c);
    }
    consume();
  }

  for(int output = 0; output < 2; output++) {
    int i = 0;
    for(; i < buf_sz; i++) {
      char c = buf[i];
      
      if(c == EOF) {
	exit(1);
      } else if (c == '&') {
	i++;
	char label_buf[4];
	label_buf[0] = buf[i++];
	label_buf[1] = buf[i++];
	label_buf[2] = buf[i++];
	label_buf[3] = buf[i];

	handle_label_reference(label_buf, output);
	
      } else if (c == ':') {
	i++;
	char label_buf[4];
	label_buf[0] = buf[i++];
	label_buf[1] = buf[i++];
	label_buf[2] = buf[i++];
	label_buf[3] = buf[i];

	handle_label_definition(label_buf, output);
      } else if (c == '\'') {
	i++;
	handle_literal_char(buf[i], output);
	i++;
	if(buf[i] != '\'') {
	  printf("Expected ' but got %c\n", buf[i]);
	}
      } else if (isdigit(c)) {
	handle_number(c, output);
      } else {
	handle_opcode(c, output);
      }
    }
    cur_addr = 0;

  }
  
  
  
  
  }
