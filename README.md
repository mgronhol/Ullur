Ullur geospatial indexing server
========

Ullur is a server for fast geospatial lookups i.e. find me points which
are near this point.


ID = 64 bit unsigned interger

Command set

	PUT x y ID
		Insert ID to coordinates (x,y)
	
	GEO-PUT lat lon ID
		Insert ID to coordinates (lat,lon)
	
	GET x y r
		Fetch all entries, whose distance is <= r from (x,y)
	
	GEO-GET lat lon r
		Fetch all entries, whose (angular) distance is <= r from (lat,lon)
	
	GETR x0 y0 x1 y1
		Fetch all entries inside rectangle (x0,y0) -> (x1,y1)
	
	GEO-GETR lat0 lon0 lat1 lon1
		Fetch all entries inside rectangle (lat0,lon0) -> (lat1,lon1)
	

- Fetching complexity O(log N + M), where N is the number of keys in database 
  and M is the number entries in the search result
- O(log N + M) essentially means that only thing that matters is how many entries
  there are in the result set, NOT how many entries are stored into the server.

- Insert is O(1)


Example usage
======
Easiest way to start is to use the libUllur.py client library for python.

Simple example:

```python
#!/usr/bin/env python
import libUllur as Ullur

client = Ullur.Ullur()
client.connect()

client.put( 0.3, 0.3, 1234 )

print client.get( 0.3, 0.3, 0.01 )

client.disconnect()
```

Installation
===========

Run "make" in the source directory.

Persistence
=========
Ullur saves all inserts to "ullur.dump" file in chronological order.
This file is flushed to disk every 10 seconds.

When Ullur is started, this file is replayed into the server.
