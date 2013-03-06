/*
 *  Vector.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef Vector_h__
#define Vector_h__

#include "dojo_common_header.h"

using namespace std;

namespace Dojo	
{
	class Plane;

	typedef glm::mat4x4 Matrix;
	typedef glm::quat Quaternion;
	
	///a wrapper to glm::tvec3, providing the most common Vector algebra needs
	class Vector : public glm::vec3
	{
	public:
		
		static const Vector ZERO;
		static const Vector UNIT_X;
		static const Vector NEGATIVE_UNIT_X;
		static const Vector UNIT_Y;
		static const Vector NEGATIVE_UNIT_Y;
		static const Vector UNIT_Z;
		static const Vector NEGATIVE_UNIT_Z;
		static const Vector ONE;
		static const Vector MAX, MIN;
		
		///xyz are set to the same value, or 0
		Vector( float f = 0 ) : glm::vec3(f)
		{
			
		}
				
		///2D constructor - xy are initialized and z is set to 0
		Vector( float X, float Y ) : glm::vec3( X, Y, 0 )
		{
			
		}
		
		Vector( float x, float y, float z ) : glm::vec3( x,y,z )
		{
			
		}
		
		Vector( const glm::vec3 & v ) : glm::vec3( v.x, v.y, v.z )
		{
			
		}

		Vector( const Vector& v ) : glm::vec3( v.x, v.y, v.z )
		{

		}
				
		inline const Vector& operator += ( const Vector& v )
		{
			x += v.x;
			y += v.y;
			z += v.z;
						
			return *this;
		}
		
		inline const Vector& operator -= ( const Vector& v )
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			
			return *this;
		}

		inline Vector operator - ( const Vector& v ) const 
		{
			return Vector( x - v.x, y - v.y, z - v.z );
		}

		inline Vector operator * ( float s ) const 
		{			
			return Vector( x * s, y * s, z * s );
		}

		inline float operator * ( const Vector& v ) const 
		{
			return glm::dot( (const glm::vec3&)(*this), (const glm::vec3&)v );
		}

		inline Vector operator ^ ( const Vector& v ) const 
		{
			return glm::cross( *this, v );
		}

		///returns the length of this Vector
		inline const float length() const
		{
			return sqrtf( x*x + y*y + z*z );
		}
		
		///returns a normalized copy of this Vector
		inline Vector normalized() const
		{
			float l = length();
			return Vector( x/l, y/l, z/l );
		}

		///linearly interpolates the two vectors
		inline Vector lerp( float s, const Dojo::Vector& v ) const
		{			
			float invs = 1.f - s;			
			return Vector( x*s + invs*v.x, y*s + invs*v.y, z*s + invs*v.z);
		}
		
		///returns a vector with abs componenents
		inline Vector absComponents() const
		{
			return Vector( abs(x), abs(y), abs(z) );
		}
		
		///multiplies component by component
		inline Vector mulComponents( const Vector& v ) const
		{
			return Vector( x*v.x, y*v.y, z*v.z );
		}

		///divides component by component
		inline Vector divideComponents( const Vector& v ) const
		{
			return Vector( x/v.x, y/v.y, z/v.z );
		}

		///returns the distance from this to v
		inline float distance( const Vector& v ) const 
		{
			return sqrt( distanceSquared(v) );
		}

		///returns the squared (and faster to compute) distance from this to v
		inline float distanceSquared( const Vector& v ) const
		{
			return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z);
		}

		inline bool isNear( const Vector& v, float threshold = 0.1 )
		{
			return distanceSquared( v ) < threshold*threshold;
		}

		///returns the dot product with this vector with only positive components
		inline float absDot( const Vector& v ) const 
		{
			return abs( x * v.x ) + abs( y * v.y ) + abs( z * v.z );
		}
		
		inline String toString() const
		{
			return "(" + String( x ) + "," + String( y ) + "," + String( z ) + ")";
		}
				
		inline size_t getHash() const
		{
			return (((size_t)x) * 73856093) ^ (((size_t)y) * 19349663) ^ (((size_t)z) * 83492791);
		}

		///reflect this vector on the plane with the given normal
		inline Vector reflect( const Vector& normal ) const
		{
			return 2.f * normal * ( normal * *this ) - *this;
		}

		///refracts this vector on the plane with the given normal, where eta is the refraction indices ratio
		inline Vector refract( const Vector& n, float eta ) const
		{
			const Vector& i = -*this;

			float N_dot_I = n*i;
			float k = 1.f - eta * eta * (1.f - N_dot_I * N_dot_I);
			if (k < 0.f)
				return 0;
			else
				return eta * i - (eta * N_dot_I + sqrtf(k)) * n;
		}

	protected:
	};
}

namespace std
	{
		///hash specialization for unordered_maps
		template<>
		struct hash<Dojo::Vector>
		{	// hash functor for vector
			size_t operator()(const Dojo::Vector& _Keyval) const
			{
				return _Keyval.getHash();
			}
		};
}

#endif