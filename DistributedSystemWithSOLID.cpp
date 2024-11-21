#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <stdexcept>

// Interface for task processing (Interface Segregation Principle)
class TaskProcessor {
public:
    virtual void processTask(int taskId) = 0;
    virtual ~TaskProcessor() = default;
};

// Provides database access functionality (Single Responsibility Principle)
class DatabaseService {
public:
    std::string fetchMessageById(int id) {
        // Simulating a database fetch
        if (id > 0 && id <= 5) {
            return "Message for ID " + std::to_string(id);
        } else {
            throw std::invalid_argument("Invalid ID: " + std::to_string(id));
        }
    }
};

// Provides middleware messaging functionality (Single Responsibility Principle)
class MiddlewareService {
public:
    void sendMessage(const std::string& message) {
        std::cout << "Middleware: Sending -> " << message << std::endl;
    }
};

// Implements task processing with database and middleware interaction (Dependency Inversion Principle)
class DistributedTaskProcessor : public TaskProcessor {
private:
    DatabaseService& databaseService;
    MiddlewareService& middlewareService;

public:
    DistributedTaskProcessor(DatabaseService& dbService, MiddlewareService& mwService)
        : databaseService(dbService), middlewareService(mwService) {}

    void processTask(int taskId) override {
        try {
            std::string message = databaseService.fetchMessageById(taskId);
            std::cout << "Task " << taskId << ": Fetched -> " << message << std::endl;
            middlewareService.sendMessage("Processed Task " + std::to_string(taskId) + ": " + message);
        } catch (const std::exception& ex) {
            std::cerr << "Task " << taskId << ": Error -> " << ex.what() << std::endl;
        }
    }
};

// Thread-safe message queue for task coordination
class MessageQueue {
private:
    std::queue<int> tasks;
    std::mutex queueMutex;
    std::condition_variable cv;

public:
    void push(int taskId) {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.push(taskId);
        cv.notify_one();
    }

    int pop() {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this]() { return !tasks.empty(); });
        int taskId = tasks.front();
        tasks.pop();
        return taskId;
    }
};

// Main application
int main() {
    DatabaseService databaseService;
    MiddlewareService middlewareService;
    DistributedTaskProcessor taskProcessor(databaseService, middlewareService);

    MessageQueue taskQueue;

    // Worker threads for processing tasks
    std::vector<std::thread> workers;
    for (int i = 0; i < 3; ++i) {
        workers.emplace_back([&]() {
            while (true) {
                int taskId = taskQueue.pop();
                taskProcessor.processTask(taskId);
            }
        });
    }

    // Simulate adding tasks to the queue
    for (int i = 1; i <= 5; ++i) {
        taskQueue.push(i);
    }

    for (auto& worker : workers) {
        worker.detach(); // Detach to keep the simulation running
    }

    return 0;
}
