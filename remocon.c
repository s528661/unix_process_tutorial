#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int sig = 0;
void signal_handler(int s) { sig = s; }

int main()
{
        signal(SIGINT,  signal_handler); /* SIGINT  ハンドラの登録 */
        signal(SIGUSR1, signal_handler); /* SIGUSR1 ハンドラの登録 */
        signal(SIGUSR2, signal_handler); /* SIGUSR2 ハンドラの登録 */

        setbuf(stdout, NULL); /* 標準出力のバッファリングを off */

        printf("\n""remocon: PID(%d)\n", getpid());
        while (sig != SIGINT) {
                switch (sig) {
                case 0:
                        break;
                case SIGUSR1:
                        printf("hello, world\n");
                        break;
                case SIGUSR2:
                        printf("give me cookies\n");
                        break;
                default:
                        printf("ouch!!\n");
                }
                sig = 0;
                pause(); /* signal を待つ */
        }
        printf("goodbye\n");

        return 0;
}
