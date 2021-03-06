#pragma once

#include "implicit_function.hpp"
#include "../basic_data_structures.hpp"
#include "../basic_functions.hpp"
#include "boost/multi_array.hpp"
#include "Eigen/Dense"
#include <iostream>
// #include <math.h>       /* sin */

namespace mp5_implicit {

namespace pt = boost::property_tree ;

using Eigen::Matrix;
using Eigen::Dynamic;
using Eigen::Array;

const REAL pi = 3.1415926535897;

// Array<REAL, Dynamic, 1> phi(const Array<REAL, Dynamic, 1> x)
// {   
//     // std::cout << x << std::endl;
//     return (2*(x - Eigen::floor(x)) - 1.0).abs()*2.0 - 1.0;

// }

Array<REAL, Dynamic, 1> phi(const Array<REAL, Dynamic, 1>& x)
{   
    Array<REAL, Dynamic, 1> sin_x(x.rows(),1);
    for (int i=0;i<x.rows();i++){
        sin_x(i, 0) = std::sin(x(i, 0)*2*pi); // math sin
    }
    return sin_x;
}

// Array<REAL, Dynamic, 1> phi(const Array<REAL, Dynamic, 1>& x)
// {   

//     /*

//     shape likes

//       |-| |-|
//       | | | |
//     --| | | |-
//         | |
//         |-|
//     */
//     Array<REAL, Dynamic, 1> M_shape(x.rows(),1);
//     for (int i=0;i<x.rows();i++){

//         REAL abs_x = x(i, 0) - std::floor(x(i, 0));

//         if (abs_x<0.2){
//             M_shape(i, 0) = 0;
//         } else if (0.2<=abs_x<0.5){
//             M_shape(i, 0) = 1;
//         } else if (0.5<=abs_x<0.8){
//             M_shape(i, 0) = -1;
//         } else if (0.8<=abs_x<=1.0){
//             M_shape(i, 0) = 0;
//         } else {
//             std::cout << "this should not happen..";
//         }
//     }
//     return M_shape;
// }


// not tested in screw.hpp
// bool integrity_invariant(const Matrix<REAL, 3, 1> w, 
//                          const Matrix<REAL, 3, 1> u, 
//                          const Matrix<REAL, 3, 1> v,
//                          const Matrix<REAL, 3, 3> UVW,
//                          const Matrix<REAL, 3, 3> UVW_inv,
//                          REAL slen,
//                          REAL r0){

//     bool sane = true;
//     REAL norm_tol, matrix_inv_tol, numerical_min_length;
//     norm_tol = 0.00000001;
//     matrix_inv_tol = 0.000001;
//     numerical_min_length = 0.1;

//     // u_norm_check = u.norm() - 1.0;
//     // use vectorwise norm
//     sane = sane & (abs(w.norm() - 1.0) < norm_tol);
//     sane = sane & (abs(u.norm() - 1.0) < norm_tol);
//     sane = sane & (abs(v.norm() - 1.0)  < norm_tol);

//     Matrix<REAL, 3, 3> UVW_multiply_UVW_inv = UVW * UVW_inv;
//     Matrix<REAL, 3, 3> UVW_inv_multiply_UVW_multiply = UVW_inv*UVW;

//     // skip sane = sane and self.UVW.shape == (3, 3)
//     Matrix<REAL, 3, 3> eye = Matrix<REAL, 3, 3>::Identity();
//     sane = sane & allclose(UVW_multiply_UVW_inv, eye, matrix_inv_tol);
//     sane = sane & allclose(UVW_inv_multiply_UVW_multiply, eye, matrix_inv_tol);
//     sane = sane & (slen > numerical_min_length);
//     sane = sane & (r0 > numerical_min_length);
//     sane = sane & allclose(v, u.cross(w), norm_tol);

//     return sane;
// };

namespace implicit_functions {

class screw : public transformable_implicit_function {

protected:
    // unsign for slen, r0??
    Matrix<REAL, 4, 4> inv_transf_matrix;
    Matrix<REAL, 3, 3> inv_transf_matrix_3_3;
    Matrix<REAL, 3, 1> inv_transf_matrix_neg_xyz;
    REAL slen, r0, delta, twist_rate, phi0; // is the REAL defined here??
    Matrix<REAL, 3, 1> A, w, u, v; // not using Eigen::Vector3d since Vector3d has only type double 
    Matrix<REAL, 3, 3> UVW, UVW_inv;
    REAL x0, y0, z0;

    static Matrix<REAL, Dynamic, 1> implicitFunction(const Matrix<REAL, 3, 1>& A,
                          const Matrix<REAL, 3, 1>& w,
                          const Matrix<REAL, 3, 3>& UVW_inv,
                          const REAL& slen,
                          const REAL& r0,
                          const REAL& delta,
                          const REAL& twist_rate,
                          const REAL& phi0,
                          const Matrix<REAL, Dynamic, 3>& x,
                          const bool return_arg)
    {
        int num_points = x.rows();
        const Matrix<REAL, Dynamic, 3> aa(num_points, 3);
        const Matrix<REAL, 1, 3> A_transpose = A.transpose();

        Matrix<REAL, Dynamic, 1> t(num_points, 1);
        t = (x.rowwise() - A_transpose)*w; // (recenter) * w where w == 0,0,1 or 0,0,-1 is basically getting the z/height value

        Matrix<REAL, 1, Dynamic> t_transpose(1, num_points);
        t_transpose = t.transpose();

        Matrix<REAL, 3, Dynamic> p(3, num_points);
        p = (w*t_transpose).colwise() + A;

        Matrix<REAL, 3, Dynamic> ab(3, num_points);

        ab = UVW_inv * (x.transpose() - p); // ?? this should map the local coordinate polar


        Matrix<REAL, 3, Dynamic> example(3, num_points);

        Matrix<REAL, 1, Dynamic> theta(1, num_points);
        for (int i=0; i<ab.cols(); i++) {

            theta(0, i) = std::atan2(ab(1,i), ab(0, i)); // angle of any given point on polar coordinates
        }

        Matrix<REAL, Dynamic, 1> r(num_points, 1);
        r = (x - p.transpose()).rowwise().norm(); // length to center

        REAL pi2 = pi*2;

        Matrix<REAL, Dynamic, 1> screw_ness(num_points, 1);

        screw_ness = (
                      -r.array() + r0 + delta * 
                        phi( t_transpose.array()/twist_rate - theta.array()/pi2 )
                     ).matrix();

        return screw_ness;

    };

    static void gradient(const REAL ax, const REAL ay, const REAL az, const REAL delta, const REAL phi0, const REAL twist_rate, 
                  const REAL uvwi00, const REAL uvwi01, const REAL uvwi02, const REAL uvwi10, const REAL uvwi11, const REAL uvwi12,
                  const REAL wx, const REAL wy, const REAL wz,
                  const REAL x, const REAL y, const REAL z,
                  REAL& dx, REAL& dy, REAL& dz) {
        dx = M_PI*delta*(-((uvwi00*(-std::pow(wx, 2) + 1) - uvwi01*wx*wy - uvwi02*wx*wz)*(-uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) - uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) - uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))/(std::pow(uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2) + std::pow(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2)) + (uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))*(uvwi10*(-std::pow(wx, 2) + 1) - uvwi11*wx*wy - uvwi12*wx*wz)/(std::pow(uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2) + std::pow(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2)))/M_PI + 2*wx/twist_rate)*cos(M_PI*(2*phi0 - atan2(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))/M_PI + 2*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z))/twist_rate)) - (-wx*wy*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) - wx*wz*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z) + (1.0/2.0)*(-2*std::pow(wx, 2) + 2)*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x))/std::sqrt(std::pow(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x, 2) + std::pow(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y, 2) + std::pow(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z, 2));
        dy = M_PI*delta*(-((uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))*(-uvwi10*wx*wy + uvwi11*(-std::pow(wy, 2) + 1) - uvwi12*wy*wz)/(std::pow(uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2) + std::pow(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2)) + (-uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) - uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) - uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))*(-uvwi00*wx*wy + uvwi01*(-std::pow(wy, 2) + 1) - uvwi02*wy*wz)/(std::pow(uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2) + std::pow(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2)))/M_PI + 2*wy/twist_rate)*cos(M_PI*(2*phi0 - atan2(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))/M_PI + 2*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z))/twist_rate)) - (-wx*wy*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) - wy*wz*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z) + (1.0/2.0)*(-2*std::pow(wy, 2) + 2)*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y))/std::sqrt(std::pow(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x, 2) + std::pow(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y, 2) + std::pow(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z, 2));
        dz = M_PI*delta*(-((uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))*(-uvwi10*wx*wz - uvwi11*wy*wz + uvwi12*(-std::pow(wz, 2) + 1))/(std::pow(uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2) + std::pow(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2)) + (-uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) - uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) - uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))*(-uvwi00*wx*wz - uvwi01*wy*wz + uvwi02*(-std::pow(wz, 2) + 1))/(std::pow(uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2) + std::pow(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), 2)))/M_PI + 2*wz/twist_rate)*cos(M_PI*(2*phi0 - atan2(uvwi10*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi11*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi12*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z), uvwi00*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) + uvwi01*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + uvwi02*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))/M_PI + 2*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z))/twist_rate)) - (-wx*wz*(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x) - wy*wz*(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y) + (1.0/2.0)*(-2*std::pow(wz, 2) + 2)*(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z))/std::sqrt(std::pow(-ax - wx*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + x, 2) + std::pow(-ay - wy*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + y, 2) + std::pow(-az - wz*(wx*(-ax + x) + wy*(-ay + y) + wz*(-az + z)) + z, 2));
    }

public: 


    screw()
    {

        Matrix<REAL, 4, 4> matrix_4_4;

        matrix_4_4 << 1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      0, 0, 0, 1;

        this->inv_transf_matrix = matrix_4_4.inverse();

        // std:cout << matrix_4_4 << "\n";

        // this->inv_transf_matrix_3_3 << inv_transf_matrix(0, 0), inv_transf_matrix(0, 1), inv_transf_matrix(0, 2),
        //                               inv_transf_matrix(1, 0), inv_transf_matrix(1, 1), inv_transf_matrix(1, 2),
        //                               inv_transf_matrix(2, 0), inv_transf_matrix(2, 1), inv_transf_matrix(2, 2);

        // this->inv_transf_matrix_neg_xyz << inv_transf_matrix(0, 3), inv_transf_matrix(1, 3), inv_transf_matrix(2, 3);

        Matrix<REAL, 4, 4> identity = Matrix<REAL, 4, 4>::Identity();


        this->u << 1, 0, 0;

        this->w << 0, 0, 1; 
        this->slen = this->w.norm(); // 1

        this->v << 0, 1, 0;
        REAL outer_diameter = this->v.norm(); 

        this->A << 0, 0, 0;
        this->A  = (this->A.array() - (this->slen/2)*this->w.array()).matrix(); // this only works if w is 0, 0, 1
        
        REAL delta_ratio = 1.5;
        REAL pitch = 0.5;
        string profile = "sin";
        string end_type = "0";

        REAL inner_diameter = outer_diameter/delta_ratio;
        this->r0 = inner_diameter/2;
        this->delta = (outer_diameter/2 - inner_diameter/2);
        this->twist_rate = pitch;

        this->UVW << this->u, this->v, this->w;
        this->UVW_inv = this->UVW.inverse();

        this->phi0 = 0.0; // not used

    }

    screw(Matrix<REAL, 3, 4> matrix, REAL pitch, std::string profile, 
          std::string end_type, REAL delta_ratio, Matrix<REAL, 3, 1> v)
    {   


        // std::cout << matrix << endl;
        // std::cout << pitch << endl;
        // std::cout << profile << endl;
        // std::cout << end_type << endl;
        // std::cout << delta_ratio << endl;
        // std::cout << v << endl; 

        // std::cout << "------------------------using matrix screw constructor: v defined----------------------" << std::endl;
        // screw current cannot do non-uniform 

        // consider u as transformation matrix * [0,1,0],
        // consider v as transformation matrix * [1,0,0],
        // consider w as transformation matrix * [0,0,1],

        // this is a problem since if the screw is stretched, the 
        // matrix.col(0).norm() (length of u) and matrix.col(1).norm() (length of v)


        // invert_matrix(this->transf_matrix, this->inv_transf_matrix);

        // std::cout << "----matrix----" << std::endl;
        // std::cout << matrix << std::endl;

        Matrix<REAL, 4, 4> matrix_4_4;

        matrix_4_4 << matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(0, 3),
                      matrix(1, 0), matrix(1, 1), matrix(1, 2), matrix(1, 3),
                      matrix(2, 0), matrix(2, 1), matrix(2, 2), matrix(2, 3),
                      0, 0, 0, 1;

        this->inv_transf_matrix = matrix_4_4.inverse();

        this->inv_transf_matrix_3_3 << inv_transf_matrix(0, 0), inv_transf_matrix(0, 1), inv_transf_matrix(0, 2),
                                      inv_transf_matrix(1, 0), inv_transf_matrix(1, 1), inv_transf_matrix(1, 2),
                                      inv_transf_matrix(2, 0), inv_transf_matrix(2, 1), inv_transf_matrix(2, 2);

        this->inv_transf_matrix_neg_xyz << inv_transf_matrix(0, 3), inv_transf_matrix(1, 3), inv_transf_matrix(2, 3);

        Matrix<REAL, 4, 4> identity = Matrix<REAL, 4, 4>::Identity();

        assert((this->inv_transf_matrix*matrix_4_4 - identity).maxCoeff() < 0.0001);
        assert((matrix_4_4*this->inv_transf_matrix - identity).maxCoeff() < 0.0001);

        // if ((this->inv_transf_matrix*matrix_4_4 - identity).maxCoeff() < 0.0001) {
        //     std::cout << "check true 0 " << std::endl;
        // }

        // if ((matrix_4_4*this->inv_transf_matrix - identity).maxCoeff() < 0.0001) {
        //     std::cout << "check true 1" << std::endl;
        // }

        // {   

        //     Matrix<REAL, 3, 3> identity_3_3 = Matrix<REAL, 3, 3>::Identity();
        //     Matrix<REAL, 3, 3> matrix_3_3;
        //     matrix_3_3 << matrix(0, 0), matrix(0, 1), matrix(0, 2),
        //               matrix(1, 0), matrix(1, 1), matrix(1, 2),
        //               matrix(2, 0), matrix(2, 1), matrix(2, 2);

        //     assert((this->inv_transf_matrix_3_3*matrix_3_3 - identity_3_3).maxCoeff() < 0.0001);


        //     if ((this->inv_transf_matrix_3_3*matrix_3_3 - identity_3_3).maxCoeff() < 0.0001) {
        //         std::cout << "check true 2" << std::endl;
        //     }


        //     assert((matrix_3_3*this->inv_transf_matrix_3_3 - identity_3_3).maxCoeff() < 0.0001);

        //     if ((matrix_3_3*this->inv_transf_matrix_3_3 - identity_3_3).maxCoeff() < 0.0001) {
        //         std::cout << "check true 2" << std::endl;
        //     }


        // }

        // std::cout << "----this->inv_transf_matrix----" << std::endl;
        // std::cout << this->inv_transf_matrix << std::endl;
        // std::cout << "----this->inv_transf_matrix_neg_xyz----" << std::endl;
        // std::cout << this->inv_transf_matrix_neg_xyz << std::endl;

        // this->u << matrix(0, 0), matrix(1, 0), matrix(2, 0); // first three element from first column
        this->u << 1, 0, 0;
        // this->u = (this->u.array()/this->u.norm()).matrix();

        // this->w << matrix(0, 2), matrix(1, 2), matrix(2, 2); // first three element from first column
        this->w << 0, 0, 1; 
        this->slen = this->w.norm(); // 1

        // std::cout << "----this->slen----" << std::endl;
        // std::cout << this->slen<< std::endl;

        // this->w = (this->w.array()/this->w.norm()).matrix();

        // this->v = v; // if v is defined or if v is not defined
        // this->v = this->u.cross(this->w);
        this->v << 0, 1, 0;
        REAL outer_diameter = this->v.norm(); 

        // std::cout << "----this->outer_diameter----" << std::endl;
        // std::cout << outer_diameter << std::endl;

        // this->v = (this->v.array()/this->v.norm()).matrix();


        // this->A << matrix(0, 3), matrix(1, 3), matrix(2, 3);
        this->A << 0, 0, 0;
        this->A  = (this->A.array() - (this->slen/2)*this->w.array()).matrix(); // this only works if w is 0, 0, 1
        
        REAL inner_diameter = outer_diameter/delta_ratio;
        this->r0 = inner_diameter/2;
        this->delta = (outer_diameter/2 - inner_diameter/2);
        this->twist_rate = pitch;

        // std::cout << "----this->inner_diameter----" << std::endl;
        // std::cout << inner_diameter << std::endl;

        // std::cout << "----this->r0----" << std::endl;
        // std::cout << this->r0 << std::endl;
        // std::cout << "----this->delta----" << std::endl;
        // std::cout << this->delta << std::endl;
        // std::cout << "----this->twist_rate----" << std::endl;
        // std::cout << this->twist_rate << std::endl;

        // std::cout << "----this->delta----" << std::endl;
        // std::cout << this->delta<< std::endl;
        // std::cout << "----this->twist_rate----" << std::endl;
        // std::cout << this->twist_rate<< std::endl;

        this->UVW << this->u, this->v, this->w;
        this->UVW_inv = this->UVW.inverse();

        // std::cout << "----this->UVW----" << std::endl;
        // std::cout << this->UVW<< std::endl;

        this->phi0 = 0.0; // not used
    }


    // }
    // screw(Matrix<REAL, 3, 4> matrix, REAL pitch, std::string profile, 
    //       std::string end_type, REAL delta_ratio)
    // : screw()
    // {   

    //     // screw current cannot do non-uniform 

    //     this->u = matrix.column(0)/matrix.column(0).norm();
    //     this->w = matrix.column(2)/matrix.column(2).norm();
    //     this->v = v;
    //     this->A = matrix.column(3);

    //     this->slen = w.norm();
        
    //     REAL outer_diameter = u.norm();
    //     REAL inner_diameter = outer_diameter/delta_ratio;
    //     this->r0 = (inner_diameter + outer_diameter)/2;
    //     this->delta = outer_diameter - this->r0;
    //     this->twist_rate = pitch;

    //     this->UVW << this->u, this->v, this->w;
    //     this->UVW_inv = this->UVW.inverse();


    // }

    virtual void eval_implicit(const vectorized_vect& x, vectorized_scalar* output) const {


        // std::cout << x[0][0] << std::endl;
        // std::cout << x[0][1] << std::endl;
        // std::cout << x[0][2] << std::endl;

        Matrix<REAL, Dynamic, 3> x_eigen_matrix(x.shape()[0], 3);
        x_eigen_matrix = vectorized_vect_to_Eigen_matrix(x);

        // matrix_vector_product(this->inv_transf_matrix, x_copy);

        // std::cout << "tiger debug x_eigen_matrix" << std::endl;
        // std::cout << x_eigen_matrix(0, 0) << std::endl;
        // std::cout << x_eigen_matrix(0, 1) << std::endl;
        // std::cout << x_eigen_matrix(0, 2) << std::endl;

        // std::cout << "----this->inv_transf_matrix----" << std::endl;
        // std::cout << this->inv_transf_matrix << std::endl;

        // inverse transform matrix apply on the left
        matrix_vector_product(this->inv_transf_matrix_3_3, this->inv_transf_matrix_neg_xyz, x_eigen_matrix);

        // std::cout << "tiger debug x_eigen_matrix" << std::endl;
        // std::cout << x_eigen_matrix(0, 0) << std::endl;
        // std::cout << x_eigen_matrix(0, 1) << std::endl;
        // std::cout << x_eigen_matrix(0, 2) << std::endl;

        Matrix<REAL, Dynamic, 1> implicitFunctionOutput(x.shape()[0], 1);
        implicitFunctionOutput = implicitFunction(this->A, this->w,this->UVW_inv, this->slen, this->r0,
                         this->delta, this->twist_rate, this->phi0, x_eigen_matrix, false);

        // std::cout << "tiger debug implicitFunctionOutput" << std::endl;
        // std::cout << implicitFunctionOutput(0, 0) << std::endl;
        // std::cout << implicitFunctionOutput(0, 1) << std::endl;
        // std::cout << implicitFunctionOutput(0, 2) << std::endl;

        *(output) = Eigen_matrix_to_vectorized_scalar(implicitFunctionOutput);


    }

    virtual void eval_gradient(const vectorized_vect& x, vectorized_vect* output) const {

        const int num_points = x.shape()[0];

        Matrix<REAL, Dynamic, 3> x_eigen_matrix(num_points, 3);
        x_eigen_matrix = vectorized_vect_to_Eigen_matrix(x);

        // inverse transform matrix apply on the left
        matrix_vector_product(this->inv_transf_matrix_3_3, this->inv_transf_matrix_neg_xyz, x_eigen_matrix);

        // std::cout << "----this->inv_transf_matrix----" << std::endl;
        // std::cout << this->inv_transf_matrix << std::endl;


        for (int j=0;j<num_points;j++){

            gradient(this->A(0,0), this->A(1,0), this->A(2,0), 
                     this->delta, this->phi0,
                     this->twist_rate,
                     this->UVW(0,0), this->UVW(0,1), this->UVW(0,2),
                     this->UVW(1,0), this->UVW(1,1), this->UVW(1,2),
                     this->w(0, 0), this->w(1, 0), this->w(2, 0),
                     x_eigen_matrix(j, 0), x_eigen_matrix(j, 1), x_eigen_matrix(j, 2),
                     (*(output))[j][0], (*(output))[j][1], (*(output))[j][2]
                     );

              REAL g0 = (*output)[j][0]; // gx 
              REAL g1 = (*output)[j][1]; // gy
              REAL g2 = (*output)[j][2]; // gz

              // std::cout << g0 << std::endl;
              // std::cout << g1 << std::endl;
              // std::cout << g2 << std::endl;

              (*output)[j][0] = this->inv_transf_matrix(0, 0)*g0 + this->inv_transf_matrix(1, 0)*g1 + this->inv_transf_matrix(2, 0)*g2;
              (*output)[j][1] = this->inv_transf_matrix(0, 1)*g0 + this->inv_transf_matrix(1, 1)*g1 + this->inv_transf_matrix(2, 1)*g2;
              (*output)[j][2] = this->inv_transf_matrix(0, 2)*g0 + this->inv_transf_matrix(1, 2)*g1 + this->inv_transf_matrix(2, 2)*g2;

              // std::cout << g0 << std::endl;
              // std::cout << g1 << std::endl;
              // std::cout << g2 << std::endl;

        };
    }

    // bool integrity_invariant() const {
    // }

    virtual mp5_implicit::bounding_box getboundingbox() const {
        return mp5_implicit::bounding_box{1,2,3,4,5,6};
    }

    static void getScrewParameters(
        Matrix<REAL, 3, 4>& matrix, REAL& pitch, std::string& profile, 
        std::string& end_type, REAL& delta_ratio, Matrix<REAL, 3, 1>& v,
        const pt::ptree& shapeparams_dict
    ){

        int i = 0;
        int j = 0;

        // std::cout << "--- here ---" << std::endl;


        for (const pt::ptree::value_type &element : shapeparams_dict.get_child("matrix")){

            // std::cout << "--- in for loop ---" << std::endl;

            // std::cout << element.second.get_value<float>() << std::endl;

            // std::cout << "---i---" << std::endl;
            // std::cout << i << std::endl;
            // std::cout << "---j---" << std::endl;
            // std::cout << j << std::endl;

            // std::cout << "---m i j ---" << std::endl;

            REAL x = element.second.get_value<float>();

            matrix(i, j) = x;

            // std::cout << matrix(i, j) << std::endl;

            // my_assert(j == 3, "there shuold be three points")

            if (j == 3) {
                j = 0;
                i++;
            } else {
                j++;
            }
            if (i==3) {
                break;
            }
        }

        // my_assert(i == 3, "there should be three row");
        // my_assert(j == 2, "last row there should be three col");

        // std::cout << "---matrix in screw parameter---" << std::endl;

        // std::cout << matrix << std::endl;
        // my_assert(!(A(0,0)==0 && A(1,0)==0 && A(2,0)==0), "possibly A is not initialised correclt");
        // my_assert(!(B(0,0)==0 && B(1,0)==0 && B(2,0)==0), "possibly B is not initialised correclt");

        // std::cout << "getV" << std::endl;
        int getv_counter = 0;
        for (const pt::ptree::value_type &element : shapeparams_dict.get_child("v")) {
                //std::clog << "matrix value : " << x << std::endl;
            v(getv_counter, 0) = element.second.get_value<float>();

            std::cout << element.second.get_value<float>() << std::endl;
            // my_assert(getv_counter<=2, "i should not exceed number 2");
            getv_counter++;
        }
        // std::cout << "getV" << std::endl;

        // std::cout << "getpitch" << std::endl;
        // std::cout << shapeparams_dict.get<float>("pitch") << std::endl;
        pitch = shapeparams_dict.get<float>("pitch");
        // std::cout << pitch << std::endl;
        // std::cout << "getpitch" << std::endl;


        // std::cout << "get-profile" << std::endl;
        profile = shapeparams_dict.get<std::string>("profile");
        // std::cout << profile << std::endl;
        // std::cout << "get-profile" << std::endl;

        // okay 

        // std::cout << "delta_ratio" << std::endl;
        // std::cout << shapeparams_dict.get<REAL>("delta_ratio") << std::endl;
        delta_ratio = shapeparams_dict.get<REAL>("delta_ratio"); // this line has global affects destroy the screw 
        // std::cout << delta_ratio << std::endl;
        // std::cout << "delta_ratio" << std::endl;

        // od = shapeparams_dict.get<REAL>("diameter-outer"); // this line has global affects destroy the screw 

        // std::cout << "--------------------------------------" << std::endl;
        // std::cout << shapeparams_dict.get<REAL>("diameter-inner") << std::endl;
        // std::cout << shapeparams_dict.get<REAL>("diameter-outer") << std::endl;
        // std::cout << "--------------------------------------" << std::endl;


        // std::cout << "get-end-typer" << std::endl;
        end_type = shapeparams_dict.get<std::string>("end_type");

        // std::cout << end_type << std::endl;

        // std::cout << "get-end-typer" << std::endl;
    }

};

} //namespace
} //namespace

