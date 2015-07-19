#!/bin/bash

while [[ TRUE ]]; do
	hcitool inq |grep -o '..:..:..:..:..:..' | tr '\n' '\0' | xargs -0 /home/benni/project/beehive-sensing/src/beehive_reader
done

