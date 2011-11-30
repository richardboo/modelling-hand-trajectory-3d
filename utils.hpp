////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file	utils.hpp
///
/// @brief	Declares the utils namespace. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_UTILS_H)
#define _UTILS_H

#include <cxcore.h>
#include <cv.h>
#include <sstream>
#include <iostream>
#include <string>



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @namespace	Utils{
///
/// @brief	Some useful utils.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Utils{

	

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	T maximum(T a, T b, T c)
	///
	/// @brief	Determines the maximum of the given three parameters.  
	///
	/// @param	a	The first element. 
	/// @param	b	The second element. 
	/// @param	c	The third element. 
	///
	/// @return	The maximum value. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
	T maximum(T a, T b, T c){
		T bigger = (a > b ? a : b);
		return (c > bigger ? c : bigger);
	}

	template<class T>


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	T minimum(T a, T b, T c)
	///
	/// @brief	Determines the minimum of the given three parameters. 
	///
	/// @param	a	The first element. 
	/// @param	b	The second element. 
	/// @param	c	The third element.
	///
	/// @return	The minimum value. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	T minimum(T a, T b, T c){
		T smaller = (a < b ? a : b);
		return (c < smaller ? c : smaller);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	inline int subMoreThanZero(const int & firstOne, const int & toSub)
	///
	/// @brief	Returns toSub - firstOne if it's greater than 0, 0 otherwise.
	///
	/// @param	firstOne	The number from which toSub should be substracted. 
	/// @param	toSub		The number to substract.
	///
	/// @return	toSub - firstOne if it's greater than 0, 0 otherwise.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	inline float subMoreThanZero(const float & firstOne, const float & toSub){
		return ((firstOne - toSub) > 0 ? (firstOne - toSub) : 0);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	inline int addLessThan(const int & firstOne, const int & toAdd, const int & max)
	///
	/// @brief	Adds toAdd to firstOne if it's less than max, returns max otherwise. 
	///
	/// @param	firstOne	The first number to add. 
	/// @param	toAdd		The second number to add. 
	/// @param	max			The maximum. 
	///
	/// @return	toAdd + firstOne if it's less than max, max otherwise. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	inline float addLessThan(const float & firstOne, const float & toAdd, const float & max){
		return ((firstOne + toAdd) > max ? max : (firstOne + toAdd) );
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	inline std::string int2string(int i)
	///
	/// @brief	Converts int to a std::string. 
	///
	/// @param	i	The int. 
	///
	/// @return	String. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	inline std::string int2string(int i) {
		std::ostringstream buffer;
		buffer << i;
		return buffer.str();
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void printMatrix(CvMat * mtr)
	///
	/// @brief	Print matrix. 
	///
	/// @param	mtr	The matrix. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//void printMatrix(CvMat * mtr);

	bool equal(IplImage* img1, IplImage* img2);

	void dilateReconstruct(IplImage* src, IplImage* mask, IplImage* tmp, IplImage* dst);
	void erodeReconstruct(IplImage* src, IplImage* mask, IplImage* tmp, IplImage* dst);
	void geoDilate(IplImage* src, IplImage* mask, IplImage* dst);
	void geoErode(IplImage* src, IplImage* mask, IplImage* dst);

	bool rectsCollide(CvRect & rect1, CvRect & rect2);
	bool rectIn(CvRect rectSmaller, CvRect & rectBigger);
	
};

#endif