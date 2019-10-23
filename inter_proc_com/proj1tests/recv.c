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
long mbx421_create(unsigned long id) {
    return syscall(__NR_hello3,id);
}
long mbx421_destroy(unsigned long id) {
    return syscall(__NR_hello4,id);
}
long mbx421_count(unsigned long id) {
    return syscall(__NR_hello5,id);
}

long mbx421_send(unsigned long id,  unsigned char  *msg, long len) {
    return syscall(__NR_hello6,id , msg , len );
}
long mbx421_recv(unsigned long id, unsigned char  msg[], long len){
    return syscall(__NR_hello7, id , msg , len);
}
long mbx421_length(unsigned long id) {
    return syscall(__NR_hello8, id);
}
long mbx421_acl_add(unsigned long id, pid_t process_id){
    return syscall(__NR_hello9, id , process_id);
}

long mbx421_acl_remove(unsigned long id, pid_t process_id){
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
    unsigned long id = 2 ;
    unsigned char msg[100];
   
    long len;
    pid_t pid = getpid();
    printf("pid = %ld\npress any key to continue", (long) pid);
    getchar();
    
    while(1)
    {
        printf("MailBox ID: \n");
        scanf("%lu" , &id);
      

        printf("msg_len:");
        scanf("%d", &len);

        printf("mbx421_recv = %d\n",mbx421_recv(id , msg , len) );

        perror("err: ");

        printf("msg  receved = %s\n",msg );
        char c;
        puts("send another msg {yes (y) , no (otherwise)}");
        scanf("%c" , &c);

        if(c != 'y')
            break;
    }


    return 0;
}
