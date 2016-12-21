'use strict';
// importScripts('../build/mcc2.compiled.js');
// GET ... .js.mem 404 (File not found)
// mcc2.compiled.js:96101 Uncaught could not load memory initializer mcc2.compiled.js.mem
// Module.filePackagePrefixURL  (prefix only)
// Module.locateFile  (fiull filename path; callback)
var Module={};
//Module.filePackagePrefixURL = ... // this is ignored
Module.locateFile = function(){return '../../build/mcc2.compiled.js.mem';};
importScripts('../../build/mcc2.compiled.js');

'use strict';

onmessage = function(event) {
    var api = w_impli2;
    // console.log('Message received from main script. event.data=',event.data);
    var data = event.data;
    var _call_id = data.call_id;  // call_identification
    var _callback_id = event.data.callbackId;
    switch (data.funcName) {
        case "query_implicit_values":
            var shape_id = data.obj_id;  // data.obj_req_id
            var result =
                api.query_implicit_values(data.mp5_str, data.points, data.reduce_callback);
            postMessage({return_callback_id:_callback_id, returned_data: result, call_id: _call_id});  // {result_allpositive:}
            break;

        case "make_geometry":
            var shape_index = data.obj_req_id;
            var result =
                api.make_geometry__workerside(data.mp5_str, data.polygonization_settings, _call_id, shape_index);  // _call_id not needed here actually
            postMessage({return_callback_id:_callback_id, returned_data: result, call_id: _call_id, shape_index:shape_index});  // {result_allpositive:}
            break;

        case "get_latest_vf":
            var shape_index = data.shape_id;
            var result_vf_output = {faces: null, verts: null};

            var nonempty = api.get_latest_vf(result_vf_output, _call_id, shape_index);  // _call_id not used here actually
            result_vf_output.nonempty = nonempty;
            // if not nonempty, returns nonempty==false
            postMessage({return_callback_id:_callback_id, returned_data: result_vf_output, call_id: _call_id, shape_index: shape_index});  // {result_allpositive:}
            // definitely box the dict
            break;

        case "query_implicit_values_old":
            var shape_index = data.shape_id;
            assert(
                //data.reduce_type === "all-outside" ||
                //data.reduce_type === "all-non-positive" ||
                //data.reduce_type === "<=0"
                data.reduce_type === "any-inside" ||
                data.reduce_type === "any-positive" ||
                data.reduce_type === ">0"
            );

            var epsilon = data.epsilon;

            // old-style. The new style should get the string on the C++ side and do the reduction there.

            var reduce_callback = function(values_tarray) {
                'use asm';
                const minus_eps = +(-epsilon);
                var found_positive_val = false|0;
                for (var i = 0|0; i < values_tarray.length|0; i++) {
                    if (values_tarray[i] >= +minus_eps ) {
                        found_positive_val = true|0;
                        break;
                    }
                }
                return found_positive_val;
            };

            var result_values = api.query_implicit_values_old(data.mp5_str, data.points, reduce_callback, _call_id, shape_index);
            // result_values.shape_index = shape_index;  // where this should be put?

            postMessage({return_callback_id:_callback_id, returned_data: result_values, call_id: _call_id, shape_index: shape_index});  // {result_allpositive:}
            break;

        case "query_a_normal":
            var shape_index = data.shape_id;
            var result_normal = new Float32Array(3);
            result_normal[0] = undefined;  // FOR DEBUG
            var result_callback = function(gradient_vector) {
                result_normal.set(gradient_vector);
            }
            api.query_a_normal(data.mp5_str, data.point, result_callback, _call_id, shape_index);
            assert(result_normal[0] === result_normal[0]);  // check it's not NaN

            postMessage({return_callback_id:_callback_id, returned_data: result_normal, call_id: _call_id, shape_index: shape_index});  // {result_allpositive:}
            break;

        default:
            console.error("Unknown worker request for unknown function call: \""+(data.funcName)+"\" via event data:", event.data);
            throw new Error("Unrecognised worker request (unknown function call)");
    }
    //postMessage({result_allpositive: result});
    // postMessage({result_data: event.data});
/*
           {
                funcName: 'query_implicit_values',
                callbackId: callbackId,
                data: {mp5_str: '', points: new Floar32Array(), reduce_callback: 'allpositive'},
            }
*/
}

var assert__ = function(x, m){if(!x) {console.error(m,x);throw m;}}

// contains those functions in "Module" that have string arguments.
var impli1 = {
    //Based on API Version 1:
    // only functions that receive 'string' arguments need to be cwrap()ed.
    set_object: Module.cwrap('set_object','number',['string','number']),
    build_geometry: Module.cwrap('build_geometry', null, [ 'string', 'string']),

    // API Version 2:
};

var w_impli2 = {};
w_impli2._module = Module;

w_impli2.needs_deallocation = false;  // state

    w_impli2.query_implicit_values = function(mp5_str, points, reduce_type, call_id_arg)
    {
        assert__(points instanceof Float32Array);
        assert__(points.length % 3 === 0);
        assert__(typeof reduce_type === 'string');
        // assert__(['allpositive',].indexOf(redureduce_typereduce_typece_type) > -1);             /* the callback that produces the return value of the function */
        const ENUM_ALL_NON_POSITIVE = 2;
        const ENUM_ALL_NEGATIVE = 3;
        const ENUM_ERROR = 123456;
        var reduce_type_enum =
        (    (reduce_type === "any-inside" ||
                reduce_type === "any-positive" ||
                reduce_type === ">0" ) ? ENUM_ALL_NON_POSITIVE
            : (
                (reduce_type === "any-negative" || reduce_type === ">=0") ? ENUM_ALL_NEGATIVE
                : (
                    ENUM_ERROR
                )
            )
        );

        assert(reduce_type_enum === ENUM_ALL_NON_POSITIVE);

        var _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;                    /* We ll need the _FLOAT_SIZE in bytes, when we deal with allocations and HEAPF32*/
        var nverts = points.length / 3;                                       /* the number of vertices for which we want to calculate the implicit value */
        /* This allocates enough space for vertices (points) in the C++ memory. */
        var verts_space_address = Module._malloc(_FLOAT_SIZE * 3 * nverts);
        /* save the given values into C++ memory. The index in that memory will be used for calling the actual function. */
        /* Note:
            This seems to be too many steps: malloc, copy the Float32Array (which is already made by the caller), then_set_x() copies it into a temporary memory location. The _set_x() could have been avoided by sending a pointer into _calculate_implicit_values(). But it would need to also allocate the resulting values. If the Reduce (see below) is done on C++, this second solution will be more relevant, because we won't need to allocate space for the results. Because they don't need to be communicated back. It would just need to send back the reduces value (a single value) which is a boolean.
            We must copy (using .set() ), because, although both are of type Float32Array, but points cannot be alraedy on C++ memory. The reason is, in that case it would need to have called Module._malloc() before the current function.
        */
        Module.HEAPF32.subarray(verts_space_address / _FLOAT_SIZE, verts_space_address / _FLOAT_SIZE + 3 * nverts
            ).set(points);

        var ignore_root_matrix = false;
        /* obj_id is an integer that will match to the function _unset_object(obj_id). This integer will be used to identify one object among multiple objects that are available on the C++ side. */
        var obj_id = /* Module. */ impli1.set_object(mp5_str, ignore_root_matrix);

        /* Consume the given vertices. The vertices are given as an integer, the index of their start in C++ mamory. */
        var success = Module._set_x(verts_space_address, nverts);
        /* If there are a problem, such as memory roblems, the result is 0. */
        //todo: rename "success"
        if (!success) {
            console.log("set_x returned false . Probably an error in memory allocation. nverts was: " + nverts);
            Module._unset_object(obj_id);
            return false;
        }
        Module._calculate_implicit_values();                                /* The actual implicit value calculation*/

        var ptr = Module._get_values_ptr();           /* retrieve a pointer to the position in C++ memory, containing the results of the calculated array of implicit_values. */
        var ptr_len = Module._get_values_size();   /* We need the size too. This is a separate function call because we cannot return more than one value, or a struct in Emscripten (There may be a better way such as using a struct). */
        var values_tarray = Module.HEAPF32.subarray(ptr / _FLOAT_SIZE , ptr / _FLOAT_SIZE + ptr_len ); /* get a 'view' of that part of C++ memory as a Float32Array object. */

        /* Generate one boolean from the array. It resembles reduce() in MapReduce. */
        /* The following code can be moved into the C++ code. In this case, we don't need to query get_values_ptr(), etc. It will also be faster on C++ side. */
        var result = null;
        switch (reduce_type_enum) {
            case ENUM_ALL_NON_POSITIVE:

    	    var reduce_callback = function(values_tarray) {
                const CONFIG__collision_detection__epsilon = 0.001;
        		var found_positive_val = false;
        		for (var i=0; i < values_tarray.length; i++)
        		{
        		    if (values_tarray[i] >= - CONFIG__collision_detection__epsilon)
        		    {
        		        found_positive_val = true;
        		        break;
        		    }
        		}
        		var result =  found_positive_val;
        		return result;
    	    };

            result = reduce_callback(values_tarray);

            break;
           otherwise:
               console.error("unrecognised reduced type: ", reduce_type);
        }

        Module._free( verts_space_address );  /* Free space allocated by JS (here) on C++ memory */
        Module._unset_object(obj_id);    /* Ask C++ to free its resources; i.e. the implicit_function object. */
        Module._unset_x(); /* Ask C++ to free the memory it used to pass the verts. */
        return  result;
    }

    
    w_impli2.query_implicit_values_old = function(mp5_str, points, reduce_callback, call_id, shape_index)
    {
        //todo: inside of this function needs refactoring.
        assert(points instanceof Float32Array);
        assert(typeof reduce_callback === 'function');

        assert(typeof mp5_str === "string");

        /* Declarations */

        var _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;
        var nverts = points.length / 3;
        var verts_space_address = Module._malloc(_FLOAT_SIZE * 3 * nverts);

        /* body */
        Module.HEAPF32.subarray(
            verts_space_address / _FLOAT_SIZE, verts_space_address / _FLOAT_SIZE + 3 * nverts
        ).set(points);

        const ignore_root_matrix = false;

        var obj_id = impli1.set_object(mp5_str, ignore_root_matrix);
        var success = Module. _set_x(verts_space_address, nverts);
        //todo: rename "success"
        if (!success){
            console.log("set_x returned false . Probably an error in memory allocation. nverts was: " + nverts);
            Module. _unset_object(obj_id);
            return false;
        }

        Module. _calculate_implicit_values();

        var ptr = Module. _get_values_ptr();
        var ptr_len = Module. _get_values_size()
        var values_tarray = Module.HEAPF32.subarray(ptr / _FLOAT_SIZE , ptr / _FLOAT_SIZE + ptr_len );

        var result = reduce_callback(values_tarray);

        Module._free( verts_space_address );
        Module. _unset_object(obj_id);
        Module. _unset_x();
        return  result;
    }


    w_impli2.query_a_normal = function(mp5_shape_json, point, result_callback, call_id, shape_index) {
        // Always takes the root matrix into account. Similar to query_implicit_values()
        const ignore_root_matrix = false;
        var objid = impli1.set_object(mp5_shape_json, ignore_root_matrix);

        var input_verts = new Float32Array([point.x, point.y, point.z]);

        var nverts = 1;                                       // the number of vertices for which we want to calculate the implicit value
        const _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;
        var verts_space_address = Module._malloc(_FLOAT_SIZE * 3 * nverts);  // This allocates space in the C++ side, in order to create the array of vertices.
        Module.HEAPF32.subarray(verts_space_address / _FLOAT_SIZE, verts_space_address / _FLOAT_SIZE + 3 * nverts).set(input_verts);

        var setx_resut__address = Module. _set_x(verts_space_address, nverts*3);
        if (!setx_resut__address){
            console.log("set_x returned false: nverts is: " + nverts);
            Module. _unset_object(objid);
            return false;
        } else {
            // console.log("OK! set_c successful");
        }

        Module. _calculate_implicit_gradients(true);  // true = normalise and invert
        var ptr = Module. _get_gradients_ptr();                                 // retrieve a pointer to the position in memory of the calculated array
        var ptr_len = Module. _get_gradients_size()
        var r = Module.HEAPF32.subarray(ptr / _FLOAT_SIZE , ptr / _FLOAT_SIZE + ptr_len );

        // console.log(r[0], r[1], r[2]);
        //output_vect3.set(-r[0], -r[1], -r[2]);
        result_callback(r);

        Module. _unset_x();
        Module. _unset_object(objid);
        Module._free( verts_space_address );
    };




//based on  implisolid.js
    w_impli2.make_geometry__workerside = function (mp5_str, polygonization_settings_json, /*result_callback,*/ call_id_arg) {
        // assert(typeof result_callback !== 'undefined');
        // assert(result_callback);
        assert(typeof call_id_arg !== 'undefined');
        assert(polygonization_settings_json);
        assert(typeof polygonization_settings_json === "string");

        var startTime = new Date();
        const _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;
        const _INT_SIZE = Uint32Array.BYTES_PER_ELEMENT
        if(w_impli2.needs_deallocation) {
            /*impli1.*/ Module. _finish_geometry();
            w_impli2. needs_deallocation = false;
        }
        //console.log("mc_params.resolution " + mc_params.resolution);
        //mc_params.resolution = 40;
        //var mp5_str = JSON.stringify(shape_params);
        //var mp5_str = JSON.stringify(shape_params);
        /*Module.*/ impli1. build_geometry(mp5_str, polygonization_settings_json);
        w_impli2. needs_deallocation = true;
        var nverts = Module. _get_v_size();
        var nfaces = Module. _get_f_size();
        var verts_address = Module. _get_v_ptr();
        var faces_address = Module. _get_f_ptr();
        var verts = Module.HEAPF32.subarray(verts_address/_FLOAT_SIZE, verts_address/_FLOAT_SIZE + 3*nverts);
        var faces = Module.HEAPU32.subarray(faces_address/_INT_SIZE, faces_address/_INT_SIZE + 3*nfaces);
        // first iteration: using the callback
        //var geom = result_callback(verts, faces);
        var endTime = new Date();
        var timeDiff = endTime - startTime;
        //report_time(timeDiff, function(){hist();});
        //return geom;

        return {verts:verts, faces:faces};
    };

    w_impli2.get_latest_vf = function (output_vf_dict, call_id, shape_index) {
        const _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;
        const _INT_SIZE = Uint32Array.BYTES_PER_ELEMENT;
        const POINTS_PER_FACE = 3;

        // todo: use shape_index
        var nverts = Module. _get_v_size();
        var nfaces = Module. _get_f_size();

        if(nfaces > 0){
            var verts_address = Module. _get_v_ptr();
            var faces_address = Module. _get_f_ptr();

            var verts = Module.HEAPF32.subarray(
                verts_address/_FLOAT_SIZE,
                verts_address/_FLOAT_SIZE + 3*nverts);

            var faces = Module.HEAPU32.subarray(
                faces_address/_INT_SIZE,
                faces_address/_INT_SIZE + nfaces * POINTS_PER_FACE);
            // return {faces: faces, verts: verts};
            output_vf_dict['faces'] = faces;
            output_vf_dict['verts'] = verts;
            return true;
        } else {
            // empty
            output_vf_dict['faces'] = null;
            output_vf_dict['verts'] = null;
            return false;
        }
    }



// worker side
console.info("worker js loaded");
