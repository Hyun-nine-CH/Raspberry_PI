#include <linux/unistd.h> 
#include <linux/kernel.h> 
#include <linux/syscalls.h>
#include <linux/mm_types.h>                /* pgprot_t 정의를 위해 추가 */     
#include <asm/uaccess.h>
#include <asm/errno.h>
//#include <sys/syscall.h>                     /* syscall() 함수 정의 */
//#include <unistd.h>                     /* syscall 번호를 포함 */

SYSCALL_DEFINE3(add, int, a, int, b, int __user *, res) 
{                    
    long err = 0;
    int ret = 0;
   
    err = access_ok(res, sizeof(int));           /* 유저 영역에 쓸 수 있는지 검사 */
    if(err == -EFAULT) return err;
   
    ret = a + b;
    printk(KERN_INFO "%d + %d = %d from Linux Kernel\n", a, b, *res);
    put_user(ret, res);                     /* 유저 영역으로 값을 보낸다. */

    return err;
}
