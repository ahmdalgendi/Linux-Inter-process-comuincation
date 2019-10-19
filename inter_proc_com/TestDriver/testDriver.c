#include <stdio.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#define __NR_hello1 335
#define __NR_hello2 336
#define __NR_hello3 337
#define __NR_hello4 338
#define __NR_hello5 339
#define __NR_hello6 340
#define __NR_hello7 341
#define __NR_hello8 342
#define __NR_hello9 343
#define __NR_hello0 344

long mbx421_init(unsigned int a ,unsigned int b) {
    return syscall(__NR_hello1,a,b);
}
long mbx421_shutdown(void) {
    return syscall(__NR_hello2);
}
long mbx421_create(unsigned int id) {
    return syscall(__NR_hello3,id);
}
long mbx421_destroy(unsigned int id) {
    return syscall(__NR_hello4,id);
}
long mbx421_count(unsigned int id) {
    return syscall(__NR_hello5,id);
}
long mbx421_send(unsigned int id,  unsigned char  msg[], long len) {
    return syscall(__NR_hello6,id , msg , len );
}
long mbx421_recv(unsigned int id, unsigned char  msg[], long len){
    return syscall(__NR_hello7, id , msg , len);
}
long mbx421_length(unsigned int id) {
    return syscall(__NR_hello8, id);
}
long mbx421_acl_add(unsigned int id, pid_t process_id){
    return syscall(__NR_hello9, id , process_id);
}

long mbx421_acl_remove(unsigned int id, pid_t process_id){
    return syscall(__NR_hello0, id , process_id);
}


void print_menu()
{
	puts("\n1-mbx421_init\n\
2-mbx421_shutdown\n\
3-mbx421_create\n\
4-mbx421_destroy\n\
5-mbx421_count\n\
6-mbx421_send\n\
7-mbx421_recv\n\
8-mbx421_length\n\
9-bx421_acl_add\n\
10-mbx421_acl_remove\n11-gets this process_id\n");

}

int main(int argc, char *argv[]) {
    unsigned int id ;
    long len;
    unsigned char msg[100];
    
    pid_t process_id;
    int p , mx;
pid_t pid ;
	while(1)
	{
		print_menu();
		printf("Choice: ");
		int choice;
		scanf("%d" , &choice);
		switch (choice){
			case 1:
				scanf("%d%d",&mx , &p );
				printf("init retu = %d\n", mbx421_init(mx , p));

			break;
			case 2:
				printf("SHud = %d\n", mbx421_shutdown());				

			break;
			case 3:
				scanf("%u" , &id);
				printf(" mbx421_create = %d\n", mbx421_create( id));

			break;
			case 4:
				scanf("%u" , &id);

				printf(" mbx421_destroy = %d\n", mbx421_destroy(id));
			break;
			case 5:

				scanf("%u" , &id);

				printf("mbx421_count = %d\n", mbx421_count(id));
			break;
			case 6:
				printf("id , msg , len\n");
				scanf("%u%s%d", &id , msg , &len);
				printf("%s\n", msg);

				printf("mbx421_send = %d\n" , mbx421_send(id, msg, len));
			break;
			case 7:
				printf("id , msg , len\n");
				scanf("%u", &id );
				printf("mbx421_send = %d\n" , mbx421_recv(id, msg, len));
				printf("mess = %s\n",msg);

			break;
			case 8:
				scanf("%u" , &id);

				printf(" mbx421_length = %d\n", mbx421_length(id) );
			break;
			case 9:
			// printf("pid = %ld\n", (long) pid);
				scanf("%u%ld" , &id, &process_id);
				printf(" mbx421_acl_add = %d\n",  mbx421_acl_add(id, process_id));

			break;
			case 10:
				scanf("%u%ld" , &id, &process_id);
				printf(" mbx421_acl_remove = %d\n",  mbx421_acl_remove(id, process_id));
			break;
			case 11:
				   pid = getpid();
			printf("pid = %ld\n", (long) pid);

			break;
			default:
			puts("wrong input");

			

		}
	}
    return 0;
}
