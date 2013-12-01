/*
 * AstroPersistence.h -- Some utility classes for persistence
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroPersistence_h
#define _AstroPersistence_h

#include <string>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

namespace astro {
namespace persistence {

/**
 * \brief abstraction field field values
 */
class FieldValue {
public:
	virtual int	intValue() const = 0;
	virtual double	doubleValue() const = 0;
	virtual std::string	stringValue() const = 0;
	virtual bool	isnull() const { return false; }
	virtual std::string	toString() const { return stringValue(); }
};

typedef std::shared_ptr<FieldValue>	FieldValuePtr;

/**
 * \brief Factory class to produce field value objects
 */
class FieldValueFactory {
public:
	FieldValuePtr	get(int value);
	FieldValuePtr	get(double value);
	FieldValuePtr	get(const std::string& value);
	FieldValuePtr	get(const char *value);
};

/**
 * \brief Wrapper class for field values
 */
class Field : public std::pair<std::string, FieldValuePtr> {
public:
	Field(const std::string& name, FieldValuePtr value)
		: std::pair<std::string, FieldValuePtr>(name, value) { }
	int	intValue() const { return second->intValue(); }
	double	doubleValue() const { return second->doubleValue(); }
	std::string	stringValue() const { return second->stringValue(); }
	bool	operator==(const std::string& othername) const {
		return first == othername;
	}
	const std::string&	name() const { return first; }
	const FieldValuePtr	value() const { return second; }
};

std::ostream&	operator<<(std::ostream& out, const Field& field);

/**
 * \brief A Row is a vector of fields, together with a vector of field names
 * 
 * A row can access fiel values by name
 */
class Row : public std::vector<Field> {
public:
	const FieldValuePtr&	operator[](const size_t idx) const {
		return std::vector<Field>::operator[](idx).second;
	}
	const FieldValuePtr&	operator[](const std::string& fieldname) const {
		std::vector<Field>::const_iterator	i
			= std::find(begin(), end(), fieldname);
		if (i == end()) {
			throw std::runtime_error("column name not found");
		}
		return i->second;
	}
};

std::ostream&	operator<<(std::ostream& out, const Row& row);

/**
 * \brief A query result is a list of rows, and a list of field names
 */
class Result : public std::list<Row> {
public:
};

std::ostream&	operator<<(std::ostream& out, const Result& result);

/**
 * \brief Interface for statements
 */
class Statement {
	std::string	_query;
public:
	const std::string&	query() const { return _query; }
	Statement(const std::string& query) : _query(query) { }
	virtual void	bindInteger(int colno, int value) = 0;
	void	bind(int colno, int value) {
		bindInteger(colno, value);
	}
	virtual void	bindDouble(int colno, double value) = 0;
	void	bind(int colno, double value) {
		bindDouble(colno, value);
	}
	virtual void	bindString(int colno, const std::string& value) = 0;
	void	bind(int colno, const std::string& value) {
		bindString(colno, value);
	}
	void	bind(int colno, const FieldValuePtr& value);
	virtual void execute() = 0;
protected:
	virtual Field	field(int colno) = 0;
	virtual Row	row() = 0;
public:
	virtual Result	result() = 0;
	virtual int	integerColumn(int colno) = 0;
	virtual double	doubleColumn(int colno) = 0;
	virtual std::string	stringColumn(int colno) = 0;
};

typedef std::shared_ptr<Statement>	StatementPtr;

/**
 * \brief The generic backend interface
 */
class DatabaseBackend {
public:
	virtual std::string	escape(const std::string& value) = 0;
	virtual Result	query(const std::string& query) = 0;
	virtual std::vector<std::string>
		fieldnames(const std::string& tablename) = 0;
	virtual void	begin() = 0;
	virtual void	commit() = 0;
	virtual void	rollback() = 0;
	virtual StatementPtr	statement(const std::string& query) = 0;
};
typedef std::shared_ptr<DatabaseBackend>	Database;

/**
 * \brief A factory for creating backends
 */
class DatabaseFactory {
public:
	Database	get(const std::string& name);
};

/**
 * \brief Update Specification used for the database independent interface
 */
class UpdateSpec : public std::map<std::string, FieldValuePtr> {
private:
	std::string	columnlist() const;
	std::string	values() const;
	std::string	update() const;
public:
	std::string	selectquery(const std::string& tablename) const;
	std::string	insertquery(const std::string& tablename) const;
	std::string	updatequery(const std::string& tablename) const;
	void	bind(StatementPtr& stmt) const;
	void	bindid(StatementPtr& stmt, int id) const;
};

/**
 * \brief Table objects are a very stripped down OR mapper
 */
class TableBase {
protected:
	Database	_database;
	std::string	_tablename;
	std::vector<std::string>	_fieldnames;
	std::string	selectquery() const;
public:
	TableBase(Database& database, const std::string& tablename);
	Row	rowbyid(long objectid);
	long	nextid();
	long	addrow(const UpdateSpec& updatespec);
	void	updaterow(long objectid, const UpdateSpec& updatespec);
	bool	exists(long objectid);
	void	remove(long objectid);
};

// The table template create below from the TableBase class needs a
// Table descriptor class that needs some
//
// class table_adapter {
// static std::string	tablename();
// object	row_to_object(int objectid, const Row& row);
// UpdateSpec	object_to_updatespec(const object& o);
// };
//
// These methods are needed by the Table template to build the table entries

/**
 * \brief Template to map any type of object to a database table
 */
template<typename object, typename dbadapter>
class Table : public TableBase {
public:
	Table(Database& database)
		: TableBase(database, dbadapter::tablename()) { }
	object	byid(long objectid);
	long	add(const object&);
	void	update(long objectid, const object& o);
};

template<typename object, typename dbadapter>
object	Table<object, dbadapter>::byid(long objectid) {
	return dbadapter::row_to_object(objectid, rowbyid(objectid));
}

template<typename object, typename dbadapter>
long	Table<object, dbadapter>::add(const object& o) {
	return addrow(dbadapter::object_to_updatespec(o));
}

template<typename object, typename dbadapter>
void	Table<object, dbadapter>::update(long objectid, const object& o) {
	updaterow(objectid, dbadapter::object_to_updatespec(o));
}

} // namespace persistence
} // namespace astro

#endif /* _AstroPersistence_h */
