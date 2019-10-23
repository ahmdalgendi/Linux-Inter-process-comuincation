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




int main(int argc, char *argv[]) {
    unsigned long id = 2 ;

   
    
    printf("enter mailbox id\n");
    scanf("%lu" , &id);

    printf("mbx421_length = %d\n",mbx421_length(id) );
    perror("err: ");
    return 0;
}
