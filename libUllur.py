#!/usr/bin/env python

import socket
import time

def send_line( sock, data ):
	L = 0
	while( L < len( data ) ):
		n = sock.send( data[L:] )
		if n > 0:
			L += n

def recv_line( sock ):
	out = ""
	n = ""
	while '\r\n' not in n:
		n = sock.recv( 4096 )
		if len(n) > 0:
			out += n
	return out

def parse_response( response ):
	lines = response.strip().splitlines()
	out = []
	for line in lines:
		line = line.strip()
		if len( line ) > 0:
			out.append( long( line ) )
	return out

class UllurClient( object ):
	def __init__( self ):
		self.sock = socket.socket( socket.AF_UNIX, socket.SOCK_STREAM )

	def connect( self ):
		self.sock.connect( '/tmp/ullur.socket' )

	def disconnect( self ):
		self.sock.close()

	def put( self, x, y, id ):
		send_line( self.sock, "PUT %f %f %i\r\n"%( x,y, id ) )
		recv_line( self.sock )

	def get( self, x, y, r ):
		send_line( self.sock, "GET %f %f %f\r\n"%( x, y, r ) )
		response = recv_line( self.sock )
		return parse_response( response )
	
	def getr( self, x0, y0, x1, y1 ):
		send_line( self.sock, "GETR %f %f %f %f\r\n"%( x0, y0, x1, y1 ) )
		response = recv_line( self.sock )
		return parse_response( response )


	def geo_put( self, x, y, id ):
		send_line( self.sock, "GEO-PUT %f %f %i\r\n"%( x,y, id ) )
		recv_line( self.sock )

	def geo_get( self, x, y, r ):
		send_line( self.sock, "GEO-GET %f %f %f\r\n"%( x, y, r ) )
		response = recv_line( self.sock )
		return parse_response( response )
	
	def geo_getr( self, x0, y0, x1, y1 ):
		send_line( self.sock, "GEO-GETR %f %f %f %f\r\n"%( x0, y0, x1, y1 ) )
		response = recv_line( self.sock )
		return parse_response( response )



# Not implemented atm.
#	def delete( self, id ):
#		send_line( self.sock, "DEL %i\r\n"%id )
#		recv_line( self.sock )




#client = UllurClient()
#
#client.connect()
#
#client.put( 0.3, 0.3, 10 )
#client.put( 0.4, 0.3, 20 )
#client.put( 0.3, 0.5, 30 )
#
#print client.get( 0.3, 0.3, 0.01 )
#print client.get( 0.3, 0.3, 0.15 )
#print client.get( 0.3, 0.3, 0.3 )
#
#print client.getr( 0.29, 0.29, 0.31, 0.31 )
#print client.getr( 0.29, 0.29, 0.41, 0.31 )
#print client.getr( 0.29, 0.29, 0.51, 0.51 )
#
#client.delete( 20 )
#client.delete( 30 )
#
#print client.getr( 0.29, 0.29, 0.51, 0.51 )
#
#
#client.disconnect()
