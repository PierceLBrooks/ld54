
# Author: Pierce Brooks

import re
import os
import sys
import json
import wget
import shlex
import inspect
import subprocess

types = """
{
"subClassOf": "http://www.w3.org/2000/01/rdf-schema#subClassOf",
"Class": "http://www.w3.org/2002/07/owl#Class",
"type": "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
"onProperty": "http://www.w3.org/2002/07/owl#onProperty",
"label": "http://www.w3.org/2000/01/rdf-schema#label",
"nil": "http://www.w3.org/1999/02/22-rdf-syntax-ns#nil",
"rest": "http://www.w3.org/1999/02/22-rdf-syntax-ns#rest",
"ObjectProperty": "http://www.w3.org/2002/07/owl#ObjectProperty",
"first": "http://www.w3.org/1999/02/22-rdf-syntax-ns#first",
"someValuesFrom": "http://www.w3.org/2002/07/owl#someValuesFrom",
"allValuesFrom": "http://www.w3.org/2002/07/owl#allValuesFrom",
"onProperty": "http://www.w3.org/2002/07/owl#onProperty",
"intersectionOf": "http://www.w3.org/2002/07/owl#intersectionOf",
"unionOf": "http://www.w3.org/2002/07/owl#unionOf",
"Restriction": "http://www.w3.org/2002/07/owl#Restriction",
"disjointWith": "http://www.w3.org/2002/07/owl#disjointWith",
"contributor": "http://purl.org/dc/terms/contributor",
"license": "http://purl.org/dc/terms/license",
"versionInfo": "http://www.w3.org/2002/07/owl#versionInfo",
"title": "http://purl.org/dc/elements/1.1/title",
"description": "http://purl.org/dc/elements/1.1/description",
"provenance": "http://purl.org/dc/terms/provenance",
"Ontology": "http://www.w3.org/2002/07/owl#Ontology",
"AnnotationProperty": "http://www.w3.org/2002/07/owl#AnnotationProperty",
"versionIRI": "http://www.w3.org/2002/07/owl#versionIRI"
}
"""

def script():
  return os.path.abspath(inspect.getframeinfo(inspect.currentframe()).filename)

def run(origin, target):
    converter = os.path.join(os.getcwd(), "ont-converter.jar")
    command = "java -jar %s -i %s -if nt -o %s -of fss -v -f" % (converter, script()+".nt", target)
    url = "https://github.com/sszuev/ont-converter/releases/download/2.0.1/ont-converter.jar"
    data = json.loads(types.strip())
    data["ldj54"] = "http://127.0.0.1:8080/ldj54"
    handle = open(origin, "r")
    lines = handle.readlines()
    handle.close()
    handle = open(script()+".nt", "w")
    errors = []
    for line in lines:
        line = line.strip()
        if (len(line) < 4):
            errors.append(1)
            continue
        if not (line.endswith(" .")):
            errors.append(2)
            continue
        line = re.split("(>)\s(<|\")", line[:(len(line)-2)].strip())
        while not (len(line) == 3):
            for i in range(len(line)):
                if (line[i] == ">"):
                    line[i-1] = line[i-1]+line[i]
                    line = line[:i]+line[(i+1):]
                    break
                elif ((line[i] == "<") or (line[i] == "\"")):
                    line[i+1] = line[i]+line[i+1]
                    if (i+1 == len(line)-1):
                        line = line[:i]+[line[i+1]]
                    else:
                        line = line[:i]+line[(i+1):]
                    break
        for i in range(len(line)):
            if (len(line[i]) < 3):
                line = None
                errors.append(3)
                break
            if ((line[i][0:1] == "<") and (line[i][(len(line[i])-1):] == ">")):
                line[i] = line[i][1:(len(line[i])-1)]
            else:
                continue
            if (line[i] in data):
                line[i] = "<"+data[line[i]]+">"
            else:
                line[i] = "<http://127.0.0.1:8080/ldj54#"+line[i]+">"
        if (line == None):
            errors.append(4)
            continue
        sub = line[0].strip()
        pred = line[1].strip()
        obj = line[2].strip()
        line = []
        line.append(sub)
        line.append(pred)
        line.append(obj)
        for i in range(len(line)):
            if not ((line[i][0:1] == "\"") or (line[i][0:1] == "<")):
                line[i] = "\""+line[i]+"\""
        handle.write(" ".join(line)+" .\n")
    handle.close()
    #print(str(errors))
    if not (os.path.exists(converter)):
        wget.download(url, out=os.getcwd())
        if not (os.path.exists(converter)):
            return -1
    print(command)
    result = subprocess.check_output(shlex.split(command))
    if (result == None):
        return -2
    print(result.decode())
    return 0

def launch(arguments):
    if not (len(arguments) == 3):
        return False
    origin = arguments[1]
    target = arguments[2]
    result = run(origin, target)
    print(str(result))
    if not (result == 0):
        return False
    return True

if (__name__ == "__main__"):
    print(str(launch(sys.argv)))

