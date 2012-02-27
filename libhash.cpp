#include "libhash.h"



uint64_t compute_hash( double x, double y ){
	uint64_t out = 0;
	double cx, cy, d;
	cx = cy = 0.5;
	d = 0.25;
	
	x = x - floor( x );
	y = y - floor( y );
	
	for( size_t i = 0 ; i < 32 ; ++i ){
		uint64_t p = 0;
		
		if( x >= cx ){
			p |= 1;
			cx += d;
			}
		else {
			cx -= d;
			}
		
		if( y <= cy ){
			p |= 2;
			cy -= d;
			}
		else{
			cy += d;
			}
		
		//std::cout << p << " ";
		
		d /= 2.0;
		out |= p;
		if( i < 32 - 1 ){
			out <<= 2;
			}
		}
	//std::cout << std::endl;
	return out;
	}

std::pair<uint64_t,uint64_t> mask_hash( uint64_t H, size_t N ){
	uint64_t  mask = 0;
	for( size_t i = 0 ; i < 32  ; ++i ){
		if( i < N ){
			mask |= 0x03;
			}
		if( i < 31 ){
			mask  <<= 0x02;
			}
		}
	
	return std::make_pair( H & mask, (H & mask) | (~mask) );
	
	} 

/*inline*/ std::pair<double, double> compute_position( uint64_t H ){
	double cx, cy, d;

	cx = 0.5;
	cy = 0.5;
	d = 0.25;
	
	for( uint64_t i = 0 ; i < 32 ; ++i ){
		uint64_t p = 2*(31 - i);
		uint64_t value = (H >> p) & 0x3;
		int x,y;
		x = value & 1;
		y = (value & 2) >> 1;
		
		cx += (2*x-1)*d;
		cy -= (2*y-1)*d;
		
		
		
		d *= 0.5;
		}
	//std::cout << std::endl;
	return std::make_pair( cx, cy );
	}


std::vector< std::pair<uint64_t, uint64_t> > get_near_indices( uint64_t level, uint64_t H ){
	std::vector< std::pair<uint64_t, uint64_t> > out;
	
	double dz = 1.0/( 1 << level );
	
	std::pair<double, double> pos = compute_position( H );
	
	uint64_t h;
	
	for( int p = -1 ; p < 2 ; ++p ){
		for( int q = -1 ; q < 2 ; ++q ){
			double x, y;
			x = pos.first + dz * p;
			y = pos.second + dz * q;
			
			h = compute_hash( x, y );
			
			out.push_back( mask_hash( h, level ) );
			
			
			}
		}
	
	
	return out;
	}


double get_squared_distance( std::pair<double, double> &A, std::pair<double, double> &B ){
	double dx, dy;
	dx = A.first - B.first;
	dy = A.second - B.second;
	return dx*dx + dy*dy;
	} 


std::vector< uint64_t > extract_points( std::set<uint64_t> & points, std::vector< std::pair<uint64_t, uint64_t> > &areas ){
	std::vector<uint64_t> out;
	
	for( size_t i = 0 ; i < areas.size() ; ++i ){
		std::set<uint64_t>::iterator start, stop, it;
			start = points.lower_bound( areas[i].first );
			stop = points.upper_bound( areas[i].second );
		
			for( it = start ; it != stop ; it++ ){
				out.push_back( *it );
				}
				}
	
	return out;
	}


std::vector<uint64_t> filter_points( std::vector< uint64_t > &keys, std::map<uint64_t, uint64_t> &values, std::pair<double, double> point, double radius ){
	std::vector<uint64_t> out;
	std::pair<double, double> pos;
	
	radius = radius*radius;
	for( size_t i = 0 ; i < keys.size() ; ++i ){
		pos = compute_position( keys[i] );
		if( get_squared_distance( pos, point ) <= radius ){
			out.push_back( values[ keys[i] ] );
			}
		}
	
	
	return out;
	}

