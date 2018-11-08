--
-- database.sql
--
-- (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
--
use snowstar;

create table statusupdate (
	id integer not null auto_increment,
	instrument varchar(32) not null default '',
	updatetime datetime not null default now(),
	avgguideerror float not null default -1,
	ccdtemperature float not null default -273.15,
	lastimagestart datetime not null default now(),
	exposuretime float not null default -1,
	currenttaskid integer not null default -1,
	rightascension float not null default 0,
	declination float not null default 0,
	west tinyint not null default 1,
	filter integer not null default -1,
	longitude float not null default 0,
	latitude float not null default 0,
	primary key(id)
);

grant all on snowstar.statusupdate to snow@localhost identified by 'blu33er';

