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

imagesdir = ${prefix}/var/images
images_DATA = README.images

databasedir = ${prefix}/var
database_DATA = README.database database-dist.db

database-dist.db:	database.sql
	if [ -f database-dist.db ];  then rm -f database-dist.db; fi
	sqlite3 database-dist.db < database.sql

sharedir = ${prefix}/share
share_DATA = README.starcatalogs

endif

if ENABLE_REPODIR

repodir = ${prefix}/repo
repo_DATA = README.repo

endif

EXTRA_DIST = README.log README.repo README.images README.database database.sql


