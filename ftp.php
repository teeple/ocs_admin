<?php
$ftp = ftp_connect( '192.168.1.151');
print_r( $ftp);

$result = ftp_login( $ftp, 'uocs', 'uocs.123');
ftp_pasv( $ftp, 1);
ftp_chdir( $ftp, '/home/uocs/HOME/lua/script/uocs/ui');
ftp_get( $ftp, '/tmp/OcsOpenItem.json', 'OcsOpenItem.json', FTP_ASCII);
