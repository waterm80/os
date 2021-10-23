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
#include <condition_variable>
#include <sys/times.h>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

typedef struct{
    int id, low, high;
}task_t;

int add_count;
queue<task_t> task_queue;
vector<pair<int, int>> vec;
pthread_mutex_t lock;
pthread_cond_t cond;
sem_t sem;

void bubble_sort(int arr[], int low, int high){
    for(int i = 0; i <= high - low; i++)
        for(int j = 0 ; j < high - low - i; j++)
            if(arr[low + j] > arr[low + j+1]){
                int tmp = arr[low + j];
                arr[low + j] = arr[low + j+1];
                arr[low + j+1] = tmp;
            }
}

void merge(int arr[], int low,int mid, int high){
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

void merge_sort(int arr[], int low, int high){
    if(low < high){
        int mid = (low + high) / 2;
        merge_sort(arr, low, mid);
        merge_sort(arr, mid + 1, high);
        merge(arr, low, mid, high);
    }
}

void add_task(int arr[], task_t task){
    pthread_mutex_lock(&lock);
    add_count++;
    task_queue.push(task);
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&cond);
}

void do_task(int arr[], task_t* task){
    if(task->id == 0){  // bottom 8 tasks
        pair<int, int> p;
        p.first = task->low;
        p.second = task->high;
        bubble_sort(arr, task->low, task->high);
        bool find = false;
        vector<pair<int, int>>::iterator it;
        for(it = vec.begin(); it != vec.end(); it++){  // find relation
            if(it->first == task->high + 1){
                task_t next = {
                    .id = 1,
                    .low = task->low,
                    .high = it->second
                };
                vec.erase(it);
                add_task(arr, next);
                find = true;
                break;
            }
            else if(it->second == task->low - 1){
                task_t next = {
                    .id = 1,
                    .low = it->first,
                    .high = task->high
                };
                vec.erase(it);
                add_task(arr, next);
                find = true;
                break;
            }
        }
        if(find == false)
            vec.push_back(p);
    }
    else{   // merge
        pair<int, int> p;
        p.first = task->low;
        p.second = task->high;
        merge_sort(arr, task->low, task->high);
        bool find = false;
        vector<pair<int, int>>::iterator it;
        for(it = vec.begin(); it != vec.end(); it++){  // find relation
            if(it->first == task->high + 1){
                task_t next = {
                    .id = 1,
                    .low = task->low,
                    .high = it->second
                };
                vec.erase(it);
                add_task(arr, next);
                find = true;
                break;
            }
            else if(it->second == task->low - 1){
                task_t next = {
                    .id = 1,
                    .low = it->first,
                    .high = task->high
                };
                vec.erase(it);
                add_task(arr, next);
                find = true;
                break;
            }
        }
        if(find == false)
            vec.push_back(p);
    }
}

void* start_thread(void* args){
    while(1){
        int *data = (int*)args;
        task_t task;
        if(add_count == 15 && vec.empty())
            break;
        pthread_mutex_lock(&lock);
        if(task_queue.size() > 0){
            task = task_queue.front();
            task_queue.pop();
        }
        else{
            pthread_mutex_unlock(&lock);
            continue;
        }
        pthread_mutex_unlock(&lock);
        do_task(data, &task);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int n, len;
    cin >> n >> len;
    int arr[len];
    for(int i = 0; i < len; i++)
        cin >> arr[i];
    add_count = 0;
    pthread_t threads[n];
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    sem_init(&sem, 0, 0);
    for(int i = 0; i < n; i++)
        pthread_create(&threads[i], NULL, &start_thread, arr);
        
    int remain = len % n, ptr = 0;
    for(int i = 0; i < n; i++){
        task_t task;
        task.id = 0;
        task.low = ptr;
        if(remain != 0){
            ptr += len / n + 1;
            remain--;
        }
        else
            ptr += len / n;
        task.high = ptr - 1;
        add_task(arr, task);
    }
    for(int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    sem_destroy(&sem);
    vec.clear();
    for(int i = 0; i < len; i++)
        cout << arr[i] << " ";
    cout<< '\n';
    
    return 0;
}