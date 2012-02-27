#include <iostream>
#include <map>
#include <fstream>
#include <unordered_map>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

#include <sys/epoll.h>
#include <fcntl.h>

#include <signal.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

#include "libhash.h"

std::map<int, std::string> connection_buffers;

std::set< uint64_t > hashes;
std::unordered_map< uint64_t, std::vector< uint64_t > > database;

FILE *append_log = NULL;

std::vector< uint64_t > multiget( std::unordered_map< uint64_t, std::vector< uint64_t > > &db, uint64_t key ){
	std::vector< uint64_t > out;
	std::unordered_map< uint64_t, std::vector< uint64_t > >::iterator it;
	
	it = db.find( key );
	if( it != db.end() ){
		for( size_t i = 0 ; i < (it->second).size() ; ++i ){
			out.push_back( (it->second)[i] );
			}
		}
	
	return out;
	}


void send_string( int sock, std::string str ){
	const char *data = str.c_str();
	size_t L = 0;
	int n;
	while( L < str.size() ){
		n = send( sock, data + L, str.size() - L, 0 );
		
		if( n >= 0 ){ L += n; }
		else {
			std::cout << "Error: " << strerror( errno ) << ", L = "<< L << std::endl;
			return;
			}
		}
	}

bool contains_message( std::string& buffer ){
	return buffer.find_first_of( '\r' ) != std::string::npos;
	}


std::string handle_put( double x, double y, uint64_t id ){
	std::string out;
	uint64_t H;
	H = compute_hash( x, y );
	hashes.insert( H );
	//database.insert( std::pair< uint64_t, uint64_t >( H, id ) );
	if( database.find( H ) != database.end() ){
		database[H].push_back( id );
		}
	else{
		database[H] = std::vector< uint64_t >();
		database[H].push_back( id );
		}
	out = "Ok.\r\n";

	return out;
	}
std::string handle_geo_put( double x, double y, uint64_t id ){
	// lat = -90 .. 90
	// lon = -180 .. 180

	x = (x + 90.0)/(2*90.0);
	y = (y + 180)/(2*180.0);

	return handle_put( x, y, id );
	}

std::string handle_get( double x, double y, double r ){
	std::string out;

	std::vector< uint64_t > results_inter, /*results_final,*/ values;
	std::vector< std::pair< uint64_t, uint64_t > > areas;
	int N = -floor( log(r) / log(2) );
	uint64_t test_point_H = compute_hash( x, y );
	std::pair< double, double > test_point( x, y );
	char buffer[256];

/*	if( N < 4 ){
		// linear search is (probablyfaster
		for( std::set< uint64_t >::iterator it = hashes.begin() ; it != hashes.end() ; it++ ){}
		}
*/

	areas = get_near_indices( N, test_point_H );
	results_inter = extract_points( hashes, areas );

	double radius = r*r;
	std::pair< double, double > point;

	for( size_t i = 0 ; i < results_inter.size() ; ++i ){
		point = compute_position( results_inter[i] );

		if( get_squared_distance( point, test_point ) <= radius ){

			values = multiget( database, results_inter[i] );

			for( size_t j = 0 ; j < values.size() ; ++j ){
				memset( buffer, 0, 256 );
				sprintf( buffer, "%lu\n", values[j] );
				out += std::string( buffer );
				}
			}

		}

	return out + "\r\n";
	}

std::string handle_geo_get( double x, double y, double r ){
	// lat = -90 .. 90
	// lon = -180 .. 180

	x = (x + 90.0)/(2*90.0);
	y = (y + 180)/(2*180.0);

	r = r / ( 6380e3 * 3.14159265358979323 * 2 ); // Divide by Earth's circumfence

	return handle_get( x, y, r );
	}

std::string handle_getr( double x0, double y0, double x1, double y1 ){
	std::string out;

        std::vector< uint64_t > results_inter, /*results_final,*/ values;
        std::vector< std::pair< uint64_t, uint64_t > > areas;

        double r = sqrt( (x0-x1)*(x0-x1) + (y0-y1)*(y0-y1) );

	int N = -floor( log(r) / log(2) );

	double x = (x0+x1) / 2.0;
	double y = (y0+y1) / 2.0;

        uint64_t test_point_H = compute_hash( x, y );
        std::pair< double, double > test_point( x, y );
        char buffer[256];


        areas = get_near_indices( N, test_point_H );
        results_inter = extract_points( hashes, areas );

        std::pair< double, double > point;
        for( size_t i = 0 ; i < results_inter.size() ; ++i ){
                point = compute_position( results_inter[i] );
		if( point.first >= x0 && point.first <= x1 ){
			if( point.second >= y0 && point.second <= y1 ){
	                        values = multiget( database, results_inter[i] );

                	        for( size_t j = 0 ; j < values.size() ; ++j ){
                        	        memset( buffer, 0, 256 );
                                	sprintf( buffer, "%lu\n", values[j] );
                                	out += std::string( buffer );
                                	}
				}
			}
		}


	return out + "\r\n";
	}

std::string handle_geo_getr( double x0, double y0, double x1, double y1 ){
	// lat = -90 .. 90
	// lon = -180 .. 180

	x0 = (x0 + 90.0)/(2*90.0);
	y0 = (y0 + 180)/(2*180.0);

	x1 = (x1 + 90.0)/(2*90.0);
	y1 = (y1 + 180)/(2*180.0);

	return handle_getr( x0, y0, x1, y1 );
	}

std::string handle_del( uint64_t id ){
	std::unordered_map< uint64_t, std::vector< uint64_t > >::iterator it;
	it = database.find( id );
	if( it != database.end() ){
		hashes.erase( it->first );
		database.erase( it );
		}
	

	return "Ok.\r\n";
	}


std::string handle_message( std::string& message, int sock ){
	std::string out = message.substr( message.find_first_of( '\r' ) + 1 );
	char command[32], dummy[32];
	float x, y, r, x0, y0, x1, y1;
	uint64_t id;
	sscanf( message.c_str(), "%s ", command );
	std::string cmd = std::string( command );
	std::string response;
	

	
	if( cmd == "PUT" ){
		sscanf( message.c_str(), "%s %f %f %lu", dummy, &x, &y, &id );

		response = handle_put( x, y, id );
		if( sock > 0 ){ fprintf( append_log, "PUT %f %f %lu\r\n", x, y, id ); }

		}
	if( cmd == "GEO-PUT" ){
		sscanf( message.c_str(), "%s %f %f %lu", dummy, &x, &y, &id );
		response = handle_geo_put( x, y, id );
		}
	
	if( cmd == "GET" ){
		sscanf( message.c_str(), "%s %f %f %f", dummy, &x, &y, &r );
		response = handle_get( x, y, r );
		}

	if( cmd == "GEO-GET" ){
		sscanf( message.c_str(), "%s %f %f %f", dummy, &x, &y, &r );
		response = handle_geo_get( x, y, r);
		}

	if( cmd == "GETR" ){
		sscanf( message.c_str(), "%s %f %f %f %f", dummy, &x0, &y0, &x1, &y1 );
		response = handle_getr( x0, y0, x1, y1 );
		}

	if( cmd == "GEO-GETR" ){
		sscanf( message.c_str(), "%s %f %f %f %f", dummy, &x0, &y0, &x1, &y1 );
		response = handle_geo_getr( x0, y0, x1, y1 );
		}

	if( cmd == "DEL" ){
		sscanf( message.c_str(), "%s %lu", dummy, &id );
		response = handle_del( id );
		if( sock > 0 ){ fprintf( append_log, "DEL %lu\r\n", id ); }
		}

//	std::cerr << "Response: '"<<response<<"'" << std::endl;
//	response = "response\r\n";
//	sleep( 1 );
	if( sock > 0 ){
		send_string( sock, response );
		}
	//return out;
	return std::string();
	}


void report(){
	std::cout << "Active connections: " << connection_buffers.size() << ", keys in db: " << database.size() << std::endl;
	fsync( fileno( append_log ) );
	}

void load_append_log(){

	std::string line;
	std::ifstream handle( "ullur.dump" );

	if( handle.is_open() ){
		std::cout << "Loading data from disk..." << std::endl;
		while( handle.good() ){
			std::getline( handle, line );
			if( line.size() < 5 ){ continue; }
			
			line = line.substr( 0, line.find_first_of( '\r' ) );
			
			line += "\r\n";
			
			handle_message( line, -1 );
			}
		}
	
	handle.close();
	
	
	}


int main( int argc, char **argv ){
	int serversock, clientsock;
	
	struct sockaddr_un address;
	
	int epfd = epoll_create( 1 );
	
	static struct epoll_event ev, events[32]; 
	
	int i, N, addrlen, flags, n;
	
	char buffer[ 4096 ];
	
	bool done = false;
	
	clock_t reporting_t0;

	serversock = socket( AF_UNIX, SOCK_STREAM, 0 );
	unlink( "/tmp/ullur.socket" );
	
	memset( &address, 0, sizeof( struct sockaddr_un ) );
	address.sun_family = AF_UNIX;
	sprintf( address.sun_path, "/tmp/ullur.socket" );
	
	
	bind( serversock, (struct sockaddr *)&address, sizeof( address ) );
	
	listen( serversock, 5 );
	
	ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	ev.data.fd = serversock;
	
	epoll_ctl( epfd, EPOLL_CTL_ADD, serversock, &ev );
	
	std::cout << "Server started." << std::endl;
	
	signal( SIGPIPE, SIG_IGN );
	load_append_log();
	append_log = fopen( "ullur.dump", "a" );
	reporting_t0 = 0;
	while( !done ){
		//std::cout <<  time(NULL) - reporting_t0 << std::endl;
		if( (time(NULL) - reporting_t0) > 9 ){
			report();
			reporting_t0 = time(NULL);
			}
		N = epoll_wait( epfd, events, 32, 10000 );
		for( i = 0 ; i < N  ; ++i ){
			
			if( events[i].data.fd == serversock ){
				
				addrlen = sizeof( struct sockaddr_un );
				clientsock = accept( serversock, (struct sockaddr*)&address, (socklen_t *)&addrlen );
				flags = fcntl( clientsock, F_GETFL, 0 );
				fcntl( clientsock, F_SETFL, flags | O_NONBLOCK );
				
				//setsockopt( clientsock, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof( int ) );
	
				ev.events = EPOLLIN /*| EPOLLET */| EPOLLERR | EPOLLRDHUP;
				ev.data.fd = clientsock;
				n = epoll_ctl( epfd, EPOLL_CTL_ADD, clientsock, &ev );
				connection_buffers[ clientsock ] = std::string();
				//std::cerr << "Client connected." << std::endl;
				}
			else {
				if( events[i].events & EPOLLIN ){
					memset( buffer, 0, 4096 );
					n = recv( events[i].data.fd, buffer, 4090, 0 );
					if( n > 0 ){
						//std::cerr << "Data received!" << std::endl;
						connection_buffers[events[i].data.fd] += std::string( buffer );
						if( contains_message( connection_buffers[events[i].data.fd] ) ){
							connection_buffers[events[i].data.fd] = handle_message( connection_buffers[events[i].data.fd], events[i].data.fd );
							}
						
						}
					}
				if( events[i].events & EPOLLRDHUP ){
					//std::cerr << "Client disconnected." << std::endl;
					connection_buffers.erase( events[i].data.fd );
					close( events[i].data.fd );
					
					
					}
				}
			}
		 }
	
	fclose( append_log );
	return 0;
	}
