#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "semutil.h"

int main()
{
        int semid; /* セマフォの ID */
        int shmid; /* 共有メモリの ID */
        int *mem;  /* プロセスメモリ空間へのポインタ */
        int i, n;
        int status = 0;

        setbuf(stdout, NULL); /* 標準出力のバッファリングを off */

        /*** セマフォの作成 ***/
        semid = semget(IPC_PRIVATE, 1, PERM | IPC_CREAT);
        if (semid == -1) { perror("semget()"); exit(1); }

        /*** 共有メモリの作成 ***/
        shmid = shmget(IPC_PRIVATE, sizeof(int), PERM | IPC_CREAT);
        if (shmid == -1) { perror("shmget()"); exit(1); }

        /*** 共有メモリのプロセスメモリ空間への割り当て ***/
        mem = shmat(shmid, 0, 0);
        if (mem == (int *) -1) { perror("shmat()"); exit(1); }

        /*** 共有メモリの内容の初期化 ***/
        *mem = 0;

        /*** ロック可能状態にする ***/
        sem_ready(semid);

        switch (fork()) {
        case -1:
        /*** fork の失敗 ***/
                perror("fork()");
                status = 1;
                goto cleanup;
        case 0:
        /*** 子プロセスの処理 ***/
        /*** 子プロセスは親プロセスから共有メモリを継承している ***/
                for (i = 0; i < 5; i++) {
                        usleep(rand() % 100000);  /* ランダムな時間待ち */
                        sem_lock(semid);          /* ロックする */
                        n = *mem;                 /* 共有メモリからの読みだし */
                        printf(" child: *mem = %d\n", *mem);
                        usleep(rand() % 100000);  /* ランダムな時間待ち */
                        *mem = n + 1;             /* 共有メモリへの書き込み */
                        sem_unlock(semid);        /* ロックの解除 */
                }
                _exit(0);
        default:
        /*** 親プロセスの処理 ***/
                for (i = 0; i < 5; i++) {
                        usleep(rand() % 100000);  /* ランダムな時間待ち */
                        sem_lock(semid);          /* ロックする */
                        n = *mem;                 /* 共有メモリからの読みだし */
                        printf("parent: *mem = %d\n", *mem);
                        usleep(rand() % 100000);  /* ランダムな時間待ち */
                        *mem = n + 1;             /* 共有メモリへの書き込み */
                        sem_unlock(semid);        /* ロックの解除 */
                }
        }
        wait(NULL); /* 子プロセスの合流待ち */
        printf("\n""  last: *mem = %d\n\n", *mem);

cleanup:
        /*** 共有メモリのプロセスメモリ空間からの切り離し ***/
        shmdt( (char *) mem );

        /*** 共有メモリの削除 ***/
        shmctl(shmid, IPC_RMID, 0);

        /*** セマフォの削除 ***/
        semctl(semid, 0, IPC_RMID, 0);

        return status;
}
