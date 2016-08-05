<?php

class Database {
// public
	// Sets PHP system variables to connect the MySQL server.
	public static function Initialize($host, $user, $password) {
		ini_set('mysql.default_host', $host);
		ini_set('mysql.default_user', $user);
		ini_set('mysql.default_password', $password);
	}

	public static function Close() {
		self::Connect(FALSE, TRUE);
	}

	// Replaces variable identifiers in the SQL query. Identifiers are specified
	// with curly braces in the query, and they are replaced into values that
	// are specified with key names in value.
	public static function Format($query, $value) {
		// Register key-variable pairs
		self::FormatTerm($value, TRUE);
		// Parse an SQL query to replace variables
		$query = preg_replace_callback(
		    '%"(?:\\\\.|[^"])*"|\'(?:\\\\.|[^\'])*\'|`(?:\\\\.|[^`])*`|' .
		    '{\s*[\w\d.]*\s*}|/\*(?Us:.*)\*/|--.*|#.*|[\w\d\s=*.:]+|(?s:.)%u',
		    array('Database', 'FormatTerm'), $query);
		return $query;
	}

	// Issues a standalone query and return the boolean result.
	// It returns TRUE if the query is successfully executed.
	public static function Command($query, $value = array()) {
		$result = self::Query($query, $value);
		if (is_resource($result)) self::Free($result);
		return $result ? TRUE : FALSE;
	}

	public static function AffectedRows() {
		return mysql_affected_rows();
	}

	public static function InsertID() {
		return mysql_insert_id();
	}

	// Issues a SELECT query and fetch all the rows.
	public static function Select($query, $value = array()) {
		$result = self::Query($query, $value);
		$table = array();
		if (is_resource($result)) {
			$int_fields = self::IntegerFields($result);
			while ($line = mysql_fetch_assoc($result)) {
				foreach ($int_fields as $key) {
					if (!is_null($line[$key])) {
						$line[$key] = intval($line[$key]);
					}
				}
				$table[] = $line;
			}
		}
		self::Free($result);
		return $table;
	}

	// Issues a SELECT query and fetch only the first row.
	public static function SelectRow($query, $value = array()) {
		$result = self::Query($query, $value);
		$line = FALSE;
		if (is_resource($result)) {
			$int_fields = self::IntegerFields($result);
			$line = mysql_fetch_assoc($result);
			foreach ($int_fields as $key) {
				if (!is_null($line[$key])) {
					$line[$key] = intval($line[$key]);
				}
			}
		}
		self::Free($result);
		return $line;
	}

	// Issues a SELECT query and fetch only the first cell.
	public static function SelectCell($query, $value = array()) {
		$row = self::SelectRow($query, $value);
		return is_array($row) ? array_shift($row) : FALSE;
	}

// private
	// Connects to the SQL server if there is no connection. Returns connection
	// status if status is TRUE. It uses user name for database name, therefore
	// you must use the same name as the user name for database name. It uses
	// only UTF-8 for a character encoding.
	private static function Connect($status = FALSE, $close = FALSE) {
		static $initialized = FALSE, $connected = FALSE;
		if ($status) return $connected;
		if ($close) {
			if ($initialized) {
				mysql_close();
				$initialized = FALSE;
				$connected = FALSE;
			}
		} else {
			if (!$initialized) {
				$connected = (
				    mysql_connect() &&
				    mysql_select_db(ini_get('mysql.default_user')) &&
				    mysql_set_charset('utf8'));
				$initialized = TRUE;
			}
		}
		return $connected;
	}

	private static function Free($result) {
		if (is_resource($result)) mysql_free_result($result);
	}

	private static function Query($query, $value) {
		if (!self::Connect()) return FALSE;
		$query = self::Format($query, $value);
		if (defined('DEBUG_MODE') && DEBUG_MODE === 'sql') {
			echo trim($query) . "\n\n";
		}
		$result = mysql_query($query);
		if ($result === FALSE) {
			trigger_error(mysql_error() . ': ' . $query, E_USER_WARNING);
		}
		return $result;
	}

	private static function EscapeValue($value, $array = FALSE) {
		if (is_array($value) && $array) {
			// An empty array is not allowed because of the SQL specification
			if (!count($value)) {
				return '() VALUES';
			}
			// Treat the value as a VALUES caluse if it is a numbered array
			if (isset($value[0]) && is_array($value[0])) {
				// Look up used keys
				$keys = array();
				foreach ($value as $val) {
					foreach (array_keys($val) as $key) {
						$keys[$key] = TRUE;
					}
				}
				// Generate a VALUES clause
				$result = array();
				foreach ($value as $val) {
					$row = array();
					foreach (array_keys($keys) as $key) {
						$row[] = self::EscapeValue(isset($val[$key]) ? $val[$key] : NULL);
					}
					$result[] = '(' . implode(', ', $row) . ')';
				}
				return '(' . implode(', ', array_keys($keys)) . ') ' .
				    'VALUES' . implode(', ', $result);
			// Otherwise, treat the value as a SET caluse
			} else {
				$result = array();
				foreach ($value as $key => $val) {
					$result[] = "`$key` = " . self::EscapeValue($val);
				}
				return "SET " . implode(', ', $result);
			}
		}
		if (is_null($value)) return 'NULL';
		if (is_int($value) || is_float($value)) return strval($value);
		// Use mysql_real_escape_string if there is a connection
		if (self::Connect(TRUE)) {
			return '"' . mysql_real_escape_string(strval($value)) . '"';
		} else {
			return '"' . mysql_escape_string(strval($value)) . '"';
		}
	}

	// Parses a part of an SQL query to replace variables in an SQL query.
	private static function FormatTerm($match, $initialize = FALSE) {
		// Regisiter key-variable pairs
		static $info = array();
		if ($initialize) {
			$info = $match;
			return NULL;
		}
		// Replace it into a value if it is a variable block
		if (substr($match[0], 0, 1) == '{' && substr($match[0], -1) == '}') {
			$key = trim(substr($match[0], 1, strlen($match[0]) - 2));
			return self::EscapeValue(isset($info[$key]) ? $info[$key] : '', TRUE);
		}
		return $match[0];
	}
	
	private static function IntegerFields($result) {
		$count = mysql_num_fields($result);
		$fields = array();
		for ($i = 0; $i < $count; $i++) {
			$type = strtolower(mysql_field_type($result, $i));
			if ($type == 'int' || $type == 'tinyint' || $type == 'mediumint') {
				$fields[] = mysql_field_name($result, $i);
			}
		}
		return $fields;
	}
}
