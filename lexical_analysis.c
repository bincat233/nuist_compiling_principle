#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REGNUM 8 //正则表达式总数
#define LINE_BUFFER_SIZE 1024
#define DEBUG 0

// ----------数据结构----------
//定义token中的类型
typedef enum { KEY = 0, ID, NUM, CHAR, STR, AI, DELIM, OP } TokenType;
char *find_typename[REGNUM] = {"KEY", "ID", "NUM",   "CHAR",
                               "STR", "AI", "DELIM", "OP"};
char *get_typename(TokenType t) { return find_typename[t]; }
// token
typedef struct {
  TokenType type;
  char *value;
} TokenRecord;

const char *patterns[REGNUM] = {
    "^(auto|break|case|char|const|continue|default|do|double|"
    "else|enum|extern|float|for|goto|if|int|long|register|return|short|"
    "signed|sizeof|static|struct|switch|typedef|union|unsigned|void|"
    "volatile|while|inline|restrict|_Bool|_Complex|_Imaginary|_Alignas|_"
    "Alignof|_Atomic|_Static_assert|_Noreturn|_Thread_local|_Generic)([[:"
    "space:]]|[[:punct:]])", // 0关键字, 超前搜索
    "^([[:alpha:]][[:alnum:]]*)([[:space:]]|[[:punct:]])", // 1标识符,
                                                           // 超前搜索
    "^([[:digit:]]+\\.?[[:digit:]]*)([[:space:]]|[!><+*&|/%=,;)-])", // 2数字常量
    "^('\\\\?.')",                           // 3字符常量
    "^(\".*\")",                             // 4字符串常量
    "^(\\[.*\\])",                           // 5数组下标
    "^([,;(){}]|\\]|\\[)",                   // 6界符
    "^([!><+*&|/%=-]=?|--|&&|\\+\\+|\\|\\|)" // 7操作符,
                                             // 注意减号要转义
};

// ----------Token处理相关函数----------
// 最终用来存放词法分析结果Tokens的动态数组
// 愿指针之神眷顾祂的每一个孩子, 愿普天下每个指针都有家可归, Amen.
TokenRecord **tokens;
// 已添加的Token计数
int tokens_count = 0;
// 存放Tokens数组的大小;
int tokens_size = 256;
// 创建数组
void init_tokens() {
  tokens = (TokenRecord **)malloc(tokens_size * sizeof(void *));
}
int get_tokens_size() { return tokens_size; }
// 添加Token
void add_token(TokenRecord *token) {
  //满则2倍
  if (tokens_count == tokens_size) {
#if DEBUG
    printf("DEBUG: tokens size and count: %d, realloc to %d\n", tokens_size,
           tokens_size << 1);
#endif
    tokens_size <<= 1;
    tokens = (TokenRecord **)realloc(tokens, tokens_size * sizeof(void *));
#if DEBUG
    printf("DEBUG: tokens value (array address) is %p, *tokens (first struct "
           "arrdess) is %p\n",
           tokens, *tokens);
#endif
  }
  tokens[tokens_count++] = token; //存入
}
void print_token(TokenRecord *t) {
  if (t != NULL && t->value != NULL)
    printf("TYPE: %s, VALUE: %s\n", find_typename[t->type], t->value);
}
int iterate_cnt = 0;
void reset_token_iterator() { iterate_cnt = 0; }
bool has_token() { return iterate_cnt != tokens_count; }
TokenRecord *next_token() {
  if (!has_token())
    return NULL;
  TokenRecord *ret = tokens[iterate_cnt];
  iterate_cnt++;
  return ret;
}
void print_tokens() {
  for (int i = 0; i < tokens_count; i++)
    print_token(tokens[i]);
}
void clean_tokens() {
  for (int i = 0; i < tokens_count; i++)
    free(tokens[i]);
}
// ----------核心部分:get_token()----------
// TODO: 可以用^(?:[[:space:]]*)吃掉空格
// TODO
/* 匹配结果数组第一个元素(下标0)存放整体的匹配结果.
 * 剩下的元素依次存放各子式的匹配结果.
 * 我们需要的匹配结果由第1个子式定义,
 * 所以除下标1外, 数组的其它部分可以忽略*/

//寻找字符串开头匹配到的优先级最高的Token
int get_token(TokenRecord *pt, char *src, regex_t *reg_p) {
#if DEBUG
  puts("DEBUG: get_token()");
#endif
  if (src[0] == '\0')
    return 0;
  regmatch_t match_results[2]; //匹配结果数组
  int i;
  for (i = 0; i < REGNUM; i++) {                       //依次匹配正则式
    if (!regexec(reg_p + i, src, 2, match_results, 0)) //匹配, 成功则跳出
      break;
  }
  if (i >= REGNUM) { //失败
    return 0;
  }
  int start_at = match_results[1].rm_so; //结果是第一个子式的结果
  int end_at = match_results[1].rm_eo;
  int length = end_at - start_at;
#if DEBUG
  printf("DEBUG: match %s start at %d, end at %d\n", find_typename[i], start_at,
         end_at);
#endif
  char *pstr = malloc(length + 1); //算上\0的空间
  strncpy(pstr, src + start_at, length);
  pstr[length] = '\0'; //添\0
  pt->type = i;        //跳出时的下标, 即正则式下标, 对应类型
  pt->value = pstr;
  return end_at;
}

// ----------文件读写相关函数----------
FILE *file_ptr;
void init_input(char *path) {
  //打开文件
  file_ptr = path ? fopen(path, "r") : stdin;
}
char *next_line(char *linebuffer) {
  return fgets(linebuffer, LINE_BUFFER_SIZE, file_ptr);
}

bool in_comment = false;
//吃掉没有用的字符
int eat_inlinecomment_blank(char *head) {
  int i = 0;
  //吃掉空格
  if (!in_comment) { //寻找注释结束处
    while (isblank(head[i]))
      i++;
    if (head[i] == '/' && head[i + 1] == '*') { //进入注释块
      i += 2;
      in_comment = true;
#if DEBUG
      printf("DEBUG: In the comment!\n");
#endif
    } else if ((head[i] == '/' && head[i + 1] == '/') || (head[i] == '#'))
      //跳过整行
      i = strlen(head);
  }
  if (in_comment) {
    i++;
    while (head[i] != '\0' && (head[i - 1] != '*' || head[i] != '/'))
      i++;
    if (head[i] != '\0') {
      in_comment = false; //找到
      i++;
#if DEBUG
      printf("DEBUG: Now the i-1: %c, i: %c. Out the comment!\n", head[i - 1],
             head[i]);
#endif
    }
  }
#if DEBUG
  printf("DEBUG: Eaten %d useless char.\n", i);
#endif

  return i;
}

void analysis() {
  char linebuffer[LINE_BUFFER_SIZE];
  //待匹配行, 从文件读取
  regex_t compiled_regexs[REGNUM]; // 编译后的正则式
  int flag = REG_EXTENDED;
  bool cfail;
  int i;
  for (i = 0; i < REGNUM; i++) {
    cfail = regcomp(compiled_regexs + i, patterns[i], flag); //依次编译
    if (cfail)
      break;
  }
#if DEBUG
  printf("DEBUG: Regex %s [%d]\n", cfail ? "compile failed!" : "compiled!", i);
#endif
  //读入一行直到EOF
  while (next_line(linebuffer)) {
    TokenRecord *token_p;
#if DEBUG
    puts("----------------------------------------");
    puts("DEBUG: In the loop and read one line: ");
    printf("=>%s", linebuffer);
#endif
    char *head = linebuffer; //每次匹配的开始处
    //在行内连续匹配token直到匹配不到东西
    while (true) {
#if DEBUG
      puts("--------------------");
      puts("DEBUG: In the inner loop. Now the head pointer at:");
      printf("->%s", head);
#endif
      head += eat_inlinecomment_blank(head);
      if (*head == '\n') { //一行结束, 跳出
#if DEBUG
        puts("DEBUG: break inner loop!");
#endif
        break;
      }
      token_p = (TokenRecord *)malloc(sizeof(TokenRecord));
      int eaten = get_token(token_p, head, compiled_regexs);
#if DEBUG
      printf("DEBUG: %d char matched!\n", eaten);
#endif
      if (!eaten) {
#if DEBUG
        puts("DEBUG: Error!!!");
#endif
        //释放最后一个匹配失败的Token
        free(token_p);
        break; //错误, 跳出
      }
      head += eaten; //前进匹配到的字符数
      add_token(token_p);
      // tokens[tokens_count++] = token_p; //存入
    }
  }
  for (int i = 0; i < REGNUM; i++)
    regfree(compiled_regexs + 0); //释放编译后的正则
}

void do_analysis(char *path) {
  init_input(path);
  init_tokens();
  analysis();
  print_tokens();
  clean_tokens();
}
