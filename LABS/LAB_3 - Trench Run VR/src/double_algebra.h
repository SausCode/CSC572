#pragma once
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#define EPSXS             0.001
#define EPS             0.00001
#define ABS(x) (x < 0 ? -(x) : (x))

#define DOT(v1,v2) (v1.x*v2.x + v1.y*v2.y+v1.z*v2.z)
#define CROSS(rez,v1,v2) \
	rez.x = v1.y*v2.z - v1.z*v2.y; \
	rez.y = v1.z*v2.x - v1.x*v2.z; \
	rez.z = v1.x*v2.y - v1.y*v2.x;

#define SUB(rez,v1,v2) \
	rez.x = v1.x - v2.x; \
	rez.y = v1.y - v2.y; \
	rez.z = v1.z - v2.z;


#define LENGTH(v) (sqrtf(v.x* v.x + v.y*v.y + v.z*v.z))

#define NORMALIZE(v) \
	v.l = LENGTH(v); \
	v.x = v.x / v.l; \
	v.y = v.y / v.l; \
	v.z = v.z / v.l;



class double_vec_
{
public:
long double x,y,z;
double_vec_()
	{
	x=y=z=0;	
	}
~double_vec_()
	{
	
	}
double_vec_(long double x_,long double y_,long double z_)
	{
	x=x_;y=y_;z=z_;	
	}
#ifdef USE_DIRECTX
double_vec_(D3DXVECTOR3 v)
	{
	setvec(v);
	}
void convert(D3DXVECTOR3 v)
	{
	x=v.x;y=v.y;z=v.z;
	}
void convert(D3DXVECTOR2 v)
	{
	x=v.x;y=v.y;z=0;
	}
void convert(D3DXVECTOR3 *v)
	{
	v->x=x;v->y=y;v->z=z;
	}
void convert(D3DXVECTOR2 *v)
	{
	v->x=x;v->y=y;
	}
double_vec_& operator = (D3DXVECTOR3& P2)
	{
	x = P2.x;
	y = P2.y;
	z = P2.z;
	return *this;
	}
D3DXVECTOR3 getvec()
	{
	return D3DXVECTOR3(x, y, z);
	}

void setvec(D3DXVECTOR3 v)
	{
	x = v.x;
	y = v.y;
	z = v.z;
	}
double winkel_vec_100(D3DXVECTOR3 v_xy)
	{
	double_vec_ vec;
	vec = *this;
	vec.normalize();
	double w = acos(vec.x);
	if (abs(w)<0.0005) return 0.0;
	if (w != 0.0 && vec.y<0.)	w = (PIe*2.0) - w;
	if (w < 0) w += (PIe*2.0);
	return w;
	}
#endif
long double dot_(double_vec_ *v)
	{
	long double dat=v->x*x+v->y*y+v->z*z;
	return dat;
	}
void normalize()
	{
	long double len=x*x +y*y + z*z;
	len=sqrt(len);
	x/=len;
	y/=len;
	z/=len;
	}
long double getlen()
	{
	long double len=x*x +y*y + z*z;
	len=sqrt(len);
	return len;
	}
void cross(double_vec_ *a,double_vec_ *b)
	{
	if(!b)return;
	if(!a)return;
	x=a->y*b->z-a->z*b->y;
	y=a->z*b->x-a->x*b->z;
	z=a->x*b->y-a->y*b->x;
	}
vec3 convertGLM()
	{
	vec3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
	}
double_vec_& operator =(const double_vec_& P2)
	 {
	   if (this!=&P2)
		{
			x=P2.x;
			y=P2.y;
			z=P2.z;
		}
	   return *this;
	 }
double_vec_& operator +=(const double_vec_& P2)
	{
	if (this != &P2)
		{
		x += P2.x;
		y += P2.y;
		z += P2.z;
		}
	return *this;
	}
double_vec_& operator -=(const double_vec_& P2)
	{
	if (this != &P2)
		{
		x -= P2.x;
		y -= P2.y;
		z -= P2.z;
		}
	return *this;
	}
double_vec_& operator /=(const double d)
	{
	x /= d;
	y /= d;
	z /= d;	
	return *this;
	}
double_vec_& operator *=(const double d)
	{
	x *= d;
	y *= d;
	z *= d;
	return *this;
	}
const double_vec_ operator+(double_vec_ const& rhs) 
	{ 
	  /* Erzeugen eines neuen Objektes, dessen Attribute gezielt einzeln gesetzt werden. Oder: */ 
	  double_vec_ tmp; //Kopie des linken Operanden 
	  tmp.x=x+rhs.x;
	  tmp.y=y+rhs.y;
	  tmp.z=z+rhs.z;
	  return tmp; 
	}

const double_vec_ operator*(long double const& rhs) 
	{ 
	  /* Erzeugen eines neuen Objektes, dessen Attribute gezielt einzeln gesetzt werden. Oder: */ 
	  double_vec_ tmp; //Kopie des linken Operanden 
	  tmp.x=x*rhs;
	  tmp.y=y*rhs;
	  tmp.z=z*rhs;
	  return tmp; 
	}
const double_vec_ operator/(long double const& rhs) 
	{ 
	/* Erzeugen eines neuen Objektes, dessen Attribute gezielt einzeln gesetzt werden. Oder: */ 
	double_vec_ tmp; //Kopie des linken Operanden 
	tmp.x=x/rhs;
	tmp.y=y/rhs;
	tmp.z=z/rhs;
	return tmp; 
	}
const double_vec_ operator-(double_vec_ const& rhs) 
	{ 
	  /* Erzeugen eines neuen Objektes, dessen Attribute gezielt einzeln gesetzt werden. Oder: */ 
	  double_vec_ tmp; //Kopie des linken Operanden 
	  tmp.x=x-rhs.x;
	  tmp.y=y-rhs.y;
	  tmp.z=z-rhs.z;
	  return tmp; 
	}



};

const double_vec_ operator+(double_vec_ const& lhs,double_vec_ const& rhs);
const double_vec_ operator*(double_vec_ const& lhs,long double const& rhs);
const double_vec_ operator*(long double const& rhs,double_vec_ const& lhs);
const double_vec_ operator/(double_vec_ const& lhs,long double const& rhs);
const double_vec_ operator-(double_vec_ const& lhs,double_vec_ const& rhs);
long double distance_vec(double_vec_ a,double_vec_ b);
long double winkel_vec(double_vec_ *v1, double_vec_ *v2);
long double winkel_vec_z(double_vec_ *v1, double_vec_ *v2);
long double winkel_vec_y(double_vec_ *v1, double_vec_ *v2);
long double winkel_vec_x(double_vec_ *v1, double_vec_ *v2);


class double_mat_
{
private:
	void set_to_array(long double feld[4][4])
	{
	feld[0][0] = _11;		feld[0][1] = _12;		feld[0][2] = _13;		feld[0][3] = _14;
	feld[1][0] = _21;		feld[1][1] = _22;		feld[1][2] = _23;		feld[1][3] = _24;
	feld[2][0] = _31;		feld[2][1] = _32;		feld[2][2] = _33;		feld[2][3] = _34;
	feld[3][0] = _41;		feld[3][1] = _42;		feld[3][2] = _43;		feld[3][3] = _44;
	}
void set_from_array(long double feld[4][4])
	{
	_11 = feld[0][0];		_12 = feld[0][1];		_13 = feld[0][2];		_14 = feld[0][3];
	_21 = feld[1][0];		_22 = feld[1][1];		_23 = feld[1][2];		_24 = feld[1][3];
	_31 = feld[2][0];		_32 = feld[2][1];		_33 = feld[2][2];		_34 = feld[2][3];
	_41 = feld[3][0];		_42 = feld[3][1];		_43 = feld[3][2];		_44 = feld[3][3];
	}
public:
long double
_11, _12, _13, _14,
_21, _22, _23, _24,
_31, _32, _33, _34,
_41, _42, _43, _44;

long double get(int x, int y)
	{
	if ( x > 3 || x < 0 || y>3 || y < 0 )
		return 0.0;
	/*float* z = (float*)&_11;
	float f = z[x * 4 + y];*/
	if ( x == 0 && y == 0 )	return _11;
	if ( x == 0 && y == 1 )	return _12;
	if ( x == 0 && y == 2 )	return _13;
	if ( x == 0 && y == 3 )	return _14;
	if ( x == 1 && y == 0 )	return _21;
	if ( x == 1 && y == 1 )	return _22;
	if ( x == 1 && y == 2 )	return _23;
	if ( x == 1 && y == 3 )	return _24;
	if ( x == 2 && y == 0 )	return _31;
	if ( x == 2 && y == 1 )	return _32;
	if ( x == 2 && y == 2 )	return _33;
	if ( x == 2 && y == 3 )	return _34;
	if ( x == 3 && y == 0 )	return _41;
	if ( x == 3 && y == 1 )	return _42;
	if ( x == 3 && y == 2 )	return _43;
	if ( x == 3 && y == 3 )	return _44;


	return 0.0;
	}
double_mat_()
	{
	set_identity();
	}
void set_identity()
	{
	_12 = _13 = _14 = 0.0;
	_21 = _23 = _24 = 0.0;
	_31 = _32 = _34 = 0.0;
	_41 = _42 = _43 = 0.0;
	_11 = _22 = _33 = _44 = 1.0;
	}
void set_transform_matrix(double_vec_ v)
	{
	set_identity();
	set_transform_part(v);
	}
void set_transform_part(double_vec_ v)
	{
	_41 = v.x;_42 = v.y;_43 = v.z;
	}
void delete_transform_part()
{
	_41 = _42 = _43 = 0;
}
void test()
{
	_14 = _24 = _34 = 0;
	_44 = 1;
}
void transpose()
	{
	double_mat_ m;
	m = *this;
	_12 = m._21; _21 = m._12;
	_13 = m._31; _31 = m._13;
	_14 = m._41; _41 = m._14; _23 = m._32; _32 = m._23;
	_24 = m._42; _42 = m._24;
	_34 = m._43; _43 = m._34;
	/*
		_11, _12, _13, _14,
		_21, _22, _23, _24,
		_31, _32, _33, _34,
		_41, _42, _43, _44;*/
	}
void set_rotation_matrix_x(long double w)
	{
	set_identity();
	_22 = cos(w);	_23 = -sin(w);
	_32 = sin(w);	_33 = cos(w);
	}
void set_rotation_matrix_y(long double w)
	{
	set_identity();
	_11 = cos(w);	_13 = sin(w);
	_31 = -sin(w);	_33 = cos(w);
	}
void set_rotation_matrix_z(long double w)
	{
	set_identity();
	_11 = cos(w);	_12 = -sin(w);
	_21 = sin(w);	_22 = cos(w);
	}
void multiplicate(double_mat_ mat)
	{
	long double a[4][4], b[4][4], c[4][4];
	set_to_array(a);
	mat.set_to_array(b);
	for (int x = 0; x<4; x++)
		{
		for (int j = 0; j<4; j++)
			{
			long double  result = 0;
			for (int y = 0; y<4; y++)
				{
				result += a[x][y] * b[y][j];
				}
			c[x][j] = result;
			}
		}
	set_from_array(c);
	}

double_vec_ multiplicate(double_vec_ vec)
	{
	double_vec_ erg;
	erg.x = _11*vec.x + _21*vec.y + _31*vec.z + _41*1.0;
	erg.y = _12*vec.x + _22*vec.y + _32*vec.z + _42*1.0;
	erg.z = _13*vec.x + _23*vec.y + _33*vec.z + _43*1.0;
	return erg;
	}

double_mat_ & operator= (double_mat_ rhs)
	{
	_11 = rhs._11;	_12 = rhs._12;	_13 = rhs._13;	_14 = rhs._14;
	_21 = rhs._21;	_22 = rhs._22;	_23 = rhs._23;	_24 = rhs._24;
	_31 = rhs._31;	_32 = rhs._32;	_33 = rhs._33;	_34 = rhs._34;
	_41 = rhs._41;	_42 = rhs._42;	_43 = rhs._43;	_44 = rhs._44;
	return *this;
	}
double_mat_ operator*(double_mat_ rhs)
	{
	double_mat_ temp;
	temp=*this;
	temp.multiplicate(rhs);
	return temp;
	}
double_vec_ operator*(double_vec_ rhs)
	{
	double_vec_ temp;
	temp = multiplicate(rhs);
	return temp;
	}

double_mat_ &operator=(mat4 &rhs)
	{
	_11 = rhs[0][0];	_12 = rhs[0][1];	_13 = rhs[0][2];	_14 = rhs[0][3];
	_21 = rhs[1][0];	_22 = rhs[1][1];	_23 = rhs[1][2];	_24 = rhs[1][3];
	_31 = rhs[2][0];	_32 = rhs[2][1];	_33 = rhs[2][2];	_34 = rhs[2][3];
	_41 = rhs[3][0];	_42 = rhs[3][1];	_43 = rhs[3][2];	_44 = rhs[3][3];
	return *this;
	}
mat4 convert_glm()
	{
	mat4 m;
	m[0][0] = _11;	m[0][1] = _12;	m[0][2] = _13;	m[0][3] = _14;
	m[1][0] = _21;	m[1][1] = _22;	m[1][2] = _23;	m[1][3] = _24;
	m[2][0] = _31;	m[2][1] = _32;	m[2][2] = _33;	m[2][3] = _34;
	m[3][0] = _41;	m[3][1] = _42;	m[3][2] = _43;	m[3][3] = _44;
	return m;
	}
double_mat_ &operator*=(mat4 rhsdx)
	{
	double_mat_ rhs;
	rhs = rhsdx;
	*this = rhs;
	return *this;
	}
#ifdef USE_DIRECTX
double_mat_ &operator=(D3DXMATRIX rhs)
	{
	_11 = rhs._11;	_12 = rhs._12;	_13 = rhs._13;	_14 = rhs._14;
	_21 = rhs._21;	_22 = rhs._22;	_23 = rhs._23;	_24 = rhs._24;
	_31 = rhs._31;	_32 = rhs._32;	_33 = rhs._33;	_34 = rhs._34;
	_41 = rhs._41;	_42 = rhs._42;	_43 = rhs._43;	_44 = rhs._44;
	return *this;
	}
double_mat_ &operator*=(D3DXMATRIX rhsdx)
	{
	double_mat_ rhs;
	rhs = rhsdx;
	*this = rhs;
	return *this;
	}

D3DXVECTOR3 get_dx_trans_vec()
{
	D3DXVECTOR3 v;
	v.x = _41;	v.y = _42;	v.z = _43;
	return v;
}
D3DXQUATERNION get_dx_quaternion()
	{
	D3DXMATRIX M = get_dx_float_mat();
	D3DXQUATERNION q;
	D3DXQuaternionRotationMatrix(&q, &M);
	return q;
	}
D3DXMATRIX get_dx_float_mat()
	{
	D3DXMATRIX M;
	M._11 = _11;	M._12 = _12;	M._13 = _13;	M._14 = _14;
	M._21 = _21;	M._22 = _22;	M._23 = _23;	M._24 = _24;
	M._31 = _31;	M._32 = _32;	M._33 = _33;	M._34 = _34;
	M._41 = _41;	M._42 = _42;	M._43 = _43;	M._44 = _44;
	return M;
	}
#endif
};
