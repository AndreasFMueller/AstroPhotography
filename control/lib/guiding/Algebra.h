/*
 * Algebra.h -- template classes to do the algebra of the kalman filter
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <iostream>
#include <sstream>
#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */
#include <AstroDebug.h>

namespace astro {
namespace guiding {

/**
 * \brief Vector template class
 */
template<typename T, int n>
class Vector {
	T	_d[n];
public:
	/**
 	 * \brief Construct a constant vector
	 *
	 * \param v	value with which to fill the vector
 	 */
	Vector(T v = 0) {
		for (int i = 0; i < n; i++) {
			_d[i] = v;
		}
	}

	/**
	 * \brief Copy a vector
	 *
	 * \param other		vector to copy
	 */
	Vector(const Vector<T, n>& other) {
		for (int i = 0; i < n; i++) {
			_d[i] = other._d[i];
		}
	}

	/**
	 * \brief Assign a vector
	 *
	 * \param other		vector to assign
	 */
	Vector<T,n>&	operator=(const Vector<T,n>& other) {
		for (int i = 0; i < n; i++) {
			_d[i] = other._d[i];
		}
		return *this;
	}

	/**
 	 * \brief read only access to vector components
	 *
	 * \param i	index of component
	 */
	const T& operator[](int i) const {
		if ((i < 0) || (i >= n)) {
			throw std::range_error("index outside bounds");
		}
		return _d[i];
	}

	/**
 	 * \brief access vector components
	 *
	 * \param i	index of component
	 */
	T& operator[](int i) {
		if ((i < 0) || (i >= n)) {
			throw std::range_error("index outside bounds");
		}
		return _d[i];
	}

	/**
	 * \brief Sum of two vectors
	 *
	 * \param other		the vector to add to the current vector
	 */
	Vector<T, n>	operator+(const Vector<T, n>& other) const {
		Vector<T, n>	result;
		for (int i = 0; i < n; i++) {
			result._d[i] = _d[i] + other._d[i];
		}
		return result;
	}

	/**
	 * \brief Difference of two vectors
	 *
	 * \param other		the vector to add to the current vector
	 */
	Vector<T, n>	operator-(const Vector<T, n>& other) const {
		Vector<T, n>	result;
		for (int i = 0; i < n; i++) {
			result._d[i] = _d[i] - other._d[i];
		}
		return result;
	}

	/**
 	 * \brief Convert a vector to a string
	 */
	std::string	toString() const {
		std::stringstream	out;
		out << *this;
		return out.str();
	}
};

/**
 * \brief Display a vector
 *
 * \param out	the output stream to write the vector to
 * \param v	the vector to write out
 */
template<typename T,int n>
std::ostream&	operator<<(std::ostream& out, const Vector<T,n>& v) {
	out << "[" << std::endl;
	for (int i = 0; i < n; i++) {
		double	value = v[i];
		out << stringprintf("%10.4f", value) << ";" << std::endl;
	}
	out << "]" << std::endl;
	return out;
	
	return out;
}

/**
 * \brief Matrix template class
 *
 * \param T	type of components of the matrix
 * \param m	height of the matrix
 * \param n	width of the matrix
 */
template<typename T, int m, int n>
class Matrix {
	T	_d[n * m];
public:
	/**
	 * \brief Construct a diagonal matrix with value d on the diagonal
	 *
	 * \param d	
	 */
	Matrix(T d = 0) {
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				(*this)(i,j) = (i==j) ? d : 0;
			}
		}
	}

	/**
	 * \brief Read only access to matrix elements
	 *
	 * \param i	row index
	 * \param j	column index
	 */
	const T&	operator()(int i, int j) const {
		return _d[n * i + j];
	}

	/**
	 * \brief Read/write access to matrix elements
	 *
	 * \param i	row index
	 * \param j	column index
	 */
	T&	operator()(int i, int j) {
		if ((i < 0) || (i >= m)) {
			throw std::range_error("row index out of bounds");
		}
		if ((j < 0) || (j >= n)) {
			throw std::range_error("column index out of bounds");
		}
		return _d[n * i + j];
	}

	/**
	 * \brief Sum of two matrices
	 *
 	 * \param other		other matrix to add to the current matrix
	 */
	Matrix<T,m,n>	operator+(const Matrix<T,m,n>& other) {
		Matrix<T,m,n>	result;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				result(i,j) = (*this)(i,j) + other(i,j);
			}
		}
		return result;
	}

	/**
	 * \brief Difference of two matrices
	 *
 	 * \param other		other matrix to subtract from the current matrix
	 */
	Matrix<T,m,n>	operator-(const Matrix<T,m,n>& other) {
		Matrix<T,m,n>	result;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				result(i,j) = (*this)(i,j) - other(i,j);
			}
		}
		return result;
	}

	/**
	 * \brief Get the transposed matrix
	 */
	Matrix<T,n,m>	transpose() const {
		Matrix<T,n,m>	result;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				result(j,i) = (*this)(i,j);
			}
		}
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "transposed matrix: %s",
		//	result.toString().c_str());
		return result;
	}

	/**
	 * \brief Compute the inverse matrix
	 */
	Matrix<T,m,n>	inverse() const {
		if (m != n) {
			throw std::runtime_error("inverse only for square matrix");
		}
		Matrix<T,m,n>	result;
		double	A[m*n], B[m*n];
		for (int i = 0; i < m*n; i++) {
			A[i] = _d[i];
			B[i] = 0;
		}
		for (int i = 0; i < m*n; i += n+1) {
			B[i] = 1;
		}
		int	np = n;
		int	ipiv[n];
		int	info;
		dgesv_(&np, &np, A, &np, ipiv, B, &np, &info);
		if (info < 0) {
			throw std::logic_error("bad dgesv call");
		}
		if (info > 0) {
			throw std::runtime_error("singular matrix");
		}
		for (int i = 0; i < m*n; i++) {
			result._d[i] = B[i];
		}
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "inverse matrix: %s",
		//	result.toString().c_str());
		return result;
	}

	/**
 	 * \brief Convert a vector to a string
	 */
	std::string	toString() const {
		std::stringstream	out;
		out << *this;
		return out.str();
	}
};

/**
 * \brief Display a Matrix
 *
 * \param out	the output stream to write the vector to
 * \param A	the matrix to write out
 */
template<typename T, int m, int n>
std::ostream&	operator<<(std::ostream& out, const Matrix<T,m,n>& A) {
	out << "[" << std::endl;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			out << stringprintf("%10.4f", A(i,j));
			if (j == (n-1)) {
				out << ";" << std::endl;
			} else {
				out << ",";
			}
		}
	}
	out << "]" << std::endl;
	return out;
}

/**
 * \brief Matrix times vector multiplication operator
 *
 * \param A	matrix to multiply
 * \param v	vector to operate on
 */
template<typename T, int m, int n>
Vector<T,m>	operator*(const Matrix<T,m,n>& A, const Vector<T,n>& v) {
	Vector<T,m>	result;
	for (int i = 0; i < m; i++) {
		T	s = 0;
		for (int j = 0; j < n; j++) {
			s += A(i,j) * v[j];
		}
		result[i] = s;
	}
	return result;
}

/**
 * \brief Matrix times vector multiplication operator
 *
 * \param A	matrix to use as left factor
 * \param B	matrix to use as right factor
 */
template<typename T, int l, int m, int n>
Matrix<T,l,n>	operator*(const Matrix<T,l,m>& A, const Matrix<T,m,n>& B) {
	Matrix<T,l,n>	result;
	for (int i = 0; i < l; i++) {
		for (int j = 0; j < n; j++) {
			T	s = 0;
			for (int k = 0; k < m; k++) {
				s += A(i,k) * B(k,j);
			}
			result(i,j) = s;
		}
	}
	return result;
}

} // namespace guiding
} // namespace astro
