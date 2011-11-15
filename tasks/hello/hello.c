/** @file hello.c
 *
 * @brief Prints out Hello world using the syscall interface.
 *
 * Links to libc.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date   2008-10-29
 */
#include <unistd.h>

const char hello[] = "Hello World\r\n";
void change_r8(void);

int main(void)
{
	change_r8();
	write(STDOUT_FILENO, hello, sizeof(hello) - 1);
	return 0;
}
