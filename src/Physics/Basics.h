#pragma once
#ifndef PHYSICS_BASICS_H_INCLUDED
#define PHYSICS_BASICS_H_INCLUDED

#include <cstddef>
#include <array>
#include <initializer_list>
#include <cmath>

#ifdef DEBUG
#include <cassert>
#define ASSERT(X) assert(X)
#else
#define ASSERT(X) (void(0))
#endif

template<int D, typename DataType=double>
class Vec
{
	std::array<DataType, D> data;
public:
	static const int Dimension=D;
	// ctor, op=
	Vec() noexcept : data(std::array<DataType,D>()) {}
	Vec(const std::array<DataType,D>& init) noexcept : data(init) {}
	// TODO
	//Vec(const DataType(&init)[D]) noexcept : data{init} {}
	Vec(std::initializer_list<DataType> init) noexcept : data()
	{
		ASSERT(init.size() == Dimension);
		const DataType* it;
		int i;
		for (i = 0, it = init.begin(); i < Dimension; ++i, ++it)
			data[i] = *it;
	}
	Vec(const Vec<D,DataType>& other) noexcept : data(other.data) {}
	Vec(Vec<D,DataType>&& other) noexcept : data(other.data) {}
	Vec<D,DataType>& operator=(const Vec<D,DataType>& other) noexcept { data=other.data; return *this; }
	Vec<D,DataType>& operator=(Vec<D,DataType>&& other) noexcept { data=other.data; return *this; }
	Vec<D,DataType>& operator=(std::initializer_list<DataType> init) noexcept
	{
		ASSERT(init.size() == Dimension);
		const DataType* it;
		int i;
		for (i = 0, it = init.begin(); i < Dimension; ++i, ++it)
			data[i] = *it;
		return *this;
	}
	// other dimensins
	template<int other_D, typename = std::enable_if_t< (other_D<D) > >
	Vec(const Vec<other_D>& other) noexcept : data{0}
	{
		for(int i=0; i<other.Dimension; ++i)
			data[i]=other.data[i];
	}
	template<int other_D, typename = std::enable_if_t< (other_D<D) > >
	Vec(Vec<other_D>&& other) noexcept : data{0} {
		for(int i=0; i<other.Dimension; ++i)
			data[i]=other.data[i];
	}
	template<int other_D, typename = std::enable_if_t< (other_D<D) > >
	Vec<D,DataType>& operator=(const Vec<other_D>& other) noexcept
	{
		for(int i=0; i<other.Dimension; ++i)
			data[i]=other.data[i];
		for(int i=other.Dimension; i<Dimension; ++i)
			data[i]=0;
		return *this;
	}
	template<int other_D, typename = std::enable_if_t< (other_D<D) > >
	Vec<D,DataType>& operator=(Vec<other_D>&& other) noexcept
	{
		for(int i=0; i<other.Dimension; ++i)
			data[i]=other.data[i];
		for(int i=other.Dimension; i<Dimension; ++i)
			data[i]=0;
		return *this;
	}
	// operators
	inline bool operator==(const Vec<D,DataType>&other) const
	{
		return data==other.data;
	}
	inline DataType operator[](size_t n) const
	{
		ASSERT(n<Dimension);
		return data[n];
	}
	inline DataType& operator[](size_t n)
	{
		ASSERT(n<Dimension);
		return data[n];
	}
	inline Vec<D,DataType>& operator+=(const Vec<D,DataType>& other)
	{
		for(int i=0; i<Dimension; ++i)
			data[i]+=other[i];
		return *this;
	}
	inline Vec<D,DataType>& operator-=(const Vec<D,DataType>& other)
	{
		for(int i=0; i<Dimension; ++i)
			data[i]-=other[i];
		return *this;
	}
	inline Vec<D,DataType> operator+(const Vec<D,DataType>& other) const
	{
		Vec<D,DataType> ret(*this);
		ret+=other;
		return ret;
	}
	inline Vec<D,DataType> operator-(const Vec<D,DataType>& other) const
	{
		Vec<D,DataType> ret(*this);
		ret-=other;
		return ret;
	}
	inline Vec<D,DataType>& operator*=(DataType s)
	{
		for(int i=0; i<Dimension; ++i)
			data[i]*=s;
		return *this;
	}
	inline Vec<D,DataType>& operator/=(DataType s)
	{
		for(int i=0; i<Dimension; ++i)
			data[i]/=s;
		return *this;
	}
	inline Vec<D,DataType> operator*(DataType s) const
	{
		Vec<D,DataType> ret(*this);
		ret*=s;
		return ret;
	}
	inline Vec<D,DataType> operator/(DataType s) const
	{
		Vec<D,DataType> ret(*this);
		ret/=s;
		return ret;
	}
	inline DataType Dot(const Vec<D,DataType>& other) const
	{
		DataType ret=0;
		for(int i=0; i<Dimension; ++i)
			ret+=data[i]*other[i];
		return ret;
	}
	template<int other_D, typename = std::enable_if_t< (other_D==D && D==3) > >
	inline Vec<D,DataType> Cross(const Vec<other_D>& other) const
	{
		Vec<D,DataType> ret;
		ret[0]=data[1]*other[2]-data[2]*other[1];
		ret[1]=data[2]*other[0]-data[0]*other[2];
		ret[2]=data[0]*other[1]-data[1]*other[0];
		return ret;
	}
	inline DataType Length() const
	{
		return sqrt(Dot(*this));
	}
	inline void Normalize()
	{
		(*this)/=Length();
	}
	inline Vec<D,DataType> Normalized() const
	{
		return (*this)/Length();
	}
};

typedef Vec<2> Vec2;
typedef Vec<3> Vec3;


#endif