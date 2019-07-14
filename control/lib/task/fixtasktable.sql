--
-- add the new columns to the task queue table
--
-- (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
--
alter table taskqueue add column tasktype integer not null default 0;
alter table taskqueue add column guiderccdindex integer not null default -1;
alter table taskqueue add column guiderccd varchar(256) not null default '';
alter table taskqueue add column guideportindex integer not null default -1;
alter table taskqueue add column guideport varchar(256) not null default '';
alter table taskqueue add column adaptiveopticsindex integer not null default -1;
alter table taskqueue add column adaptiveopticsccd varchar(256) not null default '';
