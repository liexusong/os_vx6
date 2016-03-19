#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	int max_size = 16;
	int buf_size = 128;

  int i;

	char buf[buf_size]; 
  for(i = max_size-1; i >= 0; i--) {
    if (0 == history(buf, i))
			printf(0, "%s\n", buf);
  }
  exit();
}
