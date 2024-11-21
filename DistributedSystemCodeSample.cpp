/*
Best Practices 
Security:

SQL Injection Protection: Parameterized queries in Java and boundary checks in C++.
Exception Handling: Robust error handling with appropriate logging.
Reusability:

Interface-based Design: Task processors are implemented as interfaces so they can be extended or replaced.
Modular Components: Database and middleware services are separate, promoting reusability and maintainability.
Maintainability:

SOLID Principles: Code adheres to SOLID principles, particularly Single Responsibility Principle (SRP) and Dependency Inversion Principle (DIP).
Thread Safety: Message queues are thread-safe, ensuring smooth task processing in multi-threaded environments.
Logging: Extensive logging helps trace and debug the system’s activities
*/

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <exception>

// Logger for capturing events (helps in debugging and security monitoring)
#define LOGGER std::cerr

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
        // Simulate sending a message to a middleware system (e.g., Kafka, JMS)
        LOGGER << "Middleware: Sending -> " << message << std::endl;
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
            LOGGER << "Task " << taskId << ": Fetched -> " << message << std::endl;
            middlewareService.sendMessage("Processed Task " + std::to_string(taskId) + ": " + message);
        } catch (const std::exception& ex) {
            LOGGER << "Task " << taskId << ": Error -> " << ex.what() << std::endl;
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
    try {
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
            worker.join();
        }
    } catch (const std::exception& ex) {
        LOGGER << "Application Error: " << ex.what() << std::endl;
    }

    return 0;
}
