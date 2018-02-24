#!/usr/bin/python3

import serial
import io
import re
import requests
import logging
import json
import threading
import time
import argparse


# Enabling debugging at http.client level (requests->urllib3->http.client)
# you will see the REQUEST, including HEADERS and DATA, and RESPONSE with HEADERS but without DATA.
# the only thing missing will be the response.body which is not logged.
#try: # for Python 3
#        from http.client import HTTPConnection
#except ImportError:
#        from httplib import HTTPConnection
#HTTPConnection.debuglevel = 1
#
#logging.basicConfig() # you need to initialize logging, otherwise you will not see anything from requests
#logging.getLogger().setLevel(logging.DEBUG)
#requests_log = logging.getLogger("requests.packages.urllib3")
#requests_log.setLevel(logging.DEBUG)
#requests_log.propagate = True

logging.basicConfig(format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p', level=logging.INFO)


print ("gogo")


apikey = ""
nodeIdFake = "testnode"
nodeId = "power"

class NodeData:
    def __init__(self, nodeId):
        self.items = {}
        self.time = int(time.monotonic())
        self.nodeId = nodeId
    def add(self, key, value):
        self.items[key] = value
    def getItems(self):
        return self.items
    def getNodeId(self):
        return self.nodeId
    def getTime(self):
        return self.time

pattern = re.compile('^.*ct(\d+): (.*)$')
patternState = re.compile('^.*State (\S+): (\d+).*$')
patternBegin = re.compile('^.*READ BEGIN.*$')
patternEnd = re.compile('^.*READ END.*$')

class tempReader (threading.Thread):
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name
    def run(self):
        logging.info("Starting " + self.name)
        self.readSerial()
        #self.readFakeData()
        logging.info("Exiting " + self.name)

    def readSerial(self):
        with serial.Serial('/dev/ttyUSB0', 9600) as ser:
            logging.warn("Opened " + ser.name)
            data = NodeData(nodeId)
            while True:
                #sio.flush()
                line = ser.readline()
                if line == None:
                    break
                line = line.decode("utf-8").rstrip()
                #print("L: " + str(line))
                m = pattern.match(line)
                if m:
                    id = m.group(1)
                    temp = m.group(2)
                    logging.info("Power for " + id + "=" + temp)
                    key = "Power" + id
                    data.add(key, temp)
                m = patternState.match(line)
                if m:
                    id = m.group(1)
                    state = m.group(2)
                    logging.info("State for " + id + "=" + state)
                    key = id
                    data.add(key, state)
                m = patternBegin.match(line)
                if m:
                    data = NodeData(nodeId)
                m = patternEnd.match(line)
                if m:
                    tempWriter.addData(data)

    def readFakeData(self):
        for i in range(0, 10000):
            #time.sleep(5)	
            #read some values
            data = NodeData(nodeIdFake)
            data.add("temp0", 0.2 * i)
            data.add("temp1", 0.664 * i)
            data.add("temp2", 0.334 * i)
            data.add("temp3", i)
            data.add("temp4", 1.1 * i)
            data.add("state1", i % 2)
            tempWriter.addData(data)


class tempWriter (threading.Thread):
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name
        self.queue = []
        self.queue2 = []
    def run(self):
        logging.info("Starting " + self.name)
        #print_time(self.name, self.counter, 5)
        while True:
            self.postToEmon()
            time.sleep(1)
        logging.info("Exiting " + self.name)
    def addData(self, data):
        logging.info("Adding data");
        threadLock.acquire()
        self.queue.append(data)
        threadLock.release()
    def postToEmon(self):
        threadLock.acquire()
        for item in self.queue:
            self.queue2.append(item)
        self.queue = []
        threadLock.release()


        if len(self.queue2) > 0:
            logging.info("Posting " + str(len(self.queue2)) + " items");

            data = []
            for d in self.queue2:
                #format data
                data2 = []
                data2.append(d.getTime()) #timestamp
                data2.append(d.getNodeId()) #nodeId
                for (k,v) in d.getItems().items():
                    data2.append({k: v})
                data.append(data2)

            logging.info("json encoding")
            #json encode
            fulldata = {
                    "data": json.JSONEncoder().encode(data),
                    "apikey": apikey,
                    "sentat": int(time.monotonic())
                    }

            try:
                #print(fulldata)
                logging.info("posting..." + str(fulldata))
                r = requests.post('http://192.168.1.150/emoncms/input/bulk', data = fulldata)
                body = r.text
                if r.status_code == requests.codes.ok and body == "ok": #emoncms is stupid, always returns 200
                    logging.info("Post ok: " + str(r.status_code) + " -- " + r.text)
                    #clear data
                    self.queue2 = []
                else:
                    logging.warn("Post error: " + str(r) + " -- " + r.text)
                    time.sleep(30)
            except requests.exceptions.RequestException as e:
                logging.warn("Cannot send data - skipping")
                time.sleep(30)
                #don't clear the queue2, it will retry
        else:
            logging.info("Nothing to post: " + str(len(self.queue2)))



def sleepwhile(count):
    logging.info("Sleeping...")
    time.sleep(count)	
    logging.info("Sleeping done...")


parser = argparse.ArgumentParser(description='Consuming power readings and posting them to emoncms')
parser.add_argument('--apikey', required=True, help='emoncms api key')
args = parser.parse_args()
apikey = args.apikey


logging.info("Starting up")


threadLock = threading.Lock()
threads = []

# Create new threads
thread1 = tempReader("readerThread")
tempWriter = tempWriter("writerThread")

# Start new Threads
thread1.start()
tempWriter.start()

# Add threads to thread list
threads.append(thread1)
threads.append(tempWriter)

# Wait for all threads to complete
for t in threads:
   t.join()
print ("Exiting Main Thread")

#
#
#
#
#pattern = re.compile('^.*Temperature (\d+): (\d+\.\d+)\*C.*$')
#patternState = re.compile('^.*State (\S+): (\d+).*$')
#
#
#
#with serial.Serial('/dev/ttyACM0', 9600) as ser:
#    print("Opened " + ser.name)
#    #sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser))
#    while True:
#        #sio.flush()
#        line = ser.readline()
#        if line == None:
#            break
#        line = line.decode("utf-8")
#        #print("L: " + str(line))
#        m = pattern.match(line)
#        if m:
#            id = m.group(1)
#            temp = m.group(2)
#            print("Temp for " + id + "=" + temp)
#            #http://193.168.1.150/emoncms/input/post?node=emontx&fulljson={"power1":100,"power2":200,"power3":300}&apikey=fecb2214d4a3a86516b33469c6e84506
#            key = "Temp" + id
#            data = {key: temp}
#            postToEmon(data)
#        m = patternState.match(line)
#        if m:
#            id = m.group(1)
#            state = m.group(2)
#            print("State for " + id + "=" + state)
#            #http://193.168.1.150/emoncms/input/post?node=emontx&fulljson={"power1":100,"power2":200,"power3":300}&apikey=fecb2214d4a3a86516b33469c6e84506
#            key = id
#            data = {key: state}
#            postToEmon(data)
#
#
#print ("Done")
#
