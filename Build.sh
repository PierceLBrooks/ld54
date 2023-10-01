#!/bin/sh
python3 -m pip install -r ./requirements.txt
python3 ./nt2owl.py ./ldj54.nt ./nt2owl.py.owl
python3 -m buildster ./Project.xml

