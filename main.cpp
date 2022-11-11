#include <iostream>
#include <ctype.h>
#include <string>
#include <vector>
#include <stack>
#include <map>
using namespace std;

/* 전역 변수 선언 */
int charClass;
char lexeme[100]; // 어휘를 저장할 배열
vector<char> warning; // 경고를 발생시킨 문자를 저장할 배열
char next_char; // 다음 문자
int lexLen; // lexeme배열의 인덱스 및 길이
int token; // 현재 토큰
int next_token; // 다음 토큰
char* token_string = new char; // lexeme을 저장할 문자열 변수
FILE* in_fp; // 파일을 받아올 객체
int id = 0, op = 0, num = 0; // 식별자, 연산자, 상수의 개수를 저장하기 위한 변수
map<string, int> result; // 변수의 값을 저장할 map
string operand; // 값을 할당 받아야 할 식별자
string error_op; // 참조되지 않은 식별자
int paren = 0; // 괄호의 짝이 맞는지 계산하기 위한 변수
bool error = false; // 에러 발생 여부를 체크할 변수
int sum = 0; // 계산한 값을 저장할 변수
stack<int> number; // 식별자나 상수를 읽었을 때 값을 저장하거나 계산할 값을 저장할 스택

/* 함수 선언 */
int lookup(char ch);
void addChar();
void getChar();
void getNonBlank();
int lexical();
void program();
void statements();
void statement();
void expression();
void term();
void term_tail();
void factor();
void factor_tail();

/* 문자 클래스 */
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

/* 토큰 정의 */
#define CONST 10
#define IDENT 11
#define SEMI_COLON 19
#define ASSIGN_OP 20
#define ADD_OP 21
#define MIN_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26

#define UNDEFINE 999


int main() {
    char* name = new char; // 파일명
    cout << "Enter File name: ";
    cin >> name;

    if ((in_fp = fopen(name, "r")) == NULL) // 파일이 없는 경우
        cout << "ERROR - cannot open\n" << endl;
    else { // 파일이 있는 경우
        getChar();
        do {
            program();
        } while (next_token != EOF);
        fclose(in_fp);

        // 식별자별 값 출력
        cout << "Result ==> ";
        for (auto u = result.begin(); u != result.end(); u++) {
            if (u->second >= UNDEFINE) { // 식별자의 값을 알 수 없는 경우
                if (u == --result.end())
                    cout << u->first << ": Unknown";
                else
                    cout << u->first << ": Unknown; ";
            }
            else {
                if (u == --result.end())
                    cout << u->first << ": " << u->second;
                else
                    cout << u->first << ": " << u->second << "; ";
            }

        }
    }
    return 0;
}

/*연산자와 괄호를 조사하여 그 토큰을 반환*/
int lookup(char ch) {
    switch (ch) {
    case '(':
        addChar();
        next_token = LEFT_PAREN;
        break;
    case ')':
        addChar();
        next_token = RIGHT_PAREN;
        break;
    case '+':
        addChar();
        next_token = ADD_OP;
        op++;
        break;
    case '-':
        addChar();
        next_token = MIN_OP;
        op++;
        break;
    case '*':
        addChar();
        next_token = MULT_OP;
        op++;
        break;
    case '/':
        addChar();
        next_token = DIV_OP;
        op++;
        break;
    case ':':
        addChar();
        getChar();
        if (to_string(next_char).compare("=")) { // 콜론 다음 문자가 등호라면 lexeme 배열에 추가 후 ASSIGN_OP 토큰 부여
            addChar();
            next_token = ASSIGN_OP;
        }
        break;
    case ';':
        addChar();
        next_token = SEMI_COLON;
        break;

    default:
        addChar();
        next_token = EOF;
        break;
    }
    return next_token;
}

/*next_string을 lexeme배열에 저장*/
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen] = next_char;
        lexeme[++lexLen] = 0;
    }
    else
        cout << "Error - lexeme is too long" << endl;
}

/*입력으로부터 다음 문자를 가져와서 그 문자의 유형을 결정*/
void getChar() {
    if ((next_char = getc(in_fp)) != EOF) {
        if (isalpha(next_char)) {
            charClass = LETTER;
        }
        else if (isdigit(next_char)) {
            charClass = DIGIT;
        }
        else charClass = UNKNOWN;
    }
    else charClass = EOF;
}

/*공백이 아닌 문자가 나올 때까지 getChar호출*/
void getNonBlank() {
    while (isspace(next_char)) {
        getChar();
    }
}

/*입력스트림을 분석하여 lexeme을 찾아낸 뒤 tokentype을 next_token에 저장*/
int lexical() {
    lexLen = 0;
    getNonBlank();
    token = next_token;
    switch (charClass) {
    case LETTER:
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT) {
            addChar();
            getChar();
        }
        next_token = IDENT;
        id++;
        break;
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT) {
            addChar();
            getChar();
        }
        next_token = CONST;
        num++;
        break;
    case UNKNOWN:
        lookup(next_char);
        getChar();
        break;
    case EOF:
        next_token = EOF;
        break;
    }

    /* 중복연산자인 경우 op의 수를 줄이고 중복된 연산자를 warning 배열에 저장하고 삭제 */
    if ((token == ADD_OP || token == MIN_OP || token == MULT_OP || token == DIV_OP)
        && (next_token == ADD_OP || next_token == MIN_OP || next_token == MULT_OP || next_token == DIV_OP)) {
        warning.push_back(lexeme[--lexLen]);
        lexeme[lexLen] = 0;
        op--;
    }

    /* 다음 토큰이 파일 끝을 가리키면 다음과 같이 출력 */
    if (next_token == EOF) {
        cout << "\nID:" << id << "; CONST:" << num << "; OP:" << op << ";" << endl; // 식별자, 상수, 연산자 개수 출력
        if (!warning.empty()) { // 중복 연산자가 있는 경우
            cout << "(Warning) \"remove redundant operator(" << *warning.begin() << ")\"" << endl;
        }
        else if (error == true) { // 참조되지 않은 변수가 있는 경우
            cout << "(Error) \"Undefined variable referenced(" << error_op << ")\"" << endl;
        }
        else if (paren != 0) { // 괄호의 짝이 맞지 않은 경우
            cout << "(Error) \"The parentheses do not match.\"" << endl;
        }
        else {// 그 외의 결과 출력
            cout << "(OK)" << endl;
        }
    }

    /* token_string에 lexeme 문자열을 저장*/
    token_string = lexeme;
    /*다음 토큰이 끝을 가리키지 않는다면 token_string을 출력*/
    if (next_token != EOF)
        cout << token_string;

    return next_token;
}

/*<program> -> <statements>*/
void program() {
    statements();
}

/*<statements> -> <statement><semi_colon><statements>*/
void statements() {
    statement();/* 다음 토큰이 세미콜론이나 끝이라면 오류는 해당 연산자에 0을 그 외의 연산자는 계산한 값을 저장*/
    if (next_token == SEMI_COLON) { // 다음 토큰이 세미콜론이면 현재 읽은 라인에 대한 식별자, 연산자, 상수의 개수를 출력
        cout << "\nID:" << id << "; CONST:" << num << "; OP:" << op << ";" << endl;
        if (!warning.empty()) {
            cout << "(Warning) \"Remove duplicate operator(" << *(warning.end() - 1) << ")\"" << endl;
        }
        else if (error == true) {
            cout << "(Error) \"Undefined variable referenced(" << error_op << ")\"" << endl;
        }
        else if (paren != 0) {
            cout << "(Error) \"The parentheses do not match.\"" << endl;

        }
        else {// 그 외의 결과 출력
            cout << "(OK)" << endl;
        }

        if (!error_op.empty() || paren != 0)
            result.find(operand)->second = UNDEFINE;
        else
            result.find(operand)->second = sum;

        // 식별자, 연산자, 상수 개수 초기화
        id = 0;
        op = 0;
        num = 0;

        // 오류 여부, 계산 결과, 괄호 오류 여부 초기화
        error = false;
        sum = 0;
        paren = 0;

        // 중복연산자에 대한 스택 비우기
        while (!warning.empty()) warning.pop_back();

        // 다음 토큰이 세미콜론이라면 statements 호출
        statements();
    }


    if (next_token == EOF) {
        if (!error_op.empty() || paren != 0)
            result.find(operand)->second = UNDEFINE;
        else
            result.find(operand)->second = sum;
    }
}

/* <statement> -> <assign_op><expression> */
void statement() {
    //문장의 시작이 식별자라면 변수명을 저장하고 0으로 초기화한다. 그리고 해당 문자를 operand에 저장한다. 
    if (next_token == IDENT) {
        result.insert({ token_string, UNDEFINE });
        operand = token_string;
    }
    //다음 문자를 가져오기 위해 lexical을 호출
    lexical();
    if (next_token == ASSIGN_OP) { // 다음 토큰이 assign_op라면 다음 문자를 불러오고 expression 호출
        lexical();
        expression();

        sum = number.top(); //마지막 계산 결과가 저장됨.
    }
}

/* <expression> -> <term><term_tail> */
void expression() {
    term();
    term_tail();
}

/* <term> -> <factor><factor_tail> */
void term() {
    factor();
    factor_tail();
}

/* <term_tail> -> <add_op><term><term_tail> | ε*/
void term_tail() {
    while (next_token == ADD_OP || next_token == MIN_OP) { // 다음 토큰이 ADD_OP나 MIN_OP라면 반복
        if ((token == ADD_OP || token == MIN_OP) && token == next_token) { // 중복연산자인 경우에는 계산을 하지 않고 파싱만 함.
            lexical();
            term();
            term_tail();
        }
        else if (next_token == ADD_OP) {
            lexical();
            term();
            term_tail();
            sum = number.top();
            number.pop();
            sum += number.top();
            number.pop();
            number.push(sum);
        }
        else if (next_token == MIN_OP) {
            lexical();
            term();
            term_tail();
            sum = number.top();
            number.pop();
            sum = number.top() - sum;
            number.pop();
            number.push(sum);
        }
        if (number.empty()) { // number 배열이 비어있더라도 모든 token_string을 출력하기 위해 parsing함.
            lexical();
            term();
            term_tail();
        }
    }
    return;
}

/* <factor> -> <left_paren><expression><right_paren> | <ident> | <const> */
void factor() {
    if (next_token == IDENT || next_token == CONST) {
        if (next_token == CONST) { // 다음 토큰이 CONST인 경우 token_string을 상수로 변환하여 스택에 저장.
            number.push(atoi(token_string));
        }
        else {
            if (result.find(token_string) != result.end()) {
                if (result.find(token_string)->second >= UNDEFINE) { // 테이블에는 있지만 값이 0이므로 정의되지 않은 피연산자를 참조하였음을 나타냄.
                    result.find(operand)->second = UNDEFINE;         // 그러므로 현재 계산 중인 피연산자는 값을 정의할 수 없어 0으로 저장.
                    number.push(UNDEFINE);
                }
                else
                    number.push(result.find(token_string)->second);
            }
            else if (result.find(token_string) == result.end()) { // 해당 식별자가 정의되지 않은 경우
                error = true; // 오류임을 표시
                error_op = token_string; // 오류를 야기한 변수명을 error_op에 저장.
                result.insert({ token_string, UNDEFINE }); // 정의되지 않은 변수를 저장.
                number.push(UNDEFINE); // 스택에 0 저장.
            }
        }
        lexical();
    }

    // <factor> -> <left_paren><expression><right_paren>
    else
        while (next_token == LEFT_PAREN) {
            paren++;
            lexical();
            expression();
            while (next_token == RIGHT_PAREN) {
                paren--;
                lexical();
            }
        }
}

/* <factor_tail> -> <mult_op><factor><factor_tail> | ε*/
void factor_tail() {
    while (next_token == MULT_OP || next_token == DIV_OP) { // 다음 토큰이 MULT_OP나 DIV_OP라면 반복
        if ((token == MULT_OP || token == DIV_OP) && token == next_token) { // 중복연산자인 경우에는 계산을 하지 않고 파싱만 함.
            lexical();
            factor();
            factor_tail();
        }
        else if (next_token == MULT_OP) {
            sum = number.top(); // 가장 위에 있는 값을 sum에 저장한 후,
            number.pop(); //  스택에서 제거.
            lexical();
            factor();
            factor_tail();
            sum *= number.top(); //위에서 파싱하며 저장된 수를 위에서 sum에 저장한 값과 계산.
            number.pop(); // 위에 저장된 수를 제거한 후
            number.push(sum); // 계산 결과를 스택에 저장.
        }
        else if (next_token == DIV_OP) { // 다음 토큰이 MULT_OP인 경우와 비슷한 방식으로 계산
            sum = number.top();
            number.pop();
            lexical();
            factor();
            factor_tail();
            sum /= number.top();
            number.pop();
            number.push(sum);
        }
        else if (number.empty()) { // number 배열이 비어있더라도 모든 token_string을 출력하기 위해 parsing함.
            lexical();
            factor();
            factor_tail();
        }
    }
    return;
}
