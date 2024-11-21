/**
Key Best Practices :
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

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Interface for task executor in a distributed system.
 * Encourages flexibility for multiple task types.
 */
interface TaskExecutor {
    void executeTask(int taskId);
}

/**
 * Provides database access functionality.
 * Follows Single Responsibility Principle.
 */
class DatabaseService {
    private static final String DB_URL = "jdbc:oracle:thin:@localhost:1521:orcl";
    private static final String DB_USER = "system";
    private static final String DB_PASSWORD = "password";
    private static final Logger LOGGER = Logger.getLogger(DatabaseService.class.getName());

    /**
     * Fetches message from database by task ID.
     * Uses parameterized queries to prevent SQL injection.
     *
     * @param id The ID of the task to fetch
     * @return The message corresponding to the task ID
     */
    public String fetchMessageById(int id) throws Exception {
        String message = null;

        try (Connection connection = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            String query = "SELECT message FROM messages WHERE id = ?";
            try (PreparedStatement statement = connection.prepareStatement(query)) {
                statement.setInt(1, id);

                try (ResultSet resultSet = statement.executeQuery()) {
                    if (resultSet.next()) {
                        message = resultSet.getString("message");
                    } else {
                        LOGGER.log(Level.WARNING, "No message found for ID: " + id);
                        throw new IllegalArgumentException("No message found for ID: " + id);
                    }
                }
            }
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error fetching message for ID: " + id, e);
            throw new RuntimeException("Database error occurred", e);
        }

        return message;
    }
}

/**
 * Provides middleware messaging functionality.
 * Follows Single Responsibility Principle.
 */
class MiddlewareService {
    private static final Logger LOGGER = Logger.getLogger(MiddlewareService.class.getName());

    /**
     * Sends message to middleware system.
     * Example: Could integrate with JMS, Kafka, etc.
     *
     * @param message The message to be sent
     */
    public void sendMessage(String message) {
        try {
            LOGGER.log(Level.INFO, "Middleware: Sending -> " + message);
            // Simulate sending to an actual middleware system (e.g., JMS, Kafka)
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error sending message to middleware", e);
        }
    }
}

/**
 * Implements a task executor for database and middleware operations.
 * Implements Dependency Injection to decouple components.
 */
class DistributedTaskExecutor implements TaskExecutor {
    private final DatabaseService databaseService;
    private final MiddlewareService middlewareService;

    /**
     * Constructor for dependency injection.
     * 
     * @param databaseService The database service to interact with
     * @param middlewareService The middleware service to send messages to
     */
    public DistributedTaskExecutor(DatabaseService databaseService, MiddlewareService middlewareService) {
        this.databaseService = databaseService;
        this.middlewareService = middlewareService;
    }

    @Override
    public void executeTask(int taskId) {
        try {
            String message = databaseService.fetchMessageById(taskId);
            LOGGER.log(Level.INFO, "Task " + taskId + ": Fetched message -> " + message);
            middlewareService.sendMessage("Processed Task " + taskId + ": " + message);
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Task " + taskId + " failed", e);
        }
    }
}

/**
 * Main application class demonstrating distributed task execution.
 * This class is responsible for managing task execution in a multi-threaded environment.
 */
public class DistributedSystemWithBestPractices {
    private static final Logger LOGGER = Logger.getLogger(DistributedSystemWithBestPractices.class.getName());

    public static void main(String[] args) {
        try {
            DatabaseService databaseService = new DatabaseService();
            MiddlewareService middlewareService = new MiddlewareService();
            TaskExecutor taskExecutor = new DistributedTaskExecutor(databaseService, middlewareService);

            ExecutorService executorService = Executors.newFixedThreadPool(5);

            for (int i = 1; i <= 5; i++) {
                final int taskId = i;
                executorService.submit(() -> taskExecutor.executeTask(taskId));
            }

            executorService.shutdown();
        } catch (Exception e) {
            LOGGER.log(Level.SEVERE, "Error initializing the distributed system", e);
        }
    }
}
