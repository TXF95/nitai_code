#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: nitai_reverse_httpd/0.4\r\n"

#define N 3

void *accept_request(void* pclient);

//response
void bad_request(int);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *);
void not_found(int);

//utils
int get_line(int, char *, int);
void headers(int, const char *);
void cat(int, FILE *);
void serve_file(int, const char *);
int startup(u_short *);
void unimplemented(int);
int connect_backend(char *ipaddr,int port);
void request_backend(int client,int backend_sock,char* method,char* url,char* ipaddr);


/**********************************************************************/
/* connect the to backend server 
 * return.  socket fd 
 * Parameters: backend server's ip address ,port */
/**********************************************************************/
int connect_backend(char *ipaddr,int port){
	int sockfd;
	struct sockaddr_in address;
	int result;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ipaddr);
	address.sin_port = htons(port);
	result = connect(sockfd,(struct sockaddr*)&address,sizeof(address));
	if(result==-1){
		perror("oops:connect to backend server failed");
		return 1;
	}
	return sockfd;
}


/**********************************************************************/
/* send request from proxy to backend server
 * return.  backend server response
 * Parameters: backend server's ip address ,port */
/**********************************************************************/
void request_backend(int client,int backend_sock,char* method,char* url,char* ipaddr){
	char httpstring[2048];
	char http_buf[1024];
	char http_response[1024*80];

	memset(http_buf,0,sizeof(http_buf));
	sprintf(httpstring,"%s %s HTTP/1.1\r\n"
			"Host:%s\r\n"
			"Connection:Close\r\n\r\n"
			"Content-Type:Application/octet-stream\r\n\r\n",
			method,url,ipaddr);

	write(backend_sock,httpstring,strlen(httpstring));

	int i=0;
	int rev = -1;
	int length = 0;
	while(rev = read(backend_sock,http_buf,sizeof(http_buf)))
	{
		length += rev;
		send(client,http_buf,rev,0);//网页显示后端服务器的返回结果
		printf("%s",http_buf);
	    memset(http_buf,0,sizeof(http_buf));
	}
	printf("total length is %d\n",length);
	close(backend_sock);//关闭连接
}




/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
void *accept_request(void* pclient)
{
	char buf[1024];
	int numchars;
	char method[255];
	char url[255];
	char path[512];
	size_t i, j;
	struct stat st;
	int cgi = 0;      /* becomes true if server decides this is a CGI program */
	char *query_string = NULL;
	int client = *(int*)pclient;
	int n,k,a[N];

	numchars = get_line(client, buf, sizeof(buf));
	i = 0; 
	j = 0;
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
	 {
	 	 method[i] = buf[j];
	  	 i++; j++;
	 }
	 method[i] = '\0';
/*
	 if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))

		  unimplemented(client);
		  return NULL;
 	 }	

	 if (strcasecmp(method, "POST") == 0)
     cgi = 1;
*/
	 i = 0;
	 while (ISspace(buf[j]) && (j < sizeof(buf)))
		  j++;
	 while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	 {
		  url[i] = buf[j];
		  i++; j++;
 	 }
	 url[i] = '\0';

	 //这里添加后端服务器，有几个定义几个然后调用
	 //connect_backend函数
	 //继续调用request_backend
	// int n,k,m,a[N];
	 const char* ip[N] = {"192.168.137.100",
		 				  "192.168.137.101",
						  "192.168.137.102"};
	int port[N] = {80,8081,80};
	//char *response_buf[N];
	int backend_sock_fd[N];
	
	srand(time(0));
	for(n=0;n<N;n++)
	{
		a[n] = n;
	}

	n=0;
	while(n<=N)
	{
		k = rand()%N;
		if(a[n]!=-1)
		{
			backend_sock_fd[k] = connect_backend(ip[k],port[k]);
			//response_buf[n] = request_backend(client,backend_sock_fd[n],method,url,ip[n]);
			request_backend(client,backend_sock_fd[k],method,url,ip[k]);
			//printf("response_buf%d:%s",n,response_buf[n]);
			a[n] = -1;
			n++;

			//if((++n)==N)
			//	break;
		}
	}
	
	/* 
	 const char* ip1 = "192.168.137.100";
	 int port1 = 80;

	 const char* ip2 = "192.168.137.102";
	 int port2 = 80;

	 const char* ip3 = "192.168.137.101"; 
	 int port3 = 8081;

	int backend_sock_fd3 = connect_backend(ip3,port3);
	request_backend(client,backend_sock_fd3,method,url,ip3);

	 int backend_sock_fd1 = connect_backend(ip1,port1);
	 request_backend(client,backend_sock_fd1,method,url,ip1);

	 int backend_sock_fd2 = connect_backend(ip2,port2);
	 request_backend(client,backend_sock_fd2,method,url,ip2);

*/
	// int backend_sock_fd3 = connect_backend(ip3,port3);
	// request_backend(client,backend_sock_fd3,method,url,ip3);



/**********************************************************************/
	 /*
	 if (strcasecmp(method, "GET") == 0)
	 {
		  query_string = url;
		  while ((*query_string != '?') && (*query_string != '\0'))
			   query_string++;
		  if (*query_string == '?')
		  {
			   cgi = 1;
			   *query_string = '\0';
			   query_string++;
  		  }
 	 }

	 sprintf(path, "htdocs%s", url);
	 if (path[strlen(path) - 1] == '/')
		  strcat(path, "index.html");
	 if (stat(path, &st) == -1) {
	 	 while ((numchars > 0) && strcmp("\n", buf))  // read & discard headers 
			   numchars = get_line(client, buf, sizeof(buf));
	     not_found(client);
 	 }
 	 else
	 {
		  if ((st.st_mode & S_IFMT) == S_IFDIR)
			   strcat(path, "/index.html");
		  if ((st.st_mode & S_IXUSR) ||
		      (st.st_mode & S_IXGRP) ||
		      (st.st_mode & S_IXOTH)    )
				   cgi = 1;
		  if (!cgi)
			   serve_file(client, path);
		  else
			   execute_cgi(client, path, method, query_string);
 	 }
*/
/*
 * for(n=0;n<N;n++)
	{
		free(response_buf[n]);
	}
*/

	 close(client);
	 return NULL;
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void bad_request(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "<P>Your browser sent a bad request, ");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "such as a POST without a Content-Length.\r\n");
 send(client, buf, sizeof(buf), 0);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/
void cat(int client, FILE *resource)
{
 char buf[1024];

 fgets(buf, sizeof(buf), resource);
 while (!feof(resource))
 {
  send(client, buf, strlen(buf), 0);
  fgets(buf, sizeof(buf), resource);
 }
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
 perror(sc);
 exit(1);
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi(int client, const char *path,
                 const char *method, const char *query_string)
{
 char buf[1024];
 int cgi_output[2];
 int cgi_input[2];
 pid_t pid;
 int status;
 int i;
 char c;
 int numchars = 1;
 int content_length = -1;

 buf[0] = 'A'; buf[1] = '\0';
 if (strcasecmp(method, "GET") == 0)
  while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
   numchars = get_line(client, buf, sizeof(buf));
 else    /* POST */
 {
  numchars = get_line(client, buf, sizeof(buf));
  while ((numchars > 0) && strcmp("\n", buf))
  {
   buf[15] = '\0';
   if (strcasecmp(buf, "Content-Length:") == 0)
    content_length = atoi(&(buf[16]));
   numchars = get_line(client, buf, sizeof(buf));
  }
  if (content_length == -1) {
   bad_request(client);
   return;
  }
 }

 sprintf(buf, "HTTP/1.0 200 OK\r\n");
 send(client, buf, strlen(buf), 0);

 if (pipe(cgi_output) < 0) {
  cannot_execute(client);
  return;
 }
 if (pipe(cgi_input) < 0) {
  cannot_execute(client);
  return;
 }

 if ( (pid = fork()) < 0 ) {
  cannot_execute(client);
  return;
 }
 if (pid == 0)  /* child: CGI script */
 {
  char meth_env[255];
  char query_env[255];
  char length_env[255];

  dup2(cgi_output[1], 1);
  dup2(cgi_input[0], 0);
  close(cgi_output[0]);
  close(cgi_input[1]);
  sprintf(meth_env, "REQUEST_METHOD=%s", method);
  putenv(meth_env);
  if (strcasecmp(method, "GET") == 0) {
   sprintf(query_env, "QUERY_STRING=%s", query_string);
   putenv(query_env);
  }
  else {   /* POST */
   sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
   putenv(length_env);
  }
  execl(path, path, NULL);
  exit(0);
 } else {    /* parent */
  close(cgi_output[1]);
  close(cgi_input[0]);
  if (strcasecmp(method, "POST") == 0)
   for (i = 0; i < content_length; i++) {
    recv(client, &c, 1, 0);
    write(cgi_input[1], &c, 1);
   }
  while (read(cgi_output[0], &c, 1) > 0)
   send(client, &c, 1, 0);

  close(cgi_output[0]);
  close(cgi_input[1]);
  waitpid(pid, &status, 0);
 }
}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int get_line(int sock, char *buf, int size)
{
 int i = 0;
 char c = '\0';
 int n;

 while ((i < size - 1) && (c != '\n'))
 {
  n = recv(sock, &c, 1, 0);
  /* DEBUG printf("%02X\n", c); */
  if (n > 0)
  {
   if (c == '\r')
   {
    n = recv(sock, &c, 1, MSG_PEEK);
    /* DEBUG printf("%02X\n", c); */
    if ((n > 0) && (c == '\n'))
     recv(sock, &c, 1, 0);
    else
     c = '\n';
   }
   buf[i] = c;
   i++;
  }
  else
   c = '\n';
 }
 buf[i] = '\0';
 
 return(i);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
void headers(int client, const char *filename)
{
 char buf[1024];
 (void)filename;  /* could use filename to determine file type */

 strcpy(buf, "HTTP/1.0 200 OK\r\n");
 send(client, buf, strlen(buf), 0);
 strcpy(buf, SERVER_STRING);
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 strcpy(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, SERVER_STRING);
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "your request because the resource specified\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "is unavailable or nonexistent.\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</BODY></HTML>\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/
void serve_file(int client, const char *filename)
{
 FILE *resource = NULL;
 int numchars = 1;
 char buf[1024];

 buf[0] = 'A'; buf[1] = '\0';
 while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
  numchars = get_line(client, buf, sizeof(buf));

 resource = fopen(filename, "r");
 if (resource == NULL)
  not_found(client);
 else
 {
  headers(client, filename);
  cat(client, resource);
 }
 fclose(resource);
}

/**********************************************************************/
/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
/**********************************************************************/
int startup(u_short *port)
{
	 int httpd = 0;
	 struct sockaddr_in name;

	 httpd = socket(PF_INET, SOCK_STREAM, 0);
	 if (httpd == -1)
		  error_die("socket");
	 memset(&name, 0, sizeof(name));
	 name.sin_family = AF_INET;
	 name.sin_port = htons(*port);
	 name.sin_addr.s_addr = htonl(INADDR_ANY);
	 if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		  error_die("bind");
	 
	 /*
	 if (*port == 0)  // if dynamically allocating a port 
	 {
		  socklen_t namelen = sizeof(name);
	 	  if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
			   error_die("getsockname");
		  *port = ntohs(name.sin_port);
 	 }*/
 	 if (listen(httpd, 5) < 0)
		  error_die("listen");
	 return(httpd);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
void unimplemented(int client)
{
 	char buf[1024];

	 sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, SERVER_STRING);
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "Content-Type: text/html\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "</TITLE></HEAD>\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "</BODY></HTML>\r\n");
	 send(client, buf, strlen(buf), 0);
}


int main(void)
{
     int server_sock = -1;
	 u_short port = 12345;
	 int client_sock = -1;
	 struct sockaddr_in client_name;
	 socklen_t client_name_len = sizeof(client_name);
	 pthread_t newthread;

	 server_sock = startup(&port);
	 printf("httpd running on port %d\n", port);

	 while (1)
	 {
		  client_sock = accept(server_sock,
                       (struct sockaddr *)&client_name,
                       &client_name_len);
		  if (client_sock == -1)
			   error_die("accept");
 
		  if (pthread_create(&newthread , NULL, accept_request,(void*)&client_sock) != 0)
			   perror("pthread_create");
 	 }
	 close(server_sock);
	 return(0);
}
