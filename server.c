#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_PORT 7000

//连接队列最大长度
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
}write_req_t;

uv_pipe_t file_pipe;

//负责为新来的消息申请空间
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->len = suggested_size;
	buf->base = (char *)malloc(suggested_size);
}

void on_close(uv_handle_t* handle)
{
	if (handle != NULL)
		free(handle);
}

void free_write_req(uv_write_t *req)
{
	write_req_t *wr = (write_req_t*)req;

	free(wr->buf.base);
	free(wr);
}

void echo_write(uv_write_t* req, int status)
{
	if (status)
	{
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}

	free_write_req(req);
}

void write_data(uv_stream_t *dest, size_t size, uv_buf_t buf, uv_write_cb cb) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init((char*) malloc(size), size);
        memcpy(req->buf.base, buf.base, size);
        uv_write((uv_write_t*) req, (uv_stream_t*)dest, &req->buf, 1, cb);
}

//负责处理新来的消息
void echo_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
	if (nread > 0)
	{
                write_data((uv_stream_t *)&file_pipe, nread, *buf, echo_write);
		return;
	}
	else if (nread < 0)
	{
		if (nread != UV_EOF)
		{
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		}
		else
		{
			fprintf(stderr, "client disconnect\n");
		}
		uv_close((uv_handle_t*)client, on_close);
		uv_close((uv_handle_t*)&file_pipe, NULL);
	}

	if (buf->base != NULL)
	{
		free(buf->base);
	}
}

void on_new_connection(uv_stream_t* server, int status)
{
	if (status < 0)
	{
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));


		return;
	}

	uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));

	uv_tcp_init(loop, client);

	//判断accept是否成功
	if (uv_accept(server, (uv_stream_t*)client) == 0)
	{
		uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
	}
	else
	{
		uv_close((uv_handle_t*)client, NULL);
	}
}

int main(int argc, char **argv)
{

	loop = uv_default_loop();

	uv_tcp_t server;
	uv_tcp_init(loop, &server);


	uv_ip4_addr("127.0.0.1", DEFAULT_PORT, &addr);

	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);

        uv_fs_t file_req;
        int fd = uv_fs_open(loop, &file_req, argv[1], O_CREAT | O_RDWR, 0644, NULL);
        uv_pipe_init(loop, &file_pipe, 0);
        uv_pipe_open(&file_pipe, fd);

	int r = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, on_new_connection);

	if (r)
	{
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}

	uv_run(loop, UV_RUN_DEFAULT);
        return 0;
}
