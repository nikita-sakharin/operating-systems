#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zmq.h>
#include "header.h"
#include "./bin_tree/bin_tree.h"

typedef struct
{
	Balance funds;
	ClientId identificator;
} ClientAccount;

int client_account_cmp(const void *, const void *);

//"tcp://*:4040"
int main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		printf("%s tcp://*:****\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int err_code;

	void *context = zmq_ctx_new();
	error_handler(context, NULL, "zmq_ctx_new()");

	void *respond = zmq_socket(context, ZMQ_REP);
	error_handler(respond, NULL, "zmq_socket()");

	err_code = zmq_bind(respond, argv[1u]);
	error_handler(err_code, -1, "zmq_connect()");

	BinTree *client_base;
	err_code = bin_tree_create(&client_base, sizeof(ClientAccount), NULL, client_account_cmp);
	error_handler(err_code != BIN_TREE_SUCCESS, 1, "bin_tree_create()");

	printf("%s online.\n", argv[1u]);

	while (1)
	{
		void *temp_data;

		Request req_struct;

		zmq_msg_t req;
		zmq_msg_init(&req);
		err_code = zmq_msg_recv(&req, respond, 0);
		err_handler(err_code, -1, "zmq_msg_recv()");
		temp_data = zmq_msg_data(&req);
		memcpy(&req_struct, temp_data, sizeof(Request));
		err_code = zmq_msg_close(&req);
		err_handler(err_code, -1, "zmq_msg_close()");

		Reply reply_struct;
		memset(&reply_struct, 0, sizeof(Reply));

		ClientAccount info = { .funds = { .debit_account = req_struct.sum, .credit_account = 0ll } };
		strcpy(info.identificator, req_struct.identificator);

		temp_data = bin_tree_data(client_base, &info);
		switch (req_struct.operation)
		{
			case REQUEST_PUT_D:
			{
				if (temp_data == NULL)
				{
					info.funds.debit_account = req_struct.sum;
					err_code = bin_tree_insert(client_base, &info);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					reply_struct.result = REPLY_SUCCESS;

					break;
				}

				if (((ClientAccount *) temp_data)->funds.debit_account > DEBIT_CURRENCY_MAX - req_struct.sum)
				{
					reply_struct.result = REPLY_MAXIMUM_CAPACITY;

					break;
				}

				((ClientAccount *) temp_data)->funds.debit_account += req_struct.sum;
				reply_struct.result = REPLY_SUCCESS;

				break;
			}
			case REQUEST_PUT_CR:
			{
				if (req_struct.sum > (DebitCurrency) CREDIT_CURRENCY_MAX)
				{
					reply_struct.result = REPLY_MAXIMUM_CAPACITY;

					break;
				}

				if (temp_data == NULL)
				{
					info.funds.credit_account = (CreditCurrency) req_struct.sum;
					err_code = bin_tree_insert(client_base, &info);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					reply_struct.result = REPLY_SUCCESS;

					break;
				}

				if (((ClientAccount *) temp_data)->funds.credit_account > CREDIT_CURRENCY_MAX - (CreditCurrency) req_struct.sum)
				{
					reply_struct.result = REPLY_MAXIMUM_CAPACITY;

					break;
				}

				((ClientAccount *) temp_data)->funds.credit_account += req_struct.sum;
				reply_struct.result = REPLY_SUCCESS;

				break;
			}
			case REQUEST_GET_D:
			{
				if (temp_data == NULL)
				{
					err_code = bin_tree_insert(client_base, &info);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					reply_struct.result = REPLY_INSUFFICIENTLY;

					break;
				}

				if (((ClientAccount *) temp_data)->funds.debit_account < req_struct.sum)
				{
					reply_struct.result = REPLY_INSUFFICIENTLY;

					break;
				}

				((ClientAccount *) temp_data)->funds.debit_account -= req_struct.sum;
				reply_struct.result = REPLY_SUCCESS;

				break;
			}
			case REQUEST_GET_CR:
			{
				if (req_struct.sum > (DebitCurrency) CREDIT_CURRENCY_MAX)
				{
					reply_struct.result = REPLY_OVERDRAFT;

					break;
				}

				if (temp_data == NULL)
				{
					if (req_struct.sum > (DebitCurrency) llabs(MAX_OVERDRAFT))
					{
						reply_struct.result = REPLY_OVERDRAFT;

						break;
					}

					info.funds.credit_account = -(CreditCurrency) req_struct.sum;
					err_code = bin_tree_insert(client_base, &info);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					reply_struct.result = REPLY_SUCCESS;

					break;
				}

				if (((ClientAccount *) temp_data)->funds.credit_account < (CreditCurrency) req_struct.sum && (-((ClientAccount *) temp_data)->funds.credit_account >= CREDIT_CURRENCY_MAX - (CreditCurrency) req_struct.sum || (CreditCurrency) req_struct.sum - ((ClientAccount *) temp_data)->funds.credit_account > llabs(MAX_OVERDRAFT)))
				{
					reply_struct.result = REPLY_OVERDRAFT;

					break;
				}

				((ClientAccount *) temp_data)->funds.credit_account -= (CreditCurrency) req_struct.sum;
				reply_struct.result = REPLY_SUCCESS;

				break;
			}
			case REQUEST_BALANCE:
			{
				if (temp_data == NULL)
				{
					reply_struct.funds = info.funds;
					err_code = bin_tree_insert(client_base, &info);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					reply_struct.result = REPLY_SUCCESS;

					break;
				}

				reply_struct.funds = ((ClientAccount *) temp_data)->funds;
				reply_struct.result = REPLY_SUCCESS;

				break;
			}
			case REQUEST_SEND_TO_SOMEONE:
			{
				void *temp_data_someone;

				ClientAccount someone = { .funds = { .debit_account = 0ull, .credit_account = 0ll } };
				strcpy(someone.identificator, req_struct.someone);

				temp_data_someone = bin_tree_data(client_base, &someone);
				if (temp_data_someone == NULL)
				{
					err_code = bin_tree_insert(client_base, &someone);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					temp_data_someone = bin_tree_data(client_base, &someone);
				}

				if (temp_data == NULL)
				{
					err_code = bin_tree_insert(client_base, &info);
					if (err_code != BIN_TREE_SUCCESS)
					{
						reply_struct.result = REPLY_NO_MEMORY;

						break;
					}

					reply_struct.result = REPLY_INSUFFICIENTLY;

					break;
				}

				if (((ClientAccount *) temp_data)->funds.debit_account < req_struct.sum)
				{
					reply_struct.result = REPLY_INSUFFICIENTLY;

					break;
				}

				if (((ClientAccount *) temp_data_someone)->funds.debit_account > DEBIT_CURRENCY_MAX - req_struct.sum)
				{
					reply_struct.result = REPLY_MAXIMUM_CAPACITY;

					break;
				}

				((ClientAccount *) temp_data)->funds.debit_account -= req_struct.sum;
				((ClientAccount *) temp_data_someone)->funds.debit_account += req_struct.sum;
				reply_struct.result = REPLY_SUCCESS;

				break;
			}
			default:
			{
				reply_struct.result = REPLY_UNKNOW_CODE;

				break;
			}
		}

		zmq_msg_t reply;
		err_code = zmq_msg_init_size(&reply, sizeof(Reply));
		err_handler(err_code, -1, "zmq_msg_init_size()");
		temp_data = zmq_msg_data(&reply);
		memcpy(temp_data, &reply_struct, sizeof(Reply));
		err_code = zmq_msg_send(&reply, respond, 0);
		err_handler(err_code, -1, "zmq_msg_send()");
		zmq_msg_close(&reply);
		err_handler(err_code, -1, "zmq_msg_close()");
	}

	printf("%s offline.\n", argv[1u]);

	bin_tree_destroy(client_base);

	err_code = zmq_close(respond);
	error_handler(err_code, -1, "zmq_close()");

	err_code = zmq_ctx_destroy(context);
	error_handler(err_code, -1, "zmq_ctx_destroy()");

	return 0;
}

int client_account_cmp(const void *s1, const void *s2)
{
	return strcmp(((ClientAccount *) s1)->identificator, ((ClientAccount *) s2)->identificator);
}