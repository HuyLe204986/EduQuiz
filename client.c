#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024

// chuyển đổi chữ thường thành chữ hoa
void Up(char *s)
{
	for (int i = 0; i < strlen(s); i++)
	{
		s[i] = s[i] - 32; // mảng lưu câu trả lời
	}
}

int main(int argc, char const *argv[])
{
	// valid number of argument
	// in huong dan su dung cho cac tham so dau vao va ket thuc
	char answer[11]; 
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	int client_sock; //số nguyên, đại diện cho socket của client dùng để gửi và nhận data tu server
	char username[MAX], password[MAX]; // số nguyên, đại diện cho socket của client dùng để gửi và nhận data
	char buff[8192];  // lưu  data nhận từ server hoặc data mà client gửi đi
	struct sockaddr_in server_addr; // lưu thông tin địa chỉ và cổng  của server mà client kết nối đến
	int bytes_sent, bytes_received; // số nguyên, lưu số lượng byte đã gửi và nhận khi trao đổi data giữa client và server

	// Construct socket
	/**
	tạo socket( cac tham so:
		AF_INET: xác định kiểu giao thức được sử dụng cho socket, giao thức(IPv4), 
		SOCK_STREAM: loại socket(trong trường hợp này là socket dùng truyền theo luông dữ liệu),  
		0: thường use cho giao thức mặc định tương ứng với loại giao thức đã chọn, ở đây là TCP
	) => trả về số nguyên k âm nếu thực hiện hàm thành công
	*/
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Specify server address
	// gán các thông tin
	// htons : chuyển đổi từ dạng chuỗi sang số nguyên sang định dạng dữ liệu mạng (Network Byte Order)
	// inet_addr: chuyển đổi từ chuỗi sang dạng số nguyên 32-bit của địa chỉ IP
	server_addr.sin_family = AF_INET; // xác định loại địa chỉ mạng sẽ được sử dụng. - IPv4
	server_addr.sin_port = htons(atoi(argv[2])); // cổng của server mà client sẽ kết nối đến
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); // địa chỉ IP của server mà client sẽ kết nối đến

	// Request to connect server
	// Đối số thứ hai là một con trỏ tới cấu trúc sockaddr chứa thông tin về địa chỉ của server. Do đó, nó được ép kiểu
	// Đối số thứ ba là kích thước của cấu trúc sockaddr
	// tra ve 0 neu connect thanh cong
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

	// Communicate with server
	while (1)
	{

		// get username
		puts("\nHãy nhập mật thông tin đăng nhập");
		printf("Tên người dùng: ");
		scanf("%[^\n]%*c", username); // doc cho den khi gap ky tu xuong dong
		//puts(username);
		username[strcspn(username, "\n")] = '\0'; // tìm vị trí đầu tiên của enter rồi gán thành '\0'

		// send username to server
		// gui du lieu cho server
		/** send(): trả về số lượng byte thực sự đã được gửi.
		 cac tham so:
		 - socket của client 
		 - con trỏ tới dữ liệu cần gửi
		 - độ dài của dữ liệu cần gửi
		 - cờ điều khiển cho quá trình gửi dữ liệu: 0 (không có cờ nào được sử dụng).
		*/
		if (0 >= (bytes_sent = send(client_sock, username, strlen(username), 0)))
		{
			printf("\nConnection closed!\n");
			return 0;
		}

		// receive server reply
		/* recv() được sử dụng để nhận dữ liệu từ socket, tra ve số lượng byte thực sự đã nhận được từ server
			cac tham so:
			- socket của client 
			- con trỏ tới vùng nhớ mà data nhận được sẽ được lưu trữ, trong trường này là buff.
			- kích thước tối đa của dữ liệu nhận được
			- cờ điều khiển cho quá trình nhan dữ liệu: 0 (không có cờ nào được sử dụng).
		*/
		if (0 >= (bytes_received = recv(client_sock, buff, 8192, 0)))
		{
			printf("\nError!Cannot receive data from sever!\n");
			return 0;
		}

		// exit if user not found on server
		buff[bytes_received] = '\0'; // gan ky tu ket thuc cho chuoi du lieu nhan duoc
		if (0 == strcmp(buff, "0"))
		{
			puts("Không tìm thấy tên tài khoản,đang đóng...\n");
			return 0;
		}
		else if (0 == strcmp(buff, "2"))
		{
			puts("Tài khoản đã bị khóa,đang đóng...\n");
			return 0;
		}

		// get password
		printf("Nhập mật khẩu: ");
		fgets(password, MAX, stdin);
		password[strcspn(password, "\n")] = '\0';

		// send password to server
		if (0 >= (bytes_sent = send(client_sock, password, strlen(password), 0)))
		{
			printf("\nConnection closed!\n");
			return 0;
		}

		// receive server reply
		memset(buff, '\0', MAX);
		if (0 >= (bytes_received = recv(client_sock, buff, 8192, 0)))
		{
			printf("\nError!Cannot receive data from sever!\n");
			return 0;
		}
		buff[bytes_received] = '\0';

		// analyze server reply
		if (0 == strcmp(buff, "0"))
		{ // if pass is wrong < 3 times
			puts("\nSai mật khẩu, xin mời nhập lại:\n");
			continue;
		}
		else if (0 == strcmp(buff, "2"))
		{ // if pass is wrong 3 times
			puts("\nNhập sai mật khẩu quá 3 lần,tài khoản của bạn đã bị khóa...\n");
			return 0;
		}
		else
		{ // if pass is right
			printf("\n\nĐăng nhập thành công\n%s\n câu trả lời là:\t", buff);
			scanf("%[^\n]%*c", answer);
			Up(answer);
			if (0 >= (bytes_sent = send(client_sock, answer, strlen(answer), 0)))
			{
				printf("\nConnection closed!\n");
				return 0;
			}
			memset(buff, 0, 8192);
			if (0 >= (bytes_received = recv(client_sock, buff, 8192, 0)))
			{
				printf("\nError!Cannot receive data from sever!\n");
				return 0;
			}

			printf("\nBạn trả lời đúng: %s\n", buff);
			break;
		}
	}
	// Close socket
	close(client_sock);
	return 0;
}