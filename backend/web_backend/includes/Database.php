<?php
/**
 * Database class for the ATM Management System web backend
 */

class Database {
    private $conn;
    private static $instance = null;
    
    /**
     * Private constructor to prevent direct instantiation
     */
    private function __construct() {
        try {
            $this->conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT);
            
            if ($this->conn->connect_error) {
                throw new Exception("Database connection failed: " . $this->conn->connect_error);
            }
            
            $this->conn->set_charset("utf8mb4");
        } catch (Exception $e) {
            error_log("Database connection error: " . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * Get database instance (Singleton pattern)
     * @return Database Database instance
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }
    
    /**
     * Get database connection
     * @return mysqli Database connection
     */
    public function getConnection() {
        return $this->conn;
    }
    
    /**
     * Execute a query
     * @param string $query SQL query
     * @param array $params Query parameters
     * @return mysqli_stmt|false Statement object or false on failure
     */
    public function query($query, $params = []) {
        try {
            $stmt = $this->conn->prepare($query);
            
            if ($stmt === false) {
                throw new Exception("Query preparation failed: " . $this->conn->error);
            }
            
            if (!empty($params)) {
                $types = '';
                $bindParams = [];
                
                foreach ($params as $param) {
                    if (is_int($param)) {
                        $types .= 'i';
                    } elseif (is_float($param)) {
                        $types .= 'd';
                    } elseif (is_string($param)) {
                        $types .= 's';
                    } else {
                        $types .= 'b';
                    }
                    $bindParams[] = $param;
                }
                
                array_unshift($bindParams, $types);
                call_user_func_array([$stmt, 'bind_param'], $this->refValues($bindParams));
            }
            
            $stmt->execute();
            return $stmt;
        } catch (Exception $e) {
            error_log("Query execution error: " . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * Execute a SELECT query and return all rows
     * @param string $query SQL query
     * @param array $params Query parameters
     * @return array Result rows
     */
    public function select($query, $params = []) {
        try {
            $stmt = $this->query($query, $params);
            $result = $stmt->get_result();
            $rows = [];
            
            while ($row = $result->fetch_assoc()) {
                $rows[] = $row;
            }
            
            $stmt->close();
            return $rows;
        } catch (Exception $e) {
            error_log("Select query error: " . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * Execute a SELECT query and return a single row
     * @param string $query SQL query
     * @param array $params Query parameters
     * @return array|null Result row or null if not found
     */
    public function selectOne($query, $params = []) {
        $rows = $this->select($query, $params);
        return !empty($rows) ? $rows[0] : null;
    }
    
    /**
     * Execute an INSERT query
     * @param string $table Table name
     * @param array $data Data to insert
     * @return int|false Last insert ID or false on failure
     */
    public function insert($table, $data) {
        try {
            $columns = implode(', ', array_keys($data));
            $placeholders = implode(', ', array_fill(0, count($data), '?'));
            $query = "INSERT INTO $table ($columns) VALUES ($placeholders)";
            
            $stmt = $this->query($query, array_values($data));
            $insertId = $this->conn->insert_id;
            $stmt->close();
            
            return $insertId;
        } catch (Exception $e) {
            error_log("Insert query error: " . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * Execute an UPDATE query
     * @param string $table Table name
     * @param array $data Data to update
     * @param string $where WHERE clause
     * @param array $whereParams WHERE clause parameters
     * @return int Number of affected rows
     */
    public function update($table, $data, $where, $whereParams = []) {
        try {
            $set = [];
            foreach (array_keys($data) as $column) {
                $set[] = "$column = ?";
            }
            
            $query = "UPDATE $table SET " . implode(', ', $set) . " WHERE $where";
            $params = array_merge(array_values($data), $whereParams);
            
            $stmt = $this->query($query, $params);
            $affectedRows = $stmt->affected_rows;
            $stmt->close();
            
            return $affectedRows;
        } catch (Exception $e) {
            error_log("Update query error: " . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * Execute a DELETE query
     * @param string $table Table name
     * @param string $where WHERE clause
     * @param array $params WHERE clause parameters
     * @return int Number of affected rows
     */
    public function delete($table, $where, $params = []) {
        try {
            $query = "DELETE FROM $table WHERE $where";
            
            $stmt = $this->query($query, $params);
            $affectedRows = $stmt->affected_rows;
            $stmt->close();
            
            return $affectedRows;
        } catch (Exception $e) {
            error_log("Delete query error: " . $e->getMessage());
            throw $e;
        }
    }
    
    /**
     * Begin a transaction
     * @return bool True on success, false on failure
     */
    public function beginTransaction() {
        return $this->conn->begin_transaction();
    }
    
    /**
     * Commit a transaction
     * @return bool True on success, false on failure
     */
    public function commit() {
        return $this->conn->commit();
    }
    
    /**
     * Rollback a transaction
     * @return bool True on success, false on failure
     */
    public function rollback() {
        return $this->conn->rollback();
    }
    
    /**
     * Close the database connection
     */
    public function close() {
        if ($this->conn) {
            $this->conn->close();
            $this->conn = null;
        }
    }
    
    /**
     * Convert array values to references for bind_param
     * @param array $arr Array to convert
     * @return array Array of references
     */
    private function refValues($arr) {
        $refs = [];
        foreach ($arr as $key => $value) {
            $refs[$key] = &$arr[$key];
        }
        return $refs;
    }
    
    /**
     * Prevent cloning of the instance (Singleton pattern)
     */
    private function __clone() {}
    
    /**
     * Prevent unserializing of the instance (Singleton pattern)
     */
    public function __wakeup() {
        throw new Exception("Cannot unserialize singleton");
    }
    
    /**
     * Destructor to close the database connection
     */
    public function __destruct() {
        $this->close();
    }
}
