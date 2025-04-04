//Code 1: recog of tokens
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int isKeyword(const char *word) {
    const char *keywords[] = {
        "int", "float", "if", "else", "while", "for", "return", "void", "char",
        "double", "break", "continue", "switch", "case", "struct"
    };
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isDelimiter(char ch) {
    char delimiters[] = " ,;(){}[]";
    return strchr(delimiters, ch) != NULL;
}

int isOperator(char ch) {
    char operators[] = "+-*/%=";
    return strchr(operators, ch) != NULL;
}

int main() {
    FILE *input = fopen("input.txt", "r");
    FILE *output = fopen("output.txt", "w");

    if (!input) {
        printf("Error opening input file.\n");
        return 1;
    }

    char ch, buffer[50];
    int index = 0;
    int insideComment = 0;

    while ((ch = fgetc(input)) != EOF) {
        if (ch == '/') {
            char next = fgetc(input);
            if (next == '/') {
                while ((ch = fgetc(input)) != EOF && ch != '\n');
                continue;
            } else if (next == '*') {
                insideComment = 1;
                while (insideComment && (ch = fgetc(input)) != EOF) {
                    if (ch == '*' && (ch = fgetc(input)) == '/') {
                        insideComment = 0;
                    }
                }
                continue;
            } else {
                ungetc(next, input);
            }
        }

        if (isalnum(ch) || ch == '_') {
            buffer[index++] = ch;
        } else {
            buffer[index] = '\0';
            index = 0;

            if (strlen(buffer) > 0) {
                if (isKeyword(buffer)) {
                    fprintf(output, "<keyword> ");
                } else if (isdigit(buffer[0])) {
                    fprintf(output, "<literal> ");
                } else {
                    fprintf(output, "<identifier> ");
                }
            }

            if (isOperator(ch)) {
                fprintf(output, "<operator> ");
            }

            if (isDelimiter(ch)) {
                fprintf(output, "<delimiter> ");
            }

            if (!isalnum(ch) && ch != ' ' && !isOperator(ch) && !isDelimiter(ch)) {
                fprintf(output, "<special_char> ");
            }
        }
    }

    fclose(input);
    fclose(output);

    output = fopen("output.txt", "r");
    if (output) {
        printf("\n--- Tokenized Output ---\n");
        char line[256];
        while (fgets(line, sizeof(line), output)) {
            printf("%s", line);
        }
        fclose(output);
    }

    return 0;
}


//code 2: left recursion

#include <stdio.h>
#include <string.h>

#define MAX_RULES 10
#define MAX_LEN 100

void eliminateLeftRecursion(char lhs, char rhs[MAX_RULES][MAX_LEN], int count) {
    char alpha[MAX_RULES][MAX_LEN], beta[MAX_RULES][MAX_LEN];
    int alphaCount = 0, betaCount = 0;

    for (int i = 0; i < count; i++) {
        if (rhs[i][0] == lhs) {
            strcpy(alpha[alphaCount++], rhs[i] + 1);
        } else {
            strcpy(beta[betaCount++], rhs[i]);
        }
    }

    if (alphaCount == 0) {
        printf("%c -> ", lhs);
        for (int i = 0; i < count; i++) {
            printf("%s%s", rhs[i], i == count - 1 ? "\n" : " | ");
        }
        return;
    }

    printf("%c -> ", lhs);
    for (int i = 0; i < betaCount; i++) {
        printf("%s%c' %s", beta[i], lhs, i == betaCount - 1 ? "\n" : "| ");
    }

    printf("%c' -> ", lhs);
    for (int i = 0; i < alphaCount; i++) {
        printf("%s%c' %s", alpha[i], lhs, i == alphaCount - 1 ? "\n" : "| ");
    }
    printf("e\n");
}

int main() {
    char lhs;
    int count;
    char rhs[MAX_RULES][MAX_LEN];

    printf("Enter LHS of production: ");
    scanf("%c", &lhs);
    getchar();

    printf("Enter number of RHS alternatives: ");
    scanf("%d", &count);
    getchar();

    for (int i = 0; i < count; i++) {
        printf("Enter RHS alternative %d: ", i + 1);
        fgets(rhs[i], MAX_LEN, stdin);
        rhs[i][strcspn(rhs[i], "\n")] = 0;
    }

    eliminateLeftRecursion(lhs, rhs, count);
    return 0;
}



//code 3: left factoring
#include <stdio.h>
#include <string.h>

#define MAX_RULES 10
#define MAX_LEN 100

void eliminateLeftFactoring(char lhs, char rhs[MAX_RULES][MAX_LEN], int count) {
    char commonPrefix[MAX_LEN];
    strcpy(commonPrefix, rhs[0]);

    for (int i = 1; i < count; i++) {
        int j = 0;
        while (commonPrefix[j] && rhs[i][j] && commonPrefix[j] == rhs[i][j]) {
            j++;
        }
        commonPrefix[j] = '\0';
    }

    if (strlen(commonPrefix) == 0) {
        printf("%c -> ", lhs);
        for (int i = 0; i < count; i++) {
            printf("%s%s", rhs[i], i == count - 1 ? "\n" : " | ");
        }
        return;
    }

    printf("%c -> %s%c'\n", lhs, commonPrefix, lhs);
    printf("%c' -> ", lhs);
    for (int i = 0; i < count; i++) {
        printf("%s%s", rhs[i] + strlen(commonPrefix), i == count - 1 ? "\n" : " | ");
    }
}

int main() {
    char lhs;
    int count;
    char rhs[MAX_RULES][MAX_LEN];

    printf("Enter LHS of production: ");
    scanf("%c", &lhs);
    getchar();

    printf("Enter number of RHS alternatives: ");
    scanf("%d", &count);
    getchar();

    for (int i = 0; i < count; i++) {
        printf("Enter RHS alternative %d: ", i + 1);
        fgets(rhs[i], MAX_LEN, stdin);
        rhs[i][strcspn(rhs[i], "\n")] = 0;
    }

    eliminateLeftFactoring(lhs, rhs, count);
    return 0;
}

//code 5: first, follow, predictive parsing table
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD 20
#define MAX_LEN  100
#define MAX_NT   20
#define MAX_TER  50
#define SET_SIZE 100

typedef struct {
    char lhs;
    char rhs[MAX_LEN];
} Production;

Production prods[MAX_PROD];
int numProds;

char nonTerminals[MAX_NT];
int numNT = 0;

char terminals[MAX_TER];
int numTerm = 0;

char FIRST[MAX_NT][SET_SIZE];
char FOLLOW[MAX_NT][SET_SIZE];

char parsingTable[MAX_NT][MAX_TER][MAX_LEN];

int inSet(char set[], char c) {
    for (int i = 0; set[i] != '\0'; i++) {
        if (set[i] == c)
            return 1;
    }
    return 0;
}

int addToSet(char set[], char c) {
    if (!inSet(set, c)) {
        int len = strlen(set);
        set[len] = c;
        set[len+1] = '\0';
        return 1;
    }
    return 0;
}

int unionSet(char dest[], const char src[]) {
    int added = 0;
    for (int i = 0; src[i] != '\0'; i++) {
        if (addToSet(dest, src[i]))
            added = 1;
    }
    return added;
}

int isNonTerminal(char c) {
    if (isupper(c)) {
        for (int i = 0; i < numNT; i++) {
            if (nonTerminals[i] == c)
                return 1;
        }
    }
    return 0;
}

int getNTIndex(char c) {
    for (int i = 0; i < numNT; i++) {
        if (nonTerminals[i] == c)
            return i;
    }
    return -1;
}

int getTermIndex(char c) {
    for (int i = 0; i < numTerm; i++) {
        if (terminals[i] == c)
            return i;
    }
    return -1;
}

void computeFirstString(const char *str, char result[]) {
    result[0] = '\0';
    if (str[0] == '\0')
        return;
    int i = 0;
    int allEpsilon = 1;
    while (str[i] != '\0' && allEpsilon) {
        allEpsilon = 0;
        char temp[SET_SIZE];
        temp[0] = '\0';
        if (isNonTerminal(str[i])) {
            int idx = getNTIndex(str[i]);
            strcat(temp, FIRST[idx]);
        } else {
            char t[2] = {str[i], '\0'};
            strcat(temp, t);
        }
        for (int j = 0; temp[j] != '\0'; j++) {
            if (temp[j] != 'ε') {
                addToSet(result, temp[j]);
            }
        }
        if (inSet(temp, 'ε')) {
            allEpsilon = 1;
            i++;
        }
    }
    if (allEpsilon)
        addToSet(result, 'ε');
}

void computeAllFIRST() {
    for (int i = 0; i < numNT; i++)
        FIRST[i][0] = '\0';
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < numProds; i++) {
            char A = prods[i].lhs;
            int A_idx = getNTIndex(A);
            char alpha[SET_SIZE];
            alpha[0] = '\0';
            computeFirstString(prods[i].rhs, alpha);
            if (unionSet(FIRST[A_idx], alpha))
                changed = 1;
        }
    }
}

void computeAllFOLLOW() {
    for (int i = 0; i < numNT; i++)
        FOLLOW[i][0] = '\0';
    addToSet(FOLLOW[0], '$');
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < numProds; i++) {
            char A = prods[i].lhs;
            int A_idx = getNTIndex(A);
            int len = strlen(prods[i].rhs);
            for (int pos = 0; pos < len; pos++) {
                char symbol = prods[i].rhs[pos];
                if (isNonTerminal(symbol)) {
                    int B_idx = getNTIndex(symbol);
                    char beta[SET_SIZE];
                    beta[0] = '\0';
                    if (pos+1 < len)
                        strncpy(beta, prods[i].rhs + pos + 1, len - pos);
                    beta[len - pos] = '\0';
                    char firstBeta[SET_SIZE];
                    firstBeta[0] = '\0';
                    computeFirstString(beta, firstBeta);
                    for (int k = 0; firstBeta[k] != '\0'; k++) {
                        if (firstBeta[k] != 'ε') {
                            if (addToSet(FOLLOW[B_idx], firstBeta[k]))
                                changed = 1;
                        }
                    }
                    if (strlen(beta) == 0 || inSet(firstBeta, 'ε')) {
                        if (unionSet(FOLLOW[B_idx], FOLLOW[A_idx]))
                            changed = 1;
                    }
                }
            }
        }
    }
}

void buildParsingTable() {
    for (int i = 0; i < numNT; i++) {
        for (int j = 0; j < numTerm; j++) {
            parsingTable[i][j][0] = '\0';
        }
    }
    for (int i = 0; i < numProds; i++) {
        char A = prods[i].lhs;
        int A_idx = getNTIndex(A);
        char firstAlpha[SET_SIZE];
        firstAlpha[0] = '\0';
        computeFirstString(prods[i].rhs, firstAlpha);
        for (int k = 0; firstAlpha[k] != '\0'; k++) {
            if (firstAlpha[k] != 'ε') {
                int termIdx = getTermIndex(firstAlpha[k]);
                if (termIdx != -1) {
                    char prodStr[MAX_LEN];
                    sprintf(prodStr, "%c->%s", prods[i].lhs, prods[i].rhs);
                    strcpy(parsingTable[A_idx][termIdx], prodStr);
                }
            }
        }
        if (inSet(firstAlpha, 'ε')) {
            for (int j = 0; j < numTerm; j++) {
                char term = terminals[j];
                if (inSet(FOLLOW[A_idx], term)) {
                    char prodStr[MAX_LEN];
                    sprintf(prodStr, "%c->%s", prods[i].lhs, prods[i].rhs);
                    strcpy(parsingTable[A_idx][j], prodStr);
                }
            }
        }
    }
}

void printParsingTable() {
    printf("\nPredictive Parsing Table:\n   |");
    for (int j = 0; j < numTerm; j++) {
        printf(" %c\t|", terminals[j]);
    }
    printf("\n");
    for (int i = 0; i < numNT; i++) {
        printf(" %c |", nonTerminals[i]);
        for (int j = 0; j < numTerm; j++) {
            if (strlen(parsingTable[i][j]) > 0)
                printf(" %s\t|", parsingTable[i][j]);
            else
                printf(" -\t|");
        }
        printf("\n");
    }
}

int main() {
    printf("Enter number of productions: ");
    scanf("%d", &numProds);
    getchar();
    printf("Enter productions (e.g., E->TA):\n");
    for (int i = 0; i < numProds; i++) {
        char line[MAX_LEN];
        scanf("%s", line);
        prods[i].lhs = line[0];
        strcpy(prods[i].rhs, line + 3);
    }
    for (int i = 0; i < numProds; i++) {
        char A = prods[i].lhs;
        if (!inSet(nonTerminals, A)) {
            nonTerminals[numNT++] = A;
            nonTerminals[numNT] = '\0';
        }
    }
    for (int i = 0; i < numProds; i++) {
        int len = strlen(prods[i].rhs);
        for (int j = 0; j < len; j++) {
            char sym = prods[i].rhs[j];
            if (sym == 'ε') continue;
            if (!isupper(sym)) {
                if (!inSet(terminals, sym)) {
                    terminals[numTerm++] = sym;
                    terminals[numTerm] = '\0';
                }
            } else if (!isNonTerminal(sym)) {
                if (!inSet(terminals, sym)) {
                    terminals[numTerm++] = sym;
                    terminals[numTerm] = '\0';
                }
            }
        }
    }
    if (!inSet(terminals, '$')) {
        terminals[numTerm++] = '$';
        terminals[numTerm] = '\0';
    }
    computeAllFIRST();
    computeAllFOLLOW();
    printf("\nFIRST sets:\n");
    for (int i = 0; i < numNT; i++) {
        printf("FIRST(%c) = { ", nonTerminals[i]);
        for (int j = 0; FIRST[i][j] != '\0'; j++) {
            printf("%c ", FIRST[i][j]);
        }
        printf("}\n");
    }
    printf("\nFOLLOW sets:\n");
    for (int i = 0; i < numNT; i++) {
        printf("FOLLOW(%c) = { ", nonTerminals[i]);
        for (int j = 0; FOLLOW[i][j] != '\0'; j++) {
            printf("%c ", FOLLOW[i][j]);
        }
        printf("}\n");
    }
    buildParsingTable();
    printParsingTable();
    return 0;
}




//code 6: compute leading and trailing
#include <stdio.h>
#include <string.h>

int nt, t, top = 0;
char s[50], NT[10], T[10], st[50], l[10][10], tr[50][50];

int searchnt(char a) {
    for (int i = 0; i < nt; i++) {
        if (NT[i] == a)
            return i;
    }
    return -1;
}

int searchter(char a) {
    for (int i = 0; i < t; i++) {
        if (T[i] == a)
            return i;
    }
    return -1;
}

void push(char a) {
    s[top++] = a;
}

char pop() {
    return s[--top];
}

void installl(int a, int b) {
    if (l[a][b] == 'f') {
        l[a][b] = 't';
        push(T[b]);
        push(NT[a]);
    }
}

void installt(int a, int b) {
    if (tr[a][b] == 'f') {
        tr[a][b] = 't';
        push(T[b]);
        push(NT[a]);
    }
}

int main() {
    int i, s, k, j, n;
    char pr[30][30], b, c;

    printf("Enter the number of productions: ");
    scanf("%d", &n);

    printf("Enter the productions one by one: \n");
    for (i = 0; i < n; i++)
        scanf("%s", pr[i]);

    nt = 0;
    t = 0;

    // Identify Non-Terminals
    for (i = 0; i < n; i++) {
        if (searchnt(pr[i][0]) == -1)
            NT[nt++] = pr[i][0];
    }

    // Identify Terminals
    for (i = 0; i < n; i++) {
        for (j = 3; j < strlen(pr[i]); j++) {
            if (searchnt(pr[i][j]) == -1) {
                if (searchter(pr[i][j]) == -1)
                    T[t++] = pr[i][j];
            }
        }
    }

    // Initialize Leading and Trailing matrices
    for (i = 0; i < nt; i++) {
        for (j = 0; j < t; j++)
            l[i][j] = 'f';
    }

    for (i = 0; i < nt; i++) {
        for (j = 0; j < t; j++)
            tr[i][j] = 'f';
    }

    // Calculate Leading
    for (i = 0; i < nt; i++) {
        for (j = 0; j < n; j++) {
            if (NT[searchnt(pr[j][0])] == NT[i]) {
                if (searchter(pr[j][3]) != -1)
                    installl(searchnt(pr[j][0]), searchter(pr[j][3]));
                else {
                    for (k = 3; k < strlen(pr[j]); k++) {
                        if (searchnt(pr[j][k]) == -1) {
                            installl(searchnt(pr[j][0]), searchter(pr[j][k]));
                            break;
                        }
                    }
                }
            }
        }
    }

    while (top != 0) {
        b = pop();
        c = pop();
        for (s = 0; s < n; s++) {
            if (pr[s][3] == b)
                installl(searchnt(pr[s][0]), searchter(c));
        }
    }

    // Print Leading Sets
    for (i = 0; i < nt; i++) {
        printf("Leading[%c]\t{", NT[i]);
        for (j = 0; j < t; j++) {
            if (l[i][j] == 't')
                printf("%c,", T[j]);
        }
        printf("}\n");
    }

    top = 0;

    // Calculate Trailing
    for (i = 0; i < nt; i++) {
        for (j = 0; j < n; j++) {
            if (NT[searchnt(pr[j][0])] == NT[i]) {
                if (searchter(pr[j][strlen(pr[j]) - 1]) != -1)
                    installt(searchnt(pr[j][0]), searchter(pr[j][strlen(pr[j]) - 1]));
                else {
                    for (k = strlen(pr[j]) - 1; k >= 3; k--) {
                        if (searchnt(pr[j][k]) == -1) {
                            installt(searchnt(pr[j][0]), searchter(pr[j][k]));
                            break;
                        }
                    }
                }
            }
        }
    }

    while (top != 0) {
        b = pop();
        c = pop();
        for (s = 0; s < n; s++) {
            if (pr[s][3] == b)
                installt(searchnt(pr[s][0]), searchter(c));
        }
    }

    // Print Trailing Sets
    for (i = 0; i < nt; i++) {
        printf("Trailing[%c]\t{", NT[i]);
        for (j = 0; j < t; j++) {
            if (tr[i][j] == 't')
                printf("%c,", T[j]);
        }
        printf("}\n");
    }

    return 0;
}


//code 7: construct operator precedence table
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD 20
#define MAX_LEN 100

typedef struct {
    char lhs;
    char rhs[MAX_LEN];
} Production;

Production prods[MAX_PROD];
int numProds;

int main(){
    int i, j, valid = 1;
    scanf("%d", &numProds);
    for(i = 0; i < numProds; i++){
        char line[MAX_LEN];
        scanf("%s", line);
        prods[i].lhs = line[0];
        strcpy(prods[i].rhs, line + 3);
    }
    for(i = 0; i < numProds; i++){
        if(strcmp(prods[i].rhs, "ε") == 0){
            valid = 0;
            break;
        }
        int len = strlen(prods[i].rhs);
        for(j = 0; j < len - 1; j++){
            if(isupper(prods[i].rhs[j]) && isupper(prods[i].rhs[j+1])){
                valid = 0;
                break;
            }
        }
        if(!valid) break;
    }
    if(valid)
        printf("Grammar can be used to construct an OPT.\n");
    else
        printf("Grammar cannot be used to construct an OPT.\n");
    return 0;
}


//code 8: construct operator precedence function table
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD 20
#define MAX_LEN 100
#define MAX_NT 20
#define MAX_TER 50
#define SET_SIZE 100

typedef struct {
    char lhs;
    char rhs[MAX_LEN];
} Production;

Production prods[MAX_PROD];
int numProds;

char nonTerminals[MAX_NT];
int numNT = 0;

char terminals[MAX_TER];
int numTerm = 0;

char LEAD[MAX_NT][SET_SIZE];
char TRAIL[MAX_NT][SET_SIZE];

char OPTable[MAX_TER][MAX_TER];

int inSet(char set[], char c) {
    for (int i = 0; set[i] != '\0'; i++)
        if (set[i] == c)
            return 1;
    return 0;
}

int addToSet(char set[], char c) {
    if (!inSet(set, c)) {
        int len = strlen(set);
        set[len] = c;
        set[len+1] = '\0';
        return 1;
    }
    return 0;
}

int unionSet(char dest[], const char src[]) {
    int added = 0;
    for (int i = 0; src[i] != '\0'; i++) {
        if (addToSet(dest, src[i]))
            added = 1;
    }
    return added;
}

int isNonTerminal(char c) {
    if (isupper(c)) {
        for (int i = 0; i < numNT; i++)
            if (nonTerminals[i] == c)
                return 1;
    }
    return 0;
}

int getNTIndex(char c) {
    for (int i = 0; i < numNT; i++)
        if (nonTerminals[i] == c)
            return i;
    return -1;
}

int getTermIndex(char c) {
    for (int i = 0; i < numTerm; i++)
        if (terminals[i] == c)
            return i;
    return -1;
}

void computeAllLeading() {
    for (int i = 0; i < numNT; i++)
        LEAD[i][0] = '\0';
    int changed = 1;
    while(changed) {
        changed = 0;
        for (int i = 0; i < numProds; i++) {
            char A = prods[i].lhs;
            int A_idx = getNTIndex(A);
            int len = strlen(prods[i].rhs);
            if(len == 0) continue;
            char first = prods[i].rhs[0];
            if (!isNonTerminal(first)) {
                if(addToSet(LEAD[A_idx], first))
                    changed = 1;
            } else {
                int idx = getNTIndex(first);
                if(unionSet(LEAD[A_idx], LEAD[idx]))
                    changed = 1;
                if(len > 1 && !isNonTerminal(prods[i].rhs[1])) {
                    if(addToSet(LEAD[A_idx], prods[i].rhs[1]))
                        changed = 1;
                }
            }
        }
    }
}

void computeAllTrailing() {
    for (int i = 0; i < numNT; i++)
        TRAIL[i][0] = '\0';
    int changed = 1;
    while(changed) {
        changed = 0;
        for (int i = 0; i < numProds; i++) {
            char A = prods[i].lhs;
            int A_idx = getNTIndex(A);
            int len = strlen(prods[i].rhs);
            if(len == 0) continue;
            char last = prods[i].rhs[len-1];
            if (!isNonTerminal(last)) {
                if(addToSet(TRAIL[A_idx], last))
                    changed = 1;
            } else {
                int idx = getNTIndex(last);
                if(unionSet(TRAIL[A_idx], TRAIL[idx]))
                    changed = 1;
                if(len > 1 && !isNonTerminal(prods[i].rhs[len-2])) {
                    if(addToSet(TRAIL[A_idx], prods[i].rhs[len-2]))
                        changed = 1;
                }
            }
        }
    }
}

void initOPTable() {
    for (int i = 0; i < numTerm; i++)
        for (int j = 0; j < numTerm; j++)
            OPTable[i][j] = ' ';
}

void buildOPTable() {
    initOPTable();
    for (int i = 0; i < numProds; i++) {
        int len = strlen(prods[i].rhs);
        for (int j = 0; j < len - 1; j++) {
            char a = prods[i].rhs[j];
            char b = prods[i].rhs[j+1];
            if (!isNonTerminal(a) && !isNonTerminal(b)) {
                int idxa = getTermIndex(a);
                int idxb = getTermIndex(b);
                if(idxa != -1 && idxb != -1)
                    OPTable[idxa][idxb] = '=';
            }
            if (!isNonTerminal(a) && isNonTerminal(b)) {
                int idxa = getTermIndex(a);
                int ntb = getNTIndex(b);
                if(idxa != -1 && ntb != -1) {
                    for (int k = 0; LEAD[ntb][k] != '\0'; k++) {
                        int idxb = getTermIndex(LEAD[ntb][k]);
                        if(idxb != -1)
                            OPTable[idxa][idxb] = '<';
                    }
                }
            }
            if (isNonTerminal(a) && !isNonTerminal(b)) {
                int nta = getNTIndex(a);
                int idxb = getTermIndex(b);
                if(nta != -1 && idxb != -1) {
                    for (int k = 0; TRAIL[nta][k] != '\0'; k++) {
                        int idxa = getTermIndex(TRAIL[nta][k]);
                        if(idxa != -1)
                            OPTable[idxa][idxb] = '>';
                    }
                }
            }
            if (j < len - 2) {
                char a1 = prods[i].rhs[j];
                char B = prods[i].rhs[j+1];
                char b = prods[i].rhs[j+2];
                if (!isNonTerminal(a1) && isNonTerminal(B) && !isNonTerminal(b)) {
                    int idxa = getTermIndex(a1);
                    int idxb = getTermIndex(b);
                    if(idxa != -1 && idxb != -1)
                        OPTable[idxa][idxb] = '=';
                }
            }
        }
    }
}

void printOPTable() {
    printf("\nOperator Precedence Table:\n   |");
    for (int j = 0; j < numTerm; j++) {
        printf(" %c\t|", terminals[j]);
    }
    printf("\n");
    for (int i = 0; i < numTerm; i++) {
        printf(" %c |", terminals[i]);
        for (int j = 0; j < numTerm; j++) {
            if (OPTable[i][j] != ' ')
                printf(" %c\t|", OPTable[i][j]);
            else
                printf("  \t|");
        }
        printf("\n");
    }
}

void printSets() {
    printf("\nLeading Sets:\n");
    for (int i = 0; i < numNT; i++) {
        printf("LEAD(%c) = { ", nonTerminals[i]);
        for (int j = 0; LEAD[i][j] != '\0'; j++)
            printf("%c ", LEAD[i][j]);
        printf("}\n");
    }
    printf("\nTrailing Sets:\n");
    for (int i = 0; i < numNT; i++) {
        printf("TRAIL(%c) = { ", nonTerminals[i]);
        for (int j = 0; TRAIL[i][j] != '\0'; j++)
            printf("%c ", TRAIL[i][j]);
        printf("}\n");
    }
}

int main() {
    printf("Enter number of productions: ");
    scanf("%d", &numProds);
    getchar();
    for (int i = 0; i < numProds; i++) {
        char line[MAX_LEN];
        scanf("%s", line);
        prods[i].lhs = line[0];
        strcpy(prods[i].rhs, line + 3);
    }
    for (int i = 0; i < numProds; i++) {
        char A = prods[i].lhs;
        if (!inSet(nonTerminals, A)) {
            nonTerminals[numNT++] = A;
            nonTerminals[numNT] = '\0';
        }
    }
    for (int i = 0; i < numProds; i++) {
        int len = strlen(prods[i].rhs);
        for (int j = 0; j < len; j++) {
            char sym = prods[i].rhs[j];
            if (!isupper(sym)) {
                if (!inSet(terminals, sym)) {
                    terminals[numTerm++] = sym;
                    terminals[numTerm] = '\0';
                }
            } else if (!inSet(nonTerminals, sym)) {
                if (!inSet(terminals, sym)) {
                    terminals[numTerm++] = sym;
                    terminals[numTerm] = '\0';
                }
            }
        }
    }
    if (!inSet(terminals, '$')) {
        terminals[numTerm++] = '$';
        terminals[numTerm] = '\0';
    }
    computeAllLeading();
    computeAllTrailing();
    printSets();
    buildOPTable();
    printOPTable();
    return 0;
}

// code 9: Conversion to 3AC:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 256

int tempCount = 0;
char* newTemp() {
    char *temp = (char*)malloc(10);
    sprintf(temp, "t%d", tempCount++);
    return temp;
}

void generate3AC(char *line) {
    char lhs[50], expr[200];
    if(sscanf(line, "%[^=]=%[^\n]", lhs, expr) == 2) {
        int n = strlen(expr);
        char postfix[1000] = "";
        char opStack[1000];
        int opTop = -1;
        for (int i = 0; i < n; i++) {
            if(isspace(expr[i])) continue;
            if(isalnum(expr[i])) {
                strncat(postfix, &expr[i], 1);
            } else {
                while(opTop >= 0 && (((expr[i] == '+' || expr[i] == '-') && 
                      (opStack[opTop] == '+' || opStack[opTop] == '-' || opStack[opTop] == '*' || opStack[opTop] == '/')) ||
                      ((expr[i] == '*' || expr[i] == '/') && 
                      (opStack[opTop] == '*' || opStack[opTop] == '/')))) {
                    char op[2] = {opStack[opTop--], '\0'};
                    strcat(postfix, op);
                }
                opStack[++opTop] = expr[i];
            }
        }
        while(opTop >= 0) {
            char op[2] = {opStack[opTop--], '\0'};
            strcat(postfix, op);
        }
        char *stack[100];
        int top = -1;
        for (int i = 0; i < strlen(postfix); i++) {
            char ch = postfix[i];
            if(isalnum(ch)) {
                char *temp = (char*)malloc(2);
                temp[0] = ch;
                temp[1] = '\0';
                stack[++top] = temp;
            } else {
                char *op2 = stack[top--];
                char *op1 = stack[top--];
                char *tempVar = newTemp();
                printf("%s = %s %c %s\n", tempVar, op1, ch, op2);
                stack[++top] = tempVar;
            }
        }
        if(top == 0) {
            printf("%s = %s\n", lhs, stack[top]);
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp = fopen("file.c", "r");
    if(fp == NULL) return 1;
    char line[MAX_LINE];
    while(fgets(line, MAX_LINE, fp)) {
        if(strchr(line, '=') && strchr(line, ';')) {
            generate3AC(line);
        }
    }
    fclose(fp);
    return 0;
}
