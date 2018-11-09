#!/bin/sh

# Indexing files
updatedb

# Start supervisord and services
/usr/bin/supervisord --nodaemon -c /etc/supervisord.conf
