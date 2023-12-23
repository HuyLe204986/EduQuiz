#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define BACKLOG 5 // số kết nối có thể đợi
#define MAX 8192 
#define QUES_FILENAME "question.txt"
#define QUES_IN_LEVER 10
#define QUES_NUMBER 30
// account node

typedef struct node
{
	char username[MAX];
	char password[MAX];
	int status; // trạng thái của tài khoản. 0: khóa
	int point;
	struct node *next;
} node_t;

typedef struct question
{
	int id;
	int level; //1 : easy, 2 : medium, 3 : hard
	char content[200];
	char choiceA[50];
	char choiceB[50];
	char choiceC[50];
	char choiceD[50];
	char answer;
} Question;

typedef struct message
{
	char content[8192];
	char answer[11];
} Message;

Question questionList[QUES_NUMBER];
int easyList[QUES_IN_LEVER]; // danh sách id các câu hỏi dễ
int mediumList[QUES_IN_LEVER]; // tương tự
int hardList[QUES_IN_LEVER]; // tương tự
int easyIndex = 0, mediumIndex = 0, hardIndex = 0; // theo dõi vị trí hiện tại của mảng đối với từng cấp độ khó

// load data from text file to linked list
node_t *load_data(char *filename)
{
    // status, point: thông tin về trạng thái và điểm số của user từ dữ liệu đọc từ file văn bản
    // count: Đếm số lượng tài khoản người dùng đã đọc từ tệp
	int status, point, count = 0; // number of accounts
	FILE *f;
	char username[MAX], password[MAX];
	node_t *head, *current;
	head = current = NULL;

	// open file data
	printf("Loading data from file...\n");

	if ((f = fopen(filename, "r")) == NULL)
	{
		printf("Cannot open file!\n");
		exit(0);
	}

	// load accounts to linked list
	while (fscanf(f, "%s %s %d %d\n", username, password, &status, &point) != EOF)
	{
		// create new node
		node_t *node = malloc(sizeof(node_t));
		strcpy(node->username, username);
		strcpy(node->password, password);
		node->status = status;
		node->point = point;
		// add node to list
		if (head == NULL)
			current = head = node;
		else
			current = current->next = node;
		count++;
	}

	fclose(f);
	printf("LOADED SUCCESSFULY %d ACCOUNT(S)\n", count);
	return head;
}

// find a node exist in linked list given username
node_t *find_node(node_t *head, char *username)
{
	node_t *current = head;
	while (current != NULL)
	{
		if (0 == strcmp(current->username, username))
			return current;
		current = current->next;
	}
	return NULL;
}

// save list to text file
void save_list(node_t *head, char *filename)
{
	FILE *f;
	f = fopen(filename, "w");
	node_t *current;
	for (current = head; current; current = current->next)
		fprintf(f, "%s %s %d %d\n", current->username, current->password, current->status, current->point);
	fclose(f);
}

int addLeverList(int id, int lv)
{
	switch (lv)
	{
	case 1:
		easyList[easyIndex] = id;
		easyIndex++;
		break;
	case 2:
		mediumList[mediumIndex] = id;
		mediumIndex++;
		break;
	case 3:
		hardList[hardIndex] = id;
		hardIndex++;
		break;
	default:
		break;
	}
	return 1;
}
void lastReq(node_t *head,char* bufff);
void readQues()
{
	FILE *fptr;
    // khai báo id, cấp độ, nội dung câu hỏi, các lựa chọn a,c,b,d và đáp án đúng của câu hỏi
	int id, lv;
	char content[1000], a[1000], b[1000], c[1000], d[1000];
	char answ;
    //----------
	int i = 0;
	fptr = fopen(QUES_FILENAME, "r");
	if (fptr == NULL)
	{
		printf("Can't open file !\n");
		return;
	}

    // đọc id, level, nội dung câu hỏi, các phương án và đáp án đúng
	while (fscanf(fptr, "%d | %d | %[^|] | %[^|] | %[^|] | %[^|] | %[^|] | %c", &id, &lv, content, a, b, c, d, &answ) != EOF)
	{
		questionList[i].id = id;
		questionList[i].level = lv;
		strcpy(questionList[i].content, content);
		strcpy(questionList[i].choiceA, a);
		strcpy(questionList[i].choiceB, b);
		strcpy(questionList[i].choiceC, c);
		strcpy(questionList[i].choiceD, d);
		questionList[i].answer = answ;
		addLeverList(id, lv);
		i++;
	}
	fclose(fptr);
}
/*
void randomId(int *arr, int lv)
{
	if (lv == 1)
	{
		arr[0] = rand() % 9; // tạo 1 số ngẫu nhiên từ 0-8 gán vào ptu đầu tiên của mảng
		do
		{
			arr[1] = rand() % 9;
		} while (arr[1] == arr[0]); // tạo 1 số ngẫu nhiên từ 0-8 khác ptu đầu tiên gán vào ptu thứ 2 của mảng
		do
		{
			arr[2] = rand() % 9;
		} while (arr[2] == arr[0] || arr[2] == arr[1]); // tương tự
		do
		{
			arr[3] = rand() % 9;
		} while (arr[3] == arr[0] || arr[3] == arr[1] || arr[3] == arr[2]);
		return;
	}
	if (lv == 2)
	{
		arr[0] = rand() % 9;
		do
		{
			arr[1] = rand() % 9;
		} while (arr[1] == arr[0]);
		do
		{
			arr[2] = rand() % 9;
		} while (arr[2] == arr[0] || arr[2] == arr[1]);
		do
		{
			arr[3] = rand() % 9;
		} while (arr[3] == arr[0] || arr[3] == arr[1] || arr[3] == arr[2]);
		return;
	}
	if (lv == 3)
	{
		arr[0] = rand() % 9;
		do
		{
			arr[1] = rand() % 9;
		} while (arr[1] == arr[0]);
		return;
	}
}
*/

void randomId(int *arr, int lv) {
    int lv1Count = (lv == 1) ? 4 : 0;
    int lv2Count = (lv == 2) ? 4 : 0;
    int lv3Count = (lv == 3) ? 2 : 0;

    int lv1Index = 0, lv2Index = 0, lv3Index = 0;
    int generated[QUES_NUMBER] = {0}; // Mảng để theo dõi các số đã sinh

    while (lv1Index < lv1Count || lv2Index < lv2Count || lv3Index < lv3Count) {
        int num = rand() % QUES_NUMBER; // Sinh số ngẫu nhiên từ 0 đến QUES_NUMBER - 1

        if (generated[num] == 0) {
            if (lv1Index < lv1Count && questionList[num].level == 1) {
                arr[lv1Index++] = num;
            } else if (lv2Index < lv2Count && questionList[num].level == 2) {
                arr[lv1Count + lv2Index++] = num;
            } else if (lv3Index < lv3Count && questionList[num].level == 3) {
                arr[lv1Count + lv2Count + lv3Index++] = num;
            }
            generated[num] = 1;
        }
    }
}

Message makeQuesList()
{
	int easy[4]; // chr
	int medium[4];
	int hard[2];
	int i, n;
	Message buff; // Khởi tạo một biến cấu trúc Message để chứa nội dung và câu trả lời
	srand((int)time(0));  // Khởi tạo bộ tạo số ngẫu nhiên
	randomId(easy, 1); 
	randomId(medium, 2);
	randomId(hard, 3);
	for (i = 0; i < 4; i++)
	{ 
		n = easyList[easy[i]];  // lấy ra id của câu hỏi dựa trên số ngẫu nhiên đã sinh
		strcat(buff.content, questionList[n].content); // lấy nội dung câu hỏi gán cho buff.content
		strcat(buff.content, "\n");
		strcat(buff.content, questionList[n].choiceA); // thêm nội dung các câu trả lời
		strcat(buff.content, "\t");
		strcat(buff.content, questionList[n].choiceB);
		strcat(buff.content, "\n");
		strcat(buff.content, questionList[n].choiceC);
		strcat(buff.content, "\t");
		strcat(buff.content, questionList[n].choiceD);
		strcat(buff.content, "\n\n");
		buff.answer[i] = questionList[n].answer; // lưu lại đáp án đúng
		//printf("%s\n",questionList[n].content);
	}
	for (i = 0; i < 4; i++)
	{
		n = mediumList[medium[i]]; 
		strcat(buff.content, questionList[n].content);
		strcat(buff.content, "\n");
		strcat(buff.content, questionList[n].choiceA);
		strcat(buff.content, "\t");
		strcat(buff.content, questionList[n].choiceB);
		strcat(buff.content, "\n");
		strcat(buff.content, questionList[n].choiceC);
		strcat(buff.content, "\t");
		strcat(buff.content, questionList[n].choiceD);
		strcat(buff.content, "\n\n");
		buff.answer[i + 4] = questionList[n].answer;
		//printf("%s\n",questionList[n].content);
	}
	for (i = 0; i < 2; i++)
	{
		n = hardList[hard[i]];
		strcat(buff.content, questionList[n].content);
		strcat(buff.content, "\n");
		strcat(buff.content, questionList[n].choiceA);
		strcat(buff.content, "\t");
		strcat(buff.content, questionList[n].choiceB);
		strcat(buff.content, "\n");
		strcat(buff.content, questionList[n].choiceC);
		strcat(buff.content, "\t");
		strcat(buff.content, questionList[n].choiceD);
		strcat(buff.content, "\n\n");
		//printf("%s\n",questionList[n].content);
		buff.answer[i + 8] = questionList[n].answer;
	}
	//printf("%s",buff.content);
	return buff;
}
// so sánh đáp án của người dùng (req) với đáp án đúng(local)
int checkAnswer(char *req, char *local)
{
	int point = 0;
	for (int i = 0; i < 10; i++)
	{
		if (req[i] == local[i])
			point++;
	}
	return point;
}
int main(int argc, char const *argv[])
{
	char filename[] = "account.txt";
	readQues();
	//printf("%s",buff.content);
	// valid number of argument
	if (argc != 2)
	{
		printf("Usage: ./server PortNumber\n\n");
		return 0;
	}

    /**
    listen_sock:  socket mà máy chủ sẽ sử dụng để lắng nghe các yêu cầu kết nối từ các máy khách.
    conn_sock: đại diện cho socket của mỗi kết nối được chấp nhận sau khi máy chủ chấp nhận kết nối.
    username, password, answer, point: lưu các thông tin về người dùng
    reply: Một con trỏ đến ký tự dùng để lưu trữ thông điệp phản hồi gửi cho máy khách.
    bee: Một mảng ký tự dùng để lưu trữ thông điệp cho mục đích gửi lại cho máy khách.
    */
	int listen_sock, conn_sock, n;
	char username[MAX], password[MAX], answer[11], point[3], *reply,bee[1024]={0};
	int bytes_sent, bytes_received; // Số byte được gửi và nhận qua kết nối socket.
	struct sockaddr_in server; // cấu trúc để lưu trữ thông tin về địa chỉ và port của máy chủ.
	struct sockaddr_in client; // ... máy khách.
	socklen_t sin_size; // dùng để đặt kích thước của cấu trúc sockaddr_in.
	node_t *found; // Một con trỏ đến cấu trúc dữ liệu để tham chiếu đến thông tin tài khoản được tìm thấy trong danh sách tài khoản.
	int pid; //  ID quy trình của tiến trình con,phân biệt giữa tiến trình con và cha(để xử lý các yêu cầu kết nối)

	// load file txt to linked list
	node_t *account_list = load_data(filename);

	// Construct a TCP socket to listen connection request
    /*
       socket(): trả về 1 số nguyên nếu tạo 1 socket thành công. Với các tham số:
       - AF_INET: Định nghĩa kiểu giao thức Internet.
       - SOCK_STREAM: Chọn loại socket là SOCK_STREAM để sử dụng giao thức TCP
       - 0: ối số này thường không cần thiết và được sử dụng cho các tuỳ chọn nâng cao.
    */
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	// Bind address to socket
	memset(&server, '\0', sizeof server); // đặt tất cả byte của biến server về giá trị 0. 
	server.sin_family = AF_INET; // Đặt loại địa chỉ là AF_INET
	server.sin_port = htons(atoi(argv[1])); // Gán cổng (port) cho server từ tham số được truyền vào từ dòng lệnh 
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Gắn địa chỉ IP cho socket server. 
    /*INADDR_ANY thường được sử dụng để lắng nghe trên tất cả các giao diện mạng có sẵn trên máy chủ, 
    cho phép máy chủ lắng nghe các kết nối đến từ bất kỳ địa chỉ IP nào trên máy chủ. */

    /*
        bind(): gắn địa chỉ đã được cấu hình (server) cho socket server (listen_sock). thành công trả về 0
    */
	if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

    // Listen request from client
    /**
    listen: bắt đầu lắng nghe các yêu cầu kết nối đến socket listen_sock. trả về 0 nếu thành công
    BACKLOG: chỉ định số lượng tối đa các kết nối đang chờ xử lý.
    */
	if (listen(listen_sock, BACKLOG) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	puts("Server up and running...\n");

	// Communicate with client
	while (1)
	{
		// accept request
		// chấp nhận kết nối từ client đến server
		/**
		accept():  tạo ra một socket mới (conn_sock) để giao tiếp với client này. Nó đợi và chấp nhận kết nối từ client.
		tạo ra một socket mới (conn_sock) để giao tiếp với client cụ thể đã kết nối tới server nếu thành công hoặc -1 nếu thất bại
		*/
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
			perror("\nError: ");

		printf("\nYou got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

		// start conversation on other fork
		// tạo tiến trình con mới từ tiến trình hiện tại
		/*
			sau khi tạo pid sẽ khác 0 ở tt cha và = 0 ở tt con
			< 0 nếu tạo thất bại
		*/
		pid = fork();
		if (pid < 0)
		{
			perror("Error");
			return 1;
		}

		// on child process
		if (pid == 0)
		{
			int count = 0; // count password repeatation
			while (1)
			{
				/*
				nhận dữ liệu từ socket. nhận các thông điệp từ client và lưu vào buffer username.
				*/
				if (0 >= (bytes_received = recv(conn_sock, username, MAX - 1, 0)))
				{
					printf("\nConnection closed 1\n");
					break;
				}
				username[bytes_received] = '\0';

				// check username existence
				if ((found = find_node(account_list, username)))
				{
					if (found->status == 1)
						reply = "1"; // username found
					else
						reply = "2"; // username found but has been locked
				}
				else
					reply = "0"; // username not found

				// echo to client
				if (0 >= (bytes_sent = send(conn_sock, reply, strlen(reply), 0)))
				{
					printf("\nConnection closed 2\n");
					break;
				}

				

				while (1)
				{
					// receive password
					memset(password, '\0', MAX);
					if (0 >= (bytes_received = recv(conn_sock, password, MAX - 1, 0)))
					{
						printf("\nConnection closed 3\n");
						break;
					}
					password[bytes_received] = '\0';

					// validate password
					if (0 == strcmp(found->password, password))
					{
						Message buff = makeQuesList();
						reply = buff.content;
						//printf("Đáp án:\n%s\n", buff.answer);
						// gửi câu hỏi về client
						if (0 >= (bytes_sent = send(conn_sock, reply, strlen(reply), 0)))
						{
							printf("\nConnection closed 4\n");
							break;
						}
						// nhân đáp án từ client
						if (0 >= (bytes_received = recv(conn_sock, answer, 11, 0)))
						{
							printf("\nConnection closed 5\n");
							break;
						}
						n = checkAnswer(answer, buff.answer);
						if (found->point < n)
							found->point = n;
						save_list(account_list, filename);

						/*
						Chuyển đổi giá trị số nguyên n thành chuỗi ký tự và lưu vào point, 
						đảm bảo rằng chuỗi này sẽ có độ dài tối đa là 2 ký tự (ký tự số và ký tự kết thúc chuỗi '\0').
						*/
						snprintf(point, 3, "%d", n); 
						strcat(bee,point); // nối chuỗi point vào chuỗi bee
						strcat(bee,"/10");
						strcat(bee,"\n\n");
						lastReq(account_list,bee);
						printf("%s\n",bee);
						// gửi thông điệp phản hồi trong bee cho client
						if (0 >= (bytes_sent = send(conn_sock, bee,strlen(bee), 0)))
						{
							printf("\nConnection closed 6\n");
							break;
						}
					}
					else
					{
						count++;
						if (count == 3)
						{
							reply = "2";	   // wrong pass 3 times, reply 2
							found->status = 0; // then lock account
							
						}
						else
							reply = "0"; // wrong pass < 3 times, reply 0
							// gửi phản hồi cho client
						if (0 >= (bytes_sent = send(conn_sock, reply, strlen(reply), 0)))
						{
							printf("\nConnection closed 7\n");
							break;
						}
						break;
					}

					// echo to client
				}
			}
			// save linked list state
			save_list(account_list, filename);
			close(conn_sock);
		}
		else
		{
			// on parent process
			close(conn_sock);
		}
	}
	close(listen_sock);
	return 0;
}
void lastReq(node_t *head,char* bufff){
   node_t *current;
   int m1=1,m2=1,m3=1;
   //x1, x2,x3 để lưu thông tin về xếp hạng 1, 2 và 3.
   // n[3]: chuyển đổi các số nguyên (điểm số của người chơi) thành chuỗi ký tự.
   char x1[300]={0},x2[300]={0},x3[300]={0},n[3];

   // lấy ra 3 điểm số cao nhất
	for (current = head; current; current = current->next)
		if(current->point > m1) m1 = current->point;
	for (current = head; current; current = current->next)
		if(current->point > m2 && current->point < m1) m2 = current->point;
	for (current = head; current; current = current->next)
		if(current->point > m3 && current->point < m2) m3 = current->point;
	strcat(x1,"\nXep hang 1:\t");
	strcat(x2,"\nXep hang 2:\t");
	strcat(x3,"\nXep hang 3:\t");
	// lấy ra người chơi tương ứng với 3 điểm số cao nhất
	for (current = head; current; current = current->next){
		if(current->point == m1) {
		strcat(x1,current->username);
		strcat(x1,"\t");	
		}else if(current->point == m2){
		 strcat(x2,current->username);	
		 strcat(x2,"\t");
		} else if(current->point == m3){
		 strcat(x3,current->username);
		 strcat(x3,"\t");
		}
	}
	

    snprintf(n, 3, "%d", m1); // chuyển đổi giá trị số nguyên m1 thành một chuỗi ký tự có độ dài tối đa là 3 và lưu vào mảng n
    strcat(x1,n);
    snprintf(n, 3, "%d", m2);
    strcat(x2,n);
    snprintf(n, 3, "%d", m3);
    strcat(x3,n);
    x1[strlen(x1)] ='\0';
    x2[strlen(x2)] ='\0';
    x3[strlen(x3)] ='\0';
    strcat(bufff,x1);
    strcat(bufff,x2);
    strcat(bufff,x3);
    printf("%s\n",bufff);
    return ;

}