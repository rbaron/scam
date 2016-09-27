#include <stdio.h>
#include <stdlib.h>

#define MAX_SUB_EXPR 20

enum expr_type {
  COMPOUND,
  NUMBER,
  STRING,
  SYMBOL,
};

const char *type_names[] = {
  "COMPOUND",
  "NUMBER",
  "STRING",
  "SYMBOL",
};

union values {
  double number;
  char *string;
  char *symbol;
};

struct expr {
  enum expr_type type;
  char *human_type;
  char *text;
  struct expr *children[MAX_SUB_EXPR];
  int n_children;
};

void strncpy2(char *from, char *to, int n_bytes) {
  for (int i = 0; i < n_bytes; i++) {
    to[i] = from[i];
  }
  to[n_bytes] = '\0';
}

int strlen2(char *str) {
  int i;
  for (i = 0; *str; str++, i++)
    ;
  return i;
}

int strcmp2(char *a, char *b) {
  for (; *a == *b; a++, b++)
    if (*a == '\0' && *b == '\0')
      return 0;
  return *a - *b;
}

int isspace2(char c) {
  return (c == ' ' || c == '\t' || c == '\n');
}

int isnumber2(char *token) {
  int n_dots = 0;
  char *start = token;

  for (; *token; token++) {
    if (*token > '9' || *token < '0') {

      // Only one dot
      if (*token == '.' && n_dots++ == 0) {
        continue;

      // Sign only in the beginning
      } else if (*token == '-' && token == start) {
        continue;

      } else {
        return 0;
      }
    }
  }

  // empty string is not a number
  return start != token;
}

int issymbol2(char *token) {
  char *start = token;

  if (*token >= '0' && *token <= '9')
    return 0;

  for (; *token; token++)
    if (*token == '"' || *token == ' ')
      return 0;

  // empty string is not a symbol
  return start != token;
}

int isstring2(char *token) {
  char *start = token;

  if (*token != '"')
    return 0;

  while (*token) {
    printf("D: %c\n", *token);
    if (*token == '"' && start != token && *(token+1) != '\0') {
      return 0;
    }

    token++;
  }

  // empty string is not a symbol
  return start != token && *(token-1) == '"';
}

void allocate_and_copy(char *tokens[], int token_index, char *start, int len) {
  tokens[token_index] = (char *) malloc(len + 1);
  strncpy2(start, tokens[token_index], len);
}

int tokenize(char *str, char *tokens[]) {
  int n = 0;
  int inside_string = 0;
  char *start;

  for (start = str; *str; str++) {

    if (isspace2(*str) && !inside_string) {

      // It is terminating some expression
      if (start != str) {
        allocate_and_copy(tokens, n++, start, str - start);
      }
      start = str + 1;

    } else if (*str == '(' || *str == ')') {

      // Maybe flush token being currently parsed
      if (start != str) {
        allocate_and_copy(tokens, n++, start, str - start);
        start = str;
      }

      allocate_and_copy(tokens, n++, start, 1);
      start = str + 1;

    } else if (*str == '"') {

      // Starting string
      if (!inside_string) {
        inside_string = 1;
        start = str;

      // Finishing string
      } else {
        inside_string = 0;
        allocate_and_copy(tokens, n++, start, str - start + 1);
        start = str + 1;
      }
    }
  }

  // EOF was read. Maybe flush token being currently parsed
  if (start != str) {
    allocate_and_copy(tokens, n++, start, str - start);
  }

  return n;
}

int parse_literal(char *token, struct expr *e) {
  if (isnumber2(token)) {
    e->type = NUMBER;
    e->text = token;
  } else if (isstring2(token)) {
    e->type = STRING;
    e->text = token;
  } else if (issymbol2(token)) {
    e->type = SYMBOL;
    e->text = token;
  } else {
    fprintf(stderr, "Unknown literal: %s\n", token);
    exit(1);
  }
  return 0;
}

int parse(char *tokens[], struct expr *e) {
  char **base = tokens;
  int step;

  // Compound expression
  if (!strcmp2(tokens[0], "(")) {
    e->type = COMPOUND ;
    e->n_children = 0;

    // step over the "(" token
    tokens++;

    while (strcmp2(*tokens, ")") != 0) {
      struct expr *child = (struct expr *) malloc(sizeof (struct expr));
      step = parse(tokens, child);
      e->children[e->n_children++] = child;

      // Advance tokens by the number of tokens processed
      // in the recursive call
      tokens += step;
    }
    // step over the ")" token
    tokens++;

  // Literal expression
  } else {

    parse_literal(tokens[0], e);
    //e->type = LITERAL;
    //e->text = tokens[0];
    tokens++;
  }
  return tokens - base;
}

void print_ast(struct expr *root, int indent) {
  for (int j=0; j < indent; j++)
    printf("\t");

  if (root->type == COMPOUND) {
    printf("COMPOUND\n");
    for (int i=0; i < root->n_children; i++) {
      print_ast(root->children[i], indent+1);
    }
  } else {
    printf("%s ", type_names[root->type]);
    printf("%s\n", root->text);
  }
}

void test_tokenize(char *code) {
  char *tokens[MAX_SUB_EXPR];
  int n_tokens = tokenize(code, tokens);

  printf("Will tokenize %s\n", code);
  printf("Tokenize found %d tokens\n", n_tokens);

  for (int i = 0; i < n_tokens; i++) {
    printf("%s, ", tokens[i]);
  }
  printf ("\n");

  struct expr root;
  parse(tokens, &root);

  print_ast(&root, 0);
}

int main(int argc, char **argv) {
  //char *tokens[MAX_SUB_EXPR];
  //int n_tokens = tokenize("(+ 1 (* 2 3) (/ 4 5))", tokens);
  //int n_tokens = tokenize("1", tokens);

  //struct expr root;
  ////root.type = COMPOUND;
  ////root.n_children = 0;
  //parse(tokens, &root);

  //print_ast(&root, 0);

  ////printf("strcmp2: %d\n", strcmp2("abcd", "abce"));

  //test_tokenize("");
  //test_tokenize("2");
  //test_tokenize("(+ 1 2)");
  //test_tokenize("    ( +      \t 1 2 )    \t  ");
  //test_tokenize("(+ \"aa a\")");
  //test_tokenize("(+ 1 2)\n(+ 2 3)");
  test_tokenize("(+ (* 1 2)\n(/ 3 4 (my-fun 5 6 \"hehe \")))");
  //struct vresult
  //printf("%d\n", isnumber2("23.0123"));
  //printf("%d\n", issymbol2("-aaa"));
  //printf("%d\n", issymbol2("aa12 3a"));
  //printf("%d\n", isstring2("\"aa'12 3a\""));
}

//(+ (+ 1 2) (* 4 5))
