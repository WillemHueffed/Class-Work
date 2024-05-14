#!/ bin / bash

#Variables
username = "your_username" server_address = "your_server_address" remote_dir =
    "/path/to/destination_directory" local_dir = "/path/to/local/files"

#SFTP Commands
    sftp_commands = "\
cd $remote_dir
    put $local_dir/*"

# Execute SFTP with commands
sftp $username@$server_address <<EOF
$sftp_commands
exit
EOF
