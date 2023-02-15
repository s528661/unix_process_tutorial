#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "semutil.h"

int main(int argc, char **argv)
{
        int semid; /* セマフォの ID */
        int i;
        int status = 0;

        setbuf(stdout, NULL); /* 標準出力のバッファリングを off */

        /*** セマフォの作成 ***/
        semid = semget(IPC_PRIVATE, 1, PERM | IPC_CREAT);
        if (semid == -1) { perror("semget()"); exit(1); }

        /*** ロック可能状態にする ***/
        sem_ready(semid);

        switch (fork()) {
        case -1:
        /*** fork の失敗 ***/
                perror("fork()");
                status = 1; /* セマフォの削除 */
                goto cleanup;
        case 0:
        /*** 子プロセスの処理 ***/
                for (i = 1; i <= 10; i++) {
                        sem_wait(semid);    /* 他のプロセスのロック待ち */
                        sem_lock(semid);    /* ロックする */
                        printf("child\n");  /* プリントアウトする */
                        sem_unlock(semid);  /* ロックの解除 */
                }
                _exit(0);
        default:
        /*** 親プロセスの処理 ***/
                for (i = 1; i <= 10; i++) {
                        sem_lock(semid);     /* ロックする */
                        printf("parent\n");  /* プリントアウトする */
                        getchar();           /* リターンキー入力待ち */
                        sem_unlock(semid);   /* ロックの解除 */
                        sem_wait(semid);     /* 他のプロセスのロック待ち */
                }
        }
        wait(NULL); /* 子プロセスの合流待ち */

cleanup:
        /*** セマフォの削除 ***/
        semctl(semid, 0, IPC_RMID, 0);

        return status;
}
