#ifndef semutil_h
#define semutil_h

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define PERM 0644
#define numops(op) ( sizeof(op) / sizeof(struct sembuf) )

void sem_ready(int semid)
{
        /*** セマフォの約束定義( ロック可能状態へ移行 ) ***/
        struct sembuf operation[] = {
                { 0, 1, 0 },         /* セマフォを 1 増加させる */
        };

        /*** 約束の実行 ***/
        semop( semid, operation, numops(operation) );
}

void sem_lock(int semid)
{
        /*** セマフォの約束定義(1 待ち後 1 引き) ***/
        struct sembuf operation[] = {
                { 0, -1, SEM_UNDO }, /* セマフォが 1 になるのを待って 1 引く */
        };

        /*** 約束の実行 ***/
        semop( semid, operation, numops(operation) );
}

void sem_unlock(int semid)
{
        /*** セマフォの約束定義(ロック解除) ***/
        struct sembuf operation[] = {
                { 0, 1, SEM_UNDO },  /* セマフォに 1 を足す */
        };

        /*** 約束の実行 ***/
        semop( semid, operation, numops(operation) );
}

void sem_wait(int semid)
{
        /*** セマフォの約束定義(他のプロセスのロック待ち) ***/
        struct sembuf operation[] = {
                { 0, 0, 0 },         /* セマフォが 0 になるのを待つ */
        };

        /*** 約束の実行 ***/
        semop( semid, operation, numops(operation) );
}

#endif /* semutil_h */
