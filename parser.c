#include <stdio.h>
#include <stdlib.h>

#define MAX_SUB_EXPR 32
#define MAX_TOKENS 1024

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

/*
  Parse
*/
enum parse_expr_type {
  COMPOUND,
  NUMBER,
  STRING,
  SYMBOL,
};

union parse_expr_value {
  double number;
  char *string;
  char *symbol;
};

struct parse_expr {
  enum parse_expr_type type;
  char *text;
  union parse_expr_value value;
  struct parse_expr *children[MAX_SUB_EXPR];
  int n_children;
};


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

int parse_literal(char *token, struct parse_expr *e) {
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

int parse(char *tokens[], struct parse_expr *e) {
  char **base = tokens;
  int step;

  // Compound expression
  if (!strcmp2(tokens[0], "(")) {
    e->type = COMPOUND ;
    e->n_children = 0;

    // step over the "(" token
    tokens++;

    while (strcmp2(*tokens, ")") != 0) {
      struct parse_expr *child = (struct parse_expr *) malloc(sizeof (struct parse_expr));
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
    tokens++;
  }
  return tokens - base;
}

/*
  Eval
*/
#define PROC_MAX_ARGS 32

struct env_entry {
  char *key;
  struct eval_value *value;
  struct env_entry *next;
};

struct env {
  struct env_entry *entries;
  struct env *parent;
};

void print_env(struct env *);

struct eval_value;

struct procedure {
  enum {
    PROC_USER_DEFINED,
    PROC_PRIMITIVE,
  } type;

  // TODO: make an union for user-defined/primitive procedures

  // For user-defined procedures only
  int n_args;
  char *arg_names[PROC_MAX_ARGS];
  struct env *environ;
  struct parse_expr *body;

  // A pointer to a primitive procedure.
  // A primitive procedure takes as argument an array of eval_value pointers
  // and an int with the number of arguments. It returns a *eval_value.
  struct eval_value* (*primitive)(struct eval_value **, int);
};

struct eval_value {
  enum {
    EV_NUMBER,
    EV_STRING,
    EV_SYMBOL,
    EV_PROCEDURE,
  } type;

  union {
    double number;
    char *string;
    char *symbol;
    struct procedure *proc;
  } value;
};

void print_eval_value(struct eval_value *e) {
  if (e->type == EV_NUMBER) {
    printf("NUMBER %.6f", e->value.number);
  } else if (e->type == EV_STRING) {
    printf("STRING %s", e->value.string);
  } else if (e->type == EV_SYMBOL) {
    printf("SYMBOL %s", e->value.symbol);
  } else if (e->type == EV_PROCEDURE) {
    if (e->value.proc->type == PROC_PRIMITIVE) {
      printf("PRIMITIVE PROCEDURE");
    } else if (e->value.proc->type == PROC_USER_DEFINED) {
      printf("USER DEFINED PROCEDURE WITH ARGS ");
      for (int i; i < e->value.proc->n_args; i++) {
        printf("%s, ", e->value.proc->arg_names[i]);
      }
    } else {
      printf("UNKNOWN PROCEDURE TYPE");
    }
  } else {
    printf("UNKNOWN TYPE");
  }
  printf("\n");
}

struct eval_value *make_procedure(struct parse_expr *e, struct env *environ) {
  struct eval_value *value = (struct eval_value*) malloc(sizeof (struct eval_value));
  struct parse_expr *child_exp;

  struct procedure *proc = (struct procedure *) malloc(sizeof (struct procedure));

  proc->type = PROC_USER_DEFINED;
  proc->environ = environ;
  proc->n_args = 0;
  proc->body = e->children[2];

  value->type = EV_PROCEDURE;
  value->value.proc = proc;


  child_exp = e->children[1];
  for (int child_n = 0; child_n < child_exp->n_children; child_n++) {
    proc->arg_names[proc->n_args++] = child_exp->children[child_n]->value.string;
  }

  return value;
}

struct eval_value *lookup_env_var(struct env *environ, char *key) {
  if (environ == NULL) {
    fprintf(stderr, "Variable '%s' is not defined\n", key);
    exit(1);
  }

  for (struct env_entry *p = environ->entries; p; p = p->next) {
    if (strcmp2(p->key, key) == 0) {
      return p->value;
    }
  }

  return lookup_env_var(environ->parent, key);
}

struct eval_value *define_env_var(struct env *environ, char *key, struct eval_value *value) {
  struct env_entry *new_entry = malloc(sizeof (struct env_entry));
  new_entry->key = key;
  new_entry->value = value;
  new_entry->next = environ->entries;
  environ->entries = new_entry;

  return value;
}

// TODO: probably intern these babies to save memory
struct eval_value *eval_literal(struct parse_expr *e) {
    struct eval_value *v = (struct eval_value *) malloc(sizeof (struct eval_value));

    if (e->type == NUMBER) {
      v->type = EV_NUMBER;
      v->value.number = e->value.number;
    } else if (e->type == STRING) {
      v->type = EV_STRING;
      v->value.string = e->value.string;
    } else {
      fprintf(stderr, "Invalid literal type");
      exit(1);
    }
    return v;
}

struct eval_value *eval(struct parse_expr *, struct env *);

struct eval_value *eval_if(struct parse_expr *e, struct env *environ) {
  fprintf(stderr, "if expression is not implemented yet\n");
  exit(1);
  return NULL;
}

struct eval_value *eval_application(struct parse_expr *e, struct env *environ) {
  struct eval_value *proc_value = eval(e->children[0], environ);

  int n_args = e->n_children - 1;
  struct eval_value *args_values[n_args];

  struct procedure *proc = proc_value->value.proc;

  for (int child_n = 1; child_n < e->n_children; child_n++) {
    args_values[child_n - 1] = eval(e->children[child_n], environ);
  }

  if (proc->type == PROC_PRIMITIVE) {
    return proc->primitive(args_values, n_args);

  } else if (proc->type == PROC_USER_DEFINED) {

    if(n_args != proc->n_args) {
      fprintf(stderr, "Wrong number of arguments given to procedure\n");
      exit(1);
    }

    struct env *new_env = malloc(sizeof (struct env));
    new_env->parent = proc->environ;

    // Extend new env with procedure arguments names/values
    for (int i = 0; i < n_args; i++) {
      define_env_var(new_env, proc->arg_names[i], args_values[i]);
    }
    return eval(proc->body, new_env);

  } else {
    fprintf(stderr, "Invalid procedure type\n");
    exit(1);
  }
}

struct eval_value *eval(struct parse_expr *e, struct env *environ) {
  if (e->type == NUMBER || e->type == STRING) {
    return eval_literal(e);

  } else if (e->type == SYMBOL) {
    return lookup_env_var(environ, e->value.symbol);

  } else if (e->type == COMPOUND) {
    struct parse_expr *first  = e->children[0];

    if (first->type == SYMBOL) {
      // define
      if (strcmp2(first->value.symbol, "define") == 0) {
        char *key = e->children[1]->value.symbol;
        struct eval_value *value = eval(e->children[2], environ);
        return define_env_var(environ, key, value);
      }

      // begin
      else if (strcmp2(first->value.symbol, "begin") == 0) {
        struct eval_value *value;
        for (int child_n = 1; child_n < e->n_children; child_n++) {
          value = eval(e->children[child_n], environ);
        }
        return value;
      }

      // if-else
      else if (strcmp2(first->value.symbol, "if") == 0) {
        return eval_if(e, environ);
      }

      // lambda
      else if (strcmp2(first->value.symbol, "lambda") == 0) {
        return make_procedure(e, environ);
      }

      // Procedure application
      else {
        return eval_application(e, environ);
      }
    }
  }
  return NULL;
}

struct eval_value *primitive_sum(struct eval_value *args[], int n_args) {
  struct eval_value *result = (struct eval_value *) malloc(sizeof (struct eval_value));
  double sum = 0.;
  while (n_args > 0) {
    sum += args[--n_args]->value.number;
  }

  result->type = EV_NUMBER;
  result->value.number = sum;
  return result;
}

struct env *make_root_env() {
  struct env *environ = (struct env *) malloc(sizeof (struct env));
  environ->entries = NULL;

  // Install primitive procedures
  struct eval_value *value = (struct eval_value *) malloc(sizeof (struct eval_value));
  struct procedure *proc = malloc(sizeof (struct procedure));
  proc->type = PROC_PRIMITIVE;
  proc->primitive = primitive_sum;
  value->type = EV_PROCEDURE;
  value->value.proc = proc;
  define_env_var(environ, "+", value);

  return environ;

}

/*
  Debug
*/

void print_env(struct env *environ) {
  for (struct env_entry *p = environ->entries; p; p = p->next) {
    printf("%s => ", p->key);
    print_eval_value(p->value);
  }
}

void print_parse_expr(struct parse_expr *e) {
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

void print_ast(struct parse_expr *root, int indent) {
  for (int j=0; j < indent; j++)
    printf("    ");

  if (root->type == COMPOUND) {
    printf("COMPOUND\n");
    for (int i=0; i < root->n_children; i++) {
      print_ast(root->children[i], indent+1);
    }
  } else {
    print_parse_expr(root);
  }
}

void debug_eval(char *code) {
  char *tokens[MAX_TOKENS];
  int n_tokens;
  struct env *environ = make_root_env();
  struct parse_expr parsed_code;
  struct eval_value *eval_result;

  n_tokens = tokenize(code, tokens);

  printf("TOKENS (%d)\n==========\n", n_tokens);

  for (int i = 0; i < n_tokens; i++) {
    printf("%s, ", tokens[i]);
  }
  printf ("\n\n");

  parse(tokens, &parsed_code);

  printf("AST:\n====\n");
  print_ast(&parsed_code, 0);

  eval_result = eval(&parsed_code, environ);

  printf("\nRESULT:\n=======\n");
  print_eval_value(eval_result);

  printf("\nENV:\n====\n");
  print_env(environ);

  printf("\n");
}

int main(int argc, char **argv) {
  debug_eval("(begin (define my-fun (lambda (a) (+ a 1))) (my-fun 1))");
}
