#!/bin/bash

# Variables
username="whueffed"
server_address="cs1.seattleu.edu"
remote_dir="/Class-Work/Networking/hw4"
local_dir="*"

# SFTP Commands
sftp_commands="\
cd $remote_dir
put $local_dir/*"

# Execute SFTP with commands
sshpass -p "Indianola98532" sftp $username@$server_address <<EOF
cd Class-Work/Networking/hw4
put transport.c 
exit
EOF
