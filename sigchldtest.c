#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define CHARGE 5

int sigchld_cnt = 0;
void sigchld_handler(int s) { sigchld_cnt++; }

int main()
{
        int pid;  /* 子プロセスのプロセスID */
        int n;

        setbuf(stdout, NULL); /* 標準出力のバッファリングを off */

        printf("\n""sigchldtest: CHARGE = %d\n", CHARGE);
        printf("--------+--------\n");
        printf(" parent | child  \n");
        printf("--------+--------\n");

        for (n = 0; n < CHARGE; n++) {
                printf(" n = %2d |\n", n);
                sleep(1);
        }

        switch (pid = fork()) { /* プロセスの生成(分岐) */
        case -1:
                /*** fork() の失敗 ***/
                perror("fork()"); /* エラーメッセージ表示 */
                exit(1);
        case 0:
                /*** 子プロセスの処理 ***/
                while (n > 0) {
                        printf("        |");
                        printf(" n = %2d\n", n);
                        n--;
                        sleep(1);
                }
                _exit(0); /* 子プロセス終了 */
        default:
                /*** 親プロセスの処理 ***/
                signal(SIGCHLD, sigchld_handler); /* SIGCHLD ハンドラの登録 */
                while (n < 15) {
                        if (sigchld_cnt > 0) {  
                                /*** ゾンビープロセス対策 ***/
                                printf("parent: catch SIGCHLD\n");
                                sigchld_cnt--;
                                wait(NULL);
                        }
                        printf(" n = %2d |\n", n);
                        n++;
                        sleep(1);
                }
        }

        wait(NULL); /* 子プロセスの合流待ち */
        printf("--------+--------\n");
        printf("PID: parent(%d), child(%d)\n\n", getpid(), pid);

        return 0;
}
