/*
 *		Ullur geospatial indexing server 
 *		by Markus Grönholm <markus@alshain.fi>, Alshain Oy, 2012
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *      
 */

#include <iostream>
#include <map>


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

#include <vector>


bool contains_message( std::string& buffer ){
        return buffer.find_first_of( '\r' ) != std::string::npos;
        }

void send_string( int sock, std::string str ){
        const char *data = str.c_str();
        size_t L = 0;
        int n;
//	std::cerr << "Sending string '" << str << "'" << std::endl; 
    //std::cerr << "Sending " << str.size() << " bytes." << std::endl;
        while( L < str.size() ){
                n = send( sock, data + L, str.size() - L, 0 );
                //std::cerr << "L: " << L << std::endl;
                if( n > 0 ){ L += n; }
                }
        }

std::string receive_string( int sock ){
	std::string out;
	char buffer[ 4096 ];
	int n;

	while( !contains_message( out ) ){
		memset( buffer, 0, 4096 );
		n = recv( sock, buffer, 4090, 0 );
		//std::cerr << "receive_string: n: " << n << ", buffer = "<< buffer<< std::endl;
		if( n > 0 ){
			out += std::string( buffer );
			}
		else {
			if( n < 0 ){
				std::cerr << "Error: " << strerror( errno ) << std::endl;
				}
			}
		}
	return out;
	}


int ullur_connect(){
	int sock;
	struct sockaddr_un address;
	sock = socket( AF_UNIX, SOCK_STREAM, 0 );

        memset( &address, 0, sizeof( struct sockaddr_un ) );
        address.sun_family = AF_UNIX;
        sprintf( address.sun_path, "/tmp/ullur.socket" );
	
	connect( sock, (struct sockaddr*)&address, sizeof( address ) );
	
	return sock;	
	}

void ullur_disconnect( int sock ){
	close( sock );
	}

void ullur_put( int sock, double x, double y, uint64_t id ){
	char buffer[256];
	//send_string( sock, "PUT 0.345 0.123 654321\r\n" );
	//std::cout << "RESPONSE: " << receive_string( sock ) << std::endl;
	memset( buffer, 0, 256 );
	sprintf( buffer, "PUT %f %f %li\r\n", x, y, id );

	send_string( sock, std::string( buffer ) );
	//std::cerr << "ullur_put: receive: " << receive_string( sock ) << std::endl;;
	receive_string( sock );
	}

void ullur_del( int sock, uint64_t id ){
	char buffer[256];
	//send_string( sock, "PUT 0.345 0.123 654321\r\n" );
	//std::cout << "RESPONSE: " << receive_string( sock ) << std::endl;
	memset( buffer, 0, 256 );
	sprintf( buffer, "DEL %li\r\n", id );

	send_string( sock, std::string( buffer ) );
	//std::cerr << "ullur_put: receive: " << receive_string( sock ) << std::endl;;
	receive_string( sock );
	}


std::vector< uint64_t > parse_response( std::string response ){
	std::vector< uint64_t > out;
	size_t endpos, curpos, oldpos;
	endpos = response.find_first_of( '\r' );
	oldpos = 0;
	curpos = response.find_first_of( '\n' );
	//printf( "oldpos: %i, curpos: %i, endpos: %i \n", oldpos, curpos, endpos );
	while( curpos < endpos ){
		//printf( "oldpos: %i, curpos: %i, endpos: %i \n", oldpos, curpos, endpos );
		out.push_back( atol( response.substr( oldpos, curpos-oldpos ).c_str() ) );
		oldpos = curpos+1;
		curpos = response.find_first_of( '\n', oldpos );
		}
	
	return out;
	
	}

std::vector< uint64_t > ullur_get( int sock, double x, double y, double r ){
	char buffer[256];
	std::string response;

	memset( buffer, 0, 256 );
	sprintf( buffer, "GET %f %f %f\r\n", x, y, r );
	send_string( sock, std::string( buffer ) );
	response = receive_string( sock );
	//std::cerr << "response: " << response;
	return parse_response( response );
	}


std::vector< uint64_t > ullur_getr( int sock, double x0, double y0, double x1, double y1 ){
	char buffer[256];
	std::string response;

	memset( buffer, 0, 256 );
	sprintf( buffer, "GETR %f %f %f %f\r\n", x0, y0, x1, y1 );
	send_string( sock, std::string( buffer ) );
	response = receive_string( sock );
	//std::cerr << "response: " << response;
	return parse_response( response );
	
	}

void print_vector( std::vector< uint64_t >& L ){
	
	std::cout << "-> ";
	for( size_t i = 0 ; i < L.size() ; ++i ){
		std::cout << L[i] << " ";
		}
	std::cout << std::endl;
	}

int main( int argc, char **argv ){
        int conn;

	conn = ullur_connect();
/*
	std::cout << "Putting points.." << std::endl;
	ullur_put( conn, 0.3, 0.3, 10 );
	ullur_put( conn, 0.4, 0.3, 20 );
	ullur_put( conn, 0.3, 0.5, 30 );

	std::vector< uint64_t > result;

	std::cout << "Getting points.." << std::endl;
	result = ullur_get( conn, 0.30, 0.30, 0.01 );
	print_vector( result );
	result = ullur_get( conn, 0.30, 0.30, 0.15 );
	print_vector( result );
	result = ullur_get( conn, 0.30, 0.30, 0.3 );
	print_vector( result );

	std::cout << "GETR points..." << std::endl;
	result = ullur_getr( conn, 0.29, 0.29, 0.31, 0.31 );
	print_vector( result );
	result = ullur_getr( conn, 0.29, 0.29, 0.41, 0.31 );
	print_vector( result );
	result = ullur_getr( conn, 0.29, 0.29, 0.51, 0.51 );
	print_vector( result );

	std::cout << "DEL points" << std::endl;

	ullur_del( conn, 20 );
	ullur_del( conn, 30 );

	result = ullur_getr( conn, 0.29, 0.29, 0.51, 0.51 );
	print_vector( result );
*/
	for( size_t i = 0 ; i < 100000 ; ++i ){
		double x = rand() *1.0 / RAND_MAX;
		double y = rand() *1.0 / RAND_MAX;
		ullur_put( conn, x, y, i );
		}


	ullur_disconnect( conn );

	return 0;
	}
