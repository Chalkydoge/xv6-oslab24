#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

uint64 MAGIC_NUM = 114514;
uint64 pids[1024];    // 保存每个子进程的pid
uint64 results[1024];  // 保存每个子进程的计算结果

// Process Statistics Test
// Description:
// A sample calculation Task is involved here
// The test will check if the process statistics are correctly updated
// according to different types of Scheduler [RR / PR]
uint64 big_calculation() {
    uint64 i, j, sum = 0;
    for (i = 0; i < MAGIC_NUM; i++) {
        for (j = 0; j < i; j++) {
            if (j & 1) {
                sum -= j;
            } else {
                sum += j;
            }
        }
        sum *= i;
        sum /= (i - j + 1);
        if (i % 11451 == 0) {
            sleep(1);
        }
    }
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: stat <n>\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        printf("Error: Invalid number of processes\n");
        exit(1);
    }

    // 用于统计所有进程的总运行时间、等待时间、休眠时间
    int total_run_time = 0;
    int total_wait_time = 0;
    int total_sleep_time = 0;

    for (int i = 0; i < n; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("Error: Fork failed\n");
            exit(1);
        }

        if (pid == 0) {
            // 子进程执行大规模计算
            uint64 ret = big_calculation();
            pids[i] = getpid();
            results[i] = ret;
            // just for debugging
            sleep(1);
            exit(0);  // 子进程退出
        }
    }

    // 父进程等待所有子进程并获取调度统计
    for (int i = 0; i < n; i++) {
        int runable_time, running_time, sleep_time;

        //  wait_sched(int *runable_time, int *running_time, int *sleep_time);
        int pid = wait_sched(&runable_time, &running_time, &sleep_time);  // 调用wait_sched获取统计信息

        if (pid >= 0) {
            printf("PID: %d | Runnable Time: %d ticks | Running Time: %d ticks | Sleep Time: %d ticks\n",
                   pid, runable_time, running_time, sleep_time);

            total_run_time += running_time;
            total_wait_time += runable_time;
            total_sleep_time += sleep_time;
        } else {
            printf("Error: wait_sched failed\n");
            exit(1);
        }
    }

    // 计算并输出平均轮转时间（Turnaround Time）
    int avg_turnaround_time = (total_run_time + total_wait_time + total_sleep_time) / n;
    printf("Average Turnaround Time: %d ticks\n", avg_turnaround_time);

    exit(0);
}
