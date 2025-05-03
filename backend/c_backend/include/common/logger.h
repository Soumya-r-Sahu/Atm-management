/**
 * @file logger.h
 * @brief Logging utility for the ATM Management System
 */

#ifndef LOGGER_H
#define LOGGER_H

/**
 * Log levels
 */
typedef enum {
    LOG_DEBUG,   /**< Debug level - detailed information */
    LOG_INFO,    /**< Info level - general information */
    LOG_WARNING, /**< Warning level - potential issues */
    LOG_ERROR,   /**< Error level - errors that allow the program to continue */
    LOG_FATAL    /**< Fatal level - critical errors that stop the program */
} LogLevel;

/**
 * @brief Initialize the logger
 * @param log_file Path to the log file
 * @param console_output Whether to output logs to console
 * @param min_level Minimum log level to record
 * @return int 0 on success, -1 on failure
 */
int logger_init(const char *log_file, int console_output, LogLevel min_level);

/**
 * @brief Close the logger
 */
void logger_close(void);

/**
 * @brief Log a message
 * @param level Log level
 * @param file Source file name
 * @param line Source line number
 * @param function Function name
 * @param format Format string
 * @param ... Variable arguments
 */
void logger_log(LogLevel level, const char *file, int line, const char *function, const char *format, ...);

/**
 * Convenience macros for logging
 */
#define LOG_DEBUG(format, ...) logger_log(LOG_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logger_log(LOG_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) logger_log(LOG_WARNING, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) logger_log(LOG_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) logger_log(LOG_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#endif /* LOGGER_H */
