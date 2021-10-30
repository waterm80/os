/*
Student No.: 0816058
Student Name: Hung-Ting Lin
Email: waterm80.cs08@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed
to be posted to be a public server, such as a public GitHub
repository or a public web page.
*/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <condition_variable>
using namespace std;

typedef struct{
    int type, low, high, id, mid;
}task_t;

int len, add_count, arr[1000005], task_count, complete_count;
task_t task_queue[15], complete[15];
sem_t add, r, job, grab;

void bubble_sort(int low, int high){
    for(int i = 0; i < high - low + 1; i++)
        for(int j = 0; j < high - low; j++)
            if(arr[low + i] < arr[low + j]){
                int tmp = arr[low + i];
                arr[low + i] = arr[low + j];
                arr[low + j] = tmp;
            }
}

void merge(int low, int mid, int high){
    int l1 = mid - low + 1;
    int l2 = high - mid;
    int arr_1[l1], arr_2[l2];
    for(int i = 0; i < l1; i++)
        arr_1[i] = arr[low + i];
    for(int i = 0; i < l2; i++)
        arr_2[i] = arr[mid + i + 1];
    int i = 0, j = 0, k = low;
    while(i < l1 && j < l2){
        if(arr_1[i] <= arr_2[j]){
            arr[k] = arr_1[i];
            i++;
        }
        else{
            arr[k] = arr_2[j];
            j++;
        }
        k++;
    }
    while(i < l1){
        arr[k] = arr_1[i];
        i++;
        k++;
    }
    while(j < l2){
        arr[k] = arr_2[j];
        j++;
        k++;
    }
}

void add_task(task_t task){
    sem_wait(&add);
    add_count++;
    task_queue[task_count++] = task;
    sem_post(&add);
    //sem_post(&job);
}

void relation(task_t task){
    for(int i = 0; i < complete_count; i++){  // find relation 
        if(complete[i].id != task.id && complete[i].id / 2 == task.id / 2 && add_count < 15){
            task_t next = {
                .type = 1,
                .low = min(task.low, complete[i].low),
                .high = max(task.high, complete[i].high),
                .id = task.id / 2 + 8,
                .mid = -1
            };
            if(complete[i].id % 2 == 0)
                next.mid = complete[i].high;
            else
                next.mid = task.high;
            add_task(next);
            break;
        }
    } 
}

void do_task(task_t task){
    if(task.type == 0){  // bottom 8 tasks
        bubble_sort(task.low, task.high);
        sem_wait(&r);
        complete[complete_count++] = task; 
        relation(task);
        sem_post(&r);
    }
    else{   // merge
        merge(task.low, task.mid, task.high);
        sem_wait(&r);
        complete[complete_count++] = task; 
        relation(task);
        sem_post(&r);
    }
}

void* start_thread(void*){
    while(1){
        task_t task;
        if(complete_count == 15)
            break;
        //sem_wait(&job);
        if(task_count <= 0)
            continue;
        sem_wait(&grab);
        task = task_queue[0];
        for(int i = 0; i < task_count; i++)
            task_queue[i] = task_queue[i+1];
        task_count--;
        sem_post(&grab);
        do_task(task);
    } 
    pthread_exit(NULL);
}

void output(int c){
    ofstream ofs;
    stringstream ss;
    string tmp;
    ss << c;
    ss >> tmp;
    string s = "output_" + tmp + ".txt";
    ofs.open(s.c_str());
    for(int i = 0; i < len; i++)
        ofs << arr[i] << " ";
    ofs.close();
}

int main(int argc, char* argv[]){
    struct timeval start, end;
    ifstream ifs;
    ifs.open("input.txt");
    ifs >> len;
    for(int n = 1; n <= 8; n++){
        gettimeofday(&start, NULL);
        for(int i = 0; i < len; i++)
            ifs >> arr[i];
        add_count = 0;
        task_count = 0;
        complete_count = 0;
        pthread_t threads[n];
        sem_init(&add, 0, 1);
        sem_init(&r, 0, 1);
        //sem_init(&job, 0, 0);
        sem_init(&grab, 0, 1);
        int remain = len % 8, ptr = 0;
        for(int i = 0; i < 8; i++){
            task_t task;
            task.id = i;
            task.low = ptr;
            task.type = 0;
            if(remain != 0){
                ptr += len / 8 + 1;
                remain--;
            }
            else
                ptr += len / 8;
            task.high = ptr - 1;
            task.mid = -1;
            add_task(task);
        }
        for(int i = 0; i < n; i++)
            pthread_create(&threads[i], NULL, start_thread, NULL);
        for(int i = 0; i < n; i++)
            pthread_join(threads[i], NULL);
        sem_destroy(&add);
        sem_destroy(&r);
        //sem_destroy(&job);
        sem_destroy(&grab);
        gettimeofday(&end, NULL);
        output(n);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        cout << "worker thread #" << n << ", elapsed " << fixed << setprecision(6) << sec * 1000 + (usec / 1000.0) << " ms\n";
    }
    ifs.close();
    return 0;
}