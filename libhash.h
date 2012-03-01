/*
 *		Ullur geospatial indexing library 
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

#ifndef _LIBHASH_H_
#define _LIBHASH_H_

#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <stdint.h>
#include <cmath>
#include <vector>
#include <iterator>
#include <list>

uint64_t compute_hash( double x, double y );
std::pair<uint64_t,uint64_t> mask_hash( uint64_t H, size_t N, size_t M );
std::pair<double, double> compute_position( uint64_t H );
std::vector< std::pair<uint64_t, uint64_t> > get_near_indices( uint64_t level, uint64_t H );
double get_squared_distance( std::pair<double, double> &A, std::pair<double, double> &B );

std::vector< uint64_t > extract_points( std::set<uint64_t> & points, std::vector< std::pair<uint64_t, uint64_t> > &areas );
std::vector<uint64_t> filter_points( std::vector< uint64_t > &keys, std::unordered_map<uint64_t, uint64_t> &values, std::pair<double, double> point, double radius );

#endif

