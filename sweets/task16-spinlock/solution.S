  .att_syntax noprefix
  
  .global spin_lock
spin_lock:
  movl 4(esp), ecx
  movl $1, edx
locked:
  movl $0, eax
  lock cmpxchg edx, (ecx)
  jz unlocked
  pause
  jmp locked
unlocked:
  ret

  .global spin_unlock
spin_unlock:
  movl 4(esp), eax
  movl $0, (eax)
  ret
