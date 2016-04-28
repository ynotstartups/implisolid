import sys
sys.path.append("..")

from basic_types import check_vector4_vectorized
#from numerical_raycast_bisection_vectorized
#from numerical_utils import numerical_raycast_bisection_vectorized
import math


ROOT_TOLERANCE = 0.000001


def mysign_np(v):
    return np.sign(v) * (np.abs(v) > ROOT_TOLERANCE)



def bisection_pointwise1(iobj, x1_arr, x2_arr, ROOT_TOLERANCE=ROOT_TOLERANCE):
    """ x1_arr must be outside and x2_arr must be inside the object. Then this function finds points x=x1_arr+(lambda)*(x2_arr-x1_arr) where f(x)=0 using the bisection method."""
    check_vector4_vectorized(x1_arr)
    check_vector4_vectorized(x2_arr)
    assert x1_arr.shape[0] == x2_arr.shape[0]
    v1_arr = iobj.implicitFunction(x1_arr)
    #x2_arr = x1_arr + ray_n * 1.0
    x2_arr[:, 3] = 1
    v2_arr = iobj.implicitFunction(x2_arr)

    result_x_arr = np.zeros(x1_arr.shape)

    EPS = 0.000001  # sign

    n = x1_arr.shape[0]
    for i in range(n):
        active_indices = np.arange(0, n)  # mid
        iteration = 1
        while True:
            assert mysign_np(v2_arr[i]) * mysign_np(v1_arr[i] < 0 - EPS)  # greater or equal
            assert v1_arr[i] < 0-ROOT_TOLERANCE
            x_mid_arr = ( x1_arr[i,:] + x2_arr[i,:] ) / 2.0
            x_mid_arr[3] = 1
            v_mid_val = iobj.implicitFunction(x_mid_arr)[0]
            print v_mid_val
            exit()
            assert v_mid_val.shape[0] == 4
            assert v_mid_val.ndim == 1
            boolean_boundary = np.abs(v_mid_val) <= ROOT_TOLERANCE  #eq
            boolean_outside = v_mid_val < -ROOT_TOLERANCE  # gt
            boolean_inside  = v_mid_val > +ROOT_TOLERANCE  # -v_mid_val <  ROOT_TOLERANCE
            boolean_eitherside = np.logical_not(boolean_boundary)
            assert np.all(np.logical_or(boolean_outside, boolean_inside) == np.logical_not(boolean_boundary) )

            which_zeroed = active_indices[ boolean_boundary ] # new start = mid

            #already_root[which_zeroed] = 1  # iteration
            result_x_arr[which_zeroed,:] = x_mid_arr[boolean_boundary,:]


            #x1_arr and x2_arr should have the same size eventually. the boolean_boundary should be removed from their indices.
            #the total is np.arange(n)
            v2_arr[boolean_inside] = v_mid_val[boolean_inside]
            x2_arr[boolean_inside,:] = x_mid_arr[boolean_inside,:]

            #x1_arr and x2_arr both shrink here
            v1_arr[boolean_outside] = v_mid_val[boolean_outside]
            x1_arr[boolean_outside,:] = x_mid_arr[boolean_outside,:]

            v1_arr = v1_arr[boolean_eitherside]
            v2_arr = v2_arr[boolean_eitherside]
            x1_arr = x1_arr[boolean_eitherside,:]
            x2_arr = x2_arr[boolean_eitherside,:]
            active_indices = active_indices[boolean_eitherside]
            iteration += 1

            assert x1_arr.shape == x2_arr.shape
            assert v1_arr.shape == v2_arr.shape
            assert active_indices.shape == v1_arr.shape

            if len(active_indices) == 0:
                break

    assert len(active_indices) == 0
    v_arr = iobj.implicitFunction(result_x_arr)
    assert np.all(np.abs(v_arr) < ROOT_TOLERANCE)
    return result_x_arr



def bisection_vectorized2(iobj, x1_arr, x2_arr, ROOT_TOLERANCE=ROOT_TOLERANCE):
    """ x1_arr must be outside and x2_arr must be inside the object. Then this function finds points x=x1_arr+(lambda)*(x2_arr-x1_arr) where f(x)=0 using the bisection method."""
    check_vector4_vectorized(x1_arr)
    check_vector4_vectorized(x2_arr)
    assert x1_arr.shape[0] == x2_arr.shape[0]
    #x1_arr = x1_arr  # start
    v1_arr = iobj.implicitFunction(x1_arr)
    #x2_arr = x1_arr + ray_n * 1.0
    x2_arr[:, 3] = 1
    v2_arr = iobj.implicitFunction(x2_arr)

    result_x_arr = np.zeros(x1_arr.shape)

    EPS = 0.000001  # sign

    n = x1_arr.shape[0]
    #already_root = np.zeros((n,), dtype=np.int)
    #assert v2_arr * va > 0 - EPS  # greater or equal
    active_indices = np.arange(0, n)  # mid
    iteration = 1
    while True:
        if not np.all(mysign_np(v2_arr) * mysign_np(v1_arr) < 0 - EPS):
            #print np.max(mysign_np(v2_arr) * mysign_np(v1_arr))
            #i = mysign_np(v2_arr) * mysign_np(v1_arr) < 0 - EPS
            #print np.concatenate((v2_arr[i, np.newaxis], v1_arr[i, np.newaxis]), axis=1)
            # #print np.max(v2_arr), np.min(v1_arr)
            #print np.min(v2_arr), np.max(v1_arr)
            #print mysign_np(np.min(v2_arr)), mysign_np(np.max(v1_arr))
            pass
        assert np.all(mysign_np(v2_arr) * mysign_np(v1_arr) < 0 - EPS)  # greater or equal
        assert np.all(v1_arr < 0-ROOT_TOLERANCE)
        assert active_indices.shape[0] == x1_arr.shape[0]
        assert active_indices.shape[0] == x2_arr.shape[0]
        x_mid_arr = ( x1_arr + x2_arr ) / 2.0
        x_mid_arr[:,3] = 1
        v_mid_arr = iobj.implicitFunction(x_mid_arr)
        assert v_mid_arr.shape == active_indices.shape
        assert active_indices.ndim == 1

        assert v_mid_arr.shape == active_indices.shape
        boolean_boundary = np.abs(v_mid_arr) <= ROOT_TOLERANCE  #eq
        boolean_outside = v_mid_arr < -ROOT_TOLERANCE  # gt
        boolean_inside  = v_mid_arr > +ROOT_TOLERANCE  # -v_mid_arr <  ROOT_TOLERANCE
        boolean_eitherside = np.logical_not(boolean_boundary)
        assert np.all(np.logical_or(boolean_outside, boolean_inside) == np.logical_not(boolean_boundary) )

        which_zeroed = active_indices[ boolean_boundary ] # new start = mid

        #already_root[which_zeroed] = 1  # iteration
        result_x_arr[which_zeroed,:] = x_mid_arr[boolean_boundary,:]


        #x1_arr and x2_arr should have the same size eventually. the boolean_boundary should be removed from their indices.
        #the total is np.arange(n)
        v2_arr[boolean_inside] = v_mid_arr[boolean_inside]
        x2_arr[boolean_inside,:] = x_mid_arr[boolean_inside,:]

        #x1_arr and x2_arr both shrink here
        v1_arr[boolean_outside] = v_mid_arr[boolean_outside]
        x1_arr[boolean_outside,:] = x_mid_arr[boolean_outside,:]

        v1_arr = v1_arr[boolean_eitherside]
        v2_arr = v2_arr[boolean_eitherside]
        x1_arr = x1_arr[boolean_eitherside,:]
        x2_arr = x2_arr[boolean_eitherside,:]
        active_indices = active_indices[boolean_eitherside]
        iteration += 1

        assert x1_arr.shape == x2_arr.shape
        assert v1_arr.shape == v2_arr.shape
        assert active_indices.shape == v1_arr.shape

        if len(active_indices) == 0:
            break

    assert len(active_indices) == 0
    v_arr = iobj.implicitFunction(result_x_arr)
    assert np.all(np.abs(v_arr) < ROOT_TOLERANCE)
    return result_x_arr


def numerical_raycast_bisection_vectorized(iobj, ray_x, ray_target, ROOT_TOLERANCE=ROOT_TOLERANCE):
    """ ray_x must be outside and ray_x+ray_n must be inside the object. Then this function finds points x=ray_x+(lambda)*ray_n where f(x)=0 using the bisection method."""
    ray_n = ray_target - ray_x
    ray_n[:, 3] = 1
    check_vector4_vectorized(ray_x)
    check_vector4_vectorized(ray_n)
    assert ray_x.shape[0] == ray_n.shape[0]
    x1_arr = ray_x  # start
    v1_arr = iobj.implicitFunction(x1_arr)
    x2_arr = x1_arr + ray_n * 1.0
    x2_arr[:,3] = 1
    v2_arr = iobj.implicitFunction(x2_arr)

    result_x_arr = np.zeros(ray_x.shape)

    EPS = 0.0000001  # sign

    n = x1_arr.shape[0]
    already_root = np.zeros((n,), dtype=np.int)
    #assert v2_arr * va > 0 - EPS  # greater or equal
    active_indices  = np.arange(0,n)  # mid
    iteration = 1
    while True:
        #print(v1_arr.shape, "*")
        #print(np.vstack((v1_arr,v2_arr)))
        #print(v2_arr * v1_arr )
        #print("aaaaaaaaaaaaaaaa")
        #print(v1_arr)
        #print(v2_arr)
        #print(v2_arr * v1_arr)
        #assert np.all(v2_arr * v1_arr < 0 - EPS)  # greater or equal
        assert np.all(mysign_np(v2_arr) * mysign_np(v1_arr) < 0 - EPS)  # greater or equal

        assert np.all(v1_arr < 0-ROOT_TOLERANCE)
        assert active_indices.shape[0] == x1_arr.shape[0]
        assert active_indices.shape[0] == x2_arr.shape[0]
        x_mid_arr = ( x1_arr + x2_arr ) / 2.0
        x_mid_arr[:,3] = 1
        v_mid_arr = iobj.implicitFunction(x_mid_arr)
        assert v_mid_arr.shape == active_indices.shape
        assert active_indices.ndim == 1

        #flipped_i = v_mid_arr
        #contains the indices
        dif = -v_mid_arr  # assuming x1 is always outside and x2 is inside
        assert dif.shape == active_indices.shape
        boolean_eq = np.abs(dif) <= ROOT_TOLERANCE
        boolean_gt =  dif >  ROOT_TOLERANCE
        boolean_lt =  dif <  -ROOT_TOLERANCE # dif <  ROOT_TOLERANCE
        boolean_neq = np.logical_not( boolean_eq )
        #logical_or
        assert np.all( np.logical_or(boolean_gt, boolean_lt) == np.logical_not(boolean_eq) )
        #print("boolean_gt", boolean_gt)  #t
        #print("boolean_lt", boolean_lt)  #f


        which_zeroed     = active_indices[ boolean_eq ] # new start = mid
        which_flippedAt1 = active_indices[ boolean_gt ] # new end = mid
        which_flippedAt2 = active_indices[ boolean_lt ]
        which_flippedAny = active_indices[ boolean_neq ]

        already_root[which_zeroed] = 1  # iteration
        result_x_arr[which_zeroed,:] = x_mid_arr[boolean_eq,:]


        #x1_arr and x2_arr should have the same size eventually. the boolean_eq should be removed from their indices.
        #the total is np.arange(n)
        v2_arr[boolean_lt] = v_mid_arr[boolean_lt]#[which_flippedAny]
        x2_arr[boolean_lt,:] = x_mid_arr[boolean_lt,:]#[which_flippedAny]   # which_flippedAt2

        #x1_arr and x2_arr both shrink here

        v1_arr[boolean_gt] = v_mid_arr[boolean_gt]#[which_flippedAny]
        x1_arr[boolean_gt,:] = x_mid_arr[boolean_gt,:]#[which_flippedAny]   #which_flippedAt1

        v1_arr = v1_arr[boolean_neq]
        v2_arr = v2_arr[boolean_neq]
        x1_arr = x1_arr[boolean_neq,:]
        x2_arr = x2_arr[boolean_neq,:]
        #print("active_indices = ", active_indices)
        #print("which_flippedAny = ", which_flippedAny)
        #print("active_indices[which_flippedAny] = ", active_indices[boolean_neq])
        active_indices = active_indices[boolean_neq] #which_flippedAt1 || which_flippedAt2
        iteration += 1

        assert x1_arr.shape == x2_arr.shape
        assert v1_arr.shape == v2_arr.shape
        assert active_indices.shape == v1_arr.shape

        #print(active_indices)
        #print(v1_arr, "****", v2_arr)
        #print(boolean_lt)
        #print("*******")

        if len(active_indices) == 0:
            break

    assert len(active_indices) == 0
    #result_x_arr
    v_arr = iobj.implicitFunction(result_x_arr)
    assert np.all(np.abs(v_arr) < ROOT_TOLERANCE)
    return result_x_arr

bisection_vectorized1 = numerical_raycast_bisection_vectorized

# def
#     npoints = 1000
#     radius = 5
#     x = basic_types.make_random_vector_vectorized(npoints, radius, 1, type="rand", normalize=False)
#     x1_arr = x
#     x1_arr[:,0] += 4
#     x1_arr[:,1] += 10
#     ray_n = -x # iobj_.implicitGradient(x)
#     ray_n[:,3] = 1
#     #, rayscast=None
#     xx = numerical_utils.numerical_raycast_bisection_vectorized(iobj_, x1_arr, ray_n)



import numpy as np
global STEPSIZE


def getifunc():
    from example_objects import make_example_vectorized
    ifunc = make_example_vectorized("cube_example")
    (RANGE_MIN, RANGE_MAX, STEPSIZE) = (-3/5., +5/5., 0.2*1.5/1.5  *2. /2.)
    return ifunc, (RANGE_MIN, RANGE_MAX, STEPSIZE)


def prepare():
    ifunc, (RANGE_MIN, RANGE_MAX, STEPSIZE) = getifunc()

    from stl_tests import make_mc_values_grid
    gridvals = make_mc_values_grid(ifunc, RANGE_MIN, RANGE_MAX, STEPSIZE, old=False)
    verts, facets = vtk_mc(gridvals, (RANGE_MIN, RANGE_MAX, STEPSIZE))
    return verts, facets

# verts, f_ = prepare(*getifunc())

ifunc, (RANGE_MIN, RANGE_MAX, STEPSIZE) = getifunc()

global xi
global xo

n = 1000000*3
x = (np.random.rand(n, 4)*2-1)*(RANGE_MAX-RANGE_MIN)+RANGE_MIN
x[:, 3] = 1

f = ifunc.implicitFunction(x)

xi = x[f > ROOT_TOLERANCE*2, :]
xo = x[f < -ROOT_TOLERANCE*2, :]
print xi.shape, xo.shape
n_min = min(xi.shape[0], xo.shape[0])


#xio = xi[:n_min, :], xo[:n_min, :]
xi0 = xi[:n_min, :]
xo0 = xo[:n_min, :]
del xi, xo

#bisection_vectorized2

def testout(ifunc, q):
    f = ifunc.implicitFunction(q)
    print np.max(np.fabs(f)), ROOT_TOLERANCE
    return True

def test1():
    q = bisection_vectorized2(ifunc, xo, xi)
    assert testout(ifunc, q)

def test2():
    q = bisection_vectorized1(ifunc, xo, xi)
    assert testout(ifunc, q)

def test3():
    q = bisection_pointwise1(ifunc, xo, xi)
    assert testout(ifunc, q)


def optimised_used():
    global _optimised_used
    _optimised_used = True
    def side_effect():
        global _optimised_used
        _optimised_used = False
        return True
    assert side_effect()
    print "optimisation", _optimised_used
    return  _optimised_used

import matplotlib.pyplot as plt


def experiment1():
    #test1()
    #test2()
    na = [1,2,5,10, 100,200,400,600, 800,  1000, 2000, 10000, 100000, 200000, 300000] #, 1000000]

    test_scripts = ['test1()', 'test2()']
    sty = {0: "r*-", 1: "bs-"}
    lbl = {0: 'point-wise', 1: 'numpy vectorised'}

    tl = []
    for ei in range(len(na)):
        n = na[ei]
        #x = (np.random.rand(n, 4)*2-1)*(RANGE_MAX-RANGE_MIN)+RANGE_MIN
        #x[:, 3] = 1
        global xi
        global xo
        assert xi0.shape[0] >= n
        xi = xi0[:n, :]
        xo = xo0[:n, :]
        print "init:", ei,

        repeats = 1 # max(int(math.ceil(10/n)), 2)
        import timeit
        tt = ()
        for test_i in range(len(test_scripts)):
            #t1 = timeit.timeit(test_scripts[0], "from __main__ import test1", number=repeats)
            #t2 = timeit.timeit(test_scripts[1], "from __main__ import test2", number=repeats)
            t1 = timeit.timeit(test_scripts[test_i], "from __main__ import test"+str(test_i+1), number=repeats)
            tt += (t1/repeats,)
            print tt
        #tl.append((t1/repeats, t2/repeats))
        tl.append(tt)
        print "."
    ta = np.array(tl)
    print ta.shape
    print ta * 1000  # in msec
    print "ratio =", ta[:, 1]/ta[:, 0]

    naa = np.array(na)


    if optimised_used():
        asymp_ratio1 = 0.00001
        asymp_ratio2 = 0.00001
        asymp_bias = 0.01/2.
    else:
        asymp_ratio1 = 0.00001*2
        asymp_ratio2 = 0.00001*2
        asymp_bias = 0.01

    #g0 = plt.loglog(naa, naa/asymp[0]*asymp[1]/100, "b:", label='asympt')
    #sty = {'point-wise':"r*-", 'numpy vectorised':"bs-" }
    for j in range(len(sty)):
        #k = sty.keys()[j]
        #plt.loglog(naa, ta[:, j], k, label=sty[k])
        plt.loglog(naa, ta[:, j], sty[j], label=lbl[j])
    #g1 = plt.loglog(naa, ta[:, 1], "r*-", label='point-wise')
    #g2 = plt.loglog(naa, ta[:, 0], "bs-", label='numpy vectorised')
    g0 = plt.loglog(naa, naa * asymp_ratio1, "b:", label='asympt')
    g01 = plt.loglog(naa, naa * asymp_ratio2+ asymp_bias, "b:", label='asympt')
    plt.legend(loc='upper left', numpoints = 1)
    plt.show()

    #g1 = plt.plot(naa, ta[:, 1], "r*-", label='point-wise')
    g2 = plt.plot(naa, ta[:, 0], "bs-", label='numpy vectorised')
    plt.plot(naa, naa * asymp_ratio1, "b:", label='asympt')
    g0 = plt.plot(naa, naa * asymp_ratio2 + asymp_bias, "b:", label='asympt')
    plt.legend(loc='upper left', numpoints = 1)
    plt.show()

if __name__ == "__main__":

    experiment1()