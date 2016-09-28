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

union expr_value {
  double number;
  char *string;
  char *symbol;
};

struct expr {
  enum expr_type type;
  char *text;
  union expr_value value;
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
    e->value.number = atof(token);
  } else if (isstring2(token)) {
    e->type = STRING;
    e->text = token;
    e->value.string = token;
  } else if (issymbol2(token)) {
    e->type = SYMBOL;
    e->text = token;
    e->value.symbol = token;
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

void print_expr(struct expr *e) {
  if (e->type == NUMBER) {
    printf("NUMBER %.6f", e->value.number);
  } else if (e->type == STRING) {
    printf("STRING %s", e->value.string);
  } else if (e->type == SYMBOL) {
    printf("SYMBOL %s", e->value.symbol);
  } else if (e->type == COMPOUND) {
    printf("COMPOUND");
  } else {
    printf("UNKNOWN TYPE");
  }
  printf("\n");
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
    print_expr(root);
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

struct env_entry {
  char *key;
  struct expr *value;
  struct env_entry *next;
};

struct env {
  struct env_entry *entries;
  struct env *parent;
};

struct expr *lookup_env_var(struct env *environ, char *key) {
  for (struct env_entry *p = environ->entries; p != NULL; p++) {
    if (strcmp2(p->key, key) == 0) {
      return p->value;
    }
  }
  return NULL;
}

struct expr *define_env_var(struct env *environ, char *key, struct expr *value) {
  struct env_entry *new_entry = malloc(sizeof (struct env_entry));
  new_entry->key = key;
  new_entry->value = value;
  new_entry->next = environ->entries;
  environ->entries = new_entry;
  return value;
}

struct expr *eval(struct expr *e, struct env *environ) {
  printf("EVAL: ");
  print_expr(e);

  if (e->type == NUMBER || e->type == STRING) {
    return e;

  } else if (e->type == SYMBOL) {
    printf("ANALYZING LOOKUP %s\n", e->value.symbol);
    return lookup_env_var(environ, e->value.symbol);

  } else if (e->type == COMPOUND) {
    struct expr *first  = e->children[0];

    if (first->type == SYMBOL) {
      // Definition
      if (strcmp2(first->value.symbol, "define") == 0) {
        char *key = e->children[1]->value.symbol;
        struct expr *value = eval(e->children[2], environ);
        return define_env_var(environ, key, value);
      }

      // Begin expression
      else if (strcmp2(first->value.symbol, "begin") == 0) {
        printf("ANALYZING BEGIN %d\n", e->n_children);
        struct expr *value;
        for (int child_n = 1; child_n < e->n_children; child_n++) {
          printf("ANALYZING %d\n", child_n);
          value = eval(e->children[child_n], environ);
        }
        return value;
      }
      // Lambda
    }

    // Application

  }
  return NULL;
}

void print_env(struct env *environ) {
  printf("\nENV:\n====\n");
  for (struct env_entry *p = environ->entries; p; p = p->next) {
    printf("%s => ", p->key);
    print_expr(p->value);
  }
}

void test_eval(char *code) {
  char *tokens[MAX_SUB_EXPR];
  tokenize(code, tokens);

  struct env environ;
  struct expr e;
  e.type = NUMBER;
  e.value.number = 13.;
  struct env_entry entry;
  entry.key = "my-val";
  entry.value = &e;
  entry.next = NULL;
  environ.parent = NULL;
  environ.entries = &entry;

  struct expr root;
  parse(tokens, &root);

  printf("AST:\n====\n");
  print_ast(&root, 0);

  struct expr *res;
  res = eval(&root, &environ);

  printf("\nRESULT:\n=======\n");
  print_expr(res);

  print_env(&environ);

  printf("\n");
}

int main(int argc, char **argv) {
  //test_tokenize("(+ (* 1 2)\n(/ 3 4 (my-fun 5 6 \"hehe \")))");

  //test_eval("1");
  //test_eval("my-val");
  //test_eval("(define my-other-val \"aaa\")");
  test_eval("(begin \
    (define user-val 123) \
    user-val \
  )");
  //test_eval("(begin (define user-val 123) user-val)");

  ///char *tokens[MAX_SUB_EXPR];
  ///int n_tokens = tokenize("(+ 1 (* 2 3) (/ 4 5))", tokens);
  //int n_tokens = tokenize("1", tokens);

  //eval(

  //struct expr root;
  //parse(tokens, &root);

  //print_ast(&root, 0);

  ////printf("strcmp2: %d\n", strcmp2("abcd", "abce"));

  //test_tokenize("");
  //test_tokenize("2");
  //test_tokenize("(+ 1 2)");
  //test_tokenize("    ( +      \t 1 2 )    \t  ");
  //test_tokenize("(+ \"aa a\")");
  //test_tokenize("(+ 1 2)\n(+ 2 3)");
  //struct vresult
  //printf("%d\n", isnumber2("23.0123"));
  //printf("%d\n", issymbol2("-aaa"));
  //printf("%d\n", issymbol2("aa12 3a"));
  //printf("%d\n", isstring2("\"aa'12 3a\""));
}

//(+ (+ 1 2) (* 4 5))
