###
# Sourceforge helper Makefile
###
USER=jerryjacobs
HOSTNAME=shell.sourceforge.net
PROJECT=avra
HTDOCS=/home/groups/a/av/avra/htdocs

all: help

upload: htdocs_scp htdocs_chmod

###
# Help message
###
help:
	@echo "No command specified! Available commands:"
	@echo
	@echo "Current settings"
	@echo "----------------"
	@echo "  User: ${USER}"
	@echo "  Hostname: ${HOSTNAME}"
	@echo "  Project: ${PROJECT}"
	@echo "  Remote htdoc: ${HTDOCS}"
	@echo
	@echo "Shell commands"
	@echo "--------------"
	@echo "  * shell, open a shell"
	@echo "  * shell_create, create a shell"
	@echo
	@echo "htdocs commands"
	@echo "---------------"
	@echo "  * htdocs_scp, copy all files from current dir to remote htdocs"
	@echo "  * htdocs_rm, remove all files from remote htdocs"

###
# Shell
###
shell:
	ssh ${USER},${PROJECT}@${HOSTNAME}

shell_create:
	ssh ${USER},${PROJECT}@${HOSTNAME} create

###
# htdocs
###
htdocs_scp:
	scp -r * ${USER}@${HOSTNAME}:${HTDOCS}

htdocs_chmod:
	ssh ${USER}@${HOSTNAME} chmod -Rv ug+rw ${HTDOCS}/*

htdocs_rm:
	ssh ${USER}@${HOSTNAME} rm -Rv ${HTDOCS}/*
