#!/bin/bash

# This file is part of OpenPanel - The Open Source Control Panel
# OpenPanel is free software: you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation, using version 3 of the License.
#
# Please note that use of the OpenPanel trademark may be subject to additional 
# restrictions. For more information, please visit the Legal Information 
# section of the OpenPanel website on http://www.openpanel.com/

../logsplit -b 10 -r -i 180 -m 50000 -s  ./out ./out < ./data/test.input
echo "The ./out diretory should contain splitted logfiles now and a YYYYMM-badwidth.log file"
