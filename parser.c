#include <stdio.h>
#include <stdlib.h>

#define MAX_SUB_EXPR 20

enum expr_type {
  COMPOUND, LITERAL
};

struct expr {
  enum expr_type type;
  char *text;
  struct expr *children[MAX_SUB_EXPR];
  int n_children;
};

struct text {
  char *text;
  long pos;
};

int strcmp2(char *, char *);

//struct expr parse(struct text *code) {
//  char *c = code->text;
//  struct expr e;
//
//  if (*c == '(') {
//    e.type = COMPOUND;
//    e.text = code->text;
//    // parse subexprs
//    printf("Parsing subexpressions...\n");
//    for (code->pos++; *(c = code->text + code->pos); code->pos++) {
//      printf("Got character: %c\n", *c);
//    }
//
//  } else {
//    e.type = LITERAL;
//    e.text = code->text;
//
//
//  }
//  //for (c = text; *c; c++) {
//  //  printf("Got character: %c", *c);
//  //}
//  return e;
//}

void strncpy2(char *from, char *to, int n_bytes) {
  //printf("Copying %d bytes...\n", n_bytes);
  for (int i = 0; i < n_bytes; i++) {
    to[i] = from[i];
  }
  to[n_bytes] = '\0';
  //printf("Done.\n");
}

int isspace2(char c) {
  return (c == ' ' || c == '\t' || c == '\n');
}

int tokenize(char *str, char *tokens[]) {
  int n = 0;
  int inside_string = 0;
  char *start;

  for (start = str; *str; str++) {

    if (isspace2(*str) && !inside_string) {

      // It is terminating some expression
      if (start != str) {
        tokens[n] = (char *) malloc(str - start + 1);
        strncpy2(start, tokens[n++], (int) (str - start));
      }
      start = str + 1;

    } else if (*str == '(' || *str == ')') {

      // Maybe flush token being current parsed
      if (start != str) {
        tokens[n] = (char *) malloc(str - start + 1);
        strncpy2(start, tokens[n++], (int) (str - start));
        start = str;
      }

      tokens[n] = (char *) malloc(2);
      strncpy2(start, tokens[n++], 1);
      start = str + 1;

    } else if (*str == '"') {

      // Starting string
      if (!inside_string) {
        inside_string = 1;
        start = str;

      // Finishing string
      } else {
        inside_string = 0;
        tokens[n] = (char *) malloc(str - start + 2);
        strncpy2(start, tokens[n++], (int) (str - start + 1));
        start = str + 1;
      }
    }
  }

  // EOF was read. Flush the current epxression, if any.
  if (start != str) {
    tokens[n] = (char *) malloc(str - start + 1);
    strncpy2(start, tokens[n++], (int) (str - start));
  }

  return n;
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
    e->type = LITERAL;
    e->text = tokens[0];
    tokens++;
  }
  return tokens - base;
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

void print_ast(struct expr *root, int indent) {
  for (int j=0; j < indent; j++)
    printf("\t");

  if (root->type == COMPOUND) {
    printf("COMPOUND\n");
    for (int i=0; i < root->n_children; i++) {
      print_ast(root->children[i], indent+1);
    }
  } else {
    printf("LITERAL ");
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

  test_tokenize("");
  test_tokenize("2");
  test_tokenize("(+ 1 2)");
  test_tokenize("    ( +      \t 1 2 )    \t  ");
  test_tokenize("(+ \"aa a\")");
  test_tokenize("(+ 1 2)\n(+ 2 3)");
}

//(+ (+ 1 2) (* 4 5))
