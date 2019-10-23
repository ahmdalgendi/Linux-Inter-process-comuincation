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

    unsigned char *msg;

int main(int argc, char *argv[]) {
    unsigned long id ;
   
    long len;
    msg = (unsigned char * ) malloc(sizeof(char) * 1000000);
    pid_t process_id;
    int p , mx;
pid_t pid ;
   
        // print_menu();
        int choice;
        choice = 10;
        // scanf("%d" , &choice);
        printf("enter id, then process_id:");
        switch (choice){
            case 1:
                scanf("%d%d",&mx , &p );
                printf("init retu = %d\n", mbx421_init(mx , p));

            break;
            case 2:
                printf("SHud = %d\n", mbx421_shutdown());               

            break;
            case 3:
                scanf("%lu" , &id);
                printf(" mbx421_create = %d\n", mbx421_create( id));
                perror("awwman: ");

            break;
            case 4:
                scanf("%lu" , &id);

                printf(" mbx421_destroy = %d\n", mbx421_destroy(id));
                perror("awwman: ");
            break;
            case 5:

                scanf("%lu" , &id);

                printf("mbx421_count = %d\n", mbx421_count(id));
                perror("awwman: ");
            break;
            case 6:
                printf("id , msg , len\n");
                scanf("%lu%s%d", &id , msg , &len);
                printf("%s  , pointer = %pr\n", msg, msg);

                printf("mbx421_send = %d\n" , mbx421_send(id, msg, len));
            break;
            case 7:
                printf("id , msg , len\n");
                scanf("%lu", &id );

                printf("mbx421_recv = %d\n" , mbx421_recv(id, msg, len));
                printf("mess = %s\n",msg);

            break;
            case 8:
                scanf("%lu" , &id);

                printf(" mbx421_length = %d\n", mbx421_length(id) );
            break;
            case 9:
            // printf("pid = %ld\n", (long) pid);
                scanf("%lu%ld" , &id, &process_id);
                printf(" mbx421_acl_add = %d\n",  mbx421_acl_add(id, process_id));
    perror("err: ");

            break;
            case 10:
                scanf("%lu%ld" , &id, &process_id);
                printf(" mbx421_acl_remove = %d\n",  mbx421_acl_remove(id, process_id));
    perror("err: ");

            break;
            case 11:
                   pid = getpid();
            printf("pid = %ld\n", (long) pid);

            break;
            default:
            puts("wrong input");

            

        
    }
    return 0;
}
