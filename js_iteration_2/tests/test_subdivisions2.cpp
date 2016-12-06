#include "gtest/gtest.h"
#include "../subdivision/subdiv_1to2.hpp"
#include "../subdivision/subdiv_1to4.hpp"
#include "../mesh_algorithms.hpp"
#include "faces_test_tools.hpp"
#include "../configs.hpp"

#include "../v2v_f2f.hpp"

using mp5_implicit::CONFIG_C;
using mp5_implicit::easy_edge;
using mp5_implicit::subdivision::midpointmap_type;

// using mp5_implicit::subdivide_multiple_facets_1to4;


vectorized_faces make_example_1234() {

    std::vector<std::vector<vertexindex_type>> f = {{1,2,4}, {3,2,4}, {1,4,5}, {2,3,1}, {7,1,5}, {0,1,3}};
    vectorized_faces faces = f2f(f);

    return faces;
}

auto testcase_square() {

    auto vv = std::vector<std::vector<REAL>>{
        {0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}
    };
    vectorized_vect v = v2v(vv, 1.0 * 10.0 / 4.0 );

    vectorized_faces f = f2f(std::vector<std::vector<vertexindex_type>>{
        {0, 1, 2}, {0, 2, 3}
    });

    return std::pair<vectorized_vect, vectorized_faces>(v,f);
}

auto testcase_triangle() {
    auto vv = std::vector<std::vector<REAL>>{
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}
    };
    vectorized_vect v = v2v(vv, 1.0 );   // 2.0

    vectorized_faces f = f2f(std::vector<std::vector<vertexindex_type>>{
        {0, 1, 2}
    });

    return std::pair<vectorized_vect, vectorized_faces>(v,f);
}

using mp5_implicit::encode_edge__sort;

using mp5_implicit::subdivision::subdivide_1to2;

// #define uncommented false





/**
 * @brief      {Prints a faces array in the columnar format}
 * @param[in]  mark   index of the item that is highlighted. Useful for separating the last index of the original faces. Use -1 to avoid this demarcation.
 */
void print_faces (const vectorized_faces& faces, int mark) {
    cout << "count=" << faces.shape()[0] << std::endl;
    for (int i = 0; i < faces.shape()[0]; ++i ) {
        cout << i <<": ";
        for (int j=0; j < 3; ++j) {
            cout << " " << faces[i][j];
        }
        if (i == mark )
            cout << " <--- ";
        cout << std::endl;
    }
}

bool check_verts_faces (const vectorized_faces& faces, const vectorized_vect& verts) {
    bool ok = true;
    for (int i = 0; i < faces.shape()[0]; ++i ) {
        for (int j=0; j < 3; ++j) {
            ok = ok && faces[i][j] < verts.shape()[0];
        }
    }
    return ok;
}


template <typename T>
std::tuple<T,T,T>  sort_triple(T a, T b, T c) {
    // boost::multi_array<T, 1> triangle_face_
    typename boost::multi_array<T, 1> triangle_face // = triangle_face_;
            {boost::extents[3]};
    triangle_face[0] = a;
    triangle_face[1] = b;
    triangle_face[2] = c;
    typedef typename boost::multi_array<T, 1>::index idxt;
    idxt nd = triangle_face.shape()[0];
    assert(nd==3);
    // cout << "SORT.1" << std::endl;
    for (idxt i = 0; i < nd-1; ++i) {
        // cout << "SORT.i:" << i << std::endl;
        for (idxt j = 0; j <= i; ++j) {
            // cout << "SORT.j:" << j <<  " >?? " << j+1 << "  values: " << triangle_face[j] << " ? " << triangle_face[j+1] <<  std::endl;

            if (triangle_face[j] > triangle_face[j+1]) {
                // cout << "swapping " << j << "<->" << j+1 << std::endl;
                std::swap(triangle_face[j], triangle_face[j+1]);
            }
        }
    }
    for (idxt i = 0; i < nd-1; ++i) {
        assert (triangle_face[i] <= triangle_face[i+1]);
    }
    assert (nd == 3);
    typename std::tuple<T,T,T> s;
    std::get<0>(s) = triangle_face[0];
    std::get<1>(s) = triangle_face[1];
    std::get<2>(s) = triangle_face[2];
    return s;
}


// not used
vectorized_faces sort_faces(const vectorized_faces& faces) {
    auto nf = faces.shape()[0];
    vectorized_faces  faces_idx {boost::extents[nf][3]};
    for (int i = 0; i < faces.shape()[0]; ++i ) {
        auto t3 = sort_triple(faces[i][0], faces[i][1], faces[i][2]);
        // auto t3 = sort_triple(faces[i]);
        faces_idx[i][0] = std::get<0>(t3);
        faces_idx[i][1] = std::get<1>(t3);
        faces_idx[i][2] = std::get<2>(t3);
    }
    return faces_idx;
}

boost::multi_array<long, 1> faces_indices(const vectorized_faces& faces) {
    const long B = 10000;
    auto nf = faces.shape()[0];
    // cout << "===1.1" << std::endl;
    boost::multi_array<long, 1>  faces_idx {boost::extents[nf]};
    // cout << "===1.2" << std::endl;
    for (int i = 0; i < faces.shape()[0]; ++i ) {
        // cout << "===1.3" << std::endl;
        auto t3 = sort_triple(faces[i][0], faces[i][1], faces[i][2]);
        // cout << "===1.3.a" << std::endl;
        long l0 = std::get<0>(t3);
        long l1 = std::get<1>(t3);
        long l2 = std::get<2>(t3);
        faces_idx[i] = l0 + l1*B + l2 * B*B;
    }
    //  cout << "===1.4" << std::endl;
    assert(faces_idx.shape()[0] == faces.shape()[0]);
    return faces_idx;
}

template <typename V>
void pf(V faces1) {
    int ctr = 0;
    for( auto i = faces1.begin(), e = faces1.end(); i!= e; ++i) {
        cout << *i << " ";
        if (ctr % 3 == 2) {
            cout << " | ";
        }
        ctr++;
    }
    cout << std::endl;
}

bool check_faces_equality (const vectorized_faces& faces1, const vectorized_faces& faces2 ) {
    const bool verbose_about_mismatches = false;
    // auto cfaces = sort_faces(faces);
    //cout << "-.1==" << std::endl;
    auto faces_indices1 = faces_indices(faces1);
    //cout << "-.2==" << std::endl;
    auto faces_indices2 = faces_indices(faces2);
    std::sort(faces_indices1.begin(), faces_indices1.end());
    std::sort(faces_indices2.begin(), faces_indices2.end());

    cout << "faces1: ";
    print_faces(faces1, -1);
    cout << "faces2: ";
    print_faces(faces2, -1);
    cout << "faces_indices1: ";
    pf(faces_indices1);
    cout << "faces_indices2: ";
    pf(faces_indices2);
    cout << "; " << std::endl;

    //cout << "-.1-" << std::endl;
    bool ok = true;
    ok = ok && faces1.shape()[0] == faces2.shape()[0];
    //cout << "-.2-" << std::endl;
    ok = ok && faces1.shape()[1] == faces2.shape()[1];
    //cout << "-.3-" << std::endl;
    ok = ok && faces_indices1.shape()[0] == faces_indices2.shape()[0];
    //cout << "*" << std::endl;
    for (int i = 0; i < std::min(faces_indices1.shape()[0],faces_indices2.shape()[0]); ++i ) {
        //cout << "i*" << std::endl;
        ok = ok && (faces_indices1[i] == faces_indices2[i]);
        if (verbose_about_mismatches) {
            if (!(faces_indices1[i] == faces_indices2[i])) {
                cout << faces_indices1[i] << " != " << faces_indices2[i] << std::endl;
            }
        }
    }
    return ok;
}

std::vector<REAL> flatten_verts(const vectorized_vect& verts) {
    std::vector<REAL> flat;
    for (auto v3 : verts) {
        // flat.insert(v3.begin(), v3.end());

        flat.push_back(v3[0]);
        flat.push_back(v3[1]);
        flat.push_back(v3[2]);

    }
    assert(flat.size() == verts.shape()[0] * 3);
    return flat;
}


template <typename V>
void pv(V flat_verts1) {
    int ctr = 0;
    for( auto i = flat_verts1.begin(), e = flat_verts1.end(); i!= e; ++i) {
        cout << *i;
        ++ctr;
        if (ctr % 3 != 0) {
            cout << ",";
        } else {
            cout  << ";  ";
        }
    }
    cout << std::endl;
}



bool check_verts_equality (const vectorized_vect& verts1, const vectorized_vect& verts2 ) {
    const bool verbose_about_mismatches = true;
    std::vector<REAL>  flat_verts1 = flatten_verts(verts1);
    std::vector<REAL>  flat_verts2 = flatten_verts(verts2);

    cout << "flat_verts1: ";
    pv(flat_verts1);
    cout << "flat_verts2: ";
    pv(flat_verts2);
    cout << "; " << std::endl;

    bool ok = true;
    ok = ok && verts1.shape()[0] == verts2.shape()[0];
    ok = ok && verts1.shape()[1] == verts2.shape()[1];
    ok = ok && flat_verts1.size() == flat_verts2.size();
    int m = std::min(flat_verts1.size(), flat_verts2.size());
    for (int i = 0; i < m; ++i ) {
        ok = ok && (flat_verts1[i] == flat_verts2[i]);
        if (verbose_about_mismatches) {
            if (!(flat_verts1[i] == flat_verts2[i])) {
                cout << flat_verts1[i] << " != " << flat_verts2[i] << std::endl;
            }
        }
    }
    return ok;
}


/*
    Cannot make it a loop in C++. How do you update a multi_array?
         faces_old = std::move(result_faces); ? Doesn't work
*/
std::pair<vectorized_faces, vectorized_vect> subdivide_recursively(
    const std::pair<vectorized_faces, vectorized_vect>& fv,
    int fi,
    int n,
    midpointmap_type& midpoint_map
) {
    if (n==0) {
        return fv;
    }

    std::set<faceindex_type> triangles_to_subdivide {0};  // one face only
    if (bool all_faces = true) {
        for (int i = 0; i < fv.first.shape()[0]; ++i) {  // all faces
            triangles_to_subdivide.insert(i);
        }
    }

    cout << "Going to subdivide the follinwg faces: ";
    for (auto i = triangles_to_subdivide.begin(); i != triangles_to_subdivide.end(); ++i) {
        cout << *i << ", ";
    }
    cout << std::endl;

    auto result_vf_tuple = subdivide_multiple_facets_1to4 (
        fv.first, fv.second, triangles_to_subdivide, midpoint_map);
    auto result_verts = std::get<0>(result_vf_tuple);
    auto result_faces = std::get<1>(result_vf_tuple);
    auto result_vf_pair = std::make_pair(result_faces, result_verts);

    if (bool verbose = false) {
        print_faces(fv.first, fv.first.shape()[0]-1);
        cout << " --> " << std::endl;
        print_faces(result_vf_pair.first, fv.first.shape()[0]-1);
        cout << "faces: " <<  result_vf_pair.second.shape()[0] << std::endl;
        cout << "vertices: " <<  result_vf_pair.first.shape()[0] << std::endl;
    }

    return subdivide_recursively(result_vf_pair, fi, n-1, midpoint_map);
}

int serpinski_expected_f(int n) {
    if (n==0)
        return 1;
    return serpinski_expected_f(n-1) * 4;
}

/*
    f(n) = (f==0? 1 ) || (f(n-1) * 4)   // faces of serpinski
    v(n) = (v==0? 3 ) || (v(n-1) * 4 - (3 + p(n-1) ) )  // vertices of serpinski
    p(n) = 3*2**n  // perimeter of the serpinski triangle
*/
int serpinski_expected_v(int n) {
    if (n==0)
        return 3;
    return serpinski_expected_v(n-1) * 4 - ( 3 + 3 * std::pow(2, n-1) );
}


#include "../subdivision/propagate_subdiv.hpp"
#include "../subdivision/edge_triplets.hpp"


#include "../subdivision/do_subdivision.hpp"
#include "../object_factory.hpp"

using mp5_implicit::subdivision::do_subdivision;
using mp5_implicit::subdivision::subdivide_given_faces;



std::pair< std::vector<REAL>, std::vector<vertexindex_type>> make_a_square(REAL size) {
    std::vector<REAL> verts = {
        0,0,0, 1,0,0, 1,1,0, 0,1,0
    };

    std::vector<vertexindex_type> faces = {
        0,2,1, 0,3,2,
    };

    for (auto& v : verts) {
        v -= 0.5;
        v *= size;
    }

    return std::make_pair(verts, faces);
}

TEST(subdivision_1to4, square) {

    cout << "SQUARE******************" << std::endl;

    auto vf = make_a_square(10.0);

    auto verts = vects2vects(vf.first);
    auto faces = copy_faces_from_vectorfaces(vf.second);
    pv(flatten_verts(verts));
    print_faces(faces, -1);
    cout << "****************" << std::endl;


    std::set<faceindex_type>  which_facets_set;
    which_facets_set.insert(0);

    auto fv2 = subdivide_given_faces (faces, verts, which_facets_set);
    auto f2 = std::get<0>(fv2);
    auto v2 = std::get<1>(fv2);

    pv(flatten_verts(verts));
    pv(flatten_verts(v2));

    print_faces(faces, -1);
    print_faces(f2, -1);
    EXPECT_EQ(f2.shape()[0], 2-1+4);
    EXPECT_EQ(v2.shape()[0], 4+3);
    // EXPECT_TRUE(check_faces_equality(f2, faces));
    // EXPECT_TRUE(check_verts_equality(v2, verts));
}
