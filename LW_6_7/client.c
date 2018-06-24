#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zmq.h>
#include "header.h"
#include "./bin_tree/bin_tree.h"

#define BUFFER_SIZE 8192u
#define BUFFER_SIZE_ "8191"

#define CLIENT_ID_SIZE_ "79"

void reply_print(const Reply *, int);

//"tcp://localhost:4040"
int main(int argc, const char *argv[])
{
	if (argc < 3)
	{
		printf("%s client_id tcp://localhost:****\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const size_t length = strlen(argv[1u]);
	if (length >= CLIENT_ID_SIZE)
	{
		printf("strlen(\"%s\") >= %zu\n", argv[1u], CLIENT_ID_SIZE);
		exit(EXIT_FAILURE);
	}

	int err_code;

	void *context = zmq_ctx_new();
	error_handler(context, NULL, "zmq_ctx_new()");

	void *request = zmq_socket(context, ZMQ_REQ);
	error_handler(request, NULL, "zmq_socket()");

	err_code = zmq_connect(request, argv[2u]);
	error_handler(err_code, -1, "zmq_connect()");

	printf("[Start session] client: %s, address: %s.\n", argv[1u], argv[2u]);

	while (1)
	{
		char command[BUFFER_SIZE];
		err_code = scanf("%"BUFFER_SIZE_"s", command);
		if (err_code == EOF)
		{
			break;
		}

		void *temp_data;

		Request req_struct;
		memset(&req_struct, 0, sizeof(Request));
		strcpy(req_struct.identificator, argv[1u]);

		if (!strcmp(command, "put_d"))
		{
			printf("Enter sum: ");
			err_code = scanf("%llu", &req_struct.sum);
			if (err_code == EOF)
			{
				puts("Incorrect input");
				continue;
			}

			req_struct.operation = REQUEST_PUT_D;
			printf("put_d %llu\n", req_struct.sum);
		}
		else if (!strcmp(command, "put_cr"))
		{
			printf("Enter sum: ");
			err_code = scanf("%llu", &req_struct.sum);
			if (err_code == EOF)
			{
				puts("Incorrect input");
				continue;
			}

			req_struct.operation = REQUEST_PUT_CR;
			printf("put_cr %llu\n", req_struct.sum);
		}
		else if (!strcmp(command, "get_d"))
		{
			printf("Enter sum: ");
			err_code = scanf("%llu", &req_struct.sum);
			if (err_code == EOF)
			{
				puts("Incorrect input");
				continue;
			}

			req_struct.operation = REQUEST_GET_D;
			printf("get_d %llu\n", req_struct.sum);
		}
		else if (!strcmp(command, "get_cr"))
		{
			printf("Enter sum: ");
			err_code = scanf("%llu", &req_struct.sum);
			if (err_code == EOF)
			{
				puts("Incorrect input");
				continue;
			}

			req_struct.operation = REQUEST_GET_CR;
			printf("get_cr %llu\n", req_struct.sum);
		}
		else if (!strcmp(command, "send_to_someone"))
		{
			printf("Enter sum: ");
			err_code = scanf("%llu", &req_struct.sum);
			if (err_code == EOF)
			{
				puts("Incorrect input");
				continue;
			}

			printf("Enter id: ");
			err_code = scanf("%"CLIENT_ID_SIZE_"s", req_struct.someone);
			if (err_code == EOF)
			{
				puts("Incorrect input");
				continue;
			}

			req_struct.operation = REQUEST_SEND_TO_SOMEONE;
			printf("send_to_someone %llu %s\n", req_struct.sum, req_struct.someone);
		}
		else if (!strcmp(command, "balance"))
		{
			req_struct.operation = REQUEST_BALANCE;
			printf("balance\n");
		}
		else
		{
			puts("Unknow command");
		}

		zmq_msg_t req;
		err_code = zmq_msg_init_size(&req, sizeof(Request));
		err_handler(err_code, -1, "zmq_msg_init_size()");
		temp_data = zmq_msg_data(&req);
		memcpy(temp_data, &req_struct, sizeof(Request));
		err_code = zmq_msg_send(&req, request, 0);
		err_handler(err_code, -1, "zmq_msg_send()");
		err_code = zmq_msg_close(&req);
		err_handler(err_code, -1, "zmq_msg_close()");

		Reply reply_struct;
		zmq_msg_t reply;
		zmq_msg_init(&reply);
		err_code = zmq_msg_recv(&reply, request, 0);
		err_handler(err_code, -1, "zmq_msg_recv()");
		temp_data = zmq_msg_data(&reply);
		memcpy(&reply_struct, temp_data, sizeof(Reply));
		err_code = zmq_msg_close(&reply);
		err_handler(err_code, -1, "zmq_msg_close()");

		reply_print(&reply_struct, req_struct.operation);
		putchar('\n');
	}

	printf("[Finish session].\n");

	err_code = zmq_close(request);
	error_handler(err_code, -1, "zmq_close()");

	err_code = zmq_ctx_destroy(context);
	error_handler(err_code, -1, "zmq_ctx_destroy()");

	return 0;
}

void reply_print(const Reply * restrict reply, int operation)
{
	switch (reply->result)
	{
		case REPLY_SUCCESS:
		{
			puts("Ok");
			if (operation == REQUEST_BALANCE)
			{
				printf("Debit account balance: %llu\n", reply->funds.debit_account);
				printf("Credit account balance: %lld\n", reply->funds.credit_account);
			}
			break;
		}
		case REPLY_NO_MEMORY:
		{
			puts("Insufficient memory");
			break;
		}
		case REPLY_MAXIMUM_CAPACITY:
		{
			puts("Too large sum");
			break;
		}
		case REPLY_INSUFFICIENTLY:
		{
			puts("Insufficient funds");
			break;
		}
		case REPLY_OVERDRAFT:
		{
			puts("Credit account overdraft");
			break;
		}
		case REPLY_UNKNOW_CODE:
		{
			puts("Unknowing code was sending");
		}
		default:
		{
			puts("Unknowing error");
			break;
		}
	}
}