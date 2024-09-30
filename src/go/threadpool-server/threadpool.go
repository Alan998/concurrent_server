// TODO: a better way is to use go modules
// reference: https://github.com/syafdia/go-exercise/tree/master/src/concurrency/workerpool
package main

import (
	"fmt"
)

type ThreadPool interface {
	Run()
	AddTask(task func())
}

type threadpool struct {
	max_threads int
	queued_task_chan chan func()
}

// NewThreadPool creates a instace of ThreadPool
func NewThreadPool(max_threads int) ThreadPool {
	pool := &threadpool {
		max_threads: max_threads,
		queued_task_chan: make(chan func()),
	}
	return pool
}

func (pool *threadpool) Run() {
	pool.run()
}

func (pool *threadpool) AddTask(task func()) {
	pool.queued_task_chan <- task
}

func (pool *threadpool) GetTotalQueuedTask() int {
	return len(pool.queued_task_chan)
}

func (pool *threadpool) run() {
	for i := 0; i < pool.max_threads; i++ {
		thread_id := i
		// use goroutines to execute functions in task queue
		go func(th_id int) {
			for task := range pool.queued_task_chan {
				fmt.Printf("[ThreadPool] Thread %d starts processing task\n", th_id)
				task()
				fmt.Printf("[ThreadPool] Thread %d finishes processing task\n", th_id)
			}
		} (thread_id)
	}
}
