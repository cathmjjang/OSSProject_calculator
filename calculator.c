#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Temporary
#include <getopt.h>
#include "stack.h"

#define bool char
#define true 1
#define false 0

#define PI 3.141592653589793

#define MAXTOKENLENGTH 512
#define MAXPRECISION 20
#define DEFAULTPRECISION 5
#define FUNCTIONSEPARATOR "|"

typedef enum
{
	addop,
	multop,
	expop,
	lparen,
	rparen,
	digit,
	value,
	decimal,
	space,
	text,
	function,
	identifier,
	argsep,
	invalid
} Symbol;

struct Preferences
{
	struct Display
	{
		bool tokens;
		bool postfix;
	} display;
	struct Mode
	{
		bool degrees;
	} mode;
	int precision;
	int maxtokenlength;
} prefs;

typedef enum
{
	divZero,
	overflow,
	parenMismatch,
	inputMissing,
} Error;

typedef char* token;

typedef double number;

//raise함수 - 입력된 에러 종류에 따라 에러 메세지 출력
void raise(Error err)
{
   /*
   * 인자
   * 1 : err 에러 유형
   * 변수명
   * 1 : msg 출력 메세지를 입력하기 위한 문자열 변수
   */
   char* msg;
   switch (err) // 에러 종류 구별
   {
   case divZero: // 0으로 나누기 에러
      msg = "Divide by zero";
      break;
   case overflow: // 오버플로우 에러
      msg = "Overflow";
      break;
   case parenMismatch: // 괄호 비대칭 에러
      msg = "Mismatched parentheses";
      break;
   case inputMissing: // 함수 입력 에러
      msg = "Function input missing";
      break;
   }
   printf("\tError: %s\n", msg); //에러 메세지 출력
}

//buildNumber 함수 - 문자열을 숫자로 변환하여 리턴하는 함수
number buildNumber(token str)
{
   /*
   * 인자
   * 1 : str 변환할 문자열
   * 변수명
   * 1 : result 결과값 저장을 위한 변수
   */
   number result = 0;
   result = strtod(str, NULL); // 인자값을 strtod 함수를 사용하여 숫자로 변환
   return result; // 결과값 리턴
}

//num2Str 함수 - 숫자를 문자열로 변환하여 리턴하는 함수
token num2Str(number num)
{
   /*
   * 인자
   * 1 : num 변환할 숫자
   * 변수명
   * 1 : Len 문자열의 길이를 받기 위한 변수
   * 2 : precision 소숫점 비교를 위한 변수
   */
   int len = 0;
   int precision = MAXPRECISION;
   if (prefs.precision >= 0 && prefs.precision < precision)
      precision = prefs.precision;
   token str = (token)malloc(prefs.maxtokenlength * sizeof(char)); // 리턴을 위한 문자열 선언
   len = snprintf(str, prefs.maxtokenlength - 1, "%.*f", precision, num); // 문자열 길이 저장
   if (prefs.precision == -1) // auto 기능인지 검증
   {
      while (str[len - 1] == '0') // auto 기능인 경우 소숫점 마지막 부분부터 검증
      {
         len = snprintf(str, prefs.maxtokenlength - 1, "%.*f", --precision, num); // 0이면 제거
      }
   }
   return str; // 변환된 문자열 리턴
}

//toRadians 함수 - 도를 라디안으로 변환하는 함수
number toRadians(number degrees)
{
   /*
   * 인자
   * 1 : degrees 변환할 도 값
   */
   return degrees * PI / 180.0; // 변환값 리턴
}

//toDegrees 함수 - 라디안을 도로 변환하는 함수
number toDegrees(number radians)
{/*
   * 인자
   * 1 : radians 변환할 라디안 값
   */
   return radians * 180.0 / PI; // 변환값 리턴
}

// doFunc 함수 - 입력 수학 함수에 따른 계산 함수
token doFunc(Stack *s, token function)
{
   /*
   * 인자
   * 1 : s 값을 넘겨 받을 스택
   * 2 : function 계산함수
   * 변수명
   * 1 : input 스택에서 문자열을 받기 위한 변수
   * 2 : num 위의 input 변수를 숫자로 변환하여 저장하기 위한 변수
   * 3 : result 결과값 저장을 위한 변수
   * 4 : counter 평균 등 계산을 위한 수의 갯수 변수
   */
   if (stackSize(s) == 0) // 스택에 아무런 데이터가 없을 때 error
   {
      raise(inputMissing); //raise 함수를 호출하여 Error 출력
      return "NaN"; //NaN 리턴 후 함수 종료
   }
   else if (stackSize(s) == 1 && strcmp(stackTop(s), FUNCTIONSEPARATOR) == 0) // 스택에 데이터가 있지만 함수 분리 기호일 경우 Error
   {
      stackPop(s); // 함수 분리 기호 꺼내기
      raise(inputMissing); // raise 함수를 호출하여 Error 출력
      return "NaN"; // NaN 리턴 후 함수 종료
   }
   token input = (token)stackPop(s);
   number num = buildNumber(input);
   number result = num;
   number counter = 0;
   number i, j, temp = 1;
   if (strncmp(function, "abs", 3) == 0) // 입력 함수가 abs인 경우
      result = fabs(num);
   else if (strncmp(function, "floor", 5) == 0) // 입력 함수가 floor인 경우
      result = floor(num);
   else if (strncmp(function, "ceil", 4) == 0) // 입력 함수가 ceil인 경우
      result = ceil(num);
   else if (strncmp(function, "sin", 3) == 0) // 입력 함수가 sin인 경우
      result = !prefs.mode.degrees ? sin(num) : sin(toRadians(num));
   else if (strncmp(function, "cos", 3) == 0) // 입력 함수가 cos인 경우
      result = !prefs.mode.degrees ? cos(num) : cos(toRadians(num));
   else if (strncmp(function, "tan", 3) == 0) // 입력 함수가 tan인 경우
      result = !prefs.mode.degrees ? tan(num) : tan(toRadians(num));
   else if (strncmp(function, "arcsin", 6) == 0 // 입력 함수가 arcsin인 경우
      || strncmp(function, "asin", 4) == 0)
      result = !prefs.mode.degrees ? asin(num) : toDegrees(asin(num));
   else if (strncmp(function, "arccos", 6) == 0 // 입력 함수가 arccos인 경우
      || strncmp(function, "acos", 4) == 0)
      result = !prefs.mode.degrees ? acos(num) : toDegrees(acos(num));
   else if (strncmp(function, "arctan", 6) == 0 // 입력 함수가 arctan인 경우
      || strncmp(function, "atan", 4) == 0)
      result = !prefs.mode.degrees ? atan(num) : toDegrees(atan(num));
   else if (strncmp(function, "sqrt", 4) == 0) // 입력 함수가 sqrt인 경우
      result = sqrt(num);
   else if (strncmp(function, "cbrt", 4) == 0) // 입력 함수가 cbrt인 경우
      result = cbrt(num);
   else if (strncmp(function, "log", 3) == 0) // 입력 함수가 log인 경우
      result = log(num);
   else if (strncmp(function, "exp", 3) == 0) // 입력 함수가 exp인 경우
      result = exp(num);
   else if (strncmp(function, "min", 3) == 0) // 입력 함수가 min인 경우
   {
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s); // 스택에서 데이터 pop
         num = buildNumber(input); // 계산을 위해 숫자로 변환
         if (num < result) // 현재 최소값보다 작은 경우
            result = num; //최소값 변경
      }
   }
   else if (strncmp(function, "max", 3) == 0) // 입력 함수가 max인 경우
   {
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s); // 스택에서 데이터 pop
         num = buildNumber(input); // 계산을 위해 숫자로 변환
         if (num > result) // 현재 최대값보다 큰 경우
            result = num; // 최대값 변경
      }
   }
   /* GCD function */
   else if (strncmp(function, "gcd", 3) == 0) // 입력 함수가 gcd인 경우
   {
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s);  // 스택에서 데이터 pop
         num = buildNumber(input); // 계산을 위해 숫자로 변환

         j = (result < num) ? result : num; // j에는 result 가 num 보다 작으면 result, 작지 않으면 num 이 저장
         for (i = 1; i <= j; i++) //gcd 계산하는 for문
         {
            if ((int)result % (int)i == 0.0 && (int)num % (int)i == 0.0)
               temp = i;
         }
         result = temp; // temp에 저장된 gcd 결과값을 result에 저장
      }
   }
   else if (strncmp(function, "lcm", 3) == 0)
   {
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s);  // 스택에서 데이터 pop
         num = buildNumber(input); // 계산을 위해 숫자로 변환

         j = (result > num) ? result : num;// j에는 result 가 num 보다 작으면 result, 작지 않으면 num 이 저장

         for (i = j;; i++) // lcm 계산하는 for문
         {
            if ((int)i % (int)num == 0 && (int)i % (int)result == 0)
            {
               temp = i;
               break; // lcm이 계산되면 즉시 for문을 탈출
            }
         }
         result = temp; // lcm 을 가진 temp를 result에 저장
      }
   }
   else if (strncmp(function, "fac", 3) == 0) // 입력 함수가 fac인 경우
   {
      for (i = 1; i <= num; i++) // 1부터 num 까지 fac연산 실행
      {
         temp = i * temp;
      }
      result = temp; // temp인 결과값을 result에 저장
   }
   else if (strncmp(function, "sum", 3) == 0) // 입력 함수가 sum인 경우
   {
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s); // 스택에서 데이터 pop
         num = buildNumber(input); // 계산을 위해 숫자로 변환
         result += num; // result에 값 반복 plus
      }
   }
   else if (strncmp(function, "avg", 3) == 0 || // 입력함수가 avg인 경우
      strncmp(function, "mean", 4) == 0) // 또는 입력함수가 mean인 경우
   {
      counter = 1; // 데이터가 이미 하나 나와 있으므로 counter 1로 초기화
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s); // 스택에서 데이터 pop
         num = buildNumber(input); // 계산을 위해 숫자로 변환
         result += num; // result에 값 반복 plus
         counter++; // 평균을 위해 counter 증가
      }
      result /= counter; // 평균 계산
   }
   else if (strncmp(function, "median", 6) == 0) // 입력 함수가 median인 경우
   {
      Stack tmp, safe; // 값 계산을 위한 임시 스택
      counter = 1; // 데이터가 이미 하나 나와이으므로 counter 1로 초기화
      stackInit(&tmp, (stackSize(s) > 0 ? stackSize(s) : 1)); // 스택 초기화
      stackInit(&safe, (stackSize(s) > 0 ? stackSize(s) : 1)); // 스택 초기화
      stackPush(&tmp, input); // 첫번째 데이터 tmp스택에 push
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s);
         num = buildNumber(input);

         while (stackSize(&tmp) > 0 && buildNumber(stackTop(&tmp)) < num) // tmp스택 값보다 큰 모든 수 저장
         {
            stackPush(&safe, stackPop(&tmp));
         }

         stackPush(&tmp, input); // 정렬된 스택에 값 push

         while (stackSize(&safe) > 0)
         {
            stackPush(&tmp, stackPop(&safe)); // 정렬된 모든 숫자를 다시 스택으로 push
         }
         counter++;
      }
      stackFree(&safe);
      counter = (number)(((int)counter + 1) / 2); // median 인덱스 계산
      while (counter > 1) // median 인덱스까지 모든 데이터를 스택에서 pop
      {
         stackPop(&tmp);
         counter--;
      }
      result = buildNumber(stackPop(&tmp));
      while (stackSize(&tmp) > 0) // 할당 취로를 위해 남은 데이터 모두 pop
      {
         stackPop(&tmp);
      }
      stackFree(&tmp); // tmp 스택 free
   }
   else if (strncmp(function, "var", 3) == 0) // 입력 함수가 var인 경우
   {
      Stack tmp;
      counter = 1;
      stackInit(&tmp, (stackSize(s) > 0 ? stackSize(s) : 1));
      stackPush(&tmp, input);
      number mean = result;
      while (stackSize(s) > 0 && strcmp(stackTop(s), FUNCTIONSEPARATOR) != 0) // 스택에 데이터가 없을때까지 반복
      {
         input = (token)stackPop(s);
         stackPush(&tmp, input);
         num = buildNumber(input);
         mean += num;
         counter++;
      }
      mean /= counter;
      result = 0;
      while (stackSize(&tmp) > 0) // 
      {
         input = (token)stackPop(&tmp);
         num = buildNumber(input) - mean;  
         result += pow(num, 2);             //result = 계산된 결과값의 제곱
      }
      result /= counter;
      stackFree(&tmp);
   }
   if (strcmp(stackTop(s), FUNCTIONSEPARATOR) == 0)
      stackPop(s);
   stackPush(s, num2Str(result)); //표준편차를 반환한다.
   return 0;
}

// doOp 함수 - 스택을 받아와서 받아온 토큰이 연산라면 연산 후 결과에 대한 값을 스택에 넣어준다.
int doOp(Stack *s, token op) 
{
   /*
   * 인자
   * 1 : s 값을 넘겨 받을 스택
   * 2 : op 입력받은 토큰
   * 변수명
   * 1 : roperand 우변 스택을 저장할 토큰변수
   * 2 : loperand 좌변 스택을 저장할 토큰변수
   * 3 : 우변 스택의 문자열인 값을 숫자형으로 저장할 변수
   * 4 : 좌변 스택의 문자열인 값을 숫자형으로 저장할 변수
   * 5 : ret 결과를 저장할 변수
   */
   token roperand = (token)stackPop(s); //좌변 우변 스택에서 pop 후 저장
   token loperand = (token)stackPop(s);
   number lside = buildNumber(loperand); //pop한 토큰이 문자형이므로 숫자형으로 변환
   number rside = buildNumber(roperand);
   number ret; //결과값 저장할 변수 선언
   switch (*op)
   {
   case '^': // ^연산자 이면 제곱수 연산 실행
   {
      ret = pow(lside, rside);
   }
   break;
   case '*': // *연산자 이면 곱셈 연산 실행
   {
      ret = lside * rside;
   }
   break;
   case '/': // /연산자 이면 나누기 연산 실행
   {
      if (rside == 0)
      {
         raise(divZero); //0으로 나눌경우 예외처리 
         return -1;
      }
      else
         ret = lside / rside;
   }
   break;
   case '%': // %연산자 이면 나머지 연산 실행
   {
      if (rside == 0)
      {
         raise(divZero);//0으로 나눌경우 예외처리
         return -1;
      }
      else
      {
         ret = (int)(lside / rside); //나머지 구하는 연산
         ret = lside - (ret * rside);
      }
   }
   break;
   case '+': // +연산자 이면 덧셈 연산 실행
   {
      ret = lside + rside;
   }
   break;
   case '-': // -연산자 이면 뺄셈 연산 실행
   {
      ret = lside - rside;
   }
   break;
   }
   stackPush(s, num2Str(ret)); // 결과값을 스택에 저장
   return 0;
}

// ufgets 함수 - 버퍼를 동적할당 하고 입력을 받는다.
char* ufgets(FILE* stream) 
{
   /*
   * 인자
   * 1 : stream 파일 기술자 인자
   * 변수명
   * 1 : maxlen 버퍼 동적할당 할 사이즈 제한하기 위한 변수
   * 2 : size 버퍼의 사이즈를 제한하는 함수
   * 3 : ch 파일의 끝을 나타내는 변수
   * 4 : pos 다음 문자를 삽입할때 버퍼의 크기를 늘려줄 변수
   */
   unsigned int maxlen = 128, size = 128;
   char* buffer = (char*)malloc(maxlen);

   if (buffer != NULL)
   {
      char ch = EOF;
      int pos = 0;

      while ((ch = getchar()) != EOF && ch != '\n') // 필요에 따라 버퍼 크기를 조정하면서 한 번에 하나씩 문자를 읽음.
      {
         buffer[pos++] = ch; // 버퍼 맨뒤에 EOF 넣는다
         if (pos == size) //다음 문자를 삽입하려면 메모리가 더 필요함.
         {
            size = pos + maxlen; // 버퍼 크기 늘려줌
            buffer = (char*)realloc(buffer, size); // 버퍼 동적할당
         }
      }
      buffer[pos] = '\0'; //마지막엔 널을 넣어준다.
   }
   return buffer; // 버퍼 반환
}

// type 함수 - 입력받은 인자가 어떤기능을 하는 문자인지 판별
Symbol type(char ch)
{
   /*
   * 인자
   * 1 : ch 기능 판별하기위한 글자
   * 변수명
   * 1 : result 결과를 반환하기위한 변수
   */
   Symbol result;
   switch (ch) // 입력받은 문자에 대한 판별 switch문
   {
   case '+':
   case '-':
      result = addop; //+와 - 연산자가 들어왔다면
      break; 
   case '*':
   case '/':
   case '%':
      result = multop; //곱의연산으로 들어간다.
      break;
   case '^':
      result = expop; //제곱수
      break;
   case '(':
      result = lparen; //좌괄호 open
      break;
   case ')':
      result = rparen; //우괄호 open
      break;
   case '.':
      result = decimal; //소수점이고 이하 동문. 띄어쓰기 숫자 , 알파벳(글자)까지.
      break;
   case ' ':
      result = space; // 공백문자 판별
      break;
   case ',':
      result = argsep; // argsep 함수 판별
      break;
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      result = digit; // 1 ~ 9 이면 숫자로 판별
      break;
   case 'A':
   case 'B':
   case 'C':
   case 'D':
   case 'E':
   case 'F':
   case 'G':
   case 'H':
   case 'I':
   case 'J':
   case 'K':
   case 'L':
   case 'M':
   case 'N':
   case 'O':
   case 'P':
   case 'Q':
   case 'R':
   case 'S':
   case 'T':
   case 'U':
   case 'V':
   case 'W':
   case 'X':
   case 'Y':
   case 'Z':
   case 'a':
   case 'b':
   case 'c':
   case 'd':
   case 'e':
   case 'f':
   case 'g':
   case 'h':
   case 'i':
   case 'j':
   case 'k':
   case 'l':
   case 'm':
   case 'n':
   case 'o':
   case 'p':
   case 'q':
   case 'r':
   case 's':
   case 't':
   case 'u':
   case 'v':
   case 'w':
   case 'x':
   case 'y':
   case 'z':
      result = text; // 알파벳이면 text로 판별
      break;
   default:
      result = invalid; // 나머지는 오류로 판별
      break;
   }
   return result; // 결과반환
}

// isFuncion 함수 - 입력받은 token의 글자를 비교하여 어느 함수인지 판별
bool isFunction(token tk) 
{
   return (strncmp(tk, "abs", 3) == 0 // 해당하는 함수가 있으면 해당함수  반환
      || strncmp(tk, "floor", 5) == 0
      || strncmp(tk, "ceil", 4) == 0
      || strncmp(tk, "sin", 3) == 0
      || strncmp(tk, "cos", 3) == 0
      || strncmp(tk, "tan", 3) == 0
      || strncmp(tk, "arcsin", 6) == 0
      || strncmp(tk, "arccos", 6) == 0
      || strncmp(tk, "arctan", 6) == 0
      || strncmp(tk, "asin", 4) == 0
      || strncmp(tk, "acos", 4) == 0
      || strncmp(tk, "atan", 4) == 0
      || strncmp(tk, "sqrt", 4) == 0
      || strncmp(tk, "cbrt", 4) == 0
      || strncmp(tk, "log", 3) == 0
      || strncmp(tk, "min", 3) == 0
      || strncmp(tk, "max", 3) == 0
      || strncmp(tk, "sum", 3) == 0
      || strncmp(tk, "avg", 3) == 0
      || strncmp(tk, "fac", 3) == 0
      || strncmp(tk, "gcd", 3) == 0
      || strncmp(tk, "lcm", 3) == 0
      || strncmp(tk, "mean", 4) == 0
      || strncmp(tk, "median", 6) == 0
      || strncmp(tk, "var", 3) == 0
      || strncmp(tk, "exp", 3) == 0);
}


// tokenType 함수 - 인자로 받은 토큰의 속성을 판별(함수인지 음수인지 소수인지)
Symbol tokenType(token tk)
{
   /*
   * 인자
   * 1 : tk 값을 넘겨받는 토큰변수
   * 변수명
   * 1 : ret 토큰 타입을 저장할 변수
   */
   if (!tk) //잘못된 토큰을 받아왔다면 invalid반환.
      return invalid;
   Symbol ret = type(*tk); //토큰의 타입을 저장하고 switch로 돌린다.
   switch (ret)
   {
   case text:
      if (isFunction(tk)) //글자일때 토큰의 함수 판별.
         ret = function;
      else
         ret = identifier;
      break;
   case addop: //addop이면 토큰이 음수이고 토큰의 길이가 1보다 클때 현재토큰의 다음토큰에대한 재귀값을 ret에 넣어준다.
      if (*tk == '-' && strlen(tk) > 1)
         ret = tokenType(tk + 1);
      break;
   case decimal: //소수와 숫자는 값으로 판별
   case digit:
      ret = value;
      break;
   default:
      break;
   }
   return ret; // 결과 반환
}

// tokenize 함수 - 식을 인자로 가져와서 각각의 글자들을 조합해서 토큰화한다.
int tokenize(char *str, char *(**tokensRef))
{
   /*
   * 인자
   * 1 : str 메인함수에서 입력받은 문자열
   * 2 : tokenRef 2차원배열로 저장되는 토큰의 주소값을 저장하는 주소값
   * 변수명
   * 1 : i for문 변수
   * 2 : tokens 토큰2차원으로 집합적으로 저장할 배열
   * 3 : tmp tokens의 임시저장변수
   * 4 : ptr 토큰의 인덱스 나타내는 변수
   * 5 : ch 배열의 맨뒤에 끝을 알리는 변수
   * 6 : numTokens 토큰갯수 카운트하는 변수
   * 7 : tmpToken 임시로 1차원 토큰을 입력받을 배열
   * 8 : newToken 새롭게 받아올 토큰 선언한 변수
   */
   int i = 0;
   char** tokens = NULL; //토큰2차원으로 집합적으로 저장할 배열
   char** tmp = NULL; //tokens 의 임시저장변수
   char* ptr = str; //토큰의 인덱스
   char ch = '\0';
   int numTokens = 0; //토큰갯수 카운트변수
   char* tmpToken = malloc((prefs.maxtokenlength + 1) * sizeof(char)); //임시로 받을 1차원토큰의 배열의 크기를 최대사이즈로 동적할당해준다
   if (!tmpToken)
   {
      fprintf(stderr, "Malloc of temporary buffer failed\n"); // 에러문구
      return 0;
   }
   while ((ch = *ptr++))
   {
      if (type(ch) == invalid) //잘못된 문자를 만났을 때 토큰화 중지
         break;

      token newToken = NULL;  //새롭게 받아올 토큰을 선언
      tmpToken[0] = '\0'; //tmptoken변수의 맨앞을 0으로 초기화 > 아래 스위치문에 해당하는게 없으면 \0을 넣은채로 처리하기위해서 미리선언해둠.
      switch (type(ch))
      {
      case addop:
      {
         // Check if this is a negative //음수인지 아닌지 확인
         if (ch == '-'
            && (numTokens == 0
               || (tokenType(tokens[numTokens - 1]) == addop
                  || tokenType(tokens[numTokens - 1]) == multop
                  || tokenType(tokens[numTokens - 1]) == expop
                  || tokenType(tokens[numTokens - 1]) == lparen
                  || tokenType(tokens[numTokens - 1]) == argsep)))
         {
            // n자 (null-terminator) 번호 토큰을 조립합니다
            {
               int len = 1;
               bool hasDecimal = false; //닫은상태로 소수점, 지수 초기화.
               bool hasExponent = false;

               if (type(ch) == decimal) //숫자를 소수점으로 시작하도록 허용합니다
               {
                  hasDecimal = true; //소수점이면 열어주고 .하나더받아야되니까 len하나 늘려주고 처음에 0과 .을 추가해준다.
                  len++;
                  tmpToken[0] = '0';
                  tmpToken[1] = '.';
               }
               else //소수점으로 시작하지 않는 숫자
               {
                  tmpToken[len - 1] = ch;
               }

                //나머지 숫자를 조립
               for (;  //len을 바꾸지 마세요
                  *ptr //ptr변수는 다음캐릭터가 있고, 그것은 null이 아니어야 하고.
                  && len <= prefs.maxtokenlength
                  && (type(*ptr) == digit //다음 캐릭터는 digit이어야 하고
                     || ((type(*ptr) == decimal//다음 문자는 소수이어야 하고
                        && hasDecimal == 0)) //하지만 소수점은 추가하지 말아야하고
                     || ((*ptr == 'E' || *ptr == 'e')  //또는 다음 캐릭터가 지수이거나
                        && hasExponent == false)  //하지만 아직 지수를 추가하지 않았다
                     || ((*ptr == '+' || *ptr == '-') && hasExponent == true)); //기호가 표시되어 있습니다
                  ++len)
               {
                  if (type(*ptr) == decimal) // ptr의 타입이 소수이면
                     hasDecimal = true;
                  else if (*ptr == 'E' || *ptr == 'e') //
                     hasExponent = true;
                  tmpToken[len] = *ptr++;
               }

               //NULL터미널을 추가한다.
               tmpToken[len] = '\0';
            }
            break;
         }
      }
      case multop: //다른 연산들(제곱, 지수 등등....)
      case expop:
      case lparen:
      case rparen:
      case argsep:
         
      {
         tmpToken[0] = ch;
         tmpToken[1] = '\0';
      }
      break;
      case digit:
      case decimal:
         //n자(null-terminator) 번호 토큰을 조립합니다
      {
         int len = 1;
         bool hasDecimal = false;
         bool hasExponent = false;

         if (type(ch) == decimal) //숫자를 소수점으로 시작하도록 허용합니다
         {
            //소수라면 앞에 0과 .을 넣는다.
            hasDecimal = true;
            len++;
            tmpToken[0] = '0';
            tmpToken[1] = '.';
         }
         else //소수점으로 시작하지 않는 숫자
         {
            tmpToken[len - 1] = ch;
         }

         
         for (;  //len을 바꾸지 마세요
            *ptr //ptr변수는 다음캐릭터가 있고, 그것은 null이 아니어야 하고.
            && len <= prefs.maxtokenlength
            && (type(*ptr) == digit //다음 캐릭터는 digit이어야 하고
               || ((type(*ptr) == decimal//다음 문자는 소수이어야 하고
                  && hasDecimal == 0)) //하지만 소수점은 추가하지 말아야하고
               || ((*ptr == 'E' || *ptr == 'e')  //또는 다음 캐릭터가 지수이거나
                  && hasExponent == false)  //하지만 아직 지수를 추가하지 않았다
               || ((*ptr == '+' || *ptr == '-') && hasExponent == true)); //기호가 표시되어 있습니다
            ++len)
         {
            if (type(*ptr) == decimal) //ptr의 type이 소수이라면
               hasDecimal = true; //소수부분 변수를 열어주고
            else if (*ptr == 'E' || *ptr == 'e') //지수를 나타내고있다면
               hasExponent = true; //지수기능 boolean값 열어주고
            tmpToken[len] = *ptr++; //템프토큰 맨마지막에 ptr넣어준다.
         }

         //널 터미널을 추가합니다
         tmpToken[len] = '\0';
      }
      break;
      case text:
         //n자(null-terminator) 텍스트 토큰을 조립합니다
      {
         int len = 1;
         tmpToken[0] = ch;
         for (len = 1; *ptr && type(*ptr) == text && len <= prefs.maxtokenlength; ++len)
         {
            tmpToken[len] = *ptr++; //토큰 조립
         }
         tmpToken[len] = '\0'; // 토큰 조립 후 맨뒤에 \0 붙여서 끝임을 표시
      }
      break;
      default:
         break;
      }//switch

      //토큰 목록에 추가합니다
      if (tmpToken[0] != '\0' && strlen(tmpToken) > 0)
      {
         numTokens++;

         newToken = malloc((strlen(tmpToken) + 1) * sizeof(char)); //뉴토큰 동적할당 템프토큰 크기로 (+1은 \0포함하기위해서)
         if (!newToken) //실패했다면 위에서 늘렸던 넘토큰 하나 줄이기. 설레발이었기때문에
         {
            numTokens--;
            break;
         }
         strcpy(newToken, tmpToken); //뉴토큰에 템프토큰을 저장
         newToken[strlen(tmpToken)] = '\0'; //뉴토큰 맨마지막에 넕캐릭터로 넣어서 마지막 이라고 식별기능 구현
         tmp = (char**)realloc(tokens, numTokens * sizeof(char*)); //템프 동적할당.
         if (tmp == NULL) //템프 동적할당을 했는데 실패했다면 아무것도 없다는 뜻이므로 안에서는 모두 free해줄것이다.
         {
            free(newToken); //뉴토큰 프리
            if (tokens != NULL) //2차원 배열 프리.
            {
               for (i = 0;i < numTokens - 1;i++)
               {
                  if (tokens[i] != NULL)
                     free(tokens[i]);
               }
               free(tokens);
            }
            *tokensRef = NULL; //토큰 테이블 비었다고 초기화
            free(newToken); //프리
            free(tmpToken); //프리
            return 0; //함수종료
         }
         tokens = tmp; //동적할당이 실패하지않았거나 실패해서 위에서 if문 실행했다면 토큰에 템프 넣고
         tmp = NULL; //템프 널 초기화
         tokens[numTokens - 1] = newToken; //토큰 맨마지막에 행에 뉴토큰 넣기.
      }
   }
   *tokensRef = tokens; // 토큰을 토큰테이블의 주소값에다가 저장
   free(tmpToken); //템프토큰 free
   tmpToken = NULL; //템프토큰 null로 초기화
   return numTokens; //토큰의 갯수를 반환
}

bool leftAssoc(token op)
{
   /*
   token이
   + - * / % 면 true
   ^ function이면 false를 반환하며
   같은 우서눈위 끼리 묶어 주기 위한 함수
   */
   bool ret = false;
   switch (tokenType(op))
   {
   case addop:
   case multop:

      ret = true;
      break;
   case function:
   case expop:
      ret = false;
      break;
   default:
      break;
   }
   return ret;
}

int precedence(token op1, token op2)
{
   /*
   * 연산자(현재와 스택) 우선순위 결정 함수
   * 반환되는 값(ret) 0 : 스택 == 현재
   * 반환되는 값(rett) 1 : 스택 > 현재
   * 반환되는 값(rett) 1 : 스택 < 현재
   */
   int ret = 0;

   if (op2 == NULL)
      ret = 1;
   else if (tokenType(op1) == tokenType(op2)) // 우선순위가 같을 때
      ret = 0;
   else if (tokenType(op1) == addop
      && (tokenType(op2) == multop || tokenType(op2) == expop)) // 현재가 더 낮을 때
      ret = -1;
   else if (tokenType(op2) == addop
      && (tokenType(op1) == multop || tokenType(op1) == expop)) // 현재가 더 높을 때
      ret = 1;
   else if (tokenType(op1) == multop
      && tokenType(op2) == expop) // 현재가 더 낮을 때
      ret = -1;
   else if (tokenType(op1) == expop
      && tokenType(op2) == multop) // 현재가 더 높을 때
      ret = 1;
   else if (tokenType(op1) == function
      && (tokenType(op2) == addop || tokenType(op2) == multop || tokenType(op2) == expop || tokenType(op2) == lparen)) // 현재가 더 낮을 때
      ret = 1;
   else if ((tokenType(op1) == addop || tokenType(op1) == multop || tokenType(op1) == expop) // 현재가 더 높을 때
      && tokenType(op2) == function)
      ret = -1;
   return ret; //우선순위에 대한 값을 반환
}

void evalStackPush(Stack *s, token val)
{
   /*
   * 실제로 각각의 기능에 맞게끔 연산을 하고
   * 결과를 스택에 push하는 함수
   */
   if (prefs.display.postfix) //결과값 출력시 후위에 대한 것을 표기
      printf("\t%s\n", val);

   switch (tokenType(val))
   {
   case function:
   {
      if (doFunc(s, val) < 0) //스택과 값으로, 반환이 0이면 제대로 처리됨. 아니면 nan에러
         return;
   }
   break;
   case expop:
   case multop:
   case addop:
   {
      if (stackSize(s) >= 2) // 두개의 피연산자가 있을 경우
      {
         if (doOp(s, val) < 0) //연산, 에러처리
            return;
      }
      else
      {
         stackPush(s, val); //값을 저장 함
      }
   }
   break;
   case value:
   {
      stackPush(s, val); //숫자만 있으므로 푸시
   }
   break;
   default:
      break;
   }
}


bool postfix(token *tokens, int numTokens, Stack *output)
{
   /*
   * 인자
   * 1 : *tokens 현재 입력된 토큰 단위의 계산식
   * 2 : numTokens 계산식에서 토큰 단위로 나눴을 경우의 개수
   * 3 : *output 결과값이 저장될 스택
   * 변수명
   * 1 : operaitors : 연산자 스택
   * 2 : intermediate : 중간 계산값 스택 
   * 3 : err 에러발생의 유무
   */
   Stack operators, intermediate;
   int i;
   bool err = false;
   stackInit(&operators, numTokens);
   stackInit(&intermediate, numTokens); // 스택의 numTokens(최대 개수)만큼만 생성 및 초기화
   for (i = 0; i < numTokens; i++)
   {
      switch (tokenType(tokens[i])) // 토큰의 타입 구별
      {
      case value:
      {
         evalStackPush(output, tokens[i]); // 숫자이면 무조건 결과 (스택)에 푸시
      }
      break;
      case function: // 토큰이 기능인가
      {
         while (stackSize(&operators) > 0 //연산자 스택에 값이 있고
            && (tokenType(tokens[i]) != lparen) // 현재 토큰이 '('가 아니고
            && ((precedence(tokens[i], (char*)stackTop(&operators)) <= 0))) // 연산자 우선순위가 같거나 현재가 더 낮을때
         {
            evalStackPush(output, stackPop(&operators)); //연산자를 꺼내어 계산
            stackPush(&intermediate, stackTop(output)); // 중간 계산값에 푸시
         }
         stackPush(&operators, tokens[i]); // 스택이 비어있을 경우 푸시
      }
      break;
      case argsep:
      {
         /*
         * HACK : while(stackSize(&operators)>0 && stackSize(&operatiors)>1)을 보면
         * while(stackSize(&operatiors)>1)로 바꾸어도 무방한 것으로 보임
         */
         while (stackSize(&operators) > 0 //연산자 스택에 값이 있고
            && tokenType((token)stackTop(&operators)) != lparen // 왼쪽 괄호가 스택의 최상위에 없고
            && stackSize(&operators) > 1) // 연산자 스택에 2개 이상의 값이 있을때
         {
            evalStackPush(output, stackPop(&operators));
            stackPush(&intermediate, stackTop(output));
         }
      }
      break;
      case addop:
      case multop:
      case expop:
      {
         /*
         * 연산자 우선순위에 의해 0또는 -1일 경우에
         * 연산자 스택에서 팝을 하고 그와 맞게 계산을 하고
         * 중간 계산값 스택에 푸시함.
         */
         while (stackSize(&operators) > 0
            && (tokenType((char*)stackTop(&operators)) == addop || tokenType((char*)stackTop(&operators)) == multop || tokenType((char*)stackTop(&operators)) == expop)
            && ((leftAssoc(tokens[i]) && precedence(tokens[i], (char*)stackTop(&operators)) <= 0)
               || (!leftAssoc(tokens[i]) && precedence(tokens[i], (char*)stackTop(&operators)) < 0)))
         {
            evalStackPush(output, stackPop(&operators));
            stackPush(&intermediate, stackTop(output));
         }
         stackPush(&operators, tokens[i]);
      }
      break;
      case lparen:
      {
      
         //현재 토큰이 '(' 일경우 두가지 상황이 있다
         //1. fuction뒤에 온 '(' / 2. 단독적인 '('
         if (tokenType(stackTop(&operators)) == function)
            stackPush(output, FUNCTIONSEPARATOR);
         stackPush(&operators, tokens[i]);
      }
      break;
      case rparen:
      {
         /*
         * 현재 토큰이 ')' 일 경우
         * 1번 : '(' 괄호가 나올때 까지 그 안의 것들에 대해 계산함
         * 2번 : 괄호의 짝이 안맞을 경우 raise함수를 통해 에러메세지 출력
         * 3번 : 괄호안의 계산한 값이 function에 포함되어 있을 경우 계산
         */
         while (stackSize(&operators) > 0
            && tokenType((token)stackTop(&operators)) != lparen
            && stackSize(&operators) > 1)
         {
            evalStackPush(output, stackPop(&operators));
            stackPush(&intermediate, stackTop(output));
         }//......1번
         if (stackSize(&operators) > 0
            && tokenType((token)stackTop(&operators)) != lparen)
         {
            err = true;
            raise(parenMismatch);
         }//......2번
         stackPop(&operators); // 왼쪽 괄호 스택에서 팝(제거)
         while (stackSize(&operators) > 0 && tokenType((token)stackTop(&operators)) == function)
         {
            evalStackPush(output, stackPop(&operators));
            stackPush(&intermediate, stackTop(output));
         }//......3번
      }
      break;
      default:
         break;
      }
   }
   /*
   * 토큰이 아직 스택에 남아있는 경우
   * 1번 : 토큰에 '('가 남아있을 경우 위에서 처리 못한 괄호로 raise함수를 통해 오류메세지 출력
   * 2번 : 연산할 것이 남아있으므로 연산(팝)과 중간값 계산 스택에 푸시
   * 3번 : 중간값 계산 스택 비워주기
   * 4번 : err == true 라는 것은 에러이므로 연산자 스택 비워주기
   */
   while (stackSize(&operators) > 0)
   {
      if (tokenType((token)stackTop(&operators)) == lparen)
      {
         raise(parenMismatch);
         err = true;
      }//......1번
      evalStackPush(output, stackPop(&operators));
      stackPush(&intermediate, stackTop(output));
   }//......2번
   stackPop(&intermediate);
   while (stackSize(&intermediate) > 0)
   {
      stackPop(&intermediate);
   }//......3번
   if (err == true)
   {
      while (stackSize(&operators) > 0)
      {
         token s = stackPop(&operators);
         free(s);
      }
   }//......4번
   stackFree(&intermediate); // 중간계산 스택 할당 해제
   stackFree(&operators); // 연산자 스택 할당 해제
   return err; // 에러 코드를 반환
}
/*
	입력받은 문자열을" "(공백)을 기준으로
	토큰화 시킨다.
	2차원배열에 각 토큰을 배치.
*/
int strSplit(char *str, const char split, char *(**partsRef))
{
	char **parts = NULL;
	char **tmpparts = NULL;
	char *ptr = str;
	char *part = NULL;
	char *tmppart = NULL;
	int numParts = 0;
	char ch;
	int len = 0;
	while(1)
	{
		ch = *ptr++;          //ch -> 현재 문자 //ptr -> 현재문자의 다음문자.
		                      //split = " ";
		// 현재 가르키는 ch(문자가)가 공백(' ')이거나 문자의 끝(\0)이고, 
		//현재 split중인 문자가 존재할경우.
		if((ch == '\0' || ch == split) && part != NULL) // End of part
		{

			tmppart = (char*)realloc(part, (len+1) * sizeof(char));

	/*		
		if재할당이 실패할경우, 
		현재작업중인 단어와 모든 parts의 메모리 할당해제.
	*/
			if (tmppart == NULL)          
			{
				free(part);
				part = NULL;
				for(len=0;len<numParts;len++)
				{
					if (parts[len])
						free(parts[len]);
				}
				if (parts)
					free(parts);
				parts = NULL;
				numParts = 0;
				break;
			}
			part = tmppart;
			part[len] = '\0';


			numParts++;
			if(parts == NULL)
				parts = (char**)malloc(sizeof(char**));
			else
			{
				tmpparts = (char**)realloc(parts, numParts * sizeof(char*));
	/*		
		if재할당이 실패할경우, 
		현재작업중인 단어와 모든 parts의 메모리 할당해제.
	*/
				if (tmpparts == NULL)
				{
					free(part);
					part = NULL;
					for(len=0;len<numParts-1;len++)
					{
						if (parts[len])
							free(parts[len]);
					}
					if (parts)
						free(parts);
					parts = NULL;
					numParts = 0;
					break;
				}
				parts = tmpparts;
			}
			parts[numParts - 1] = part;
			part = NULL;
			len = 0;
		}
		else
		{
			len++;
			if(part == NULL)
			{
				part = (char*)malloc(sizeof(char));
			}
			else
			{
				tmppart = (char*)realloc(part, len * sizeof(char));
	/*		
		if재할당이 실패할경우, 
		현재작업중인 단어와 모든 parts의 메모리 할당해제.
	*/
				if (tmppart == NULL)
				{
					free(part);
					part = NULL;
					for(len=0;len<numParts;len++)
					{
						if (parts[len])
							free(parts[len]);
					}
					free(parts);
					numParts = 0;
					parts = NULL;
					break;
				}
				part = tmppart;
			}
			part[len - 1] = ch;
		}

		if(ch == '\0')
			break;
	}
	*partsRef = parts;
	return numParts;
}

/*
	execCommand메서드
	계산기 모드의 설정 확인 메서드.
	변수 recognized의 true false를 이용하여 진위여부 확인.
	현재 계산기의 모드 확인기능 (get)
	현재 계산기의 모드 변경기능 (set)
*/

bool execCommand(char *str)
{
	int i = 0;
	bool recognized = false;    //올바른 명령어인지 확인.
	char **words = NULL;
	int len = strSplit(str, ' ', &words);// strSplit -> 입력받은 str을 공백단위로 split하여 2차원배열에 각각저장.
	
	if(len >= 1 && strcmp(words[0], "get") == 0)
	{
		if(len >= 2 && strcmp(words[1], "display") == 0)
		{
			if(len >= 3 && strcmp(words[2], "tokens") == 0)
			{
				recognized = true;
				printf("\t%s\n", (prefs.display.tokens ? "on" : "off"));
			}
			else if(len >= 3 && strcmp(words[2], "postfix") == 0)
			{
				recognized = true;
				printf("\t%s\n", (prefs.display.postfix ? "on" : "off"));
			}
		}
		else if(len >= 2 && strcmp(words[1], "mode") == 0)
		{
			recognized = true;
			printf("\t%s\n", (prefs.mode.degrees ? "degrees" : "radians"));
		}
		else if(len >= 2 && strcmp(words[1], "precision") == 0)
		{
			recognized = true;
			if (prefs.precision > 0)
				printf("\t%d\n", prefs.precision);
			else
				printf("\tauto\n");
		}
	}
	else if(len >= 1 && strcmp(words[0], "set") == 0)
	{
		if(len >= 2 && strcmp(words[1], "display") == 0)
		{
			if(len >= 3 && strcmp(words[2], "tokens") == 0)
			{
				if(len >= 4 && strcmp(words[3], "on") == 0)
				{
					recognized = true;
					prefs.display.tokens = true;
				}
				else if(len >= 4 && strcmp(words[3], "off") == 0)
				{
					recognized = true;
					prefs.display.tokens = false;
				}
			}
			else if(len >= 3 && strcmp(words[2], "postfix") == 0)
			{
				if(len >= 4 && strcmp(words[3], "on") == 0)
				{
					recognized = true;
					prefs.display.postfix = true;
				}
				else if(len >= 4 && strcmp(words[3], "off") == 0)
				{
					recognized = true;
					prefs.display.postfix = false;
				}
			}
		}
		else if(len >= 2 && strcmp(words[1], "mode") == 0)
		{
			if(len >= 3 && strcmp(words[2], "radians") == 0)
			{
				recognized = true;
				prefs.mode.degrees = false;
			}
			else if(len >= 3 && strcmp(words[2], "degrees") == 0)
			{
				recognized = true;
				prefs.mode.degrees = true;
			}
		}
		else if (len >= 2 && strcmp(words[1], "precision") == 0)
		{
			if(len >= 3 && strcmp(words[2], "auto") == 0)
			{
				recognized = true;
				prefs.precision = -1;
			}
			else if (len >= 3 && type(words[2][0]) == digit)
			{
				recognized = true;
				prefs.precision = atoi(words[2]);
			}
		}
	}
	if (words)           //2차원 동적할당의 해제.
	{
		for (i=0;i<len;i++)
		{
			if (words[i])
				free(words[i]);
		}
		free(words);

	}

	return recognized;       //진위여부 반환.
}

int main(int argc, char *argv[])
{
	char* str = NULL;
	token* tokens = NULL;
	int numTokens = 0;
	Stack expr;
	int i;
	int ch, rflag = 0;
	prefs.precision = DEFAULTPRECISION;
	prefs.maxtokenlength = MAXTOKENLENGTH;

	printf("┌ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡHELPㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ┐\n");
	printf("｜Operators[ex. 1 + 1]                                        ｜\n");
	printf("｜ + , -, *, / , ^, %%                                         ｜\n");
	printf("｜Functions[ex.abs(-1)]                                       ｜\n");
	printf("｜abs, floor, ceil, sin, cos, tan, arcsin, arccos, arctan,    ｜\n");
	printf("｜sqrt, cbrt, log, exp, min, max, sum, mean, avg, median, var ｜\n");
	printf("｜Plus Functions : fac, lcm, gcd                              ｜\n");
	printf("├ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ┤\n");
	printf("｜Settings[ex.get postfix / set postfix on]                   ｜\n");
	printf("｜get / set                                                   ｜\n");
	printf("｜postfix(off / on)                                           ｜\n");
	printf("｜tokens(off / on)                                            ｜\n");
	printf("｜mode(radians / degrees)                                     ｜\n");
	printf("｜precision(X / auto)                                         ｜\n");
	printf("｜Type quit to close                                          ｜\n");
	printf("└ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ┘\n");

	while ((ch = getopt(argc, argv, "rm:")) != -1) {
		switch (ch) {
			case 'r':
				rflag = 1;
				break;
			case 'm':
				prefs.maxtokenlength = atoi(optarg);
		}
	}
	str = ufgets(stdin);

	while (str != NULL && strcmp(str, "quit") != 0)
	{
		while (strlen(str) == 0)
		{
			str = ufgets(stdin);
		}

		if(!execCommand(str))//입력받은 문자열이 모드변경 문구인지 확인.
		{
			numTokens = tokenize(str, &tokens); //입력받은 문자열을 토큰화시켜 tokens에 저장.
			free(str);
			str = NULL;

			if (prefs.display.tokens)    //모드변경으로 tokens일경우.
			{
				printf("\t%d tokens:\n", numTokens);
				for (i = 0; i < numTokens; i++)
				{
					printf("\t\"%s\"", tokens[i]);
					if (tokenType(tokens[i]) == value)
						printf(" = %f", buildNumber(tokens[i]));
					printf("\n");        //계산과정을 토큰과정으로 제시.
				}
			}

			stackInit(&expr, numTokens);
			if (prefs.display.postfix)
				printf("\tPostfix stack:\n");
			postfix(tokens, numTokens, &expr);//후위연산 계산.

			if (stackSize(&expr) != 1)  // 결과값(1)이외의 모든경우 error로 판단.
			{	
				printf("\tError evaluating expression\n");
			}
			else 
			{
				if (!rflag)
					printf("\t= ");
				printf("%s\n", (char*)stackTop(&expr));  //결과값 출력.
				for (i = 0; i < numTokens; i++)          //모든 변수의 초기화 실행.
				{
					if (tokens[i] == stackTop(&expr))
						tokens[i] = NULL;
				}
				free(stackPop(&expr));
			}

			for (i = 0; i < numTokens; i++)
			{
				if (tokens[i] != NULL)
					free(tokens[i]);
			}
			free(tokens);
			tokens = NULL;
			numTokens = 0;
			stackFree(&expr);


		}
		else {
			free(str);            //모드 변경후 할당 해제
			str = NULL;
		}

		str = ufgets(stdin);      //계산완료 후 재입력.
	}

	free(str);         //종료시 str 할당 해제.
	str = NULL;


	return EXIT_SUCCESS;
}
