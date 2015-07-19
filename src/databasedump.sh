#!/bin/bash

influx -database 'mydb' -execute 'select * from temperature_C where time > now() - 2d' -format 'csv' > /tmp/temperatures_log.csv
zip /tmp/temperatures /tmp/temperatures_log.csv

# Add and edit the following line containing <mail from> address  and <mail to> addresses.
#mailx -r <mail from> -s "Temperaturen Log $(date)" -a /tmp/temperatures.zip <mail to> < /home/benni/mailtext.txt

sleep 25
