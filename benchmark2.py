#!/usr/bin/env python

import time
import libUllur as Ullur
import random

client = Ullur.UllurClient()

client.connect()

#L = 100000
#t0 = time.time()
#for i in range( L ):
#	x = random.random()
#	y = random.random()
#	client.put( x, y, random.randint( 1, 100*L) )
#	if i % 1000 == 0:
#		print float(i)/L * 100, "%"
t1 = time.time()

#print "It took %.3f secs to insert %i entries into Ullur. (%.3f entries/s)"%( t1-t0, L, L/(t1-t0) )

N = 10000
z = 0
t2 = time.time()
for i in range( N ):
	x = random.random()
	y = random.random()
	#print i, z, x, y
	data = client.get( x, y, 0.003 )
	z += len( data )
	
t3 = time.time()

print "It took %.3f secs to do %i queries and get %i entries from Ullur. (%.3f queries / s, %.1f entries / query )"%( t3-t2, N, z, N/(t3-t2), float(z)/N )


client.disconnect()
