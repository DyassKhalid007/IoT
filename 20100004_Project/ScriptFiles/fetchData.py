import urllib.request
import json
import pickle 
import numpy as np
import random
from random import randint
import time




urls = ['http://[aaaa::212:7401:1:101]/','http://[aaaa::212:7402:2:202]/','http://[aaaa::212:7403:3:303]/']

sendback = '3'
mydict = {'temp':0,'hum':0,'light':0}

data = {'Light':[]}

count = 0

avgThreshold=10
def add(obj):
	global count 
	mydict['temp']+=obj['Temperature']
	mydict['hum']+=obj['Humidity']
	mydict['light']+=obj['Light']
	# print("Calling from add function")
	# print(mydict['temp'],"  ",obj['Temperature'])

def calculate_avg():
	mydict['temp']/=avgThreshold
	mydict['hum']/=avgThreshold
	mydict['light']/=avgThreshold

	print("Temp Average=",mydict['temp'])
	print()
	print('Humidity Average=',mydict['hum'])
	print()
	print('Light Average=',mydict['light'])
	print()

def reset():
	mydict['temp']=0
	mydict['hum']=0
	mydict['light']=0

def dataCollection(obj):
	data['Light'].append(obj['Light'])

def dumpData():
	dbfile = open('examplePickle', 'wb')
	pickle.dump(data, dbfile)         
	dbfile.close()

def loadData(): 
    dbfile = open('examplePickle', 'rb')      
    db = pickle.load(dbfile) 
    print(db)
    dbfile.close() 

def doRest():
	global count
	for i in range(3):
		webUrl  = urllib.request.urlopen(urls[i])
		data = webUrl.read()
		encoding = webUrl.info().get_content_charset('utf-8')
		obj = json.loads(data.decode(encoding))
		print("Count=",count+1)
		# print(obj)
		add(obj)
		count = count+1
		# dataCollection(obj)
		if(count%avgThreshold==0):
			calculate_avg()
			reset()
			# dumpData()
			# loadData()

		# webUrl  = urllib.request.urlopen(urls[i]+"3")
		
		
		# webUrl  = urllib.request.urlopen(urls[i]+"4")

def generateTemp(a):
    val = 0 
    if(a>=180):
        b = randint(24,50)
        c = randint(12,24)
        val = random.choice([b,c])
    else:
        b = randint(-15,0)
        c = randint(0,23)
        val = random.choice([b,c])
    return val

def generateHumidity(a):
    val = randint(0,100)
    return val 
        
def defineModel(data):
    coeff = np.array([[ -9.30937009,0.04652369,  0.01890434, -0.00509404]])
    result = np.dot(coeff,data.T)
    # print(result)
    result = 1/(1+np.exp(-result))
    # print(result)
    if result>=0.5:
        return 1
    else:
        return 0
    

def classification():
	global count
	for i in range(3):
		webUrl  = urllib.request.urlopen(urls[i])
		data = webUrl.read()
		encoding = webUrl.info().get_content_charset('utf-8')
		obj = json.loads(data.decode(encoding))
		# print("Count=",count+1)
		a = obj['Light']
		b = generateTemp(a)
		c = generateHumidity(a)
		sample = np.array([[1.0,a,b,c]])
		start = time.process_time() 
		result = defineModel(sample)
		print("Time to run the ML model is:")
		print(time.process_time() - start,"ms")
		print(sample)

		if result:
			#raise alarm
			print("Alarm Raised at",urls[i])
			webUrl  = urllib.request.urlopen(urls[i]+"3")
		else:
			#unraise alarm
			print("Alarm not raised at",urls[i])
			webUrl  = urllib.request.urlopen(urls[i]+"4")

		
		







while True:
	#doRest()
	classification()
