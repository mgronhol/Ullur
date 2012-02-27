#include <iostream>
#include <map>
#include <set>
#include <unordered_map>

#include <time.h>
#include <sys/time.h>

#include <cstdio>
#include <cstdlib>

#include "libhash.h"

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	}


void print_set( std::set<uint64_t> res ){
	for( std::set<uint64_t>::iterator it = res.begin() ; it != res.end() ; it++ ){
		std::cout << *it << " ";
		}
	std::cout << std::endl;
	}

int main( int argc, char **argv ){

	
	std::set< uint64_t > hashes;
	std::vector<uint64_t> res_euc, res_hash;
	std::unordered_map< uint64_t, uint64_t > db, db_euc;
	
	std::pair<double, double> test_point = std::make_pair( 0.5, 0.5 );
	uint64_t test_point_h = compute_hash( test_point.first, test_point.second );
	
	if( argc < 2 ){
		std::cerr << "Usage: " << argv[0] << " N" << std::endl;
		}
	
	int N = atoi( argv[1] );//15;
	double radius = 1.0/(1<<N);
	
	std::vector< std::pair<double, double> > points;
	
	double t10 = get_time();
	
	for( size_t i = 0 ; i < 2e6 ; ++i ){
		double x, y;
		x = rand() * 1.0 / RAND_MAX;
		y = rand() * 1.0 / RAND_MAX;
		points.push_back( std::make_pair( x, y ) );
		
		uint64_t H = compute_hash( x, y );
		hashes.insert( H );
		db[ H ] = i;
		db_euc[ i ] = i;
		}
	
	double t11 = get_time();
	
	std::cout << "Initializing took "<< t11-t10 << std::endl;
	
	double t0, t1, t2, t3;
	
	t0 = get_time();
	for( size_t i = 0 ; i < points.size() ; ++i ){
		if( get_squared_distance( points[i], test_point ) < radius * radius ){
			res_euc.push_back( db_euc[ i ] );
			}
		} 
	
	t1 = get_time();
	
	std::cout<< "It took " << (t1 - t0)*1e3 << " msecs" << std::endl;
	
	t2 = get_time();
	size_t iter = 5 * N * N;
	std::vector< std::pair< uint64_t, uint64_t > > areas;
	std::vector<uint64_t> results_inter;
	double t2_1, t2_2;
	
	for( size_t i = 0 ; i < iter ; ++i ){
		areas = get_near_indices( N, test_point_h );
		}
	t2_1 = get_time();
	for( size_t i = 0 ; i < iter ; ++i ){
		results_inter = extract_points( hashes,  areas );
		}
	t2_2 = get_time();
	for( size_t i = 0 ; i < iter ; ++i ){
		res_hash = filter_points( results_inter, db, test_point, radius );
		}
	t3 = get_time();
	
	std::cout<< "It took " << (t3 - t2)/iter * 1e3 << " msecs" << std::endl;
	
	std::cout << "Speed-up factor " << (t1-t0)/((t3 - t2)/iter) << std::endl;
	
	//std::cout << "results_inter.size() = " << results_inter.size()  << std::endl;
	
	std::cout << "Rejection factor " << hashes.size()/((double)results_inter.size() ) << std::endl;
	std::cout << std::endl;
	std::cout << "Time breakout:" << std::endl;
	std::cout <<"\t"<< "get_near_indices: " << (t2_1 - t2)/iter*1e3 << " msec"<< std::endl;
	std::cout <<"\t"<< "extract_points: " << (t2_2 - t2_1)/iter*1e3 << " msec" << std::endl;
	std::cout <<"\t"<< "filter_points: " << (t3 - t2_2)/iter*1e3 << " msec" << std::endl;
	std::cout << std::endl;
	std::cout << "res_euc.size() = " << res_euc.size() << " and res_hash.size() = " << res_hash.size() << std::endl;
	
	return 0;
	}

