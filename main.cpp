#include <iostream>
#include <ctype.h>
#include <string>
#include <vector>
#include <stack>
#include <map>
using namespace std;

/* ���� ���� ���� */
int charClass;
char lexeme[100]; // ���ָ� ������ �迭
vector<char> warning; // ��� �߻���Ų ���ڸ� ������ �迭
char next_char; // ���� ����
int lexLen; // lexeme�迭�� �ε��� �� ����
int token; // ���� ��ū
int next_token; // ���� ��ū
char* token_string = new char; // lexeme�� ������ ���ڿ� ����
FILE* in_fp; // ������ �޾ƿ� ��ü
int id = 0, op = 0, num = 0; // �ĺ���, ������, ����� ������ �����ϱ� ���� ����
map<string, int> result; // ������ ���� ������ map
string operand; // ���� �Ҵ� �޾ƾ� �� �ĺ���
string error_op; // �������� ���� �ĺ���
int paren = 0; // ��ȣ�� ¦�� �´��� ����ϱ� ���� ����
bool error = false; // ���� �߻� ���θ� üũ�� ����
int sum = 0; // ����� ���� ������ ����
stack<int> number; // �ĺ��ڳ� ����� �о��� �� ���� �����ϰų� ����� ���� ������ ����

/* �Լ� ���� */
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

/* ���� Ŭ���� */
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

/* ��ū ���� */
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
    char* name = new char; // ���ϸ�
    cout << "Enter File name: ";
    cin >> name;

    if ((in_fp = fopen(name, "r")) == NULL) // ������ ���� ���
        cout << "ERROR - cannot open\n" << endl;
    else { // ������ �ִ� ���
        getChar();
        do {
            program();
        } while (next_token != EOF);
        fclose(in_fp);

        // �ĺ��ں� �� ���
        cout << "Result ==> ";
        for (auto u = result.begin(); u != result.end(); u++) {
            if (u->second >= UNDEFINE) { // �ĺ����� ���� �� �� ���� ���
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

/*�����ڿ� ��ȣ�� �����Ͽ� �� ��ū�� ��ȯ*/
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
        if (to_string(next_char).compare("=")) { // �ݷ� ���� ���ڰ� ��ȣ��� lexeme �迭�� �߰� �� ASSIGN_OP ��ū �ο�
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

/*next_string�� lexeme�迭�� ����*/
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen] = next_char;
        lexeme[++lexLen] = 0;
    }
    else
        cout << "Error - lexeme is too long" << endl;
}

/*�Է����κ��� ���� ���ڸ� �����ͼ� �� ������ ������ ����*/
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

/*������ �ƴ� ���ڰ� ���� ������ getCharȣ��*/
void getNonBlank() {
    while (isspace(next_char)) {
        getChar();
    }
}

/*�Է½�Ʈ���� �м��Ͽ� lexeme�� ã�Ƴ� �� tokentype�� next_token�� ����*/
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

    /* �ߺ��������� ��� op�� ���� ���̰� �ߺ��� �����ڸ� warning �迭�� �����ϰ� ���� */
    if ((token == ADD_OP || token == MIN_OP || token == MULT_OP || token == DIV_OP)
        && (next_token == ADD_OP || next_token == MIN_OP || next_token == MULT_OP || next_token == DIV_OP)) {
        warning.push_back(lexeme[--lexLen]);
        lexeme[lexLen] = 0;
        op--;
    }

    /* ���� ��ū�� ���� ���� ����Ű�� ������ ���� ��� */
    if (next_token == EOF) {
        cout << "\nID:" << id << "; CONST:" << num << "; OP:" << op << ";" << endl; // �ĺ���, ���, ������ ���� ���
        if (!warning.empty()) { // �ߺ� �����ڰ� �ִ� ���
            cout << "(Warning) \"remove redundant operator(" << *warning.begin() << ")\"" << endl;
        }
        else if (error == true) { // �������� ���� ������ �ִ� ���
            cout << "(Error) \"Undefined variable referenced(" << error_op << ")\"" << endl;
        }
        else if (paren != 0) { // ��ȣ�� ¦�� ���� ���� ���
            cout << "(Error) \"The parentheses do not match.\"" << endl;
        }
        else {// �� ���� ��� ���
            cout << "(OK)" << endl;
        }
    }

    /* token_string�� lexeme ���ڿ��� ����*/
    token_string = lexeme;
    /*���� ��ū�� ���� ����Ű�� �ʴ´ٸ� token_string�� ���*/
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
    statement();/* ���� ��ū�� �����ݷ��̳� ���̶�� ������ �ش� �����ڿ� 0�� �� ���� �����ڴ� ����� ���� ����*/
    if (next_token == SEMI_COLON) { // ���� ��ū�� �����ݷ��̸� ���� ���� ���ο� ���� �ĺ���, ������, ����� ������ ���
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
        else {// �� ���� ��� ���
            cout << "(OK)" << endl;
        }

        if (!error_op.empty() || paren != 0)
            result.find(operand)->second = UNDEFINE;
        else
            result.find(operand)->second = sum;

        // �ĺ���, ������, ��� ���� �ʱ�ȭ
        id = 0;
        op = 0;
        num = 0;

        // ���� ����, ��� ���, ��ȣ ���� ���� �ʱ�ȭ
        error = false;
        sum = 0;
        paren = 0;

        // �ߺ������ڿ� ���� ���� ����
        while (!warning.empty()) warning.pop_back();

        // ���� ��ū�� �����ݷ��̶�� statements ȣ��
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
    //������ ������ �ĺ��ڶ�� �������� �����ϰ� 0���� �ʱ�ȭ�Ѵ�. �׸��� �ش� ���ڸ� operand�� �����Ѵ�. 
    if (next_token == IDENT) {
        result.insert({ token_string, UNDEFINE });
        operand = token_string;
    }
    //���� ���ڸ� �������� ���� lexical�� ȣ��
    lexical();
    if (next_token == ASSIGN_OP) { // ���� ��ū�� assign_op��� ���� ���ڸ� �ҷ����� expression ȣ��
        lexical();
        expression();

        sum = number.top(); //������ ��� ����� �����.
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

/* <term_tail> -> <add_op><term><term_tail> | ��*/
void term_tail() {
    while (next_token == ADD_OP || next_token == MIN_OP) { // ���� ��ū�� ADD_OP�� MIN_OP��� �ݺ�
        if ((token == ADD_OP || token == MIN_OP) && token == next_token) { // �ߺ��������� ��쿡�� ����� ���� �ʰ� �Ľ̸� ��.
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
        if (number.empty()) { // number �迭�� ����ִ��� ��� token_string�� ����ϱ� ���� parsing��.
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
        if (next_token == CONST) { // ���� ��ū�� CONST�� ��� token_string�� ����� ��ȯ�Ͽ� ���ÿ� ����.
            number.push(atoi(token_string));
        }
        else {
            if (result.find(token_string) != result.end()) {
                if (result.find(token_string)->second >= UNDEFINE) { // ���̺��� ������ ���� 0�̹Ƿ� ���ǵ��� ���� �ǿ����ڸ� �����Ͽ����� ��Ÿ��.
                    result.find(operand)->second = UNDEFINE;         // �׷��Ƿ� ���� ��� ���� �ǿ����ڴ� ���� ������ �� ���� 0���� ����.
                    number.push(UNDEFINE);
                }
                else
                    number.push(result.find(token_string)->second);
            }
            else if (result.find(token_string) == result.end()) { // �ش� �ĺ��ڰ� ���ǵ��� ���� ���
                error = true; // �������� ǥ��
                error_op = token_string; // ������ �߱��� �������� error_op�� ����.
                result.insert({ token_string, UNDEFINE }); // ���ǵ��� ���� ������ ����.
                number.push(UNDEFINE); // ���ÿ� 0 ����.
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

/* <factor_tail> -> <mult_op><factor><factor_tail> | ��*/
void factor_tail() {
    while (next_token == MULT_OP || next_token == DIV_OP) { // ���� ��ū�� MULT_OP�� DIV_OP��� �ݺ�
        if ((token == MULT_OP || token == DIV_OP) && token == next_token) { // �ߺ��������� ��쿡�� ����� ���� �ʰ� �Ľ̸� ��.
            lexical();
            factor();
            factor_tail();
        }
        else if (next_token == MULT_OP) {
            sum = number.top(); // ���� ���� �ִ� ���� sum�� ������ ��,
            number.pop(); //  ���ÿ��� ����.
            lexical();
            factor();
            factor_tail();
            sum *= number.top(); //������ �Ľ��ϸ� ����� ���� ������ sum�� ������ ���� ���.
            number.pop(); // ���� ����� ���� ������ ��
            number.push(sum); // ��� ����� ���ÿ� ����.
        }
        else if (next_token == DIV_OP) { // ���� ��ū�� MULT_OP�� ���� ����� ������� ���
            sum = number.top();
            number.pop();
            lexical();
            factor();
            factor_tail();
            sum /= number.top();
            number.pop();
            number.push(sum);
        }
        else if (number.empty()) { // number �迭�� ����ִ��� ��� token_string�� ����ϱ� ���� parsing��.
            lexical();
            factor();
            factor_tail();
        }
    }
    return;
}
