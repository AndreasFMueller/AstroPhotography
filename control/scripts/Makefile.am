#
# Makefile.am
#
# (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswli
#

sbin_SCRIPTS = startastro

noinst_SCRIPTS = snowstar.service sky.png.php status.php status.php astrofb \
	getent

if ENABLE_SYSTEMD

systemddir = /etc/systemd/system
systemd_DATA = snowstar.service

varlogdir = ${prefix}/var/log
varlog_DATA = README.log

repodir = ${prefix}/repo
repo_DATA = README.repo

imagesdir = ${prefix}/var/images
images_DATA = README.images

databasedir = ${prefix}/var
database_DATA = README.database database-dist.db

database-dist.db:	database.sql
	sqlite3 database-dist.db < database.sql

endif

EXTRA_DIST = README.log README.repo README.images README.database database.sql


