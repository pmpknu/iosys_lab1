#include "kernel.h"
#include "common.h"

extern char __bss[], __bss_end[], __stack_top[];

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

long getchar(void) {
    struct sbiret ret = sbi_call(0, 0, 0, 0, 0, 0, 0, 2);
    return ret.error;
}

char readchar(void) {
  while (1) {
    long ch = getchar();
    if (ch >= 0) {
      return (char)ch;
    }
  }
}

long readlong(void) {
    long value = 0;
    int sign = 1;

    while (1) {
        long ch = getchar();

        if (ch == '-') {
            sign = -1; 
            continue;
            putchar((char)ch);
        }

        if (ch >= '0' && ch <= '9') {
            value = value * 10 + (ch - '0');
            putchar((char)ch);
        } else if (ch == '\n' || ch == ' ' || ch == '\t') {
            return value * sign;
        }
    }
}

void getsbi(void) {
  struct sbiret s = sbi_call(0, 0, 0, 0, 0, 0, 0, 0x10);
  printf("\nSBI version: %d\n", (int)s.value);
}

void hartgetstatus(void) {
  long hartid = readlong();
  struct sbiret s = sbi_call(hartid, 0, 0, 0, 0, 0, 2, 0x48534D);
  printf("\n Hart get status: %d\n", (int)s.error); // handle error 
}

void hartstop(void) {
  struct sbiret s = sbi_call(0, 0, 0, 0, 0, 0, 1, 0x48534D);
  printf("\n Hart stop status: %d\n", (int)s.value);
}

void shutdown(void) {
  struct sbiret s = sbi_call(0, 0, 0, 0, 0, 0, 0, 0x08);
  printf("\nshutdown\n");
}
  
void kernel_main(void) {
    for (;;) {
      printf("\n \
              menu\n \
              1. get sbi -v \n \
              2. hart get status \n \
              3. hart stop \n \
              4. shutdown \n \
      "); 
      long l = readlong();
      
      switch (l) {
      case 1: 
        getsbi();
        break;
      case 2:
        hartgetstatus();
        break;
      case 3:
        hartstop();
        break;
      case 4:
        shutdown();
        break;
      default:
        continue;
      }
    } 


    for (;;) {
        __asm__ __volatile__("wfi");
    }
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // Устанавливаем указатель стека
        "j kernel_main\n"       // Переходим к функции main ядра
        :
        : [stack_top] "r" (__stack_top) // Передаём верхний адрес стека в виде %[stack_top]
    );
}
