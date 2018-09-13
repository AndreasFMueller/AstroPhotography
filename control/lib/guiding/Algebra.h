/*
 * Algebra.h -- template classes to do the algebra of the kalman filter
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

namespace astro {
namespace guiding {

template<typename T, int n>
class Vector {
	T	_d[n];
public:
	Vector() {
		for (int i = 0; i < n; i++) {
			_d[i] = 0;
		}
	}

	Vector(const Vector<T, n>& other) {
		for (int i = 0; i < n; i++) {
			_d[i] = other._d[i];
		}
	}

	Vector<T,n>&	operator=(const Vector<T,n>& other) {
		for (int i = 0; i < n; i++) {
			_d[i] = other._d[i];
		}
		return *this;
	}

	const T& operator[](int i) const { return _d[i]; }

	T& operator[](int i) { return _d[i]; }

	Vector<T, n>	operator+(const Vector<T, n>& other) const {
		Vector<T, n>	result;
		for (int i = 0; i < n; i++) {
			result._d[i] = _d[i] + other._d[i];
		}
		return result;
	}
	Vector<T, n>	operator-(const Vector<T, n>& other) const {
		Vector<T, n>	result;
		for (int i = 0; i < n; i++) {
			result._d[i] = _d[i] - other._d[i];
		}
		return result;
	}
};

/**
 * \brief Matrix template class
 */
template<typename T, int m, int n>
class Matrix {
	T	_d[n * m];
public:
	Matrix() {
		for (int i = 0; i < n * m; i++) {
			_d[i] = 0;
		}
	}
	Matrix(T d) {
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				(*this)(i,j) = (i=j) ? d : 0;
			}
		}
	}
	const T&	operator()(int i, int j) const {
		return _d[n * i + j];
	}
	T&	operator()(int i, int j) {
		return _d[n * i + j];
	}
	Matrix<T,m,n>	operator+(const Matrix<T,m,n>& other) {
		Matrix<T,m,n>	result;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				result(i,j) = (*this)(i,j) + other(i,j);
			}
		}
		return result;
	}
	Matrix<T,m,n>	operator-(const Matrix<T,m,n>& other) {
		Matrix<T,m,n>	result;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				result(i,j) = (*this)(i,j) - other(i,j);
			}
		}
		return result;
	}
	Matrix<T,n,m>	transpose() const {
		Matrix<T,n,m>	result;
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				result(j,i) = (*this)(i,j);
			}
		}
		return result;
	}
	Matrix<T,m,n>	inverse() const {
		if (m != n) {
			throw std::runtime_error("inverse only for square matrix");
		}
		Matrix<T,m,n>	result;
		return result;
	}
};

template<typename T, int m, int n>
Vector<T,m>	operator*(const Matrix<T,m,n>& A, const Vector<T,n>& v) {
	Vector<T,m>	result;
	for (int i = 0; i < m; i++) {
		T	s = 0;
		for (int j = 0; j < n; j++) {
			s += A(i,j) * result[j];
		}
		result[i] = s;
	}
	return result;
}

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
