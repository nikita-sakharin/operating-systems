#ifndef _HEADER_H_
#define _HEADER_H_

#include <assert.h>
#include <limits.h>

#define error_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		exit(EXIT_FAILURE);\
	}\

#define err_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		continue;\
	}\

typedef unsigned long long DebitCurrency;
typedef long long CreditCurrency;

#define DEBIT_CURRENCY_MAX ULLONG_MAX
#define CREDIT_CURRENCY_MAX LLONG_MAX
#define CREDIT_CURRENCY_MIN LLONG_MIN

#define MAX_OVERDRAFT -10000ll

#define CLIENT_ID_SIZE 80u
typedef char ClientId[CLIENT_ID_SIZE];


typedef struct
{
	DebitCurrency debit_account;
	CreditCurrency credit_account;
} Balance;

#define REQUEST_PUT_D           0
#define REQUEST_PUT_CR          1
#define REQUEST_GET_D           2
#define REQUEST_GET_CR          3
#define REQUEST_BALANCE         4
#define REQUEST_SEND_TO_SOMEONE 5

typedef struct
{
	DebitCurrency sum;
	int operation;
	ClientId identificator;
	ClientId someone;
} Request;

#define REPLY_SUCCESS          0
#define REPLY_NO_MEMORY        1
#define REPLY_MAXIMUM_CAPACITY 2
#define REPLY_INSUFFICIENTLY   3
#define REPLY_OVERDRAFT        4
#define REPLY_UNKNOW_CODE      5

typedef struct
{
	Balance funds;
	int result;
} Reply;

#endif