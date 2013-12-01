#! /bin/sh
#
# testdb.sh -- this creates the test database and add's a few rows so that
#              we can play with them during tests
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

sqlite3 testdb.db <<EOF
drop table testtable;
create table testtable (
	id integer not null,
	intfield integer not null default 0,
	floatfield float not null default 0,
	stringfield varchar(256) not null default '',
	primary key (id)
);
insert into testtable(id, intfield, floatfield, stringfield)
values(0, 3, 3.1415926535, 'pi');
insert into testtable(id, intfield, floatfield, stringfield)
values(1, 2, 2.7182818285, 'e');
insert into testtable(id, intfield, floatfield, stringfield)
values(2, 1, 0.6180339887, 'phi');
EOF
