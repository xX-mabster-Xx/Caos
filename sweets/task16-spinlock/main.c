#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <unistd.h>

void spin_lock(volatile int *s);
void spin_unlock(volatile int *s);

struct state {
  volatile int lock;
  int value;
};

int main() {
  struct state *state = mmap(NULL, sizeof(*state), PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  for (int i = 0; i < 10; ++i) {
    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    }
    if (pid == 0) {
      for (int j = 0; j < 100000; ++j) {
        spin_lock(&state->lock);
        state->value++;
        spin_unlock(&state->lock);
      }
      return 0;
    }
  }
  while (wait(NULL) > 0);
  printf("%d\n", state->value);
}
